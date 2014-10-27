#include "application-sensor.h"
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
#include <math.h>

enum
{
	samples = 256
};

ISR(ADC_vect)
{
	adc_interrupts++;
}

enum
{
	tsl_read_ok = 0,
	tsl_read_io_error = 1,
	tsl_read_overflow = 2,
};

static uint8_t tsl_adc2count(uint8_t in, uint16_t *out)
{
	uint8_t	valid	= !!(in & 0x80);
	uint8_t	chord	= (in & 0x70) >> 4;
	uint8_t	step	= (in & 0x0f);

	if(!valid)
		return(tsl_read_io_error);

	if((in & 0x7f) == 0x7f)
		return(tsl_read_overflow);

	uint16_t	chordval	= (33 * ((1 << chord) - 1)) / 2;
	uint8_t		stepval		= step * (1 << chord);

	*out = chordval + stepval;

	return(tsl_read_ok);
}

static float tsl_count2lux(uint16_t ch0, uint16_t ch1, uint8_t multiplier)
{
	float r, e, l;

	if(ch0 == ch1)
		r = 0;
	else
		r = (float)ch1 / ((float)ch0 - (float)ch1);

	e = exp(-0.181 * r * r);
	l = ((float)ch0 - (float)ch1) * 0.39 * e * (float)multiplier;

	if(l > 100)
		l = round(l);
	else if(l > 10)
		l = round(l * 10) / 10;
	else
		l = round(l * 100) / 100;

	return(l);
}

static uint8_t tsl_write_read(uint8_t *byte)
{
	uint8_t twierror;

	if((twierror = twi_master_send(0x39, 1, byte)) != tme_ok)
		return(0);

	if((twierror = twi_master_receive(0x39, 1, byte)) != tme_ok)
		return(0);

	return(1);
}

static uint8_t tsl_read_sensor(float *result, uint8_t extended_range)
{
	uint8_t		byte;
	uint8_t		rv;
	uint8_t		ch0, ch1;
	uint16_t	cch0, cch1;

	byte = 0x03;	// power up (if not already running)

	if(!tsl_write_read(&byte))
		return(tsl_read_io_error);

	if(byte != 0x03)
		return(tsl_read_io_error);

	if(extended_range)
		byte = 0x1d;	// extended range
	else
		byte = 0x18;	// standard range

	if(!tsl_write_read(&byte))
		return(tsl_read_io_error);

	if(byte != 0x1b)
		return(tsl_read_io_error);

	byte = 0x43;	// read from channel 0

	if(!tsl_write_read(&byte))
		return(tsl_read_io_error);

	ch0 = byte;

	byte = 0x83;	// read from channel 1

	if(!tsl_write_read(&byte))
		return(tsl_read_io_error);

	ch1 = byte;

	if((rv = tsl_adc2count(ch0, &cch0)) != tsl_read_ok)
		return(rv);

	if((rv = tsl_adc2count(ch1, &cch1)) != tsl_read_ok)
		return(rv);

	if((*result = tsl_count2lux(cch0, cch1, extended_range ? 5 : 1)) < 0)
		return(tsl_read_io_error);

	return(tsl_read_ok);
}

void application_init_sensor(void)
{
	PRR			&= ~_BV(PRADC);
	DIDR0		= _BV(ADC0D) | _BV(ADC1D);
	ADMUX		= _BV(REFS1) | _BV(REFS0) | 0x0e;	// ref = 1.1V, input = 1.1V
	ADCSRA		= _BV(ADEN)  | _BV(ADIF)  |
				  _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	// 8 Mhz / 128 = 75 kHz > 50 < 200 kHz
	ADCSRB		=	0x00;

	uint8_t byte = 0x03; // power up tsl2550
	tsl_write_read(&byte);
}

