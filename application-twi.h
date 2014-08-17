#ifndef application_twi_h
#define application_twi_h

#include "application.h"

#include <stdint.h>

extern const __flash char description_twiaddress[];
extern const __flash char description_twiread[];
extern const __flash char description_twireset[];
extern const __flash char description_twiwrite[];

extern uint8_t application_function_twiaddress(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
extern uint8_t application_function_twiread(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
extern uint8_t application_function_twireset(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
extern uint8_t application_function_twiwrite(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);

#endif
