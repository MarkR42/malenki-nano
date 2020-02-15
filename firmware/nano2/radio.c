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

// Number of hops we do each time.
#define HOP_COUNT 1

// Auto bind time in centiseconds.
#define AUTO_BIND_TIME (60 * 100)

// GIO port is on this pin
PORT_t * const GIO1_PORT = &PORTA;
const uint8_t GIO1_PIN = 2;
const uint8_t GIO1_bm = 1 << GIO1_PIN;

// Where is the LED?
VPORT_t * const LED_VPORT = &VPORTC;
const uint8_t LED_PIN = 1;
const uint8_t LED_PIN_bm = 1 << LED_PIN;

radio_state_t radio_state;
static struct {
    uint16_t rx; // rx packets
    uint16_t missed; // missed
    uint32_t last_report_time;
} radio_counters;

static void init_bind_mode();
static void dump_buf(uint8_t *buf, uint8_t len);

/*
 * Blinking LED goes on this GPIO pin.
 * Note: on attiny1614, this pin does not exist, but it's ok.
 * We will blink it on a chip where it does exist.
 */
PORT_t * const BLINKY_PORT = &PORTC;
const uint8_t BLINKY_bm = 5 << 4;

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
    // Messages are sent about every 3800 microseconds
    // Timeout period should be slightly more.
    
    // Timer will run at CLK_PER (10mhz) / 2
    // So 5mhz.
    
    diag_println("Initialising interrupts");
    memset(&radio_counters, 0, sizeof(radio_counters));
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

static bool is_all_zeros(const uint8_t *buf, uint8_t len)
{
    uint8_t acc = 0;
    for (uint8_t i=0; i<len; i++) {
        acc |= buf[i];
    }
    return (acc == 0);
}

void radio_init()
{
    diag_println("Begin radio_init");
    init_a7105_hardware();

    /* Determine whether we have any tx info loaded from nvram.
     */
    // If there are no data loaded, tx_id will be all zeros
    if (is_all_zeros(radio_state.tx_id, sizeof(radio_state.tx_id))) {
        init_bind_mode();
    } else {
        diag_println("initialising from saved tx id: ");
        dump_buf(radio_state.tx_id, 4);
        radio_state.state = RADIO_STATE_HOPPING;
    }
    // Init led BLINKY gpio - set it as output.
    LED_VPORT->DIR |= LED_PIN_bm;
    
    init_interrupts();
    diag_println("End radio_init");
    /*
     * Note: we will not start the rx here,
     * we just wait for the radio timeout, which will restart the
     * rx anyway.
     */
}

static void enable_rx(uint8_t channel);
static uint8_t test1_channel = 0x8c;// bind channel 0x0d or 0x8c

__attribute__ ((unused)) static void radio_test1()
{
    diag_println("radio_test1: this must not run with interrupts doing rx");
    enable_rx(test1_channel); 
    diag_println("Waiting for packet on chan %02x", (int) test1_channel);
    uint32_t timeout = get_tickcount() + 100;
    while (GIO1_PORT->IN & GIO1_bm) {
        // Wait for GIO1 to go low.
        if (get_tickcount() > timeout) {
            diag_println("No packet received, timeout");
            return;
        }
    }
    diag_println("got packet");
    // Load pack into buf
    uint8_t buf[RADIO_PACKET_LEN];
    spi_strobe_then_read_block(STROBE_READ_PTR_RESET,
        0x5, // address to read packet data.
        buf, RADIO_PACKET_LEN);
    // dump
    for (uint8_t i=0; i< RADIO_PACKET_LEN; i++) {
        diag_print("%02x ", (int) buf[i]);
    }
    diag_puts("\r\n");
    
    if (buf[0] == 0xbb) {
        // Bind packet
        test1_channel = buf[11]; // Hopping channel from bind
    }
    diag_println("radio_test1 finished");
}

#define PACKET_TYPE_STICKS 0x58

static void handle_packet_sticks()
{
    uint8_t type = radio_state.packet[0];
    if (type != PACKET_TYPE_STICKS) {
        // Ignore
        return;
    }
    // packet is in radio_state.packet.
    // diag_println("stixchan: %02x", (int) radio_state.packet_channel);
    const uint8_t sticks_offset = 9; // Bytes offset
    uint16_t sticks[NUM_CONTROL_CHANNELS]; 
    for (uint8_t n=0; n< NUM_CONTROL_CHANNELS; n++) {
        uint16_t *stick = (uint16_t *) (radio_state.packet + sticks_offset + (2*n));
        sticks[n] = *stick;
    }
    // Centre is *always* 1500.
    // Convert to signed.
    uint16_t rel_steering = (int16_t) sticks[CHANNEL_INDEX_STEERING] - 1500;
    uint16_t rel_throttle = (int16_t) sticks[CHANNEL_INDEX_THROTTLE] - 1500;
    uint16_t rel_weapon = (int16_t) sticks[CHANNEL_INDEX_WEAPON] - 1500;
    bool invert = false; // TODO
    mixing_drive_motors(rel_throttle, rel_steering, rel_weapon, invert);
    // Turn on the LED so the driver can see it's connected.
    radio_state.led_on = true;
}

