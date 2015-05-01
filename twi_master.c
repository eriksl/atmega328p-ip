#include "twi_master.h"

#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>

uint8_t twi_master_status(void)
{
	uint8_t twsr;

	twsr = TWSR & 0xf8;
	return(twsr >> 3);
}

void twi_master_wait(void)
{
	while(!(TWCR & _BV(TWINT)))
		(void)0;
}

void twi_master_send_stop_no_wait(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

void twi_master_send_stop(void)
{
	twi_master_send_stop_no_wait();

	while(TWCR & _BV(TWSTO))
		(void)0;
}

uint8_t twi_master_send_start(void)
{
	uint8_t stat;

	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTA);

	twi_master_wait();
	stat = twi_master_status();

	if(stat != tms_start_sent)
	{
		twi_master_send_stop();
		return(tme_send_start | (stat << 4));
	}

	return(tme_ok);
}

uint8_t twi_master_send_address(uint8_t address, uint8_t request_write)
{
	uint8_t stat;

	TWDR = (address << 1) | (request_write ? 0 : 1);
	TWCR = _BV(TWINT) | _BV(TWEN);

	twi_master_wait();
	stat = twi_master_status();

	if(request_write)
	{
		if(stat != tms_addr_w_ack)
		{
			twi_master_send_stop();
			return(tme_send_address_w | (stat << 4));
		}
	}
	else
	{
		if(stat != tms_addr_r_ack)
		{
			twi_master_send_stop();
			return(tme_send_address_r | (stat << 4));
		}
	}

	return(tme_ok);
}

uint8_t twi_master_send_byte(uint8_t data)
{
	uint8_t stat;

	TWDR = data;
	TWCR = _BV(TWINT) | _BV(TWEN);

	twi_master_wait();
	stat = twi_master_status();

	if(stat != tms_data_sent_ack)
	{
		twi_master_send_stop();
		return(tme_send_byte | (stat << 4));
	}

	return(tme_ok);
}

uint8_t twi_master_receive_byte(uint8_t *data)
{
	uint8_t stat;

	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);

	twi_master_wait();
	stat = twi_master_status();

	if((stat != tms_data_recvd_ack) && (stat != tms_data_recvd_nack))
	{
		twi_master_send_stop();
		return(tme_receive_byte | (stat << 4));
	}

	if(stat == tms_data_recvd_nack)
		*data = 0xfe;
	else
		*data = TWDR;

	return(tme_ok);
}

uint8_t twi_master_send_nack(void)
{
	uint8_t stat;

	TWCR = _BV(TWINT) | _BV(TWEN);

	twi_master_wait();
	stat = twi_master_status();

	if((stat != tms_data_recvd_ack) && (stat != tms_data_recvd_nack))
	{
		twi_master_send_stop();
		return(tme_send_nack | (stat << 4));
	}

	return(tme_ok);
}

uint8_t twi_master_send(uint8_t address, uint8_t length, const uint8_t *buffer)
{
	uint8_t rv;

	if((rv = twi_master_send_start()) != tme_ok)
		return(rv);

	if((rv = twi_master_send_address(address, 1)) != tme_ok)
		return(rv);

	for(; length > 0; length--, buffer++)
		if((rv = twi_master_send_byte(*buffer)) != tme_ok)
			return(rv);

	twi_master_send_stop();

	return(tme_ok);
}

uint8_t twi_master_receive(uint8_t address, uint8_t size, uint8_t *buffer)
{
	uint8_t rv, ix;

	if((rv = twi_master_send_start()) != tme_ok)
		return(rv);

	if((rv = twi_master_send_address(address, 0)) != tme_ok)
		return(rv);

	for(ix = 0; ix < size; ix++)
		if((rv = twi_master_receive_byte(&buffer[ix])) != tme_ok)
			return(rv);

	if((rv = twi_master_send_nack()) != tme_ok)
		return(rv);

	twi_master_send_stop();

	return(tme_ok);
}

void twi_master_error(uint8_t *dst, uint16_t size, uint8_t error)
{
	static const __flash uint8_t format_string[] = "TWI error: %x, state %x\n";

	snprintf_P(dst, size, format_string, error & 0x0f, (error & 0xf0) >> 4);
}

void twi_master_init(void)
{
	PRR		|= _BV(PRTWI);
	TWCR	= 0x00;
	DDRC	&= ~(_BV(4) | _BV(5));	// SCL, SDA HiZ

	PRR		&= ~_BV(PRTWI);
	TWBR	= 92;
	TWSR	= 0;
	TWDR	= 0xff;
	TWCR	= _BV(TWEN);
}

void twi_master_recover(void)
{
	uint8_t ix;

	// try to send stop condition

	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
	_delay_ms(1);
	TWCR = _BV(TWINT) | _BV(TWEN);
	_delay_ms(1);

	// try to resolve bus locked by slave

	TWCR = 0x00;	// disable TWI, make ports available

	DDRC |= _BV(5);		//	SCL	-> output
	DDRC &= ~_BV(6);	//	SDA	-> HiZ

	for(ix = 16; ix > 0; --ix)
	{
		PORTC &= ~_BV(5);	// SCL low
		_delay_ms(1);
		PORTC |= _BV(5);	// SCL high
		_delay_ms(1);
	}

	return(twi_master_init());
}

