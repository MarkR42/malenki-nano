#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 3333333 /* 20MHz / 6(default prescale) */
#include <util/delay.h>

#include <string.h>

char unused1;
unsigned char somestorage[10];

void init_ports_out()
{
	// Set all the ports which need to be outputs, to be outputs.
	// should all be low initially.
	// motor pwm outputs = PB0,1,2 and PA3,4,5
	// PB0, PB1, PB2
	// clear initial output
	PORTB.OUTCLR = 1 << 0;
	PORTB.OUTCLR = 1 << 1;
	PORTB.OUTCLR = 1 << 2;
	// set directions to out
	PORTB.DIRSET = 1 << 0;
	PORTB.DIRSET = 1 << 1;
	PORTB.DIRSET = 1 << 2;
}

void init_timer_interrupts()
{
	// This uses TCB0 for timer interrupts.
	const unsigned short compare_value = 33333; // 100hz
	TCB0.CCMP = compare_value;
	TCB0.INTCTRL = TCB_CAPT_bm;
	// CTRLB bits 0-2 are the mode, which by default
	// 000 is "periodic interrupt" -which is correct
	TCB0.CTRLA = TCB_ENABLE_bm;
}

volatile uint16_t tickcount = 0;
ISR(TCB0_INT_vect)
{
    tickcount++;
    TCB0.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

int main(void)
{
	memset(somestorage, 0, sizeof(somestorage));
	init_ports_out();	
	init_timer_interrupts();
	sei();  // enable interrupts
	_delay_ms(100);
	while(1) ;
}
