#include "application.h"
#include "application-sensor.h"
#include "application-twi.h"
#include "application-pwm.h"
#include "stats.h"
#include "util.h"
#include "stackmonitor.h"
#include "eeprom.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t (*application_function_t)(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);

typedef struct
{
	const char					command[7];
	uint8_t						required_args;
	application_function_t		function;
	const char					description[37];
} application_function_table_t;

static uint8_t cmd_led_timeout = 0;
static uint8_t heartbeat_led_timeout = 0;

static uint8_t application_function_edmp(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_help(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_quit(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_reset(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_sdmp(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_stack(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_stats(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst);

static const __flash application_function_table_t application_function_table[] =
{
	{
		"bgw",
		1,
		application_function_bg_write,
		"write bandgap caliburation (V)",
	},
	{
		"beep",
		0,
		application_function_beep,
		"beep duration period",
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
		"pwmw",
		1,
		application_function_pwmw,
		"pwmw index/value(/speed/min/max)",
	},
	{
		"q",
		0,
		application_function_quit,
		"quit",
	},
	{
		"quit",
		0,
		application_function_quit,
		"quit",
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
		(application_function_t)0,
		"",
	},
};

void application_init(void)
{
	application_init_sensor();
	application_init_pwm();
}

void application_periodic(void)
{
	uint16_t missed_ticks;

	if((missed_ticks = t1_unhandled) == 0)
		return;

	t1_unhandled = 0;

	if(missed_ticks > t1_unhandled_max)
		t1_unhandled_max = missed_ticks;

	if(heartbeat_led_timeout > missed_ticks)
		heartbeat_led_timeout -= missed_ticks;
	else
	{
		heartbeat_led_timeout = 122;
		PORTD ^= _BV(6);
	}

	if(cmd_led_timeout > missed_ticks)
		cmd_led_timeout -= missed_ticks;
	else
	{
		cmd_led_timeout = 0;
		PORTD &= ~_BV(7);
	}

	application_periodic_pwm(missed_ticks);
}

int16_t application_content(uint16_t src_length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static const __flash char error_fmt_unknown[] = "Command \"%s\" unknown\n";
	static const __flash char error_fmt_args[] = "Insufficient arguments: %d (%d required)\n";

	uint8_t args[application_num_args][application_length_args];
	uint8_t args_count, arg_current;
	uint8_t src_current = 0;
	uint8_t ws_skipped;
	const application_function_table_t __flash *tableptr;

	cmd_led_timeout = 10;
	PORTD |= _BV(6);

	if((src_length == 0) || (src[0] == 0xff)) // telnet options
		return(0);

	for(args_count = 0; (src_length > 0) && (args_count < application_num_args);)
	{
		ws_skipped = 0;

		for(arg_current = 0;
				(src_length > 0) && (arg_current < (application_length_args - 1));
				src_current++, src_length--)
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

		while((src_length > 0) && (src[src_current] > ' ') && (src[src_current] <= '~'))
		{
			src_length--;
			src_current++;
		}
	}

	*dst = '\0';

	cmd_led_timeout = 10;
	PORTD |= _BV(7);

	if(args_count == 0)
		return(0);

	for(tableptr = application_function_table; tableptr->function; tableptr++)
		if(!strcmp_P((const char *)args[0], tableptr->command))
			break;

	if(tableptr->function)
	{
		if(args_count < (tableptr->required_args + 1))
		{
			snprintf_P((char *)dst, size, error_fmt_args, args_count - 1, tableptr->required_args);
			return(strlen((char *)dst));
		}

		if(tableptr->function(args_count, args, size, dst))
			return(strlen((const char *)dst));
		else
			return(-1);
	}

	snprintf_P((char *)dst, size, error_fmt_unknown, args[0]);

	return(strlen((char *)dst));
}

static uint8_t application_function_edmp(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char format1[] = "> bandgap: %.5f\n";
	static const __flash char format2[] = "> sensor cal[%d]: *=%.5f / +=%.5f\n";

	uint8_t index, offset;

	float cfactor, coffset;

	offset	= snprintf_P((char *)dst, size, format1, eeprom_read_bandgap());
	dst		+= offset;
	size	-= offset;

	index = 0;

	while(eeprom_read_cal(index, &cfactor, &coffset))
	{
		offset	= snprintf_P((char *)dst, size, format2, index, cfactor, coffset);
		dst		+= offset;
		size	-= offset;
		index++;
	}

	return(1);
}

static uint8_t application_function_sdmp(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	uint8_t index, offset;

	index = 0;

	while(application_sensor_read(index, size, dst))
	{
		offset	= strlen((const char *)dst);
		dst		+= offset;
		size	-= offset;
		index++;
	}

	return(1);
}

static uint8_t application_function_help(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char list_header[]		= "> %S[%d]\n";
	static const __flash char detail_header[]	= "> %S[%d]: ";
	static const __flash char detail_footer[]	= "\n";
	static const __flash char detail_error[]	= "> no help for \"%s\"\n";

	const application_function_table_t __flash *tableptr;
	uint8_t		offset;

	if(nargs > 1)
	{
		for(tableptr = application_function_table; tableptr->function; tableptr++)
			if(!strcmp_P((const char *)args[1], tableptr->command))
				break;

		if(tableptr->function)
		{
			snprintf_P((char *)dst, size, detail_header, tableptr->command, tableptr->required_args);
			strlcat_P((char *)dst, tableptr->description, size);
			strlcat_P((char *)dst, detail_footer, size);
		}
		else
			snprintf_P((char *)dst, size, detail_error, (const char *)args[1]);
	}
	else
	{
		for(tableptr = application_function_table; tableptr->function; tableptr++)
		{
			offset = snprintf_P((char *)dst, size, list_header, tableptr->command, tableptr->required_args);
			dst		+= offset;
			size	-= offset;
		}
	}

	return(1);
	}

static uint8_t application_function_quit(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	return(0);
}

static uint8_t application_function_reset(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	reset();

	return(0);
}

static uint8_t application_function_stats(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	stats_generate(size, dst);

	return(1);
}

static uint8_t application_function_stack(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char stackfree_fmt[] = "Stackmonitor: %d bytes free\n";

	snprintf_P((char *)dst, (size_t)size, stackfree_fmt, stackmonitor_free());

	return(1);
}
