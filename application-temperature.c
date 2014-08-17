#include "application-temperature.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const __flash char description_temp_read[] = "read temp sensor";

static uint16_t get_adc(void)
{
	uint16_t rv;

	ADCSRA |= _BV(ADSC);

	while(ADCSRA & _BV(ADSC))
		(void)0;

	rv  = ADCL;
	rv |= ADCH << 8;

	return(rv);
}

uint8_t application_function_temp_read(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	// ADC:		0 V = 0 pcm, 1.1 V = 1023 pcm
	// 			mV = pcm / 1023 * 1100
	// tmp36:	250 mV = -25.0 C, 500 mV = 0.0, 750 mV = 25.0 C
	// 			C = (mV - 500) / 10

	static const __flash char ok[] = "> temp sensor %d ok value %.1f\n";
	static const __flash char error[] = "> invalid sensor\n";
	uint8_t sensor;
	uint8_t ix;
	uint32_t total;
	float calibrated;

	sensor = (uint8_t)strtoul((const char *)args[1], 0, 0);

	switch(sensor)
	{
		case(0):
		{
			PRR &= ~_BV(PRADC);
			ADCSRB = 0x00;
			break;
		}
	}

	switch(sensor)
	{
		case(0):
		{
			ADMUX =		_BV(REFS1) | _BV(REFS0);	// ref = 1.1V, input = ADC0
			ADCSRA =	_BV(ADEN)  | _BV(ADIF);
						_BV(ADPS2) | _BV(ADPS1);	// 8 Mhz / 64 = 125 kHz > 50 < 200 kHz
			ADCSRB =	0x00;

			//calibrated = ((float)get_adc() * 1080.0) / 1024.0;
			//calibrated = (calibrated - 500) / 10;

			for(ix = 8; ix > 0; ix--)
				get_adc();

			total = 0;

			for(ix = 255; ix > 0; ix--)
				total += get_adc();

			calibrated = total / 256;

			snprintf_P((char *)dst, size, ok, sensor, calibrated);
			break;
		}

		default:
		{
			strlcpy_P((char *)dst, error, (size_t)size);
			break;
		}
	}

	switch(sensor)
	{
		case(0):
		{
			ADMUX	 = 0x00;
			ADCSRA	 = 0x00;
			ADCSRB	 = 0x00;
			PRR		|= _BV(PRADC);

			break;
		}
	}

	return(1);
}
