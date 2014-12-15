#ifndef _stats_h_
#define _stats_h_

#include <stdint.h>

typedef uint16_t stats_t;

extern stats_t uart_rx_interrupts;
extern stats_t uart_tx_interrupts;
extern stats_t wd_interrupts;
extern stats_t adc_interrupts;
extern stats_t t1_interrupts;
extern stats_t t1_unhandled;
extern stats_t t1_unhandled_max;

void stats_generate(uint16_t size, uint8_t *dst);

#endif
