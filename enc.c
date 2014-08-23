#include <avr/io.h>
#include <avr/interrupt.h>

#include "enc-private.h"
#include "enc.h"
#include "spi.h"
#include "stats.h"
#include "util.h"

#include <stdint.h>

ISR (INT0_vect, ISR_NOBLOCK)
{
	eth_interrupts++;
}

static uint16_t	next_frame_pointer;

static void sc(void)
{
	spi_start();
	spi_io(PAD);
	spi_stop();

	spi_start();
	spi_io(SC);
	spi_io(PAD);
	spi_stop();
}

static uint8_t rcr(uint8_t reg_idx, uint8_t delayed)
{
	uint8_t rv;

	spi_start();
	spi_io(RCR(reg_idx));
	rv = spi_io(PAD);
	if(delayed)
		rv = spi_io(PAD);
	spi_stop();

	return(rv);
}

static void wcr(uint8_t reg_idx, uint8_t data)
{
	spi_start();
	spi_io(WCR(reg_idx));
	spi_io(data);
	spi_stop();
}

static void bfs(uint8_t reg_idx, uint8_t data)
{
	spi_start();
	spi_io(BFS(reg_idx));
	spi_io(data);
	spi_stop();
}

static void bfc(uint8_t reg_idx, uint8_t data)
{
	spi_start();
	spi_io(BFC(reg_idx));
	spi_io(data);
	spi_stop();
}

static void select_bank(uint8_t bank)
{
	bfc(REG(ECON1), ECON1_BANK_MASK);
	bfs(REG(ECON1), bank & ECON1_BANK_MASK);
}

static uint8_t read_register_ll(uint16_t reg_idx)
{
	select_bank(BANK(reg_idx));
	return(rcr(REG(reg_idx), TYPE(reg_idx) == TYPE(REG_MAC)));
}

static void write_register_ll(uint16_t reg_idx, uint8_t data)
{
	select_bank(BANK(reg_idx));
	wcr(REG(reg_idx), data);
}

static void clearbits_register(uint16_t reg_idx, uint8_t data)
{
	select_bank(BANK(reg_idx));
	bfc(REG(reg_idx), data);
}

static void setbits_register(uint16_t reg_idx, uint8_t data)
{
	select_bank(BANK(reg_idx));
	bfs(REG(reg_idx), data);
}

static uint8_t read_memory_8(void)
{
	uint8_t rv;

	spi_start();
	spi_io(RBM);
	rv = spi_io(PAD);
	spi_stop();

	return(rv);
}

static void write_memory_8(uint8_t data)
{
	spi_start();
	spi_io(WBM);
	spi_io(data);
	spi_stop();
}

static uint16_t read_memory_16(void)
{
	return((read_memory_8() << 0) | (read_memory_8() << 8));
}

uint16_t read_register(uint16_t reg)
{
	uint8_t	rvh, rvl;
	uint16_t rv = 0;

	switch(TYPE(reg))
	{
		case(TYPE(REG_ETH)):
		case(TYPE(REG_MAC)):
		{
			rv = read_register_ll(reg);
			break;
		}

		case(TYPE(REG_PHY)):
		{
			write_register_ll(MIREGADR, REG(reg));
			write_register_ll(MICMD, _BV(MICMD_MIIRD));

			while(read_register_ll(MISTAT) & _BV(MISTAT_BUSY))
				(void)0;

			write_register_ll(MICMD, 0x00);
			rvh = read_register_ll(MIRDH);
			rvl = read_register_ll(MIRDL);
			rv	= (rvh << 8) | (rvl << 0);

			break;
		}
	}

	return(rv);
}

static void write_register(uint16_t reg, uint16_t data)
{
	switch(TYPE(reg))
	{
		case(TYPE(REG_ETH)):
		case(TYPE(REG_MAC)):
		{
			write_register_ll(reg, data & 0xff);
			break;
		}

		case(TYPE(REG_PHY)):
		{
			write_register_ll(MIREGADR, REG(reg));
			write_register_ll(MIWRL, (data >> 0) & 0xff);
			write_register_ll(MIWRH, (data >> 8) & 0xff);

			while(read_register_ll(MISTAT) & _BV(MISTAT_BUSY))
				(void)0;

			break;
		}
	}
}

