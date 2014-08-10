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
	watchdog_prescaler = WATCHDOG_PRESCALER_256,
};

typedef struct
{
	uint16_t	multiplier;
	uint16_t	offset;
} temperature_calibration_t;

typedef struct
{
	temperature_calibration_t	temp_cal_data[8];
	mac_addr_t					my_mac_address;
} eeprom_t;

static const eeprom_t *eeprom = (eeprom_t *)0;

static ipv4_addr_t my_ipv4_address;
static mac_addr_t my_mac_address;
static uint16_t bootp_timer = 0;

ISR(WDT_vect, ISR_NOBLOCK)
{
	wd_interrupts++;
}

uint16_t receive_frame(uint8_t *frame, uint16_t frame_size)
{
	static uint16_t frame_length;

	if(!enc_rx_complete() && !enc_tx_error() && !enc_tx_error())
	{
		enc_arm_interrupt();
		watchdog_reset();
		sleep_mode();
	}

	if(enc_tx_error())
	{
		eth_txerr++;
		enc_clear_errors();
		return(0);
	}

	if(enc_rx_error())
	{
		eth_rxerr++;
		enc_clear_errors();
		return(0);
	}

	if(!enc_rx_complete())
		return(0);

	if(!(frame_length = enc_receive_frame(frame_size, frame)))
		return(0);

	eth_pkt_rx++;

	if(frame_length < sizeof(etherframe_t))
		return(0);

	return(frame_length);
}

void send_frame(const uint8_t *frame, uint16_t frame_length)
{
	while(!enc_tx_complete() && !enc_rx_error() && !enc_tx_error())
	{
		enc_arm_interrupt();
		watchdog_reset();
		sleep_mode();
	}

	eth_pkts_buffered = enc_rx_pkts_buffered();

	if(enc_tx_error())
	{
		eth_txerr++;
		enc_clear_errors();
	}

	if(enc_rx_error())
	{
		eth_rxerr++;
		enc_clear_errors();
	}

	enc_send_frame(frame_length, frame);
	eth_pkt_tx++;
}

int main(void)
{
	static	uint8_t 		rx_frame[max_frame_size];
	static	uint8_t			tx_frame[max_frame_size];

	static	uint16_t		rx_frame_length;
	static	uint16_t		tx_frame_length;
	static	uint16_t		tx_payload_length;

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

	my_mac_address.byte[0] = eeprom_read_uint8(&eeprom->my_mac_address.byte[0]);
	my_mac_address.byte[1] = eeprom_read_uint8(&eeprom->my_mac_address.byte[1]);
	my_mac_address.byte[2] = eeprom_read_uint8(&eeprom->my_mac_address.byte[2]);
	my_mac_address.byte[3] = eeprom_read_uint8(&eeprom->my_mac_address.byte[3]);
	my_mac_address.byte[4] = eeprom_read_uint8(&eeprom->my_mac_address.byte[4]);
	my_mac_address.byte[5] = eeprom_read_uint8(&eeprom->my_mac_address.byte[5]);

	my_ipv4_address.byte[0] = 0;
	my_ipv4_address.byte[1] = 0;
	my_ipv4_address.byte[2] = 0;
	my_ipv4_address.byte[3] = 0;

	PORTD = _BV(0);
	sleep(500);
	PIND = _BV(0) | _BV(1);
	sleep(500);
	PIND = _BV(0) | _BV(1);
	sleep(500);
	PORTD = 0;

	spi_init();
	twi_master_init();
	enc_init(max_frame_size, &my_mac_address);
	enc_set_led(PHLCON_LED_RCV, PHLCON_LED_XMIT);

	watchdog_start(watchdog_prescaler);
	sei();

	application_init();

	for(;;)
	{
		application_idle();

		if(ipv4_address_match(&my_ipv4_address, &ipv4_addr_zero))
		{
			if(bootp_timer == 0)
			{
				tx_frame_length = bootp_create_request(tx_frame, &my_mac_address);
				send_frame(tx_frame, tx_frame_length);
				bootp_timer = 40;
			}

			if(bootp_timer > 0)
				bootp_timer--;
		}
		
		if((rx_frame_length = receive_frame(rx_frame, sizeof(rx_frame))) == 0)
			continue;

		if((tx_frame_length = ethernet_process_frame(rx_frame, rx_frame_length, tx_frame, sizeof(tx_frame), &my_mac_address, &my_ipv4_address)) == 0)
			continue;

		send_frame(tx_frame, tx_frame_length);
	}
}
