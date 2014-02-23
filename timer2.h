#ifndef _timer2_h_
#define _timer2_h_

#include <stdint.h>

typedef enum
{
	timer2_off = 0,
	timer2_1,
	timer2_8,
	timer2_32,
	timer2_64,
	timer2_128,
	timer2_256,
	timer2_1024,
} timer2_prescaler_t;

void		timer2_init(uint8_t prescaler);
void		timer2_start(void);
void		timer2_stop(void);
void		timer2_set_oc2b(uint16_t value);
uint16_t	timer2_get_oc2b(void);

static inline uint16_t timer2_get_counter(void)
{
	return(TCNT2);
}

#endif
