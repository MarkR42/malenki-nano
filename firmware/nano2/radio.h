
#define NR_HOP_CHANNELS 16


#define POSSIBLE_TX_COUNT 4

// This is the number of channels that we care about.
// The transmitter will send more, but we ignore those.
#define NUM_CONTROL_CHANNELS 6

// Packet length, in bytes.
#define RADIO_PACKET_LEN 37

typedef struct {
    // Saved binding info from the transmitter:
    // saved into nvrame:
    uint8_t tx_id[4]; // Transmitter ID
    uint8_t hop_channels[NR_HOP_CHANNELS]; // List of hopping channels.
    // Other info - not saved between boots
    uint8_t state;
    uint8_t hop_index;
    uint8_t current_channel; // channel we are listening, right now
    uint8_t packet_channel; // channel of last saved packet.
    uint8_t missed_packet_count; // successive
    uint16_t bind_packet_count; // successive bind packets from same tx.
    uint8_t packet[RADIO_PACKET_LEN];
    bool packet_is_valid; // set to true if the packet is waiting to be processed, set false after.
    uint32_t packet_counter;
    uint8_t got_signal_ever; // Set this to 1 after we got anything, to suppress autobind.
    uint16_t sticks[NUM_CONTROL_CHANNELS];
} radio_state_t;

extern radio_state_t radio_state;
// Bind mode
#define RADIO_STATE_BIND 0
// hopping along happily.
#define RADIO_STATE_HOPPING 2

void radio_init();

void radio_loop();
