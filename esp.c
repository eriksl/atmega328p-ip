#include "esp.h"

#include "stats.h"
#include "uart-line.h"
#include "util.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef enum
{
	wd_esp_init = 300, // 600 seconds esp watchdog timeout
} const_t;

typedef enum
{
	token_nodata,
	token_empty,
	token_error,
	token_unknown,
	token_ok,
	token_link,
	token_unlink,
	token_prompt,
	token_sendok,
	token_systemready,
	token_ipd,
	token_none,
	token_any,
} token_t;

static const __flash char init_string_0[] = "ATE0\r\n";
static const __flash char init_string_1[] = "AT+CIPMUX=1\r\n";
static const __flash char init_string_2[] = "AT+CIPSERVER=1,23\r\n";

static const __flash char token_match_ok[]			= "OK";
static const __flash char token_match_link[]		= "Link";
static const __flash char token_match_unlink[]		= "Unlink";
static const __flash char token_match_prompt[]		= "> ";
static const __flash char token_match_sendok[]		= "SEND OK";
static const __flash char token_match_systemready[]	= "[System Ready";
static const __flash char token_match_ipd[]			= "+IPD,";

static uint8_t receive(uint16_t timeout, uint8_t *conn_id, uint16_t buffer_size, uint8_t *buffer)
{
	uint16_t data_length;
	uint8_t receive_uart_buffer[255];
	uint8_t *rup;

	while((timeout-- > 0) && !uart_receive_ready())
		pause_idle();

	if(!uart_receive(sizeof(receive_uart_buffer), receive_uart_buffer))
		return(token_nodata);

	if(receive_uart_buffer[0] == '\0')
		return(token_empty);

	if(!strcmp_P(receive_uart_buffer, token_match_ok))
		return(token_ok);

	if(!strcmp_P(receive_uart_buffer, token_match_link))
		return(token_link);

	if(!strcmp_P(receive_uart_buffer, token_match_unlink))
		return(token_unlink);

	if(!strcmp_P(receive_uart_buffer, token_match_prompt))
		return(token_prompt);

	if(!strcmp_P(receive_uart_buffer, token_match_sendok))
		return(token_sendok);

	if(!strncmp_P(receive_uart_buffer, token_match_systemready, sizeof(token_match_systemready) - 1))
		return(token_systemready);

	if(!strncmp_P(receive_uart_buffer, token_match_ipd, sizeof(token_match_ipd) - 1))
	{
		rup = receive_uart_buffer + sizeof(token_match_ipd) - 1;

		if(conn_id)
			*conn_id = *rup - '0';

		rup++;

		if(*rup++ != ',')
			return(token_error);

		for(data_length = 0; (*rup >= '0') && (*rup <= '9'); rup++)
		{
			data_length *= 10;
			data_length += *rup - '0';
		}

		if(*rup++ != ':')
			return(token_error);

		if(data_length > (buffer_size - 2))
			data_length = buffer_size - 2;

		if(buffer)
			strlcpy(buffer, (const char *)rup, data_length);

		return(token_ipd);
	}

	return(token_unknown);
}

static uint8_t writeread(uint16_t timeout, const uint8_t *cmd, uint8_t expect_token)
{
	uint8_t token;
	uint16_t t;

	for(t = timeout; t > 0; t--)
	{
		if(!uart_transmit(cmd))
			break;

		pause_idle();
	}

	if(t == 0)
		goto timeout;

	for(t = timeout; t > 0; t--)
	{
		token = receive(1, 0, 0, 0);

		if(token == token_nodata)
			continue;

		if(expect_token == token)
			goto ok;
	}

timeout:
	return(0);

ok:
	return(1);
}

void esp_init(uint32_t baud)
{
	uint8_t buffer[32];

	uart_init();
	//uart_baud(baud);

	PORTC &= ~_BV(3);
	msleep(10);
	PORTC |= _BV(3);

	while(receive(0xffff, 0, 0, 0) != token_systemready)
		(void)0;

	strlcpy_P(buffer, init_string_0, sizeof(buffer));
	writeread(0xffff, buffer, token_ok);

	strlcpy_P(buffer, init_string_1, sizeof(buffer));
	writeread(0xffff, buffer, token_ok);

	strlcpy_P(buffer, init_string_2, sizeof(buffer));
	writeread(0xffff, buffer, token_ok);

	esp_wd_timeout = wd_esp_init;
}

uint8_t esp_read(uint8_t *conn_id, uint16_t buffer_size, uint8_t *buffer)
{
	uint8_t token;

	token = receive(0, conn_id, buffer_size, buffer);

	if(token == token_ipd)
		return(1);

	return(0);
}

void esp_write(uint8_t conn_id, const uint8_t *data)
{
	static const __flash char cipsendcmd[] = "AT+CIPSEND=%u,%u\r\n";

	uint8_t cipsend[32];
	uint8_t data_length;
	uint8_t attempt;

	data_length = strlen(data);

	if(!data_length)
		return;

	snprintf_P(cipsend, sizeof(cipsend), cipsendcmd, conn_id, data_length);

	for(attempt = 4; attempt > 0; attempt--)
	{
		if(writeread(128, cipsend, token_prompt))
		{
			if(writeread(128, data, token_sendok))
				break;
		}
	}
}
