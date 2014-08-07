#ifndef _ipv4_h_
#define _ipv4_h_

#include <stdint.h>

#include "net.h"

enum
{
	ip4_icmp = 0x01,
	ip4_tcp	= 0x06,
	ip4_udp	= 0x11,
};

enum
{
	iv_ipv4 = 0x4,
};

typedef struct
{
	uint8_t byte[4];
} ipv4_addr_t;

typedef struct
{
	unsigned int	header_length:4;
	unsigned int	version:4;
	unsigned int	ecn:2;
	unsigned int	dscp:6;
	uint16_t		total_length;
	uint16_t		id;
	unsigned int	fragment_offset_1:5;
	unsigned int	flag_reserved:1;
	unsigned int	flag_df:1;
	unsigned int	flag_mf:1;
	unsigned int	fragment_offset_2:8;
	uint8_t			ttl;
	uint8_t			protocol;
	uint16_t		checksum;
	ipv4_addr_t		src;
	ipv4_addr_t		dst;
	uint8_t			payload[];
} ipv4_packet_t;

extern const ipv4_addr_t ipv4_addr_zero;
extern const ipv4_addr_t ipv4_addr_broadcast;

uint8_t ipv4_address_match(const ipv4_addr_t *a, const ipv4_addr_t *b);

uint16_t ipv4_checksum(uint16_t length1, const uint8_t *data1,
						uint16_t length2, const uint8_t *data2);

uint16_t process_ipv4(const uint8_t *payload_in, uint16_t payload_in_length,
		uint8_t *payload_out, uint16_t payload_out_length,
		const ipv4_addr_t *my_ipv4);

void ipv4_add_packet_header(ipv4_packet_t *ipv4_packet,
		const ipv4_addr_t *src, const ipv4_addr_t *dst,
		uint8_t protocol, uint16_t payload_length);

#endif
