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

static void twi_reset(uint16_t size, uint8_t *dst)
{
	twi_master_recover();
	xstrncpy((uint8_t *)"Reset ok\n", size, dst);
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
		xstrncpy((uint8_t *)"Syntax error (hex/addr)\n", size, dst);
		return;
	}

	while((*src <= ' ') && (length > 0))
	{
		length--;
		src++;
	}

	if(!hex_to_int(&length, &src, &rlen))
	{
		xstrncpy((uint8_t *)"Syntax error (hex/datalen)\n", size, dst);
		return;
	}

	if(rlen > sizeof(buffer))
		rlen = sizeof(buffer);

	if((rv = twi_master_receive(addr, rlen, buffer)) != tme_ok)
		twi_error(size, dst, rv);
	else
	{
		xstrncpy((uint8_t *)"Ok, data:", size, dst);
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
		xstrncpy((uint8_t *)"Syntax error (hex/addr)\n", size, dst);
		return;
	}

	buflen = 0;

	while((buflen < sizeof(buffer)) && (length > 0))
	{
		while((*src <= ' ') && (length > 0))
		{
			length--;
			src++;
		}

		if(!hex_to_int(&length, &src, &buffer[buflen]))
		{
			xstrncpy((uint8_t *)"Syntax error (hex/data)\n", size, dst);
			return;
		}
		buflen++;
	}

	if((rv = twi_master_send(addr, buflen, buffer)) != tme_ok)
		twi_error(size, dst, rv);
	else
		xstrncpy((uint8_t *)"Ok\n", size, dst);
}

int16_t content(uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static uint8_t conv[8];
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
			xstrncat((uint8_t *)"command:\n\n  e)echo\n\n  ?/h)help\n  q)uit\n  R)eset\n  s)tats\n  r)ead twi\n  w)write twi\n  i)nit/reset twi", size, dst);
			break;
		}
	}

	return((int16_t)xstrlen(dst));
}
