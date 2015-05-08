#include "application.h"
#include "application-sensor.h"
#include "application-twi.h"
#include "application-timer.h"
#include "application-vfd.h"
#include "stats.h"
#include "util.h"
#include "stackmonitor.h"
#include "eeprom.h"
#include "sensor.h"
#include "esp.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	const uint8_t	command[7];
	uint8_t			required_args;
	uint8_t			(*function)(application_parameters_t);
	const uint8_t	description[37];
} application_function_table_t;

static uint8_t cmd_led_timeout = 0;

static uint8_t application_function_edmp(application_parameters_t ap);
static uint8_t application_function_help(application_parameters_t ap);
static uint8_t application_function_reset(application_parameters_t ap);
static uint8_t application_function_stack(application_parameters_t ap);
static uint8_t application_function_stats(application_parameters_t ap);

static const __flash application_function_table_t application_function_table[] =
{
	{
		"bgw",
		1,
		application_function_bg_write,
		"write bandgap caliburation (V)",
	},
	{
		"dbr",
		1,
		application_function_dbr,
		"display brightness",
	},
	{
		"dclr",
		0,
		application_function_dclr,
		"clear display",
	},
	{
		"dhshw",
		0,
		application_function_dhshw,
		"home and show text",
	},
	{
		"dshow",
		0,
		application_function_dshow,
		"show text on display",
	},
	{
		"edmp",
		0,
		application_function_edmp,
		"dump eeprom contents",
	},
	{
		"help",
		0,
		application_function_help,
		"help (command)",
	},
	{
		"?",
		0,
		application_function_help,
		"help (command)",
	},
	{
		"outd",
		0,
		application_function_output_dump,
		"dump all outputs"
	},
	{
		"outr",
		1,
		application_function_output_read,
		"read output ix",
	},
	{
		"outs",
		1,
		application_function_output_set,
		"set output ix val/min[ spd max]",
	},
	{
		"reset",
		0,
		application_function_reset,
		"reset using watchdog timeout",
	},
	{
		"S",
		0,
		application_function_stack,
		"free memory (stack usage)",
	},
	{
		"stack",
		0,
		application_function_stack,
		"free memory (stack usage)",
	},
	{
		"s",
		0,
		application_function_stats,
		"statistics",
	},
	{
		"stats",
		0,
		application_function_stats,
		"statistics",
	},
	{
		"sdmp",
		0,
		application_function_sdmp,
		"dump sensors values",
	},
	{
		"sensr",
		1,
		application_function_sensor_read,
		"read sensor",
	},
	{
		"sensw",
		3,
		application_function_sensor_write,
		"write sensor calibr: factor/offset",
	},
	{
		"twia",
		1,
		application_function_twiaddress,
		"set twi slave address",
	},
	{
		"twir",
		1,
		application_function_twiread,
		"read bytes from twi slave",
	},
	{
		"twirst",
		0,
		application_function_twireset,
		"twi interface reset",
	},
	{
		"twiw",
		1,
		application_function_twiwrite,
		"write bytes to twi slave",
	},
	{
		"",
		0,
		(void *)0,
		"",
	},
};

void application_init(void)
{
	application_init_sensor();
	application_init_timer();
	application_init_vfd();
}

void application_periodic(void)
{
	uint16_t missed_ticks;

	if((missed_ticks = t1_unhandled) == 0)
		return;

	t1_unhandled = 0;

	if(missed_ticks > t1_unhandled_max)
		t1_unhandled_max = missed_ticks;

	if(cmd_led_timeout > missed_ticks)
		cmd_led_timeout -= missed_ticks;
	else
	{
		cmd_led_timeout = 0;
		PORTD &= ~_BV(3);
	}

	application_periodic_timer(missed_ticks);
	application_periodic_sensor(missed_ticks);

	if((t1_interrupts % 122) == 0)
		PORTD ^= _BV(4);
}

