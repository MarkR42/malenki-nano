#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 10000000 /* 10MHz / prescale=2 */
#include <util/delay.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "diag.h"
#include "a7105_spi.h"
#include "radio.h"
#include "state.h"

volatile master_state_t master_state;
extern const char * const end_marker;

static void init_clock()
{
    // This is where we change the cpu clock if required.
    uint8_t val = CLKCTRL_PDIV_2X_gc | 0x1;  // 0x1 = PEN enable prescaler.
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, val);
    _delay_ms(10);
}

static void init_serial()
{
    // UART0- need to use "alternate" pins 
    // This puts TxD and RxD on PA1 and PA2
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc; 
    // Diagnostic uart output       
    // TxD pin PA1 is used for diag, should be an output
    // And set it initially high
    PORTA.OUTSET = 1 << 1;
    PORTA.DIRSET = 1 << 1;
    
    uint32_t want_baud_hz = 230400; // Baud rate (was 115200)
    uint32_t clk_per_hz = F_CPU; // CLK_PER after prescaler in hz
    uint16_t baud_param = (64 * clk_per_hz) / (16 * want_baud_hz);
    USART0.BAUD = baud_param;
    USART0.CTRLB = 
        USART_TXEN_bm | USART_RXEN_bm; // Start Transmitter and receiver
    // Enable interrupts from the usart rx
    // USART0.CTRLA |= USART_RXCIE_bm;
}

static void init_timer()
{
    // This uses TCB1 for timer interrupts.
    // We need to count 100k clock cycles, so count to 50k
    // and enable divide by 2
    const unsigned short compare_value = 50000; 
    TCB1.CCMP = compare_value;
    TCB1.INTCTRL = TCB_CAPT_bm;
    // CTRLB bits 0-2 are the mode, which by default
    // 000 is "periodic interrupt" -which is correct
    TCB1.CTRLB = 0;
    TCB1.CNT = 0;
    // CTRLA- select CLK_PER/2 and enable.
    TCB1.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKDIV2_gc;
    master_state.tickcount = 0;
}

ISR(TCB1_INT_vect)
{
    master_state.tickcount++;
    TCB1.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag
}

void trigger_reset()
{
    // Pull the reset line.
    cli();
    while (1) {
        _PROTECTED_WRITE(RSTCTRL.SWRR, 1);
    }
}

uint32_t get_tickcount()
{
    uint32_t tc;
    cli(); // disable irq
    tc = master_state.tickcount;
    sei(); // enable irq
    return tc;
}

void epic_fail(const char * reason)
{
    diag_puts(reason);
    diag_puts("\r\nFAIL FAIL FAIL!\r\n\n\n");
    _delay_ms(250);
    trigger_reset(); // Reset the whole device to try again.
}

int main(void)
{
    init_clock();
    init_serial();
    diag_println("\r\nReceiver test / scanner");
    init_timer();
    sei(); // interrupts on

    spi_init();
    // Initialise the radio last.
    radio_init();
    
    radio_rxtest_loop();
}
