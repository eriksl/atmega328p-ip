#ifndef application_h
#define application_h

#include <stdint.h>

enum
{
	application_num_args = 4,
	application_length_args = 8,
};

void application_init(void);
void application_idle(void);
int16_t application_content(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst);

#endif
