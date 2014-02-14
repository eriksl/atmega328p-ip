#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"

void spi_init(void)
{
	//	 pin	spi		i/o
	//	*b0 	/cs
	//	 b1				oc1a
	//	*b2 	/ss		oc1b
	//	*b3		mosi
	//	*b4		miso
	//	*b5		sck
	//	 d3				oc2b
	//	 d5				oc0b
	//	 d6				oc0a

#if 1
	SPCR = 0x00; // ensure SPI is properly reset
	_delay_ms(1);
	PRR |= _BV(PRSPI); // turn SPI off
	_delay_ms(1);
#endif
	
	DDRB |= _BV(0) | _BV(2) | _BV(3) | _BV(5);
	DDRB &= ~(_BV(4));

	PORTB |= _BV(0); // /CS = 1

	PRR &= ~_BV(PRSPI); // turn SPI on
	_delay_ms(1);

	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
	SPSR = 0;
}

void spi_start(void)
{
	PORTB &= ~_BV(0);	// /CS = 0
}

void spi_stop(void)
{
	PORTB |= _BV(0);	// /CS = 1
}

uint8_t spi_io(uint8_t out)
{
	SPDR = out;

	while(!(SPSR & _BV(SPIF)))
		(void)0;

	return(SPDR);
}
