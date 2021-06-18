#include <avr/io.h>
#include <avr/interrupt.h>

#include "vsense.h"
#include "diag.h"
#include "state.h"
#include "radio.h"

vsense_state_t vsense_state;

static void vsense_hw_init()
{
    // NB: input voltage to mcu should be 3.3
    // Set voltage reference to 2.5v (highest which is guaranteed lower than input)
    VREF.CTRLA = VREF_ADC0REFSEL_2V5_gc;
    // We use pin PC2 as analogue input.
    // Disable the digital input on PA7.
    PORTC.PIN2CTRL &= ~PORT_ISC_gm;
    PORTC.PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;

    ADC0.CTRLC = ADC_PRESC_DIV16_gc      /* CLK_PER divider */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS  = ADC_MUXPOS_AIN8_gc; // ADC0.AIN8 == Pin PB5
    
    /* Enable FreeRun mode */
    ADC0.CTRLA |= ADC_FREERUN_bm;

    // Start the first conversion.
    ADC0.COMMAND = ADC_STCONV_bm;    
}

static void tempsense_hw_init()
{
    // TODO 
}

static uint16_t temp_calc()
{
    return 200;
}

void vsense_init()
{
    vsense_hw_init();
    tempsense_hw_init();
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
#define CRITICAL_VOLTAGE_PER_CELL 3150
// Number of readings below CRITICAL_VOLTAGE_PER_CELL to take before
// shutdown.
#define CRITICAL_COUNT 12

void vsense_loop()
{
    // called every loop.
    // Only do stuff every vsense_period
    uint32_t now = get_tickcount();
    uint32_t age = (now - last_vsense_tickcount);
    if (age >= vsense_period) 
    {
        // Check if ready.
        if (ADC0.INTFLAGS & ADC_RESRDY_bm)
        {
            uint16_t val = ADC0.RES; // 0..1023
            // Scale to give voltage in millivolts
            uint32_t vsense_mv = (((uint32_t) val) * 2500) / 1023;
            // This is the voltage of the pin, we need to scale it up because we have
            // A potential divider in circuit with 10k and 33k resistors to gnd and +v
            vsense_mv = (vsense_mv * (10+33)) / 10;
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
                // Only do battery diagnostics if we have a known size pack.
                uint16_t warn_voltage = vsense_state.cells_count  * 3500;
                if (vsense_mv < warn_voltage) 
                    diag_println("Warning: low battery");
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
        vsense_state.temperature_c10 = temp_calc();
        last_vsense_tickcount = now;
    }
}
