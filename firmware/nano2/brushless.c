#include "brushless.h"
#include "dshot.h"
#include "state.h"

#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 10000000 /* 10MHz / prescale=2 */
#include <util/delay.h>

#define BRUSHLESS_PORT PORTA
#define BRUSHLESS_PIN (3)
#define BRUSHLESS_PIN_bm (1 << BRUSHLESS_PIN)

/*
 * When not active (before signal, or after signal loss) we
 * want to keep the pin High-Z, so that the user can
 * connect their own programmer to the signal pin to configure the
 * ESC, if they want to do that.
 */

// Count the number of packets.
static uint32_t signal_start_time; // Time of starting
static bool signal_active; 

static void pin_set_output()
{
    // Enable output from the pin
    BRUSHLESS_PORT.DIRSET = BRUSHLESS_PIN_bm;
}

void brushless_init()
{
    brushless_off();
}

void brushless_off()
{
    // Set port as input.
    BRUSHLESS_PORT.DIRCLR = BRUSHLESS_PIN_bm;
    signal_active = 0;
    signal_start_time = 0;
}

static uint16_t calc_dshot_crc(uint16_t top_12_bits);


// We need to send these command 6x to take effect.
// We do not need to send the "save" command, we will send
// every time.
#define DSHOT_CMD_3D_MODE_OFF 9
#define DSHOT_CMD_3D_MODE_ON 10

// NB: tick = 1/100s
// Number of ticks to wait before sending anything
#define STARTUP_DELAY 80
// Number of ticks to wait for arming, after we start sending 0
// commands.
#define ARM_DELAY 120
// Amount of time we send commands,
// (enable 3d mode)
#define COMMAND_DELAY 20

/*
 * NB: AM32 requires 30x "zero" inputs to arm the ESC.
 * 
 * It will only take DSHOT mode commands when armed.
 * Dshot mode commands must be repeated 6x
 */
 
static uint16_t pulse_to_dshot_command(uint16_t pulse)
{
    /* Convert a pulse, in the range 1000-2000, to a dshot command
     * dshot commands are 11-bit values (0-2047)
     * 
     * See https://www.swallenhardware.io/battlebots/2019/4/20/a-developers-guide-to-dshot-escs
     * 
     * Reverse:  48 is the slowest, 1047 is the fastest
     * Forwards: 1049 is the slowest, 2047 is the fastest
     * 1048 does NOT stop the motor! Use command 0 for that.
     */
    const uint16_t deadzone = 25; 
    uint16_t dshot = 0;
    if (pulse < (1500 - deadzone)) {
        // Reverse
        dshot = (1500 - pulse) * 2;
    }
    if (pulse > (1500 + deadzone)) {
        dshot = ((pulse - 1500) * 2) + 1000;
    }
    if ((dshot != 0) && (dshot < 48)) dshot = 48;
    if (dshot > 2047) dshot = 2047;
    return dshot;
}

/* Note on startup:
 * 
 * This is done the first time we enter drive mode, and subsequently
 * if we lose the signal and regain the signal (e.g. tx is off and on)
 * 
 * 1. For STARTUP_DELAY, we send nothing to ensure that the bootloader
 *   loads the AM32 firmware (holding signal high is bad).
 * 
 *   Then await the AM32 bootup and startup tune (600ms), during
 *      this time, commands are ignored.
 * 
 * 2. We start sending zero commands (30x required to arm),
 *   this will (eventually) arm the firmware (armed = 1) , BUT
 *  the firmware then calls playInputTune() and waits a further 100ms
 *  - this takes > 400ms total.
 *  While calling playInputTune and calling delayMillis(), AM32 will
 *      not count input commands
 * 
 * 3. After this we send the 3d command for a while, COMMAND_DELAY
 *  to make sure the AM32 firmware gets the message.
 */
/*
 * Output a specific value 1000-2000
 */
void brushless_set(uint16_t pulse)
{
    pin_set_output();
    
    if (! signal_active) {
        // Initially wait for STARTUP_DELAY.
        uint32_t now = get_tickcount();
        // Check the amount of time since signal started,
        if (signal_start_time == 0) {
            signal_start_time = now;
        }
        uint32_t age = (now - signal_start_time);
        
        if (age < STARTUP_DELAY) {
            // Send nothing at all, we are waiting for bootloader,
            // sending data interferes with the bootloader
            // signal must remain low
            return;
        }
        if (age < (STARTUP_DELAY + ARM_DELAY)) {
            brushless_send_command(0,0); // Arm.
            return;
        }
        if (age < (STARTUP_DELAY + ARM_DELAY + COMMAND_DELAY)) {
            // Dshot command must have telemetry bit.
            brushless_send_command(DSHOT_CMD_3D_MODE_ON, 1);
            return;
        } else {
            // Ready to go!
            signal_active = 1;
        }
    }
    // After delay and startup commands finished, send normal pulses:
    
    uint16_t dshot_speed = pulse_to_dshot_command(pulse);

    uint16_t top_12_bits = (dshot_speed << 1);
    uint16_t dshot_word = calc_dshot_crc(top_12_bits);
    // Now send the dshot command with interrupts disabled
    cli(); // Interrupts off
    dshot_send_word(dshot_word);
    sei(); // Interrupts on.
}

void brushless_send_command(uint16_t command, uint8_t telemetry)
{
    uint16_t top_12_bits = (command << 1);
    if (telemetry) { 
        top_12_bits |= 1; // Set least significant bit.
    }
    uint16_t dshot_word = calc_dshot_crc(top_12_bits);
    pin_set_output();
    cli(); // Interrupts off
    dshot_send_word(dshot_word);
    sei(); // Interrupts on.    
}

/*
 * Taken from https://github.com/gueei/DShot-Arduino/blob/master/src/DShot.cpp
 */
// top_12_bits should contain the 11 bits of throttle,
// and 1 bit of telemetry request (least significant)
// Throttle values 1..47 are reserved for commands,
// 0=stop, 1024 = half, 2047 = max
// This function shifts top_12_bits left 4 bits and fills in the crc
// in the least significant 4 bits.
static uint16_t calc_dshot_crc(uint16_t top_12_bits){
  uint8_t csum = 0;
  uint16_t csum_data = top_12_bits;
  for (uint8_t i=0; i<3; i++){
    csum ^= csum_data;
    csum_data >>= 4;
  }
  csum &= 0xf;
  return (top_12_bits<<4)|csum;
}
