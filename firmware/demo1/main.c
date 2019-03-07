#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 3333333 /* 20MHz / 6(default prescale) */
#include <util/delay.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "diag.h"

char unused1;
unsigned char somestorage[10];

uint8_t junk1=0x2a;

void init_hardware_rx()
{
	// UART0- need to use "alternate" pins 
	// This puts TxD and RxD on PA1 and PA2
	PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc; 

	// Timer B TCB0 is used to measure the pulse width.
	// RxD pin (PA2) 
	PORTA.DIRCLR = 1 << 2;
	// EVENTSYS - use ASYNC channel 0 (works with porta)
	EVSYS.ASYNCCH0 = EVSYS_ASYNCCH0_PORTA_PIN2_gc;
	// routing for asynch user 0 == TCB0
	EVSYS.ASYNCUSER0 = EVSYS_ASYNCUSER0_ASYNCCH0_gc;
	// TODO:Configure TCB0

	// Diagnostic uart output	
	// TxD pin PA1 is used for diag, should be an output
	PORTA.DIRSET = 1 << 1;
	// Set baud rate etc
	USART0.CTRLC =
        	USART_CMODE_ASYNCHRONOUS_gc | // Mode: Asynchronous[default]
        	USART_PMODE_DISABLED_gc | // Parity: None[default]
        	USART_SBMODE_1BIT_gc | // StopBit: 1bit[default]
		USART_CHSIZE_8BIT_gc; // CharacterSize: 8bit[default]
	uint32_t want_baud_hz = 9600; // Baud rate
	uint32_t clk_per_hz = 3333L * 1000; // CLK_PER after prescaler in hz
	uint16_t baud_param = (64 * clk_per_hz) / (16 * want_baud_hz);
	USART0.BAUD = baud_param;
        USART0.CTRLB = USART_TXEN_bm ; // Start Transmitter
}

void init_hardware_motors()
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
	// We want a time period of about 1000hz.
	// As CLK_PER is 3.33 mhz, we need about 3k counts
	// HOWEVER, in split mode we have 2x 8-bit timers,
	// so we cannot count to 3000.
	//
	// So we need to enable divide by 16
	// 3333 / 16 =~ 208
	//
	// We can count to 200 because it's a more "round" number.
	// that gives us 200 PWM levels and slightly > 1khz freq.
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
	// Finally, turn the timer on.
	TCA0.SPLIT.CTRLA = 
		TCA_SPLIT_CLKSEL_DIV16_gc | // divide sys. clock by 16
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

void init_hardware()
{
	init_hardware_motors();
	init_hardware_rx();
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

volatile uint16_t tickcount = 0;
ISR(TCB1_INT_vect)
{
    tickcount++;
    TCB1.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

int main(void)
{
	memset(somestorage, 0, sizeof(somestorage));
	init_hardware();	
	init_timer_interrupts();
	sei();  // enable interrupts
	diag_puts("Hello\r\n");
	while (1) {
		_delay_ms(500);
		diag_println("ticks: %06d", tickcount);	
	}
}
