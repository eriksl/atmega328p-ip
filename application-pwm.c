#include "application-pwm.h"
#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	volatile uint16_t	*ocr;
	float				speed;
	uint16_t			min_value;
	uint16_t			max_value;
} pwm_t;

static uint8_t beep_length		= 0;
static uint8_t beep_period		= 0;

static pwm_t pwm[2];

ISR(TIMER1_OVF_vect)
{
	t1_interrupts++;
	t1_unhandled++;
}

void application_init_pwm(void)
{
	PRR &= ~_BV(PRTIM1);

	DDRB |= _BV(1) | _BV(2); // b1=oc1a, b2=oc1b

	TCCR1A	= _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
	TCCR1B	= _BV(WGM13)  | _BV(WGM12); // fast pwm, top = ICR1
	TCCR1C	= 0x00;
	ICR1	= 0xffff;
	OCR1A	= 0x0000;
	OCR1B	= 0x0000;
	TIMSK1	= _BV(TOIE1);	// interrupt on overflow
	TIFR1	= _BV(TOV1); 	// clear event bits
	TCCR1B	|= _BV(CS10);	// start timer at prescaler 1, rate = 122 Hz

	for(uint8_t ix = 0; ix < 2; ix++)
	{
		pwm[ix].ocr		= ix ? &OCR1B : &OCR1A;
		pwm[ix].speed	= 0;
	}
}

void application_periodic_pwm(uint16_t missed_ticks)
{
	static uint8_t called		= 0;
	static uint8_t count_up		= 0;
	static uint8_t count_down	= 0;
	static uint8_t state_up		= 0;
	static uint8_t state_down	= 0;

	if(!(PINB & _BV(7)))
		count_down++;

	if(!(PIND & _BV(7)))
		count_up++;

	if(++called > 5)
	{
		if(count_down > 3)
		{
			if(!state_down)
				OCR1A >>= 1;

			state_down = 1;
			pwm[0].speed = 0;
		}
		else
			state_down = 0;

		if(count_up > 3)
		{
			if(!state_up)
			{
				if(!(OCR1A & (1 << 15)))
				{
					if(OCR1A)
						OCR1A <<= 1;
					else
						OCR1A = 1;
				}

				state_up = 1;
				pwm[0].speed = 0;
			}
		}
		else
			state_up = 0;

		called		= 0;
		count_up	= 0;
		count_down	= 0;
	}

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
			value		= (uint32_t)*pwm[ix].ocr;

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

			*pwm[ix].ocr = (uint16_t)value;
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

	*pwm[entry].ocr			= minvalue;
	pwm[entry].speed		= speed;
	pwm[entry].min_value	= minvalue;
	pwm[entry].max_value	= maxvalue;
	minvalue				= *pwm[entry].ocr;

    snprintf_P((char *)dst, size, pwm_ok, entry, minvalue, speed, maxvalue);
	return(1);
}
