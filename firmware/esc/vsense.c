#include <avr/io.h>
#include <avr/interrupt.h>

#include "vsense.h"
#include "diag.h"
#include "state.h"

void vsense_init()
{
   // Set voltage reference to 4.3v (highest)
   VREF.CTRLA = VREF_ADC0REFSEL_4V34_gc;
	// We use pin PA7.
	// Disable the digital input on PA7.
	PORTA.PIN7CTRL &= ~PORT_ISC_gm;
	PORTA.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc;

	ADC0.CTRLC = ADC_PRESC_DIV16_gc      /* CLK_PER divider */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS  = ADC_MUXPOS_AIN7_gc;
    
    /* Enable FreeRun mode */
   ADC0.CTRLA |= ADC_FREERUN_bm;

    // Start the first conversion.
    ADC0.COMMAND = ADC_STCONV_bm;

}

static const uint32_t vsense_period = 300; // Centiseconds
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
    	// if (ADC0.INTFLAGS & ADC_RESRDY_bm) {
	    if (ADC0.INTFLAGS & ADC_RESRDY_bm)
        {
            uint16_t val = ADC0.RES; // 0..1023
            // Scale to give voltage in millivolts
            uint32_t vsense_mv __attribute__((unused)) = (((uint32_t) val) * 4300) / 1023;
            // This is the voltage of the pin, we need to scale it up because we have
            // A potential divider in circuit with 10k and 33k resistors to gnd and +v
            vsense_mv = (vsense_mv * (10+33)) / 10;
            // Print some debug info  (which is annoying)
            diag_println("battery voltage=%05ld mv", vsense_mv);
        }
        last_vsense_tickcount = now;
    }
}
