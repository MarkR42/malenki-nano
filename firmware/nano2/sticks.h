//

// This is the number of channels that we care about.
// The transmitter will send more, but we ignore those.
#define NUM_CONTROL_CHANNELS 8

#define CHANNEL_INDEX_STEERING 0
#define CHANNEL_INDEX_THROTTLE 1
#define CHANNEL_INDEX_WEAPON 2
// Invert mode
#define CHANNEL_INDEX_INVERT 4
// Calibrate switch - only used in config mode.
#define CHANNEL_INDEX_CONFIG 4
// Additional weapons- PWM signals sent in weapons.c
#define CHANNEL_INDEX_WEAPON2 3
#define CHANNEL_INDEX_WEAPON3 5

#include <stdint.h>
#include <stdbool.h>

void sticks_init();
void sticks_loop();

typedef struct {
    bool config_mode;
    bool led_state;
    uint8_t rpm_value; // For telemetry
} sticks_result_t;

sticks_result_t sticks_receive_positions(uint16_t *sticks); // at least NUM_CONTROL_CHANNELS
void sticks_no_signal();

