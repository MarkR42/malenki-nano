
#include <stdint.h>

#include "nvconfig.h"
#include "diag.h"
#include <avr/io.h>
#include <avr/eeprom.h>

#include <string.h>

/*
 * Load / save state into non-volatile memory.
 */

nvconfig_state_t nvconfig_state;

EEMEM nvconfig_state_t state_in_eeprom;

static void * eeprom_addr = (void *) 0;

void nvconfig_init() {
    memset((void *) &nvconfig_state, 0, sizeof(nvconfig_state));
    nvconfig_state.magic = NVCONFIG_MAGIC_CORRECT;
}

static void diag_show_flags()
{
    diag_println("invert flags: L: %hhd R: %hhd W: %hhd",
            nvconfig_state.invert_left,
            nvconfig_state.invert_right,
            nvconfig_state.invert_weapon);
}

void nvconfig_load() {
    eeprom_read_block((void *) &nvconfig_state, eeprom_addr, sizeof(nvconfig_state)); 
    diag_println("Loaded nvconfig magic=%04x", nvconfig_state.magic); 
    diag_println("Loaded nvconfig boot_count=%ld", nvconfig_state.boot_count); 
    if (nvconfig_state.magic != NVCONFIG_MAGIC_CORRECT) {
        diag_println("nvconfig:wrong magic, reinitialising.");
        nvconfig_init();
    }
    nvconfig_state.boot_count += 1;
    diag_show_flags();
}

void nvconfig_save(){
    eeprom_update_block((void *) &nvconfig_state, eeprom_addr, sizeof(nvconfig_state)); 
    diag_show_flags();
}

