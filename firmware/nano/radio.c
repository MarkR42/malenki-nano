#include <avr/io.h>

#include <stdint.h>
#include <stdbool.h>

#include "a7105_spi.h"
#include "diag.h"
#include "radio.h"
#include "state.h"

#define F_CPU 2000000
#include <util/delay.h>

#include <string.h>

// GIO port is on this pin
PORT_t * const GIO1_PORT = &PORTA;
const uint8_t GIO1_PIN = 2;
const uint8_t GIO1_bm = 1 << GIO1_PIN;

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

static void radio_state_init();

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
    diag_println("End radio_init");
    radio_state_init(); 
}

static bool wait_gio1()
{
    // Wait for gio1 to go high then low
    // wait for it to go high (rx started)
    int i=0;
    while (! ( GIO1_PORT->IN & GIO1_bm))  {
        // Nothing 
        _delay_us(100);
        i += 1;
        if (i >= 2000) {
            return false;
        }
    }
    // Wait for it to go low (rx finished)
    i=0;
    while (GIO1_PORT->IN & GIO1_bm) {
        // Nothing 
        i += 1;
        if (i >= 2000) {
            return false;
        }
    }
    return true;
}

static void print_packet(const uint8_t *buf, uint8_t len)
{
    for (uint8_t i=0; i< len; i++) {
        diag_print("%02x ", (int) buf[i]);
    }
    diag_println(" ");
}

static void set_led(bool on)
{
    // Use GIO2 to light the led.
    const uint8_t gio2_reg = 0x0c;
    const uint8_t gio2_function_bm = 0x30; // 0x30=inhibited
    if (on) {
        spi_write_byte(gio2_reg, gio2_function_bm | 0x1 | 0x02 ); // Enable, invert
    } else {
        spi_write_byte(gio2_reg, gio2_function_bm | 0x1 ); // Enable only
    }
}

radio_state_t radio_state;

static void radio_state_init() {
    memset((void *) &radio_state, 0, sizeof(radio_state));
    radio_state.state = RADIO_STATE_WAITING; // We may set it to bind later.
}

static bool is_all_zeros(const uint8_t *buf, uint8_t len)
{
    for (uint8_t i=0; i<len; i++) {
        if (buf[i] != 0) {
            return false;
        }
    } 
    return true;
}

static void set_rx_channel(uint8_t channel)
{
    spi_write_byte(0x0f, channel);
}

static void start_rx()
{
    spi_strobe(STROBE_READ_PTR_RESET);
    spi_strobe(STROBE_RX);
    // after a packet is received, it goes into standby automatically.
    // then we have to re-trigger it by doing a READ_PTR_RESET
    // Then RX strobe.
}

static void enter_bind_mode()
{
    diag_println("Entering bind mode");
    radio_state.state = RADIO_STATE_BIND;
    // In bind mode, the tx uses 0x0d, but we need to set one channel lower.
    set_rx_channel(0x0c); 
    radio_state.flash_time = get_micros();
    start_rx();
}


static void do_radio_waiting()
{
    // Waiting for a signal from the tx
    // Check if the tx ID is actually zeros?
    if (is_all_zeros((uint8_t *) & radio_state.tx_id, sizeof(radio_state.tx_id))) {
        diag_println("No tx id stored.");
        enter_bind_mode();
        return;
    }
    // here we should check for received packet, etc.
}

static bool rx_packet()
{   
    bool ret = false;
    // Return true iff a packet is received.
    // Put the packet data in radio_state.packet
    if (! (GIO1_PORT->IN & GIO1_bm)) {
        // Got something?
        uint8_t modeflags = spi_read_byte(0);
        // I think flag 0x01 (bit 0) is the "read data ready" flag, active low,doc is not clear about this.
        // bits 5 and 6 are read error flags.
        uint8_t errflags = (1 << 6) | (1 << 5);
        if (! (modeflags & errflags)) {
            // No error.    
            // Read packet buffer, 
            spi_read_block(0x5, radio_state.packet, RADIO_PACKET_LEN);
            ret = true;
        }
        // restart the rx.
        start_rx();
    }
    return ret;
}


static void do_radio_bind()
{
    uint32_t now = get_micros();
    // Blink led quickly.
    if ((now - radio_state.flash_time) > 0x10000) {
        set_led((now / 0x10000) % 2);  
    }
    if (rx_packet()) {
        // Switch to next channel
        radio_state.packet_counter += 1;
        uint8_t newchan = (radio_state.packet_counter & 1) ? 0x0c : 0x8b;
        set_rx_channel(newchan);        
        diag_print("bind: got packet ");
        print_packet(radio_state.packet, RADIO_PACKET_LEN);
    }
}

// BIND MODE: uses channels
// 0c and 8b
// Transmitter transmits one channel number higher?
// So we need to subtract one from every channel no?
void radio_loop()
{
    // called each time
    switch (radio_state.state) {
        case RADIO_STATE_BIND:
            do_radio_bind();
            break;
        case RADIO_STATE_WAITING:
            do_radio_waiting();
            break;
        case RADIO_STATE_HOPPING:
            // TODO
            break;

    }
}

