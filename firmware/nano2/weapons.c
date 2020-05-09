#include <avr/io.h>

#include "weapons.h"
#include "diag.h"

#define F_CPU 10000000 /* 10MHz / prescale=2 */
#include <util/delay.h>

#include <stdbool.h>

// Maximum count of the timer-counter 
static const uint16_t max_count = 3125;

static void weapons_ccl_init()
{
    // Set up CCL so that TCD.WOC is mirrored on    CCL LUT0OUT
    // Set alternate output pins for ccl
    // PB4 pin12 
    PORTMUX.CTRLA |= PORTMUX_LUT0_ALTERNATE_gc;
    // Set those pins as out
    PORTB.DIRSET |= 1 << 4;
    // Set up LUT0
    // Truth:
    // Wire as a "OR" so we or all the inputs together and NOT
    // So we set all bits to 1, except bit0
    CCL.TRUTH0 = 0xfe;
    CCL.LUT0CTRLB = 0x9; // INSEL0 = 0x9 = TCD WOA bit
    // ALl other inputs will default to "mask" which I think is always 0
    CCL.LUT0CTRLA =  CCL_OUTEN_bm | CCL_ENABLE_bm; //enable out+lut
    CCL.CTRLA = CCL_ENABLE_bm; // Enable CCL
}

static void weapon3_init()
{
    /*
     * Determine if weapon3 pin is electrically shorted to weapon2.
     * If it is shorted, then leave the pin set as input.
     * 
     * If it is not shorted, set it as an output.
     * 
     * This allows the builder to solder a single wire across both
     * pads, which is much easier than just connecting weapon2.
     * 
     * e.g. for only one pwm weapon.
     */
    diag_println("checking weapon 3");
    // Set both to input.
    uint8_t weapon3_bm = 1 << 1;
    uint8_t weapon2_bm = 1 << 0;
    uint8_t both = weapon2_bm | weapon3_bm;
    // Set both low, and wait long enough for any residual charge
    // to drain away.
    PORTC.OUTCLR = both;
    PORTC.DIRSET = both;
    _delay_us(1000);
    // Now set weapon3 as input,see if it can detect a high
    // on weapon2.
    PORTC.DIRCLR = weapon3_bm;
    PORTC.OUTSET = weapon2_bm;
    _delay_us(50);
    uint8_t in0 = PORTC.IN;
    PORTC.OUTCLR = weapon2_bm;
    _delay_us(50);
    uint8_t in1 = PORTC.IN;

    diag_println("in0=%02x in1=%02x", in0, in1);
    bool v0 = in0 & weapon3_bm;
    bool v1 = in1 & weapon3_bm;
    // If we saw the other pin pulse too, then they are
    // presumably shorted.
    if (v1 != v0) {
        diag_println("Weapon3 appears shorted to weapon2");
        // Do not activate. It can stay as an input.
    } else {
        diag_println("Weapon3 ok.");
        // Activate it.
        PORTC.DIRSET = weapon3_bm;
    }
}

void weapons_init() 
{
    diag_println("Initialising extra weapons!");
    /*
     * Use ports PORTC 0 and 1
     */


    // Initially disable tcd outputs.
    _PROTECTED_WRITE(TCD0.FAULTCTRL, 0);
    // And disable TCD
    TCD0.CTRLA = 0;

    weapon3_init();
    PORTC.DIRSET = 1 << 0;
    
    /* Set up TCD */
    // one ramp mode (default)
    
    
    // " The two additional outputs WOC and WOD can be configured by 
    // software to be connected to either WOA or WOB by writing the
    //  Compare C/D Output Select bits (CMPCSEL and CMPDSEL) in
    //  the Control C register (TCDn.CTRLC)" (datasheet)
    TCD0.CTRLC |= TCD_CMPDSEL_bm;
    
    // enable WOA
    // enable WOB
    // Enable A and B comparator
    // And enable WOC and WOD
    _PROTECTED_WRITE(TCD0.FAULTCTRL, 
        TCD_CMPCEN_bm | TCD_CMPDEN_bm);
    // Divider will be 64, 
    // clock rate is 10mhz / 64 = 
    // clock len = 6.4 us
    // We want to make servo pulses every 20ms,
    // So we need to count to ~ 3125
    TCD0.CMPACLR = max_count; // reset time
    TCD0.CMPBCLR = max_count; // reset time
    // To generate pulses, we reduce CMPASET to lower values, then
    // it creates a positive pulse 
    TCD0.CMPASET = max_count;
    TCD0.CMPBSET = max_count;
    
    // Finally turn the thing on.
    TCD0.CTRLA = TCD_ENABLE_bm |
        TCD_CLKSEL_SYSCLK_gc | // sys clock
        TCD_SYNCPRES_DIV2_gc | // prescale by 2
        TCD_CNTPRES_DIV32_gc; // further prescale by 32
    weapons_ccl_init();
}

static uint16_t scale_pulse(uint16_t pulse)
{
    // Scale pulse into TCD counts.
    // pulse should be 1000-2000 (us)
    // essentially divide by 6.4
    pulse = pulse * 10;
    pulse = pulse / 64;
    return pulse;
}

void weapons_set(uint16_t pulse2, uint16_t pulse3)
{
    uint16_t p2 = scale_pulse(pulse2);
    uint16_t p3 = scale_pulse(pulse3);
    TCD0.CMPASET = max_count - p2;
    TCD0.CMPBSET = max_count - p3;
    // Sync domain
    TCD0.CTRLE = TCD_SYNCEOC_bm;
    
}

void weapons_all_off()
{
    TCD0.CMPASET = max_count;
    TCD0.CMPBSET = max_count;    
    // Sync domain
    TCD0.CTRLE = TCD_SYNCEOC_bm;
}
