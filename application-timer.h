#ifndef application_timer_h
#define application_timer_h

#include "application.h"

#include <stdint.h>

void	application_init_timer(void);
void	application_periodic_timer(uint16_t missed_ticks);
uint8_t application_function_beep(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
uint8_t application_function_pwmw(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
uint8_t application_function_clockr(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
uint8_t application_function_clockw(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);

#define JIFFIES_PER_SECOND	((float)F_CPU / 65536)
#define JIFFIES_PER_DAY		((uint32_t)(JIFFIES_PER_SECOND * 60 * 60 * 24))

#endif
