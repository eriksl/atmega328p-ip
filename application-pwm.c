#include "application-pwm.h"
#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

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
}

void application_periodic_pwm(uint16_t missed_ticks)
{
	static uint16_t oc1a = 0;
	static uint16_t oc1b = 0;

	oc1a += missed_ticks;
	oc1b -= missed_ticks;

	OCR1A = oc1a;
	OCR1B = oc1b;
}