static void dump_buf(uint8_t *buf, uint8_t len)
{
    for (uint8_t i=0; i<len; i++) {
        diag_print("%02x ", (int) buf[i]);
    }
    diag_puts("\r\n");
}

static void handle_bind_complete()
{
    diag_println("Bind completed.");
    diag_print("tx id: ");
    dump_buf(radio_state.tx_id, 4);
    // copy the list of hopping channels.
    memcpy(radio_state.hop_channels, radio_state.packet + 11,
        NR_HOP_CHANNELS);
    diag_print("hop: ");
    dump_buf(radio_state.hop_channels, NR_HOP_CHANNELS);
    radio_state.hop_index = 0;
    radio_state.state = RADIO_STATE_HOPPING;
    // Save the transmitter id, etc, in nvram.
    nvconfig_save();
}

static void handle_packet_bind()
{
    // packet is in radio_state.packet.
    uint8_t *bind_tx_id = radio_state.packet + 1;
    // Check if a lot of repeated bind packets appear.
    if (memcmp(bind_tx_id, radio_state.tx_id, 4) == 0) {
        // Repeated bind packet.
        radio_state.bind_packet_count += 1;
        if (radio_state.bind_packet_count >= 100) {
            // Excellent, done.
            handle_bind_complete();
        }
    } else {
        // New bind packet
        diag_println("New tx detected");
        radio_state.bind_packet_count = 0;
        memcpy(radio_state.tx_id, bind_tx_id, 4); // store id.
    }
}

// BIND MODE: uses channels
// 0c and 8b
// Transmitter transmits one channel number higher?
// So we need to subtract one from every channel no?
void radio_loop()
{
    uint32_t now = get_tickcount();
    if (radio_state.packet_is_valid) {
        if (radio_state.state == RADIO_STATE_BIND) {
            handle_packet_bind();
        } else {
            handle_packet_sticks();
            radio_state.last_sticks_packet = now;
        }
        radio_state.got_signal_ever = true;
        radio_state.packet_is_valid = false;
    } else {
        if (radio_state.state != RADIO_STATE_BIND) {
            uint32_t age = now - radio_state.last_sticks_packet;
            if (age > 25) { // centiseconds
                // Lost signal.
                diag_println("No signal");
                motors_all_off();
                radio_state.led_on = false;
                radio_state.last_sticks_packet = now; // avoid spamming debug port
                // Auto-rebind:
                // if we have got no signal since power on, then
                // automatically enter bind mode after some time.
                if ((! radio_state.got_signal_ever) && (now > AUTO_BIND_TIME)) {
                    diag_println("Auto-bind mode");
                    init_bind_mode();
                }
            }
        }
    }
    if ((now - radio_counters.last_report_time) > 50) {
        if (radio_state.hop_index == 0) {
            radio_counters.last_report_time = now;
            cli();
            uint16_t rx = radio_counters.rx;
            uint16_t missed = radio_counters.missed;
            radio_counters.rx = 0;
            radio_counters.missed = 0;
            sei();
            diag_println("rx: %05u missed: %05u", rx, missed);
        }
        if (radio_state.state == RADIO_STATE_BIND) {
            // Blink led in bind mode.
            radio_state.led_on = (now & 0x10);
        }
    }
}

static void init_bind_mode()
{
    diag_println("initialising bind mode");
    radio_state.state = RADIO_STATE_BIND;
    // Set up the hop-table with the bind channels.
    for (uint8_t i=0; i< NR_HOP_CHANNELS; i++) {
        radio_state.hop_channels[i] = (i & 2) ? 0x0d : 0x8c;
    }
}

/***************************************************
 * IRQ CODE BELOW HERE!
 * Here be dragons, woof woof
 **********************/
 
static void maybe_diag_putc(char c)
{
    // Write character to tx for diag, only if enabled.
#if 0
    USART0.TXDATAL = c;
#endif
}

static void enable_rx(uint8_t channel)
{
    uint8_t chanminus1 = channel - 1 ;
    spi_write_byte_then_strobe(0x0f, chanminus1, STROBE_RX);
    radio_state.current_channel = channel;
}
 
