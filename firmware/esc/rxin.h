
#include <stdbool.h>

void rxin_init();
void rxin_loop();

// Minimum number of channels required
#define RX_CHANNELS 6

typedef struct {
	uint16_t last_pulse_len;
	uint16_t pulse_count;

    // Used for decoding PPM:
    int8_t next_channel; // The next channel we expect a pulse
    uint16_t pulse_lengths_next[RX_CHANNELS]; // Currently reading data
    uint16_t pulse_lengths[RX_CHANNELS]; // Good data
    bool pulses_valid; // Flag - are the pulses valid
    uint32_t last_valid_tickcount;
    uint16_t debug_count;
} rxin_state_t;

extern volatile rxin_state_t rxin_state;

