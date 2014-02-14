#ifndef _util_h_
#define _util_h_

#include <stdint.h>

void int_to_str(uint16_t in, uint8_t outlen, uint8_t *out);
void xstrncat(const uint8_t *in, uint16_t outlen, uint8_t *out);
uint16_t xstrlen(const uint8_t *str);

#endif
