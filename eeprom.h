#ifndef eeprom_h
#define eeprom_h

#include <stdint.h>

float eeprom_read_bandgap(void);
void eeprom_write_bandgap(float bandgap);

uint8_t eeprom_read_cal(uint8_t index, float *factor, float *offset);
uint8_t eeprom_write_cal(uint8_t index, float factor, float offset);

#endif
