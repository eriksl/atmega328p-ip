#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

static uint8_t txbuffer[128];
static uint8_t txbuffer_in;
static uint8_t txbuffer_out;

static uint8_t rxbuffer[128];
static uint8_t rxbuffer_in;
static uint8_t rxbuffer_out;

ISR(USART_RX_vect)
{
    uart_rx_interrupts++;

	rxbuffer[rxbuffer_in] = UDR0;
	rxbuffer_in = (rxbuffer_in + 1) % sizeof(rxbuffer);
}

ISR(USART_UDRE_vect)
{
    uart_tx_interrupts++;

	if(txbuffer_in == txbuffer_out)
		UCSR0B &= ~_BV(UDRIE0);
	else
	{
		UDR0 = txbuffer[txbuffer_out];
		txbuffer_out = (txbuffer_out + 1) % sizeof(txbuffer);
	}
}

void uart_init(void)
{
	PRR		&= PRUSART0;

	UDR0	= 0;
	UCSR0A	= 0x00;
	UCSR0A	= 0x00;
	UCSR0B	= _BV(TXEN0) | _BV(RXEN0);
	UCSR0C	= _BV(UCSZ00) | _BV(UCSZ01); // 8 bits character size

	txbuffer_in = 0;
	txbuffer_out = 0;

	rxbuffer_in = 0;
	rxbuffer_out = 0;
}

void uart_baud(uint32_t baud)
{
	UCSR0B &= ~(_BV(RXCIE0) | _BV(UDRIE0));

	baud *= 16;
	UBRR0 = (uint16_t)(((uint32_t)F_CPU / baud) - (uint32_t)1);

	UCSR0B |= _BV(RXCIE0);
}

uint16_t uart_receive(uint16_t size, uint8_t *buffer)
{
	uint16_t received = 0;

	while((size > 0) && (rxbuffer_out != rxbuffer_in))
	{
		*buffer++ = rxbuffer[rxbuffer_out];
		rxbuffer_out = (rxbuffer_out + 1) % sizeof(rxbuffer);
		received++;
		size--;
	}

	return(received);
}

uint16_t uart_transmit(uint16_t length, const uint8_t *buffer)
{
	uint16_t sent = 0;

	UCSR0B &= ~_BV(UDRIE0);

	while((length > 0) && (((txbuffer_in + 1) % sizeof(txbuffer)) != txbuffer_out))
	{
		txbuffer[txbuffer_in] = *buffer++;
		txbuffer_in = (txbuffer_in + 1) % sizeof(txbuffer);
		sent++;
		length--;
	}

	UCSR0B |= _BV(UDRIE0);

	return(sent);
}
