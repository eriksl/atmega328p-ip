#include <avr/io.h>

#include "timer0.h"

static uint8_t cs[3];

void timer0_init(uint8_t prescaler)
{
	static	uint8_t temp, mask;

	PRR &= ~_BV(PRTIM0);

	DDRD |= _BV(5) | _BV(6); // d5=oc0b, d6=oc0a

	if(prescaler > timer0_ext_rising)
		return;

	cs[0] = (prescaler & (1 << 0)) >> 0;
	cs[1] = (prescaler & (1 << 1)) >> 1;
	cs[2] = (prescaler & (1 << 2)) >> 2;

	timer0_stop();

	TCCR0A	= _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); // fast pwm, top = 0xff
	TCCR0B	= 0x00;	// stop clock
	OCR0A	= 0x00;
	OCR0B	= 0x00;
	TIMSK0	= 0x00;	// no interrupts
	TIFR0	= 0xff;	// clear event bits
}

void timer0_stop(void)
{
	TCCR0B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
}

void timer0_start(void)
{
	timer0_stop();
	TCCR0B |= (cs[2] << CS02) | (cs[1] << CS01) | (cs[0] << CS00);
}

void timer0_set_oc0a(uint16_t pwm_value)
{
	if((pwm_value == 0) || (pwm_value >= 0xff))
	{
		OCR0A = 0;

		if(pwm_value == 0)
			PORTD &= ~_BV(6);	// oc0a=d6
		else
			PORTD |= _BV(6);
	}
	else
		OCR0A = pwm_value;
}

uint16_t timer0_get_oc0a(void)
{
	uint16_t rv;

	rv = OCR0A;

	if((rv == 0) && (PIND & _BV(6)))	// oc0a=d6
		rv = 0xff;

	return(rv);
}

void timer0_set_oc0b(uint16_t pwm_value)
{
	if((pwm_value == 0) || (pwm_value >= 0xff))
	{
		OCR0B = 0;

		if(pwm_value == 0)
			PORTD &= ~_BV(5);	// oc0a=d5
		else
			PORTD |= _BV(5);
	}
	else
		OCR0B = pwm_value;
}

uint16_t timer0_get_oc0b(void)
{
	uint16_t rv;

	rv = OCR0B;

	if((rv == 0) && (PIND & _BV(5)))	// oc0a=d5
		rv = 0xff;

	return(rv);
}
