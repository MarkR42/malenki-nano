
#define NR_HOP_CHANNELS 16

typedef struct {
    // Saved binding info from the transmitter:
    uint8_t tx_id[4]; // Transmitter ID
    uint8_t hop_channels[NR_HOP_CHANNELS]; // List of hopping channels.
    // Other info
    uint8_t state;
    uint8_t hop_index;
    uint32_t last_packet_micros; // so we can detect late etc
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
