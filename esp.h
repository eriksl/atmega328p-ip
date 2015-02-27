#ifndef esp_h
#define esp_h

#include <stdint.h>

void		esp_init(uint16_t rsize, uint8_t *rbuffer, uint16_t ssize, uint8_t *sbuffer); // call cli()
void		esp_periodic(void);

uint8_t		esp_receive_finished(void);
uint16_t	esp_receive_length(uint8_t *connection);
void		esp_send_start(uint16_t length, uint8_t *connection);
uint8_t		esp_send_finished(void);

#endif
