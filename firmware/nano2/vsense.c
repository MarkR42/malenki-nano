#include <avr/io.h>
#include <avr/interrupt.h>

#include "vsense.h"
#include "diag.h"
#include "state.h"

vsense_state_t vsense_state;

void vsense_init()
{
    // NB: input voltage to mcu should be 3.3
    // Set voltage reference to 2.5v (highest which is guaranteed lower than input)
    VREF.CTRLC = VREF_ADC1REFSEL_2V5_gc;
    // We use pin PC2 as analogue input.
    // Disable the digital input on PA7.
    PORTC.PIN2CTRL &= ~PORT_ISC_gm;
    PORTC.PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;

    ADC1.CTRLC = ADC_PRESC_DIV16_gc      /* CLK_PER divider */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC1.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC1.MUXPOS  = ADC_MUXPOS_AIN8_gc;
    
    /* Enable FreeRun mode */
    ADC1.CTRLA |= ADC_FREERUN_bm;

    // Start the first conversion.
    ADC1.COMMAND = ADC_STCONV_bm;

}

static const uint32_t vsense_period = 100; // Centiseconds
static uint32_t last_vsense_tickcount = 0;

void vsense_loop()
{
    // called every loop.
    // Only do stuff every vsense_period
    uint32_t now = get_tickcount();
    uint32_t age = (now - last_vsense_tickcount);
    if (age >= vsense_period) 
    {
        // Check if ready.
        if (ADC1.INTFLAGS & ADC_RESRDY_bm)
        {
            uint16_t val = ADC1.RES; // 0..1023
            // Scale to give voltage in millivolts
            uint32_t vsense_mv = (((uint32_t) val) * 2500) / 1023;
            // This is the voltage of the pin, we need to scale it up because we have
            // A potential divider in circuit with 10k and 33k resistors to gnd and +v
            vsense_mv = (vsense_mv * (10+33)) / 10;
            // Print some debug info
            // Calculate the number of cells
            // Do this only during startup.
            if (now < 700) {
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
            // diag_println("vbat=%04ld mv", vsense_mv);
            vsense_state.voltage_mv = vsense_mv;
            if (vsense_state.cells_count > 0) 
            {
                // Only do battery diagnostics if we have a known size pack.
                int warn_voltage = vsense_state.cells_count  * 3400;
                if (vsense_mv < warn_voltage) 
                    diag_println("Warning: low battery");
                // TODO: flash lights if low battery
                // TODO: shut down the system if battery too low.
            }
        }
        last_vsense_tickcount = now;
    }
}
