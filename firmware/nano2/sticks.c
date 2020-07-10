#include "sticks.h"
#include "nvconfig.h"

#include <stdint.h>

#include "mixing.h"
#include "motors.h"
#include "weapons.h"
#include "state.h"
#include "diag.h"
#include "nvconfig.h"

// NB: implicitly initialised to zero.
static uint32_t last_signal_time;
static bool configuration_mode;
static bool has_signal;
static uint16_t throttle_min;
static uint16_t throttle_max;
static bool config_switch_last;
static uint32_t last_switch_time;
static uint8_t config_switch_count;

void sticks_init()
{
}

void sticks_loop()
{
    if (! has_signal) {
        if (! configuration_mode) {
            uint32_t now = get_tickcount();
            if ((now - last_signal_time) > 500)
            {
                diag_puts("Entering configuration mode");
                configuration_mode = true;
                throttle_min = 2000;
                throttle_max = 0;
                config_switch_last = false;
                config_switch_count = 0;
            }
        }
    }
}

static bool is_centred(uint16_t stick)
{
    return ((stick > 1400) && (stick < 1600));
}

static void handle_switches()
{
    diag_println("config_switch_count: %d", config_switch_count);
    bool need_save = false;
    switch (config_switch_count) {
        case 1: // Nothing, in case they pressed it by mistake.
            break; 
        case 2: // Invert left
            mixing_state.invert_left = ! mixing_state.invert_left;
            need_save = true;
            break;
        case 3: // Invert right
            mixing_state.invert_right = ! mixing_state.invert_right;
            need_save = true;
            break;
        case 4: // Invert weapon
            mixing_state.invert_weapon = ! mixing_state.invert_weapon;
            need_save = true;
            break;
        case 5: // Reset defaults.
            mixing_init();
            need_save = true;
            break;
        case 6: // Mixing on/off (on by default)
            mixing_state.enable_mixing = ! mixing_state.enable_mixing;
            need_save = true;
            break;
        case 7: // Factory reset / unbind
            nvconfig_reset();
            trigger_reset(); // reset the chip
            break;
        case 8: // Option to swap the weapon channels.
            mixing_state.swap_weapon_channels = ! mixing_state.swap_weapon_channels;
            need_save = true;
            break;
        case 9: // Option to disable braking.
            mixing_state.enable_braking = ! mixing_state.enable_braking;
            need_save = true;
            break;
    }
    if (need_save) {
        nvconfig_save();
    }
    config_switch_count = 0;
}

static void handle_configuration_mode(uint16_t *sticks)
{
    // Decide if we need to quit config mode
    uint16_t throttle = sticks[CHANNEL_INDEX_THROTTLE];
    uint16_t weapon  = sticks[CHANNEL_INDEX_WEAPON];
    if (throttle > throttle_max) throttle_max = throttle;
    if (throttle < throttle_min) throttle_min = throttle;
    // If the weapon is centred and the throttle has moved a bit,
    // then let's go.
    if (is_centred(weapon) && ((throttle_max - throttle_min) > 100)) {
        // Leaving config mode.
        diag_puts("Leaving configuration mode");
        configuration_mode = false;
        return;
    }
    bool config_switch = (sticks[CHANNEL_INDEX_CONFIG] > 1600);
    uint32_t now = get_tickcount();
    if (config_switch && ! config_switch_last) {
        // Switch pressed
        config_switch_count += 1;
        last_switch_time = now;
    }
    config_switch_last = config_switch;
    if (config_switch_count > 0) {
        // Delay with no switches?
        // Handle it.
        if ((now - last_switch_time) > 250) {
            handle_switches();
        }
    }
}

bool sticks_receive_positions(uint16_t *sticks)
{
    // Swap the weapon & weapon2 channels, if enabled.
    if (mixing_state.swap_weapon_channels) {
        int16_t temp;
        temp = sticks[CHANNEL_INDEX_WEAPON];
        sticks[CHANNEL_INDEX_WEAPON] = sticks[CHANNEL_INDEX_WEAPON2];
        sticks[CHANNEL_INDEX_WEAPON2] = temp;
    }
    // Returns the value of configuration_mode.
    if (configuration_mode)
    {
        handle_configuration_mode(sticks);
    } else {
        // Centre is *always* 1500.
        // Convert to signed.
        int16_t rel_steering = (int16_t) sticks[CHANNEL_INDEX_STEERING] - 1500;
        int16_t rel_throttle = (int16_t) sticks[CHANNEL_INDEX_THROTTLE] - 1500;
        int16_t rel_weapon = (int16_t) sticks[CHANNEL_INDEX_WEAPON] - 1500;
        // Inverted driving option
        bool invert = (bool) (sticks[CHANNEL_INDEX_INVERT] > 1600) ;
        mixing_drive_motors(rel_throttle, rel_steering, rel_weapon, invert);
        // Activate extra weapon channels
        weapons_set(sticks[CHANNEL_INDEX_WEAPON2], sticks[CHANNEL_INDEX_WEAPON3]);
    }
    has_signal = true;
    last_signal_time = get_tickcount();
    return configuration_mode;
}

// Called when there is no signal, signal is lost or
// something.
void sticks_no_signal()
{
    motors_all_off();
    weapons_all_off();
    has_signal = false;
}
