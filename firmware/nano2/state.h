
#include <stdint.h>

/*
 * Master state 
 */

typedef struct {
    volatile uint32_t tickcount; // centiseconds
    volatile uint8_t radio_interrupt_ok; // set to 1 in irq handler if everything ok
} master_state_t;
// Accessor function which disables interrupts
uint32_t get_tickcount();
void trigger_reset();
void epic_fail(const char * reason);
void shutdown_system();

extern volatile master_state_t master_state;


