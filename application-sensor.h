#ifndef application_sensor_h
#define application_sensor_h

#include "application.h"

#include <stdint.h>

extern void application_init_sensor(void);
extern uint8_t application_sensor_read(uint8_t sensor, uint16_t size, uint8_t *dst);
extern uint8_t application_function_sensor_read(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
extern uint8_t application_function_sensor_write(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
extern uint8_t application_function_bg_write(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
#endif
