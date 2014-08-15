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

void pause(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}

uint8_t hex_to_int(uint16_t *length, uint8_t const **in, uint8_t *value)
{
	if(*length < 2)
		return(0);

	if((**in >= '0') && (**in <= '9'))
		*value = **in - '0';
	else
		if((**in >= 'a') && (**in <= 'f'))
			*value = **in - 'a' + 10;
		else
			return(0);

	(*length)--;
	(*in)++;

	*value <<= 4;

	if((**in >= '0') && (**in <= '9'))
		*value |= **in - '0';
	else
		if((**in >= 'a') && (**in <= 'f'))
			*value |= **in - 'a' + 10;
		else
			return(0);

	(*length)--;
	(*in)++;

	return(1);
}

void int_to_hex(uint8_t *buffer, uint8_t value)
{
	uint8_t nibble_high, nibble_low;

	nibble_high = (value & 0xf0) >> 4;
	nibble_low  = (value & 0x0f) >> 0;

	if(nibble_high < 10)
		buffer[0] = nibble_high + '0';
	else
		buffer[0] = nibble_high - 10 + 'a';

	if(nibble_low < 10)
		buffer[1] = nibble_low + '0';
	else
		buffer[1] = nibble_low - 10 + 'a';
}
