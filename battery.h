/* Header file battery.h */

/** Header file for the battery level **/



#ifndef BATTERY_H
#define BATTERY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "custom_board.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "app_timer.h"
#include "nrfx_saadc.h"

#define BATTER_LEVEL_INPUT_PIN  NRF_SAADC_INPUT_AIN5
#define BATTERY_DIV_SCALE       2     //Voltage divider from the battery resistors (both 100k)
#define ADC_GAIN                6     //ADC gain is 1/6
#define ADC_RESOLUTION          4096  //12bit ADC resolution setting
#define ADC_REF                 600   //ADC reference voltage

/** @brief Function to initialize the SAADC for reading the battery voltage */
uint32_t battery_level_init(void);

/** @brief Function for uninitializing the SAADC */
void battery_level_uninit(void);

/** @brief Function for calculating the battery percent level */
uint8_t battery_level_update(void);

/** @brief Function ofr the SAADC handler */
static void battery_level_evt_handler(nrfx_saadc_evt_t const * p_evt);

#endif