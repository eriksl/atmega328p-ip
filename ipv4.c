#include "ipv4.h"
#include "icmp4.h"
#include "udp4.h"
#include "tcp4.h"
#include "stats.h"

typedef enum
{
	iv_ipv4 = 0x4,
} ipv4_version_t;

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

uint8_t ipv4_address_match(const ipv4_addr_t *a, const ipv4_addr_t *b)
{
	if(a->byte[0] != b->byte[0])
		return(0);

	if(a->byte[1] != b->byte[1])
		return(0);

	if(a->byte[2] != b->byte[2])
		return(0);

	if(a->byte[3] != b->byte[3])
		return(0);

	return(1);
}

uint16_t ipv4_checksum(uint16_t length1, const uint8_t *data1,
					uint16_t length2, const uint8_t *data2)
{
	static uint16_t	offset;
	static uint32_t	sum;

	sum = 0;

	for(offset = 0; offset < length1; offset++)
		if(!(offset & 0x0001))	// even offset
			sum += ((data1[offset] << 8) & 0xff00);
		else					// odd offset
			sum += ((data1[offset] << 0) & 0x00ff);

	for(offset = 0; offset < length2; offset++)
		if(!(offset & 0x0001))	// even offset
			sum += ((data2[offset] << 8) & 0xff00);
		else					// odd offset
			sum += ((data2[offset] << 0) & 0x00ff);

	while(sum >> 16)
		sum = ((sum >> 0) & 0xffff) + ((sum >> 16) & 0xffff);

	sum = ~sum;

	if(sum == 0)
		sum = ~0;

	return(htons(sum & 0xffff));
}

uint16_t process_ipv4(uint16_t packet_length, const uint8_t *packet,
		uint16_t reply_size, uint8_t *reply,
		const mac_addr_t *mac, const ipv4_addr_t *ipv4)
{
	static const	ipv4_packet_t *src;
	static			ipv4_packet_t *dst;
	static uint8_t	header_length;
	static uint16_t	total_length;
	static uint16_t	reply_length ;

	src = (ipv4_packet_t *)packet;
	dst = (ipv4_packet_t *)reply;

	reply_length = 0;

	if(src->version != iv_ipv4)
		return(0);

	header_length	= src->header_length << 2;
	total_length	= ntohs(src->total_length);

	if(packet_length == 46) // min. ethernet payload size, padded
	{
		if(total_length > packet_length)
			return(0);
	}
	else
	{
		if(total_length != packet_length)
			return(0);
	}

	if(ipv4_checksum(header_length, packet, 0, 0) != 0)
	{
		ip_bad_checksum++;
		return(0);
	}

	if(!ipv4_address_match(&src->dst, ipv4))
	{
		ip_stray_pkt++;
		return(0);
	}

	switch(src->protocol)
	{
		case(ip4_icmp):
		{
			ip_icmp4_pkt_in++;

			reply_length = process_icmp4(total_length - header_length, &packet[header_length],
					reply_size - sizeof(*dst), &dst->payload[0]);

			if(reply_length)
				ip_icmp4_pkt_out++;

			break;
		}

		case(ip4_udp):
		{
			ip_udp4_pkt_in++;

			reply_length = process_udp4(total_length - header_length, &packet[header_length],
					reply_size - sizeof(*dst), &dst->payload[0],
					&src->src, &src->dst, src->protocol);

			if(reply_length)
				ip_udp4_pkt_out++;

			break;
		}

		case(ip4_tcp):
		{
			ip_tcp4_pkt_in++;
			reply_length = process_tcp4(total_length - header_length, &packet[header_length],
					reply_size - sizeof(*dst), &dst->payload[0],
					&src->src, &src->dst, src->protocol);

			if(reply_length)
				ip_tcp4_pkt_out++;

			break;
		}

		default:
		{
			ip_other_pkt++;
			return(0);
		}
	}

	if(reply_length)
	{
		reply_length = reply_length + sizeof(*dst);		// header + payload

		dst->version			= iv_ipv4;
		dst->header_length		= sizeof(*dst) >> 2;
		dst->dscp				= 0x00;
		dst->ecn				= 0x00;
		dst->total_length		= htons(reply_length);		// header + payload
		dst->id					= src->id;
		dst->fragment_offset_1	= 0x00;
		dst->fragment_offset_2	= 0x00;
		dst->flag_reserved		= 0;
		dst->flag_df			= 1;
		dst->flag_mf			= 0;
		dst->ttl				= 0x40;
		dst->protocol			= src->protocol;
		dst->checksum			= 0;
		dst->src				= src->dst;
		dst->dst				= src->src;

		dst->checksum = ipv4_checksum(sizeof(*dst), reply, 0, 0);
	}

	return(reply_length);
}
