#ifndef _spi_h_
#define _spi_h_
#include <stdint.h>

void spi_init(void);
void spi_start(void);
void spi_stop(void);
uint8_t spi_io(uint8_t out);
#endif
