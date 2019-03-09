#include <avr/io.h>
#include <avr/interrupt.h>

#include "vsense.h"
#include "diag.h"

void init_vsense()
{
	// We use pin PA7.
	// Disable the digital input on PA7.
	PORTA.PIN7CTRL &= ~PORT_ISC_gm;
	PORTA.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc;

	ADC0.CTRLC = ADC_PRESC_DIV4_gc      /* CLK_PER divided by 4 */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS  = ADC_MUXPOS_AIN7_gc;
    
    /* Enable FreeRun mode */
   ADC0.CTRLA |= ADC_FREERUN_bm;

   // Set voltage reference to 4.3v (highest)
   VREF.CTRLA = VREF_ADC0REFSEL_4V34_gc;

}

void vsense_test()
{
	// Check if ready.
	// if (ADC0.INTFLAGS & ADC_RESRDY_bm) {
	ADC0.INTFLAGS = ADC_RESRDY_bm;
		uint16_t val = ADC0.RES;
		diag_println("vsense=%08d", val);
	// }
}
