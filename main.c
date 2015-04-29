#include "twi_master.h"
#include "esp.h"
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

	if(esp_wd_timeout > 0)
		esp_wd_timeout--;
	else
		reset();
}

int main(void)
{
	static	uint8_t		receive_buffer[512];
	static	uint8_t		send_buffer[512];
			uint16_t	length;
			uint8_t		connection;

	cli();
    wdt_reset();
    MCUSR = 0;
    wdt_disable();

	PRR = 0xff;

	MCUCR	|= _BV(PUD);		//	disable pullups
	DDRB	= 0;
	DDRD	= _BV(2) | _BV(3) | _BV(4);

	for(length = 8; length > 0; length--)
	{
		PORTD |= _BV(3);
		PORTD &= ~_BV(4);

		msleep(50);

		PORTD |= _BV(4);
		PORTD &= ~_BV(3);

		msleep(50);
	}

	PORTD &= ~(_BV(3) | _BV(4));

	esp_wd_timeout = 0xffff;
	wdt_enable(WDTO_4S);
	twi_master_init();
	application_init();
	esp_init(sizeof(receive_buffer), receive_buffer, sizeof(send_buffer), send_buffer);

	sei();

	for(;;)
	{
		pause_idle();			// gets woken by the ~150 Hz pwm timer1 interrupt or packet arrival or watchdog interrupt
		WDTCSR |= _BV(WDIE);	// enable wdt interrupt, reset

		if(esp_send_finished() && esp_receive_finished())
		{
			application_content(receive_buffer, sizeof(send_buffer) - 1, send_buffer);
			esp_send_start(strlen(send_buffer), &connection);
		}

		application_periodic();	// run background tasks
	}
}
