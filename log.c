/* File: log.c */

/** C file for the data log functionality **/


#include "log.h"

static bool volatile fds_is_init;
settingsStruct settings_register;
static uint8_t angleLogBuffer[LOG_BYTES_PER_PAGE]; 
static uint32_t data_counter = 0;
static uint8_t save_period_index = 0;
static uint8_t log_current_page = 0;
static bool isFlashWriting = false;
union timeStampUnion epoch_time;
uint32_t last_log_time = 0;
bool runGC = false;



/* @brief Function to init the log */
void log_fds_init(void)
{
    ret_code_t err_code;

    //Register FDS to recieve events
    (void) fds_register(log_fds_evt_handler);

    //init FDS
    err_code = fds_init();
    APP_ERROR_CHECK(err_code);

    while(!fds_is_init);

    //Init the settings register
    settings_reg_init();

    //init the Log buffer
    log_buffer_init();

}

/* Log Buffer init */
static void log_buffer_init(void)
{
    bool isValid = false;
    uint8_t current_page = settings_register.angleLogHead / LOG_BYTES_PER_PAGE;

    isValid = log_flash_read(current_page, angleLogBuffer);

    if(isValid)
    {
        NRF_LOG_INFO("found log page\r\n");
    }
    else
    {
        // Init the angleLogBuffer
        NRF_LOG_INFO("No Log, reinit\r\n");
        memset(angleLogBuffer, 0xFF, sizeof(angleLogBuffer));
    }
}

/* @brief Function to init the settings register */
static void settings_reg_init(void)
{
    bool recall_good;

    //check for flash settings
    recall_good = settings_reg_flash_recall();
    if(!recall_good)
    {
        NRF_LOG_INFO("No saved settings, reinit registers!");
        //recall failed, init with default values
        settings_register.angleLogHead = 0;
        settings_register.angleLogTail = 0;
        settings_register.angleMax = 70;
        settings_register.angleMin = 50;
        settings_register.motorDuration = 1000;
        settings_register.motorIntensity = 80;
        settings_register.motor_pulses = 3;
        settings_register.touchDuration = 30000;
        settings_register.timestamp = 0x5F7EC4FA;

        settings_reg_flash_write();
    }

    //Set current epoch time
    epoch_time.time = settings_register.timestamp;
    //Set Current data log buffer index
    data_counter = settings_register.angleLogHead % LOG_BYTES_PER_PAGE;
    log_current_page = (settings_register.angleLogHead) / LOG_BYTES_PER_PAGE;
 }


/* @brief Function for handling FDS events */
static void log_fds_evt_handler(fds_evt_t const * p_evt)
{
    switch(p_evt->id)
    {
        case FDS_EVT_INIT:
            if(p_evt->result == NRF_SUCCESS)
            {
                NRF_LOG_INFO("FDS Initialized!");
                fds_is_init = true;
            }
            break;

        case FDS_EVT_WRITE:
            if(p_evt->result == NRF_SUCCESS)
            {
                NRF_LOG_INFO("FDS data Saved!");
                isFlashWriting = false;
            }
            break;

        case FDS_EVT_GC:
            if(p_evt->result == NRF_SUCCESS)
            {
                NRF_LOG_INFO("FDS GC cleared");
                is_on_GC = false;
            }
            break;

    }
}

/* @brief Function for getting the current log size */
uint32_t get_log_size(void)
{
    if(settings_register.angleLogHead > settings_register.angleLogTail)
    {
        return (settings_register.angleLogHead - settings_register.angleLogTail);
    }
    else if(settings_register.angleLogTail > settings_register.angleLogHead)
    {
        return (LOG_MAX_BYTES - (settings_register.angleLogTail - settings_register.angleLogHead));
    }
    else
    {
        if(isLogFull)
        {
            return LOG_MAX_BYTES;
        }
        else
        {
            return 0;
        }
    }
}

/* @brief Function for resetting log full status */
void log_full_flush(void)
{
    ret_code_t err_code;

    isLogFull = false;
    settings_register.angleLogHead = 0;
    settings_register.angleLogTail = 0;
    data_counter = 0;
    save_period_index = 0;
    log_current_page = 0;
    err_code = settings_reg_flash_write();
    APP_ERROR_CHECK(err_code);

}

