#include "application.h"
#include "application-twi.h"
#include "twi_master.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t twi_address = 0;

static void twi_error(uint8_t *dst, uint16_t size, uint8_t error)
{
	static const __flash char format_string[] = "TWI error: %x, state %x\n";

	snprintf_P((char *)dst, (size_t)size, format_string,
			error & 0x0f, (error & 0xf0) >> 4);
}

uint8_t application_function_twiaddress(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char fmt[] = "> TWI slave address set to 0x%02x\n";

	twi_address = (uint8_t)strtoul((const char *)args[1], 0, 16);

	snprintf_P((char *)dst, (size_t)size, fmt, twi_address);

	return(1);
}

uint8_t application_function_twiread(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char header[] = "> TWI read %d bytes from %02x:";
	static const __flash char entry[] = " %02x";
	static const __flash char footer[] = "\n";
	static const __flash char error[] = "> TWI read error, read max %d bytes: %d\n";

	uint8_t src_current, rv, offset, amount;
	uint8_t bytes[8];

	amount = (uint8_t)strtoul((const char *)args[1], 0, 0);

	if(amount > sizeof(bytes))
	{
		snprintf_P((char *)dst, (size_t)size, error, sizeof(bytes), amount);
		return(1);
	}

	if((rv = twi_master_receive(twi_address, sizeof(bytes), bytes)) != tme_ok)
		twi_error(dst, size, rv);
	else
	{
		offset = snprintf_P((char *)dst, (size_t)size, header, amount, twi_address);
		dst += offset;
		size -= offset;

		for(src_current = 0; (src_current < amount) && (size > 0); src_current++)
		{
			offset = snprintf_P((char *)dst, (size_t)size, entry, bytes[src_current]);
			dst += offset;
			size -= offset;
		}

		strlcpy_P((char *)dst, footer, (size_t)size);
	}

	return(1);
}

uint8_t application_function_twireset(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> TWI reset ok\n";

	twi_master_recover();

	strlcpy_P((char *)dst, ok, size);

	return(1);
}

uint8_t application_function_twiwrite(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char fmt[] = "> TWI written %d bytes to %02x\n";

	uint8_t src_current, dst_current, rv;
	uint8_t bytes[8];

	for(src_current = 1, dst_current = 0;
			(src_current < nargs) && (dst_current < sizeof(bytes));
			src_current++, dst_current++)
	{
		bytes[dst_current] = (uint8_t)strtoul((const char *)args[src_current], 0, 16);
	}

	if((rv = twi_master_send(twi_address, dst_current, bytes)) != tme_ok)
		twi_error(dst, size, rv);
	else
		snprintf_P((char *)dst, (size_t)size, fmt, dst_current, twi_address);

	return(1);
}
