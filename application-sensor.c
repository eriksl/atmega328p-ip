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

uint8_t tsl2560_write(uint8_t reg, uint8_t value)
{
	uint8_t twierror;
	uint8_t twistring[2];

	twistring[0] = 0xc0 | reg; // write byte
	twistring[1] = value;

	if((twierror = twi_master_send(0x39, 2, twistring)) != tme_ok)
		return(twierror);

	return(0);
}

uint8_t tsl2560_read(uint8_t reg, uint8_t *value)
{
	uint8_t twierror;
	uint8_t twistring[1];

	twistring[0] = 0xc0 | reg; // read byte

	if((twierror = twi_master_send(0x39, 1, twistring)) != tme_ok)
		return(twierror);

	if((twierror = twi_master_receive(0x39 , 1, value)) != tme_ok)
		return(twierror);

	return(0);
}

uint8_t tsl2560_read_quad(uint8_t reg, uint8_t *values)
{
	uint8_t twierror;
	uint8_t twistring[1];

	twistring[0] = 0xd0 | reg; // read block

	if((twierror = twi_master_send(0x39, 1, twistring)) != tme_ok)
		return(twierror);

	if((twierror = twi_master_receive(0x39 , 4, values)) != tme_ok)
		return(twierror);

	return(0);
}

void application_init_sensor(void)
{
	uint8_t twistring[1];

	PRR			&= ~_BV(PRADC);
	DIDR0		= _BV(ADC0D) | _BV(ADC1D);
	ADMUX		= _BV(REFS1) | _BV(REFS0) | 0x0e;	// ref = 1.1V, input = 1.1V
	ADCSRA		= _BV(ADEN)  | _BV(ADIF)  |
				  _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	// 11 Mhz / 128 = 85 kHz = > 50 < 200 kHz
	ADCSRB		=	0x00;

	tsl2560_write(0x00, 0x03);	// power up tsl2560
	tsl2560_write(0x01, 0x01);	// set timing to 100 ms = 0b01
	tsl2560_write(0x06, 0x00);	// disable interrupts

	twistring[0] = 0x01;		// bh1750; power on
	twi_master_send(0x23, 1, twistring);
	twistring[0] = 0x07;		// reset
	twi_master_send(0x23, 1, twistring);
	twistring[0] = 0x11;		// start continuous sampling at 0.5 Lx
	twi_master_send(0x23, 1, twistring);
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

		case(5): // tsl2560 on twi 0x39, low gain
		case(6): // tsl2560 on twi 0x39, high gain
		{
			float ch0, ch1;

			if(sensor == 5)
				twistring[0] = 0b00000001;	// low gain 1x, integration time = 100 ms, scale = 0.252
			else
				twistring[0] = 0b00010001;	// high gain 16, integration time = 100 ms, scale = 0.252 * 16

			if((twierror = tsl2560_write(0x01, twistring[0])) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}

			if((twierror = tsl2560_read_quad(0x0c, twistring)) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}


			ch0 = twistring[0] | (twistring[1] << 8);
			ch1 = twistring[2] | (twistring[3] << 8);

			if(ch0 != 0)
				raw_value = ch1 / ch0;
			else
				raw_value = 0;

			if(raw_value <= 0.50)
				value = (0.0304 * ch0) - (0.062 * ch0 * powf(raw_value, 1.4));
			else if(raw_value <= 0.61)
				value = (0.0224 * ch0) - (0.031 * ch1);
			else if(raw_value <= 0.80)
				value = (0.0128 * ch0) - (0.0153 * ch1);
			else if(raw_value <= 1.30)
				value = (0.00146 * ch0) - (0.00112 * ch1);
			else
				value = 0;

			if(isnan(value))
			{
				value = 10000;
			}
			else
			{
				if(sensor == 6)
					value /= 16;

				value /= 0.252; // integration time = 100 ms, scale = 0.252
			}

			format = format_light;

			break;
		}

		case(7): // bh1750 on twi 0x23, high gain
		{
			if((twierror = twi_master_receive(0x23, 2, twistring)) != tme_ok)
			{
				snprintf_P((char *)dst, size, twi_error, sensor);
				return(1);
			}

			value = (float)(uint16_t)((twistring[0] << 8) | twistring[1]);
			value = value * 0.42;

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
