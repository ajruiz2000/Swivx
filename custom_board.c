/* File: custom_board.c */

/** C file for the SwivX Custom Board Functions **/


#include "custom_board.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "app_timer.h"
#include "motor.h"
#include "capsense.h"
#include "kxtj3.h"
#include "log.h"

APP_TIMER_DEF(m_led_timer);       //LED indicator timer
APP_TIMER_DEF(m_apploop_timer);   //app loop timer
APP_TIMER_DEF(m_clock_timer);     //Current Time Timer

static led_indicate_t led_ind_state = LED_INDICATE_IDLE;
static led_indicate_t last_led_state = LED_INDICATE_IDLE;
static led_indicate_t last_apploop_state = APPLOOP_TIMER_OFF;
static capsense_state_t cap_state = CSENSE_STATE_UNINIT;
static bool led_ind_on = false;
bool take_angle_reading = false;
bool is_awake_from_sleep = false;
bool touch_button_press = false;


void custom_board_init(void)
{
    ret_code_t err_code;

    //init accel
    //init pwm motor
    //init capsense
    //init adc

    gpio_init();
    timers_init();
    //log_fds_init();

    #if MOTOR_ENABLE == 1
        motor_init();
    #endif

    #if ACCL_ENABLE == 1
        kxtj3_init_t m_accel_init = {
            .i2c_scl_pin = SCL_PIN,
            .i2c_sda_pin = SDA_PIN,
            .reso_mode = RESO_LOW_8BIT,
            .range_mode = RANGE_4G
        };

        err_code = kxtj3_init(&m_accel_init);
        //APP_ERROR_CHECK(err_code);
    #endif

    #if CAPSENSE_ENABLE == 1
        //capsense_init();
    #endif

    battery_level_init();
}


