#include "application-temperature.h"
#include "util.h"
#include "eeprom.h"
#include "stats.h"
#include "twi_master.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum
{
	samples = 256
};

ISR(ADC_vect)
{
	adc_interrupts++;
}

void application_init_temp(void)
{
	PRR			&= ~_BV(PRADC);
	DIDR0		= _BV(ADC0D) | _BV(ADC1D);
	ADMUX		= _BV(REFS1) | _BV(REFS0) | 0x0e;	// ref = 1.1V, input = 1.1V
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

uint8_t application_function_bg_write(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> bandgap calibration set to %.4f V\n";

	float value;

	value = atof((const char *)args[1]);

	eeprom_write_bandgap(value);
	value = eeprom_read_bandgap();

	snprintf_P((char *)dst, (size_t)size, ok, value);

	return(1);
}

uint8_t application_function_temp_read(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> temp sensor %d ok temp [%.2f] C, %.5f V\n";
	static const __flash char error[] = "> invalid sensor\n";

	uint8_t		sensor;
	uint16_t	ix;
	uint8_t		admux;
	uint32_t	raw;
	float		raw_v = 0;
	float		temp = 0;

	sensor = (uint8_t)strtoul((const char *)args[1], 0, 0);

	switch(sensor)
	{
		case(0): // internal bandgap thermosensor
		case(1): // TMP36
		{
			admux = ADMUX;
			admux &= 0xf0;

			if(sensor == 0)
				admux |= 0x08; // 0x8 = ADC8 = bg
			else
				admux |= 0x00; // 0x0 = ADC0 = TMP36

			ADMUX = admux;

			for(ix = 32; ix > 0; ix--)
				get_adc();

			raw = 0;

			for(ix = samples; ix > 0; ix--)
			{
				raw += get_adc();
				application_periodic();
			}

			raw_v = ((float)raw / (float)samples) / 1000 * eeprom_read_bandgap();

			if(sensor == 0)
				temp = (raw_v - 0.2897) * 0.942 * 1000; // bg
			else
				temp = (raw_v - 0.5) * 100;	// TMP36

			snprintf_P((char *)dst, size, ok, sensor, temp, raw_v);

			admux = ADMUX;
			admux |= 0x0e; // 0xe = 1.1V
			ADMUX = admux;

			break;
		}

		case(2): // ds7505 on twi 0x48 / 0x49
		case(3): // tmp275 on twi 0x48 / 0x49
		{
			uint8_t address;
			uint8_t twierror;
			uint8_t twistring[2];

			if(sensor == 2)
				address = 0x48;
			else
				address = 0x49;

			twistring[0] = 0x01;	// select config register
			twistring[1] = 0x60;	// write r0=r1=1, other bits zero

			if((twierror = twi_master_send(address, 2, twistring)) != tme_ok)
			{
				twi_master_error(dst, size, twierror);
				break;
			}

			twistring[0] = 0x00;	// select temperature register

			if((twierror = twi_master_send(address, 1, twistring)) != tme_ok)
			{
				twi_master_error(dst, size, twierror);
				break;
			}

			if((twierror = twi_master_receive(address, 2, twistring)) != tme_ok)
			{
				twi_master_error(dst, size, twierror);
				break;
			}

			temp = ((int16_t)(((twistring[0] << 8) | twistring[1]) >> 4)) * 0.0625;

			break;
		}

		default:
		{
			goto error;
			break;
		}
	}

	temp *= eeprom_read_temp_cal_factor(sensor);
	temp += eeprom_read_temp_cal_offset(sensor);

	snprintf_P((char *)dst, size, ok, sensor, temp, raw_v);

	return(1);

error:
	strlcpy_P((char *)dst, error, (size_t)size);
	return(1);
}

uint8_t application_function_temp_write(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[] = "> temperature calibration set to *=%.4f +=%.4f\n";

	uint8_t index;
	float factor, offset;

	index	= atoi((const char *)args[1]);
	factor	= atof((const char *)args[2]);
	offset	= atof((const char *)args[3]);

	eeprom_write_temp_cal_factor(index, factor);
	eeprom_write_temp_cal_offset(index, offset);

	factor = eeprom_read_temp_cal_factor(index);
	offset = eeprom_read_temp_cal_offset(index);

	snprintf_P((char *)dst, (size_t)size, ok, factor, offset);

	return(1);
}