void enc_init(uint16_t max_frame_size, const mac_addr_t *mac)
{
	sc();			// reset
	sleep(100);		// see erratum

	EICRA &= ~_BV(ISC00);	// INT0
	EICRA |=  _BV(ISC01);	// falling edge
	EIMSK |=  _BV(INT0);	// enable INT0

	write_register(EIE, 0x00);
	write_register(EIR, 0x00);

	next_frame_pointer = RXBUFFER;

	// read buffer

	write_register(ERXSTL, (RXBUFFER >> 0) & 0xff);
	write_register(ERXSTH, (RXBUFFER >> 8) & 0xff);

	write_register(ERXRDPTL, (RXBUFFER >> 0) & 0xff);
	write_register(ERXRDPTH, (RXBUFFER >> 8) & 0xff);

	write_register(ERXNDL, ((RXBUFFER + RXBUFLEN - 1) >> 0) & 0xff);
	write_register(ERXNDH, ((RXBUFFER + RXBUFLEN - 1) >> 8) & 0xff);

	// write buffer

	write_register(ETXSTL, (TXBUFFER >> 0) & 0xff);
	write_register(ETXSTH, (TXBUFFER >> 8) & 0xff);

	write_register(ETXNDL, ((TXBUFFER + TXBUFLEN - 1) >> 0) & 0xff);
	write_register(ETXNDH, ((TXBUFFER + TXBUFLEN - 1) >> 8) & 0xff);

	write_register(MACON1, _BV(MACON1_MARXEN));
	write_register(MACON2, 0x00); // mac out of reset
	write_register(MACON3, (MACON3_PADCFG_PAD60 << MACON3_PADCFG) |
					_BV(MACON3_TXCRCEN) | _BV(MACON3_FRMLNEN) |
					_BV(MACON3_FULDPX));

	write_register(MAMXFLL, (max_frame_size >> 0) & 0xff);
	write_register(MAMXFLH, (max_frame_size >> 8) & 0xff);

	write_register(MAIPGL, 0x12);	// inter-frame gap non back-to-back
	write_register(MAIPGH, 0x0c);

	write_register(MABBIPG, 0x15);	// inter-frame gap back-to-back

	// mac address

	write_register(MAADR0, mac->byte[5]);
	write_register(MAADR1, mac->byte[4]);
	write_register(MAADR2, mac->byte[3]);
	write_register(MAADR3, mac->byte[2]);
	write_register(MAADR4, mac->byte[1]);
	write_register(MAADR5, mac->byte[0]);

	write_register(ERXFCON, _BV(ERXFCON_UCEN) | _BV(ERXFCON_CRCEN) | _BV(ERXFCON_BCEN));

	write_register(PHCON1, _BV(PHCON1_PDPXMD));	// enable full duplex
	write_register(PHCON2, _BV(PHCON2_HDLDIS));	// disable loopback

	setbits_register(ECON1, _BV(ECON1_RXEN));
	setbits_register(ECON2, _BV(ECON2_AUTOINC));
}

void enc_set_led(uint8_t how1, uint8_t how2)
{
	write_register(PHLCON, ((how1 & 0x0f) << PHLCON_LEDA) | ((how2 & 0x0f) << PHLCON_LEDB) |
		_BV(PHLCON_LFRQ1) | _BV(PHLCON_STRCH));
}

void enc_send_frame(const uint8_t *frame, uint16_t length)
{
	uint16_t current;

	if(read_register(EIR) & _BV(EIR_TXERIF)) // transmit stuck, reset
	{
		eth_txerr++;
		setbits_register  (ECON1, _BV(ECON1_TXRST));
		clearbits_register(ECON1, _BV(ECON1_TXRST));
		clearbits_register(EIR,   _BV(EIR_TXERIF));
	}

	while(read_register(ECON1) & (_BV(ECON1_TXRTS) | _BV(ECON1_DMAST)))
		(void)0;

	/* frame start */

	write_register(EWRPTL, (TXBUFFER >> 0) & 0xff);
	write_register(EWRPTH, (TXBUFFER >> 8) & 0xff);

	/* frame end */

	write_register(ETXNDL, ((TXBUFFER + length) >> 0) & 0xff);
	write_register(ETXNDH, ((TXBUFFER + length) >> 8) & 0xff);

	/* frame control byte */

	write_memory_8(0x00); // 0x00 = macon3

	for(current = 0; current < length; current++)
		write_memory_8(frame[current]);

	clearbits_register(EIR, _BV(EIR_TXIF) | _BV(EIR_TXERIF));
	setbits_register(ECON1,	_BV(ECON1_TXRTS));

	while(read_register(ECON1) & (_BV(ECON1_TXRTS) | _BV(ECON1_DMAST)))
		(void)0;

	eth_pkt_tx++;
}

uint16_t enc_receive_frame(uint8_t *frame, uint16_t buffer_length)
{
	uint16_t length, current;
	uint8_t	 rxstat;

	if(read_register(ECON1) & _BV(EIR_RXERIF))
	{
		eth_rxerr++;
		clearbits_register(EIR, _BV(EIR_RXERIF));
	}

	if(read_register(EPKTCNT) == 0)
	{
		eth_nopkt_rx++;
		return(0);
	}

	eth_pkt_rx++;

	write_register(ERDPTL, (next_frame_pointer >> 0) & 0xff);
	write_register(ERDPTH, (next_frame_pointer >> 8) & 0xff);

	next_frame_pointer	= read_memory_16();
	length				= read_memory_16();
	rxstat				= read_memory_16();
	length				-= 4; // - CRC

	if(length > buffer_length)
		length = buffer_length - 1;

	if(!(rxstat & 0x80)) // frame valid
	{
		eth_rxerr++;
		length = 0;
	}

	for(current = 0; current < length; current++)
		frame[current] = read_memory_8();

	write_register(ERXRDPTL, (next_frame_pointer >> 0) & 0xff); // move rx pointer, free memory
	write_register(ERXRDPTH, (next_frame_pointer >> 8) & 0xff);

	setbits_register(ECON2, _BV(ECON2_PKTDEC));

	return(length);
}
