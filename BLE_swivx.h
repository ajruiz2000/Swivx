/* Header file BLE_swivx.h */

/** Header file for the custom BLE service
  * and characteristics for the SwivX device **/



#ifndef BLE_SWIVX_H
#define BLE_SWIVX_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "log.h"

//SwivX custom base UUID                // {8ec91800-f315-4f60-9fb8-838830daea50}
#define SWIVX_SERVICE_UUID_BASE         {0x73, 0x66, 0xA9, 0x46, 0xD4, 0x6E, 0x6B, 0xB2,              \
                                          0x13, 0x47, 0x83, 0xA8, 0x00, 0x14, 0x88, 0x8A}

#define SWIVX_SERVICE_UUID             0x1800
#define SWIVX_SETTINGS_CHAR_UUID       0x1801
#define SWIVX_MODE_CHAR_UUID           0x1802
#define SWIVX_LOG_CHAR_UUID            0x1803
#define SWIVX_DATA_CHAR_UUID           0x1804
#define SWIVX_BAT_CHAR_UUID            0x1805

extern bool is_ble_data_notifications_en;
extern bool is_ble_connected;
extern bool is_ble_log_notifications_en;

/**@brief   Macro for defining a Swivx custom BLE instance. Register with Softdevice Observer.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_SWIVX_DEF(_name)                                                                          \
static ble_swivx_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                   \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                       \
                     ble_swivx_on_ble_evt, &_name)

/** @brief types of events for SwivX service **/
typedef enum
{
    BLE_SWIVX_EVT_DATA_NOTIFICATION_ENABLED,                             
    BLE_SWIVX_EVT_DATA_NOTIFICATION_DISABLED,
    BLE_SWIVX_EVT_LOG_NOTIFICATION_ENABLED,
    BLE_SWIVX_EVT_LOG_NOTIFICATION_DISABLED,
    BLE_SWIVX_EVT_SETTINGS_WRITTEN,
    BLE_SWIVX_EVT_MODE_WRITTEN,
    BLE_SWIVX_EVT_DISCONNECTED,
    BLE_SWIVX_EVT_CONNECTED
} ble_swivx_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_swivx_evt_type_t evt_type;                                  /**< Type of event. */

} ble_swivx_evt_t;

// Forward declaration of the ble_swivx_t type.
typedef struct ble_swivx_s ble_swivx_t;

/**@brief SwivX Service event handler type. */
typedef void (*ble_swivx_evt_handler_t) (ble_swivx_t * p_cus, ble_swivx_evt_t * p_evt,void * p_context);

/**@brief SwivX Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_swivx_evt_handler_t       evt_handler;                    /**< Event handler for the SwivX Application >**/
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  swivx_settings_char_attr_md;    /**< Initial security level for Settings characteristics attribute */
    ble_srv_cccd_security_mode_t  swivx_mode_char_attr_md;        /**< Initial security level for Mode characteristics attribute */
    ble_srv_cccd_security_mode_t  swivx_log_char_attr_md;         /**< Initial security level for Log characteristics attribute */
    ble_srv_cccd_security_mode_t  swivx_data_char_attr_md;        /**< Initial security level for Data characteristics attribute */
    ble_srv_cccd_security_mode_t  swivx_bat_char_attr_md;         /**< Initial security level for Battery characteristics attribute */
} ble_swivx_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_swivx_s
{
    ble_swivx_evt_handler_t       evt_handler;                 /**< Event handler for the SwivX Application >**/
    uint16_t                      service_handle;              /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      swivx_settings_handles;      /**< Handles related to the SwivX Settings characteristic. */
    ble_gatts_char_handles_t      swivx_mode_handles;          /**< Handles related to the SwivX Mode characteristic. */
    ble_gatts_char_handles_t      swivx_log_handles;           /**< Handles related to the SwivX Log characteristic. */
    ble_gatts_char_handles_t      swivx_data_handles;          /**< Handles related to the SwivX Data characteristic. */
    ble_gatts_char_handles_t      swivx_bat_handles;           /**< Handles related to the SwivX Battery characteristic. */
    uint16_t                      conn_handle;                 /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};


/**@brief Function for initializing the SwivX Custom Service.
 *
 * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_swivx_init(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the SwivX Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_swivx_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for updating the swivX data Characteristic.
 *
 * @details The application calls this function when the data value should be updated. If
 *          notification has been enabled, the data characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_cus          Custom Service structure.
 * @param[in]   Data value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_swivx_data_update(ble_swivx_t * p_cus, uint8_t data_value);

/**@brief Function for updating the swivX data Characteristic.
 *
 * @details The application calls this function when the Log packet should be updated. If
 *          notification has been enabled, the Log characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_cus          Custom Service structure.
 * @param[in]   Packet value   18 byte packet of angle data nad timestamp
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_swivx_log_packet_send(ble_swivx_t * p_cus, uint8_t * packet_value);

/** @brief Function to update the GATT database with current settings register values **/
uint32_t ble_swivx_settings_update(ble_swivx_t * p_cus);

/** @brief Function to update the GATT database with current battery level **/
uint32_t ble_swivx_bat_value_update(ble_swivx_t * p_cus, uint8_t data_value);

/** static functions declarations **/
static uint32_t swivx_settings_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init);
static uint32_t swivx_mode_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init);
static uint32_t swivx_log_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init);
static uint32_t swivx_data_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init);
static uint32_t swivx_bat_char_add(ble_swivx_t * p_cus, const ble_swivx_init_t * p_cus_init);
static void on_connect(ble_swivx_t * p_cus, ble_evt_t const * p_ble_evt);
static void on_disconnect(ble_swivx_t * p_cus, ble_evt_t const * p_ble_evt);
static void on_write(ble_swivx_t * p_cus, ble_evt_t const * p_ble_evt);
static void split_register_to_array(uint8_t * reg_array);
static void settings_register_write(uint8_t * data_packet);


#endif