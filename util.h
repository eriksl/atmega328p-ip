#ifndef util_h
#define util_h

#include <stdint.h>

void	reset(void);
void	msleep(uint16_t ms);
void	pause_idle(void);
uint8_t crc8(uint8_t crc_mode, uint8_t length, const uint8_t *block);

#endif
