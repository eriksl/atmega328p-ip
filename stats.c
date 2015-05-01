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

static const __flash uint8_t format_string[] =
{
	"int uart rx: %u\n"
	"int uart tx: %u\n"
	"int uart fe: %u\n"
	"int uart overrun: %u\n"
	"int wd: %u\n"
	"int adc: %u\n"
	"int t1: %u\n"
	"int t1um: %u\n"
};

void stats_generate(uint16_t size, uint8_t *dst)
{
	snprintf_P(dst, size, format_string,
			uart_rx_interrupts,
			uart_tx_interrupts,
			uart_fe,
			uart_overrun,
			wd_interrupts,
			adc_interrupts,
			t1_interrupts,
			t1_unhandled_max);
}
