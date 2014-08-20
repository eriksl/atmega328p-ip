#include "eeprom.h"

#include <avr/eeprom.h>

#include "ethernet_macaddr.h"

#include <stdint.h>

typedef struct
{
	mac_addr_t	my_mac_address;
	float		bandgap;
	float		temp_cal_factor[temp_cal_size];
	float		temp_cal_offset[temp_cal_size];
} eeprom_t;

void eeprom_read_mac_address(mac_addr_t *dst)
{
	eeprom_read_block(dst, (const void *)offsetof(eeprom_t, my_mac_address), sizeof(mac_addr_t));
}

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

float eeprom_read_temp_cal_factor(uint8_t index)
{
	float value;

	if(index >= temp_cal_size)
		return(0);

	eeprom_read_block((void *)&value, (const void *)offsetof(eeprom_t, temp_cal_factor[index]), sizeof(value));

	return(value);
}

void eeprom_write_temp_cal_factor(uint8_t index, float value)
{
	if(index >= temp_cal_size)
		return;

	eeprom_update_block((const void *)&value, (void *)offsetof(eeprom_t, temp_cal_factor[index]), sizeof(value));
}

float eeprom_read_temp_cal_offset(uint8_t index)
{
	float value;

	if(index >= temp_cal_size)
		return(0);

	eeprom_read_block((void *)&value, (const void *)offsetof(eeprom_t, temp_cal_offset[index]), sizeof(value));

	return(value);
}

void eeprom_write_temp_cal_offset(uint8_t index, float value)
{
	if(index >= temp_cal_size)
		return;

	eeprom_update_block((const void *)&value, (void *)offsetof(eeprom_t, temp_cal_offset[index]), sizeof(value));
}
