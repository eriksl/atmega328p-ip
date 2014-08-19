#include "eeprom.h"

#include <avr/eeprom.h>

#include "ethernet_macaddr.h"

#include <stdint.h>

typedef struct
{
	mac_addr_t	my_mac_address;
	uint16_t	bandgap;
} eeprom_t;

void eeprom_read_mac_address(mac_addr_t *dst)
{
	eeprom_read_block(dst, (const void *)offsetof(eeprom_t, my_mac_address), sizeof(mac_addr_t));
}

uint16_t eeprom_read_bandgap(void)
{
	return(eeprom_read_word((const uint16_t *)offsetof(eeprom_t, bandgap)));
}

void eeprom_write_bandgap(uint16_t bandgap)
{
	eeprom_update_word((uint16_t *)offsetof(eeprom_t, bandgap), bandgap);
}
