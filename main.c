#include "twi_master.h"
#include "stats.h"
#include "eeprom.h"
#include "application.h"
#include "util.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

ISR(WDT_vect)
{
	wd_interrupts++;
}

int main(void)
{
	cli();
    wdt_reset();
    MCUSR = 0;
    wdt_disable();

	PRR = 0xff;

	MCUCR	|= _BV(PUD);		//	disable pullups
	DDRB	= 0;
	DDRC	= _BV(0);
	DDRD	= _BV(2) | _BV(3) | _BV(4);

	PORTD = _BV(3) | _BV(4);
	msleep(200);
	PORTD = _BV(3);
	msleep(200);
	PORTD = _BV(4);
	msleep(200);
	PORTD = _BV(2);

	wdt_enable(WDTO_2S);
	twi_master_init();
	application_init();

	sei();

	for(;;)
	{
		pause_idle();			// gets woken by the 122 Hz timer1 interrupt or packet arrival or watchdog interrupt
		WDTCSR |= _BV(WDIE);	// enable wdt interrupt, reset
		application_periodic();	// run periodic tasks
	}
}
