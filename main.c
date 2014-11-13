#include "spi.h"
#include "twi_master.h"
#include "net.h"
#include "ethernet.h"
#include "arp.h"
#include "ipv4.h"
#include "bootp.h"
#include "stats.h"
#include "eeprom.h"
#include "enc.h"
#include "application.h"
#include "util.h"
#include "display.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

enum
{
	max_frame_size = 512,
};

static ipv4_addr_t	my_ipv4_address;
static mac_addr_t	my_mac_address;

ISR(WDT_vect)
{
	wd_interrupts++;
}

int main(void)
{
	uint8_t 		rx_frame[max_frame_size];
	uint8_t			tx_frame[max_frame_size];

	uint16_t		rx_frame_length;
	uint16_t		tx_frame_length;

	uint8_t			bootp_timer;

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

	eeprom_read_mac_address(&my_mac_address);

	my_ipv4_address.byte[0] = 0;
	my_ipv4_address.byte[1] = 0;
	my_ipv4_address.byte[2] = 0;
	my_ipv4_address.byte[3] = 0;

	PORTD = _BV(6) | _BV(3);
	msleep(200);
	PORTD = _BV(7);
	msleep(200);
	PORTD = _BV(6) | _BV(7) | _BV(3);
	msleep(200);
	PORTD = 0;

	wdt_enable(WDTO_2S);
	spi_init();
	twi_master_init();
	enc_init(max_frame_size, &my_mac_address);
	enc_set_led(PHLCON_LED_RCV, PHLCON_LED_XMIT);
	application_init();

	sei();

	strncpy((char *)tx_frame, "boot", sizeof(tx_frame));
	tx_frame[3] |= 0x80; // add dot to last digit
	display_show(tx_frame);

	bootp_timer = 1;

	for(;;)
	{
		pause_idle();			// gets woken by the 122 Hz timer1 interrupt or packet arrival or watchdog interrupt
		WDTCSR |= _BV(WDIE);	// enable wdt interrupt, reset
		application_periodic();	// run periodic tasks

		if(bootp_timer)
		{
			if(bootp_timer > 250)
			{
				if(ipv4_address_match(&my_ipv4_address, &ipv4_addr_zero))
				{
					tx_frame_length = bootp_create_request(tx_frame, &my_mac_address);
					enc_send_frame(tx_frame, tx_frame_length);
					bootp_timer = 1;
				}
				else
				{
					snprintf((char *)tx_frame, sizeof(tx_frame), "%4u", my_ipv4_address.byte[3]);
					display_show(tx_frame);
					bootp_timer = 0;
				}

			}
			else
				bootp_timer++;
		}

		if((rx_frame_length = enc_receive_frame(rx_frame, sizeof(rx_frame))) == 0)
			continue;

		if((tx_frame_length = ethernet_process_frame(rx_frame, rx_frame_length, tx_frame, sizeof(tx_frame), &my_mac_address, &my_ipv4_address)) == 0)
			continue;

		enc_send_frame(tx_frame, tx_frame_length);
	}
}
