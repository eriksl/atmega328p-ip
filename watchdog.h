#ifndef _watchdog_h_
#define _watchdog_h_

#include <avr/wdt.h>

#include <stdint.h>

enum
{
	WATCHDOG_PRESCALER_16		=	WDTO_15MS,
	WATCHDOG_PRESCALER_32		=	WDTO_30MS,
	WATCHDOG_PRESCALER_64		=	WDTO_60MS,
	WATCHDOG_PRESCALER_128		=	WDTO_120MS,
	WATCHDOG_PRESCALER_256		=	WDTO_250MS,
	WATCHDOG_PRESCALER_512		=	WDTO_500MS,
	WATCHDOG_PRESCALER_1024		=	WDTO_1S,
	WATCHDOG_PRESCALER_2048		=	WDTO_2S,
	WATCHDOG_PRESCALER_4096		=	WDTO_4S,
	WATCHDOG_PRESCALER_8192		=	WDTO_8S,
};

void watchdog_reset(void);
void watchdog_stop(void);
void watchdog_start(uint8_t scaler);

#endif
