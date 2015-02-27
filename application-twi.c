#include "application.h"
#include "application-twi.h"
#include "twi_master.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t twi_address = 0;

uint8_t application_function_twiaddress(application_parameters_t ap)
{
	static const __flash char fmt[] = "> TWI slave address set to 0x%02x\n";

	twi_address = (uint8_t)strtoul((const char *)(*ap.args)[1], 0, 16);

	snprintf_P((char *)ap.dst, (size_t)ap.size, fmt, twi_address);

	return(1);
}

uint8_t application_function_twiread(application_parameters_t ap)
{
	static const __flash char header[] = "> TWI read %d bytes from %02x:";
	static const __flash char entry[] = " %02x";
	static const __flash char footer[] = "\n";
	static const __flash char error[] = "> TWI read error, read max %d bytes: %d\n";

	uint8_t src_current, rv, offset, amount;
	uint8_t bytes[8];

	amount = (uint8_t)strtoul((const char *)(*ap.args)[1], 0, 0);

	if(amount > sizeof(bytes))
	{
		snprintf_P((char *)ap.dst, (size_t)ap.size, error, sizeof(bytes), amount);
		return(1);
	}

	if((rv = twi_master_receive(twi_address, sizeof(bytes), bytes)) != tme_ok)
		twi_master_error(ap.dst, ap.size, rv);
	else
	{
		offset = snprintf_P((char *)ap.dst, (size_t)ap.size, header, amount, twi_address);
		ap.dst += offset;
		ap.size -= offset;

		for(src_current = 0; (src_current < amount) && (ap.size > 0); src_current++)
		{
			offset = snprintf_P((char *)ap.dst, (size_t)ap.size, entry, bytes[src_current]);
			ap.dst += offset;
			ap.size -= offset;
		}

		strlcpy_P((char *)ap.dst, footer, (size_t)ap.size);
	}

	return(1);
}

uint8_t application_function_twireset(application_parameters_t ap)
{
	static const __flash char ok[] = "> TWI reset ok\n";

	twi_master_recover();

	strlcpy_P((char *)ap.dst, ok, ap.size);

	return(1);
}

uint8_t application_function_twiwrite(application_parameters_t ap)
{
	static const __flash char fmt[] = "> TWI written %d bytes to %02x\n";

	uint8_t src_current, dst_current, rv;
	uint8_t bytes[8];

	for(src_current = 1, dst_current = 0;
			(src_current < ap.nargs) && (dst_current < sizeof(bytes));
			src_current++, dst_current++)
	{
		bytes[dst_current] = (uint8_t)strtoul((const char *)(*ap.args)[src_current], 0, 16);
	}

	if((rv = twi_master_send(twi_address, dst_current, bytes)) != tme_ok)
		twi_master_error(ap.dst, ap.size, rv);
	else
		snprintf_P((char *)ap.dst, (size_t)ap.size, fmt, dst_current, twi_address);

	return(1);
}
