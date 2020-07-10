
#include "motors.h"
#include "diag.h"
#include <avr/io.h>
#include <avr/eeprom.h>

#define F_CPU 10000000
#include <util/delay.h>

/*
 * This flag is a special magic value in USERROW0 which must
 * be set externally by a updi command during firmware programming.
 * 
 * UPDI sets userrow using a fuse set command with offset 0x80
 * Example command to set USERROW0:
 * pyupdi.py -d tiny1617 -c /dev/ttyUSB0 -fs 128:0x2a
 * 
 * When set, this makes the board go into this self-test mode 
 * allowing easy test of motor driver chips.
 * 
 * The firmware will then (after a few seconds) clear the flag
 * so that without any more programming, on the next bootup
 * it will run in normal mode, the ESCs should not be shipped
 * to the users in this state, it will confuse them and be 
 * inconvenient as it immediately starts driving and wiggling
 * around without a transmitter.
 * 
 * If this value isn't flashed in USERROW, then nothing special
 * happens.
 */

// Magic value in USERROW[0]
// Factory value of userrow is
#define USERROW_MAGIC_VALUE 0x2a

static void do_motor_test();
static void clear_userrow_magic();
/*
 * This function never returns if it is going into a motor-test loop.
 */
void motor_test_init()
{
    uint8_t ur_value = USERROW.USERROW0;
    diag_println("motor_test: USERROW0=%02x", (int) ur_value);
    if (ur_value == USERROW_MAGIC_VALUE) {
        do_motor_test(); // Never returns.
    }
}

static void red_on()
{
    PORTA.DIRSET = 1 << 1;
}
static void red_off()
{
    PORTA.DIRCLR = 1 << 1;
}

static void do_motor_test()
{
    diag_println("*** SELF TEST *** - running motors and led test");
    // Set port output for blue led
    // PC5 = blue led
    uint8_t led_bm = 1 << 5; // led port bitmask
    PORTC.DIRSET = led_bm;
    uint8_t motor_id=0;
    uint16_t count = 0;
    while (1) { 
        // Flash leds and run motors
        diag_println("Running motor %d", (int) motor_id);
        _delay_ms(250);
        PORTC.OUTSET = led_bm;
        set_motor_direction_duty(motor_id, -100);
        _delay_ms(250);
        red_off();
        set_motor_direction_duty(motor_id, 0);
        _delay_ms(250);
        PORTC.OUTSET = led_bm;
        set_motor_direction_duty(motor_id, 100);
        _delay_ms(250);
        PORTC.OUTCLR = led_bm;
        motors_all_off();
        red_on();
        motor_id = (motor_id + 1) % 3;
        count += 1;
        if (count == 6) {
            clear_userrow_magic();
        }
    }    
}

static void clear_userrow_magic()
{
    // Called after a few seconds of testing,
    // So that the test-routine will not repeat next time.
    // This is the offset from the eeprom address and the 
    // userrow, which happens to be 0x100 bytes lower
    uint8_t * offset = (uint8_t *) 0xff00; 
    eeprom_write_byte(offset, 0x0);
    diag_println("Cleared userrow test flag");
}
