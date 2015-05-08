#include "twi_master.h"
#include "stats.h"
#include "application.h"
#include "util.h"
#include "uart-telnet.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include <stdint.h>

ISR(WDT_vect)
{
	wd_interrupts++;
}

int main(void)
{
	uint8_t receive_buffer[128];
	uint8_t send_buffer[640];
	uint8_t ix;

	cli();
    wdt_reset();
    MCUSR = 0;
    wdt_disable();

	PRR = 0xff;

	MCUCR	|= _BV(PUD);		//	disable pullups

	DDRB	= 0;
	DDRD	= _BV(2) | _BV(3) | _BV(4);

	PORTB	= 0x00;
	PORTC	= 0x00;
	PORTD = _BV(2);				// release esp8266 from reset

	for(ix = 8; ix > 0; ix--)
	{
		PORTD |= _BV(3);

		msleep(50);

		PORTD &= ~_BV(4);

		msleep(50);

		PORTD |= _BV(4);

		msleep(50);

		PORTD &= ~_BV(3);

		msleep(50);
	}

	wdt_enable(WDTO_4S);
	twi_master_init();
	application_init();
	uart_init();

	sei();

	for(;;)
	{
		pause_idle();			// gets woken by the ~150 Hz pwm timer1 interrupt or byte arrival or watchdog interrupt
		WDTCSR |= _BV(WDIE);	// enable wdt interrupt, reset

		if(uart_transmit_ready() && uart_receive_ready())
		{
			uart_receive(sizeof(receive_buffer), receive_buffer);
			application_content(receive_buffer, sizeof(send_buffer), send_buffer);
			uart_transmit(send_buffer);
		}

		application_periodic();	// run background tasks
	}
}
