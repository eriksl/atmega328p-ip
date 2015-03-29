#ifndef sensor_h
#define sensor_h

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
	sensor_htu21_temperature,
	sensor_htu21_humidity,
	sensor_end
} sensors_t;

void	sensor_init_bandgap(void);
void	sensor_init_tsl2560(void);
void	sensor_init_bh1750(void);

void	sensor_read_bandgap(float *value, float *raw_value);
uint8_t	sensor_read_digipicco(float *temp, float *temp_raw, float *hum, float *hum_raw);
uint8_t	sensor_read_lm75(float *value, float *raw_value);
uint8_t	sensor_read_bmp085(float *temp, float *temp_raw, float *pressure, float *pressure_raw);
uint8_t	sensor_read_tsl2560(float *value, float *raw_value);
uint8_t	sensor_read_bh1750(float *value, float *raw_value);
uint8_t	sensor_read_htu21_temp(float *value, float *raw_value);
uint8_t	sensor_read_htu21_hum(float *value, float *raw_value);

#endif
