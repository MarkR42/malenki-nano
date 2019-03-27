#include <stdbool.h>
#include <stdint.h>

typedef struct {
    // Modify these when the transmitter status changes or error
    bool blue_on; // Enable blue LED (D2) during off-time
    uint8_t flash_count; // Number of flashes to do during on-time
} blinky_state_t;

extern blinky_state_t blinky_state;
/*
 * Blinky will turn on the red LED with a particular number of flashes
 * every period (e.g. 2.5 seconds)
 *
 * Number of flashes is the error code.
 */

void blinky_init();
void blinky_loop();