uint8_t application_content(const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static const __flash uint8_t error_fmt_unknown[] = "Command \"%s\" unknown\n";
	static const __flash uint8_t error_fmt_args[] = "Insufficient arguments: %d (%d required)\n";

	args_t	args;
	uint8_t args_count, arg_current;
	uint8_t src_current = 0;
	uint16_t src_left;
	uint8_t ws_skipped;
	const application_function_table_t __flash *tableptr;

	cmd_led_timeout = 10;
	PORTD |= _BV(3);

	*dst = '\0';

	if(src[0] == '\0')
		return(1);

	src_left = strlen(src);

	for(args_count = 0; (src_left > 0) && (args_count < application_num_args);)
	{
		ws_skipped = 0;

		for(arg_current = 0;
				(src_left > 0) && (arg_current < (application_length_args - 1));
				src_current++, src_left--)
		{
			if((src[src_current] <= ' ') || (src[src_current] > '~'))
			{
				if(!ws_skipped)
					continue;
				else
					break;
			}

			ws_skipped = 1;

			args[args_count][arg_current++] = src[src_current];
		}

		args[args_count][arg_current] = '\0';

		if(arg_current)
			args_count++;

		while((src_left > 0) && (src[src_current] > ' ') && (src[src_current] <= '~'))
		{
			src_left--;
			src_current++;
		}
	}

	if(args_count == 0)
		return(1);

	for(tableptr = application_function_table; tableptr->function; tableptr++)
		if(!strcmp_P(args[0], tableptr->command))
			break;

	if(tableptr->function)
	{
		if(args_count < (tableptr->required_args + 1))
		{
			snprintf_P(dst, size, error_fmt_args, args_count - 1, tableptr->required_args);
			return(1);
		}

		application_parameters_t ap;

		ap.cmdline			= src;
		ap.nargs			= args_count;
		ap.args				= &args;
		ap.size				= size;
		ap.dst				= dst;

		return(tableptr->function(ap));
	}

	snprintf_P(dst, size, error_fmt_unknown, args[0]);
	return(1);
}

static uint8_t application_function_edmp(application_parameters_t ap)
{
	static const __flash uint8_t format1[] = "> bg: %.3f\n";
	static const __flash uint8_t format2[] = "> sensor[%d]: factor %.3f, offset %.3f\n";

	uint8_t index, offset;
	float cfactor, coffset;

	offset	= snprintf_P(ap.dst, ap.size, format1, eeprom_read_bandgap());
	ap.dst	+= offset;
	ap.size	-= offset;

	index = 0;

	while(eeprom_read_cal(index, &cfactor, &coffset))
	{
		offset	= snprintf_P(ap.dst, ap.size, format2, index, cfactor, coffset);
		ap.dst	+= offset;
		ap.size	-= offset;
		index++;
	}

	return(1);
}

static uint8_t application_function_help(application_parameters_t ap)
{
	static const __flash uint8_t list_header[]		= "> %S[%d]\n";
	static const __flash uint8_t detail_header[]	= "> %S[%d]: ";
	static const __flash uint8_t detail_footer[]	= "\n";
	static const __flash uint8_t detail_error[]		= "> no help for \"%s\"\n";

	const application_function_table_t __flash *tableptr;
	uint8_t		offset;

	if(ap.nargs > 1)
	{
		for(tableptr = application_function_table; tableptr->function; tableptr++)
			if(!strcmp_P((*ap.args)[1], tableptr->command))
				break;

		if(tableptr->function)
		{
			snprintf_P(ap.dst, ap.size, detail_header, tableptr->command, tableptr->required_args);
			strlcat_P(ap.dst, tableptr->description, ap.size);
			strlcat_P(ap.dst, detail_footer, ap.size);
		}
		else
			snprintf_P(ap.dst, ap.size, detail_error, (*ap.args)[1]);
	}
	else
	{
		for(tableptr = application_function_table; tableptr->function; tableptr++)
		{
			offset = snprintf_P(ap.dst, ap.size, list_header, tableptr->command, tableptr->required_args);
			ap.dst	+= offset;
			ap.size	-= offset;
		}
	}

	return(1);
}

static uint8_t application_function_reset(application_parameters_t ap)
{
	reset();

	return(0);
}

static uint8_t application_function_stats(application_parameters_t ap)
{
	stats_generate(ap.size, ap.dst);

	return(1);
}

static uint8_t application_function_stack(application_parameters_t ap)
{
	static const __flash uint8_t stackfree_fmt[] = "Stackmonitor: %d bytes free\n";

	snprintf_P(ap.dst, ap.size, stackfree_fmt, stackmonitor_free());

	return(1);
}
