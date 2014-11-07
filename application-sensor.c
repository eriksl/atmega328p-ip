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

static uint8_t bmp085_write(uint8_t reg, uint8_t value)
{
	uint8_t twierror;
	uint8_t twistring[2];

	twistring[0] = reg;
	twistring[1] = value;

	if((twierror = twi_master_send(0x77, 2, twistring)) != tme_ok)
		return(twierror);

	return(0);
}

static uint8_t bmp085_read(uint8_t reg, uint16_t *value)
{
	uint8_t twierror;
	uint8_t twistring[2];

	twistring[0] = reg;

	if((twierror = twi_master_send(0x77, 1, twistring)) != tme_ok)
		return(twierror);

	if((twierror = twi_master_receive(0x77, 2, twistring)) != tme_ok)
		return(twierror);

	*value = ((uint16_t)twistring[0] << 8) | (uint16_t)twistring[1];

	return(0);
}

static uint8_t bmp085_read_long(uint8_t reg, uint32_t *value)
{
	uint8_t twierror;
	uint8_t twistring[4];

	twistring[0] = reg;

	if((twierror = twi_master_send(0x77, 1, twistring)) != tme_ok)
		return(twierror);

	if((twierror = twi_master_receive(0x77, 3, twistring)) != tme_ok)
		return(twierror);

	*value = ((uint32_t)twistring[0] << 16) | ((uint32_t)twistring[1] << 8) | (uint32_t)twistring[2];

	return(0);
}

static uint8_t tsl2560_write(uint8_t reg, uint8_t value)
{
	uint8_t twierror;
	uint8_t twistring[2];

	twistring[0] = 0xc0 | reg; // write byte
	twistring[1] = value;

	if((twierror = twi_master_send(0x39, 2, twistring)) != tme_ok)
		return(twierror);

	return(0);
}

static uint8_t tsl2560_read(uint8_t reg, uint8_t *values)
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

	tsl2560_write(0x00, 0x03);	// tsl2560; power up
	tsl2560_write(0x06, 0x00);	// disable interrupts
	tsl2560_write(0x01, 0x11);	// start continuous sampling every 100 ms, high gain = 16x

	twistring[0] = 0x01;		// bh1750; power on
	twi_master_send(0x23, 1, twistring);
	twistring[0] = 0x07;		// reset
	twi_master_send(0x23, 1, twistring);
	twistring[0] = 0x11;		// start continuous sampling every 120 ms, high resolution = 0.42 Lx
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
	static const __flash char format_temp[]		= "%d/%s: temp [%.2f] C, (%ld)\n";
	static const __flash char format_humidity[]	= "%d/%s: humidity [%.0f] %% (%ld)\n";
	static const __flash char format_light[]	= "%d/%s: light [%.2f] Lux (%ld)\n";
	static const __flash char format_pressure[]	= "%d/%s: pressure [%.2f] hPa (%ld)\n";
	static const __flash char twi_error[]		= "%d/%s: error: twi\n";

	const __flash char *format;

	uint16_t	ix;
	uint8_t		admux;
	uint32_t	raw;
	float		raw_value = 0;
	float		value = 0;
	float		factor, offset;
	uint8_t		twistring[4];
	uint8_t		twierror;
	const char	*id;

	switch(sensor)
	{
		case(0): // internal bandgap thermosensor
		{
			id = "bg";

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

			raw_value	= ((float)raw / (float)samples) / 1000.0 * eeprom_read_bandgap();
			value		= (raw_value - 0.2897) * 0.942 * 1000.0; // bg
			raw_value	*= 1000;

			admux = ADMUX;
			admux |= 0x0e; // 0xe = 1.1V
			ADMUX = admux;

			format = format_temp;

			break;
		}

		case(1): // digipicco temperature on twi 0x78
		{
			id = "digipicco";

			if((twierror = twi_master_receive(0x78, 4, twistring)) != tme_ok)
				goto twierror;

			raw_value	= ((uint16_t)twistring[2] << 8) | (uint16_t)twistring[3];
			value		= ((raw_value * 165.0) / 32767) - 40.5;
			format		= format_temp;

			break;
		}

		case(2): // tmp275 or compatible on twi 0x48
		{
			id = "lm75ad";

			twistring[0] = 0x01;	// select config register
			twistring[1] = 0x60;	// write r0=r1=1, other bits zero

			if((twierror = twi_master_send(0x48, 2, twistring)) != tme_ok)
				goto twierror;

			twistring[0] = 0x00; // select temperature register

			if((twierror = twi_master_send(0x48, 1, twistring)) != tme_ok)
				goto twierror;

			if((twierror = twi_master_receive(0x48, 2, twistring)) != tme_ok)
				goto twierror;

			raw_value	= (uint16_t)(((twistring[0] << 8) | twistring[1]) >> 4);
			value		= raw_value * 0.0625;
			format		= format_temp;

			break;
		}

		case(3): // bmp085 temperature
		case(5): // bmp085 pressure
		{
			int16_t		ac1, ac2, ac3;
			uint16_t	ac4, ac5, ac6;
			int16_t		b1, b2;
			int16_t		mc, md;
			uint16_t	ut;
			uint32_t	up;
			int32_t		b3, b4, b5, b6;
			uint32_t	b7;
			int32_t		x1, x2, x3, p;
			uint8_t		oss = 3;

			id = "bmp085";

			if((twierror = bmp085_read(0xaa, (uint16_t *)&ac1)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xac, (uint16_t *)&ac2)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xae, (uint16_t *)&ac3)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xb0, &ac4)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xb2, &ac5)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xb4, &ac6)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xb6, (uint16_t *)&b1)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xb8, (uint16_t *)&b2)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xbc, (uint16_t *)&mc)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_read(0xbe, (uint16_t *)&md)) != tme_ok)
				goto twierror;

			if((twierror = bmp085_write(0xf4, 0x2e)) != tme_ok) // set cmd = 0x2e = start temperature measurement
				goto twierror;

			sleep(5);

			if((twierror = bmp085_read(0xf6, &ut)) != tme_ok) // select result 0xf6+0xf7
				goto twierror;

