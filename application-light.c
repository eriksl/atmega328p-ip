#include "application-light.h"
#include "eeprom.h"
#include "twi_master.h"
#include "util.h"

#include <avr/pgmspace.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

enum
{
	read_ok = 0,
	read_io_error = 1,
	read_overflow = 2,
};

static uint8_t adc2count(uint8_t in, uint16_t *out)
{
	uint8_t	valid	= !!(in & 0x80);
	uint8_t	chord	= (in & 0x70) >> 4;
	uint8_t	step	= (in & 0x0f);

	if(!valid)
		return(read_io_error);

	if((in & 0x7f) == 0x7f)
		return(read_overflow);

	uint16_t	chordval	= (33 * ((1 << chord) - 1)) / 2;
	uint8_t		stepval		= step * (1 << chord);

	*out = chordval + stepval;

	return(read_ok);
}

static float count2lux(uint16_t ch0, uint16_t ch1, uint8_t multiplier)
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

static uint8_t twi_write_read(uint8_t *byte)
{
	uint8_t twierror;

	if((twierror = twi_master_send(0x39, 1, byte)) != tme_ok)
		return(0);

	if((twierror = twi_master_receive(0x39, 1, byte)) != tme_ok)
		return(0);

	return(1);
}

static uint8_t read_sensor(float *result, uint8_t extended_range)
{
	uint8_t		byte;
	uint8_t		rv;
	uint8_t		ch0, ch1;
	uint16_t	cch0, cch1;

	byte = 0x03;	// power up (if not already running)

	if(!twi_write_read(&byte))
		return(read_io_error);

	if(byte != 0x03)
		return(read_io_error);

	if(extended_range)
		byte = 0x1d;	// extended range
	else
		byte = 0x18;	// standard range

	if(!twi_write_read(&byte))
		return(read_io_error);

	if(byte != 0x1b)
		return(read_io_error);

	byte = 0x43;	// read from channel 0

	if(!twi_write_read(&byte))
		return(read_io_error);

	ch0 = byte;

	byte = 0x83;	// read from channel 1

	if(!twi_write_read(&byte))
		return(read_io_error);

	ch1 = byte;

	if((rv = adc2count(ch0, &cch0)) != read_ok)
		return(rv);

	if((rv = adc2count(ch1, &cch1)) != read_ok)
		return(rv);

	if((*result = count2lux(cch0, cch1, extended_range ? 5 : 1)) < 0)
		return(read_io_error);

	return(read_ok);
}

void application_init_light(void)
{
	uint8_t byte = 0x03; // power up

	twi_write_read(&byte);
}

uint8_t application_function_light_read(application_parameters_t ap)
{
	static const __flash char ok[]				= "> light sensor %d ok light [%.3f] Lux\n";
	static const __flash char error_bounds[]	= "> invalid sensor\n";
	static const __flash char twi_error[]		= "> twi error\n";
	static const __flash char overflow[]		= "> sensor overflow\n";

	uint8_t	sensor;
	float	light = 0;

	sensor = atoi((const char *)(*ap.args)[1]);

	switch(sensor)
	{
		case(0):	// tsl2550 standard range
		case(1):	// tsl2550 extended range
		{
			switch(read_sensor(&light, (sensor == 1)))
			{
				case(read_io_error):
				{
					strcpy_P((char *)ap.dst, twi_error);
					return(1);
				}

				case(read_overflow):
				{
					strcpy_P((char *)ap.dst, overflow);
					return(1);
				}
			}

			break;
		}

		default:
		{
			strlcpy_P((char *)ap.dst, error_bounds, (size_t)ap.size);
			return(1);
		}
	}

	light *= eeprom_read_light_cal_factor(sensor);
	light += eeprom_read_light_cal_offset(sensor);

	snprintf_P((char *)ap.dst, ap.size, ok, sensor, light);

	return(1);
}

uint8_t application_function_light_write(application_parameters_t ap)
{
	static const __flash char ok[] = "> light sensor calibration set to *=%.4f +=%.4f\n";

	uint8_t index;
	float factor, offset;

	index	= atoi((const char *)(*ap.args)[1]);
	factor	= atof((const char *)(*ap.args)[2]);
	offset	= atof((const char *)(*ap.args)[3]);

	eeprom_write_light_cal_factor(index, factor);
	eeprom_write_light_cal_offset(index, offset);

	factor = eeprom_read_light_cal_factor(index);
	offset = eeprom_read_light_cal_offset(index);

	snprintf_P((char *)ap.dst, (size_t)ap.size, ok, factor, offset);

	return(1);
}
