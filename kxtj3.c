/* File: kxtj3.c */

/** C file for the KXTJ3 Accelerometer functions **/


#include "kxtj3.h"
#include <string.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrfx_twi.h"
#include "math.h"
#include "motor.h"


/* TWI instance. */
static const nrfx_twi_t m_twi = NRFX_TWI_INSTANCE(TWI_INSTANCE_ID);
static kxtj3_init_t m_accl_init;
static bool twi_is_init = false;
static bool is_hi_res = true;
static float angle_buffer[10] = {0};
static float total_sum = 0;
static uint8_t buff_count = 0;
static uint32_t current_time = 0;

/* averaging variables */
float last_angle = -1;


/**@brief Function writing to the KXTJ3 by I2C
 *
 * @return   NRF_SUCCESS on successful initialization, otherwise an error code.
 */
 static uint32_t i2c_write(uint8_t *data, uint8_t length)
 {
    ret_code_t err_code;

    err_code = nrfx_twi_tx(&m_twi, ACCL_I2C_ADDR, data, length, false);
    return err_code;
 }

/**@brief Function reading from the KXTJ3 by I2C
 *
 * @return   NRF_SUCCESS on successful initialization, otherwise an error code.
 */
 static uint32_t i2c_read(uint8_t *data, uint8_t length)
 {
    ret_code_t err_code;

    err_code = nrfx_twi_rx(&m_twi, ACCL_I2C_ADDR, data, length);
    return err_code;
 }


