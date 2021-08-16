
typedef struct {
    int cells_count;
    uint16_t voltage_mv;
    uint8_t critical_count;
} vsense_state_t;

extern vsense_state_t vsense_state;

void vsense_init();
void vsense_loop();
