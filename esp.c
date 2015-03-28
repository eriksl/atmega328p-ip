#include "esp.h"

#include "stats.h"
#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef enum
{
	state_setup_init,
	state_setup_reset_done,
	state_setup_ate_sent,
	state_setup_cipmux_sent,
	state_setup_cipserver_sent,
	state_setup_finished,
} setup_state_t;

typedef enum
{
	state_pull_reset,
		state_pull_retry,
	state_pull_O,
		state_pull_K,
		state_pull_cr,
	state_pull_plus,
		state_pull_I,
		state_pull_P,
		state_pull_D,
		state_pull_comma_1,
		state_pull_connection,
		state_pull_comma_2,
		state_pull_length,
		state_pull_data,
} pull_state_t;

typedef enum
{
	state_push_idle,
	state_push_start,
	state_push_send_data,
} push_state_t;

static uint8_t esp_setup_state;

static uint8_t	*data_in;
static uint16_t	data_in_size;
static uint16_t	data_in_length;
static uint8_t	data_in_state;
static uint8_t	data_in_connection;
static uint16_t	data_in_length_digits;
static uint16_t	data_in_todo;
static uint16_t	data_in_received_ok;

static uint8_t	*data_out;
static uint16_t	data_out_size;
static uint16_t	data_out_length;
static uint8_t	data_out_state;
static uint8_t	data_out_connection;
static uint16_t data_out_current;

static void esp_uart_pull(void)
{
	uint8_t data;

	for(;;)
	{
		if((data_in_state != state_pull_retry) && uart_receive(sizeof(data), &data) != 1)
			break;

		switch(data_in_state)
		{
			case(state_pull_reset):
			case(state_pull_retry):
			{
				if(data == '+')
					data_in_state = state_pull_plus;
				else
					if(data == 'O')
						data_in_state = state_pull_O;
					else
						data_in_state = state_pull_reset;

				break;
			}

			case(state_pull_O):
			{
				if(data == 'K')
					data_in_state = state_pull_K;
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_K):
			{
				if(data == '\r')
					data_in_state = state_pull_cr;
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_cr):
			{
				if(data == '\n')
				{
					data_in_received_ok = 1;
					data_in_state = state_pull_reset;
				}
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_plus):
			{
				if(data == 'I')
					data_in_state = state_pull_I;
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_I):
			{
				if(data == 'P')
					data_in_state = state_pull_P;
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_P):
			{
				if(data == 'D')
					data_in_state = state_pull_D;
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_D):
			{
				if(data == ',')
					data_in_state = state_pull_comma_1;
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_comma_1):
			{
				if((data >= '0') && (data <= '9'))
				{
					data_in_connection = data - '0';
					data_in_state = state_pull_connection;
				}
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_connection):
			{
				if(data == ',')
					data_in_state = state_pull_comma_2;
				else
					data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_comma_2):
			{
				if((data >= '0') && (data <= '9'))
				{
					data_in_length_digits = 0;
					data_in_todo = 0;
					data_in_state = state_pull_length; // fall through to state_pull_length
				}
				else
				{
					data_in_state = state_pull_retry;

					break;
				}
			}

			case(state_pull_length):
			{
				if(data_in_length_digits > 4) // emergency brake
					data_in_state = state_pull_reset;
				else
					if(data == ':')
					{
						data_in_state = state_pull_data;
						data_in_length = 0;
					}
					else
						if((data >= '0') && (data <= '9'))
						{
							data_in_length_digits++;
							data_in_todo = (data_in_todo * 10) + (data - '0');
						}
						else
							data_in_state = state_pull_retry;

				break;
			}

			case(state_pull_data):
			{
				if(data_in_todo > 0)
				{
					data_in_todo--;

					if(data_in_length < data_in_size)
					{
						if((data >= ' ') && (data <= '~'))
						{
							data_in[data_in_length] = data;
							data_in_length++;
						}
					}
				}
				else
					data_in_state = state_pull_reset;

				break;
			}
		}
	}
}

