/*
 * For the "Scarab" product.
 * 
 * Motor driver chip is TI DRV8243HQRXY
 * 
 * Signals are:
 * 
 * MOTOR1 (RIGHT)
 * PA3 -> MOTOR1F
 * PB0 -> MOTOR1R
 * MOTOR2 (LEFT)
 * PB1 -> MOTOR2F
 * PB2 -> MOTOR2R
 * 
 * PA4 -> nSLEEP (both motors)
 * nFAULT -> PA5 (both motors) (open collector; set internal pullup)
 */

#include "motors.h"
#include "diag.h"
#include "state.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdbool.h>

#define F_CPU 10000000 /* 10MHz */
#include <util/delay.h>

#define NFAULT_PORT PORTA
#define NFAULT_PIN 5
#define NSLEEP_PORT PORTA
#define NSLEEP_PIN 4

static void reset_drivers();
static void maybe_reset_drivers();

static uint32_t last_reset_ticks;

void motors_init()
{
	// Set all the ports which need to be outputs, to be outputs.
	// should all be low initially.
	// motor pwm outputs = PB0,1,2 and PA3
	// PB0, PB1, PB2
	// clear initial output
	PORTB.OUTCLR = 1 << 0;
	PORTB.OUTCLR = 1 << 1;
	PORTB.OUTCLR = 1 << 2;
	PORTA.OUTCLR = 1 << 3;
	// set directions to out
	PORTB.DIRSET = 1 << 0;
	PORTB.DIRSET = 1 << 1;
	PORTB.DIRSET = 1 << 2;
	PORTA.DIRSET = 1 << 3;
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
		TCA_SPLIT_LCMP0EN_bm | // PB0
		TCA_SPLIT_LCMP1EN_bm | // PB1
		TCA_SPLIT_LCMP2EN_bm | // PB2
		TCA_SPLIT_HCMP0EN_bm; // PA3
		// Unused channel: TCA_SPLIT_HCMP1EN_bm - PA4
		// Unused channel: TCA_SPLIT_HCMP2EN_bm - PA5
	// Set the period - use the same period for both halves.
	TCA0.SPLIT.HPER = period_val;
	TCA0.SPLIT.LPER = period_val;
	// Set the halves to be "out of sync"
	TCA0.SPLIT.HCNT = period_val /2;
	// Finally, turn the timer on.
    // Divide by 64, 3.333mhz / 64 =~ 52khz
    // 52khz / period_val =~ 260hz = PWM frequency.
	TCA0.SPLIT.CTRLA = 
		TCA_SPLIT_CLKSEL_DIV16_gc | // divide sys. clock by 16 
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
    
    // TODO: set nFault as input and enable pullup.
    NFAULT_PORT.DIRCLR = (1 << NFAULT_PIN);
    // Enable pullup
    NFAULT_PORT.PIN5CTRL = (PORT_PULLUPEN_bm);
    // Enable nsleep as an output, make it high
    NSLEEP_PORT.OUTSET = (1 << NSLEEP_PIN);
    NSLEEP_PORT.DIRSET = (1 << NSLEEP_PIN);
    // Two attempts to reset the drivers, at least so we can see that
    // the driver reset is successful.
    // We need to wait at least long enough, that the pullup enabled above
    // has time to pull the pin high. This could conceivably take a few 
    // microseconds.
    maybe_reset_drivers();
    maybe_reset_drivers();
    
    last_reset_ticks = get_tickcount();
}

static void reset_drivers()
{
    // pulse nSleep to reset drivers.
    // tRESET time max is 20 microseconds, we need to wait at least that long,
    // But must be less than 40us ( tSLEEP min)
    NSLEEP_PORT.OUTCLR = (1 << NSLEEP_PIN);
    _delay_us(30);
    NSLEEP_PORT.OUTSET = (1 << NSLEEP_PIN);
}

static uint8_t test_for_fault()
{
    return (NFAULT_PORT.IN & (1 << NFAULT_PIN));
}

