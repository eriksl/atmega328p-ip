#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

#include "uart-line.h"

static uint8_t txbuffer[255];
static uint16_t txbuffer_current;

static uint8_t rxbuffer[255];
static uint16_t rxbuffer_current;

ISR(USART_RX_vect)
{
	uint8_t data;

    uart_rx_interrupts++;

	if(UCSR0A & _BV(FE0))
		uart_fe++;

	if(UCSR0A & _BV(DOR0))
		uart_overrun++;

	data = UDR0;

	if((data != '\n') && (data != '\r') && ((rxbuffer_current + 2) < sizeof(rxbuffer)))
		rxbuffer[rxbuffer_current++] = data;

	if(data == '\n')
	{
		UCSR0B &= ~_BV(RXCIE0); // stop receiving until buffer fetched by application
		rxbuffer[rxbuffer_current] = '\0';
	}
}

ISR(USART_UDRE_vect)
{
	uint8_t data;

    uart_tx_interrupts++;

	data = txbuffer[txbuffer_current++];

	if(data != '\0')
		UDR0 = data;
	else
		UCSR0B &= ~_BV(UDRIE0); // stop sending until buffer filled by application
}

void uart_init(void)
{
	PRR		|=  _BV(PRUSART0);
	PRR		&= ~_BV(PRUSART0);

	UDR0	= 0;
	UCSR0A	= 0x00;
	UCSR0B	= _BV(TXEN0) | _BV(RXEN0);
	UCSR0C	= _BV(UCSZ00) | _BV(UCSZ01); // 8 bits character size

	txbuffer_current = 0;
	rxbuffer_current = 0;

	UCSR0B |= _BV(RXCIE0); // start receiving
}

void uart_baud(uint32_t baud)
{
	uint8_t ucsr0b;
	uint8_t bitmask = _BV(TXEN0) | _BV(RXEN0) | _BV(UDRIE0) | _BV(RXCIE0);

	ucsr0b = UCSR0B & bitmask;
	UCSR0B &= ~bitmask;

	baud *= 16;
	UBRR0 = (uint16_t)(((uint32_t)F_CPU / baud) - (uint32_t)1);

	UCSR0B |= ucsr0b;
}

uint8_t uart_transmit(const uint8_t *buffer)
{
	if(!uart_transmit_ready())
		return(0);

	strlcpy(txbuffer, buffer, sizeof(txbuffer));

	txbuffer_current = 0;
	UCSR0B |= _BV(UDRIE0); // start transmitting

	return(1);
}

uint8_t uart_receive(uint16_t size, uint8_t *buffer)
{
	if(!uart_receive_ready())
		return(0);

	strlcpy(buffer, rxbuffer, size);

	rxbuffer_current = 0;
	UCSR0B |= _BV(RXCIE0); // start receiving

	return(1);
}
