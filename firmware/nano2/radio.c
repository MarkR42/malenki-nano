#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stdbool.h>

#include "a7105_spi.h"
#include "diag.h"
#include "radio.h"
#include "state.h"
#include "nvconfig.h"
#include "motors.h"
#include "mixing.h"

#define F_CPU 10000000
#include <util/delay.h>

#include <string.h>

// GIO port is on this pin
PORT_t * const GIO1_PORT = &PORTA;
const uint8_t GIO1_PIN = 2;
const uint8_t GIO1_bm = 1 << GIO1_PIN;

radio_state_t radio_state;

/*
 * Blinking LED goes on this GPIO pin.
 * Note: on attiny1614, this pin does not exist, but it's ok.
 * We will blink it on a chip where it does exist.
 */
PORT_t * const BLINKY_PORT = &PORTB;
const uint8_t BLINKY_bm = 1 << 4;

static void register_dump()
{
    // Dump regs
    for (uint8_t n=0; n<0x30; n++) {
        uint8_t b = spi_read_byte(n);
        diag_print("%02x ", (int) b);
        if ((n % 16) == 15) {
            diag_puts("\r\n");
        }
    }
    diag_puts("\r\n");
}

//A7105 registers values -> ROM
// magic value 0xff means don't write.
static const unsigned char reg_init_values[] = {
    // NOTE: Registers 0xb and 0xc control GIO1 and GIO2, set them to 0 for now.
    0xFF, 0x42 , 0x00, 0x25, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x50,        // 00 - 0f
    0x9e, 0x4b, 0x00, 0x02, 0x16, 0x2b, 0x12, 0x00, 0x62, 0x80, 0xFF, 0xFF, 0x2a, 0x32, 0xc3, 0x1f,                         // 10 - 1f
    0x13, 0xc3, 0x00, 0xFF, 0x00, 0x00, 0x3b, 0x00, 0x17, 0x47, 0x80, 0x03, 0x01, 0x45, 0x18, 0x00,                         // 20 - 2f
    0x01, 0x0f // 30 - 31
};

static void program_config()
{
    for (uint8_t addr =0; addr < sizeof(reg_init_values); addr ++) {
        uint8_t v = reg_init_values[addr];
        if (v != 0xff) {
            spi_write_byte(addr, v);
        }
    }
}
static void wait_auto_clear(uint8_t reg, uint8_t bit)
{
    while (1) {
        uint8_t v = spi_read_byte(reg);
        if (! (v & bit)) {
            // Cleared.
            break;
        }
    }
}

static void init_interrupts()
{
    // TCB0 - we set this up to generate periodic interrupts at the timeout
    // period.
    // Timeout period should be 3900 microseconds
    
    // Timer will run at CLK_PER (10mhz) / 2
    // So 5mhz.
    
    diag_println("Initialising interrupts");
    uint16_t compare_value = 3900 * 5 ; // Must not overflow 16-bit int.
    TCB0.CCMP = compare_value;
    TCB0.INTCTRL = TCB_CAPT_bm; // Turn on irq
    // CTRLB bits 0-2 are the mode, which by default
    // 000 is "periodic interrupt" -which is correct
    TCB0.CTRLB = 0;
    TCB0.CNT = 0;
    // CTRLA- select CLK_PER/2 and enable.
    TCB0.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKDIV2_gc;
    /*
     * RX packet interrupt: this should fire on the LOW edge of
     * GIO1 pin
     * assume GIO1_PORT = PORTA.
     */
    PORTA.PIN2CTRL = PORT_ISC_FALLING_gc;
    
}


// Strobe commands - should have bit A7 set (0x80)
#define STROBE_STANDBY 0xa0 
#define STROBE_PLL 0xb0 
#define STROBE_RX 0xc0 
#define STROBE_READ_PTR_RESET 0xf0 

// This is the magic ID for the a7105 for flysky protocol
// AFHDS2A protocol - contains this ID ending with 2A
static const uint8_t radio_id[] = {
    0x54, 0x75, 0xc5, 0x2a
    };

