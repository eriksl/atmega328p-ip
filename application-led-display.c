#include "application-led-display.h"
#include "led-display.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t application_function_bright(application_parameters_t ap)
{
	static const __flash char format[] = "> brightness: %u\n";

	uint8_t level;

	level = (uint8_t)atoi((const char *)(*ap.args)[1]);

	display_brightness(level);

	snprintf_P((char *)ap.dst, ap.size, format, (unsigned int)level & 0x07);

	return(1);
}

uint8_t application_function_show(application_parameters_t ap)
{
	static const __flash char format[] = "> show: %d\n";

	uint8_t ix;

	for(ix = 0; (ix + 1) < application_num_args; ix++)
	{
		if((ix + 1) < ap.nargs)
			strncpy((char *)display_string[ix], (const char *)(*ap.args)[ix + 1], 4);
		else
			display_string[ix][0] = '\0';
	}

	snprintf_P((char *)ap.dst, ap.size, format, ap.nargs - 1);

	return(1);
}
