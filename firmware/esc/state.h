
#include <stdint.h>

/*
 * Master state 
 */

typedef struct {
    volatile uint32_t tickcount; // centiseconds
} master_state_t;
// Accessor function which disables interrupts
uint32_t get_tickcount();

extern volatile master_state_t master_state;

extern const char * const build_date;


