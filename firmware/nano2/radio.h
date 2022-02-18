
#include <stdbool.h>

#define NR_HOP_CHANNELS 16

// Packet length, in bytes.
#define RADIO_PACKET_LEN 37
// The amount of packet that we care about;
// Bind packets only need 27 (11 + 16) bytes,
// Sticks packets need 9 + (rc channels * 2) - so only 25 bytes for 8 channels
// Because most tx don't support more than 8 channels anyway, we don't need
// to worry about channels 9-14  
#define RADIO_PACKET_SIGNIFICANT_LEN 27
// Length of a telemetry we care about (more bytes are set to 0xff)
#define RADIO_TELEMETRY_SIGNIFICANT_LEN 22

typedef struct {
    // Saved binding info from the transmitter:
    // saved into nvrame:
    uint8_t tx_id[4]; // Transmitter ID
    uint8_t rx_id[4]; // Receiver (our id)
    uint8_t hop_channels[NR_HOP_CHANNELS]; // List of hopping channels.
    // Hopping channels saved from nvram, in case we decide to change hopping
    // channels going into bind mode, then want to restore them back.
    uint8_t hop_channels_saved[NR_HOP_CHANNELS];
    uint8_t hop_channels_new[NR_HOP_CHANNELS]; // From a NEW tx
    uint8_t tx_id_saved[4]; // Saved previous transmitter ID
    // Other info - not saved between boots
    uint8_t state;
    uint8_t hop_index;
    uint8_t old_hop_index;
    uint8_t current_channel; // channel we are listening, right now
    uint8_t packet_channel; // channel of last saved packet.
    uint8_t missed_packet_count; // successive
    uint16_t bind_packet_count; // successive bind packets from same tx.
    uint32_t bind_complete_time; // When we should complete binding
    uint8_t packet[RADIO_PACKET_LEN];
    bool packet_is_valid; // set to true if the packet is waiting to be processed, set false after.
    uint32_t last_sticks_packet; // time of last sticks packet
    uint32_t last_reinit_time; // When did we last reinit the radio?
    uint8_t got_signal_ever; // Set this to 1 after we got anything, to suppress autobind.
    bool led_on; // Set in the main loop, interrupt code enables it via a7105 and spi
    bool got_signal;
    uint8_t telemetry_packet[RADIO_PACKET_LEN];
    bool telemetry_packet_is_valid; // Ready to tx
    uint8_t telemetry_countdown; // Number of packets until next telem tx.
    uint8_t sticks_packet_count; // A general packet count that overflows all the time.
    bool tx_active; // flag so that interrupt knows tx is happening.
    // For telemetry error rate:
    uint8_t e_packet_count; // Count of packets including missed and error (up to 100)
    uint8_t e_good_count; // Count of good packets
    uint8_t e_error_rate; // Error rate per 100 packets
} radio_state_t;

extern radio_state_t radio_state;
// Bind mode
#define RADIO_STATE_BIND 0
// hopping along happily.
#define RADIO_STATE_HOPPING 2

void radio_init();

void radio_loop();

void radio_shutdown();
