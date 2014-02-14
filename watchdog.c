#include <stdint.h>
#include <avr/io.h>

#include "watchdog.h"

static void watchdog_set(uint8_t value)
{
	__builtin_avr_wdr();
	WDTCSR |= _BV(WDIE);
	MCUSR &= ~_BV(WDRF);
	WDTCSR |= _BV(WDCE) | _BV(WDE);
	WDTCSR = value;
}

void watchdog_start(uint8_t scaler)
{
	uint8_t wdp[4];

	wdp[0] = (scaler & (1 << 0)) >> 0;
	wdp[1] = (scaler & (1 << 1)) >> 1;
	wdp[2] = (scaler & (1 << 2)) >> 2;
	wdp[3] = (scaler & (1 << 3)) >> 3;

	watchdog_set(
		WDTCSR =
			(1		<< WDIF)	|	//	clear pending interrupt
			(1		<< WDIE)	|	//	enable interrupt
			(wdp[3]	<< WDP3)	|
			(0		<< WDCE)	|	//	!enable change
			(1		<< WDE)		|	//	enable
			(wdp[2] << WDP2)	|
			(wdp[1] << WDP1)	|
			(wdp[0] << WDP0));
}

void watchdog_stop(void)
{
	watchdog_set(0x00);
}
