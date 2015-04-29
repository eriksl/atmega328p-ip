#ifndef application_h
#define application_h

#include <stdint.h>

enum
{
	application_num_args = 5,
	application_length_args = 8,
};

typedef uint8_t args_t[application_num_args][application_length_args];

typedef struct
{
	const uint8_t *cmdline;
	uint8_t nargs;
	args_t *args;
	uint16_t size;
	uint8_t *dst;
} application_parameters_t;

void application_init(void);
void application_periodic(void);
uint8_t application_content(const uint8_t *src, uint16_t size, uint8_t *dst);

#endif
