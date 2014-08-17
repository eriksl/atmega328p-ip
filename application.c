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
#include <stdlib.h>

enum
{
	num_args = 8,
	length_args = 8,
};

typedef uint8_t (*application_function_t)(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);

typedef struct
{
	const char					*id;
	uint8_t						required_args;
	application_function_t		function;
	const __flash char			*description;
} application_function_table_t;

static uint8_t twi_address = 0;

static uint8_t application_function_dump(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_help(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_quit(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_reset(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_stack(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_stats(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_twiaddress(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_twiread(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_twireset(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);
static uint8_t application_function_twiwrite(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst);

static const __flash char description_dump[] = "";
static const __flash char description_help[] = "help";
static const __flash char description_quit[] = "quit";
static const __flash char description_reset[] = "reset system";
static const __flash char description_stack[] = "free memory";
static const __flash char description_stats[] = "statistics";
static const __flash char description_twiaddress[] = "set TWI slave";
static const __flash char description_twiread[] = "read n bytes from TWI";
static const __flash char description_twireset[] = "reset TWI";
static const __flash char description_twiwrite[] = "write bytes to TWI";

static const __flash application_function_table_t application_function_table[] =
{
	{
		"dump",
		0,
		application_function_dump,
		description_dump,
	},
	{
		"help",
		0,
		application_function_help,
		description_help,
	},
	{
		"?",
		0,
		application_function_help,
		0,
	},
	{
		"q",
		0,
		application_function_quit,
		description_quit,
	},
	{
		"quit",
		0,
		application_function_quit,
		0,
	},
	{
		"reset",
		0,
		application_function_reset,
		description_reset,
	},
	{
		"S",
		0,
		application_function_stack,
		description_stack,
	},
	{
		"stack",
		0,
		application_function_stack,
		0,
	},
	{
		"s",
		0,
		application_function_stats,
		description_stats,
	},
	{
		"stats",
		0,
		application_function_stats,
		0,
	},
	{
		"twia",
		1,
		application_function_twiaddress,
		description_twiaddress,
	},
	{
		"twir",
		1,
		application_function_twiread,
		description_twiread,
	},
	{
		"twirst",
		0,
		application_function_twireset,
		description_twireset,
	},
	{
		"twiw",
		1,
		application_function_twiwrite,
		description_twiwrite,
	},
	{
		"",
		0,
		(application_function_t)0,
		0,
	},
};

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

int16_t application_content(uint16_t src_length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static const __flash char error_fmt_unknown[] = "Command \"%s\" unknown\n";
	static const __flash char error_fmt_args[] = "Insufficient arguments: %d (%d required)\n";

	uint8_t args[num_args][length_args];
	uint8_t args_count, arg_current;
	uint8_t src_current = 0;
	uint8_t ws_skipped;
	uint8_t tmp[length_args + 10];
	const application_function_table_t __flash *tableptr;

	if((src_length == 0) || (src[0] == 0x0ff)) // telnet options
		return(0);

	for(args_count = 0; (src_length > 0) && (args_count < num_args);)
	{
		ws_skipped = 0;

		for(arg_current = 0;
				(src_length > 0) && (arg_current < (length_args - 1));
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

	if(args_count == 0)
		return(0);

	for(tableptr = application_function_table; tableptr->function; tableptr++)
		if(!strcmp((const char *)args[0], tableptr->id))
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

static uint8_t application_function_dump(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char format[] = "> arg %d: \"%s\"\n";

	uint8_t narg, offset;

	for(narg = 0; narg < nargs; narg++)
	{
		offset = snprintf_P((char *)dst, size, format, narg, args[narg]);
		dst += offset;
		size -= offset;
	}

	return(1);
}

static uint8_t application_function_help(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char header[] = "> ";
	static const __flash char footer[] = "\n";
	const application_function_table_t __flash *tableptr;
	uint8_t offset;

	for(tableptr = application_function_table; tableptr->function; tableptr++)
	{
		offset = strlen_P(header);
		strlcpy_P((char *)dst, header, size);
		dst += offset;
		size -= offset;

		offset = strlen(tableptr->id);
		strlcpy((char *)dst, (const char *)tableptr->id, size);
		dst += offset;
		size -= offset;

		offset = snprintf((char *)dst, size, "(%d): ", tableptr->required_args);
		dst += offset;
		size -= offset;

		if(tableptr->description)
		{
			offset = strlen_P(tableptr->description);
			strlcpy_P((char *)dst, tableptr->description, size);
			dst += offset;
			size -= offset;
		}

		offset = strlen_P(footer);
		strlcat_P((char *)dst, footer, size);
		dst += offset;
		size -= offset;
	}

	return(1);
}

static uint8_t application_function_quit(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	return(0);
}

static uint8_t application_function_reset(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	reset();

	return(0);
}

static uint8_t application_function_stats(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	stats_generate(size, dst);

	return(1);
}

static uint8_t application_function_stack(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char stackfree_fmt[] = "Stackmonitor: %d bytes free\n";

	snprintf_P((char *)dst, (size_t)size, stackfree_fmt, stackmonitor_free());

	return(1);
}

static void twi_error(uint8_t *dst, uint16_t size, uint8_t error)
{
	static const __flash char format_string[] = "TWI error: %x, state %x\n";

	snprintf_P((char *)dst, (size_t)size, format_string,
			error & 0x0f, (error & 0xf0) >> 4);
}

static uint8_t application_function_twiaddress(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char fmt[] = "> TWI slave address set to 0x%02x\n";

	twi_address = (uint8_t)strtoul((const char *)args[1], 0, 16);

	snprintf_P((char *)dst, (size_t)size, fmt, twi_address);

	return(1);
}

static uint8_t application_function_twiread(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
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

static uint8_t application_function_twireset(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> TWI reset ok\n";

	twi_master_recover();

	strlcpy_P((char *)dst, ok, size);

	return(1);
}

static uint8_t application_function_twiwrite(uint8_t nargs, uint8_t args[num_args][length_args], uint16_t size, uint8_t *dst)
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
