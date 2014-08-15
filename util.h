#ifndef _util_h_
#define _util_h_

#include <stdint.h>

void reset(void);
void sleep(uint16_t ms);
void pause(void);

uint8_t hex_to_int(uint16_t *length, uint8_t const **in, uint8_t *value);
void int_to_hex(uint8_t *buffer, uint8_t value);

#endif
