#ifndef clock_h
#define clock_h

#include "stats.h"
#include <stdint.h>

#define JIFFIES_PER_SECOND	((float)F_CPU / 65536)
#define JIFFIES_PER_DAY		((uint32_t)(JIFFIES_PER_SECOND * 60 * 60 * 24))

static inline void clock_update(void)
{
	if(t1_jiffies < JIFFIES_PER_DAY)
		t1_jiffies++;
	else
		t1_jiffies = 0;
}

void clock_get(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void clock_set(uint8_t hours, uint8_t minutes, uint8_t seconds);

#endif
