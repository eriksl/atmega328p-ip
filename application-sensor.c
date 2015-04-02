#include "application-sensor.h"
#include "sensor.h"
#include "eeprom.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void application_init_sensor(void)
{
	sensor_init_bandgap();
	sensor_init_tsl2560();
	sensor_init_bh1750();
}

uint8_t application_function_bg_write(application_parameters_t ap)
{
	static const __flash char ok[] = "> bandgap calibration set to %.4f V\n";

	float value;

	value = atof((const char *)(*ap.args)[1]);

	eeprom_write_bandgap(value);
	value = eeprom_read_bandgap();

	snprintf_P((char *)ap.dst, (size_t)ap.size, ok, value);

	return(1);
}

uint8_t application_sensor_read(uint8_t sensor, uint16_t size, uint8_t *dst)
{
	static const __flash char format_temperature[]	= "%d/%s: temp [%.2f] C, (%ld)\n";
	static const __flash char format_humidity[]		= "%d/%s: humidity [%.0f] %% (%ld)\n";
	static const __flash char format_light[]		= "%d/%s: light [%.2f] Lux (%ld)\n";
	static const __flash char format_airpressure[]	= "%d/%s: pressure [%.2f] hPa (%ld)\n";
	static const __flash char twi_error[]			= "%d/%s: error: twi\n";

	const __flash char *format;

	float		value = 0, raw_value = 0;
	uint8_t		twierror;
	const char	*id;

	switch(sensor)
	{
		case(sensor_bandgap):
		{
			id = "bg";
			format = format_temperature;

			sensor_read_bandgap(&value, &raw_value);

			break;
		}

		case(sensor_digipicco_temperature):
		{
			float hum, hum_raw;

			id = "digipicco";
			format = format_temperature;

			if((twierror = sensor_read_digipicco(&value, &raw_value, &hum, &hum_raw)))
				goto twierror;

			break;
		}

		case(sensor_digipicco_humidity):
		{
			float temp, temp_raw;

			id = "digipicco";
			format = format_humidity;

			if((twierror = sensor_read_digipicco(&temp, &temp_raw, &value, &raw_value)))
				goto twierror;

			break;
		}

		case(sensor_lm75):
		{
			id = "lm75";
			format = format_temperature;

			if((twierror = sensor_read_lm75(&value, &raw_value)))
				goto twierror;

			break;
		}

		case(sensor_bmp085_temperature):
		{
			float pressure, pressure_raw;

			id = "bmp085";
			format = format_temperature;

			if((twierror = sensor_read_bmp085(&value, &raw_value, &pressure, &pressure_raw)))
				goto twierror;

			break;
		}

		case(sensor_bmp085_airpressure):
		{
			float temp, temp_raw;

			id = "bmp085";
			format = format_airpressure;

			if((twierror = sensor_read_bmp085(&temp, &temp_raw, &value, &raw_value)))
				goto twierror;

			break;
		}

		case(sensor_tsl2560):
		{
			id = "tsl2560";
			format = format_light;

			if((twierror = sensor_read_tsl2560(&value, &raw_value)))
				goto twierror;

			break;
		}

		case(sensor_bh1750):
		{
			id = "bh1750";
			format = format_light;

			if((twierror = sensor_read_bh1750(&value, &raw_value)))
				goto twierror;

			break;
		}

		case(sensor_htu21_temperature):
		{
			id = "htu21";
			format = format_temperature;

			if((twierror = sensor_read_htu21_temp(&value, &raw_value)))
				goto twierror;

			break;
		}

		case(sensor_htu21_humidity):
		{
			id = "htu21";
			format = format_humidity;

			if((twierror = sensor_read_htu21_hum(&value, &raw_value)))
				goto twierror;

			break;
		}

		default:
		{
			return(0);
		}
	}

	snprintf_P((char *)dst, size, format, sensor, id, value, (int32_t)raw_value);
	return(1);

twierror:
	snprintf_P((char *)dst, size, twi_error, sensor, id);
	return(1);
}

uint8_t application_function_sensor_read(application_parameters_t ap)
{
	static const __flash char error[] = "> invalid sensor: %d\n";

	uint8_t sensor;

	sensor = (uint8_t)strtoul((const char *)(*ap.args)[1], 0, 0);

	if(!application_sensor_read(sensor, ap.size, ap.dst))
		snprintf_P((char *)ap.dst, ap.size, error, sensor);

	return(1);
}

uint8_t application_function_sensor_write(application_parameters_t ap)
{
	static const __flash char ok[]		= "> sensor %d calibration set to *=%.4f +=%.4f\n";
	static const __flash char error[]	= "> no sensor %d\n";

	uint8_t sensor;
	float factor, offset;

	sensor	= atoi((const char *)(*ap.args)[1]);
	factor	= atof((const char *)(*ap.args)[2]);
	offset	= atof((const char *)(*ap.args)[3]);

	if(!eeprom_write_cal(sensor, factor, offset))
	{
		snprintf_P((char *)ap.dst, (size_t)ap.size, error, sensor);
		return(1);
	}

	if(!eeprom_read_cal(sensor, &factor, &offset))
	{
		snprintf_P((char *)ap.dst, (size_t)ap.size, error, sensor);
		return(1);
	}

	snprintf_P((char *)ap.dst, (size_t)ap.size, ok, sensor, factor, offset);

	return(1);
}

uint8_t application_function_sdmp(application_parameters_t ap)
{
	uint8_t index, offset;

	index = 0;

	while(application_sensor_read(index, ap.size, ap.dst))
	{
		offset	= strlen((const char *)ap.dst);
		ap.dst	+= offset;
		ap.size	-= offset;
		index++;
	}

	return(1);
}
