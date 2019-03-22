
#include <stdint.h>

#include "nvconfig.h"
#include <avr/io.h>

#include <string.h>

/*
 * Load / save state into non-volatile memory.
 */

nvconfig_state_t nvconfig_state;

void nvconfig_init() {
    memset((void *) &nvconfig_state, 0, sizeof(nvconfig_state));
}

void nvconfig_load() {

    // Can just copy nvconfig_state from USERROW.
}

void nvconfig_save(){
    // TODO: something with the nvm controller
}

