#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 3333333 /* 20MHz / 6(default prescale) */
#include <util/delay.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "diag.h"
#include "a7105_spi.h"
#include "radio.h"

static void init_clock()
{
    // Change main clock to 10mhz
    // Default  is divide by 6, = 3.33 mhz
    // Just enable the prescaler, which by default will divide the clock /2
    // _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PEN_bm);
    // Which should be 10mhz.
}


static void init_serial()
{
    // UART0- need to use "alternate" pins 
    // This puts TxD and RxD on PA1 and PA2
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc; 
    // Diagnostic uart output       
    // TxD pin PA1 is used for diag, should be an output
    PORTA.DIRSET = 1 << 1;
    
    uint32_t want_baud_hz = 115200; // Baud rate
    uint32_t clk_per_hz = 3333L * 1000; // CLK_PER after prescaler in hz
    uint16_t baud_param = (64 * clk_per_hz) / (16 * want_baud_hz);
    USART0.BAUD = baud_param;
    USART0.CTRLB = 
        USART_TXEN_bm | USART_RXEN_bm; // Start Transmitter and receiver
    // Enable interrupts from the usart rx
    // USART0.CTRLA |= USART_RXCIE_bm;
 
}

static void init()
{
    // Use these pins:
    // PA3 and PA4 - these are the two closes to the power pins.
    PORTA.DIRSET = 1 << 3;
    PORTA.DIRSET = 1 << 4;
}

int main(void)
{
    init();
    init_clock();
    init_serial();
    init_spi();
    sei(); // interrupts on
    diag_println("Hello, world");
    diag_println("This is a test program to test SPI communication with the A7105");
    diag_println("And also the A7105 radio");
    PORTA.OUTSET = 1 << 4;
    _delay_ms(250); 
    PORTA.OUTCLR = 1 << 4;
    radio_init();
    test_read_start();
    
    int counter=0;
    PORTA.OUTSET = 1 << 4; // led on
    while(1) {
        counter ++;
        // PORTA.OUTSET = 1 << 4;
        _delay_ms(50); 
        // PORTA.OUTCLR = 1 << 4;
        if ((counter & 0x0f) ==0) diag_println("loop %d", counter);
        test_read();
    }
}
