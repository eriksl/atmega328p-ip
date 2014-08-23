#ifndef application_pwm_h
#define application_pwm_h

#include <stdint.h>

void application_init_pwm(void);
void application_periodic_pwm(uint16_t missed_ticks);

#endif
