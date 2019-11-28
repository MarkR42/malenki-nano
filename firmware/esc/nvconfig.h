
#include <stdint.h>
#include <stdbool.h>

void nvconfig_init();

void nvconfig_load();
void nvconfig_save();

// If we change the fields, also change this, so that any
// newly reflashed units won't pick up junk config.
#define NVCONFIG_MAGIC_CORRECT 0x9176

// Normal weapon mode - directly proportional to controller position
// with reasonable size dead zone:
#define WEAPON_MODE_NORMAL 0
// "Flip hard" weapon mode - 
// Flip with full force when controller > 25%
// Do nothing between 0-25%
// retract normally.
#define WEAPON_MODE_FLIPHARD 1
// Axe weapon-mode.
// Retract normally.
// When >25%, enter a loop automatically firing and retracting the axe
#define WEAPON_MODE_AXE1 2


#define WEAPON_MODE_MAX 2

typedef struct {
    uint32_t boot_count;
    uint16_t unused1;
    // Invert flags - to invert the output in case
    // The channels are the opposite to what we expect.
    bool invert_left;
    bool invert_right;
    bool invert_weapon;
    uint8_t weapon_mode;
    uint16_t magic;
} nvconfig_state_t;

extern nvconfig_state_t nvconfig_state;

