#include <avr/io.h>

#include "content.h"
#include "twi_master.h"
#include "stats.h"
#include "util.h"

static void twi_error(uint16_t size, uint8_t *dst, uint8_t error)
{
	static uint8_t numbuf[8];

	xstrncpy((uint8_t *)"Error: ", size, dst);
	int_to_str((error & 0x0f) >> 0, sizeof(numbuf), numbuf);
	xstrncat(numbuf, size, dst);
	xstrncat((uint8_t *)", state: ", size, dst);
	int_to_str((error & 0xf0) >> 4, sizeof(numbuf), numbuf);
	xstrncat(numbuf, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);
}

static void twi_receive(uint16_t size, uint8_t *dst)
{
	static const uint8_t set_ddr[]		= { 0x00, ~(_BV(2) | _BV(3)) };
	static const uint8_t set_pullup[]	= { 0x06, _BV(1) };
	static const uint8_t select_gpio[]	= { 0x09 };
	static uint8_t gpio, rv;
	static uint8_t numbuf[4];

	if((rv = twi_master_send(0x20, sizeof(set_ddr), set_ddr)) != tme_ok)
		goto error;

	if((rv = twi_master_send(0x20, sizeof(set_pullup), set_pullup)) != tme_ok)
		goto error;

	if((rv = twi_master_send(0x20, sizeof(select_gpio), select_gpio)) != tme_ok)
		goto error;

	if((rv = twi_master_receive(0x20, 1, &gpio)) != tme_ok)
		goto error;

	xstrncpy((uint8_t *)"OK: ", size, dst);
	int_to_str(gpio, sizeof(numbuf), numbuf);
	xstrncat(numbuf, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	return;

error:
	twi_error(size, dst, rv);
	twi_master_recover();
}

static void twi_send(uint16_t size, uint8_t *dst, uint8_t val)
{
	static uint8_t rv;

	static const uint8_t set_ddr[] = { 0x00, ~(_BV(2) | _BV(3)) };
	static uint8_t select_gpio[2];

	if((rv = twi_master_send(0x20, sizeof(set_ddr), set_ddr)) != tme_ok)
		goto error;

	select_gpio[0] = 0x09;
	select_gpio[1] = ~val;

	if((rv = twi_master_send(0x20, sizeof(select_gpio), select_gpio)) != tme_ok)
		goto error;

	xstrncpy((uint8_t *)"OK\n", size, dst);

	return;

error:
	twi_error(size, dst, rv);
	twi_master_recover();
}

int16_t content(uint16_t port, uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static uint8_t conv[8];
	static uint8_t cmd;

	if(port == 28022)
	{
		dst[0] = 't';
		dst[1] = 'e';
		dst[2] = 's';
		dst[3] = 't';
		dst[4] = '\n';

		return(5);
	}
	else
	{
		if(size == 0)
			return(0);

		*dst = 0;

		if(length == 0)
			cmd = '?';
		else
			cmd = src[0];

		switch(cmd)
		{
			case('0'):
			{
				twi_send(size, dst, 0x00);
				break;
			}

			case('1'):
			{
				twi_send(size, dst, _BV(2));
				break;
			}

			case('2'):
			{
				twi_send(size, dst, _BV(3));
				break;
			}

			case('3'):
			{
				twi_send(size, dst, _BV(2) | _BV(3));
				break;
			}

			case('4'):
			{
				twi_receive(size, dst);
				break;
			}

			case('e'):
			{
				length++; // room for null byte

				if(length > size)
					length = size;

				xstrncat(src, length, dst);

				break;
			}

			case('q'):
			{
				return(-1);
				break;
			}

			case('r'):
			{
				// let watchdog do it's job
				for(;;)
					(void)0;

				break;
			}

			case('s'):
			{
				stats_generate(size, dst);
				break;
			}

			default:
			{
				xstrncat((uint8_t *)"command:\n\n  e)echo\n\n  ?/h)help\n  q)uit\n  r)eset\n  s)tats\n", size, dst);
				break;
			}
		}
	}

	return((int16_t)xstrlen(dst));
}
