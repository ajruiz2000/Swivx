/* Header file kxtj3.h */

/** Header file for the KXTJ3 Accelerometer **/



#ifndef KXTJ3_H
#define KXTJ3_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

/** KXTJ3 I2C Address **/
#define ACCL_I2C_ADDR         0x0F
/* TWI instance ID. */
#define TWI_INSTANCE_ID       0

/** KXTJ3 Register definitions **/
#define XOUT_L                0x06
#define XOUT_H                0x07
#define YOUT_L                0x08
#define YOUT_H                0x09
#define ZOUT_L                0x0A
#define ZOUT_H                0x0B
#define CNTRL_REG1            0x1B
#define DATA_CNTRL_REG        0x21

/** KXTJ3 Mode definitions **/
#define RESO_HI_12_14Bit      0x40
#define RESO_LOW_8BIT         0x00
#define RANGE_4G              0x08
#define POWER_STNDBY_MODE     0x00
#define POWER_RUN_MODE        0x80
#define CNTRL_RESET           0x00

/** Averaging Definitions **/
#define SMOOTH_FACTOR         0.05
#define MOTOR_SMOOTH_FACTOR   0.01
#define BUFFER_SIZE           10

/** @brief KXTJ3  init object Struct **/
typedef struct
{
    uint8_t i2c_scl_pin;
    uint8_t i2c_sda_pin;
    uint8_t reso_mode;
    uint8_t range_mode;
} kxtj3_init_t;


/**@brief Function for initializing the KXTJ3 Accelerometer.
 *
 * @param[in]   p_accl_init  Information needed to initialize the KXTJ3 accelerometer.
 *
 * @return      NRF_SUCCESS on successful initialization, otherwise an error code.
 */
 uint32_t kxtj3_init(kxtj3_init_t * p_accl_init);


/**@brief Function for getting the current accelerometer angle.
 *
 * @return   float    calculated angle of the XYZ readouts of the KXTJ3.
 */
 float kxtj3_get_angle(void);


 /**@brief Function for calculating filtered angle.
 *
 * @return   float    calculated filtered angle.
 */
 uint8_t kxtj3_filter_angle(float unf_angle);


/**@brief Function to put the KXTJ3 accelerometer into standby/operating mode **/
uint32_t kxtj3_power_mode(uint8_t p_mode);


#endif