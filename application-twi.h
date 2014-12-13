#ifndef application_twi_h
#define application_twi_h

#include "application.h"

#include <stdint.h>

extern uint8_t application_function_twiaddress(application_parameters_t);
extern uint8_t application_function_twiread(application_parameters_t);
extern uint8_t application_function_twireset(application_parameters_t);
extern uint8_t application_function_twiwrite(application_parameters_t);

#endif
