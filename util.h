#ifndef _util_h_
#define _util_h_

#include <stdint.h>

void int_to_str(uint16_t in, uint8_t outlen, uint8_t *out);
uint8_t hex_to_int(uint16_t *length, uint8_t const **in, uint8_t *value);
void int_to_hex(uint8_t *buffer, uint8_t value);
void xstrncpy(const uint8_t *in, uint16_t outlen, uint8_t *out);
void fxstrncpy(const __flash uint8_t *in, uint16_t outlen, uint8_t *out);
void xstrncat(const uint8_t *in, uint16_t outlen, uint8_t *out);
void fxstrncat(const __flash uint8_t *in, uint16_t outlen, uint8_t *out);
uint16_t xstrlen(const uint8_t *str);

#endif
