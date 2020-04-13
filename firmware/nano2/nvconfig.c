
#include <stdint.h>
#include "nvconfig.h"
#include "radio.h"
#include "diag.h"
#include "mixing.h"
#include <avr/io.h>
#include <avr/eeprom.h>

#include <string.h>

static void * eeprom_addr = (void *) 0;

#define NVCONFIG_MAGIC 0x442a

typedef struct {
    uint16_t magic; // If set to the wrong value, we know the nvconfig data are invalid.
    uint8_t tx_id[4];
    uint8_t hop_channels[NR_HOP_CHANNELS];
    // Put mixing_state directly into nvconfig, because we want to save it all
    // into non-volatile area.
    mixing_state_t mixing_state; 
} nvconfig_layout;

void nvconfig_load()
{
    nvconfig_layout nvdata;

    eeprom_read_block((void *) &nvdata, eeprom_addr, sizeof(nvdata));
    if (nvdata.magic == NVCONFIG_MAGIC) {
        // Great.
        // Copy the config data.
        memcpy(radio_state.tx_id, nvdata.tx_id, 4);
        memcpy(radio_state.hop_channels, nvdata.hop_channels, NR_HOP_CHANNELS);
        memcpy(&mixing_state, &nvdata.mixing_state, sizeof(mixing_state));
    } // otherwise, invalid nvconfig.
}

void nvconfig_save()
{
    nvconfig_layout nvdata;
    nvdata.magic = NVCONFIG_MAGIC;
    memcpy(nvdata.tx_id, radio_state.tx_id, 4);
    memcpy(nvdata.hop_channels, radio_state.hop_channels, NR_HOP_CHANNELS);
    memcpy(&nvdata.mixing_state, &mixing_state, sizeof(mixing_state));
    eeprom_update_block((void *) &nvdata, eeprom_addr, sizeof(nvdata));
}

static const uint8_t blank_data[2];

void nvconfig_reset()
{
    // Overwrite the magic number
    eeprom_update_block((void *) &blank_data, eeprom_addr, sizeof(blank_data));
}

