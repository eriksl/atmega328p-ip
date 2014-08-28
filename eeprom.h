#ifndef eeprom_h
#define eeprom_h

#include <stdint.h>

#include "ethernet_macaddr.h"

enum
{
	temp_cal_size = 4,
	light_cal_size = 2,
};

void eeprom_read_mac_address(mac_addr_t *dst);

float eeprom_read_bandgap(void);
void eeprom_write_bandgap(float bandgap);

float eeprom_read_temp_cal_factor(uint8_t index);
void eeprom_write_temp_cal_factor(uint8_t index, float factor);

float eeprom_read_temp_cal_offset(uint8_t index);
void eeprom_write_temp_cal_offset(uint8_t index, float offset);

float eeprom_read_light_cal_factor(uint8_t index);
void eeprom_write_light_cal_factor(uint8_t index, float factor);

float eeprom_read_light_cal_offset(uint8_t index);
void eeprom_write_light_cal_offset(uint8_t index, float offset);

#endif
