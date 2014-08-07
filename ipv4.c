#include "ipv4.h"
#include "icmp4.h"
#include "udp4.h"
#include "tcp4.h"
#include "stats.h"

const ipv4_addr_t ipv4_addr_zero = {{ 0x00, 0x00, 0x00, 0x00 }};
const ipv4_addr_t ipv4_addr_broadcast  = {{ 0xff, 0xff, 0xff, 0xff }};

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

void ipv4_add_packet_header(ipv4_packet_t *ipv4_packet,
		const ipv4_addr_t *src, const ipv4_addr_t *dst, uint8_t protocol,
		uint16_t payload_length)
{
	ipv4_packet->version			= iv_ipv4;
	ipv4_packet->header_length		= sizeof(*ipv4_packet) >> 2;
	ipv4_packet->dscp				= 0x00;
	ipv4_packet->ecn				= 0x00;
	ipv4_packet->total_length		= htons(sizeof(*ipv4_packet) + payload_length);
	ipv4_packet->id					= 0;
	ipv4_packet->fragment_offset_1	= 0x00;
	ipv4_packet->fragment_offset_2	= 0x00;
	ipv4_packet->flag_reserved		= 0;
	ipv4_packet->flag_df			= 1;
	ipv4_packet->flag_mf			= 0;
	ipv4_packet->ttl				= 0x40;
	ipv4_packet->protocol			= protocol;
	ipv4_packet->checksum			= 0;
	ipv4_packet->src				= *src;
	ipv4_packet->dst				= *dst;
}

uint16_t process_ipv4(const uint8_t *payload_in, uint16_t payload_in_length,
		uint8_t *payload_out, uint16_t payload_out_size,
		const ipv4_addr_t *ipv4)
{
	static const	ipv4_packet_t *src;
	static			ipv4_packet_t *dst;
	static uint8_t	header_length;
	static uint16_t	total_length;
	static uint16_t	payload_out_length;

	src = (ipv4_packet_t *)payload_in;
	dst = (ipv4_packet_t *)payload_out;

	payload_out_length = 0;

	if(src->version != iv_ipv4)
		return(0);

	header_length	= src->header_length << 2;
	total_length	= ntohs(src->total_length);

	if(payload_in_length == 46) // min. ethernet payload size, padded #FIXME
	{
		if(total_length > payload_in_length)
			return(0);
	}
	else
	{
		if(total_length != payload_in_length)
			return(0);
	}

	if(ipv4_checksum(header_length, payload_in, 0, 0) != 0)
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

			payload_out_length = process_icmp4(total_length - header_length, &payload_in[header_length],
					payload_out_size - sizeof(ipv4_packet_t), &dst->payload[0]);

			if(payload_out_length)
				ip_icmp4_pkt_out++;

			break;
		}

		case(ip4_udp):
		{
			ip_udp4_pkt_in++;

			payload_out_length = process_udp4(total_length - header_length, &payload_in[header_length],
					payload_out_size - sizeof(ipv4_packet_t), &dst->payload[0],
					&src->src, &src->dst, src->protocol);

			if(payload_out_length)
				ip_udp4_pkt_out++;

			break;
		}

		case(ip4_tcp):
		{
			ip_tcp4_pkt_in++;
			payload_out_length = process_tcp4(total_length - header_length, &payload_in[header_length],
					payload_out_size - sizeof(ipv4_packet_t), &dst->payload[0],
					&src->src, &src->dst, src->protocol);

			if(payload_out_length)
				ip_tcp4_pkt_out++;

			break;
		}

		default:
		{
			ip_other_pkt++;
			return(0);
		}
	}

	if(payload_out_length)
	{
		payload_out_length = payload_out_length + sizeof(ipv4_packet_t);

		dst->version			= iv_ipv4;
		dst->header_length		= sizeof(ipv4_packet_t) >> 2;
		dst->dscp				= 0x00;
		dst->ecn				= 0x00;
		dst->total_length		= htons(payload_out_length); // header + payload
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

		dst->checksum = ipv4_checksum(sizeof(ipv4_packet_t), payload_out, 0, 0);
	}

	return(payload_out_length);
}
