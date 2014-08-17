#ifndef eeprom_h
#define eeprom_h

#include <stdint.h>

#include "ethernet_macaddr.h"

void eeprom_read_mac_address(mac_addr_t *dst);

#endif