/* @brief Function for updating log buffer data */
bool log_write(uint8_t log_data)
{
    ret_code_t err_code;
    bool did_page_save = false;
    bool is_new_page = false;


    //Save angle data to buffer
    angleLogBuffer[data_counter] = log_data;
    data_counter++;
    save_period_index++;
    //printf("angle counter:%d\r\n", data_counter);

    // If buffered more than LOG_SAVE_PERIOD, save packet with timestamp       
    if(save_period_index == LOG_SAVE_PERIOD)
    {
        //Check if head and tail overlap
        if(settings_register.angleLogHead == settings_register.angleLogTail && isLogFull)
        {
            //Shift tail by 1 packet
            settings_register.angleLogTail += 18;
            //printf("tail: %d\r\n", settings_register.angleLogTail);

            if(settings_register.angleLogTail >= LOG_MAX_BYTES)
            {
                settings_register.angleLogTail = 0;
            }
        }

        //Reset period index counter
        save_period_index = 0;
        //Add EPOCH timestamp data to packet
        epoch_time.time = settings_register.timestamp;
        angleLogBuffer[data_counter] = epoch_time.bytes[3];
        angleLogBuffer[data_counter + 1] = epoch_time.bytes[2];
        angleLogBuffer[data_counter + 2] = epoch_time.bytes[1];
        angleLogBuffer[data_counter + 3] = epoch_time.bytes[0];
        data_counter += 4;
        settings_register.angleLogHead += 18;
        
        //printf("time stamp: %X %X\r\n", epoch_time.bytes[1], epoch_time.bytes[0]);
        printf("head: %d, tail: %d\r\n", settings_register.angleLogHead, settings_register.angleLogTail);
        err_code = log_flash_write(log_current_page);
        if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
        {
            //Did not catch full log, reset log data
            isLogFull = false;
            settings_register.angleLogTail = 0;
            settings_register.angleLogHead = 0;
        }
        err_code = settings_reg_flash_write();
        APP_ERROR_CHECK(err_code);
        last_log_time = millis();
        did_page_save = true;

        if(data_counter >= LOG_BYTES_PER_PAGE)
        {
            is_new_page = true;
        }

    }
    //If total log is more than a page, set new page
    if(is_new_page)
    {
        //Check if flash is full
        if(settings_register.angleLogHead >= LOG_MAX_BYTES)
        {
            //reset location
            isLogFull = true;
            settings_register.angleLogHead = 0;
            log_current_page = 0;
            //printf("page reset");
        }
        else
        {
            log_current_page++;
        }

        //Wait until whole page is written
        while(isFlashWriting);
        data_counter = 0;
        is_new_page = false;
    
        //Load next page into log buffer
        if(log_flash_read(log_current_page, angleLogBuffer));

        else
        {
            //No Log on next page
            //clear log buffer data for fresh page
            memset(angleLogBuffer, 0xFF, sizeof(angleLogBuffer)); 
        } 
    }

    return did_page_save;
}


/* @brief Function for saving Log data to flash */
uint32_t log_flash_write(uint8_t page)
{
    ret_code_t  err_code;
    uint16_t local_key = LOG_REC_BASE_KEY + page;

    if(page > LOG_MAX_PAGES)
    {
        isLogFull = true;
        err_code = FDS_ERR_NO_SPACE_IN_FLASH;
        return err_code;
    }

    fds_record_t          record;
    fds_record_desc_t    record_desc;
    fds_find_token_t      ftok;

    //setup record
    record.file_id = LOG_FILE_BASE_ID;
    record.key = local_key;
    record.data.p_data = &angleLogBuffer;
    record.data.length_words = ((sizeof(angleLogBuffer)+3) / 4 );
    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    //If record exists, update.
    if(fds_record_find(LOG_FILE_BASE_ID, local_key, &record_desc, &ftok) == NRF_SUCCESS)
    {
        err_code = fds_record_delete(&record_desc);
        APP_ERROR_CHECK(err_code);

        err_code = fds_record_write(&record_desc, &record);
        //APP_ERROR_CHECK(err_code);
        if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
        {
          NRF_LOG_INFO("Log ran out of space for real\r\n");
        }
        NRF_LOG_INFO("Log updating, page: %d\r\n", page);
        isFlashWriting = true;
        runGC = true;
    }
    else
    {
        err_code = fds_record_write(&record_desc, &record);
        NRF_LOG_INFO("Log creating, page: %d\r\n", page);
    }

    return err_code;
}


