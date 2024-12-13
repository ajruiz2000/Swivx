/* File: motor.c */

/** C file for the SwivX vibration motor control **/


#include "motor.h"

/** Motor state timer instance **/
APP_TIMER_DEF(m_motor_timer);

/** PWM Drvier instance **/
static nrfx_pwm_t m_pwm0 = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_common_t seq_values2[5] = {0,0,0,0, 5000};
static uint32_t vib_duration = 0;
static bool is_motor_driver_init = false;
static motor_state_t motor_current_state = MOTOR_STOP;
static motor_state_t last_motor_state = MOTOR_STOP;
static uint8_t motor_state_counter = 0;
bool is_motor_on = false;
uint32_t motor_roll_delay = 0;


/**@brief Function for the Motor and PWM initialization.
 *
 * @details Initializes the PWM module. Setus up Sequence for the vibration motor.
 */
 uint32_t motor_init(void)
 {
    ret_code_t err_code;

    //Period calculation: (1/base_clock) * top_value
    nrfx_pwm_config_t const config0 = 
    {
        .output_pins = 
        {
            MOTOR_PWM,
            NRFX_PWM_PIN_NOT_USED,
            NRFX_PWM_PIN_NOT_USED,
            NRFX_PWM_PIN_NOT_USED
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock = NRF_PWM_CLK_125kHz,
        .count_mode = NRF_PWM_MODE_UP,
        .top_value = 6250,                                //Period of 50ms
        .load_mode = NRF_PWM_LOAD_COMMON,
        .step_mode = NRF_PWM_STEP_AUTO
    };

    err_code = nrfx_pwm_init(&m_pwm0, &config0, NULL);

    is_motor_driver_init = true;

    get_sequence_values();

    return err_code;
 }


/**@brief Function for the setting the PWM sequence and motor duty cycles.
 *
 * @details Sets the indicator state which sets the PWM sequences for the motor feedback.
 */
 void motor_indicate(motor_indicate_t indicate)
 {
    ret_code_t err_code;

    switch(indicate)
    {
        case MOTOR_ALERT_SHORT:
            NRF_LOG_INFO("Motor Alert Short!");
            static nrf_pwm_values_common_t seq_values[] = {0,0,0,0, 5000};
            nrf_pwm_sequence_t const seq = 
            {
                .values.p_common = seq_values,
                .length = NRF_PWM_VALUES_LENGTH(seq_values),
                .repeats = 0,
                .end_delay = 0
            };
            nrf_gpio_pin_set(MOTOR_EN);
            nrfx_pwm_simple_playback(&m_pwm0, &seq, 2, NRFX_PWM_FLAG_STOP);
            break;
      
        case MOTOR_ALERT_FEEDBACK:
            NRF_LOG_INFO("Motor Alert Long!");
            nrf_pwm_sequence_t const seq2 = 
            {
                .values.p_common = seq_values2,
                .length = NRF_PWM_VALUES_LENGTH(seq_values2),
                .repeats = 0,
                .end_delay = 0
            };
            nrf_gpio_pin_set(MOTOR_EN);
            nrfx_pwm_simple_playback(&m_pwm0, &seq2, 2, NRFX_PWM_FLAG_STOP);
            break;
        case MOTOR_POWER_ON:
            nrf_gpio_pin_set(MOTOR_EN);
            nrf_delay_ms(500);
            nrf_gpio_pin_clear(MOTOR_EN);
            break;
        case MOTOR_BUTTON_PRESS:
            nrf_gpio_pin_set(MOTOR_EN);
            nrf_delay_ms(200);
            nrf_gpio_pin_clear(MOTOR_EN);
            nrf_delay_ms(200);
            nrf_gpio_pin_set(MOTOR_EN);
            nrf_delay_ms(200);
            nrf_gpio_pin_clear(MOTOR_EN);
        default:
            break;
    }
 }

void motor_state_handler(motor_state_t state)
{
    ret_code_t err_code;
    motor_current_state = state;

    if(last_motor_state != state)
    {
        app_timer_stop(m_motor_timer);
    }
    last_motor_state = state;

    switch(state)
    {   
        case MOTOR_START:
            //indicate feedback
            motor_indicate(MOTOR_ALERT_FEEDBACK);
            motor_current_state = MOTOR_RUNNING;
            motor_state_counter++;
            is_motor_on = true;
            err_code = app_timer_start(m_motor_timer, APP_TIMER_TICKS(MOTOR_CHECK_DELAY), NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case MOTOR_RUNNING:
            if(is_motor_stopped())
            {
                motor_roll_delay = millis();
                nrf_gpio_pin_clear(MOTOR_EN);

                if(motor_state_counter >= 5)
                {
                    motor_current_state = MOTOR_STOP;
                }
                else
                {
                    motor_current_state = MOTOR_START;
                    err_code = app_timer_start(m_motor_timer, APP_TIMER_TICKS(MOTOR_INDICATE_DELAY), NULL);
                    APP_ERROR_CHECK(err_code);
                }
            }
            else
            {
                err_code = app_timer_start(m_motor_timer, APP_TIMER_TICKS(MOTOR_CHECK_DELAY), NULL);
                APP_ERROR_CHECK(err_code);
            }
            break;

        case MOTOR_STOP:
            is_motor_on = false;
            nrfx_pwm_stop(&m_pwm0, false);
            nrf_gpio_pin_clear(MOTOR_EN);
            motor_state_counter = 0;
            break;

        default:
            break;
    }
}

/* @brief Function for the motor timer handling */
static void motor_timer_handler(void *p_context)
{
    motor_state_handler(motor_current_state);
}

 /**@brief Function for uninitializing the PWM module.
 *
 * @details Uninits the PWM module. To be used to get to low power modes
 */
 void motor_uninit(void)
 {
    nrfx_pwm_uninit(&m_pwm0);
    is_motor_driver_init = false;
    is_motor_on - false;
 }

/** @brief Function to return the motor driver status **/
bool is_motor_init(void)
{
    return is_motor_driver_init;
}

 /**@brieef Function to check status of PWM.
 *
 * @details returns true if the PWM is stopped, false if it is running.
 */
 bool is_motor_stopped(void)
 {
    if(is_motor_driver_init)
    {
        return nrfx_pwm_is_stopped(&m_pwm0);
    }
    else
    {
        return true;
    }
 }

 /** @brief Function to dynamically create PWM sequence values based on the settings register */
void get_sequence_values(void)
{
    //PWM off value max is 5000, full on is 0. Calculate based on settings register vibration intensity percent

    //Check if intensity is above 100
    if(settings_register.motorIntensity > 100)
    {
        settings_register.motorIntensity = 100;
    }
   

    float intensity_percent = (settings_register.motorIntensity / 100.0);
    uint16_t vib_intensity = (5000 - (5000 * intensity_percent));
    for(uint8_t i=2; i<4; i+=1)
    {
        seq_values2[i] = vib_intensity;
    }

    //Calculate vibration pulse duration based on number of pulses and total motor duration from settings register
    //((Duration / num pulses) / 5ms period) / 2
    vib_duration = ((settings_register.motorDuration / settings_register.motor_pulses) / 5) / 2;
}

/** create motor Timer **/
void motor_timer_init(void)
{
    ret_code_t err_code;
    
    //motor state timer
    err_code = app_timer_create(&m_motor_timer, APP_TIMER_MODE_SINGLE_SHOT, motor_timer_handler);
    APP_ERROR_CHECK(err_code);
}