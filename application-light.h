#ifndef application_light_h
#define application_light_h

#include "application.h"

#include <stdint.h>

extern void application_init_light(void);
extern uint8_t application_function_light_read(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
extern uint8_t application_function_light_write(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
#endif
