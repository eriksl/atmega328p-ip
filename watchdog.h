#if !defined(_WATCHDOG_H_)
#define _WATCHDOG_H_ 1

#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>

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

static void inline watchdog_reset(void)
{
	WDTCSR |= _BV(WDIE);
}

static void inline watchdog_stop(void)
{
	wdt_reset();
	MCUSR = 0;
	wdt_disable();
}

static void inline watchdog_start(uint8_t scaler)
{
	watchdog_stop();
	wdt_enable(scaler);
	watchdog_reset();
}

#endif
