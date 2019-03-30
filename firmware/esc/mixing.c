#include <stdint.h>

#include "motors.h"
#include "diag.h"

static int signedclamp(int n, int maxval)
{
    if (n > maxval) return maxval;
    if (n < -maxval) return -maxval;
    return n;
}

static int deadzone(int n, int deadzone)
{
    if ((n < deadzone) && (n > (-deadzone))) {
        return 0;
    }
    return n;
}

static uint16_t diag_count=0;

void mixing_drive_motors(int16_t throttle, int16_t steering, int16_t weapon)
{
    // inputs are already calibrated to centre on zero.
    // Inputs should be in (approx) microseconds of pulse.
    // Throttle, steering and weapon range is typically +- 450 full range.
    
    // Scale throttle to range +- 100
    throttle = (throttle * 10) / 45;
    // Scale steering to range +- 100
    steering = (steering * 10) / 45;
    // Clamp
    throttle = signedclamp(throttle, 100);  
    steering = signedclamp(steering, 100);  
    
    throttle = deadzone(throttle, 10);    
    steering = deadzone(steering, 10);    
    // Scale steering further, to stop steering too fast.
    steering = steering / 2;
    
    int left = throttle + steering;   
    int right = throttle - steering;   
   
    // left / right should now have range -150 ... 150
    // Clamp them again at 100
    left = signedclamp(left, 100); 
    right = signedclamp(right, 100); 
    // Now scale back to += 200 which is correct for pwm
    left = left *2;
    right = right * 2;
    
    // Fix up weapon to correct range 
    weapon = (weapon * 20) / 45; // -200 ... 200
    weapon = deadzone(weapon, 20);
    weapon = signedclamp(weapon, 200);

    set_motor_direction_duty(MOTOR_WEAPON, weapon);
    set_motor_direction_duty(MOTOR_LEFT, left);
    set_motor_direction_duty(MOTOR_RIGHT, right);
    if (diag_count == 0) {
        diag_count = 10;
        diag_println("L: %03d R: %03d W: %03d", left, right, weapon);
    } else {
        --diag_count;
    }
}
