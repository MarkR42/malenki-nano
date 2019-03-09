
void init_rxin();

typedef struct {
	uint16_t last_pulse_len;
	uint16_t pulse_count;
} rxin_state_t;

extern rxin_state_t rxin_state;

