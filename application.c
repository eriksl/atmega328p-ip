#include "application.h"
#include "timer1.h"
#include "twi_master.h"
#include "stats.h"
#include "util.h"
#include "watchdog.h"
#include "stackmonitor.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static const __flash char string_usage[] =
	"command:\n"
	"\n"
	"  ?/h)help\n"
	"  e)echo\n\n"
	"  q)uit\n"
	"  R)eset\n"
	"  s)tats\n"
	"  r)ead twi\n"
	"  w)write twi\n"
	"  i)nit/reset twi\n"
	"  S)tack usage\n";

static const __flash char synhexaddr[]	= "Syntax error (hex/addr)\n";

static void twi_error(uint16_t size, uint8_t *dst, uint8_t error)
{
	static const __flash char format_string[] = "Error: %x, state %x\n";

	snprintf_P((char *)dst, (size_t)size, format_string,
			error & 0x0f, (error & 0xf0) >> 4);
}

static void twi_reset(uint16_t size, uint8_t *dst)
{
	static const __flash char return_string[] = "Reset ok\n";

	twi_master_recover();
	strlcpy_P((char *)dst, return_string, size);
}

static void twi_read(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static const __flash char synhexdatalen[]	= "Syntax error (hex/datalen)\n";
	static const __flash char okdata[]			= "Ok, data:";

	uint8_t addr, rv, rlen, slen, current;
	uint8_t buffer[16];

	while((*src <= ' ') && (length > 0))
	{
		length--;
		src++;
	}

	if(!hex_to_int(&length, &src, &addr))
	{
		strlcpy_P((char *)dst, synhexaddr, size);
		return;
	}

	while((*src <= ' ') && (length > 0))
	{
		length--;
		src++;
	}

	if(!hex_to_int(&length, &src, &rlen))
	{
		strlcpy_P((char *)dst, synhexdatalen, size);
		return;
	}

	if(rlen > sizeof(buffer))
		rlen = sizeof(buffer);

	if((rv = twi_master_receive(addr, rlen, buffer)) != tme_ok)
		twi_error(size, dst, rv);
	else
	{
		strlcpy_P((char *)dst, okdata, size);
		slen = strlen((const char *)dst);

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
	static const __flash char synhexdata[]	= "Syntax error (hex/data)\n";
	static const __flash char ok[]			= "Ok\n";

	uint8_t addr, rv;
	uint8_t buffer[16];
	uint8_t buflen;

	while((*src <= ' ') && (length > 0))
	{
		length--;
		src++;
	}

	if(!hex_to_int(&length, &src, &addr))
	{
		strlcpy_P((char *)dst, synhexaddr, size);
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
			strlcpy_P((char *)dst, synhexdata, size);
			return;
		}
		buflen++;
	}

	if((rv = twi_master_send(addr, buflen, buffer)) != tme_ok)
		twi_error(size, dst, rv);
	else
		strlcpy_P((char *)dst, ok, size);
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
	static const __flash char stackfree_fmt [] = "Stackmonitor: %d bytes free\n";

	uint8_t cmd;

	if(size == 0)
		return(0);

	*dst = 0;

	if(length == 0)
		cmd = '?';
	else
		cmd = src[0];

	switch(cmd)
	{
		case(0xff):
		{
			return(0); // telnet protocol
		}

		case('e'):
		{
			length++; // room for null byte

			if(length > size)
				length = size;

			strlcat((char *)dst, (const char *)src, length);

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

		case('S'):
		{
			snprintf_P((char *)dst, (size_t)size, stackfree_fmt, stackmonitor_free());
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
			strlcat_P((char *)dst, string_usage, size);
			break;
		}
	}

	return(strlen((const char *)dst));
}
