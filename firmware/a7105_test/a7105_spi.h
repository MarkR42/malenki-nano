
void init_spi();

void spi_write_byte(uint8_t addr, uint8_t data);
void spi_strobe(uint8_t cmd);
uint8_t spi_read_byte(uint8_t addr);

void spi_read_block(uint8_t addr, uint8_t *buf, uint8_t datalen);
void spi_write_block(uint8_t addr, const uint8_t *buf, uint8_t datalen);

