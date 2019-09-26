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
#include "mixing.h"

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

//  Return the number of microseconds since boot
// This will wrap after less than 2 hours.
uint32_t get_micros()
{
    uint32_t ticks;
    uint16_t cnt;
    // There is an occasional race condition if cnt
    // has wrapped very recently, and an interrupt is due
    // but not happened yet.
    do {
        cli();
        ticks = master_state.tickcount;
        cnt = TCB1.CNT; // clock cycles
        sei();
    } while (cnt < 10); // if we lost the race, try again to allow irq
    cnt = cnt / 5;  // ticks to microsec
    uint32_t micros = ticks * 10000L + cnt;
    return micros;
}

ISR(TCB1_INT_vect)
{
    master_state.tickcount++;
    TCB1.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

static void __attribute__ ((unused)) test_get_micros()
{
    uint32_t times[50];
    _delay_us(99L * 1000L);
    for (uint8_t i=0; i< 50; i++) {
        times[i] = get_micros();
        _delay_us(3);
    }
    for (uint8_t i=0; i< 50; i++) {
        diag_println("micros = %ld", times[i]); 
    }
    uint32_t t0 = get_micros();
    _delay_ms(100);
    uint32_t t1 = get_micros();
    diag_println("before  delay = %ld", t0);
    diag_println("after delay = %ld", t1);
    diag_println("Testing monotonic clock");
    uint32_t fin_time = get_micros() + 500000L;
    uint32_t now = get_micros();
    while (1) {
        if (now > fin_time) {
            break;
        }
        _delay_us(50);
        uint32_t now2 = get_micros();
        if (now2 < now) {
            diag_println("ERROR ERROR - time went backwards");
            diag_println("%ld to %ld", now2, now);
            while(1) { 
                // it's the end.
            }
        }
        now = now2;
    }
    diag_println("ok");
}

void trigger_reset()
{
    // Pull the reset line.
    cli();
    while (1) {
        _PROTECTED_WRITE(RSTCTRL.SWRR, 1);
    }
}


int main(void)
{
    init_clock();
    init_serial();
    // Write the greeting message as soon as possible.
    diag_println("Malenki-nano receiver starting up");
    init_timer();
    sei(); // interrupts on
    // test_get_micros();
    spi_init();
    motors_init();
    mixing_init();
    nvconfig_load();
    radio_init();
    
    while(1) {
        // Main loop
        radio_loop();
    }
}