#if 0
			ac1	= 408;
			ac2	= -72;
			ac3	= -14383;
			ac4	= 32741;
			ac5 = 32757;
			ac6 = 23153;
			b1	= 6190;
			b2	= 4;
			mc	= -8711;
			md	= 2868;

			ut = 27898;
#endif

			x1 = (((uint32_t)ut - (uint32_t)ac6) * (uint32_t)ac5) >> 15;
			x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
			b5 = x1 + x2;

			if(sensor == 4) // temperature
			{
				format		= format_temp;
				raw_value	= ut;
				value		= ((((float)b5 + 8) / 16) / 10); // temperature
			}
			else // pressure
			{
				if((twierror = bmp085_write(0xf4, 0x34 | (oss << 6))) != tme_ok) // set cmd = 0x34 = start air pressure measurement
					goto twierror;

				sleep(20);

				up = 0;

				if((twierror = bmp085_read_long(0xf6, &up)) != tme_ok) // select result 0xf6+0xf7+f8
					goto twierror;
#if 0
			up	= 23843;
#endif

				up = up >> (8 - oss);

				b6	= b5 - 4000;
				x1	= ((int32_t)b2 * ((b6 * b6) >> 12)) >> 11;
				x2	= ((int32_t)ac2 * b6) >> 11;
				x3	= x1 + x2;
				b3	= ((((int32_t)ac1 * 4 + x3) << oss) + 2) / 4;
				x1	= ((int32_t)ac3 * b6) >> 13;
				x2	= ((int32_t)b1 * ((b6 * b6) >> 12)) >> 16;
				x3	= (x1 + x2 + 2) >> 2;
				b4	= ((uint32_t)ac4 * (uint32_t)(x3 + 32768)) >> 15;
				b7	= (uint32_t)(((uint32_t)up - b3) * (50000 >> oss));

				if(b7 & 0x80000000)
					p = (b7 / b4) << 1;
				else
					p = (b7 << 1) / b4;

				x1	= (p >> 8) * (p >> 8);
				x1	= (x1 * 3038UL) >> 16;
				x2	= (p * -7357) >> 16;
				p	= p + ((x1 + x2 + 3791L) >> 4);

				format		= format_pressure;
				raw_value	= up;
				value		= p / 100.0;
			}

			break;
		}

		case(4): // digipicco humidity on twi 0x78
		{
			id = "digipicco";

			if((twierror = twi_master_receive(0x78, sizeof(twistring), twistring)) != tme_ok)
				goto twierror;

			raw_value	= ((uint16_t)twistring[0] << 8)) | (uint16_t)twistring[1];
			value		= (raw_value * 100.0) / 32768.0;
			format		= format_humidity;

			break;
		}

		case(5): // tsl2560 on twi 0x39, high gain, short exposure (100 ms)
		{
			float ch0, ch1;

			id = "tsl2560";

			if((twierror = tsl2560_read(0x0c, twistring)) != tme_ok)
				goto twierror;

			ch0 = (uint16_t)(twistring[0] | (twistring[1] << 8));
			ch1 = (uint16_t)(twistring[2] | (twistring[3] << 8));

			if((ch0 < 37170) && (ch1 < 37170))
			{
				if(ch0 != 0)
				{
					raw_value = ch1 / ch0;

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

					value /= 0.252; // integration time = 100 ms, scale = 0.252
				}
				else
					value = 0;
			}
			else
				value = -1;

			raw_value	= (10000 * ch0) + ch1;
			format		= format_light;

			break;
		}

		case(6): // bh1750 on twi 0x23, long exposure, high resolution
		{
			id = "bh1750";

			if((twierror = twi_master_receive(0x23, 2, twistring)) != tme_ok)
				goto twierror;

			raw_value	= ((uint16_t)twistring[0] << 8) | (uint16_t)twistring[1];
			value		= raw_value * 0.42;
			format		= format_light;

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

	snprintf_P((char *)dst, size, format, sensor, id, value, (int32_t)raw_value);
	return(1);

twierror:
	snprintf_P((char *)dst, size, twi_error, sensor, id);
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
