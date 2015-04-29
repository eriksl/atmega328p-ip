#include "stats.h"
#include "util.h"

#include <stdio.h>

stats_t uart_rx_interrupts = 0;
stats_t uart_tx_interrupts = 0;
stats_t uart_fe = 0;
stats_t uart_overrun = 0;
stats_t wd_interrupts = 0;
stats_t adc_interrupts = 0;
stats_t t1_interrupts = 0;
stats_t t1_unhandled = 0;
stats_t t1_unhandled_max = 0;
stats_t esp_wd_timeout = 0;

static const __flash char format_string[] =
{
	"int uart rx: %u\n"
	"int uart tx: %u\n"
	"int uart fe: %u\n"
	"int uart overrun: %u\n"
	"int wd: %u\n"
	"int adc: %u\n"
	"int t1: %u\n"
	"int t1um: %u\n"
	"esp wd: %u\n"
};

void stats_generate(uint16_t size, uint8_t *dst)
{
	snprintf_P((char *)dst, (size_t)size, (const char *)format_string,
			uart_rx_interrupts,
			uart_tx_interrupts,
			uart_fe,
			uart_overrun,
			wd_interrupts,
			adc_interrupts,
			t1_interrupts,
			t1_unhandled_max,
			esp_wd_timeout);
}
