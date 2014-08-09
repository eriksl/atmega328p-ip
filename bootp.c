#include "bootp.h"
#include "udp4.h"
#include "ipv4.h"
#include "ethernet.h"

#include <string.h>

uint16_t bootp_create_request(uint8_t *frame, const mac_addr_t *src)
{
	static udp4_composite_header_t	*composite_header;
	static bootp_packet_t			*packet;

	composite_header	= (udp4_composite_header_t *)frame;
	packet				= (bootp_packet_t *)&composite_header->payload;

	memset(packet, 0, sizeof(*packet));

	packet->op			= bo_bootrequest;
	packet->htype		= bh_ether;
	packet->hlen		= sizeof(mac_addr_t);
	packet->xid.byte[0]	= src->byte[5];
	packet->xid.byte[1]	= src->byte[4];
	packet->xid.byte[2]	= src->byte[3];
	packet->xid.byte[3]	= src->byte[2];
	packet->flagsb		= 0x01;
	packet->chaddr		= *src;

	udp4_add_datagram_header(&composite_header->udp4_header,
			&ipv4_addr_zero, &ipv4_addr_broadcast,
			bp_port_bootpc, bp_port_bootps,
			sizeof(bootp_packet_t));

	ipv4_add_packet_header(&composite_header->ipv4_header,
			&ipv4_addr_zero, &ipv4_addr_broadcast,
			0, ip4_udp, sizeof(bootp_packet_t) + sizeof(udp4_datagram_t));

	ethernet_add_frame_header(&composite_header->ether_header, src, &mac_addr_broadcast, et_ipv4);

	return(sizeof(udp4_composite_header_t) + sizeof(bootp_packet_t));
}

void bootp_process_reply(const uint8_t *payload, uint16_t length,
		const mac_addr_t *dst, ipv4_addr_t *my_ipv4_addr)
{
	static bootp_packet_t *packet;

	packet	= (bootp_packet_t *)payload;

	if(packet->op != bo_bootreply)
		return;

	if(packet->htype != bh_ether)
		return;

	if(packet->hlen != sizeof(mac_addr_t))
		return;

	if(packet->xid.byte[0] != dst->byte[5] ||
			packet->xid.byte[1] != dst->byte[4] ||
			packet->xid.byte[2] != dst->byte[3] ||
			packet->xid.byte[3] != dst->byte[2])
		return;

	if(!ethernet_address_match(&packet->chaddr, dst))
		return;

	*my_ipv4_addr = packet->yiaddr;
}
