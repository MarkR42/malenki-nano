#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 3333333 /* 20MHz / 6(default prescale) */
#include <util/delay.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "diag.h"
#include "rxin.h"
#include "motors.h"
#include "vsense.h"
#include "state.h"
#include "nvconfig.h"
#include "blinky.h"

#include <stdlib.h>

volatile master_state_t master_state;

void init_hardware()
{
	motors_init();
	rxin_init();
	vsense_init();
    blinky_init();
}

void init_timer_interrupts()
{
	// This uses TCB1 for timer interrupts.
	const unsigned short compare_value = 33333; // 100hz
	TCB1.CCMP = compare_value;
	TCB1.INTCTRL = TCB_CAPT_bm;
	// CTRLB bits 0-2 are the mode, which by default
	// 000 is "periodic interrupt" -which is correct
	TCB1.CTRLB = 0;
	TCB1.CTRLA = TCB_ENABLE_bm;
}

ISR(TCB1_INT_vect)
{
    master_state.tickcount++;
    TCB1.INTFLAGS |= TCB_CAPT_bm; //clear the interrupt flag(to reset TCB0.CNT)
}

uint32_t get_tickcount()
{
    uint32_t tc;
    cli(); // disable irq
    tc = master_state.tickcount;
    sei(); // enable irq
    return tc;
}

static uint32_t last_test_tickcount=0;
static int pulsewidth = 0;

static void test_loop()
{
    // Do a few testing things, periodically
    uint32_t now = get_tickcount();
    uint32_t age = now - last_test_tickcount;
    if (age > 100) {
		diag_println("ticks: %08ld", master_state.tickcount);
		diag_println("last_pulse_len: %06d pulse_count: %06d next_channel: %d", 
			(int) rxin_state.last_pulse_len, (int) rxin_state.pulse_count, (int) rxin_state.next_channel);
		pulsewidth += 10;
		if (pulsewidth > 180) {
			pulsewidth = 0;
		}
		TCA0.SPLIT.HCMP0 = pulsewidth;

        last_test_tickcount = now;
    }    

}

static void mainloop()
{
    // int8_t movedir = 4;
    // int16_t speed = 0; // Range -DUTY_MAX to DUTY_MAX
    blinky_state.blue_on = 1;
	while (1) {
		vsense_loop();
        motors_loop();
        rxin_loop();
        test_loop();
        blinky_loop();
        
        /* 
        speed += movedir;
        if (speed >= DUTY_MAX) {
            movedir = -abs(movedir);
        }
        if (speed <= -DUTY_MAX) {
            movedir = abs(movedir);
        }
        set_motor_direction_duty(MOTOR_WEAPON,speed);
        set_motor_direction_duty(MOTOR_LEFT,speed);
        set_motor_direction_duty(MOTOR_RIGHT,speed);
        _delay_ms(20);
        */
	}
}

int main(void)
{
	init_hardware();	
	init_timer_interrupts();
	sei();  // enable interrupts
    nvconfig_init();
	diag_puts("\r\n\r\n");
	diag_puts("Malenki-ESC starting. BUILD_DATE=");
    diag_puts(build_date);
	diag_puts("\r\n\r\n");
    nvconfig_load();
    nvconfig_save();
	mainloop();
}
