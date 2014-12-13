#include "application.h"
#include "application-uart.h"
#include "uart.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t application_function_uart_baud(application_parameters_t ap)
{
	static const __flash char ok[] = "> uart baud %lu\n";
	uint32_t baud = strtoul((const char *)(*ap.args)[1], 0, 0);

	uart_baud(baud);

	snprintf_P((char *)ap.dst, (size_t)ap.size, ok, baud);

	return(1);
}

uint8_t application_function_uart_transmit(application_parameters_t ap)
{
	static const __flash char ok[] = "> uart transmit %d bytes: ";
	static const __flash char error[] = "> uart transmit cmd too short(%d)\n";
	static const __flash char nl[] = "\n";
	static const char *crnl = "\r\n";
	uint16_t written;
	uint16_t length;

	if(ap.cmdline_length < 3)
	{
		snprintf_P((char *)ap.dst, (size_t)ap.size, error, ap.cmdline_length);
	}
	else
	{
		ap.cmdline_length -= 3;
		ap.cmdline += 3;

		written = uart_transmit(ap.cmdline_length, ap.cmdline);
		written += uart_transmit(strlen(crnl), (const uint8_t *)crnl);

		length = snprintf_P((char *)ap.dst, (size_t)ap.size, ok, written);
		ap.dst += length;
		ap.size -= length;

		if(ap.cmdline_length < ap.size)
		{
			memcpy(ap.dst, ap.cmdline, ap.cmdline_length);
			ap.dst += ap.cmdline_length;
			ap.size -= ap.cmdline_length;
		}

		length = strlen_P(nl);

		if(length < ap.size)
		{
			memcpy_P(ap.dst, nl, strlen_P(nl));
			ap.dst += length;
			ap.size -= length;
		}

		*ap.dst = '\0';
	}

	return(1);
}

uint8_t application_function_uart_receive(application_parameters_t ap)
{
	static const __flash char msg[] = "> uart receive: ->";
	static const __flash char nl[] = "<-\n";
	uint16_t length;

	strlcpy_P((char *)ap.dst, msg, (size_t)ap.size);
	length = strlen((const char *)ap.dst);
	ap.dst += length;
	ap.size -= length;

	length = uart_receive(ap.size - 1, ap.dst);
	ap.dst += length;
	ap.size -= length;

	strlcpy_P((char *)ap.dst, nl, (size_t)ap.size);

	return(1);
}
