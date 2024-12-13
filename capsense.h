/* Header file capsense.h */

/** Header file for the capacitive sensing functionality **/



#ifndef CAPSENSE_H
#define CAPSENSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "custom_board.h"
#include "nrf_drv_csense.h"

#define AIN_3           CAP_SENSE1
#define AIN_5           BAT_SENSE
#define PAD_0_MASK      (1UL << AIN_3)
#define BAT_SENSE_MASK  (1UL << AIN_5)


typedef enum
{
    CSENSE_STATE_UNINIT,
    CSENSE_STATE_PRESSED,
    CSENSE_STATE_RELEASED,
} capsense_state_t;

extern bool touch_activated;
extern uint32_t last_touch_time;

/**@brief Function for the Capacitive sensing initialization.
 *
 * @details Initializes the Capsense driver.
 */
uint32_t capsense_init(void);

/**@brief Function for the Capacitive sensing uninitialization.
 *
 * @details Uninitializes the Capsense driver.
 */
void capsense_uninit(void);

/** @brief Function to return capsense driver init status **/
bool capsense_is_init(void);


/**@brief Function for the sampling the Capcitive sensor.
 *
 * @details samples the capacitive sensor.
 */
void capsense_sample(void);


/**@brief Function for the Capacitive sensing Handler.
 *
 * @param[in]   p_event_struct - csense driver event
 *
 * @details Handles the capsense events.
 */
static void capsense_handler(nrf_drv_csense_evt_t * p_event_struct);


#endif