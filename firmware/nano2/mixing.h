#include <stdint.h>
#include <stdbool.h>

#define WEAPON_MODE_DEFAULT 0
#define WEAPON_MODE_FLIPHARD 1

typedef struct {
    uint8_t weapon_mode;
    bool invert_left;
    bool invert_right;
    bool invert_weapon;
    bool enable_mixing;
    bool swap_weapon_channels; 
    bool enable_braking;
// Flag: enable "servo double" logic which expands the range of
// some servos by sending longer and shorter pulses.
    bool enable_servo_double;
// Flag: enable 100% steering mix. Useful for some robots / transmitters.
    bool enable_max_steering;
} mixing_state_t;

extern mixing_state_t mixing_state;

void mixing_init();
void mixing_drive_motors(int16_t throttle, int16_t steering, int16_t weapon, bool invert);

