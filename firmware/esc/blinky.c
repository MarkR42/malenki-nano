#include "blinky.h"
#include "state.h"

#include <string.h>
#include <avr/io.h>

blinky_state_t blinky_state;

void blinky_init() {
    // NB: called with interrupts off
    memset((void *) & blinky_state, 0, sizeof(blinky_state));
    // start with a single flash
    blinky_state.flash_count = 1;
}

void blinky_loop() {
    // Get the time and divide by blink period / interval.
    // Which can be 16 centiseconds (or about 1/6 sec)
    uint32_t blinkticks = get_tickcount() / 16; // Compiler should optimise to a shift
    // Find out how far through the period we are?
    uint8_t blinkticks1 = ((uint8_t) blinkticks) & 0x0f; 
    bool red_on = ((blinkticks1 / 2) < blinky_state.flash_count) && (blinkticks1 & 1);
    bool blue_on = blinky_state.blue_on;
    // Set the led state.

    PORT_t *port = & (PORTA);
    uint8_t pin = 6;
    uint8_t pin_bm = 1 << pin;
    if (red_on || blue_on) {
        port->DIRSET = pin_bm;
    } else {
        // High-impedence
        port->DIRCLR = pin_bm;
    }
    if (red_on) {
        port->OUTCLR = pin_bm;
    } else {
        if (blue_on) {
            port->OUTSET = pin_bm;
        }
    }    
}

