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

// using PA7
VPORT_t * const SCK_VPORT = &VPORTA;
const uint8_t SCK_PIN = 7; // PA7

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

void spi_init() {
    // SCS must be set as an output and initially high
    SCS_VPORT->DIR |= 1 << SCS_PIN;
    SCS_VPORT->OUT |= 1 << SCS_PIN;
    // SCK must be set as an output and initially low
    SCK_VPORT->DIR |= 1 << SCK_PIN;
    // SDIO must be set as an input
    SDIO_VPORT->DIR &= ~(1 << SDIO_PIN);
}

// All other functions are implemented in assembler.
