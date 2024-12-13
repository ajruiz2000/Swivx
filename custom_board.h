/* Header for the custom board SwivX nRF52 version */

/** Created by: Trevor Clements
  * 9/10/2020
  * This is owned and only to be used by permission of
  * Star Technologies
**/

#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** Hardware peripherals enabled **/
#define MOTOR_ENABLE        1
#define CAPSENSE_ENABLE     0
#define ACCL_ENABLE         1

/** App modes **/
#define APP_MODE_LP         0
#define APP_MODE_ACTIVE     1
#define APP_MODE_REQ_LOG    2


/* Pinout definitions */
#define LED_R               14
#define LED_B               3
#define LED_G               16
#define CAP_SENSE1          0
#define CAP_IN              5           //SAADC input - AIN_3
#define BAT_SENSE           5           //SAADC input - AIN_5, P29
#define STAT                4
#define PSHOLD              6
#define PBOUT               8
#define MOTOR_PWM           9
#define MOTOR_EN            10
#define C1_CHRGE            11
#define SCL_PIN             22
#define SDA_PIN             24

/* Board Support Package */
#define BUTTONS_NUMBER      0
#define LEDS_NUMBER         0

#define LEDS_ACTIVE_STATE     0
#define LEDS_INV_MASK         LEDS_MASK
#define BUTTONS_ACTIVE_STATE  0

//LED timer delays
#define LED_IND_ON_DELAY      200
#define LED_IND_OFF_DELAY     800

//APP timer value
#define APP_TIMEOUT_ACTIVE      20
#define APP_TIMEOUT_LP          500
#define CLOCK_TIMEOUT           1000
#define BATTERY_LEVEL_TIMEOUT   10000
#define NOTOUCH_TIMEOUT         0
#define LOG_SAMPLE_TIMEOUT      100
#define POWER_OFF_TIMEOUT       3000
#define TOUCH_TIMEOUT_ACTIVE    5000
#define DOUBLETAP_TIMEOUT       500

//Millis overflow condition
#define OVERFLOW    ((uint32_t)(0xFFFFFFFF/32.768))

/** LED States **/
typedef enum 
{
  LED_INDICATE_IDLE,
  LED_INDICATE_ADVERTISING,
  LED_INDICATE_CONNECTED,
  LED_INDICATE_LOW_BATTERY
} led_indicate_t;

/** Capsense timer states **/
typedef enum
{
   APPLOOP_TIMER_LP,
   APPLOOP_TIMER_ACTIVE,
   APPLOOP_TIMER_OFF
} apploop_timer_t;

static bool is_capsense_timer_running;
extern bool take_angle_reading;
extern bool touch_button_press;


/**@brief Function for initializing the custom hardware.
 *
 * @return   NRF_SUCCESS on successful initialization, otherwise an error code.
 */
void custom_board_init(void);

uint32_t swivx_led_indication(led_indicate_t indicate);


/* Static Functions */

/* Init all hardware GPIO pins */
static void gpio_init(void);

/* Init peripheral timers */
static void timers_init(void);

/* LED timer handler for changing LED modes */
static void led_ind_timer_handler(void *p_context);

/* Capsense timer handler for sampling rates */
static void apploop_timer_handler(void *p_context);

/* EPOCH clock timer handler */
static void clock_timer_handler(void * p_context);


/* Starts the Capsense timer in different modes.
*
* @param[in]  mode - Low Power or Active, sets the
*             samping rate of 100ms or 500ms.
*/
void apploop_timer_start_mode(apploop_timer_t mode);

static void leds_off(void);


/* millis functions */
uint32_t millis(void);
uint32_t compare_millis(uint32_t previous_millis, uint32_t current_millis);


#endif