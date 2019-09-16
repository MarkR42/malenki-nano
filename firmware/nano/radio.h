
#define NR_HOP_CHANNELS 16

// Packet length, in bytes.

#define RADIO_PACKET_LEN 37
typedef struct {
    // Saved binding info from the transmitter:
    uint8_t tx_id[4]; // Transmitter ID
    uint8_t hop_channels[NR_HOP_CHANNELS]; // List of hopping channels.
    // Other info
    uint8_t state;
    uint8_t hop_index;
    uint32_t last_packet_micros; // so we can detect late etc
    uint32_t flash_time; // for blinking the lamp 
    uint8_t packet[RADIO_PACKET_LEN];
    uint32_t packet_counter;
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
