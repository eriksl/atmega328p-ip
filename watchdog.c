#include <avr/io.h>
#include <avr/wdt.h>

#include "watchdog.h"

#include <stdint.h>

void inline watchdog_stop(void)
{
	wdt_reset();
	MCUSR = 0;
	wdt_disable();
}

void inline watchdog_start(uint8_t scaler)
{
	watchdog_stop();
	wdt_enable(scaler);
	watchdog_rearm();
}
