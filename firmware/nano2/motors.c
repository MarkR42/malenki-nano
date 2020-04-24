#include "motors.h"
#include "diag.h"
#include "state.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdbool.h>

#define F_CPU 10000000 /* 10MHz */
#include <util/delay.h>

static bool pin_check(char portname, PORT_t *port, uint8_t pin)
{
    uint8_t bm = 1<<pin;
    // Set the pin as an input
    port->DIRCLR = bm;
    // Wait a very tiny time
    _delay_ms(1);
    bool state = (port->IN & bm);
    if (state) {
        // High!
        diag_println("motor pin_check FAIL: port %c %d stuck high",
            portname, pin);
        return false;
    }
    return true;
}

static void motors_check()
{
    // Check that all of our motor lines are working
    // The driver chips should hold the control lines all low.
    // If any are high, it is an error.
	// motor pwm outputs = PB0,1,2 and PA3,4,5
    bool ok = true;
    ok = ok && pin_check('A', &PORTA,3);
    ok = ok && pin_check('A', &PORTA,4);
    ok = ok && pin_check('A', &PORTA,5);
    ok = ok && pin_check('B', &PORTB,0);
    ok = ok && pin_check('B', &PORTB,1);
    ok = ok && pin_check('B', &PORTB,2);
    if (! ok) {
        diag_println("Motor pin stuck high"); // never returns
        diag_println("Continuing anyway");
    }
}

void motors_init()
{
    motors_check();
	// Set all the ports which need to be outputs, to be outputs.
	// should all be low initially.
	// motor pwm outputs = PB0,1,2 and PA3,4,5
	// PB0, PB1, PB2
	// clear initial output
	PORTB.OUTCLR = 1 << 0;
	PORTB.OUTCLR = 1 << 1;
	PORTB.OUTCLR = 1 << 2;
	PORTA.OUTCLR = 1 << 3;
	PORTA.OUTCLR = 1 << 4;
	PORTA.OUTCLR = 1 << 5;
	// set directions to out
	PORTB.DIRSET = 1 << 0;
	PORTB.DIRSET = 1 << 1;
	PORTB.DIRSET = 1 << 2;
	PORTA.DIRSET = 1 << 3;
	PORTA.DIRSET = 1 << 4;
	PORTA.DIRSET = 1 << 5;
	//
	// Timer A should be in split mode
	//
	// As CLK_PER is 3.33 mhz, 
	// HOWEVER, in split mode we have 2x 8-bit timers,
	// so we cannot count to 3000.
	//
	// We can count to 200 because it's a more "round" number.
	// that gives us 200 PWM levels
	uint8_t period_val = DUTY_MAX;
	// Enable split mode
	TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
	// Enable all comparators in split mode
	TCA0.SPLIT.CTRLB = 
		TCA_SPLIT_LCMP0EN_bm |
		TCA_SPLIT_LCMP1EN_bm |
		TCA_SPLIT_LCMP2EN_bm |
		TCA_SPLIT_HCMP0EN_bm |
		TCA_SPLIT_HCMP1EN_bm |
		TCA_SPLIT_HCMP2EN_bm;
	// Set the period - use the same period for both halves.
	TCA0.SPLIT.HPER = period_val;
	TCA0.SPLIT.LPER = period_val;
	// Set the halves to be "out of sync"
	TCA0.SPLIT.HCNT = period_val /2;
	// Finally, turn the timer on.
    // Divide by 64, 3.333mhz / 64 =~ 52khz
    // 52khz / period_val =~ 260hz = PWM frequency.
	TCA0.SPLIT.CTRLA = 
		TCA_SPLIT_CLKSEL_DIV64_gc | // divide sys. clock by 64 
		TCA_SPLIT_ENABLE_bm;
	// To set the PWM duty, we can now write to
	// TCA0.HCMP0, .HCMP1, .HCMP2, .LCMP0, .LCMP1, .LCMP2
	// these would have values of 0.. period_val
	// here are sample values
	// TCA0.SPLIT.LCMP0 = 80; // WO0 / PB0
	// TCA0.SPLIT.LCMP1 = 100; // WO1 / PB1
	// TCA0.SPLIT.LCMP2 = 120; // WO2 / PB2
	// TCA0.SPLIT.HCMP0 = 20; // WO3 / pin PA3
	// TCA0.SPLIT.HCMP1 = 40; // WO4 / PA4
	// TCA0.SPLIT.HCMP2 = 60; // W05 / PA5
}

// Pin IDs
// Motor 1 = weapon
#define MOTOR_1F 0
#define MOTOR_1R 1
// Left drive
#define MOTOR_2F 2
#define MOTOR_2R 3
// Right drive
#define MOTOR_3F 4
#define MOTOR_3R 5

// Table mapping the motor ID to the compare register
// which controls its duty.
static uint8_t volatile * motor_map[] = {
    & (TCA0.SPLIT.HCMP0), // 1F, PA3
    & (TCA0.SPLIT.LCMP0), // 1R, PB0
    & (TCA0.SPLIT.LCMP2), // 2F, PB2
    & (TCA0.SPLIT.LCMP1), // 2R, PB1
    & (TCA0.SPLIT.HCMP2), // 3F, PA5
    & (TCA0.SPLIT.HCMP1), // 3R, PA4
} ;

static void set_pin_duty(uint8_t signal_id, uint8_t duty)
{
    volatile uint8_t * compare_register=motor_map[signal_id]; 
    *compare_register = duty;
}

void set_motor_direction_duty(uint8_t motor_id, int16_t direction_and_duty)
{
    int8_t direction=0;
    if (direction_and_duty > 0) { direction=1; }
    if (direction_and_duty < 0) { direction=-1; }
    uint8_t duty = (uint8_t) abs(direction_and_duty);

    uint8_t fwd=0;
    uint8_t rev=0;
    switch(motor_id) {
        case MOTOR_WEAPON:
            fwd=MOTOR_1F; rev=MOTOR_1R; break; 
        case MOTOR_LEFT:
            fwd=MOTOR_2F; rev=MOTOR_2R; break; 
        case MOTOR_RIGHT:
            fwd=MOTOR_3F; rev=MOTOR_3R; break; 
    }
    // fwd and rev both zero - unknown motor id.
    if (fwd || rev) {
        uint8_t fwd_duty = duty;
        uint8_t rev_duty = duty;
        if (direction <= 0) {
            // Not going forward
            fwd_duty=0; 
        }
        if (direction >= 0) {
            // Not going back
            rev_duty=0; 
        }
        set_pin_duty(fwd, fwd_duty);
        set_pin_duty(rev, rev_duty);
    }
}

void enable_motor_brake(uint8_t motor_id)
{
    volatile uint8_t * comp1 = motor_map[motor_id * 2]; 
    volatile uint8_t * comp2 = motor_map[motor_id * 2 + 1]; 
    // Set both pins high.
    // We should do this as quickly as possible.
    *comp1 = DUTY_MAX;
    *comp2 = DUTY_MAX;
}

void motors_loop()
{

}

void motors_all_off()
{
    set_motor_direction_duty(MOTOR_WEAPON,0);
    set_motor_direction_duty(MOTOR_LEFT,0);
    set_motor_direction_duty(MOTOR_RIGHT,0);
}
