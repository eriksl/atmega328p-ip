#include "application-timer.h"
#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define JIFFIES_PER_SECOND	((float)F_CPU / 65536)
#define JIFFIES_PER_DAY		((uint32_t)(JIFFIES_PER_SECOND * 60 * 60 * 24))

typedef struct
{
	float		speed;
	uint16_t	min_value;
	uint16_t	max_value;
} pwm_t;

static uint8_t beep_length = 0;
static uint8_t beep_period = 0;

static pwm_t pwm[2];

ISR(TIMER1_OVF_vect)
{
	t1_interrupts++;
	t1_unhandled++;

	if(t1_jiffies < JIFFIES_PER_DAY)
		t1_jiffies++;
	else
		t1_jiffies = 0;
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
	if(beep_length > 0)
	{

		if((beep_length % beep_period) == 0)
			PORTD ^= _BV(3);

		beep_length--;
	}
	else
		PORTD &= ~_BV(3);

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

uint8_t application_function_beep(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> beep %u %u\n";

	beep_length = 60;
	beep_period = 0;

	if(nargs > 1)
		beep_length = (uint8_t)atoi((const char *)args[1]);

	if(nargs > 2)
		beep_period = (uint8_t)atoi((const char *)args[2]);

	if((beep_length > 0) && (beep_period == 0))
		PORTD |= _BV(3);

    snprintf_P((char *)dst, size, ok, beep_length, beep_period);

	return(1);
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

uint8_t application_function_clockr(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> clock %02u:%02u.%02u\n";

	uint32_t	seconds;
	uint8_t		minutes;
	uint8_t		hours;

	cli();
	seconds	= t1_jiffies;
	sei();

	seconds	= (uint32_t)((float)seconds / (float)JIFFIES_PER_SECOND);
	hours	= seconds / (60UL * 60UL);
	seconds	= seconds - (hours * 60UL * 60UL);
	minutes	= seconds / 60UL;
	seconds	= seconds - (minutes * 60UL);

    snprintf_P((char *)dst, size, ok, (int)hours, (int)minutes, (int)seconds);
	return(1);
}

uint8_t application_function_clockw(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	uint32_t	seconds;
	uint8_t		minutes;
	uint8_t		hours;

	hours	= (uint32_t)atoi((const char *)args[1]);
	minutes	= atoi((const char *)args[2]);
	seconds	= atoi((const char *)args[3]);

	seconds += minutes * 60UL;
	seconds += hours * 60UL * 60UL;
	seconds = (uint32_t)((float)seconds * (float)JIFFIES_PER_SECOND);

	cli();
	t1_jiffies = seconds;
	sei();

	return(application_function_clockr(nargs, args, size, dst));
}
