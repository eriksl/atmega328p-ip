#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "spi.h"
#include "enc.h"
#include "net.h"
#include "ethernet.h"
#include "arp.h"
#include "ipv4.h"
#include "watchdog.h"
#include "stats.h"

#define MAX_FRAME_SIZE 256
#define WATCHDOG_PRESCALER WATCHDOG_PRESCALER_512

static const ipv4_addr_t ipv4	= {{ 10, 1, 10, 126 }};
static const mac_addr_t mac 	= {{ 0x00, 0x1e, 0xc0, 0x01, 0x01, 0x01 }};

static void sleep(uint16_t tm)
{
	while(tm-- > 0)
	{
		_delay_ms(1);
		watchdog_reset();
	}
}

ISR(WDT_vect)
{
	wd_interrupts++;
}

ISR (INT0_vect)
{
	eth_interrupts++;
}

static void init_enc(void)
{
	enc_init(MAX_FRAME_SIZE, &mac);
	enc_set_led(PHLCON_LED_RCV, PHLCON_LED_XMIT);

	PORTD = 0;
	sleep(100);
	PORTD = _BV(0) | _BV(1) | _BV(3) | _BV(5) | _BV(6);
	sleep(100);
	PORTD = 0;
}

/*
 * B0 = alternative master /SS ("/CS"), pin on B2 is not used
 * B2 = /SS for slave operation, not used
 * B3 = MOSI
 * B4 = MISO
 * B5 = SCK
 * D6 = activity led
 */

int main(void)
{
	static	uint16_t		rx_frame_length = 0;
	static	uint8_t 		rx_frame[MAX_FRAME_SIZE];
	static	uint16_t		tx_frame_length;
	static	uint8_t			tx_frame[MAX_FRAME_SIZE];

	static	etherframe_t	*rx_etherframe;
	static	uint8_t			*rx_payload;
	static	uint16_t		rx_payload_length;

	static	etherframe_t	*tx_etherframe;
	static	uint8_t			*tx_payload;
	static	uint16_t		tx_payload_size;
	static	uint16_t		tx_payload_length;

	cli();
	watchdog_stop();

	PRR = 0xff;

	//	pin		spi		i/o
	//	b0 		/cs
	//	b1				oc1a
	//	b2 		/ss		oc1b
	//	b3		mosi
	//	b4		miso
	//	b5		sck
	//	d3				oc2b
	//	d5				oc0b
	//	d6				oc0a

	DDRB	= _BV(1) | _BV(2);
	DDRC	= 0;
	DDRD	= _BV(0) | _BV(1) | _BV(3) | _BV(5) | _BV(6);

	PORTB	= 0;
	PORTC	= 0;
	PORTD	= 0;

	EICRA	= _BV(ISC01);	//	falling edge
	EIMSK	= _BV(INT0);	//	enable INT0

	set_sleep_mode(SLEEP_MODE_IDLE);

	PORTD = 0;
	sleep(1000);
	PORTD = _BV(0) | _BV(1) | _BV(3) | _BV(5) | _BV(6);
	sleep(1000);
	PORTD = 0;

	spi_init();
	init_enc();
	watchdog_start(WATCHDOG_PRESCALER);
	sei();

	for(;;)
	{
		while(!enc_rx_complete() && !enc_rx_error() && !enc_tx_error())
		{
			watchdog_reset();
			enc_arm_interrupt();
			//sleep_mode();
		}

		if(enc_tx_error())
		{
			eth_txerr++;
			cli();
			init_enc();
			sei();
			continue;
		}

		if(enc_rx_error())
		{
			eth_rxerr++;
			enc_clear_errors();
		}

		if(!(rx_frame_length = enc_receive_frame(sizeof(rx_frame), rx_frame)))
			continue;

		eth_pkt_rx++;

		if(rx_frame_length < sizeof(etherframe_t))
			continue;

		PIND = _BV(0);

		rx_etherframe		= (etherframe_t *)rx_frame;
		rx_payload			= &rx_etherframe->payload[0];
		rx_payload_length	= rx_frame_length - sizeof(etherframe_t);

		tx_etherframe		= (etherframe_t *)tx_frame;
		tx_payload			= &tx_etherframe->payload[0];
		tx_payload_size		= sizeof(tx_frame) - sizeof(etherframe_t);
		tx_payload_length	= 0;

		switch(rx_etherframe->ethertype)
		{
			case(et_arp):
			{
				ip_arp_pkt_in++;
				tx_payload_length = process_arp(rx_payload_length, rx_payload, tx_payload_size, tx_payload, &mac, &ipv4);
				if(tx_payload_length)
					ip_arp_pkt_out++;
				break;
			}

			case(et_ipv4):
			{
				ip_ipv4_pkt_in++;
				tx_payload_length = process_ipv4(rx_payload_length, rx_payload, tx_payload_size, tx_payload, &mac, &ipv4);
				if(tx_payload_length)
					ip_ipv4_pkt_out++;
				break;
			}
		}

		if(tx_payload_length)
		{
			PIND = _BV(1);
			tx_etherframe->destination	= rx_etherframe->source;
			tx_etherframe->source		= mac;
			tx_etherframe->ethertype	= rx_etherframe->ethertype;
			tx_frame_length				= sizeof(etherframe_t) + tx_payload_length;

			while(!enc_tx_complete() && !enc_rx_error() && !enc_tx_error())
			{
				watchdog_reset();
				enc_arm_interrupt();
				sleep_mode();
			}

			eth_pkts_buffered = enc_rx_pkts_buffered();

			if(enc_tx_error())
			{
				eth_txerr++;
				cli();
				init_enc();
				sei();
				continue;
			}

			if(enc_rx_error())
			{
				eth_rxerr++;
				enc_clear_errors();
			}

			enc_send_frame(tx_frame_length, tx_frame);
			eth_pkt_tx++;
		}
	}
}
