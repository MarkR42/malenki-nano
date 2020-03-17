#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stdbool.h>

#include "a7105_spi.h"
#include "diag.h"
#include "radio.h"
#include "state.h"

#define F_CPU 10000000
#include <util/delay.h>

#include <string.h>

// GIO port is on this pin
PORT_t * const GIO1_PORT = &PORTA;
const uint8_t GIO1_PIN = 2;
const uint8_t GIO1_bm = 1 << GIO1_PIN;

static void dump_buf(uint8_t *buf, uint8_t len);

static void register_dump()
{
    // Dump regs
    const size_t nregs = 0x30;
    uint8_t buf[nregs];
    for (uint8_t n=0; n<0x30; n++) {
        buf[n] = spi_read_byte(n);
    }
    dump_buf(buf, nregs);
}

static void dump_buf(uint8_t *buf, uint8_t len)
{
    for (uint8_t i=0; i<len; i++) {
        diag_print("%02x ", (int) buf[i]);
        if ((i % 16) == 15) {
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

// Strobe commands - should have bit A7 set (0x80)
#define STROBE_STANDBY 0xa0 
#define STROBE_PLL 0xb0 
#define STROBE_RX 0xc0 
#define STROBE_TX 0xd0 
#define STROBE_READ_PTR_RESET 0xf0 
#define STROBE_WRITE_PTR_RESET 0xe0 

// This is the magic ID for the a7105 for flysky protocol
// AFHDS2A protocol - contains this ID ending with 2A
static const uint8_t radio_id[] = {
    0x54, 0x75, 0xc5, 0x2a
    };

static void init_gio1_pin()
{
    diag_println("Configuring GIO1 pin...");
    // Bit 0 = enable
    // Bit 1= invert
    // Bits 2-5 = select mode (0= WTR wait until ready, 0xc = inhibit)
    // We should initially have no received data, so it
    // Should go low  as soon as we enable.
    uint8_t gio_setting = 0x1; // This is what we want to program.   
    spi_write_byte(0xb, gio_setting);    
    uint8_t in0 = (GIO1_PORT->IN & GIO1_bm);
    spi_write_byte(0xb, gio_setting | 0x2); // Enable invert bit    
    uint8_t in1 = (GIO1_PORT->IN & GIO1_bm);
    diag_println(" in0=%02x in1=%02x", (int) in0, (int) in1);
    if (in0 == in1) {
        // This should really not happen
        diag_println("GIO1 pin does not appear to work");
        epic_fail("GIO1 pin fail");        
    }
    // Reset to expected value.
    spi_write_byte(0xb, gio_setting);        
}

static void init_a7105_hardware()
{
    // Assume spi is initialised.
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
    init_gio1_pin();
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
        epic_fail("ERROR: Wrong ID readback");
    }
    // Check channel is reading back.
    uint8_t b = spi_read_byte(0x0f);
    if (b != 0xa0) {
        diag_println("channel value %02x should be a0",
            b);
        epic_fail("Fail to read channel back");
    }
}

void radio_init()
{
    diag_println("Begin radio_init");
    init_a7105_hardware();

    diag_println("End radio_init");
}

void radio_txtest()
{
    diag_println("radio_txtest");
    // Prepare packet
    const size_t packet_len = 24;
    uint8_t packet[packet_len];
    memset((void *) packet, 0, packet_len);
    packet[0] = 0xfc; // Special magic value
    // Try to transmit
    // strobe: reset write ptr
    spi_strobe(STROBE_WRITE_PTR_RESET);
    // Write our data with spi_write_block
    spi_write_block(0x5, // Data buffer register
        packet, packet_len);

    // Set channel and Strobe: begin tx
    uint8_t channel = 0xc; // Bind channel
    uint8_t gio1_before = GIO1_PORT->IN & GIO1_bm;
    spi_write_byte_then_strobe(0x0f, channel, STROBE_TX);
    uint8_t gio1_after = GIO1_PORT->IN & GIO1_bm;
    diag_println("before %d after %d", (int)gio1_before, (int)gio1_after);
    // wait for tx finish
    // this could read status register or use gio1?
    // gio1 should go low when tx is completed.
    for(uint8_t i=0; i<100; i++) {
        bool on = GIO1_PORT->IN & GIO1_bm;
        if (! on) {
            diag_println("Transmitted after %d count", i);
            break;
        }
        _delay_us(10);
    }
}
