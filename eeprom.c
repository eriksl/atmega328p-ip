#include "eeprom.h"

#include <avr/eeprom.h>

#include "ethernet_macaddr.h"

#include <stdint.h>

typedef struct
{
	mac_addr_t my_mac_address;
} eeprom_t;

void eeprom_read_mac_address(mac_addr_t *dst)
{
	eeprom_read_block(dst, (const void *)offsetof(eeprom_t, my_mac_address), sizeof(mac_addr_t));
}
