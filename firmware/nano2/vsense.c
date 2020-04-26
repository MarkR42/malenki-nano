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

static void tempsense_hw_init()
{
    // Use ADC0 to sense temperature.
    VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc;
    ADC0.CTRLC = ADC_PRESC_DIV16_gc      /* CLK_PER divider */
               | ADC_REFSEL_INTREF_gc  /* Internal reference */
               | ADC_SAMPCAP_bm; // capacitance for temp sensor
    ADC0.CTRLD = ADC_INITDLY_DLY64_gc;
    // ADCn.SAMPCTRL select SAMPLEN≥32μs
    // (each clock cycle is 1.6 us so we need at least 20 of them)
    ADC0.SAMPCTRL = 0x18; // only 5 bits
    ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;
    ADC0.CTRLA = ADC_ENABLE_bm |
        ADC_RESSEL_10BIT_gc;
    ADC0.CTRLA |= ADC_FREERUN_bm;
    // Start the first conversion.
    ADC0.COMMAND = ADC_STCONV_bm; 
}

static uint16_t temp_calc()
{
    // returns temperature in Celsius * 10
    
    // Code from datasheet
    int8_t sigrow_offset = SIGROW.TEMPSENSE1;  // Read signed value from signature
    uint8_t sigrow_gain = SIGROW.TEMPSENSE0;    // Read unsigned value from signature row
    uint16_t adc_reading = ADC0.RES;   // ADC conversion result with 1.1 V internal reference 
    uint32_t temp = adc_reading - sigrow_offset;
    temp *= sigrow_gain;  // Result might overflow 16 bit variable (10bit+8bit)
    // We now have temperature in K * 256
    // temp += 0x80;               // Add 1/2 to get correct rounding on division below
    // temp >>= 8;                 // Divide result to get Kelvin 
    // uint16_t temperature_in_K = temp;
    // Let's assume at this point that the temp is above freezing...
    temp -= ((uint32_t) (273.1 * 256.0));
    // Now we can fit in a 16 bit
    uint16_t tempc_256 = temp;
    
    // This jiggery-pokery should convert to C * 10
    return (((tempc_256 / 8) * 10) /32);
}

void vsense_init()
{
    vsense_hw_init();
    tempsense_hw_init();
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
                uint16_t critical_voltage = vsense_state.cells_count * 3350;
                if (vsense_mv < critical_voltage) {
                    vsense_state.critical_count += 1;
                    diag_println("Warning: VERY LOW BATTERY");
                    if (vsense_state.critical_count > 4) {
                        shutdown_system();
                    }
                } else {
                    vsense_state.critical_count = 0;
                }
            }
        }
        if (ADC0.INTFLAGS & ADC_RESRDY_bm) {
            vsense_state.temperature_c10 = temp_calc();
            // diag_println("Temperature: %03d degrees C*10", vsense_state.temperature_c10);
        }
        last_vsense_tickcount = now;
    }
}
