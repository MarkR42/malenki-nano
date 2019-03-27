
#include <stdbool.h>

void rxin_init();
void rxin_loop();

// Minimum number of channels required
#define RX_CHANNELS 6

typedef struct {
	uint16_t last_pulse_len;
	uint16_t packet_count;

    // Used for decoding PPM:
    int8_t next_channel; // The next channel we expect a pulse
    uint16_t pulse_lengths_next[RX_CHANNELS]; // Currently reading data
    uint16_t pulse_lengths[RX_CHANNELS]; // Good data
    bool pulses_valid; // Flag - are the pulses valid
    uint32_t last_valid_tickcount;
    uint16_t debug_count;

    // State data 
    bool got_signal; // If we have a signal NOW
    uint32_t last_signal_time; // tickcount
    uint32_t last_nosignal_time; // tickcount when we last had no signal.
    uint8_t running_mode;

    // Calibration data...
    // Set when we first receive data
    uint16_t throttle_max_position;
    uint16_t throttle_min_position;
    uint16_t throttle_centre_position;;
    uint16_t steering_centre_position; 
    uint16_t weapon_centre_position; 
} rxin_state_t;


// Running modes:
// No signal
#define RUNNING_MODE_NOSIGNAL 0
// Got a signal, but not yet calibrated stick positions
// (do not start yet)
#define RUNNING_MODE_CALIBRATION 1
// Ready to drive
#define RUNNING_MODE_READY 2

// Channels (zero-based)
#define CHANNEL_INDEX_STEERING 0
#define CHANNEL_INDEX_WEAPON 1
#define CHANNEL_INDEX_THROTTLE 2

extern volatile rxin_state_t rxin_state;

