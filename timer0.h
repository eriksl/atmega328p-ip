#ifndef _timer0_h_
#define _timer0_h_

#include <stdint.h>

typedef enum
{
	timer0_off = 0,
	timer0_1,
	timer0_8,
	timer0_64,
	timer0_256,
	timer0_1024,
	timer0_ext_falling,
	timer0_ext_rising,
} timer0_prescaler_t;

void		timer0_init(uint8_t prescaler);
void		timer0_start(void);
void		timer0_stop(void);
void		timer0_set_oc0a(uint16_t value);
uint16_t	timer0_get_oc0a(void);
void		timer0_set_oc0b(uint16_t value);
uint16_t	timer0_get_oc0b(void);

static inline uint16_t timer0_get_counter(void)
{
	return(TCNT0);
}

#endif
