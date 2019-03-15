
#include <stdbool.h>

void rxin_init();
void rxin_loop();

#define RX_CHANNELS 6

typedef struct {
	uint16_t last_pulse_len;
	uint16_t pulse_count;

    // Used for decoding PPM:
    uint16_t pulse_lengths[RX_CHANNELS];
    int8_t next_channel; // The next channel we expect a pulse
    bool pulses_valid; // Flag - are the pulses valid
    uint32_t last_valid_tickcount;
} rxin_state_t;

extern volatile rxin_state_t rxin_state;

