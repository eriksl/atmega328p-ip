#include "stats.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

#include "uart-telnet.h"

static uint8_t txbuffer[640];
static uint16_t txbuffer_current;

static uint8_t rxbuffer[128];
static uint16_t rxbuffer_current;

enum
{
	ts_raw,
	ts_dodont,
	ts_data,
};

static uint8_t telnet_state;

ISR(USART_RX_vect)
{
	uint8_t data;

    uart_rx_interrupts++;

	if(UCSR0A & _BV(FE0))
		uart_fe++;

	if(UCSR0A & _BV(DOR0))
		uart_overrun++;

	data = UDR0;

	switch(telnet_state)
	{
		case(ts_raw):
		{
			if((data != 0xff) && (data != '\0') && (data != '\n') && (data != '\r') && ((rxbuffer_current + 2) < sizeof(rxbuffer)))
				rxbuffer[rxbuffer_current++] = data;

			if(data == '\n')
			{
				UCSR0B &= ~_BV(RXCIE0); // stop receiving until buffer fetched by application
				rxbuffer[rxbuffer_current] = '\0';
			}

			if(data == 0xff)
				telnet_state = ts_dodont;

			break;
		}

		case(ts_dodont):
		{
			telnet_state = ts_data;
			break;
		}

		case(ts_data):
		{
			telnet_state = ts_raw;
			break;
		}
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
	static const uint16_t ubrr0 = (F_CPU / (460800UL * 8UL)) - 1UL;	// baud rate 460800, double data rate

	PRR		|=  _BV(PRUSART0);
	PRR		&= ~_BV(PRUSART0);

	UDR0	= 0;
	UCSR0A	= _BV(U2X0);					// usart double speed mode
	UCSR0B	= _BV(TXEN0) | _BV(RXEN0);		// enable transmitter and receiver
	UCSR0C	= _BV(UCSZ00) | _BV(UCSZ01);	// async uart, 8N1

	UBRR0 = ubrr0;

	txbuffer_current = 0;
	rxbuffer_current = 0;

	telnet_state = ts_raw;

	UCSR0B |= _BV(RXCIE0); // start receiving
}

uint8_t uart_transmit(const uint8_t *buffer)
{
	strlcpy(txbuffer, buffer, sizeof(txbuffer));

	txbuffer_current = 0;
	UCSR0B |= _BV(UDRIE0); // start transmitting

	return(1);
}

uint8_t uart_receive(uint16_t size, uint8_t *buffer)
{
	strlcpy(buffer, rxbuffer, size);

	rxbuffer_current = 0;
	UCSR0B |= _BV(RXCIE0); // start receiving

	return(1);
}
