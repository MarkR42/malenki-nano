#include <stdint.h>

void motors_init();
void motors_loop();

#define DUTY_MAX 200

#define MOTOR_WEAPON 0
#define MOTOR_LEFT 1
#define MOTOR_RIGHT 2

void set_motor_direction_duty(uint8_t motor_id, int16_t direction_and_duty);

void motors_all_off();

void enable_motor_brake(uint8_t motor_id);

