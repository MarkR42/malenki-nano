#include <stdint.h>

/* Routines to drive the brushless ESC.
 */


void brushless_init();
/*
 * Turn off brushless, called in failsafe.
 */
void brushless_off();

/*
 * Output a specific value 1000-2000
 */
void brushless_set(uint16_t pulse);

/* For sending dshot commands */
void brushless_send_command(uint16_t command, uint8_t telemetry);

