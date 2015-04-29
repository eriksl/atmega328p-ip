#ifndef esp_h
#define esp_h

#include <stdint.h>

void	esp_init(uint32_t baud);
void	esp_write(uint8_t conn_id, const uint8_t *data);
uint8_t	esp_read(uint8_t *conn_id, uint16_t buffer_size, uint8_t *buffer);

#endif
