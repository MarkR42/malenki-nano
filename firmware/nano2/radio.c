#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stdbool.h>

#include "a7105_spi.h"
#include "diag.h"
#include "radio.h"
#include "state.h"
#include "nvconfig.h"
#include "vsense.h"
#include "sticks.h"
#include "mixing.h"

#define F_CPU 10000000
#include <util/delay.h>

#include <string.h>

// Auto bind time in centiseconds.
#define AUTO_BIND_TIME (60 * 100)
// How often we reinitialise the radio after losing signal
#define REINIT_TIME (100)
// GIO port is on this pin
PORT_t * const GIO1_PORT = &PORTA;
const uint8_t GIO1_PIN = 2;
const uint8_t GIO1_bm = 1 << GIO1_PIN;

// Where is the LED?
VPORT_t * const LED_VPORT = &VPORTC;
const uint8_t LED_PIN = 5;
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
#ifdef ENABLE_DIAG
    // Dump regs
    for (uint8_t n=0; n<0x30; n++) {
        uint8_t b = spi_read_byte(n);
        diag_print("%02x ", (int) b);
        if ((n % 16) == 15) {
            diag_puts("\r\n");
        }
    }
    diag_puts("\r\n");
#endif // ENABLE_DIAG
}

//A7105 registers values -> ROM
// magic value 0xff means don't write.
static const unsigned char reg_init_values[] = {
    // NOTE: Registers 0xb and 0xc control GIO1 and GIO2, set them to 0 for now.
    0xFF, 0x42 , 0x00, 0x25, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x50,        // 00 - 0f
    0x9e, 0x4b, 0x00, 0x02, 0x16, 0x2b, 0x12, 0x4f, 0x62, 0x80, 0xFF, 0xFF, 0x2a, 0x32, 0xc3, 0x1f,                         // 10 - 1f
    0x1e, 0xff, 0x00, 0xFF, 0x00, 0x00, 0x3b, 0x00, 0x17, 0x47, 0x80, 0x03, 0x01, 0x45, 0x18, 0x00,                         // 20 - 2f
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
    uint16_t counter = 100;
    while (counter > 0) {
        uint8_t v = spi_read_byte(reg);
        if (! (v & bit)) {
            // Cleared.
            return;
        }
        -- counter;
        _delay_ms(1);
    }
    epic_fail("TIMEOUT initialising radio. Faulty crystal?");
}

