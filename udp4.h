#ifndef _udp4_h_
#define _udp4_h_

#include <stdint.h>

#include "ipv4.h"
#include "ethernet.h"

typedef struct
{
	uint16_t		sport;
	uint16_t		dport;
	uint16_t		length;
	uint16_t		checksum;
	uint8_t			payload[];
} udp4_datagram_t;

typedef struct
{
	ipv4_addr_t		src;
	ipv4_addr_t		dst;
	uint8_t			zero;
	uint8_t			protocol;
	uint16_t		length;
} udp4_datagram_checksum_t;

typedef struct
{
			uint16_t	sport;
	const	uint16_t	dport;
			ipv4_addr_t	src;
} state_entry_t;

typedef struct
{
	etherframe_t	ether_header;
	ipv4_packet_t	ipv4_header;
	udp4_datagram_t	udp4_header;
	uint8_t			payload[];
} udp4_composite_header_t;

uint16_t process_udp4(uint16_t length, const uint8_t *packet,
		uint16_t reply_size, uint8_t *reply,
		const ipv4_addr_t *src_ipv4_addr, const ipv4_addr_t *dst_ipv4_addr,
		uint8_t protocol,
		const mac_addr_t *my_mac_addr, ipv4_addr_t *my_ipv4_addr);

void udp4_add_datagram_header(udp4_datagram_t *udp4_datagram,
		const ipv4_addr_t *ipv4_src, const ipv4_addr_t *ipv4_dst,
		uint16_t udp4_src, uint16_t udp4_dst,
		uint16_t payload_length);

#endif
