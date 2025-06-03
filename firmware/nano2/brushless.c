#include "brushless.h"
#include "dshot.h"

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
static uint8_t signal_count; 

static void pin_set_output()
{
    // Enable output from the pin
    BRUSHLESS_PORT.DIRSET = BRUSHLESS_PIN_bm;
}

void brushless_init()
{
    /*
    _delay_ms(40);
    brushless_send_command(1); // Beep
    _delay_ms(5);
    */
    brushless_off();
}

void brushless_off()
{
    // Set port as input.
    BRUSHLESS_PORT.DIRCLR = BRUSHLESS_PIN_bm;
    signal_count = 0;
}

static uint16_t calc_dshot_crc(uint16_t top_12_bits);

/*
 * Output a specific value 1000-2000
 */

void send_startup_command()
{
    // Called for the first few packets after startup 
    // (or when signal is regained after loss)
    // Decide what to send, depending on signal_count
    if ((signal_count > 4) && (signal_count % 1) ) {
        // Delay a few packets,
        // then send zeros every other packet.
        brushless_send_command(0);
    }
}

/* Note on startup:
 * 
 * For the first N requests, we ignore the pulse parameter,
 * and instead send a series of initialisation commands
 */
void brushless_set(uint16_t pulse)
{
    pin_set_output();
    
    if (signal_count < 20) {
        send_startup_command();
        signal_count += 1;
        return; // Ignore pulse.
    }
    
    // pulse is in range 1000-2000
    // DSHOT is in range 0-1023 
    // values 1..47 are reserved for commands, so we never send those.
    // special value 0 means off.
    
    uint16_t dshot_speed;
    if (pulse < 1050) {
        // Special magical value "0"
        dshot_speed = 0;
    } else {
        dshot_speed = pulse - 1047;
        // Clamp max
        if (dshot_speed > 1023) dshot_speed = 1023;
    }
    // dshot_speed now in range 48..1023 or 0
    // Make the top 12 bits, which includes the ESC telemetry bit (always 0)
    uint16_t top_12_bits = (dshot_speed << 1);
    uint16_t dshot_word = calc_dshot_crc(top_12_bits);
    // Now send the dshot command with interrupts disabled
    cli(); // Interrupts off
    dshot_send_word(dshot_word);
    sei(); // Interrupts on.
}

void brushless_send_command(uint16_t command)
{
    uint16_t top_12_bits = (command << 1);
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
