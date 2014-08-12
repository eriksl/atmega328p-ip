#include "application.h"
#include "timer1.h"
#include "twi_master.h"
#include "stats.h"
#include "util.h"
#include "watchdog.h"

#include <stdint.h>

static const __flash uint8_t string_usage[]			= "command:\n\n  e)echo\n\n  ?/h)help\n  q)uit\n  R)eset\n  s)tats\n  r)ead twi\n  w)write twi\n  i)nit/reset twi\n";
static const __flash uint8_t string_error[]			= "Error: ";
static const __flash uint8_t string_state[]			= ", state: ";
static const __flash uint8_t string_newline[]		= "\n";
static const __flash uint8_t string_resetok[]		= "Reset ok\n";
static const __flash uint8_t string_synhexaddr[]	= "Syntax error (hex/addr)\n";
static const __flash uint8_t string_synhexdatalen[]	= "Syntax error (hex/datalen)\n";
static const __flash uint8_t string_synhexdata[]	= "Syntax error (hex/data)\n";
static const __flash uint8_t string_ok[]			= "Ok\n";
static const __flash uint8_t string_okdata[]		= "Ok, data:";

static void twi_error(uint16_t size, uint8_t *dst, uint8_t error)
{
	static uint8_t numbuf[8];

	fxstrncpy(string_error, size, dst);
	int_to_str((error & 0x0f) >> 0, sizeof(numbuf), numbuf);
	xstrncat(numbuf, size, dst);
	fxstrncat(string_state, size, dst);
	int_to_str((error & 0xf0) >> 4, sizeof(numbuf), numbuf);
	xstrncat(numbuf, size, dst);
	fxstrncat(string_newline, size, dst);
}

static void twi_reset(uint16_t size, uint8_t *dst)
{
	twi_master_recover();
	fxstrncpy(string_resetok, size, dst);
}

static void twi_read(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static uint8_t addr, rv, rlen, slen, current;
	static uint8_t buffer[16];

	while((*src <= ' ') && (length > 0))
	{
		length--;
		src++;
	}

	if(!hex_to_int(&length, &src, &addr))
	{
		fxstrncpy(string_synhexaddr, size, dst);
		return;
	}

	while((*src <= ' ') && (length > 0))
	{
		length--;
		src++;
	}

	if(!hex_to_int(&length, &src, &rlen))
	{
		fxstrncpy(string_synhexdatalen, size, dst);
		return;
	}

	if(rlen > sizeof(buffer))
		rlen = sizeof(buffer);

	if((rv = twi_master_receive(addr, rlen, buffer)) != tme_ok)
		twi_error(size, dst, rv);
	else
	{
		fxstrncpy(string_okdata, size, dst);
		slen = xstrlen(dst);

		if(slen < size)
		{
			dst += slen;
			size -= slen;
		}

		for(current = 0; (current < rlen) && ((current + 3) < size); current++, dst += 3, size -= 3)
		{
			dst[0] = ' ';
			int_to_hex(dst + 1, buffer[current]);
		}

		if(size > 1)
			*dst++ = '\n';

		*dst = '\0';
	}
}

static void twi_write(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static uint8_t addr, rv;
	static uint8_t buffer[16];
	static uint8_t buflen;

	while((*src <= ' ') && (length > 0))
	{
		length--;
		src++;
	}

	if(!hex_to_int(&length, &src, &addr))
	{
		fxstrncpy(string_synhexaddr, size, dst);
		return;
	}

	buflen = 0;

	while((buflen < sizeof(buffer)) && (length > 1))
	{
		while((*src <= ' ') && (length > 0))
		{
			length--;
			src++;
		}

		if(length < 2)
			break;

		if(!hex_to_int(&length, &src, &buffer[buflen]))
		{
			fxstrncpy(string_synhexdata, size, dst);
			return;
		}
		buflen++;
	}

	if((rv = twi_master_send(addr, buflen, buffer)) != tme_ok)
		twi_error(size, dst, rv);
	else
		fxstrncpy(string_ok, size, dst);
}

uint8_t application_init(void)
{
	timer1_init_pwm1a1b(timer1_1);	// pwm timer 1 resolution: 16 bits, frequency = 122 Hz
	timer1_start();

	return(WATCHDOG_PRESCALER_8192);
}

void application_idle(void)
{
	static uint8_t phase = 0;

	timer1_set_oc1a(_BV(phase));
	timer1_set_oc1b(_BV(15 - phase));

	if(++phase > 15)
		phase = 0;
}

int16_t application_content(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static uint8_t cmd;

	if(size == 0)
		return(0);

	*dst = 0;

	if(length == 0)
		cmd = '?';
	else
		cmd = src[0];

	switch(cmd)
	{
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

		case('R'):
		{
			reset();

			break;
		}

		case('s'):
		{
			stats_generate(size, dst);
			break;
		}

		case('r'):
		{
			twi_read(length - 1, src + 1, size, dst);
			break;
		}

		case('w'):
		{
			twi_write(length - 1, src + 1, size, dst);
			break;
		}

		case('i'):
		{
			twi_reset(size, dst);
			break;
		}

		default:
		{
			fxstrncat(string_usage, size, dst);
			break;
		}
	}

	return((int16_t)xstrlen(dst));
}
