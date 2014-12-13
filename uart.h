#ifndef uart_h
#define uart_h

#include <stdint.h>

void 		uart_init(void);
void		uart_baud(uint32_t baud);
uint16_t	uart_transmit(uint16_t length, const uint8_t *buffer);
uint16_t	uart_receive(uint16_t size, uint8_t *buffer);
void		uart_flush(void);

#endif
