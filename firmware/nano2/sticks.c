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
static uint8_t config_rpm_value; // For display
static uint8_t config_flash_count; // Counter - number of flashes
static uint32_t config_flash_start_time;

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
                config_rpm_value = 0;
            }
        }
    }
}

static bool is_centred(uint16_t stick)
{
    return ((stick > 1400) && (stick < 1600));
}

static void show_rpm_value(uint8_t count, uint8_t value)
{
    config_rpm_value = (count * 10) + value;
}

static void handle_switches(uint32_t now)
{
    diag_println("config_switch_count: %d", config_switch_count);
    bool need_save = false;
    switch (config_switch_count) {
        case 1: // Nothing, in case they pressed it by mistake.
            break; 
        case 2: // Invert left
            mixing_state.invert_left = ! mixing_state.invert_left;
            show_rpm_value(config_switch_count, mixing_state.invert_left);
            need_save = true;
            break;
        case 3: // Invert right
            mixing_state.invert_right = ! mixing_state.invert_right;
            show_rpm_value(config_switch_count, mixing_state.invert_right);
            need_save = true;
            break;
        case 4: // Invert weapon
            mixing_state.invert_weapon = ! mixing_state.invert_weapon;
            show_rpm_value(config_switch_count, mixing_state.invert_weapon);
            need_save = true;
            break;
        case 5: // Reset defaults.
            mixing_init();
            need_save = true;
            show_rpm_value(config_switch_count, 0);
            break;
        case 6: // Mixing on/off (on by default)
            mixing_state.enable_mixing = ! mixing_state.enable_mixing;
            need_save = true;
            show_rpm_value(config_switch_count, mixing_state.enable_mixing);
            break;
        case 7: // Factory reset / unbind
            nvconfig_reset();
            trigger_reset(); // reset the chip
            break;
        case 8: // Option to swap the weapon channels.
            mixing_state.swap_weapon_channels = ! mixing_state.swap_weapon_channels;
            need_save = true;
            show_rpm_value(config_switch_count, mixing_state.swap_weapon_channels);
            break;
        case 9: // Option to disable braking.
            mixing_state.enable_braking = ! mixing_state.enable_braking;
            need_save = true;
            show_rpm_value(config_switch_count, mixing_state.enable_braking);
            break;
        case 10: // Servo doubling.
            mixing_state.enable_servo_double = ! mixing_state.enable_servo_double;
            need_save = true;
            show_rpm_value(config_switch_count, mixing_state.enable_servo_double);
            break;
        case 11: // Max steering (100% mix)
            mixing_state.enable_max_steering = ! mixing_state.enable_max_steering;
            need_save = true;
            show_rpm_value(config_switch_count, mixing_state.enable_max_steering);
            break;
    }
    if (need_save) {
        nvconfig_save();
        // Flash the led N times
        config_flash_count = config_switch_count;
        config_flash_start_time = now;
    }
    config_switch_count = 0;
}

static bool handle_configuration_mode(uint16_t *sticks)
{
    // Return led state.
    
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
        return 1;
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
        if ((now - last_switch_time) > 200) {
            handle_switches(now);
        }
    }
    
    // Flash the LED after a successful configuration command
    // So that the operator knows.
    if (config_flash_count != 0) {
        uint32_t t = now - config_flash_start_time;
        // Create config_flash_count flashes:
        uint8_t ticks = (t / 25);
        if (ticks > (config_flash_count*2)) {
            config_flash_count = 0; // Stop flashing.
        }
        return ! (ticks & 1); 
    } else {
        // No recent config command:
        // Configuration mode LED status:
        // Create a high-duty-cycle blinking
        uint8_t blinky_counter = (now & 0x1f);
        return (blinky_counter >= 0x2);
    }
}

sticks_result_t sticks_receive_positions(uint16_t *sticks)
{
    // Swap the weapon & weapon2 channels, if enabled.
    if (mixing_state.swap_weapon_channels) {
        int16_t temp;
        temp = sticks[CHANNEL_INDEX_WEAPON];
        sticks[CHANNEL_INDEX_WEAPON] = sticks[CHANNEL_INDEX_WEAPON2];
        sticks[CHANNEL_INDEX_WEAPON2] = temp;
    }
    // Returns this structure;
    sticks_result_t result;
    result.led_state = 1;
    result.rpm_value = 0;
    result.config_mode = configuration_mode;

    if (configuration_mode)
    {
        result.led_state = handle_configuration_mode(sticks);
        result.rpm_value = config_rpm_value;
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
    return result;
}

// Called when there is no signal, signal is lost or
// something.
void sticks_no_signal()
{
    motors_all_off();
    weapons_all_off();
    has_signal = false;
}
