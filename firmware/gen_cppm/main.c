#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 3333333 /* 20MHz / 6(default prescale) */
#include <util/delay.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/*
 * Generate a test CPPM signal:
 *
 * - should repeat approximately every 20 ms
 * - N high pulses of 1000-2000us, followed by
 * a single very long pulse of >3000us, which
 * typically stays high until just before the next
 * set starts.
 */

static void init()
{
    // Use these pins:
    // PA3 and PA4 - these are the two closes to the power pins.
    PORTA.DIRSET = 1 << 3;
    PORTA.DIRSET = 1 << 4;
}

static void init_timer_interrupts()
{
        // This uses TCB1 for timer interrupts.
        const unsigned short compare_value = 33333; // 100hz
        TCB1.CCMP = compare_value;
        TCB1.INTCTRL = TCB_CAPT_bm;
        // CTRLB bits 0-2 are the mode, which by default
        // 000 is "periodic interrupt" -which is correct
        TCB1.CTRLB = 0;
        // Divide CLK_PER by 2 to give 50hz
        TCB1.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKDIV2_gc;
}

volatile bool got_interrupt=0;

ISR(TCB1_INT_vect)
{
    got_interrupt=1;
    TCB1.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

static void wait_for_interrupt()
{
    while (! got_interrupt) {
        // nothing
    }
    got_interrupt=0;
}

static void set_out(uint8_t level)
{
    uint8_t bm = ( 1 << 3) | (1 << 4);
    if (level) {
        PORTA.OUTSET = bm;
    } else {
        PORTA.OUTCLR = bm;
    }
}

int main(void)
{
    init();
    init_timer_interrupts();
    sei(); // interrupts on
    static uint16_t widths[] = {
        1800, 1500, 1000, 1000, 1000, 1000, 0
    };
    while(1) {
        for (int i=0; widths[i]; i++) {
            uint16_t w = widths[i];
            set_out(0);
            _delay_us(100); 
            set_out(1);
            for (uint16_t j=0; j < w; j+= 50) {
                _delay_us(50); 
            }
        }
        // Spend the rest of the 20ms waiting with output high
        wait_for_interrupt();
    }	
}
