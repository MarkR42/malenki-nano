
#include <stdint.h>

/*
 * Master state 
 */

typedef struct {
    volatile uint32_t tickcount; // centiseconds
} master_state_t;
// Accessor function which disables interrupts
uint32_t get_tickcount();
uint32_t get_micros();
void trigger_reset();

extern volatile master_state_t master_state;


