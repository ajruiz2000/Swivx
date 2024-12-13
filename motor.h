/* Header file motor.h */

/** Header file for the vibration motor **/



#ifndef MOTOR_H
#define MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "custom_board.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "nrfx_pwm.h"
#include "log.h"

#define MOTOR_CHECK_DELAY       100
#define MOTOR_INDICATE_DELAY    2000
#define MOTOR_ROLL_TIMEOUT      1000
#define MOTOR_HYST_DELAY        2000

/** Motor status states **/
typedef enum
{
    MOTOR_START,
    MOTOR_RUNNING,
    MOTOR_STOP
} motor_state_t;

/** Motor indicators **/
typedef enum
{
   MOTOR_ALERT_SHORT,
   MOTOR_ALERT_FEEDBACK,
   MOTOR_POWER_ON,
   MOTOR_BUTTON_PRESS,
} motor_indicate_t;

extern bool is_motor_on;
extern uint32_t motor_roll_delay;

/**@brief Function for the Motor and PWM initialization.
 *
 * @details Initializes the PWM module. Setus up Sequence for the vibration motor.
 */
 uint32_t motor_init(void);


/**@brief Function for the setting the PWM sequence and motor duty cycles.
 *
 * @details Sets the indicator state which sets the PWM sequences for the motor feedback.
 */
 void motor_indicate(motor_indicate_t indicate);


/**@brief Function for uninitializing the PWM module.
 *
 * @details Uninits the PWM module. To be used to get to low power modes
 */
 void motor_uninit(void);

/** @brief Function to return the motor driver status **/
bool is_motor_init(void);

 void motor_state_handler(motor_state_t state);

/**@brieef Function to check status of PWM.
 *
 * @details returns true if the PWM is stopped, false if it is running.
 */
 bool is_motor_stopped(void);

 static void motor_timer_handler(void *p_context);

/** @brief Function to set the sequency values for the vibration intensity from settings registers **/
void get_sequence_values(void);

/** @brief Function to init the motor timer **/
void motor_timer_init(void);

#endif