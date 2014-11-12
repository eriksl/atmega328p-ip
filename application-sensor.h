#ifndef application_sensor_h
#define application_sensor_h

#include "application.h"

#include <stdint.h>

typedef enum
{
	sensor_bandgap,
	sensor_digipicco_temperature,
	sensor_digipicco_humidity,
	sensor_lm75,
	sensor_bmp085_temperature,
	sensor_bmp085_airpressure,
	sensor_tsl2560,
	sensor_bh1750,
	sensor_end
} sensors_t;

extern void application_init_sensor(void);
extern uint8_t application_sensor_read(uint8_t sensor, uint16_t size, uint8_t *dst);
extern uint8_t application_function_sensor_read(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
extern uint8_t application_function_sensor_write(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);
extern uint8_t application_function_bg_write(uint8_t nargs,
		uint8_t args[application_num_args][application_length_args],
		uint16_t size, uint8_t *dst);

extern void		sensor_read_bandgap(float *value, float *raw_value);
extern uint8_t	sensor_read_digipicco(float *temp, float *temp_raw, float *hum, float *hum_raw);
extern uint8_t	sensor_read_lm75(float *value, float *raw_value);
extern uint8_t	sensor_read_bmp085(float *temp, float *temp_raw, float *pressure, float *pressure_raw);
extern uint8_t	sensor_read_tsl2560(float *value, float *raw_value);
extern uint8_t	sensor_read_bh1750(float *value, float *raw_value);

#endif
