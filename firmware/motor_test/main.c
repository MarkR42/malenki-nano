#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 3333000 // default prescale
#include <util/delay.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "diag.h"
#include "state.h"
#include "motors.h"

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
    
    uint32_t want_baud_hz = 9600; // Baud rate
    uint32_t clk_per_hz = F_CPU; // CLK_PER after prescaler in hz
    uint16_t baud_param = (64 * clk_per_hz) / (16 * want_baud_hz);
    USART0.BAUD = baud_param;
    USART0.CTRLB = 
        USART_TXEN_bm | USART_RXEN_bm; // Start Transmitter and receiver
    // Enable interrupts from the usart rx
    // USART0.CTRLA |= USART_RXCIE_bm;
}

static void trigger_reset()
{
    // Pull the reset line.
    cli();
    while (1) {
        _PROTECTED_WRITE(RSTCTRL.SWRR, 1);
    }
}

static void uninit_everything()
{
    USART0.CTRLB = 0; //disable rx and tx
    // Set all ports as inputs.
    PORTA.DIR = 0 ;
    PORTB.DIR = 0 ;
    PORTC.DIR = 0 ;
}

void epic_fail(const char * reason)
{
    diag_puts(reason);
    diag_puts("\r\nFAIL FAIL FAIL!\r\n\n\n");
    _delay_ms(10); // allow transmission of message
    cli(); // No interrupts now please.
    uninit_everything();
    // This gives UPDI a chance to come in and erase or 
    // reflash the device, which we might not be able to do
    // if there is an electrical fault with some pins held.
    _delay_ms(1000); 
    trigger_reset(); // Reset the whole device to try again.
}

static void read_all_ports(uint8_t *ports_state)
{
    ports_state[0] = PORTA.IN;
    ports_state[1] = PORTB.IN;
    ports_state[2] = PORTC.IN;
}

static void diag_binary(uint8_t byte)
{
    for (uint8_t i=0; i< 8; i++) {
        if (byte & 0x80) {
            diag_puts("1");
        } else {
            diag_puts("0");
        }
        byte = byte << 1;
    }
}

static void dump_ports(const uint8_t * ports_state)
{
    for (uint8_t i=0; i<3; i++) {
        diag_binary(ports_state[i]);
        diag_puts(" ");
    }
    diag_puts("\r\n");
}

static void electrical_test_port(char portname, PORT_t *port, uint8_t pin)
{
    uint8_t port_index = (portname - 'A'); // 0 = porta etc
    // set the port as an input, get initial data
    uint8_t bm = 1 << pin;
    uint8_t bm_inverse = ~ bm;
    port->DIRCLR = bm;
    _delay_ms(50); // long pause to allow serial to finish.
    uint8_t ports_initial[3];
    read_all_ports(ports_initial);
    // Try setting it high
    port->OUTSET = bm;
    port->DIRSET = bm;
    _delay_us(100);
    uint8_t ports_high[3];
    read_all_ports(ports_high);
    _delay_us(50);
    // Try setting it low.
    port->OUTCLR = bm;
    _delay_us(50);
    uint8_t ports_low[3];
    read_all_ports(ports_low);
    // return to input state.
    port->DIRCLR = bm;
    
    // Mask out the port itself
    ports_initial[port_index] &= bm_inverse;
    ports_high[port_index] &= bm_inverse;
    ports_low[port_index] &= bm_inverse;
    
    // Check if there are any differences.
    int diff_high = memcmp(ports_initial, ports_high, 3);
    int diff_low = memcmp(ports_initial, ports_low, 3);
    
    diag_print("Testing port %c %d ", portname, (int) pin);
    
    if (diff_high != 0) {
        diag_println("Difference when pin is high!");
    } else {
        if (diff_low != 0) {
            diag_println("Difference when pin is low!");
        } else {
            diag_println("OK");
        }
    }
    diag_puts("init: "); dump_ports(ports_initial);
    diag_puts("high: "); dump_ports(ports_high);
    diag_puts("low:  "); dump_ports(ports_low);
    
}

static void electrical_test()
{
    // Carry out an electrical test, to check if any of our GPIO
    // pins are shorted with each other 
    // We will not test every pin, because some are connected to 
    // important stuff:
    // PA0 - UPDI - PA1 - TXDEBUG (red led+ debug tx data)
    // PC3, PC4 - connected to GND by design to make layout easier
    // While we test one pin, all other pins should be set as inputs.
    // (except debug port which is needed to print out results)
    char portname = 'A';
    uint8_t pin;
    for (pin=2; pin < 8; pin ++) {
        electrical_test_port(portname, &PORTA, pin);
    }
    portname = 'B';
    for (pin=0; pin < 8; pin ++) {
        electrical_test_port(portname, &PORTB, pin);
    }
    portname = 'C';
    for (pin=0; pin < 2; pin ++) {
        electrical_test_port(portname, &PORTC, pin);
    }
}

int main(void)
{
    // PA1 = red led and also serial

    init_serial();
    diag_println("Motor test");
    electrical_test();
    
    // Set port output for blue led
    // PB4 = blue led
    PORTB.DIRSET = 1 << 4;
    motors_init();

    uint8_t motor_id=0;
    while (1) { 
        // Flash leds and run motors
        diag_println("Running motor %d", (int) motor_id);
        _delay_ms(250);
        PORTB.OUTSET = 1 << 4;
        set_motor_direction_duty(motor_id, 100);
        _delay_ms(250);
        PORTB.OUTCLR = 1 << 4;
        set_motor_direction_duty(motor_id, 0);
        _delay_ms(250);
        PORTB.OUTSET = 1 << 4;
        set_motor_direction_duty(motor_id, -100);
        _delay_ms(250);
        PORTB.OUTCLR = 1 << 4;
        motors_all_off();
        motor_id = (motor_id + 1) % 3;
    }
}
