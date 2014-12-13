#ifndef application_light_h
#define application_light_h

#include "application.h"

#include <stdint.h>

extern void application_init_light(void);
extern uint8_t application_function_light_read(application_parameters_t);
extern uint8_t application_function_light_write(application_parameters_t);
#endif