void radio_init()
{
    // Assume spi is initialised.
    diag_println("Begin radio_init");
    diag_println("Resetting the a7105");
    _delay_ms(25); // Wait for it to be ready, in case we just powered on.
    spi_write_byte(0, 0x00); // Zero - reset register.
    _delay_ms(50);
    // Write the ID
    diag_println("Writing 4-byte ID");
    spi_write_block(0x6, radio_id, 4);
    // diag_println("Register dump after reset");
    // register_dump();
    // program config registers
    program_config();
    diag_println("Configuring GIO1 pin...");
    spi_write_byte(0xb, 0x1);
    diag_println("Register dump after programming");
    register_dump();
    spi_strobe(STROBE_STANDBY);
    diag_println("IF filter bank calibration");
    spi_write_byte(0x02, 0x01);
    // Wait for register to auto clear.
    wait_auto_clear(0x02, 0x01);
    diag_println("VCO Current Calibration");
    spi_write_byte(0x24, 0x13); //Recomended calibration from A7105 Datasheet
    diag_println("VCO bank calibration");
	spi_write_byte(0x26, 0x3b);
    diag_println("VCO Bank Calibrate channel 0");
    spi_write_byte(0x0f, 0); // set channel
    spi_write_byte(0x02, 0x02);
    wait_auto_clear(0x02, 0x02); 
    uint8_t cal0 = spi_read_byte(0x25);
    if (cal0 & 0x08) 
        diag_println("!!! VCO Calibration fail");
    diag_println("VCO Bank Calibrate channel 0xa0");
    spi_write_byte(0x0f, 0xa0); // set channel
    spi_write_byte(0x02, 0x02);
    wait_auto_clear(0x02, 0x02); 
    cal0 = spi_read_byte(0x25);
    diag_println("vco cal a0=%02x", (int) cal0);
    if (cal0 & 0x08) 
        diag_println("!!! VCO Calibration fail");
    diag_println("Reset VCO band calibration");
    spi_write_byte( 0x25, 0x08 );

    diag_println("Strobe standby");
    spi_strobe(STROBE_STANDBY);
    diag_println("Final register dump");
    register_dump();
    uint8_t id_readback[4];
    spi_read_block(0x06, id_readback,4);
    for (int i=0; i<4; i++) {
        diag_println("id_readback[%d]=%02x", i, id_readback[i]);
    }
    if (memcmp(id_readback, radio_id,4) != 0) {
        diag_println("ERROR: Wrong ID readback");
        diag_println("Possible hardware fault. Error not recoverable.");
        diag_println("FAIL FAIL FAIL!");
        _delay_ms(250);
        trigger_reset(); // Reset the whole device.
    }
    init_interrupts();
    diag_println("End radio_init");
    /*
     * Note: we will not start the rx here,
     * we just wait for the radio timeout, which will restart the
     * rx anyway.
     */
}


// BIND MODE: uses channels
// 0c and 8b
// Transmitter transmits one channel number higher?
// So we need to subtract one from every channel no?
void radio_loop()
{
    // called each time
    /*
    switch (radio_state.state) {
        case RADIO_STATE_BIND:
            do_radio_bind();
            break;
        case RADIO_STATE_WAITING:
            do_radio_waiting();
            break;
        case RADIO_STATE_HOPPING:
            do_radio_hopping();
            break;

    }
    */
}

ISR(TCB0_INT_vect)
{
    // Periodic interrupt:
    // TODO: Handle rx timeout channel hopping.
    // The timer should have its CNT reset to 0 every time we get a
    // real packet, to suppress the timeout
    TCB0.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

ISR(PORTA_PORT_vect)
{
    // RX packet.
    // Check the flag on the port - if it is low, then check for reset.
    // pin could legitimately be high, if the interrupt arrived while
    // we were processing a timeout interrupt, in which case, it's
    // too late and we missed it.
    uint8_t bit = PORTA.IN & GIO1_bm;
    if (! bit) {
        // check rx packet.
    }
    // clear the irq
    PORTA.INTFLAGS = GIO1_bm;
}