static void esp_uart_push(void)
{
	static const __flash char cipsendcmd[] = "AT+CIPSEND=%u,%u\r\n";

	uint8_t cipsend[24];
	uint8_t cipsendlength;

	switch(data_out_state)
	{
		case(state_push_idle):
		{
			break;
		}

		case(state_push_start):
		{
			cipsendlength = snprintf_P((char *)cipsend, sizeof(cipsend), cipsendcmd, data_out_connection, data_out_length);
			uart_transmit(cipsendlength, cipsend);
			data_out_state = state_push_send_data;

			break;
		}

		case(state_push_send_data):
		{
			data_out_current += uart_transmit(data_out_length - data_out_current, data_out + data_out_current);

			if(data_out_current >= data_out_length)
			{
				data_out_length = 0;
				data_out_current = 0;
				data_out_state = state_push_idle;
			}

			break;
		}
	}
}

static void esp_uart_flush()
{
	data_in_length		= 0;
	data_in_state		= state_pull_reset;
	data_in_todo		= 0;
	data_in_connection	= 0;
	data_in_length		= 0;

	data_out_length		= 0;
	data_out_state		= state_push_idle;
	data_out_connection	= 0;
	data_out_current	= 0;
}

void esp_init(uint16_t rsize, uint8_t *rbuffer, uint16_t ssize, uint8_t *sbuffer)
{
	data_in			= rbuffer;
	data_in_size	= rsize;
	data_out		= sbuffer;
	data_out_size	= ssize;

	esp_uart_flush();

	uart_init();
	uart_baud(38400);

	PORTD &= ~_BV(7);
	esp_setup_state = state_setup_init;
}

void esp_periodic(void)
{
	static const uint8_t *ate		= (uint8_t *)"ATE0\r\n";
	static const uint8_t *cipmux	= (uint8_t *)"AT+CIPMUX=1\r\n";
	static const uint8_t *cipserver	= (uint8_t *)"AT+CIPSERVER=1,23\r\n";

	esp_uart_pull();
	esp_uart_push();

	switch(esp_setup_state)
	{
		case(state_setup_init):
		{
			if(t1_interrupts > 2)
			{
				PORTD |= _BV(7);
				esp_setup_state = state_setup_reset_done;
			}

			break;
		}

		case(state_setup_reset_done):
		{
			if(t1_interrupts > 64)
			{
				data_in_received_ok = 0;
				esp_uart_flush();
				uart_transmit(strlen((const char *)ate), ate);
				esp_setup_state = state_setup_ate_sent;
			}

			break;
		}

		case(state_setup_ate_sent):
		{
			if(data_in_received_ok)
			{
				data_in_received_ok = 0;
				uart_transmit(strlen((const char *)cipmux), cipmux);
				esp_setup_state = state_setup_cipmux_sent;
			}

			break;
		}

		case(state_setup_cipmux_sent):
		{
			if(data_in_received_ok)
			{
				data_in_received_ok = 0;
				uart_transmit(strlen((const char *)cipserver), cipserver);
				esp_setup_state = state_setup_cipserver_sent;
			}

			break;
		}

		case(state_setup_cipserver_sent):
		{
			if(data_in_received_ok)
				esp_setup_state = state_setup_finished;

			break;
		}

		case(state_setup_finished):
		{
			break;
		}
	}
}

uint8_t esp_receive_finished(void)
{
	if(data_in_todo || (data_in_length == 0))
		return(0);

	return(1);
}

uint16_t esp_receive_length(uint8_t *connection)
{
	uint16_t length;

	if(!esp_receive_finished())
		return(0);

	length = data_in_length;
	data_in_length = 0;

	if(connection)
		*connection = data_in_connection;

	return(length);
}

void esp_send_start(uint16_t length, uint8_t *connection)
{
	data_out_length		= length;
	data_out_current	= 0;
	data_out_state		= state_push_start;

	if(connection)
		data_out_connection = *connection;
}

uint8_t esp_send_finished(void)
{
	return(data_out_state == state_push_idle);
}