static void maybe_reset_drivers()
{
    // Wait a little in case we already just called reset_drivers
    // And wait a little so that the pullup has time to start working
    // and the chip has time to get itself ready.
    _delay_us(300);
    // Check NFAULT
    uint8_t nfault_state = test_for_fault();
    if (nfault_state == 0) {
        diag_println("motors_scarab: nfault is active, resetting drivers");
        reset_drivers();
        _delay_us(100); // Wait a little for it to wake up.
    } else {
        diag_println("motors_scarab: nfault high (ok)");
    }
}

// Pin IDs
// Motor 1 = RIGHT
#define MOTOR_1F 0
#define MOTOR_1R 1
// Motor 2 = LEFT
#define MOTOR_2F 2
#define MOTOR_2R 3

// Table mapping the motor ID to the compare register
// which controls its duty.
static uint8_t volatile * motor_map[] = {
    & (TCA0.SPLIT.HCMP0), // 1F, PA3
    & (TCA0.SPLIT.LCMP0), // 1R, PB0
    & (TCA0.SPLIT.LCMP1), // 2F, PB1
    & (TCA0.SPLIT.LCMP2), // 2R, PB2 
} ;

static void set_pin_duty(uint8_t signal_id, uint8_t duty)
{
    volatile uint8_t * compare_register=motor_map[signal_id]; 
    *compare_register = duty;
}

/*
 * DRV8243 PWM table (page 34 of datasheet)
 * 
 * IN1 IN2  OUT1 OUT2
 * 0   0    H    H  <-- Braking / recirculation
 * 0   1    L    H
 * 1   0    H    L 
 * 1   1    Hi-Z Hi-Z  (fell on to page 35)
 * 
 * When pulsing we should use braking / recirculation.
 */
void set_motor_direction_duty(uint8_t motor_id, int16_t direction_and_duty)
{
    int8_t direction=0;
    if (direction_and_duty > 0) { direction=1; }
    if (direction_and_duty < 0) { direction=-1; }
    uint8_t duty = (uint8_t) abs(direction_and_duty);

    // Set the other side fully high
    uint8_t fwd=0;
    uint8_t rev=0;
    switch(motor_id) {
        case MOTOR_RIGHT:
            fwd=MOTOR_1F; rev=MOTOR_1R; break; 
        case MOTOR_LEFT:
            fwd=MOTOR_2F; rev=MOTOR_2R; break; 
    }
    // fwd and rev both zero - unknown motor id.
    if (fwd || rev) {
        // Set the pin low for the duty cycle we want:
        uint8_t fwd_duty = duty;
        uint8_t rev_duty = duty;
        // Set the opposite site fully high.
        if (direction <= 0) {
            // Not going forward
            fwd_duty=0; 
        }
        if (direction >= 0) {
            // Not going back
            rev_duty=0; 
        }
        if (direction == 0) {
            // Stop - set both high to allow free run
            fwd_duty = rev_duty = DUTY_MAX;
        }
        set_pin_duty(fwd, fwd_duty);
        set_pin_duty(rev, rev_duty);
    }
}

void enable_motor_brake(uint8_t motor_id)
{
    // drv8243 brakes when IN1 / IN2 are low.
    // Set the other side fully high
    uint8_t fwd=0;
    uint8_t rev=0;
    switch(motor_id) {
        case MOTOR_RIGHT:
            fwd=MOTOR_1F; rev=MOTOR_1R; break; 
        case MOTOR_LEFT:
            fwd=MOTOR_2F; rev=MOTOR_2R; break; 
    }
    set_pin_duty(fwd, 0);
    set_pin_duty(rev, 0);
}

static void motors_check_for_fault()
{
    if (test_for_fault() == 0) {
        // nFault active low
        maybe_reset_drivers();
    }
}

void motors_loop()
{
    // Every so often, check the status of nFault,
    // If there is a fault detected, reset the drivers.
    // We don't want to do this too often as it might interfere
    // (glitch) the non-fault side if we have a fault on one side 
    // only.
    uint32_t t= get_tickcount();
    if (t != last_reset_ticks) {
        uint32_t delta = t - last_reset_ticks;
        if (delta > 10) {
            motors_check_for_fault();
            last_reset_ticks = t;
        }
    }
}

void motors_all_off()
{
    set_motor_direction_duty(MOTOR_LEFT,0);
    set_motor_direction_duty(MOTOR_RIGHT,0);
}
