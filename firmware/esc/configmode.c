
#include <avr/io.h>
#include <stdint.h>

#include "diag.h"
#include "rxin.h"
#include "blinky.h"
#include "state.h"


void configmode_init()
{

}

void configmode_begin()
{
    // This is called just once when we enter config mode.
    rxin_state.rx_protocol = RX_PROTOCOL_CONFIG; // stop autodetect
    rxin_state.serial_mode = SERIAL_MODE_IBUS;  // set 115.2k
    rxin_init_serial(); // force rate and settings in case we were in sbus mode.
    // "Open drain" mode ?
    USART0.CTRLB |= USART_ODME_bm;
    // Set UART to LBME (Loop-back enable mode) so that we will send data
    // on the rx pin
    USART0.CTRLA |= USART_LBME_bm;
    diag_println("*** ENTERING CONFIG MODE ***");
    // Go super disco mode:
    blinky_state.blue_on = 1;
    blinky_state.flash_count = 6;
}

void configmode_handle_byte(uint8_t b)
{

}

static uint32_t last_high_time=0;
static uint32_t last_low_time=0;
static uint8_t config_enabled=0;

void configmode_loop() 
{

    // Try to detect a low pulse which is at least 10 ticks (100ms) long,
    // but less than 100 ticks long
    if (! config_enabled) {
        uint32_t now = get_tickcount(); 
        uint8_t pin = PORTA.IN & (1 << 1); // PA1
        if (pin) {
            uint32_t low_len = now - last_high_time;
            last_high_time = now;
            if ((low_len < 150) && ( low_len > 10)) {
                config_enabled = 1;
                configmode_begin();
            }
        }
        else {
            last_low_time = now;
        }
    }
}


