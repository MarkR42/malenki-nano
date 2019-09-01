#include <avr/io.h>
#include <avr/cpufunc.h>

/*
 * SPI three wires,
 * SCS (Chip select) - which is active LOW
 * SCK - clock - rising edge is active.
 * SDIO - bidirectional, contains the data.
 *
 */

PORT_t * const SCS_PORT = &PORTB;
PORT_t * const SCK_PORT = &PORTA;
PORT_t * const SDIO_PORT = &PORTA;

const uint8_t SCS_PIN = 3; // PB3
const uint8_t SCK_PIN = 7; // PA7
const uint8_t SDIO_PIN = 6; // PA6
/*
 * Before sending a command to the device, we must set SCS low
 * for at least 50ns before the first clock pulse.
 *
 * Clock freq is 10mhz max (we are unlikely to reach that with
 * the default cpu clock rate of 3.3mhz) 
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
static void scs_low()
{
    SCS_PORT->OUTCLR = 1 << SCS_PIN;
}
// SCS high / inactive.
static void scs_high()
{
    SCS_PORT->OUTSET = 1 << SCS_PIN;
}
static void pulse_clock()
{
    SCK_PORT->OUTSET = 1 << SCK_PIN;
    _NOP();
    SCK_PORT->OUTCLR = 1 << SCK_PIN;
}

static void send_byte(uint8_t data)
{
    // Send a single byte of stuff.
    // Does not touch SCS
    SDIO_PORT->DIRSET = 1 << SDIO_PIN ; // set the pin for write.
    for (uint8_t n=0; n <8; n++) {
        if (data & 0x80) {
            SDIO_PORT->OUTSET = 1 << SDIO_PIN ;
        } else {
            SDIO_PORT->OUTCLR = 1 << SDIO_PIN ;
        }
        // Move the data left
        data = data << 1;
        pulse_clock();
    }
    SDIO_PORT->DIRCLR = 1 << SDIO_PIN ; // clear to allow to read.
}

void init_spi() {
    // SCS must be set as an output and initially high
    SCS_PORT->DIRSET = 1 << SCS_PIN;
    SCS_PORT->OUTSET = 1 << SCS_PIN;
    // SCK must be set as an output and initially low
    SCK_PORT->DIRSET = 1 << SCK_PIN;
    // SDIO must be set as an input
    SDIO_PORT->DIRCLR = 1 << SDIO_PIN;
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

uint8_t spi_read_byte(uint8_t addr) {
    // Read a single byte
    //
    uint8_t firstbyte = (addr & 0x3f) | 0x40; // Set read flag.
    uint8_t data = 0;
    scs_low();
    send_byte(firstbyte);
    for (uint8_t n=0; n<8; n++) {
        data = data << 1;
        pulse_clock();
        if (SDIO_PORT->IN & (1 << SDIO_PIN)) {
            data = data | 1; // High = 1 bit.
        }
    }
    scs_high();
    return data;
}

void spi_read_block(uint8_t addr, uint8_t *buf, uint8_t datalen) {
    // Read more than 1 byte
    uint8_t firstbyte = (addr & 0x3f) | 0x40; // Set read flag.
    scs_low();
    send_byte(firstbyte);
    for (uint8_t i=0; i< datalen; i++) {
        uint8_t data = 0;
        for (uint8_t n=0; n<8; n++) {
            data = data << 1;
            pulse_clock();
            if (SDIO_PORT->IN & (1 << SDIO_PIN)) {
                data = data | 1; // High = 1 bit.
            }
        }
        buf[i] = data;
    }
    scs_high();
}
