#ifndef _application_h_
#define _application_h_

#include <stdint.h>

void application_init(void);
void application_idle(void);
int16_t application_content(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst);

#endif
