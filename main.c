#include <stdint.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/io.h>

#include "spi.h"
#include "twi_master.h"
#include "net.h"
#include "ethernet.h"
#include "arp.h"
#include "ipv4.h"
#include "bootp.h"
#include "watchdog.h"
#include "stats.h"
#include "eeprom.h"
#include "enc.h"
#include "application.h"
#include "util.h"

enum
{
	max_frame_size = 384,
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

	cli();
	watchdog_stop();

	PRR = 0xff;

#if 0
	b0		/cs				b0		O	square green				spi ss
	b1						oc1a	O	small red					light
	b2		(/ss)			oc1b	O	small green					ir
	b3		mosi					O	square green				spi mosi
	b4		miso					I	square red					spi miso
	b5		sck						O	square red					spi sck
	b6						b6		O	small orange				led spi
	b7						b7		I								button light down

	c0						adc0	I								tmp36
	c1						c1		O								*
	c2						c2		O								*
	c3						c3		O								*
	c4				sda				O								i2c sda
	c5				scl				O								i2c scl
	c6						RESET	O

	d0						d0		O	small transparent green		led heartbeat
	d1						d1		O	small transparent red		led command
	d2						int0	I								enc int
	d3						d3		O	rectangular green			DEBUG 1
	d4		 				d4		O	small red					led i2c
	d5						d5		O	large green					DEBUG 2
	d6						d6		O	large red					DEBUG 3
	d7						d7		I								button light up
#endif

	MCUCR	|= _BV(PUD);		//	disable pullups
	DDRB	= _BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(5) | _BV(6);
	DDRC	= _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6);
	DDRD	= _BV(0) | _BV(1) | _BV(3) | _BV(4) | _BV(5) | _BV(6);

	eeprom_read_mac_address(&my_mac_address);

	my_ipv4_address.byte[0] = 0;
	my_ipv4_address.byte[1] = 0;
	my_ipv4_address.byte[2] = 0;
	my_ipv4_address.byte[3] = 0;

	PORTD = 0;
	sleep(500);
	PORTD = _BV(0);
	sleep(500);
	PORTD = _BV(1);
	sleep(500);
	PORTD = 0;

	spi_init();
	twi_master_init();
	enc_init(max_frame_size, &my_mac_address);
	enc_set_led(PHLCON_LED_RCV, PHLCON_LED_XMIT);
	watchdog_start(application_init());

	sei();

	for(;;)
	{
		watchdog_rearm();

		application_idle();

		if(ipv4_address_match(&my_ipv4_address, &ipv4_addr_zero))
		{
			tx_frame_length = bootp_create_request(tx_frame, &my_mac_address);
			enc_send_frame(tx_frame, tx_frame_length);
		}

		if((rx_frame_length = enc_receive_frame(rx_frame, sizeof(rx_frame))) == 0)
			continue;

		if((tx_frame_length = ethernet_process_frame(rx_frame, rx_frame_length, tx_frame, sizeof(tx_frame), &my_mac_address, &my_ipv4_address)) == 0)
			continue;

		enc_send_frame(tx_frame, tx_frame_length);
	}
}
