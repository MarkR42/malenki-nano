#include <avr/io.h>

#include <stdint.h>
#include <stdbool.h>

#include "a7105_spi.h"
#include "diag.h"
#include "radio.h"
#include "state.h"
#include "nvconfig.h"

#define F_CPU 10000000
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

static void enter_waiting_mode();
static void enter_bind_mode();

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
    diag_println("End radio_init");
    enter_waiting_mode();
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

static bool is_all_zeros(const uint8_t *buf, uint8_t len)
{
    for (uint8_t i=0; i<len; i++) {
        if (buf[i] != 0) {
            return false;
        }
    } 
    return true;
}

/*
 * Note when calling set_rx_channel we need to subtract 1 from the channel number
 * used by the tx,
 *
 * Because of Reasons :)
 */

static void start_rx(uint8_t channel)
{
    spi_strobe2(STROBE_STANDBY, STROBE_READ_PTR_RESET);
    spi_write_byte(0x0f, channel);
    spi_strobe(STROBE_RX);
    // after a packet is received, it goes into standby automatically.
    // then we have to re-trigger it by doing a READ_PTR_RESET
    // Then RX strobe.
}

static void restart_rx()
{
    spi_strobe3(STROBE_STANDBY, STROBE_READ_PTR_RESET, STROBE_RX);
}

static void enter_waiting_mode()
{
    diag_println("Entering waiting mode; waiting for sticks data signal");
    radio_state.state = RADIO_STATE_WAITING;
    set_led(0);
    // Check if the tx ID is actually zeros?
    if (is_all_zeros((uint8_t *) & radio_state.tx_id, sizeof(radio_state.tx_id))) {
        diag_println("No tx id stored.");
        enter_bind_mode();
        return;
    }
    diag_print("stored tx id: "); print_packet(radio_state.tx_id, 4);
    radio_state.hop_index = 0;
    start_rx(radio_state.hop_channels[0] - 1); // subtract 1 from tx channel
}

static void enter_bind_mode()
{
    diag_println("Entering bind mode");
    radio_state.state = RADIO_STATE_BIND;
    // Clear the tx list (in case we used it earlier?)
    memset((void *) &(radio_state.possible_tx_list), 0, sizeof(radio_state.possible_tx_list));
    // In bind mode, the tx uses 0x0d, but we need to set one channel lower.
    start_rx(0x0c); 
    radio_state.flash_time = get_micros();
}

#define PACKET_CMD_STICKS 0x58
#define PACKET_CMD_BIND1 0xbb
#define PACKET_CMD_BIND2 0xbc

static bool process_sticks_packet()
{
    // Returns true iff it is a sticks packet from our tx
    // Check tx id.
    uint8_t * tx_id = (radio_state.packet + 1);
    if (memcmp(tx_id, radio_state.tx_id, 4)) {
        // Wrong transmitter.
        return false;
    }
    uint8_t cmd = radio_state.packet[0];
    if (cmd != PACKET_CMD_STICKS) {
        // Not a sticks packet.
        // diag_puts("odd:");
        // print_packet(radio_state.packet, 10);
        return false;
    } else {
        // got a good sticks packet.
        // todo: store sticks data.
        // print_packet(radio_state.packet, RADIO_PACKET_LEN);
        return true;
    } 
}

static bool rx_packet(bool restart_always)
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
        // restart the rx, if restart_always is on,
        // or if we didn't receive a valid packet.
        if (restart_always || (! ret) ) restart_rx();
    }
    return ret;
}

// Number of hops to hop, per hop
// Use this to slow down the rate to get more chance to process.
#define HOP_COUNT 3

static void hop_next_channel()
{
    radio_state.hop_index += HOP_COUNT;
    if (radio_state.hop_index >= NR_HOP_CHANNELS)  {
        radio_state.hop_index -= NR_HOP_CHANNELS;
    }
    uint8_t nextchan = radio_state.hop_channels[radio_state.hop_index];
    start_rx(nextchan - 1); // subtract 1 because reasons.
}

static void enter_hopping_mode()
{
    // Called when we reach "full" hopping mode and receive a signal
    diag_println("gotsticks");
    radio_state.state = RADIO_STATE_HOPPING;
    radio_state.missed_packet_count = 0;
    set_led(1);
}

// Time to automatically enter bind mode if we get no signal
// from power-on
#define AUTOBIND_TIME (80 * 1000L * 1000L)

static void do_radio_waiting()
{
    uint32_t micros_now = get_micros();
    // Waiting for a signal from the tx
    // here we should check for received packet, etc.
    if (! radio_state.got_signal_ever) {
        // Automatically enter bind mode after a while.
        if (micros_now > AUTOBIND_TIME) {
            enter_bind_mode();
            return;
        }
    }
    if (rx_packet(false)) {
        radio_state.got_signal_ever = 1;
        if (process_sticks_packet()) {
            hop_next_channel();
            enter_hopping_mode();
            radio_state.last_packet_micros = micros_now;
        } else { 
           restart_rx();
        }
    }
}

// Packets are normally sent every 3.8 ms
// Multiply this by hop count.
#define MISSED_PACKET_MICROS (3900 * HOP_COUNT)

// If we miss so many packets, something is wrong
// and we will need to resynchronise.
#define MISSED_PACKET_MAX 8

