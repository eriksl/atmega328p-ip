#include "application-vfd.h"

#include "twi_master.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void application_init_vfd(void)
{
	static const uint8_t init_vfd[] =
	{
		0xfe, 'C',	// auto line wrapping on
		0xfe, 'R',	// auto scroll off
		0xfe, 'T',	// cursor blink off
		0xfe, 'K',	// cursor off
		0xfe, 'X', 	// clear screen
		'O', 'K',
		0x00
	};

	twi_master_send(0x28, strlen(init_vfd), init_vfd);
}

uint8_t application_function_dbr(application_parameters_t ap)
{
	static const __flash char ok[] = "> dbr %d\n";

	static const uint8_t code_on[]		= { 0xfe, 'B', 0 };
	static const uint8_t code_off[]		= { 0xfe, 'F' };
	static const uint8_t code_bright[]	= { 0xfe, 'Y' };
	static const uint8_t translate[]	= { 0x03, 0x03, 0x02, 0x01, 0x00 };
	int16_t bright;

	bright = atoi((*ap.args)[1]);

	if(bright < 0)
		bright = 0;

	if(bright > 4)
		bright = 4;

	if(bright)
		twi_master_send(0x28, sizeof(code_on), code_on);
	else
		twi_master_send(0x28, sizeof(code_off), code_off);

	twi_master_send(0x28, sizeof(code_bright), code_bright);
	twi_master_send(0x28, 1, &translate[bright]);

	snprintf_P(ap.dst, ap.size, ok, bright);

	return(1);
}

uint8_t application_function_dclr(application_parameters_t ap)
{
	static const			uint8_t clr[]	= { 0xfe, 'X', 0 };
	static const __flash	char ok[]		= "> dclr\n";

	twi_master_send(0x28, strlen(clr), clr);
	snprintf_P(ap.dst, ap.size, ok);

	return(1);
}

uint8_t application_function_dshow(application_parameters_t ap)
{
	static const __flash char ok[] = "> dshow \"%s\"\n";

	if(strlen(ap.cmdline) >= 6)
	{
		twi_master_send(0x28, strlen(ap.cmdline) - 6, &ap.cmdline[6]);

		snprintf_P(ap.dst, ap.size, ok, &ap.cmdline[6]);
	}
	else
		snprintf_P(ap.dst, ap.size, ok, "<error>");

	return(1);
}

uint8_t application_function_dhshw(application_parameters_t ap)
{
	static const __flash char ok[] = "> dhshw \"%s\"\n";
	static const uint8_t home[] = { 0xfe, 'H' };

	if(strlen(ap.cmdline) >= 6)
	{
		twi_master_send(0x28, sizeof(home), home);
		twi_master_send(0x28, strlen(ap.cmdline) - 6, &ap.cmdline[6]);

		snprintf_P(ap.dst, ap.size, ok, &ap.cmdline[6]);
	}
	else
		snprintf_P(ap.dst, ap.size, ok, "<error>");

	return(1);
}
