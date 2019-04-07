
#include <stdint.h>
#include <stdbool.h>

void nvconfig_init();

void nvconfig_load();
void nvconfig_save();

// If we change the fields, also change this, so that any
// newly reflashed units won't pick up junk config.
#define NVCONFIG_MAGIC_CORRECT 0x9176

typedef struct {
    uint32_t boot_count;
    uint16_t unused1;
    // Invert flags - to invert the output in case
    // The channels are the opposite to what we expect.
    bool invert_left;
    bool invert_right;
    bool invert_weapon;
    uint16_t magic;
} nvconfig_state_t;

extern nvconfig_state_t nvconfig_state;

