
#include <stdint.h>

/*
 * Master state 
 */

typedef struct {
    volatile uint32_t tickcount; // centiseconds
} master_state_t;

extern volatile master_state_t master_state;

