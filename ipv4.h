#ifndef _ipv4_h_
#define _ipv4_h_

#include <stdint.h>

#include "net.h"
#include "ethernet.h"

typedef enum
{
	ip4_icmp = 0x01,
	ip4_tcp	= 0x06,
	ip4_udp	= 0x11,
} ipv4_protocol_t;

typedef struct
{
	uint8_t byte[4];
} ipv4_addr_t;

uint8_t ipv4_address_match(const ipv4_addr_t *a, const ipv4_addr_t *b);

uint16_t ipv4_checksum(uint16_t length1, const uint8_t *data1,
						uint16_t length2, const uint8_t *data2);

uint16_t process_ipv4(uint16_t length, const uint8_t *packet,
		uint16_t reply_size, uint8_t *reply,
		const mac_addr_t *mac, const ipv4_addr_t *ipv4);

#endif
