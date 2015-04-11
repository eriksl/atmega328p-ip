#include "twi_master.h"
#include "esp.h"
#include "stats.h"
#include "eeprom.h"
#include "application.h"
#include "util.h"
#include "display.h"

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

#if 0
	b0		/cs				b0		O	square green				spi ss
	b1						oc1a	O	small red					light
	b2		(/ss)			oc1b	O	small green					ir
	b3		mosi					O	square green				spi mosi
	b4		miso					I	square red					spi miso
	b5		sck						O	square red					spi sck
	b6								*								*
	b7						b7		I								button light down

	c0						adc0	I								tmp36
	c1						c1		*								*
	c2						c2		*								*
	c3						c3		*								*
	c4				sda				O								i2c sda
	c5				scl				O								i2c scl
	c6						RESET	O

	d0						d0		O	small transparent green		led heartbeat
	d1						d1		O	small transparent red		led command
	d2						int0	I								enc int
	d3						d3		O	buzzer
	d4		 						*								*
	d5						d5		*								*
	d6						d6		*								*
	d7						d7		I								button light up
#endif

	MCUCR	|= _BV(PUD);		//	disable pullups
	DDRB	= 0;
	DDRC	= 0;
	DDRD	= _BV(3) | _BV(6) | _BV(7);

	PORTD = _BV(6) | _BV(3);
	msleep(200);
	PORTD = _BV(7);
	msleep(200);
	PORTD = _BV(6) | _BV(7) | _BV(3);
	msleep(200);
	PORTD = 0;

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
