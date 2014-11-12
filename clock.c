#include "clock.h"
#include "stats.h"

#include <avr/interrupt.h>
#include <stdint.h>

void clock_get(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
	uint32_t jiffies;

	cli();
	jiffies = t1_jiffies;
	sei();

	jiffies	= (uint32_t)((float)jiffies / JIFFIES_PER_SECOND);
	*hours	= jiffies / (60UL * 60UL);
	jiffies	= jiffies - (*hours * 60UL * 60UL);
	*minutes= jiffies / 60UL;
	jiffies	= jiffies - (*minutes * 60UL);
	*seconds= jiffies;
}

void clock_set(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	uint32_t jiffies;

	jiffies	= (uint32_t)seconds + ((uint32_t)minutes * 60) + ((uint32_t)hours * 60 * 60);
	jiffies	= (uint32_t)((float)jiffies * JIFFIES_PER_SECOND);

	cli();
	t1_jiffies = jiffies;
	sei();
}
