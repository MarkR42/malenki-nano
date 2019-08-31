#include <stdint.h>

#include "a7105_spi.h"
#include "diag.h"

#define F_CPU 3333333 /* 20MHz / 6(default prescale) */
#include <util/delay.h>

static void register_dump()
{
    // Dump regs
    for (uint8_t n=0; n<0x30; n++) {
        uint8_t b = spi_read_byte(n);
        diag_print("%02x ", (int) b);
        if ((n % 8) == 7) {
            diag_println("");
        }
    }
    diag_println("");
}

//A7105 registers values -> ROM
// magic value 0xff means don't write.
static const unsigned char reg_init_values[] = {
	0xFF, 0x42, 0x00, 0x14, 0x00, 0xFF, 0xFF ,0x00,  // 0-7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x50, // 8-f 
    0x9e, 0x4b, 0x00, 0x02, 0x16, 0x2b, 0x12, 0x00, // 10-17
    0x62, 0x80, 0x80, 0x00, 0x0a, 0x32, 0xc3, 0x0f, // 18-1f
    0x13, 0xc3, 0x00, 0xFF, 0x00, 0x00, 0x3b, 0x00, // 20-27
    0x17, 0x47, 0x80, 0x03, 0x01, 0x45, 0x18, 0x00, // 28-2f
    0x01, 0x0f }; // 30,31

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

void radio_init()
{
    // Assume spi is initialised.
    diag_println("Resetting the a7105");
    spi_write_byte(0, 0x01); // Zero - reset register.
    _delay_ms(50);
    diag_println("Register dump after reset");
    register_dump();
    // program config registers
    program_config();
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
    diag_println("VCO Bank Calibrate channel 0xa0");
    spi_write_byte(0x0f, 0xa0); // set channel
    spi_write_byte(0x02, 0x02);
    wait_auto_clear(0x02, 0x02); 
    diag_println("Strobe standby");
    spi_strobe(STROBE_STANDBY);
    diag_println("Final register dump");
    register_dump();
    diag_println("End of radio_init");

}
