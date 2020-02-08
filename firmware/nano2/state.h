
#include <stdint.h>

/*
 * Master state 
 */

typedef struct {
    volatile uint32_t tickcount; // centiseconds
} master_state_t;
// Accessor function which disables interrupts
uint32_t get_tickcount();
void trigger_reset();
void epic_fail(const char * reason);

extern volatile master_state_t master_state;


