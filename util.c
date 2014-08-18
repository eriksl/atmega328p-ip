#include "util.h"
#include "watchdog.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdint.h>

void reset(void)
{
	for(;;)
		(void)0;
}

void sleep(uint16_t ms)
{
	while(ms-- > 0)
	{
		_delay_ms(1);
		watchdog_rearm();
	}
}

void pause_idle(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}

void pause_adc(void)
{
	set_sleep_mode(SLEEP_MODE_ADC);
	sleep_mode();
}
