#if !defined(_WATCHDOG_H_)
#define _WATCHDOG_H_ 1

#include <stdint.h>
#include <avr/io.h>

enum
{
	WATCHDOG_PRESCALER_16		=	0,
	WATCHDOG_PRESCALER_32		=	1,
	WATCHDOG_PRESCALER_64		=	2,
	WATCHDOG_PRESCALER_128		=	3,
	WATCHDOG_PRESCALER_256		=	4,
	WATCHDOG_PRESCALER_512		=	5,
	WATCHDOG_PRESCALER_1024		=	6,
	WATCHDOG_PRESCALER_2048		=	7,
	WATCHDOG_PRESCALER_4096		=	8,
	WATCHDOG_PRESCALER_8192		=	9
};

void watchdog_start(uint8_t scaler);
void watchdog_stop(void);

static void inline watchdog_reset(void)
{
	WDTCSR |= _BV(WDIE);
}

#endif