/* @brief Function for reading log flash data */
bool log_flash_read(uint8_t page, uint8_t * dataBuffer)
{
    ret_code_t err_code;
    bool isValid = false;

    fds_flash_record_t  record;
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok;
    uint16_t local_key = LOG_REC_BASE_KEY + page;

    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    //Check if log page exists
    if(fds_record_find(LOG_FILE_BASE_ID, local_key, &record_desc, &ftok) == NRF_SUCCESS)
    {
        if(fds_record_open(&record_desc, &record) != NRF_SUCCESS)
        {
            //error reading from flash, return error
            return isValid;
        }

        //Copy log data to buffer
        memcpy(dataBuffer, record.p_data, LOG_BYTES_PER_PAGE);
        isValid = true;
        fds_record_close(&record_desc);
    }
    else
    {
        isValid = false;
    }

    return isValid;
}


/* @brief Function for saving settings register to flash */
uint32_t settings_reg_flash_write(void)
{
    ret_code_t err_code;

    fds_record_t          record;
    fds_record_desc_t    record_desc;
    fds_find_token_t      ftok;

     //setup record
    record.file_id = SETTINGS_FILE_ID;
    record.key = SETTINGS_FILE_KEY;
    record.data.p_data = &settings_register;
    record.data.length_words = (sizeof(settings_register)); //24 bytes
    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    //If record exists, update.
    if(fds_record_find(SETTINGS_FILE_ID, SETTINGS_FILE_KEY, &record_desc, &ftok) == NRF_SUCCESS)
    {
        err_code = fds_record_delete(&record_desc);
        APP_ERROR_CHECK(err_code);

        err_code = fds_record_write(&record_desc, &record);
        APP_ERROR_CHECK(err_code);
        NRF_LOG_INFO("Settings registery updating.\r\n");
        isFlashWriting = true;
        runGC = true;
    }
    else
    {
        err_code = fds_record_write(&record_desc, &record);
        APP_ERROR_CHECK(err_code);
        NRF_LOG_INFO("Settings registry creating.\r\n");
    }

    return err_code;
}


/* @brief Function for recalling settings register from flash */
bool settings_reg_flash_recall(void)
{
    ret_code_t err_code;
    bool isValid = false;

    fds_flash_record_t  record;
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok;

    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    //Check if settings register exists
    if(fds_record_find(SETTINGS_FILE_ID, SETTINGS_FILE_KEY, &record_desc, &ftok) == NRF_SUCCESS)
    {
        if(fds_record_open(&record_desc, &record) != NRF_SUCCESS)
        {
            //error reading from flash, reinit settings register
            return isValid;
        }

        //Copy settings register from flash
        memcpy(&settings_register, record.p_data, sizeof(settingsStruct));
        isValid = true;
        fds_record_close(&record_desc);
    }

    return isValid;

}


/* @brief Function to delete a single log flash record */
uint32_t log_page_delete(uint8_t page)
{
    ret_code_t err_code;

    fds_flash_record_t  record;
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok;
    uint16_t local_key = LOG_REC_BASE_KEY + page;

    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    //Check if log page exists
    err_code = fds_record_find(LOG_FILE_BASE_ID, local_key, &record_desc, &ftok);

    if(err_code == NRF_SUCCESS)
    {
        //found page, delete it
        err_code = fds_record_delete(&record_desc);
        runGC = true;
        return err_code;
    }

    return err_code;
}


/* @brief Function to delete all log data */
uint32_t log_data_delete(void)
{
    ret_code_t err_code;

    /** Implement if app needs to delete all data. Currently, app can reset log by writting 0 to the angleHeadLog
    *   in the settings register.
    **/
}

/* @brief Function to start a garbage collection process */
void run_garbage_collection(void)
{
    ret_code_t err_code;
    runGC = false;
    is_on_GC = true;

    err_code = fds_gc();

}