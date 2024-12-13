/* File: capsense.c */

/** C file for the SwivX capacitive sensing functionality **/


#include "capsense.h"


/** Threshold for sensor activation **/
volatile uint32_t threshold_value = 1350; //3000
static capsense_state_t cap_state = CSENSE_STATE_UNINIT;
static bool capsense_driver_is_init = false;
bool touch_activated = false;
uint32_t last_touch_time = 0;


/**@brief Function for the Capacitive sensing initialization.
 *
 * @details Initializes the Capsense driver.
 */
 uint32_t capsense_init(void)
 {
    ret_code_t err_code;
    nrf_drv_csense_config_t csense_config;

    csense_config.output_pin = C1_CHRGE;

    err_code = nrf_drv_csense_init(&csense_config, capsense_handler);
    APP_ERROR_CHECK(err_code);

    //Pad 0 is set on AIN 3 (pin P05)
    nrf_drv_csense_channels_enable(PAD_0_MASK);

    capsense_driver_is_init = true;

    return err_code;
 }

/** @brief Function to return capsense driver init status **/
bool capsense_is_init(void)
{
    return capsense_driver_is_init;
}

/**@brief Function for the Capacitive sensing uninitialization.
 *
 * @details Uninitializes the Capsense driver.
 */
void capsense_uninit(void)
{
    ret_code_t err_code = nrf_drv_csense_uninit();
    capsense_driver_is_init = false;
}


/**@brief Function for the sampling the Capcitive sensor.
 *
 * @details samples the capacitive sensor.
 */
void capsense_sample(void)
{
    ret_code_t err_code;
    if(!nrf_drv_csense_is_busy())
    {
        err_code = nrf_drv_csense_sample();
        APP_ERROR_CHECK(err_code);
    }
}


 /**@brief Function for the Capacitive sensing Handler.
 *
 * @param[in]   p_event_struct - csense driver event
 *
 * @details Handles the capsense events.
 */
static void capsense_handler(nrf_drv_csense_evt_t * p_event_struct)
{
    switch(p_event_struct->analog_channel)
    {
        case AIN_3:
            printf("Capacitive sensor value: %d \n", p_event_struct->read_value);
            if(p_event_struct->read_value >= threshold_value && cap_state != CSENSE_STATE_PRESSED)
            {
                cap_state = CSENSE_STATE_PRESSED;
                touch_activated = true;
                last_touch_time = millis();
                printf("Sensor pressed \n");
            }
            else if (p_event_struct->read_value < threshold_value && cap_state == CSENSE_STATE_PRESSED)
            {
                cap_state = CSENSE_STATE_RELEASED;
                touch_activated = false;
                last_touch_time = millis();
                printf("Sensor released \n");
            }
            break;

        case AIN_5:
            break;

        default:
            break;
    }
}
