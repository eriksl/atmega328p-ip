#include "application-timer.h"
#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	float		speed;
	uint16_t	min_value;
	uint16_t	max_value;
} pwm_t;

static pwm_t pwm[2];

ISR(TIMER1_OVF_vect)
{
	t1_interrupts++;
	t1_unhandled++;
}

static uint16_t getpwm(uint8_t entry)
{
	if(entry == 0)
		return(OCR1A);
	else
		return(OCR1B);
}

static void setpwm(uint8_t entry, uint16_t value)
{
	uint8_t	ocflags;
	uint8_t	output;

	if(entry == 0)
	{
		ocflags	= _BV(COM1A1);
		output	= _BV(1);
		OCR1A	= value;
	}
	else
	{
		ocflags	= _BV(COM1B1);
		output	= _BV(2);
		OCR1B	= value;
	}

	if(value > 0)
	{
		TCCR1A	|= ocflags;
	}
	else
	{
		TCCR1A	&= ~ocflags;
		PORTB	&= ~output;

	}
}

void application_init_timer(void)
{
	PRR &= ~_BV(PRTIM1);

	DDRB |= _BV(1) | _BV(2); // b1=oc1a, b2=oc1b

	TCCR1A	= _BV(WGM11);
	TCCR1B	= _BV(WGM13)  | _BV(WGM12); // fast pwm, top = ICR1
	TCCR1C	= 0x00;
	ICR1	= 0xffff;
	TIMSK1	= _BV(TOIE1);	// interrupt on overflow
	TIFR1	= _BV(TOV1); 	// clear event bits

	for(uint8_t ix = 0; ix < 2; ix++)
	{
		pwm[ix].speed		= 0;
		pwm[ix].min_value	= 0;
		pwm[ix].max_value	= 0;
	}

	setpwm(0, 0);
	setpwm(1, 0);

	TCCR1B |= _BV(CS10);	// start timer at prescaler 1, rate = 169 Hz
}

void application_periodic_timer(uint16_t missed_ticks)
{
	uint32_t min_value, max_value, old_value, value;

	for(uint8_t ix = 0; ix < 2; ix++)
	{
		if(pwm[ix].speed)
		{
			min_value	= pwm[ix].min_value;
			max_value	= pwm[ix].max_value;
			value		= (uint32_t)getpwm(ix);

			if(pwm[ix].speed > 0) // up
			{
				old_value	= value;
				value		= (uint32_t)((float)value * pwm[ix].speed);

				if(old_value == value)
					value++;

				if(value >= max_value)
				{
					value = max_value;
					pwm[ix].speed = 0 - pwm[ix].speed; // -> down
				}
			}
			else // down
			{
				old_value	= value;
				value		= (uint32_t)((float)value / (0 - pwm[ix].speed));

				if((old_value == value) && (value > 0))
					value--;

				if(value <= min_value)
				{
					value = min_value;
					pwm[ix].speed = 0 - pwm[ix].speed; // -> up
				}
			}

			setpwm(ix, value);
		}
	}
}

static const __flash char pwm_ok[] = "> pwm %u (min)value %u speed %f max %u\n";
static const __flash char pwm_error[] = "> invalid pwm %u\n";

uint8_t application_function_pwmw(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	uint8_t				entry;
	float				speed;
	uint16_t			minvalue, maxvalue;

	entry		= 0;
	minvalue	= 0;
	speed		= 0;
	maxvalue	= 0xffff;

	if(nargs > 1)
		entry = (uint8_t)atoi((const char *)args[1]);

	if(nargs > 2)
		minvalue = (uint16_t)atoi((const char *)args[2]);

	if(nargs > 3)
		speed = atof((const char *)args[3]);

	if(nargs > 4)
		maxvalue = (uint16_t)atoi((const char *)args[4]);

	if(entry > 1)
	{
		snprintf_P((char *)dst, size, pwm_error, entry);
		return(1);
	}

	setpwm(entry, minvalue);
	pwm[entry].speed		= speed;
	pwm[entry].min_value	= minvalue;
	pwm[entry].max_value	= maxvalue;
	minvalue				= getpwm(entry);

    snprintf_P((char *)dst, size, pwm_ok, entry, minvalue, speed, maxvalue);
	return(1);
}
