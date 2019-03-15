#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 3333333 /* 20MHz / 6(default prescale) */
#include <util/delay.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "diag.h"
#include "rxin.h"
#include "motors.h"
#include "vsense.h"
#include "state.h"

volatile master_state_t master_state;

void init_hardware()
{
	init_motors();
	init_rxin();
	init_vsense();
}

void init_timer_interrupts()
{
	// This uses TCB1 for timer interrupts.
	const unsigned short compare_value = 33333; // 100hz
	TCB1.CCMP = compare_value;
	TCB1.INTCTRL = TCB_CAPT_bm;
	// CTRLB bits 0-2 are the mode, which by default
	// 000 is "periodic interrupt" -which is correct
	TCB1.CTRLA = TCB_ENABLE_bm;
}

ISR(TCB1_INT_vect)
{
    master_state.tickcount++;
    TCB1.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

static void mainloop()
{
	int pulsewidth = 0;
	while (1) {
		_delay_ms(500);
		diag_println("ticks: %08ld", master_state.tickcount);
		diag_println("last_pulse_len: %06d pulse_count: %06d", 
			(int) rxin_state.last_pulse_len, (int) rxin_state.pulse_count);
		TCA0.SPLIT.HCMP0 = pulsewidth;
		pulsewidth += 10;
		if (pulsewidth > 180) {
			pulsewidth = 0;
		}
		vsense_test();
	}
}

int main(void)
{
	init_hardware();	
	init_timer_interrupts();
	sei();  // enable interrupts
	diag_puts("Malenki-ESC starting.\r\n");
	mainloop();
}
