#include "application-led-display.h"
#include "led-display.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t application_function_dbr(application_parameters_t ap)
{
	static const __flash uint8_t ok[] = "> dbr %d\n";
    int16_t level;

    level = atoi((*ap.args)[1]);

	level = led_display_brightness(level);

	snprintf_P(ap.dst, ap.size, ok, level);

	return(1);
}

uint8_t application_function_dclr(application_parameters_t ap)
{
    static const __flash uint8_t ok[] = "> dclr\n";

	led_display_clear();

    snprintf_P(ap.dst, ap.size, ok);

    return(1);
}

uint8_t application_function_dshow(application_parameters_t ap)
{
    static const __flash uint8_t ok[] = "> dshow \"";

    if(strlen(ap.cmdline) >= 6)
    {
		led_display_show_chunk(strlen(ap.cmdline) - 6, &ap.cmdline[6]);

		strncpy_P(ap.dst, ok, ap.size);
		ap.size -= strlen(ok);
		strncat(ap.dst, &ap.cmdline[6], ap.size);
		ap.size -= strlen(&ap.cmdline[6]);
		strncat(ap.dst, "\"\n", ap.size);
    }
    else
        snprintf_P(ap.dst, ap.size, ok, "<error>");

    return(1);
}

uint8_t application_function_dhshw(application_parameters_t ap)
{
	led_display_clear();

	return(application_function_dshow(ap));
}