/**@brief Function for initializing the KXTJ3 Accelerometer.
 *
 * @param[in]   p_accl_init  Information needed to initialize the KXTJ3 accelerometer.
 *
 * @return      NRF_SUCCESS on successful initialization, otherwise an error code.
 */
 uint32_t kxtj3_init(kxtj3_init_t * p_accl_init)
 {
    ret_code_t err_code;
    uint8_t txBuffer[5] = {0};        //I2C transmit buffer

    //Initialize the I2C module
    const nrfx_twi_config_t twi_kxtj3_config = {
        .scl                = p_accl_init->i2c_scl_pin,
        .sda                = p_accl_init->i2c_sda_pin,
        .frequency          = NRF_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .hold_bus_uninit     = false
    };

    err_code = nrfx_twi_init(&m_twi, &twi_kxtj3_config, NULL, NULL);      //Blocking mode
    APP_ERROR_CHECK(err_code);

    nrfx_twi_enable(&m_twi);
    twi_is_init = true;

    m_accl_init = * p_accl_init;

    //Initialize the KXTJ3 accelerometer with init settings
    txBuffer[0] = CNTRL_REG1;
    txBuffer[1] = CNTRL_RESET;
    err_code = i2c_write(txBuffer, 2);
    if(err_code != NRF_SUCCESS)
        return err_code;

    txBuffer[1] = 0x00 | p_accl_init->range_mode | p_accl_init->reso_mode | POWER_RUN_MODE;
    err_code = i2c_write(txBuffer, 2);

    if(p_accl_init->reso_mode == RESO_HI_12_14Bit)
    {
       is_hi_res = true;
    }
    
    return err_code;
 }


 /**@brief Function for getting the current accelerometer angle.
 *
 * @return   float    calculated angle of the XYZ readouts of the KXTJ3.
 */
 float kxtj3_get_angle(void)
 {
    ret_code_t err_code;
    uint8_t txBuffer[1] = {XOUT_L};
    uint8_t rxBuffer[7] = {0};

    if(twi_is_init != true)
    {
      NRF_LOG_INFO("TWI is not initialized");
      return 0;
    }
    
    //Read from the x,y,z axis registers of the KXTJ3
    err_code = i2c_write(txBuffer, 1);
    //APP_ERROR_CHECK(err_code);
    err_code = i2c_read(rxBuffer, 6);
    //APP_ERROR_CHECK(err_code);
    
    float x_valF = 0;
    float y_valF = 0;
    float z_valF = 0;

    if(is_hi_res)
    {
      //Shift into 16bit variables
      uint16_t x_u16_t  = (rxBuffer[1] << 8) | rxBuffer[0];
      uint16_t y_u16_t  = (rxBuffer[3] << 8) | rxBuffer[2];
      uint16_t z_u16_t  = (rxBuffer[5] << 8) | rxBuffer[4];

      //2's compliment
      uint16_t x_valT_16 = (x_u16_t & 0x8000 ? (~x_u16_t + 1) & 0xFFFF : x_u16_t) >> 4;
      uint16_t y_valT_16 = (y_u16_t & 0x8000 ? (~y_u16_t + 1) & 0xFFFF : y_u16_t) >> 4;
      uint16_t z_valT_16 = (z_u16_t & 0x8000 ? (~z_u16_t + 1) & 0xFFFF : z_u16_t) >> 4;

      x_valF = (float) x_valT_16;
      y_valF = (float) y_valT_16;
      z_valF = (float) z_valT_16;

      x_valF = x_valF / 512.0 * (x_u16_t & 0x8000 ? (-1.0) : (1.0));
      y_valF = y_valF / 512.0 * (y_u16_t & 0x8000 ? (-1.0) : (1.0));
      z_valF = z_valF / 512.0 * (z_u16_t & 0x8000 ? (-1.0) : (1.0));
    }
    else
    {
      //Shift into 16bit variables
      uint8_t x_u8_t  = rxBuffer[1];
      uint8_t y_u8_t  = rxBuffer[3];
      uint8_t z_u8_t  = rxBuffer[5];

      //2's compliment
      uint8_t x_valT_8 = (x_u8_t & 0x80 ? (~x_u8_t + 1) & 0xFF : x_u8_t);
      uint8_t y_valT_8 = (y_u8_t & 0x80 ? (~y_u8_t + 1) & 0xFF : y_u8_t);
      uint8_t z_valT_8 = (z_u8_t & 0x80 ? (~z_u8_t + 1) & 0xFF : z_u8_t);

      x_valF = (float) x_valT_8;
      y_valF = (float) y_valT_8;
      z_valF = (float) z_valT_8;

      x_valF = x_valF / 32.0 * (x_u8_t & 0x80 ? (-1.0) : (1.0));
      y_valF = y_valF / 32.0 * (y_u8_t & 0x80 ? (-1.0) : (1.0));
      z_valF = z_valF / 32.0 * (z_u8_t & 0x80 ? (-1.0) : (1.0));
    }

    if(z_valF == 0)
    {
      //printf("divid by zero\r\n");
      z_valF = 0.01;
    }
    //Calculate the angle
    float norm = sqrtf(x_valF * x_valF + y_valF * y_valF);
    float angle = (float)atan(-1 * norm/z_valF) * 180 / 3.14159;

    if(angle < 0.0f)
    {
      angle += 180.0f;
    }

    return angle;
 }


 /**@brief Function for calculating filtered angle.
 *
 * @return   uint8_t    calculated filtered angle.
 */
 uint8_t kxtj3_filter_angle(float unf_angle)
 {
    float f_angle = 0;
    //float temp_buff[5] = {0};


    angle_buffer[buff_count] = unf_angle;
    buff_count++;
    total_sum += unf_angle;

    if(buff_count >= 10)
    {
      f_angle = total_sum/BUFFER_SIZE;
      buff_count = 0;
      total_sum = 0;

      return (uint8_t)f_angle;
    }
    else 
    {
      return 200;
    }




/*
    memcpy(temp_buff, angle_buffer, sizeof(angle_buffer));
    angle_buffer[0] = unf_angle;
    total_sum = unf_angle;

    uint8_t i;
    for(i = 1; i<5; i++)
    {
        angle_buffer[i] = temp_buff[i-1];
        total_sum += temp_buff[i-1];
    }

    f_angle = total_sum / 5;
  
    current_time = millis();

    Extra filtering during motor
    if(!is_motor_stopped() && compare_millis(motor_roll_delay, current_time) > MOTOR_ROLL_TIMEOUT)
    {
        if(last_angle <= 0)
        {
            last_angle = f_angle;
        }

        // Filtering uses exponential smoothing.
        f_angle = last_angle + (SMOOTH_FACTOR)*(f_angle - last_angle); 
     }
    
    if(f_angle >= (last_angle + 2))
    {
        f_angle = (last_angle + 2);
    }
    else if(f_angle <= (last_angle-2))
    {
        f_angle = (last_angle - 2);
    }
    else
    {
    } 

    last_angle = f_angle;


    return (uint8_t)f_angle;
   */
 }


/**@brief Function to put the KXTJ3 accelerometer into standby/operating mode **/
uint32_t kxtj3_power_mode(uint8_t p_mode)
{
    ret_code_t err_code;
    uint8_t txBuffer[2] = {0};

    if(p_mode == POWER_STNDBY_MODE)
    {
        //place KXTJ3 into standby mode
        txBuffer[0] = CNTRL_REG1;
        txBuffer[1] = POWER_STNDBY_MODE;
        err_code = i2c_write(txBuffer, 2);
    }
    else
    {
        //place KXTJ3 into standby mode
        txBuffer[0] = CNTRL_REG1;
        txBuffer[1] = 0x00 | m_accl_init.range_mode | m_accl_init.reso_mode | POWER_RUN_MODE;
        err_code = i2c_write(txBuffer, 2);
    }

    return err_code;
    
}