static void gpio_init()
{

  //initialize pinout cofigurations
  nrf_gpio_cfg_output(LED_R);
  nrf_gpio_cfg_output(LED_G);
  nrf_gpio_cfg_output(LED_B);
  nrf_gpio_cfg_output(PSHOLD);
  nrf_gpio_cfg_output(MOTOR_EN);
  nrf_gpio_cfg_output(MOTOR_PWM);
  nrf_gpio_cfg_input(PBOUT, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(CAP_IN, NRF_GPIO_PIN_NOPULL);

  nrf_gpio_pin_set(PSHOLD);
  nrf_gpio_pin_set(LED_R);
  nrf_gpio_pin_set(LED_B);
  nrf_gpio_pin_set(LED_G);
  nrf_gpio_pin_clear(MOTOR_EN);
  nrf_gpio_cfg_output(MOTOR_PWM);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{

    // Initialize timer module.
    uint32_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    //Motor Timer
    motor_timer_init();

    // LED timer
    err_code = app_timer_create(&m_led_timer, APP_TIMER_MODE_SINGLE_SHOT, led_ind_timer_handler);
    APP_ERROR_CHECK(err_code); 

    //Capacitive sensor timer
    err_code = app_timer_create(&m_apploop_timer, APP_TIMER_MODE_REPEATED, apploop_timer_handler);
    APP_ERROR_CHECK(err_code);

    //Clock timer for EPOCH time
    err_code = app_timer_create(&m_clock_timer, APP_TIMER_MODE_REPEATED, clock_timer_handler);
    APP_ERROR_CHECK(err_code);

    //Start clock timer
    err_code = app_timer_start(m_clock_timer, APP_TIMER_TICKS(CLOCK_TIMEOUT), NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief       Configure leds to indicate required state.
 * @param[in]   indicate   State to be indicated.
 */
uint32_t swivx_led_indication(led_indicate_t indicate)
{
    uint32_t err_code   = NRF_SUCCESS;
    uint32_t next_delay = 0;
    led_ind_state = indicate;
    
    if(last_led_state != indicate)
    {
        leds_off();
        err_code       = app_timer_stop(m_led_timer);
    }
    last_led_state = indicate;

    switch (indicate)
    {

        case LED_INDICATE_ADVERTISING:
            // in advertising blink LED_0
            if (!led_ind_on)
            {
                nrf_gpio_pin_clear(LED_B);
                next_delay = LED_IND_ON_DELAY;
                led_ind_on = true;
            }
            else
            {
                nrf_gpio_pin_set(LED_B);
                next_delay = LED_IND_OFF_DELAY;
                led_ind_on = false;
            }

            err_code       = app_timer_start(m_led_timer, APP_TIMER_TICKS(next_delay), NULL);
            break;
            
        case LED_INDICATE_CONNECTED:
            if (!led_ind_on)
            {
                nrf_gpio_pin_clear(LED_G);
                next_delay = 1000;
                led_ind_on = true;
                err_code       = app_timer_start(m_led_timer, APP_TIMER_TICKS(next_delay), NULL);
            }
            else
            {
                nrf_gpio_pin_set(LED_G);
                led_ind_on = false;
            }
            break;

        case LED_INDICATE_LOW_BATTERY:
            if(!led_ind_on)
            {
                nrf_gpio_pin_clear(LED_R);
                next_delay = 500;
                led_ind_on = true;
                err_code       = app_timer_start(m_led_timer, APP_TIMER_TICKS(next_delay), NULL);
            }
            else
            {
                nrf_gpio_pin_set(LED_R);
                led_ind_on = false;
            }
            break;

        default:
            break;
    }

    return err_code;
}

static void leds_off(void)
{
    nrf_gpio_pin_set(LED_G);
    nrf_gpio_pin_set(LED_R);
    nrf_gpio_pin_set(LED_B);
}

static void checkCapsense_Button(void)
{
  //Read cap sense input
  if(nrf_gpio_pin_read(CAP_IN) && cap_state != CSENSE_STATE_PRESSED)
  {
    cap_state = CSENSE_STATE_PRESSED;
    last_touch_time = millis();
    //NRF_LOG_INFO("touch pressed");
  }
  else if(!nrf_gpio_pin_read(CAP_IN) && cap_state == CSENSE_STATE_PRESSED) 
  {
     touch_button_press = true;
     cap_state = CSENSE_STATE_RELEASED;
     last_touch_time = millis();
     NRF_LOG_INFO("touch tapped.");
  }
}
/******************** @Timers handlers ********************/

/** LED timer handler **/
static void led_ind_timer_handler(void *p_context)
{
    swivx_led_indication(led_ind_state);
}

/** Capsense timer handler **/
static void apploop_timer_handler(void *p_context)
{
        //capsense_sample();
        //checkCapsense_Button();
        take_angle_reading = true;
}

/** Clock Timer handler **/
static void clock_timer_handler(void * p_context)
{
    settings_register.timestamp++;
}

/* Starts the Capsense timer in different modes.
*
* @param[in]  mode - Low Power or Active, sets the
*             samping rate of 100ms or 500ms.
*/
void apploop_timer_start_mode(apploop_timer_t mode)
{
    ret_code_t err_code;

    if(last_apploop_state != mode)
    {
        err_code = app_timer_stop(m_apploop_timer);
        is_capsense_timer_running = false;
    }

    last_apploop_state = mode;

    if(mode == APPLOOP_TIMER_ACTIVE)
    {
        err_code = app_timer_start(m_apploop_timer, APP_TIMER_TICKS(APP_TIMEOUT_ACTIVE), NULL);
        APP_ERROR_CHECK(err_code);
        is_capsense_timer_running = true;
    }
    else if(mode == APPLOOP_TIMER_LP)
    {
        err_code = app_timer_start(m_apploop_timer, APP_TIMER_TICKS(APP_TIMEOUT_LP), NULL);
        APP_ERROR_CHECK(err_code);
        is_capsense_timer_running = true;
    }
    else if(mode == APPLOOP_TIMER_OFF)
    {
        err_code = app_timer_stop(m_apploop_timer);
        is_capsense_timer_running = false;
    }
}


/* millis functions */
uint32_t millis(void)
{
    return(app_timer_cnt_get() / 32.768);
}

uint32_t compare_millis(uint32_t previous_millis, uint32_t current_millis)
{
    if(current_millis < previous_millis) return(current_millis + OVERFLOW + 1 - previous_millis);
    return(current_millis - previous_millis);
}