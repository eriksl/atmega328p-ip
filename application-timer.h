#ifndef application_timer_h
#define application_timer_h

#include "application.h"

#include <stdint.h>

void	application_init_timer(void);
void	application_periodic_timer(uint16_t missed_ticks);
uint8_t application_function_beep(application_parameters_t ap);
uint8_t application_function_pwmr(application_parameters_t ap);
uint8_t application_function_pwmw(application_parameters_t ap);

#endif
