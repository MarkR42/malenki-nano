#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t fire_time; // timestamp when the trigger was pressed (centiseconds)
    bool fire_ready; // weapon ready to fire, not already fired (semi-auto mode)
} mixing_state_t;

extern mixing_state_t mixing_state;

void mixing_drive_motors(int16_t throttle, int16_t steering, int16_t weapon, bool invert);
void mixing_init(); // Called at startup.
void mixing_reset(); // Call this when signal lost.
