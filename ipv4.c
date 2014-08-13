#include "ipv4.h"
#include "icmp4.h"
#include "udp4.h"
#include "tcp4.h"
#include "stats.h"

const ipv4_addr_t ipv4_addr_zero = {{ 0x00, 0x00, 0x00, 0x00 }};
const ipv4_addr_t ipv4_addr_broadcast  = {{ 0xff, 0xff, 0xff, 0xff }};

uint8_t ipv4_address_match(const ipv4_addr_t *address1, const ipv4_addr_t *address2)
{
	uint8_t ix;

	for(ix = 0; ix < sizeof(ipv4_addr_t); ix++)
		if(address1->byte[ix] != address2->byte[ix])
			return(0);

	return(1);
}

uint16_t ipv4_checksum(uint16_t length1, const uint8_t *data1,
					uint16_t length2, const uint8_t *data2)
{
	uint16_t	offset;
	uint32_t	sum;

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

void ipv4_add_packet_header(ipv4_packet_t *packet,
		const ipv4_addr_t *src, const ipv4_addr_t *dst,
		uint16_t id, uint8_t protocol, uint16_t payload_length)
{
	packet->version				= iv_ipv4;
	packet->header_length		= sizeof(ipv4_packet_t) >> 2;
	packet->dscp				= 0x00;
	packet->ecn					= 0x00;
	packet->total_length		= htons(sizeof(ipv4_packet_t) + payload_length);
	packet->id					= htons(id);
	packet->fragment_offset_1	= 0x00;
	packet->fragment_offset_2	= 0x00;
	packet->flag_reserved		= 0;
	packet->flag_df				= 1;
	packet->flag_mf				= 0;
	packet->ttl					= 0x40;
	packet->protocol			= protocol;
	packet->checksum			= 0;
	packet->src					= *src;
	packet->dst					= *dst;

	packet->checksum			= ipv4_checksum(sizeof(ipv4_packet_t), (uint8_t *)packet, 0, 0);
}

uint16_t process_ipv4(const uint8_t *payload_in, uint16_t payload_in_length,
		uint8_t *payload_out, uint16_t payload_out_size,
		const mac_addr_t *my_mac_addr, ipv4_addr_t *my_ipv4_addr)
{
	const	ipv4_packet_t	*src;
			ipv4_packet_t	*dst;
			uint8_t			header_length;
			uint16_t		total_length;
			uint16_t		payload_out_length;

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

	if(!ipv4_address_match(&src->dst, my_ipv4_addr) &&
			!ipv4_address_match(&src->dst, &ipv4_addr_broadcast))
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
					&src->src, &src->dst, src->protocol, my_mac_addr, my_ipv4_addr);

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

	if(payload_out_length > 0)
	{
		ipv4_add_packet_header(dst, &src->dst, &src->src,
				ntohs(src->id), src->protocol, payload_out_length);

		payload_out_length += sizeof(ipv4_packet_t);
	}

	return(payload_out_length);
}
