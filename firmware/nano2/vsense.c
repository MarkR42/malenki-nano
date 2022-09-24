#include <avr/io.h>
#include <avr/interrupt.h>

#include "vsense.h"
#include "diag.h"
#include "state.h"
#include "radio.h"

vsense_state_t vsense_state;

/*
 * The old version of the Malenki 1617 used ADC1
 */
#ifdef __AVR_ATtiny1617__
#define ADC_VSENSE ADC1
#else
#define ADC_VSENSE ADC0
#endif


static void vsense_hw_init()
{
    // NB: input voltage to mcu should be 3.3
    // Set voltage reference to 2.5v (highest which is guaranteed lower than input)
    VREF.CTRLA = VREF_ADC0REFSEL_2V5_gc;
    // We use pin PC2 as analogue input (on attiny1617).
    // Disable the digital input.
    PORTC.PIN2CTRL &= ~PORT_ISC_gm;
    PORTC.PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    // We use pin PB5 as analogue input on attiny817
    // disable its digital input.
    PORTB.PIN5CTRL &= ~PORT_ISC_gm;
    PORTB.PIN5CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    // Note that PB5 and PC2 are electrically connected so we can
    // possibly reduce noise by disabling them both
#ifdef __AVR_ATtiny1617__
    // Attiny1617 - old version which used PC2 - use ADC1 mux channel 8.
    VREF.CTRLC |= VREF_ADC1REFSEL_2V5_gc;
    
    ADC1.CTRLC = ADC_PRESC_DIV16_gc      /* CLK_PER divider */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC1.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    ADC1.MUXPOS  = ADC_MUXPOS_AIN8_gc; // ADC1 muxpos 8 = PC2
#else
    // Attiny817, 1616 and maybe others that use PB5
    ADC0.CTRLC = ADC_PRESC_DIV16_gc      /* CLK_PER divider */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS  = ADC_MUXPOS_AIN8_gc; // ADC0.AIN8 == Pin PB5
#endif

    /* Enable FreeRun mode */
    ADC_VSENSE.CTRLA |= ADC_FREERUN_bm;

    // Start the first conversion.
    ADC_VSENSE.COMMAND = ADC_STCONV_bm;    
}

void vsense_init()
{
    vsense_hw_init();
}

static const uint32_t vsense_period = 100; // Centiseconds
static uint32_t last_vsense_tickcount = 0;

/*
 * Shutdown on low voltage, but not too easily.
 * 
 * A very high current spike, e.g. spinner starting or servo might
 * cause a temporary voltage drop which does not indicate low battery,
 * so we will wait a while to see if it recovers.
 */

// Voltage, in mv, per cell, where we shut down.
#define CRITICAL_VOLTAGE_PER_CELL 2800
// NB: On a single cell pack at 2.8 volts the radio won't work and the MCU might brownout
// if that happens, we will shut down anyway.

// Number of readings below CRITICAL_VOLTAGE_PER_CELL to take before
// shutdown (seconds)
#define CRITICAL_COUNT 12

#define VSENSE_BOTTOMRESISTOR 10

#ifdef PRODUCT_IS_PLUS
#define VSENSE_TOPRESISTOR 100
#else
#define VSENSE_TOPRESISTOR 33
#endif

void vsense_loop()
{
    // called every loop.
    // Only do stuff every vsense_period
    uint32_t now = get_tickcount();
    uint32_t age = (now - last_vsense_tickcount);
    if (age >= vsense_period) 
    {
        // Check if ready.
        if (ADC_VSENSE.INTFLAGS & ADC_RESRDY_bm)
        {
            uint16_t val = ADC_VSENSE.RES; // 0..1023
            // Scale to give voltage in millivolts
            uint32_t vsense_mv = (((uint32_t) val) * 2500) / 1023;
            // This is the voltage of the pin, we need to scale it up because we have
            // A potential divider in circuit with 10k and 33k resistors to gnd and +v
            vsense_mv = (vsense_mv * (VSENSE_BOTTOMRESISTOR+VSENSE_TOPRESISTOR)) / VSENSE_BOTTOMRESISTOR;
            // Print some debug info
            // Calculate the number of cells
            // Do this only during startup.
            if (now < 700) {
                diag_println("vbat=%04ld mv", vsense_mv);
                if ((vsense_mv > 6000) && (vsense_mv < 8500)) {
                    // 2s pack
                    vsense_state.cells_count = 2;
                } else if ((vsense_mv > 3000) && (vsense_mv < 4250)) {
                    // 1s lipo
                    vsense_state.cells_count = 1;
#ifdef PRODUCT_IS_PLUS
				} else if ((vsense_mv > 9000) && (vsense_mv < 18000)) {
                    // 3s or 4s lipo
                    vsense_state.cells_count = 3;
#endif
                } else {
                    // This means we have neither 1s nor 2s pack.
                    vsense_state.cells_count = 0;
                }
                if (vsense_state.cells_count > 0) {
                    diag_println("Detected %d battery cells", vsense_state.cells_count); 
                } else {
                    diag_println("Battery voltage out of range, bench test?");
                }
            }
            vsense_state.voltage_mv = vsense_mv;
            if (vsense_state.cells_count > 0) 
            {
                uint16_t critical_voltage = vsense_state.cells_count * CRITICAL_VOLTAGE_PER_CELL;
                if (vsense_mv < critical_voltage) {
                    vsense_state.critical_count += 1;
                    diag_println("Warning: VERY LOW BATTERY");
                    if (vsense_state.critical_count > CRITICAL_COUNT) {
                        shutdown_system();
                    }
                } else {
                    // Battery seems to have recovered, zero the count.
                    vsense_state.critical_count = 0;
                }
            }
        }
        last_vsense_tickcount = now;
    }
}
