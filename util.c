#include "util.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdint.h>

void reset(void)
{
	cli();

	PORTD |= _BV(3) | _BV(4);

	for(;;)
		(void)0;
}

void msleep(uint16_t ms)
{
	while(ms-- > 0)
		_delay_ms(1);
}

void pause_idle(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}
