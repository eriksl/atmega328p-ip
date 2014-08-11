#ifndef _application_h_
#define _application_h_

#include <stdint.h>

uint8_t application_init(void); // return requested idle frequency
void application_idle(void);
int16_t application_content(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst);

#endif
