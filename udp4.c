#include <avr/io.h>

#include "net.h"
#include "udp4.h"
#include "ipv4.h"
#include "content.h"
#include "stats.h"

typedef struct
{
	uint16_t		sport;
	uint16_t		dport;
	uint16_t		length;
	uint16_t		checksum;
	uint8_t			payload[];
} datagram_t;

typedef struct
{
	ipv4_addr_t		src;
	ipv4_addr_t		dst;
	uint8_t			zero;
	uint8_t			protocol;
	uint16_t		length;
} datagram_checksum_t;

typedef struct
{
			uint16_t	sport;
	const	uint16_t	dport;
			ipv4_addr_t	src;
} state_entry_t;

static state_entry_t state[] =
{
	{ 0, 28022, {{ 0, 0, 0, 0 }}, },
	{ 0, 28023, {{ 0, 0, 0, 0 }}, },
	{ 0, 0,		{{ 0, 0, 0, 0 }}, },
};

static datagram_checksum_t checksum_header;

uint16_t process_udp4(uint16_t length, const uint8_t *packet,
		uint16_t reply_size, uint8_t *reply,
		const ipv4_addr_t *src_ipv4, const ipv4_addr_t *dst_ipv4,
		uint8_t protocol)
{
	static const			datagram_t *src;
	static					datagram_t *dst;
	static state_entry_t *	state_entry;
	static int16_t			content_length;

	src = (datagram_t *)packet;
	dst = (datagram_t *)reply;

	checksum_header.src			= *src_ipv4;
	checksum_header.dst			= *dst_ipv4;
	checksum_header.zero		= 0;
	checksum_header.protocol	= protocol;
	checksum_header.length		= htons(length);

	if(ipv4_checksum(sizeof(checksum_header), (uint8_t *)&checksum_header, length, packet) != 0)
	{
		ip_bad_checksum++;
		return(0);
	}

	for(state_entry = &state[0]; state_entry->dport != 0; state_entry++)
		if(ntohs(src->dport) == state_entry->dport)
			break;

	if(state_entry->dport == 0)
		return(0);	// drop, this should actually send an icmp unreachable

	state_entry->sport	= ntohs(src->sport);
	state_entry->src	= *src_ipv4;

	content_length	= content(state_entry->dport,
			length     - sizeof(*src), &src->payload[0],
			reply_size - sizeof(*dst), &dst->payload[0]);

	checksum_header.src			= *dst_ipv4;
	checksum_header.dst			= *src_ipv4;
	checksum_header.zero		= 0;
	checksum_header.protocol	= protocol;
	checksum_header.length		= htons(sizeof(*dst) + content_length);

	dst->sport		= src->dport;
	dst->dport		= src->sport;
	dst->length		= htons(sizeof(*dst) + content_length);
	dst->checksum	= 0;

	dst->checksum = ipv4_checksum(sizeof(checksum_header), (uint8_t *)&checksum_header, sizeof(*dst) + content_length, (uint8_t *)dst);

	return(sizeof(*dst) + content_length);
}
