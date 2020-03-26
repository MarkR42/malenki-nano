
typedef struct {
    int cells_count;
    uint16_t voltage_mv;
    uint16_t temperature_c10; 
} vsense_state_t;

extern vsense_state_t vsense_state;

void vsense_init();
void vsense_loop();
