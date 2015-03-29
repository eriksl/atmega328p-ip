#include "util.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdint.h>

void reset(void)
{
	for(;;)
		(void)0;
}

void msleep(uint16_t ms)
{
	while(ms-- > 0)
		_delay_ms(1);
}

void pause_idle(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}

uint8_t crc8(uint8_t crc_mode, uint8_t length, const uint8_t *block)
{
	uint8_t outer, inner, crc = 0;

	for(outer = 0; outer < length; outer++)
	{
		crc = crc ^ block[outer];

		for(inner = 0; inner < 8; inner++)
		{
			if((crc & 0x80) != 0)
			{
				crc <<= 1;
				crc ^= crc_mode;
			}
			else
				crc <<= 1;
		}
	}

	return(crc);
}
