
#include <stdbool.h>

#define NR_HOP_CHANNELS 16


// This is the number of channels that we care about.
// The transmitter will send more, but we ignore those.
#define NUM_CONTROL_CHANNELS 8

#define CHANNEL_INDEX_STEERING 0
#define CHANNEL_INDEX_WEAPON 2
#define CHANNEL_INDEX_THROTTLE 1
#define CHANNEL_INDEX_INVERT 4
#define CHANNEL_INDEX_CALIBRATE 5
// Additional weapons.
#define CHANNEL_INDEX_WEAPON2 6
#define CHANNEL_INDEX_WEAPON3 7

// Packet length, in bytes.
#define RADIO_PACKET_LEN 37
// The amount of packet that we care about;
// Bind packets only need 27 (11 + 16) bytes,
// Sticks packets need 9 + (rc channels * 2) - so only 25 bytes for 8 channels
// Because most tx don't support more than 8 channels anyway, we don't need
// to worry about channels 9-14  
#define RADIO_PACKET_SIGNIFICANT_LEN 27

typedef struct {
    // Saved binding info from the transmitter:
    // saved into nvrame:
    uint8_t tx_id[4]; // Transmitter ID
    uint8_t hop_channels[NR_HOP_CHANNELS]; // List of hopping channels.
    // Hopping channels saved from nvram, in case we decide to change hopping
    // channels going into bind mode, then want to restore them back.
    uint8_t hop_channels_saved[NR_HOP_CHANNELS];
    uint8_t tx_id_saved[4]; // Saved previous transmitter ID
    // Other info - not saved between boots
    uint8_t state;
    uint8_t hop_index;
    uint8_t old_hop_index;
    uint8_t current_channel; // channel we are listening, right now
    uint8_t packet_channel; // channel of last saved packet.
    uint8_t missed_packet_count; // successive
    uint16_t bind_packet_count; // successive bind packets from same tx.
    uint8_t packet[RADIO_PACKET_LEN];
    bool packet_is_valid; // set to true if the packet is waiting to be processed, set false after.
    uint32_t last_sticks_packet; // time of last sticks packet
    uint8_t got_signal_ever; // Set this to 1 after we got anything, to suppress autobind.
    bool led_on; // Set in the main loop, interrupt code enables it via a7105 and spi 
} radio_state_t;

extern radio_state_t radio_state;
// Bind mode
#define RADIO_STATE_BIND 0
// hopping along happily.
#define RADIO_STATE_HOPPING 2

void radio_init();

void radio_rxtest_loop();
