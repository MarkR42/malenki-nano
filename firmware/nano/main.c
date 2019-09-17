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
#include "motors.h"
#include "nvconfig.h"

volatile master_state_t master_state;

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
    
    uint32_t want_baud_hz = 115200; // Baud rate
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
    const unsigned short compare_value = 10000; // 100hz in clock ticks
    TCB1.CCMP = compare_value;
    TCB1.INTCTRL = TCB_CAPT_bm;
    // CTRLB bits 0-2 are the mode, which by default
    // 000 is "periodic interrupt" -which is correct
    TCB1.CTRLB = 0;
    TCB1.CNT = 0;
    // CTRLA- select CLK_PER and enable.
    TCB1.CTRLA = TCB_ENABLE_bm;
    master_state.tickcount = 0;
}

//  Return the number of microseconds since boot
// This will wrap after less than 2 hours.
uint32_t get_micros()
{
    cli();
    uint32_t ticks = master_state.tickcount;
    uint16_t cnt = TCB1.CNT / 10; // clock cycles 
    uint32_t micros = ticks * 10000 + cnt;
    sei();
    return micros;
}

ISR(TCB1_INT_vect)
{
    master_state.tickcount++;
    TCB1.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

static void __attribute__ ((unused)) test_get_micros()
{
    uint32_t times[10];
    for (uint8_t i=0; i< 10; i++) {
        times[i] = get_micros();
    }
    for (uint8_t i=0; i< 10; i++) {
        diag_println("micros = %ld", times[i]); 
    }
    uint32_t t0 = get_micros();
    _delay_ms(100);
    uint32_t t1 = get_micros();
    diag_println("before  delay = %ld", t0);
    diag_println("after delay = %ld", t1);
}

int main(void)
{
    init_clock();
    init_serial();
    // Write the greeting message as soon as possible.
    diag_println("Malenki-nano receiver starting up");
    init_timer();
    // test_get_micros();
    spi_init();
    motors_init();
    sei(); // interrupts on
    nvconfig_load();
    radio_init();
    
    while(1) {
        // Main loop
        radio_loop();
    }
}
