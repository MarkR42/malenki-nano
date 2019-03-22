#include <avr/io.h>
#include <avr/interrupt.h>

#include "diag.h"
#include "rxin.h"

#include <string.h>

// Our global state
volatile rxin_state_t rxin_state;

// Set rxin_state.next_channel to CHANNEL_SYNC
// if we are expecting a sync pulse.
#define CHANNEL_SYNC (-1)

// expected length of a sync pulse.
const uint16_t SYNC_PULSE_MIN=3000; // us
const uint16_t SYNC_PULSE_MAX=18000; // us
// Normal pulse len
// Allow a little bit of slack for weirdness etc.
const uint16_t PULSE_MIN=800; // us
const uint16_t PULSE_MAX=2500; // us

/*

    Our rx pin has a pull-down resistor. So if it has nothing
    driving it, it should stay low.

    If we are getting serial (uart) ibus or sbus data, we'd expect
    it to be mostly high with a lot of very short pulses 
    becuase they use baud rates of 100k+, so we'll see very short pulses
    of 10-80 microseconds, maybe longer ones between packets.

    If we are getting CPPM data, we expect it to be mostly high
    with long sync pulses (3-18 ms) between normal packets.

*/
static void rxin_init_hw()
{
	// UART0- need to use "alternate" pins 
	// This puts T xD and RxD on PA1 and PA2
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
	TCB0.CTRLB = TCB_CNTMODE_PW_gc ;
    // DO NOT SET TCB_CCMPEN_bm - because we don't want it outputting.
    // If TCB_CCMPEN_bm is enabled, then the timer will take one of our
    // gpio pins as its own, and TCA will not be able to send pulses.
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
}

static void rxin_init_state()
{
    memset((void *) &rxin_state, 0, sizeof(rxin_state));
    rxin_state.next_channel = CHANNEL_SYNC;
}

void rxin_init()
{
    rxin_init_state();
    rxin_init_hw();
}

ISR(TCB0_INT_vect)
{
    // Received when the rx pulse pin goes low.
    // No need to explicitly clear the irq, it is automatically
    // cleared when we read CCPM
    uint16_t pulse_len = TCB0.CCMP;
    // Early exit: quickly ignore very short pulses.
    // This is because another short pulse might happen, we do not want
    // to waste time in interrupts.
    if (pulse_len < 20) {
        return;
    }
    // This will be in clock units, which is 3.333mhz divided by
    // whatever the divider is set to,
    // so about 1.66mhz
    uint16_t pulse_len_us = ((uint32_t) pulse_len) * 100 / 166;
    rxin_state.last_pulse_len = pulse_len_us;
    rxin_state.pulse_count ++;
    if ((pulse_len_us >= SYNC_PULSE_MIN) && (pulse_len_us < SYNC_PULSE_MAX))
    {
        // Got a sync pulse
        // expect next channel 0.
        // pulses are not yet valid.
        rxin_state.next_channel = 0;
        rxin_state.pulses_valid = false;
        // Clear pulse_lengths_next
        memset((void *) rxin_state.pulse_lengths_next, 0, sizeof(rxin_state.pulse_lengths_next));
    } else {
        if ((pulse_len_us >= PULSE_MIN) && (pulse_len_us < PULSE_MAX)) {
            // Got a normal pulse
            if (rxin_state.next_channel != CHANNEL_SYNC) {
                rxin_state.pulse_lengths_next[rxin_state.next_channel] = pulse_len_us;
                rxin_state.next_channel += 1;
                if (rxin_state.next_channel >= RX_CHANNELS) {
                    // Got all channels.
                    // If the rx sends more channels, we ignore them.
                    // We have enough channels now, the rx can send whatever it wants.
                    rxin_state.pulses_valid = true; // tell main loop that the data are valid.
                    rxin_state.next_channel = CHANNEL_SYNC;
                } else {
                    // Not valid yet.
                }
            }
        }
    }
}

void rxin_loop()
{
    // put rx logic here
    if (rxin_state.pulses_valid) {
        // Great! read the pulses.
        // Avoid race condition - take a copy with irq disabled.
        cli(); // interrupts off
        memcpy((void *) rxin_state.pulse_lengths, (void *) rxin_state.pulse_lengths_next, sizeof(rxin_state.pulse_lengths));
        // clear valid flag so we do not do the same work again.
        rxin_state.pulses_valid = 0;
        sei(); // interrupts on
        // TODO: Here is where we will call the function to set motor speeds etc.
    }
}

