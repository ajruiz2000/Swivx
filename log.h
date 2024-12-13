/* Header file log.h */

/** Header file for the data Logging **/



#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "fds.h"
#include "custom_board.h"

//Log Definitions
#define LOG_BYTES_PER_PAGE        (4032)    //1024 words per page, 4096 bytes per page (4032)
#define LOG_SAVE_PERIOD           (14)      //Byte chunks to buffer before writting to flash
#define LOG_MAX_PAGES             (21)      //Total number of pages for log data in flash
#define LOG_MAX_BYTES             (LOG_MAX_PAGES * LOG_BYTES_PER_PAGE)//(84672)   //Max bytes of data possilbe in Log (MAX_PAGES * 4032 Bytes per page)
#define LOG_PACKET_SIZE           (LOG_SAVE_PERIOD + 4) //size of save period plus 4 bytes for timestamp
#define LOG_PACKETS_PER_PAGE      (LOG_BYTES_PER_PAGE / LOG_PACKET_SIZE) //224 packets per page

//FDS definitions
#define LOG_FILE_BASE_ID      0x1112
#define LOG_REC_BASE_KEY      0x0001
#define SETTINGS_FILE_ID      0x1111
#define SETTINGS_FILE_KEY     0x2222

static bool isLogFull = false;
extern bool runGC;
static bool is_on_GC = false;
extern uint32_t last_log_time;


/** Settings register struct **/
typedef struct
{
    uint8_t angleMin;                         
    uint8_t angleMax;                        
    uint32_t touchDuration;                   
    uint16_t motorDuration;                   
    uint8_t motorIntensity;
    uint8_t motor_pulses;
    uint32_t angleLogHead;
    uint32_t angleLogTail;
    uint32_t timestamp;
} settingsStruct;

/* Current Time in EPOCH Format */
union timeStampUnion
{
    uint8_t bytes[sizeof(uint32_t)];
    uint32_t time;
};

extern settingsStruct settings_register;
extern union timeStampUnion epoch_time;

/* @brief Function to init the Log */
void log_fds_init(void);

/* @brief Function for updating Log buffer data */
bool log_write(uint8_t log_data);

/* @brief Function for saving Log data to flash */
uint32_t log_flash_write(uint8_t page);

/* @brief Function for reading log flash data */
bool log_flash_read(uint8_t page, uint8_t * dataBuffer);

/* @brief Function for getting the current log size */
uint32_t get_log_size(void);

/* @brief Function for resetting log full status */
void log_full_flush(void);

/* @brief Function to init the settings register */
static void settings_reg_init(void);

/* @brief Function to init the Log Buffer */
static void log_buffer_init(void);

/* @brief Function for saving settings register to flash */
uint32_t settings_reg_flash_write(void);

/* @brief Function for recalling settings register from flash */
bool settings_reg_flash_recall(void);

/* @brief Function to delete a single log flash record */
uint32_t log_page_delete(uint8_t page);

/* @brief Function to delete all log data */
uint32_t log_data_delete(void);

/* @brief Function to start a garbage collection process */
void run_garbage_collection(void);

/* @brief Function for handling FDS events */
static void log_fds_evt_handler(fds_evt_t const * p_evt);

#endif