#include <avr/io.h>

#include "weapons.h"
#include "diag.h"

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

void weapons_init() 
{
    diag_puts("Initialising extra weapons!");
    /*
     * Use ports PORTC 0 and 1
     */
    PORTC.DIRSET |= 1 << 0;
    PORTC.DIRSET |= 1 << 1;
    
    /* Set up TCD */
    // one ramp mode
    // enable WOA
    // enable WOB
    // Enable A and B comparator
    // And enable WOC and WOD
    TCD0.FAULTCTRL = 
        TCD_CMPAEN_bm   
        | TCD_CMPBEN_bm;
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
}

void weapons_all_off()
{
    TCD0.CMPASET = max_count;
    TCD0.CMPBSET = max_count;    
}
