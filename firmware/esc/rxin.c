#include <avr/io.h>
#include <avr/interrupt.h>

#include "diag.h"
#include "rxin.h"

// Our global state
rxin_state_t rxin_state;

void init_rxin()
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
	// Configure TCB0
	// Set up Timer/Counter B to measure pulse length.
	TCB0.CTRLB = TCB_CNTMODE_PW_gc |
		TCB_CCMPEN_bm;
	TCB0.EVCTRL = // default edge
		TCB_CAPTEI_bm; // Enable capture.
	TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | // Divide system clock by2
		TCB_ENABLE_bm; // Turn on
	// Now TCB0 will wait until the pin is active, then start counting,
	// then capture the counter when the pin goes inactive.
	// We can receive the interrupt from the pin and get the captured value.
	// Enable the capture interrupt from TCB0.
	TCB0.INTCTRL =	TCB_CAPT_bm;

	// Diagnostic uart output	
	// TxD pin PA1 is used for diag, should be an output
	PORTA.DIRSET = 1 << 1;
	// Set baud rate etc
	USART0.CTRLC =
        	USART_CMODE_ASYNCHRONOUS_gc | // Mode: Asynchronous[default]
        	USART_PMODE_DISABLED_gc | // Parity: None[default]
        	USART_SBMODE_1BIT_gc | // StopBit: 1bit[default]
		USART_CHSIZE_8BIT_gc; // CharacterSize: 8bit[default]
	uint32_t want_baud_hz = 115200; // Baud rate
	uint32_t clk_per_hz = 3333L * 1000; // CLK_PER after prescaler in hz
	uint16_t baud_param = (64 * clk_per_hz) / (16 * want_baud_hz);
	USART0.BAUD = baud_param;
        USART0.CTRLB = USART_TXEN_bm ; // Start Transmitter
	rxin_state.last_pulse_len=0;
	rxin_state.pulse_count=0;
}

ISR(TCB0_INT_vect)
{
    // Received when the rx pulse pin goes low.
    // No need to explicitly clear the irq, it is automatically
    // cleared when we read CCPM
    uint16_t pulse_len = TCB0.CCMP;
    // TODO: this will check if the pulse is very big, or very small.
    // tiny pulses (<0.5ms) should be ignored.
    // Very long pulses (>3ms) indicate end of ppm/ sync
    rxin_state.last_pulse_len = pulse_len;
    rxin_state.pulse_count ++;
}

