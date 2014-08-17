#ifndef application_h
#define application_h

#include <stdint.h>

enum
{
	application_num_args = 8,
	application_length_args = 8,
};

uint8_t application_init(void); // return requested idle frequency
void application_idle(void);
int16_t application_content(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst);

#endif
