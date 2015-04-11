#include "twi_master.h"
#include "esp.h"
#include "stats.h"
#include "eeprom.h"
#include "application.h"
#include "util.h"
#include "led-display.h"

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
	DDRC	= _BV(3);
	DDRD	= _BV(3) | _BV(6) | _BV(7);

	for(length = 0; length < 8; length++)
	{
		PORTD |= _BV(6);
		msleep(50);
		PORTD &= ~_BV(6);
		PORTD |= _BV(7);
		msleep(50);
		PORTD &= ~_BV(7);
	}

	esp_wd_timeout = 0xffff;
	wdt_enable(WDTO_1S);
	twi_master_init();
	application_init();
	esp_init(sizeof(receive_buffer), receive_buffer, sizeof(send_buffer), send_buffer);

	sei();

	{
		uint8_t boot[16];
		strncpy((char *)boot, "boot", sizeof(boot));
		display_show(boot);
	}

	for(;;)
	{
		pause_idle();			// gets woken by the ~150 Hz pwm timer1 interrupt or packet arrival or watchdog interrupt
		WDTCSR |= _BV(WDIE);	// enable wdt interrupt, reset

		if(esp_send_finished() && esp_receive_finished())
		{
			length = esp_receive_length(&connection);

			if(length > 0)
			{
				length = application_content(length, receive_buffer, sizeof(send_buffer), send_buffer);

				if(length > 0)
					esp_send_start(length, &connection);
			}
		}

		application_periodic();	// run background tasks
	}
}
