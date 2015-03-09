#include "application-timer.h"
#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	uint16_t	min_value;
	uint16_t	max_value;
	float		speed;
} output_t;

static output_t output[3] =
{
	{ 0, 0, 0 },	// PWM OCR1A
	{ 0, 0, 0 },	// PWM OCR1B
	{ 0, 0, 0 },	// OUTPUT C0
};

#define sizeof_output (sizeof(output) / sizeof(output[0]))

ISR(TIMER1_OVF_vect)
{
	t1_interrupts++;
	t1_unhandled++;
}

static uint16_t getoutput(uint8_t entry)
{
	switch(entry)
	{
		case(0): return(OCR1A);
		case(1): return(OCR1B);
		case(2): return(!!(PORTC & _BV(0)));
	}

	return(0);
}

static void setoutput(uint8_t entry, uint16_t value)
{
	switch(entry)
	{
		case(0):
		{
			if(value > 0)
			{
				OCR1A	= value;
				TCCR1A	|= _BV(COM1A1);
			}
			else
			{
				OCR1A	= 0;
				TCCR1A	&= ~_BV(COM1A1);
				PORTB	&= ~_BV(1);
			}

			break;
		}
		case(1):
		{
			if(value > 0)
			{
				OCR1B	= value;
				TCCR1A |= _BV(COM1B1);
			}
			else
			{
				OCR1B	= 0;
				TCCR1A	&= ~_BV(COM1B1);
				PORTB	&= ~_BV(2);
			}

			break;
		}

		case(2):
		{
			if(value)
				PORTC |= _BV(0);
			else
				PORTC &= ~_BV(0);

			break;
		}
	}
}

void application_init_timer(void)
{
	PRR &= ~_BV(PRTIM1);

	DDRB |= _BV(1) | _BV(2);	// b1=oc1a, b2=oc1b
	DDRC |= _BV(0);				// c0=extra output

	TCCR1A	= _BV(WGM11);
	TCCR1B	= _BV(WGM13)  | _BV(WGM12); // fast pwm, top = ICR1
	TCCR1C	= 0x00;
	ICR1	= 0xffff;
	TIMSK1	= _BV(TOIE1);	// interrupt on overflow
	TIFR1	= _BV(TOV1); 	// clear event bits

	setoutput(0, 0);
	setoutput(1, 0);
	setoutput(2, 0);

	TCCR1B |= _BV(CS10);	// start timer at prescaler 1, rate = 169 Hz
}

void application_periodic_timer(uint16_t missed_ticks)
{
	uint32_t min_value, max_value, old_value, value;

	for(uint8_t ix = 0; ix < 2; ix++)
	{
		if(output[ix].speed)
		{
			min_value	= output[ix].min_value;
			max_value	= output[ix].max_value;
			value		= (uint32_t)getoutput(ix);

			if(output[ix].speed > 0) // up
			{
				old_value	= value;
				value		= (uint32_t)((float)value * output[ix].speed);

				if(old_value == value)
					value++;

				if(value >= max_value)
				{
					value = max_value;
					output[ix].speed = 0 - output[ix].speed; // -> down
				}
			}
			else // down
			{
				old_value	= value;
				value		= (uint32_t)((float)value / (0 - output[ix].speed));

				if((old_value == value) && (value > 0))
					value--;

				if(value <= min_value)
				{
					value = min_value;
					output[ix].speed = 0 - output[ix].speed; // -> up
				}
			}

			setoutput(ix, value);
		}
	}
}

static const __flash char output_error[] = "> invalid output %u\n";

static uint16_t output_read(uint8_t entry, uint16_t size, uint8_t *dst)
{
	static const __flash char output_pwm_dynamic[]		= "> output %u pwm/dynamic %u %u-%u * %f\n";
	static const __flash char output_pwm_static[]		= "> output %u pwm/static  %u\n";
	static const __flash char output_normal[]			= "> output %u digital     %u\n";

	uint16_t length;

	if(entry < 2)
	{
		if(output[entry].speed != 0)
			length = snprintf_P((char *)dst, size, output_pwm_dynamic, entry, getoutput(entry), output[entry].min_value, output[entry].max_value, output[entry].speed);
		else
			length = snprintf_P((char *)dst, size, output_pwm_static, entry, getoutput(entry));
	}
	else
	{
		if(entry < sizeof_output)
			length = snprintf_P((char *)dst, size, output_normal, entry, getoutput(entry));
		else
			length = snprintf_P((char *)dst, size, output_error, entry);
	}

	return(length);
}

uint8_t application_function_output_read(application_parameters_t ap)
{
	uint8_t entry;

	entry = (uint8_t)atoi((const char *)(*ap.args)[1]);

	output_read(entry, ap.size, ap.dst);

	return(1);
}

uint8_t application_function_output_set(application_parameters_t ap)
{
	uint8_t		entry;
	float		speed;
	uint16_t	minvalue, maxvalue;

	entry		= 0;
	minvalue	= 0;
	speed		= 0;
	maxvalue	= 0xffff;

	entry = (uint8_t)atoi((const char *)(*ap.args)[1]);

	if(entry > 2)
	{
		snprintf_P((char *)ap.dst, ap.size, output_error, entry);
		return(1);
	}

	if(ap.nargs > 2)
		minvalue = (uint16_t)atoi((const char *)(*ap.args)[2]);

	if(ap.nargs > 3)
		speed = atof((const char *)(*ap.args)[3]);

	if(ap.nargs > 4)
		maxvalue = (uint16_t)atoi((const char *)(*ap.args)[4]);

	setoutput(entry, minvalue);
	output[entry].speed		= speed;
	output[entry].min_value	= minvalue;
	output[entry].max_value	= maxvalue;

	output_read(entry, ap.size, ap.dst);

	return(1);
}

uint8_t application_function_output_dump(application_parameters_t ap)
{
	uint8_t entry;
	uint16_t length;

	for(entry = 0; entry < sizeof_output; entry++)
	{
		length = output_read(entry, ap.size, ap.dst);
		ap.dst	+= length;
		ap.size	-= length;
	}

	return(1);
}
