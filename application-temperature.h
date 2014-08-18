#ifndef application_temperature_h
#define application_temperature_h

#include "application.h"

#include <stdint.h>

extern const __flash char description_temp_read[];
extern const __flash char description_temp_write[];

extern void application_init_temp_read(void);
extern uint8_t application_function_temp_read(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
extern uint8_t application_function_temp_write(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
#endif
