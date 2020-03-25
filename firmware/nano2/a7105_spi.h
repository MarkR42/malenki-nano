
void spi_init();

void spi_write_byte(uint8_t addr, uint8_t data);
void spi_strobe(uint8_t cmd);
void spi_strobe2(uint8_t cmd, uint8_t cmd2);
void spi_strobe3(uint8_t cmd, uint8_t cmd2, uint8_t cmd3);
void spi_write_byte_then_strobe(uint8_t addr, uint8_t data, uint8_t cmd);
uint8_t spi_read_byte(uint8_t addr);

void spi_read_block(uint8_t addr, uint8_t *buf, uint8_t datalen);
void spi_write_block(uint8_t addr, const uint8_t *buf, uint8_t datalen);

void spi_strobe_then_read_block(uint8_t cmd,
    uint8_t addr, uint8_t *buf, uint8_t datalen);

void spi_strobe_then_write_block(uint8_t cmd,
    uint8_t addr, uint8_t *buf, uint8_t datalen);
