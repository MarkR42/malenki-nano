
#define NR_HOP_CHANNELS 16

// Packet length, in bytes.

// Structure to hold possible transmitter ids, used 
// during binding.
typedef struct {
    uint8_t tx_id[4];
    uint16_t count; // Number of packets received.
} possible_tx;

#define POSSIBLE_TX_COUNT 4

// This is the number of channels that we care about.
// The transmitter will send more, but we ignore those.
#define NUM_CONTROL_CHANNELS 6

#define RADIO_PACKET_LEN 37
typedef struct {
    // Saved binding info from the transmitter:
    // saved into nvrame:
    uint8_t tx_id[4]; // Transmitter ID
    uint8_t hop_channels[NR_HOP_CHANNELS]; // List of hopping channels.
    // Other info - not saved between boots
    uint8_t state;
    uint8_t hop_index;
    uint32_t last_packet_micros; // so we can detect late etc
    uint16_t missed_packet_count; 
    uint32_t flash_time; // for blinking the lamp 
    uint8_t packet[RADIO_PACKET_LEN];
    uint32_t packet_counter;
    possible_tx possible_tx_list[POSSIBLE_TX_COUNT];
    uint8_t got_signal_ever; // Set this to 1 after we got anything, to suppress autobind.
    uint16_t sticks[NUM_CONTROL_CHANNELS];
} radio_state_t;

extern radio_state_t radio_state;
// Bind mode
#define RADIO_STATE_BIND 0
// Waiting for a signal
#define RADIO_STATE_WAITING 1
// hopping along happily.
#define RADIO_STATE_HOPPING 2

void radio_init();

void radio_loop();
