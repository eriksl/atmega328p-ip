#include "eeprom.h"

#include <avr/eeprom.h>

#include <stdint.h>

enum
{
	eeprom_cal_size = 10,
};

typedef struct
{
	float factor;
	float offset;
} calibration_t;

typedef struct
{
	float			bandgap;
	calibration_t	value[eeprom_cal_size];
} eeprom_t;

float eeprom_read_bandgap(void)
{
	float value;

	eeprom_read_block((void *)&value, (const void *)offsetof(eeprom_t, bandgap), sizeof(value));

	return(value);
}

void eeprom_write_bandgap(float value)
{
	eeprom_update_block((const void *)&value, (void *)offsetof(eeprom_t, bandgap), sizeof(value));
}

uint8_t eeprom_read_cal(uint8_t index, float *factor, float *offset)
{
	if(index >= eeprom_cal_size)
		return(0);

	eeprom_read_block((void *)factor, (const void *)offsetof(eeprom_t, value[index].factor), sizeof(*factor));
	eeprom_read_block((void *)offset, (const void *)offsetof(eeprom_t, value[index].offset), sizeof(*offset));

	return(1);
}

uint8_t eeprom_write_cal(uint8_t index, float factor, float offset)
{
	if(index >= eeprom_cal_size)
		return(0);

	eeprom_update_block((const void *)&factor, (void *)offsetof(eeprom_t, value[index].factor), sizeof(factor));
	eeprom_update_block((const void *)&offset, (void *)offsetof(eeprom_t, value[index].offset), sizeof(offset));

	return(1);
}