static void do_radio_hopping()
{
    uint32_t now = get_micros();
    if (rx_packet(false)) {
        radio_state.got_signal_ever = 1;
        bool good = process_sticks_packet(false);
        // hop channel, if needed, then restart the rx 
        if (good) {
            hop_next_channel();
        } else {
            // start the rx again even if the received packet was bad.
            restart_rx();
        }
        if (good) {
            // diagnostic stuff
            uint8_t i = radio_state.hop_index;
            if (i == 0) {
                diag_puts("\r\n");
            }
            diag_puts("+");
            radio_state.last_packet_micros = now;
            radio_state.missed_packet_count = 0;
        }
    } else {
        // handle missed packet.
        uint32_t age = (now - radio_state.last_packet_micros);
        if (age > MISSED_PACKET_MICROS) {
            hop_next_channel();
            if (age > 15000) {
                diag_println("age=%lu", age);
                diag_println("now=%lu last=%lu", now, radio_state.last_packet_micros);
            }
            // Missed packet.
            radio_state.missed_packet_count += 1;
            // Remember that the last packet was sent about now, even though
            // we didn't receive it.
            radio_state.last_packet_micros = now;
            // Make a nice pattern.
            diag_puts("-");
            if (radio_state.hop_index == 0) {
                diag_puts("\r\n");
            }
            if (radio_state.missed_packet_count > MISSED_PACKET_MAX ) {
                diag_print("Lost too many packets.");
                enter_waiting_mode();
            }  
        }

    }
}

static void do_successful_bind(uint8_t *tx_id, uint8_t *hop_channels)
{
    diag_println("SUCCESSFUL BIND! SAVING TX ID!");
    diag_print("WINNER TX ID: "); print_packet(tx_id, 4);
    memcpy(radio_state.tx_id, tx_id, 4);
    memcpy(radio_state.hop_channels, hop_channels, NR_HOP_CHANNELS);
    diag_print("HOP CHANNELS: "); print_packet(radio_state.hop_channels, NR_HOP_CHANNELS);
    nvconfig_save();
    enter_waiting_mode();
}


static void do_radio_bind()
{
    uint32_t now = get_micros();
    // Blink led quickly.
    uint32_t flash_delta = (now - radio_state.flash_time);
    if (flash_delta > 0x10000) {
        set_led((now / 0x10000) % 2);  
    }
    // Work out which channel we should receive?
    // Flip between two channels every few packets received. This is not the same
    // As what the tx does, but it means we will possibly receive more if there is
    // interference or something.
    uint32_t ticks = (flash_delta / 32768) & 0xf; // counts 0..15 dec
    // Binding channels
    uint8_t nextchan = ( ticks & 0x4 ) ? 0x0c : 0x8b;
    // Also sometimes check the tx sticks channel (tx chan -1)
    if (ticks < 4) nextchan = radio_state.hop_channels[0] - 1;
    
    if (rx_packet(true)) {
        // Switch to next channel
        radio_state.packet_counter += 1;
        radio_state.last_packet_micros = now;
        start_rx(nextchan);
        // Check if it's interesting
        // Check for packet type bb - BIND1.
        uint8_t cmd = radio_state.packet[0];
        if ((cmd == PACKET_CMD_BIND1) || (cmd == PACKET_CMD_BIND2)) {
            radio_state.got_signal_ever = 1;
            uint8_t * bind_tx_id = (radio_state.packet + 1); 
            uint8_t * bind_hop_channels = (radio_state.packet + 11); 
            diag_print("bind: got tx id=");
            print_packet(bind_tx_id, 4);
            // find the correct slot in possible_tx_list
            uint8_t tx_index=42; // magic value
            for (uint8_t i=0; i< POSSIBLE_TX_COUNT; i+= 1) {
                if (memcmp(bind_tx_id, radio_state.possible_tx_list[i].tx_id, 4) == 0) {
                    // Correct id found.
                    tx_index = i;
                    break;
                }
                if (is_all_zeros(radio_state.possible_tx_list[i].tx_id, 4)) {
                    // Empty slot found.
                    tx_index = i;
                    memcpy(radio_state.possible_tx_list[i].tx_id, bind_tx_id, 4);
                    break;
                }
            }
            if (tx_index != 42) {
                uint16_t counter = radio_state.possible_tx_list[tx_index].count;
                counter += 1;
                radio_state.possible_tx_list[tx_index].count = counter;
                diag_println(" slot=%d c=%03d", (int) tx_index, (int) counter );
                // If we received enough packets for *this* transmitter, then it's the
                // winner.
                if (counter >= 30)
                {
                    do_successful_bind(bind_tx_id, bind_hop_channels);
                    return;
                }
            } else {
                // No free slots. Junk received?
            }
        }
        // Check for a sticks packet, from our already bound tx.
        if (cmd == PACKET_CMD_STICKS) {
            uint8_t * bind_tx_id = (radio_state.packet + 1); 
            if (memcmp(bind_tx_id, radio_state.tx_id, 4) == 0) {
                // Got a sticks packet while in bind mode?
                // we are re-binding the same tx, so let's do that.
                diag_println("OLD TX: GOT STICKS PACKET when in bind mode");
                diag_println("Bind not needed: exiting bind mode");
                radio_state.got_signal_ever = 1;
                enter_waiting_mode();
            }
        }
    } else {
        // Hey if we didn't receive anything for a while, switch channel anyway.
        uint32_t age = (now - radio_state.last_packet_micros);
        if (age > 20000) {
            start_rx(nextchan);
            // remember that we changed channel now
            radio_state.last_packet_micros = now;
            // diag_println("bchan %02x", (int) nextchan);
        }
    }
    // If we didn't receive anything, or have a timeout, stay on the same channel and
    // wait; it might be currently receiving something. Changing the channel or settings
    // too often might be counter-productive, as it will destroy a packet which is
    // in transit.
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
            do_radio_hopping();
            break;

    }
}

