#include "util.h"
#include "watchdog.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdint.h>

void reset(void)
{
	cli();
	watchdog_stop();
	watchdog_start(WATCHDOG_PRESCALER_16);
	for(;;)
		sleep_mode();
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

void int_to_str(uint16_t in, uint8_t outlen, uint8_t *out)
{
	uint16_t divisor;
	uint8_t current;
	uint8_t leading;

	divisor = 10000;
	leading = 1;

	while((outlen > 1) && (divisor > 0))
	{
		current = in / divisor;

		if((current > 0) || !leading)
		{
			leading = 0;
			*out++ = '0' + current;
			outlen--;
			in -= current * divisor;
		}
		
		divisor /= 10;
	}

	if(leading && (outlen > 1))
		*out++ = '0';

	*out = 0;
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

void xstrncpy(const uint8_t *in, uint16_t outlen, uint8_t *out)
{
	for(; (outlen > 1) && *in; outlen--, in++, out++)
		*out = *in;

	*out = 0;
}

void fxstrncpy(const __flash uint8_t *in, uint16_t outlen, uint8_t *out)
{
	for(; (outlen > 1) && *in; outlen--, in++, out++)
		*out = *in;

	*out = 0;
}

void xstrncat(const uint8_t *in, uint16_t outlen, uint8_t *out)
{
	const uint8_t *from;
	uint8_t *to;

	for(to = out; (outlen > 1) && *to; outlen--, to++)
		(void)0;

	for(from = in; (outlen > 1) && *from; outlen--, from++, to++)
		*to = *from;

	*to = 0;
}

void fxstrncat(const __flash uint8_t *in, uint16_t outlen, uint8_t *out)
{
	const __flash uint8_t *from;
	uint8_t *to;

	for(to = out; (outlen > 1) && *to; outlen--, to++)
		(void)0;

	for(from = in; (outlen > 1) && *from; outlen--, from++, to++)
		*to = *from;

	*to = 0;
}

uint16_t xstrlen(const uint8_t *str)
{
	uint16_t length;
	
	length = 0;

	while(*str)
	{
		str++;
		length++;
	}

	return(length);
}
