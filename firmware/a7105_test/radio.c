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
            diag_puts("\r\n");
        }
    }
    diag_puts("\r\n");
}

//A7105 registers values -> ROM
// magic value 0xff means don't write.
static const unsigned char reg_init_values[] = {
    // NOTE: Registers 0xb and 0xc control GIO1 and GIO2, set them to 0 for now.
	0xFF, 0x42 , 0x00, 0x25, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x50,	// 00 - 0f
	0x9e, 0x4b, 0x00, 0x02, 0x16, 0x2b, 0x12, 0x4f, 0x62, 0x80, 0xFF, 0xFF, 0xff, 0x32, 0xc3, 0x1f,				// 10 - 1f
	0x1e, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x3b, 0x00, 0x17, 0x47, 0x80, 0x03, 0x01, 0x45, 0x18, 0x00,				// 20 - 2f
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
#define STROBE_READ_PTR_RESET 0xf0 

// This is the magic ID for the a7105 for flysky protocol
// AFHDS2A protocol - contains this ID ending with 2A
static const uint8_t radio_id[] = {
    0x54, 0x75, 0xc5, 0x2a
    };
void radio_init()
{
    // Assume spi is initialised.
    diag_println("Resetting the a7105");
    spi_write_byte(0, 0x00); // Zero - reset register.
    _delay_ms(50);
    // Write the ID
    diag_println("Writing 4-byte ID");
    spi_write_block(0x6, radio_id, 4);
    diag_println("Register dump after reset");
    register_dump();
    // program config registers
    program_config();
    diag_println("Register dump after programming");
    register_dump();
    spi_strobe(STROBE_PLL);
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
    diag_println("Reset VCO band calibration");
    spi_write_byte( 0x25, 0x08 );

    diag_println("Strobe standby");
    spi_strobe(STROBE_STANDBY);
    diag_println("Final register dump");
    register_dump();
    //uint8_t id_readback[4];
    //spi_read_block(0x06, id_readback,4);
    //for (int i=0; i<4; i++) {
    //    diag_println("id_readback[%d]=%02x", i, id_readback[i]);
    //}
    diag_println("End of radio_init");
    
}

void test_read_start()
{
    spi_write_byte(0x0f, 0x0); // set channel back to zero
    // Now reset our pointer - FIFO register 1 (0x03)
    spi_strobe(STROBE_READ_PTR_RESET);
    // Set to PLL mode
    spi_strobe(STROBE_PLL);
    // Enable RX
    spi_write_byte(0x0f, 0x1); // set channel
    spi_strobe(STROBE_RX); 
}

void test_read()
{
    // Check flags
    uint8_t modeflags = spi_read_byte(0);
    // I think flag 0x01 (bit 0) is the "read data ready" flag, doc is not clear about this.
    // bits 5 and 6 are read error flags.

    // diag_println("modeflags=%02x endptr=%02x", (int) modeflags, (int) endptr);
    uint8_t errflags = (1 << 6) | (1 << 5);
    if (! (modeflags & errflags))
    {
        diag_print("modeflags=%02x data=", (int) modeflags);
        // Read buffer
        uint8_t buf[16];
        spi_read_block(0x5, buf, sizeof(buf));
        for (uint8_t i=0; i< sizeof(buf); i++) {
            diag_print("%02x ", (int) buf[i]);
        }
        diag_println(" ");
        // Return to reading.
        // RX already enabled
        // re-Enable rx
        spi_strobe(STROBE_READ_PTR_RESET);
        spi_write_byte(0x0f, 0x1); // set channel
        spi_strobe(STROBE_RX); 
    }
        
}