static void restart_rx(uint8_t channel_increment)
{
    radio_state.hop_index = (radio_state.hop_index + channel_increment) & 0xf;
    uint8_t chan = radio_state.hop_channels[radio_state.hop_index];
    enable_rx(chan);
}

static void update_led()
{
    // Called in interrupts, update the "BLINKY" led with the state of
    // led_on
    uint8_t gio_control =
        ((0x0c) << 2 )  // GIO2 pin control: "Inhibited"
        | 1; // Enable
    if (radio_state.led_on) {
        gio_control |= 0x2; // Invert signal bit for gio
        LED_VPORT->OUT |= LED_PIN_bm; // turn on microcontroller gpio
    } else {
        LED_VPORT->OUT &= ~LED_PIN_bm; // turn off micro gpio
    }
    const uint8_t gio2_register = 0xc; 
    spi_write_byte(gio2_register, gio_control);
} 

static uint8_t timeout_irq_count;

ISR(TCB0_INT_vect)
{
    // Periodic interrupt:
    // Handle rx timeout channel hopping.
    // The timer should have its CNT reset to 0 every time we get a
    // real packet, to suppress the timeout
    // Check the INTFLAGS, because it might have been cancelled after
    // the irq was generated (if do_rx was in progress during timeout)
    if (TCB0.INTFLAGS & TCB_CAPT_bm) {
        TCB0.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
        timeout_irq_count += 1;
        if (timeout_irq_count == HOP_COUNT) {
            radio_counters.missed += 1;
            if (radio_state.missed_packet_count < 5) {
                // Try to hop to catch up.
                restart_rx(HOP_COUNT);
                radio_state.missed_packet_count += 1; // successive missed packets.
            } else {
                // Too out of sync, stay on same channel and wait for
                // it to come around again.
                restart_rx(0);
            }
            maybe_diag_putc('.');
            timeout_irq_count = 0;
            update_led();
        }
    }
}

static void reset_timeout()
{
    // Reset the counter AND clear the irq flag
    // So that a pending interrupt won't trigger until the timeout
    // expires again
    TCB0.CNT = 0;
    TCB0.INTFLAGS |= TCB_CAPT_bm;
    timeout_irq_count = 0;
}


static void do_rx()
{
    // We received something, check what it is,
    // is it a packet we care about?
    uint8_t buf[RADIO_PACKET_LEN];
    // Get the flags
    uint8_t modeflags = spi_read_byte(0);
    // I think flag 0x01 (bit 0) is the "read data ready" flag, active low,doc is not clear about this.
    // bits 5 and 6 are read error flags.
    uint8_t errflags = (1 << 6) | (1 << 5);

    // Check packet type.
    uint8_t packet_type = buf[0];
    uint8_t state = radio_state.state;
    bool ok = false;
    bool do_hop = false;
    if ((modeflags & errflags)) {
        maybe_diag_putc('e');
        do_hop = true; // Assume that the error packet was for us.
    } else {
        spi_strobe_then_read_block(STROBE_READ_PTR_RESET,
            0x5, // address to read packet data.
            buf, RADIO_PACKET_SIGNIFICANT_LEN);
        if (state == RADIO_STATE_BIND) {
            if ((packet_type == 0xbb) || (packet_type == 0xbc)) {
                // Bind packet.
                ok = true;
                do_hop = true;
            }
        }
        if (state == RADIO_STATE_HOPPING) {
            // sticks, settings, failsafe? or something else.
            // Check tx id.
            if (memcmp(& (buf[1]), radio_state.tx_id, 4) == 0) {
                ok = true;
                do_hop = true;
            }
        }
        if (!ok) maybe_diag_putc('0');
    }
    if (do_hop) {
        // GOOD packet.
        restart_rx(HOP_COUNT);
        radio_counters.rx += 1;
        reset_timeout();         
    } else {
        // BAD packet; stay on the same channel, do not reset timer.
        restart_rx(0);
    }
    
    if (ok) {
        // If radio_state.packet does not already contain a packet,
        // then copy the packet there, otherwise, drop packet.
        if (! radio_state.packet_is_valid) {
            // Copy packet into buffer for the main loop.
            memcpy(radio_state.packet, buf, RADIO_PACKET_SIGNIFICANT_LEN);
            radio_state.packet_is_valid = true;
            radio_state.packet_channel = radio_state.current_channel;
        } else {
            // Drop the packet contents, but still hop to next channel
        }
        radio_state.missed_packet_count = 0; // successive missed packets.
        maybe_diag_putc('1');
    }
    update_led();
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
        do_rx();
    }
    // clear the irq
    PORTA.INTFLAGS = GIO1_bm;
}
