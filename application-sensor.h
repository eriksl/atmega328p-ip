#ifndef application_sensor_h
#define application_sensor_h

#include "application.h"

#include <stdint.h>

void application_init_sensor(void);
uint8_t application_sensor_read(uint8_t sensor, uint16_t size, uint8_t *dst);
uint8_t application_function_sensor_read(application_parameters_t ap);
uint8_t application_function_sensor_write(application_parameters_t ap);
uint8_t application_function_bg_write(application_parameters_t ap);
uint8_t application_function_sdmp(application_parameters_t ap);
#endif
