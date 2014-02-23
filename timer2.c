#include <avr/io.h>

#include "timer2.h"

static uint8_t cs[3];

void timer2_init(uint8_t prescaler)
{
	PRR &= ~_BV(PRTIM2);

	DDRD |= _BV(3); // d3=oc2b

	if(prescaler > timer2_1024)
		return;

	cs[0] = (prescaler & (1 << 0)) >> 0;
	cs[1] = (prescaler & (1 << 1)) >> 1;
	cs[2] = (prescaler & (1 << 2)) >> 2;

	timer2_stop();

	TCCR2A	= _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); // fast pwm, top = 0xff
	TCCR2B	= 0x00;	// stop clock
	OCR2A	= 0x00;
	OCR2B	= 0x00;
	TIMSK2	= 0x00;	// no interrupts
	TIFR2	= 0xff;	// clear event bits
}

void timer2_stop(void)
{
	TCCR2B &= ~(_BV(CS22) | _BV(CS21) | _BV(CS20));
}

void timer2_start(void)
{
	timer2_stop();
	TCCR2B |= (cs[2] << CS22) | (cs[1] << CS21) | (cs[0] << CS20);
}

void timer2_set_oc2a(uint16_t value)
{
	OCR2A = value;
}

uint16_t timer2_get_oc2a(void)
{
	return(OCR2A);
}

void timer2_set_oc2b(uint16_t pwm_value)
{
	if((pwm_value == 0) || (pwm_value >= 0xff))
	{
		OCR2B = 0;

		if(pwm_value == 0)
			PORTD &= ~_BV(3); // oc2b=d3
		else
			PORTD |= _BV(3);
	}
	else
		OCR2B = pwm_value;
}

uint16_t timer2_get_oc2b(void)
{
	uint16_t rv;

	rv = OCR2B;

	if((rv == 0) && (PIND & _BV(3))) // oc2b=d3
		rv = 0xff;

	return(rv);
}
