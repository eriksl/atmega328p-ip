#include <avr/io.h>

#include "timer1.h"

static uint8_t cs[3];

void timer1_init_pwm1a1b(uint8_t prescaler)
{
	PRR &= ~_BV(PRTIM1);

	DDRB |= _BV(1) | _BV(2); // b1=oc1a, b2=oc1b

	if(prescaler > timer1_ext_rising)
		return;

	cs[0] = (prescaler & (1 << 0)) >> 0;
	cs[1] = (prescaler & (1 << 1)) >> 1;
	cs[2] = (prescaler & (1 << 2)) >> 2;

	timer1_stop();

	TCCR1A	= _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11); // fast pwm, top = icr1
	TCCR1B	= _BV(WGM13) | _BV(WGM12); // stop clock, fast pwm, top = icr1
	TCCR1C	= 0x00;
	ICR1	= 0xffff;
	OCR1A	= 0x0000;
	OCR1B	= 0x0000;
	TIMSK1	= 0x00;	// no interrupts
	TIFR1	= 0xff;	// clear event bits
}

void timer1_stop(void)
{
	TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
}

void timer1_start(void)
{
	timer1_stop();
	TCCR1B |= (cs[2] << CS12) | (cs[1] << CS11) | (cs[0] << CS10);
}

void timer1_set_oc1a(uint16_t pwm_value)
{
	if((pwm_value == 0) || (pwm_value == 0xffff))
	{
		OCR1A = 0;

		if(pwm_value == 0)
			PORTB &= ~_BV(1); // oc1a=b1
		else
			PORTB |= _BV(1);
	}
	else
		OCR1A = pwm_value;
}

uint16_t timer1_get_oc1a(void)
{
	uint16_t rv;

	rv = OCR1A;

	if((rv == 0) && (PINB & _BV(1))) // oc1a=b1
		rv = 0xffff;

	return(rv);
}

void timer1_set_oc1b(uint16_t pwm_value)
{
	if((pwm_value == 0) || (pwm_value == 0xffff))
	{
		OCR1B = 0;

		if(pwm_value == 0)
			PORTB &= ~_BV(2); // oc1b=b2
		else
			PORTB |= _BV(2);
	}
	else
		OCR1B = pwm_value;
}

uint16_t timer1_get_oc1b(void)
{
	uint16_t rv;

	rv = OCR1B;

	if((rv == 0) && (PINB & _BV(2))) // oc1b=b2
		rv = 0x00ff;

	return(rv);
}
