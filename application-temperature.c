#include "application-temperature.h"
#include "util.h"
#include "eeprom.h"
#include "stats.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum
{
	samples = 1024,
};

const __flash char description_temp_read[] = "read temp sensor";
const __flash char description_temp_write[] = "write bg cal. adc";

ISR(ADC_vect)
{
	adc_interrupts++;
}

void application_init_temp_read(void)
{
	PRR			&= ~_BV(PRADC);
	DIDR0		= _BV(ADC0D) | _BV(ADC1D);
	ADMUX		= _BV(REFS1) | _BV(REFS0) | 0x0f;	// ref = 1.1V, input = 0 V
	ADCSRA		= _BV(ADEN)  | _BV(ADIF)  |
				  _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	// 8 Mhz / 128 = 75 kHz > 50 < 200 kHz
	ADCSRB		=	0x00;
}

static uint16_t get_adc(void)
{
	ADCSRA |= _BV(ADSC) | _BV(ADIF) | _BV(ADIE);

	while(ADCSRA & _BV(ADSC))
		pause_adc();

	ADCSRA &= ~_BV(ADIE);
	ADCSRA |=  _BV(ADIF);

	return(ADC);
}

uint8_t application_function_temp_write(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> bandgap calibration set to %d mV\n";

	uint16_t new_bg_mv;

	new_bg_mv = (uint16_t)strtoul((const char *)args[1], 0, 0);

	eeprom_write_bandgap_mv(new_bg_mv);
	new_bg_mv = eeprom_read_bandgap_mv();

	snprintf_P((char *)dst, (size_t)size, ok, new_bg_mv);

	return(1);
}

uint8_t application_function_temp_read(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	// ADC:		0 V = 0 pcm, 1.1 V = 1023 pcm
	// 			mV = pcm / 1023 * 1100
	// tmp36:	250 mV = -25.0 C, 500 mV = 0.0, 750 mV = 25.0 C
	// 			C = (mV - 500) / 10

	static const __flash char ok[] = "> temp sensor %d ok temp %.2f C, raw: %04lx, %.5f V\n";
	static const __flash char error[] = "> invalid sensor\n";

	uint8_t sensor;
	uint16_t ix;
	uint8_t admux;
	uint16_t bandgap_mv;
	uint32_t raw;
	float raw_v;
	float temp;

	bandgap_mv = eeprom_read_bandgap_mv();

	sensor = (uint8_t)strtoul((const char *)args[1], 0, 0);

	switch(sensor)
	{
		case(0):
		{
			admux = ADMUX;
			admux &= 0xf0;
			admux |= 0x00; // 0x00 = ADC0
			ADMUX = admux;

			for(ix = 32; ix > 0; ix--)
				get_adc();

			raw = 0;

			for(ix = samples; ix > 0; ix--)
				raw += get_adc();

			raw_v	= ((float)raw / (float)samples) / 1000 * (float)bandgap_mv / 1000;
			temp	= (raw_v - 0.5) * 100;

			snprintf_P((char *)dst, size, ok, sensor, temp, raw / samples, raw_v);

			admux = ADMUX;
			admux |= 0x0f; // 0x0f = GND
			ADMUX = admux;

			break;
		}

		default:
		{
			strlcpy_P((char *)dst, error, (size_t)size);
			break;
		}
	}

	return(1);
}