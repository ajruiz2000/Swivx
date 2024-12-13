/* File: battery.c */

/** C file for the SwivX battery level **/


#include "battery.h"



/** @brief Function to initialize the SAADC for Battery Level reading */
uint32_t battery_level_init(void)
{
    ret_code_t err_code;

    nrfx_saadc_config_t saadc_config = NRFX_SAADC_DEFAULT_CONFIG;
    nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(BATTER_LEVEL_INPUT_PIN);

    err_code = nrfx_saadc_init(&saadc_config, battery_level_evt_handler);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = nrfx_saadc_channel_init(BATTER_LEVEL_INPUT_PIN, &channel_config);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

}

/** @brief Function to uninitialize the SAADC for Battery level Reading */
void battery_level_uninit(void)
{
    //Disable all channels and uninit the SAADC
    nrfx_saadc_uninit();
}


/** @brief Function for calculating the battery percent level */
uint8_t battery_level_update(void)
{
    ret_code_t err_code;
    uint16_t bat_lvl_16 = 0;
    uint8_t bat_level_value = 0;
    nrf_saadc_value_t bat_value_adc;

    err_code = nrfx_saadc_sample_convert(BATTER_LEVEL_INPUT_PIN, &bat_value_adc);
    APP_ERROR_CHECK(err_code);

    bat_lvl_16 = (((bat_value_adc * ADC_REF)/ADC_RESOLUTION) * ADC_GAIN)*2;
    //printf("Battery level %d\r\n", bat_lvl_16);

    //Battery level set statments
    if(bat_lvl_16 <= 3000)
    {
        bat_level_value = 0;
    }
    else if(bat_lvl_16 <= 3300)
    {
        bat_level_value = 5;
    }
    else if(bat_lvl_16 <= 3600)
    {
        bat_level_value = 10;
    }
    else if(bat_lvl_16 <= 3700)
    {
        bat_level_value = 20;
    }
    else if(bat_lvl_16 <= 3750)
    {
        bat_level_value = 30;
    }
    else if(bat_lvl_16 <= 3790)
    {
        bat_level_value = 40;
    }
    else if(bat_lvl_16 <= 3830)
    {
        bat_level_value = 50;
    }
    else if(bat_lvl_16 <= 3870)
    {
        bat_level_value = 60;
    }
    else if(bat_lvl_16 <= 3920)
    {
        bat_level_value = 70;
    }
    else if(bat_lvl_16 <= 3970)
    {
        bat_level_value = 80;
    }
    else if(bat_lvl_16 <= 4100)
    {
        bat_level_value = 90;
    }
    else
    {
        bat_level_value = 100;
    }

  return bat_level_value;
}





/** @brief Function ofr the SAADC handler */
static void battery_level_evt_handler(nrfx_saadc_evt_t const * p_evt)
{
    //Do nothing, not needed, but required for initialization
}