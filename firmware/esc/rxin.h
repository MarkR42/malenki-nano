
#include <stdbool.h>

void rxin_init();
void rxin_loop();

// Minimum number of channels required
#define RX_CHANNELS 6

// Ibus packet length is 2* channels + 4
#define IBUS_DATA_LEN_MIN ((RX_CHANNELS * 2) +4)
#define IBUS_DATA_LEN_MAX 32

typedef struct {
	uint16_t last_pulse_len;
	uint16_t packet_count;
    bool detected_serial_data; // Set this to true if we ever detect serial data.

    // Used for decoding PPM:
    int8_t next_channel; // The next channel we expect a pulse
    uint16_t pulse_lengths_next[RX_CHANNELS]; // Currently reading data
    uint16_t pulse_lengths[RX_CHANNELS]; // Good data
    bool pulses_valid; // Flag - are the pulses valid (usable)
    // pulses_valid false = pulses have been read already, new ones can be put in.
    // pulses_valid true = pulses are valid and ready to use.
    uint32_t last_valid_tickcount;
    uint16_t debug_count;

    // Used for decoding serial data.
    uint8_t ibus_len; // Length byte, if one was received
    uint8_t serial_previous_byte; // Just the previous byte
    uint8_t ibus_buf[IBUS_DATA_LEN_MAX];
    uint8_t ibus_index; // index of next byte to be received in ibus_buf

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

