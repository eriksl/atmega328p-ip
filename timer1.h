#ifndef _timer1_h_
#define _timer1_h_

#include <avr/io.h>

#include <stdint.h>

typedef enum
{
	timer1_off = 0,
	timer1_1,
	timer1_8,
	timer1_64,
	timer1_256,
	timer1_1024,
	timer1_ext_falling,
	timer1_ext_rising,
} timer1_prescaler_t;

void		timer1_init_pwm1a1b(uint8_t prescaler);
void		timer1_start(void);
void		timer1_stop(void);
void		timer1_set_oc1a(uint16_t value);
uint16_t	timer1_get_oc1a(void);
void		timer1_set_oc1b(uint16_t value);
uint16_t	timer1_get_oc1b(void);

static inline uint16_t timer1_get_counter(void)
{
	return(TCNT1);
}

#endif
