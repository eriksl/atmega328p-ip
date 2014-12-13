#ifndef application_h
#define application_h

#include <stdint.h>

enum
{
	application_num_args = 4,
	application_length_args = 8,
};

typedef uint8_t args_t[application_num_args][application_length_args];

typedef struct
{
	const uint8_t *cmdline;
	uint16_t cmdline_length;
	uint8_t nargs;
	args_t *args;
	uint16_t size;
	uint8_t *dst;
} application_parameters_t;

void application_init(void); 		// return requested idle frequency
void application_periodic(void);	// to be called periodicly from main or any other long running routine
int16_t application_content(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst);

#endif
