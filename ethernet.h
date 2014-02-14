#ifndef _ethernet_h_
#define _ethernet_h_

#include "net.h"

typedef enum
{
	et_ipv4	= htons(0x0800),
	et_arp	= htons(0x0806),
} ether_type_t;

typedef struct
{
	uint8_t byte[6];
} mac_addr_t;

typedef struct
{
	mac_addr_t	destination;
	mac_addr_t	source;
	uint16_t	ethertype;
	uint8_t		payload[];
} etherframe_t;

#endif
