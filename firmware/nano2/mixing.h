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
} mixing_state_t;

extern mixing_state_t mixing_state;

void mixing_init();
void mixing_drive_motors(int16_t throttle, int16_t steering, int16_t weapon, bool invert);

