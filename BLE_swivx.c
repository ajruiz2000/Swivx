
/* File: BLE_swivx.c */

/** C file for the SwivX custom BLE services and characteristics **/


#include "sdk_common.h"
#include "ble_srv_common.h"
#include "BLE_swivx.h"
#include <string.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"

bool is_ble_data_notifications_en = false;
bool is_ble_connected = false;
bool is_ble_log_notifications_en = false;

/**@brief Function for initializing the SwivX Custom Service. **/
uint32_t ble_swivx_init(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_cus->evt_handler               = p_cus_init->evt_handler;
    p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add SwivX Custom Service UUID
    ble_uuid128_t base_uuid = {SWIVX_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SWIVX_SERVICE_UUID;

    // Add the SwivX Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add SwivX Settings characteristic
    err_code = swivx_settings_char_add(p_cus, p_cus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add SwivX Mode characteristic
    err_code = swivx_mode_char_add(p_cus, p_cus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add SwivX Log characteristic
    err_code = swivx_log_char_add(p_cus, p_cus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add SwivX Data characteristic
    err_code = swivx_data_char_add(p_cus, p_cus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add SwivX Battery characteristic
    swivx_bat_char_add(p_cus, p_cus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
}


/**@brief Function for adding the SwivX Settings characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t swivx_settings_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t default_value[14];

    split_register_to_array(default_value);

    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->swivx_data_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->swivx_data_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SWIVX_SETTINGS_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 14;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 14;
    attr_char_value.p_value = default_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->swivx_settings_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


/**@brief Function for adding the SwivX Settings characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t swivx_mode_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->swivx_data_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->swivx_data_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SWIVX_MODE_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->swivx_mode_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


/**@brief Function for adding the SwivX Settings characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t swivx_log_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->swivx_data_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->swivx_data_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SWIVX_LOG_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 18;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 18;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->swivx_log_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


/**@brief Function for adding the SwivX Data characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t swivx_data_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->swivx_data_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->swivx_data_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SWIVX_DATA_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->swivx_data_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the SwivX Battery characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t swivx_bat_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL; 
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->swivx_bat_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->swivx_bat_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SWIVX_BAT_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->swivx_bat_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/** @brief Function for handling incoming ble events related to the SwivX Service **/
void ble_swivx_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_swivx_t * p_cus = (ble_swivx_t *) p_context;
    
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_cus, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_cus, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_cus, p_ble_evt);
           break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_swivx_t * p_cus, ble_evt_t const * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_swivx_evt_t evt;

    //propogate event to application handler
    evt.evt_type = BLE_SWIVX_EVT_CONNECTED;
    p_cus->evt_handler(p_cus, &evt, NULL);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_swivx_t * p_cus, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;

    ble_swivx_evt_t evt;

    //propogate event to application handler
    evt.evt_type = BLE_SWIVX_EVT_DISCONNECTED;
    p_cus->evt_handler(p_cus, &evt, NULL);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_swivx_t * p_cus, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_cus->swivx_settings_handles.value_handle)
    {
        uint8_t * data_packet;

        //if Mode is written
        ble_swivx_evt_t evt;
        
        data_packet = p_evt_write->data;
        settings_register_write(data_packet);

        evt.evt_type = BLE_SWIVX_EVT_SETTINGS_WRITTEN;
        p_cus->evt_handler(p_cus, &evt, NULL);
    }

    if (p_evt_write->handle == p_cus->swivx_mode_handles.value_handle)
    {
        //if Mode is written
        ble_swivx_evt_t evt;

        evt.evt_type = BLE_SWIVX_EVT_MODE_WRITTEN;
        p_cus->evt_handler(p_cus, &evt, p_evt_write->data);
    }
    
    // Check if the Log Char value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
    if ((p_evt_write->handle == p_cus->swivx_log_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        // CCCD written, call application event handler
        if (p_cus->evt_handler != NULL)
        {
            ble_swivx_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_SWIVX_EVT_LOG_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_SWIVX_EVT_LOG_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_cus->evt_handler(p_cus, &evt, NULL);
        }
    }

    // Check if the Data Char value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
    if ((p_evt_write->handle == p_cus->swivx_data_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        // CCCD written, call application event handler
        if (p_cus->evt_handler != NULL)
        {
            ble_swivx_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_SWIVX_EVT_DATA_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_SWIVX_EVT_DATA_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_cus->evt_handler(p_cus, &evt, NULL);
        }
    }

}

/**@brief Function for updating The SwivX Data characteristic and sending notification.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   uint8_t     Data to be updated
 */
uint32_t ble_swivx_data_update(ble_swivx_t * p_cus, uint8_t data_value)
{ 
    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &data_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                      p_cus->swivx_data_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cus->swivx_data_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }


    return err_code;
}

/**@brief Function for updating The SwivX Data characteristic and sending notification.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   uint8_t     Data to be updated
 */
uint32_t ble_swivx_log_packet_send(ble_swivx_t * p_cus, uint8_t * packet_value)
{ 
    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = 18;
    gatts_value.offset  = 0;
    gatts_value.p_value = packet_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                      p_cus->swivx_log_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cus->swivx_log_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }


    return err_code;
}

/** @brief Function for updating the GATT database for the settings register */
uint32_t ble_swivx_settings_update(ble_swivx_t * p_cus)
{
    ret_code_t err_code;

    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ble_gatts_value_t gatts_value;
    uint8_t settings_array[14];

    split_register_to_array(settings_array);

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = 14;
    gatts_value.offset  = 0;
    gatts_value.p_value = settings_array;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                      p_cus->swivx_settings_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return err_code;
}


/** @brief Function to update the GATT database with current battery level **/
uint32_t ble_swivx_bat_value_update(ble_swivx_t * p_cus, uint8_t data_value)
{
    ret_code_t err_code;

    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ble_gatts_value_t gatts_value;
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len = 1;
    gatts_value.offset = 0;
    gatts_value.p_value = &data_value;

    //Update database
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle, p_cus->swivx_bat_handles.value_handle,
                                       &gatts_value);
    if(err_code != NRF_SUCCESS)
    {
        return  err_code;
    }

    return err_code;
}

/* @brief Function to split the Settings Register to an array for the settings characteristic */
static void split_register_to_array(uint8_t * reg_array)
{
    reg_array[0] = settings_register.angleMin;
    reg_array[1] = settings_register.angleMax;
    reg_array[2] = (uint8_t)((settings_register.touchDuration >> 24) & 0x000000FF);
    reg_array[3] = (uint8_t)((settings_register.touchDuration >> 16) & 0x000000FF);
    reg_array[4] = (uint8_t)((settings_register.touchDuration >> 8) & 0x000000FF);
    reg_array[5] = (uint8_t)((settings_register.touchDuration) & 0x000000FF);
    reg_array[6] = (uint8_t)((settings_register.motorDuration >> 8) & 0x00FF);
    reg_array[7] = (uint8_t)((settings_register.motorDuration) & 0x00FF);
    reg_array[8] = settings_register.motorIntensity;
    reg_array[9] = settings_register.motor_pulses;
    reg_array[10] = (uint8_t)((settings_register.timestamp >> 24) & 0x000000FF);
    reg_array[11] = (uint8_t)((settings_register.timestamp >> 16) & 0x000000FF);
    reg_array[12] = (uint8_t)((settings_register.timestamp >> 8) & 0x000000FF);
    reg_array[13] = (uint8_t)((settings_register.timestamp) & 0x000000FF);
}

static void settings_register_write(uint8_t * data_packet)
{
    settings_register.angleMin = data_packet[0];
    settings_register.angleMax = data_packet[1];
    settings_register.touchDuration = (data_packet[2] << 24) + (data_packet[3] << 16) + (data_packet[4] << 8) + (data_packet[5]);
    settings_register.motorDuration = (data_packet[6] << 8) + (data_packet[7]);
    settings_register.motorIntensity = data_packet[8];
    settings_register.motor_pulses = data_packet[9];
    settings_register.timestamp = (data_packet[10] << 24) + (data_packet[11] << 16) + (data_packet[12] << 8) + (data_packet[13]);
}

