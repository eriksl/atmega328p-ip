#ifndef application_temperature_h
#define application_temperature_h

#include "application.h"

#include <stdint.h>

extern void application_init_temp(void);
extern uint8_t application_function_temp_read(application_parameters_t);
extern uint8_t application_function_temp_write(application_parameters_t);
extern uint8_t application_function_bg_write(application_parameters_t);
#endif
