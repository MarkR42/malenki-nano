
#include <stdint.h>

void nvconfig_init();

void nvconfig_load();
void nvconfig_save();

typedef struct {
    uint32_t boot_count;
    uint16_t unused1;
} nvconfig_state_t;

extern nvconfig_state_t nvconfig_state;

