#ifndef _eeprom_h_
#define _eeprom_h_

#include <stdint.h>
#include <avr/eeprom.h>

static inline uint8_t eeprom_read_uint8(const uint8_t *from)
{
	return(eeprom_read_byte(from));
}

static inline void eeprom_write_uint8(uint8_t *to, uint8_t value)
{
	eeprom_update_byte(to, value);
}

static inline uint16_t eeprom_read_uint16(const uint16_t *from)
{
	return(eeprom_read_word(from));
}

static inline void eeprom_write_uint16(uint16_t *to, uint16_t value)
{
	eeprom_update_word(to, value);
}

#endif
