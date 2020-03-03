#include <avr/io.h>
#include <avr/cpufunc.h>

/*
 * SPI three wires,
 * SCS (Chip select) - which is active LOW
 * SCK - clock - rising edge is active.
 * SDIO - bidirectional, contains the data.
 *
 */

/*
 * NOTE: We use some micro-optimisations in here, because it really
 * matters if we want to talk to the radio quickly.
 * 
 * Inlining functions and using VPORTS.
 */
// We use the VPORTs for more efficient operations, because the vport
// locations can use optimised instructions
VPORT_t * const SDIO_VPORT = &VPORTA;
VPORT_t * const SCS_VPORT = &VPORTB;

const uint8_t SCS_PIN = 3; // PB3
const uint8_t SDIO_PIN = 6; // PA6

#if NANOX_PINOUT
// Nanox design uses PB7 for SPI_SCK
VPORT_t * const SCK_VPORT = &VPORTB;
const uint8_t SCK_PIN = 7; // PB7
#else
// Other versions using PA7
VPORT_t * const SCK_VPORT = &VPORTA;
const uint8_t SCK_PIN = 7; // PA7
#endif

/*
 * Before sending a command to the device, we must set SCS low
 * for at least 50ns before the first clock pulse.
 *
 * Clock freq is 10mhz max (we are unlikely to reach that with
 * the cpu clock rate of 10mhz) 
 * When sending a command to the device, the bits are:
 * A7 (the first bit) - 1= strobe command - these are four-bit commands with no parameters.
 *      0= anything else.
 * A6 - 0=write, 1=read
 * A5 ... A0 - address to read/write
 * 
 * If reading, after 8 bits, we will set the sdio pin as an input, and
 * The next clock pulse will read the first bit, etc.
 *
 * If writing, after 8 bits we will then write more bits as necessary.
 *
 * When transaction is finished, we will raise SCS high (inactive)
 * and set sdio as input.
 *
 */

/* 
 * When sending a strobe command, we can send only 4 bits instead of 8,
 * A3...A0 can be omitted (they are ignored anyway)
 *
 */

// Set scs low, i.e. active.
__attribute__((always_inline)) inline static void scs_low()
{
    SCS_VPORT->OUT &= ~(1 << SCS_PIN);
}
// SCS high / inactive.
__attribute__((always_inline)) inline static void scs_high()
{
    SCS_VPORT->OUT |= 1 << SCS_PIN;
}
__attribute__((always_inline)) inline static void pulse_clock()
{
    SCK_VPORT->OUT |= 1 << SCK_PIN;
    _NOP();
    SCK_VPORT->OUT &= ~(1 << SCK_PIN);
}
__attribute__((always_inline)) inline static uint8_t pulse_clock_read_bit()
{
    SCK_VPORT->OUT |= 1 << SCK_PIN;
    uint8_t res = (SDIO_VPORT->IN & (1 << SDIO_PIN));
    SCK_VPORT->OUT &= ~(1 << SCK_PIN);
    return res;
}

__attribute__((always_inline)) inline static void send_byte(uint8_t data)
{
    // Send a single byte of stuff.
    // Does not touch SCS
    SDIO_VPORT->DIR |= 1 << SDIO_PIN ; // set the pin for write.
    for (uint8_t n=0; n <8; n++) {
        if (data & 0x80) {
            SDIO_VPORT->OUT |= 1 << SDIO_PIN ;
        } else {
            SDIO_VPORT->OUT &= ~(1 << SDIO_PIN) ;
        }
        // Move the data left
        data = data << 1;
        pulse_clock();
    }
    SDIO_VPORT->DIR &= ~(1 << SDIO_PIN) ; // clear to allow to read.
}

void spi_init() {
    // SCS must be set as an output and initially high
    SCS_VPORT->DIR |= 1 << SCS_PIN;
    SCS_VPORT->OUT |= 1 << SCS_PIN;
    // SCK must be set as an output and initially low
    SCK_VPORT->DIR |= 1 << SCK_PIN;
    // SDIO must be set as an input
    SDIO_VPORT->DIR &= ~(1 << SDIO_PIN);
}

void spi_write_byte(uint8_t addr, uint8_t data)
{
    uint8_t firstbyte = addr & 0x3f;
    scs_low();
    send_byte(firstbyte);
    send_byte(data);
    scs_high();
}

void spi_write_block(uint8_t addr, const uint8_t *buf, uint8_t datalen)
{
    uint8_t firstbyte = addr & 0x3f;
    scs_low();
    send_byte(firstbyte);
    for (uint8_t i=0; i < datalen; i++) {
        send_byte(buf[i]);
    }
    scs_high();
}

void spi_strobe(uint8_t cmd)
{
    uint8_t firstbyte = cmd; // must have top bit set.
    scs_low();
    send_byte(firstbyte);
    scs_high();
}

// Optimised: write a byte then send strobe command.
void spi_write_byte_then_strobe(uint8_t addr, uint8_t data, uint8_t cmd)
{
    uint8_t firstbyte = addr & 0x3f;
    scs_low();
    send_byte(firstbyte);
    send_byte(data);
    send_byte(cmd);
    scs_high();
}

// Two strobe commands without deasserting scs
void spi_strobe2(uint8_t cmd, uint8_t cmd2)
{
    scs_low();
    send_byte(cmd);
    send_byte(cmd2);
    scs_high();
}
void spi_strobe3(uint8_t cmd, uint8_t cmd2, uint8_t cmd3)
{
    scs_low();
    send_byte(cmd);
    send_byte(cmd2);
    send_byte(cmd3);
    scs_high();
}

uint8_t spi_read_byte(uint8_t addr) {
    // Read a single byte
    //
    uint8_t firstbyte = (addr & 0x3f) | 0x40; // Set read flag.
    uint8_t data = 0;
    scs_low();
    send_byte(firstbyte);
    for (uint8_t n=0; n<8; n++) {
        data = data << 1;
        if (pulse_clock_read_bit()) {
            data = data | 1; // High = 1 bit.
        }
    }
    scs_high();
    return data;
}

static void read_block1(uint8_t addr, uint8_t *buf, uint8_t datalen) {
    // Read block with scs low.
    // Read more than 1 byte
    uint8_t firstbyte = (addr & 0x3f) | 0x40; // Set read flag.
    send_byte(firstbyte);
    for (uint8_t i=0; i< datalen; i++) {
        uint8_t data = 0;
        for (uint8_t n=0; n<8; n++) {
            data = data << 1;
            if (pulse_clock_read_bit()) {
                data = data | 1; // High = 1 bit.
            }
        }
        buf[i] = data;
    }
}

void spi_read_block(uint8_t addr, uint8_t *buf, uint8_t datalen) {
    scs_low();
    read_block1(addr, buf, datalen);
    scs_high();
}

/*
 * This is typically used to read packet data, by sending a strobe
 * command to reset the read pointer, then read.
 */
void spi_strobe_then_read_block(uint8_t cmd,
    uint8_t addr, uint8_t *buf, uint8_t datalen) 
{
    scs_low();
    send_byte(cmd);
    read_block1(addr, buf, datalen);
    scs_high();
}
