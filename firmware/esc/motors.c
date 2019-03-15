#include <avr/io.h>
#include <avr/interrupt.h>

void motors_init()
{
	// Set all the ports which need to be outputs, to be outputs.
	// should all be low initially.
	// motor pwm outputs = PB0,1,2 and PA3,4,5
	// PB0, PB1, PB2
	// clear initial output
	PORTB.OUTCLR = 1 << 0;
	PORTB.OUTCLR = 1 << 1;
	PORTB.OUTCLR = 1 << 2;
	PORTA.OUTCLR = 1 << 3;
	PORTA.OUTCLR = 1 << 4;
	PORTA.OUTCLR = 1 << 5;
	// set directions to out
	PORTB.DIRSET = 1 << 0;
	PORTB.DIRSET = 1 << 1;
	PORTB.DIRSET = 1 << 2;
	PORTA.DIRSET = 1 << 3;
	PORTA.DIRSET = 1 << 4;
	PORTA.DIRSET = 1 << 5;
	//
	// Timer A should be in split mode
	//
	// As CLK_PER is 3.33 mhz, 
	// HOWEVER, in split mode we have 2x 8-bit timers,
	// so we cannot count to 3000.
	//
	// We can count to 200 because it's a more "round" number.
	// that gives us 200 PWM levels
	uint8_t period_val = 200;
	// Enable split mode
	TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
	// Enable all comparators in split mode
	TCA0.SPLIT.CTRLB = 
		TCA_SPLIT_LCMP0EN_bm |
		TCA_SPLIT_LCMP1EN_bm |
		TCA_SPLIT_LCMP2EN_bm |
		TCA_SPLIT_HCMP0EN_bm |
		TCA_SPLIT_HCMP1EN_bm |
		TCA_SPLIT_HCMP2EN_bm;
	// Set the period - use the same period for both halves.
	TCA0.SPLIT.HPER = period_val;
	TCA0.SPLIT.LPER = period_val;
	// Set the halves to be "out of sync"
	TCA0.SPLIT.HCNT = period_val /2;
	// Finally, turn the timer on.
	TCA0.SPLIT.CTRLA = 
		TCA_SPLIT_CLKSEL_DIV64_gc | // divide sys. clock by 64 
		TCA_SPLIT_ENABLE_bm;
	// To set the PWM duty, we can now write to
	// TCA0.HCMP0, .HCMP1, .HCMP2, .LCMP0, .LCMP1, .LCMP2
	// these would have values of 0.. period_val
	// here are sample values
	TCA0.SPLIT.HCMP0 = 20; // WO3 / pin PA3
	TCA0.SPLIT.HCMP1 = 40; // WO4 / PA4
	TCA0.SPLIT.HCMP2 = 60; // W05 / PA5
	TCA0.SPLIT.LCMP0 = 80; // WO0 / PB0
	TCA0.SPLIT.LCMP1 = 100; // WO1 / PB1
	TCA0.SPLIT.LCMP2 = 120; // WO2 / PB2
}

void motors_loop()
{

}
