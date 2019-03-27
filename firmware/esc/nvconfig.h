
#include <stdint.h>

void nvconfig_init();

void nvconfig_load();
void nvconfig_save();

#define NVCONFIG_MAGIC_CORRECT 0x9175

typedef struct {
    uint32_t boot_count;
    uint16_t unused1;
    uint16_t magic;
} nvconfig_state_t;

extern nvconfig_state_t nvconfig_state;