static uint16_t get_adc(void)
{
	ADCSRA |= _BV(ADSC) | _BV(ADIF) | _BV(ADIE);

	while(ADCSRA & _BV(ADSC))
		pause_idle();

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

uint8_t application_sensor_read(uint8_t sensor, uint16_t size, uint8_t *dst)
{
	static const __flash char format_temp[]		= "> sensor %d ok temp [%.2f] C, %.5f V\n";
	static const __flash char format_humidity[]	= "> sensor %d ok humidity [%.0f] %%\n";
	static const __flash char format_light[]	= "> sensor %d ok light [%.3f] Lux\n";
	static const __flash char twi_error[]		= "> sensor %d twi error\n";
	static const __flash char overflow[]		= "> sensor %d overflow\n";

	const __flash char *format;

	uint16_t	ix;
	uint8_t		admux;
	uint32_t	raw;
	float		raw_value = 0;
	float		value = 0;
	float		factor, offset;
	uint8_t		twistring[4];
	uint8_t		twierror;
	uint8_t		address;

	switch(sensor)
	{
		case(0): // internal bandgap thermosensor
		{
			admux = ADMUX;
			admux &= 0xf0;
			admux |= 0x08; // 0x8 = ADC8 = bg

			ADMUX = admux;

			for(ix = 32; ix > 0; ix--)
				get_adc();

			raw = 0;

			for(ix = samples; ix > 0; ix--)
			{
				raw += get_adc();
				application_periodic();
			}

			raw_value	= ((float)raw / (float)samples) / 1000 * eeprom_read_bandgap();
			value		= (raw_value - 0.2897) * 0.942 * 1000; // bg

			admux = ADMUX;
			admux |= 0x0e; // 0xe = 1.1V
			ADMUX = admux;

			format = format_temp;

			break;
		}

		case(1): // digipicco temperature on twi 0x78
		{
			if((twierror = twi_master_receive(0x78, 4, twistring)) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}

			value = (float)((twistring[2] << 8) | twistring[3]);
			value = (value * 165.0) / 32767;
			value -= 40.5;

			format = format_temp;

			break;
		}

		case(2): // tmp275 or compatible on twi 0x48
		case(3): // tmp275 or compatible on twi 0x49
		{
			if(sensor == 2)
				address = 0x48;
			else
				address = 0x49;

			twistring[0] = 0x01;	// select config register
			twistring[1] = 0x60;	// write r0=r1=1, other bits zero

			if((twierror = twi_master_send(address, 2, twistring)) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}

			twistring[0] = 0x00; // select temperature register

			if((twierror = twi_master_send(address, 1, twistring)) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}

			if((twierror = twi_master_receive(address, 2, twistring)) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}

			value = ((int16_t)(((twistring[0] << 8) | twistring[1]) >> 4)) * 0.0625;

			format = format_temp;

			break;
		}

		case(4): // digipicco humidity on twi 0x78
		{
			if((twierror = twi_master_receive(0x78, sizeof(twistring), twistring)) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}

			value = (float)((twistring[0] << 8) | twistring[1]);
			value = (value * 100) / 32768;

			format = format_humidity;

			break;
		}

		case(5): // tsl2550 standard range on twi
		case(6): // tsl2550 extended range on twi
		{
			switch(tsl_read_sensor(&value, (sensor == 6)))
			{
				case(tsl_read_io_error):
				{
					snprintf_P((char *)dst, size, twi_error, sensor);
					return(1);
				}

				case(tsl_read_overflow):
				{
					snprintf_P((char *)dst, size, overflow, sensor);
					return(1);
				}
			}

			format = format_light;

			break;
		}

		default:
		{
			return(0);
		}
	}

	if(eeprom_read_cal(sensor, &factor, &offset))
	{
		value *= factor;
		value += offset;
	}

	snprintf_P((char *)dst, size, format, sensor, value, raw_value);

	return(1);
}

uint8_t application_function_sensor_read(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char error[] = "> invalid sensor: %s\n";

	uint8_t sensor;

	sensor = (uint8_t)strtoul((const char *)args[1], 0, 0);

	if(!application_sensor_read(sensor, size, dst))
		snprintf_P((char *)dst, size, error, args[1]);

	return(1);
}

uint8_t application_function_sensor_write(uint8_t nargs, uint8_t args[application_num_args][application_length_args], uint16_t size, uint8_t *dst)
{
	static const __flash char ok[]		= "> sensor %d calibration set to *=%.4f +=%.4f\n";
	static const __flash char error[]	= "> no sensor %d\n";

	uint8_t sensor;
	float factor, offset;

	sensor	= atoi((const char *)args[1]);
	factor	= atof((const char *)args[2]);
	offset	= atof((const char *)args[3]);

	if(!eeprom_write_cal(sensor, factor, offset))
	{
		snprintf_P((char *)dst, (size_t)size, error, sensor);
		return(1);
	}

	if(!eeprom_read_cal(sensor, &factor, &offset))
	{
		snprintf_P((char *)dst, (size_t)size, error, sensor);
		return(1);
	}

	snprintf_P((char *)dst, (size_t)size, ok, sensor, factor, offset);

	return(1);
}