static void init_interrupts()
{
    // TCB0 - we set this up to generate periodic interrupts at the timeout
    // period.
    // Messages are sent about every 3800 microseconds
    // Timeout period should be slightly more.
    
    // Timer will run at CLK_PER (10mhz) / 2
    // So 5mhz.
    const uint16_t timeout_ms = ((3800 * 5) / 4 );
    
    diag_println("Initialising interrupts");
    memset(&radio_counters, 0, sizeof(radio_counters));
    uint16_t compare_value = timeout_ms * 5 ; // Must not overflow 16-bit int.
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
#define STROBE_SLEEP 0x80 
#define STROBE_STANDBY 0xa0 
#define STROBE_PLL 0xb0 
#define STROBE_RX 0xc0 
#define STROBE_TX 0xd0 
#define STROBE_WRITE_PTR_RESET 0xe0 
#define STROBE_READ_PTR_RESET 0xf0 

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
    // Init send buffer with all 0xff
    // For sending telemetry - make sure the unused parts of the
    // telemetry packet are 0xff so they do not confuse the tx
    spi_strobe(STROBE_WRITE_PTR_RESET);
    for (uint8_t j=0; j<RADIO_PACKET_LEN; j++) {
        spi_write_byte(0x5, 0xff);
        // Also init telemetry packet in state
        radio_state.telemetry_packet[j] = 0xff;
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

static void radio_init_rx_id()
{
    // Real rx gives: 9c fb 96 41
    radio_state.rx_id[0] = 0x2a;
    // Mix the device serial number into the rx id.
    // serial number is 10 bytes.
    uint8_t * serialnum = (uint8_t *) & (SIGROW.SERNUM0);
    for (uint8_t i=0; i<10; i++) {
        radio_state.rx_id[i % 4] ^= serialnum[i];
    }
#ifdef ENABLE_DIAG
    diag_puts("rx_id=");
    dump_buf(radio_state.rx_id, 4);
#endif
}

void radio_init()
{
    diag_println("Begin radio_init");
    radio_init_rx_id();
    init_a7105_hardware();

    /* Determine whether we have any tx info loaded from nvram.
     */
    // If there are no data loaded, tx_id will be all zeros
    if (is_all_zeros(radio_state.tx_id, sizeof(radio_state.tx_id))) {
        init_bind_mode();
    } else {
        diag_println("initialising from saved tx id: ");
        dump_buf(radio_state.tx_id,  4);
        radio_state.state = RADIO_STATE_HOPPING;
        // Save the hopping channels in case we need them later.
        memcpy(radio_state.hop_channels_saved, radio_state.hop_channels,
            NR_HOP_CHANNELS);
        // Save previous tx id.
        memcpy(radio_state.tx_id_saved, radio_state.tx_id,
            sizeof(radio_state.tx_id_saved));
        diag_println("Hop channels: ");
        dump_buf(radio_state.hop_channels, NR_HOP_CHANNELS);
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

/*
 * Called to reinitialise the radio hardware, but not
 * clear all state etc.
 */
static void radio_reinit()
{
    // Interrupts will be enabled at this point, and might arrive,
    // so we should disable them during the reinitialisation.
    // Note also that disabling interrupts breaks the wait_auto_clear function,
    // But that's ok, because the watchdog will reset if the reinit
    // takes too long.
    cli(); 
    
    // If this fails because the radio is broken, it will call
    // epic_fail which will either reset or the watchdog will reset.
    init_a7105_hardware();
    init_interrupts();
    /*
     * This will also reinitialise the timer, so a timeout
     * interrupt will arrive very soon and cause the receiver to
     * start on an appropriate channel- we don't need to
     * explicitly start the receiver. 
     */
    sei();
}

static void enable_rx();

static uint8_t test1_channel = 0x8c;// bind channel 0x0d or 0x8c

__attribute__ ((unused)) static void radio_test1()
{
    diag_println("radio_test1: this must not run with interrupts doing rx");
    radio_state.current_channel = test1_channel;
    enable_rx(); 
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

#define PACKET_TYPE_BIND1 0xbb
#define PACKET_TYPE_BIND2 0xbc
#define PACKET_TYPE_TELEMETRY 0xaa

// Send telemetry an undivisible number of packets,
// So that the telemetry does not always appear on one hopping
// channel, it moves around them all.
#define TELEMETRY_INTERVAL_PACKETS 41

#define AFHDS2A_SENSOR_RX_VOLTAGE 0x00
#define AFHDS2A_SENSOR_RX_ERR_RATE 0xfe
#define AFHDS2A_SENSOR_RX_RSSI 0xfc

static void prepare_telemetry(bool is_config_mode, uint8_t rpm_value)
{
    uint8_t *p = radio_state.telemetry_packet;
    p[0] = PACKET_TYPE_TELEMETRY;
    memcpy(p+1, radio_state.tx_id, 4);
    memcpy(p+5, radio_state.rx_id, 4);
    uint8_t n=9;
    bool sense_ok = true;

    // Temperature telemetry is removed.

    if (vsense_state.voltage_mv > 100) {
        // Only send voltage telemetry if it is sensible.
        p[n++] = 0x0; // telemetry type (0=volts)
        // Some transmitters (FS i6) assume that "internal" voltage
        // is always a 2S lipo (6.4-8.4 volts) 
        // Because the official receivers for FS do not support 1S
        // packs. 
        // So we will set the voltage as "external" if we have a
        // 1-cell pack.
        // Otherwise the fs i6 will be constantly beeping low battery if we run off 1S.
        // It appears that the voltage alarm is not configurable
        // for the internal sensor, and also cannot be disabled
        // (which is annoying!)
        uint8_t voltage_id = 0; // "Internal sensor"
        if (vsense_state.cells_count == 1) {
            // 1 cell pack, use "external sensor"
            voltage_id = 2;
        }
        p[n++] = voltage_id; // telemetry id (0=internal 1 or more=external)
        uint16_t volts_100 = vsense_state.voltage_mv / 10;
        p[n++] = (volts_100 & 0xff); // low
        p[n++] = (volts_100 >> 8);// high
        if (vsense_state.voltage_mv < 1000) {
            // Voltage sensor is broken, or not initialised yet
            // Do not send telemetry with an invalid value, it
            // causes the tx to make all sorts of beeps and flash leds.
            sense_ok = false;
        }
    }
    // Sensors to keep opentx happy.
    p[n++] = AFHDS2A_SENSOR_RX_ERR_RATE;
    p[n++] = 0; // Instance ID
    p[n++] = radio_state.e_error_rate; p[n++] = 0;
    
    p[n++] = AFHDS2A_SENSOR_RX_RSSI;
    p[n++] = 0; // Instance ID
    p[n++] = 0; p[n++] = 0;
    
    // Special sensor for "rpm" we use to send data back to the tx.
    if (is_config_mode) // Only send in config mode.
    {
        p[n++] = 0x2; // "RPM"
        p[n++] = 1; // channel number, instance number?
        p[n++] = rpm_value;
        p[n++] = 0;
    }
    
    p[n++] = 0xff; // end
    radio_state.telemetry_packet_is_valid = sense_ok;
}

static void handle_packet_sticks()
{
    radio_state.sticks_packet_count += 1;
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
    sticks_result_t sticks_result = sticks_receive_positions(sticks);
    // Turn on the LED so the driver can see it's connected.
    // Flicker the LED if we are in config mode, solid if driving mode.
    if (sticks_result.config_mode) {
        // Flicker a mostly on duty cycle.
        radio_state.led_on = sticks_result.led_state;
    } else {
        // Driving mode.
        radio_state.led_on = true;
    }
    
    if (!radio_state.got_signal) {
        // Re-established signal
        radio_state.e_packet_count = 0;
        radio_state.e_good_count   = 0;
        radio_state.e_error_rate   = 0;
    }
    
    radio_state.got_signal = true;
    // Send telemetry from time to time.
    if (radio_state.telemetry_countdown == 0) {
        prepare_telemetry(sticks_result.config_mode, sticks_result.rpm_value);
        radio_state.telemetry_countdown = TELEMETRY_INTERVAL_PACKETS;
    } else {
        radio_state.telemetry_countdown --;
    }
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
    memcpy(radio_state.hop_channels, radio_state.hop_channels_new,
        NR_HOP_CHANNELS);
    diag_print("hop: ");
    dump_buf(radio_state.hop_channels, NR_HOP_CHANNELS);
    radio_state.hop_index = 0;
    radio_state.state = RADIO_STATE_HOPPING;
    // Save the transmitter id, etc, in nvram.
    nvconfig_save();
}

static void handle_packet_bind(uint32_t now)
{
    // packet is in radio_state.packet.
    uint8_t *bind_tx_id = radio_state.packet + 1;
    // Check for a packet from our *old* saved transmitter, 
    // This is a normal situation and we should immediately resume operation.
    if (memcmp(bind_tx_id, radio_state.tx_id_saved, 4) == 0) {
        // Old transmitter came back!
        diag_println("Old transmitter reappeared, will use old settings");
        memcpy(radio_state.tx_id, radio_state.tx_id_saved, 
            sizeof(radio_state.tx_id));
        memcpy(radio_state.hop_channels, radio_state.hop_channels_saved, 
            sizeof(radio_state.hop_channels));
        radio_state.hop_index = 0;
        radio_state.state = RADIO_STATE_HOPPING;
    }
    // Check if a lot of repeated bind packets appear.
    // (This is for non-telemetry transmitters)
    if (memcmp(bind_tx_id, radio_state.tx_id, 4) == 0) {
        // Repeated bind packet.
        radio_state.bind_packet_count += 1;
        if (radio_state.bind_packet_count >= 100) {
            // Excellent, done.
            handle_bind_complete();
            return;
        }

        // For telemetry transmitters, which heard our responses,
        // Check for a "BIND2" type
        // Which should contain our rx id.
        if (radio_state.packet[9] == 2) {
            if (memcmp(radio_state.packet + 5, radio_state.rx_id, 4) == 0) {
                // Yeah!
                diag_println("Bind2 detected");
                // handle_bind_complete();
                radio_state.bind_complete_time = now + 50;
            }
        }

    } else {
        // Make sure it really is a bind packet.
        uint8_t t = radio_state.packet[0];
        if ((t == PACKET_TYPE_BIND1) || (t == PACKET_TYPE_BIND2)) {
            // New bind packet
            diag_println("New tx detected");
            radio_state.bind_packet_count = 0;
            memcpy(radio_state.tx_id, bind_tx_id, 4); // store id.
            memcpy(radio_state.hop_channels_new, radio_state.packet + 11, NR_HOP_CHANNELS);
        }
    }
    
}

static void radio_show_diagnostics(uint32_t now)
{
    // Do this every time the hop index resets, e.g.
    // once every 32 packets or missed packets.
    uint8_t temp_hop_index = radio_state.hop_index;
    if (temp_hop_index < radio_state.old_hop_index) {
        radio_counters.last_report_time = now;
        cli();
        uint16_t rx = radio_counters.rx;
        uint16_t missed = radio_counters.missed;
        radio_counters.rx = 0;
        radio_counters.missed = 0;
        sei();
        diag_println("rx: %05u missed: %05u", rx, missed);
        if (radio_state.state == RADIO_STATE_BIND) {
            // Blink led in bind mode.
            radio_state.led_on = ! radio_state.led_on;
        }
    }
    radio_state.old_hop_index = temp_hop_index;
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
            handle_packet_bind(now);
        } else {
            handle_packet_sticks();
            radio_state.last_sticks_packet = now;
        }
        radio_state.got_signal_ever = true;
        radio_state.packet_is_valid = false;
    } else {
        if (radio_state.state == RADIO_STATE_BIND) {
            if ((radio_state.bind_complete_time != 0) && (radio_state.bind_complete_time < now)) {
                handle_bind_complete();
            }
        } 
        else 
        {
            // radio_state.state != RADIO_STATE_BIND
            uint32_t age = now - radio_state.last_sticks_packet;
            if (age > 25) { // centiseconds
                // No signal.
                sticks_no_signal();
                radio_state.led_on = false;
                radio_state.got_signal = false;
                // Auto-rebind:
                // if we have got no signal since power on, then
                // automatically enter bind mode after some time.
                if ((! radio_state.got_signal_ever) && (now > AUTO_BIND_TIME)) {
                    diag_println("Auto-bind mode");
                    init_bind_mode();
                }
                if (radio_state.got_signal_ever) {
                    // If we lost signal, periodically reinitialise the radio.
                    if (radio_state.last_reinit_time == 0) {
                        radio_state.last_reinit_time = radio_state.last_sticks_packet;
                    }
                    uint32_t reinit_age = now - radio_state.last_reinit_time;
                    if (reinit_age > REINIT_TIME) {
                        radio_reinit();
                        radio_state.last_reinit_time = now;
                    }
                }
            }
        }
    }
    radio_show_diagnostics(now);
}

static void init_bind_mode()
{
    diag_println("initialising bind mode");
    radio_state.state = RADIO_STATE_BIND;
    // Set up the hop-table with the bind channels.
    for (uint8_t i=0; i< NR_HOP_CHANNELS; i++) {
        radio_state.hop_channels[i] = (i & 2) ? 0x0d : 0x8c;
    }
#ifdef ENABLE_DIAG
    diag_puts("Bind mode hopping pattern: ");
    dump_buf(radio_state.hop_channels, NR_HOP_CHANNELS);
    diag_println("tx_id_saved: ");
    dump_buf(radio_state.tx_id_saved, 4);
#endif
}

void radio_shutdown()
{
    // Shut down the radio and sleep.
    // after this, we need a full reset
    // Turn off our interrupts.
    cli();// Make sure no more interrupts arrive.
    TCB0.INTCTRL = 0; // Turn off irq
    PORTA.PIN2CTRL = 0; // Turn off irq from there.
    TCB0.CTRLA = 0; // Shut down timer
    spi_strobe(STROBE_STANDBY);
    _delay_ms(10);
    // Get the radio into low power mode.
    spi_strobe(STROBE_SLEEP);
    // There is a possibility that some interrupts are still
    // pending, but it should be harmless?    
    // If the radio is in sleep mode, we will read rubbish from
    // its rx buffer.
}


/***************************************************
 * IRQ CODE BELOW HERE!
 * Here be dragons, woof woof
 **********************/
 
static void maybe_diag_putc(char c)
{
    // Write character to tx for diag, only if enabled.
#if 1
    USART0.TXDATAL = c;
#endif
}

static void enable_rx()
{
    /*
     * If for some reason, we were transmitting telemetry,
     * it will stop (and generate a bad packet)
     */
    uint8_t chanminus1 = radio_state.current_channel - 1 ;
    spi_write_byte_then_strobe(0x0f, chanminus1, STROBE_RX);
    radio_state.tx_active = false; // Ensure this flag is not left set
}

static void do_tx(uint8_t channel)
{
    // Transmit packet in radio_state.telemetry_packet
    spi_strobe(STROBE_STANDBY);
    // Write data
    spi_strobe_then_write_block(STROBE_WRITE_PTR_RESET,
        0x5, radio_state.telemetry_packet, RADIO_TELEMETRY_SIGNIFICANT_LEN);
    // Set tx channel
    spi_write_byte(0x0f, channel);
    spi_strobe(STROBE_TX);
    radio_state.tx_active = true;
    radio_state.telemetry_packet_is_valid = false; // consumed.
    maybe_diag_putc('t');
}

static void do_tx_complete()
{
    maybe_diag_putc('T');
    // reactivate the receiver 
    enable_rx();
}

static void hop_to_next_channel(uint8_t channel_increment)
{
    // hop_index should count up to 31,
    radio_state.hop_index = (radio_state.hop_index + channel_increment) & 0x1f;
    // but chan_index only count to 15
    uint8_t chan_index = radio_state.hop_index & 0xf;
    uint8_t bind_seek_old = radio_state.hop_index & 0x10;
    uint8_t chan = radio_state.hop_channels[chan_index];
    // In bind mode, we will switch to a different channel half the time.
    if ((radio_state.state == RADIO_STATE_BIND) && bind_seek_old) {
        // Channel from the old transmitter.
        chan = radio_state.hop_channels_saved[0];
    }
    radio_state.current_channel = chan;
}


static void update_led()
{
    if (radio_state.led_on) {
        LED_VPORT->OUT |= LED_PIN_bm; // turn on microcontroller gpio
    } else {
        LED_VPORT->OUT &= ~LED_PIN_bm; // turn off micro gpio
    }
} 

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
        radio_counters.missed += 1;
        if ((radio_state.missed_packet_count < 5) || (radio_state.state==RADIO_STATE_BIND)) {
            // Try to hop to catch up.
            // Always hop in bind mode.
            hop_to_next_channel(1);
            radio_state.missed_packet_count += 1; // successive missed packets.
        } else {
            // Too out of sync, stay on same channel and wait for
            // it to come around again.
            hop_to_next_channel(0);
        }
        enable_rx();    
        maybe_diag_putc('.');
        update_led();
        master_state.radio_interrupt_ok = 1; // for watchdog
        ++ radio_state.e_packet_count; // count mised packets as errors.
    }
}

static void reset_timeout()
{
    // Reset the counter AND clear the irq flag
    // So that a pending interrupt won't trigger until the timeout
    // expires again
    TCB0.CNT = 0;
    TCB0.INTFLAGS |= TCB_CAPT_bm;
}   

static void irq_prepare_bind_response(uint8_t *buf)
{
    // Send a response to a bind packet.
    // Example stage 0/1 bind packet:
    // BC 44 2A 37 77 FF FF FF FF 00 00 
    // Example stage 1 response: BC 44 2A 37 77 25 31 91 41 01 00 FF
    // Example stage 2 bind packet:
    // BC 44 2A 37 77 25 31 91 41 02 00
    // Example stage 2: BC 44 2A 37 77 25 31 91 41 02 00 FF
    uint8_t b9 = buf[9];
    uint8_t response = 0;
    // Check we actually have the tx id saved.
    // Do not send responses too soon.
    if (memcmp(buf+1, radio_state.tx_id, 4) != 0) {
        // Not saved yet.
        return;
    }
    if (b9 == 0) {
        // Send stage 1 response
        response = 1; 
    }
    if ((b9 == 2) && (memcmp(buf+5, radio_state.rx_id, 4) == 0)) {
        // Send stage 2 response.
        response = 2;
    }
    // Build this packet as quickly as we can.
    if (response) {
        // First 5 bytes are identical to the request.
        memcpy(radio_state.telemetry_packet, buf, 5);
        // Add our rx id.
        memcpy(radio_state.telemetry_packet+5, radio_state.rx_id, 4);
        // Then the "response" byte
        radio_state.telemetry_packet[9] = response;
        radio_state.telemetry_packet[10] = 0;
        // then some ff
        radio_state.telemetry_packet[11] = 0xff;
        radio_state.telemetry_packet[12] = 0xff;
        
        radio_state.telemetry_packet_is_valid = true;
    }
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
        // If it has our tx_id, it's always important.
        // Packet type is checked in main receive function
        if (memcmp(& (buf[1]), radio_state.tx_id, 4) == 0) {
            ok = true;
            do_hop = true;
        }
        
        // If we are in bind mode, bind packets are important, 
        // But so are sticks packets.
        if (state == RADIO_STATE_BIND) {
            if ((packet_type == PACKET_TYPE_BIND1) || (packet_type == PACKET_TYPE_BIND1) || 
                (packet_type == PACKET_TYPE_STICKS)) {
                // Bind packet.
                ok = true;
                do_hop = true;
            }
        }
        // Do this *in* the interrupt so we can send a response asap.
        if ((state == RADIO_STATE_BIND) && (packet_type == PACKET_TYPE_BIND2)) {
            irq_prepare_bind_response(buf);
        }
        if (!ok) maybe_diag_putc('0');
    }
    uint8_t rx_channel = radio_state.current_channel;
    if (do_hop) {
        // GOOD packet.
        hop_to_next_channel(1);
        radio_counters.rx += 1;
        reset_timeout();         
    } else {
        // BAD packet; stay on the same channel, do not reset timer.
        hop_to_next_channel(0);
    }
    // If we have any telemetry to send, do it now.
    if (radio_state.telemetry_packet_is_valid) {
        do_tx(rx_channel);
    } else {
        enable_rx();
    }
    
    if (ok) {
        // If radio_state.packet does not already contain a packet,
        // then copy the packet there, otherwise, drop packet.
        if (! radio_state.packet_is_valid) {
            // Copy packet into buffer for the main loop.
            memcpy(radio_state.packet, buf, RADIO_PACKET_SIGNIFICANT_LEN);
            radio_state.packet_is_valid = true;
            radio_state.packet_channel = rx_channel;
        } else {
            // Drop the packet contents, but still hop to next channel
        }
        radio_state.missed_packet_count = 0; // successive missed packets.
        maybe_diag_putc('1');
        ++ radio_state.e_good_count;
    }
    
    update_led();
    master_state.radio_interrupt_ok = 1; // for watchdog
    // Measure error rate for telemetry
    ++ radio_state.e_packet_count;
    if (radio_state.e_packet_count >= 100) {
        // Store error rate.
        radio_state.e_error_rate = 100 - radio_state.e_good_count;
        // Restart counters.
        radio_state.e_packet_count = 0;
        radio_state.e_good_count = 0;
    }
}

ISR(PORTA_PORT_vect)
{
    // Either RX complete, or TX complete.
    
    // Check the flag on the port - if it is low, then check for reset.
    // pin could legitimately be high, if the interrupt arrived while
    // we were processing a timeout interrupt, in which case, it's
    // too late and we missed it.
    uint8_t bit = PORTA.IN & GIO1_bm;
    if (! bit) {
        if (radio_state.tx_active) {
            do_tx_complete();
        } else 
        {
            // RX packet.
            do_rx();
        }
    }
    // clear the irq
    PORTA.INTFLAGS = GIO1_bm;
}
