#include "bootp.h"
#include "udp4.h"
#include "ipv4.h"
#include "ethernet.h"

#include <string.h>

uint16_t bootp_create_request_frame(uint8_t *buffer, const mac_addr_t *src)
{
	static udp4_composite_header	*composite_header;
	static bootp_packet_t			*payload;

	composite_header	= (udp4_composite_header *)buffer;
	payload				= (bootp_packet_t *)&composite_header->payload;

	memset(payload, 0, sizeof(*payload));

	payload->op				= bo_bootrequest;
	payload->htype			= bh_ether;
	payload->hlen			= sizeof(mac_addr_t);
	payload->xid.byte[0]	= src->byte[5];
	payload->xid.byte[1]	= src->byte[4];
	payload->xid.byte[2]	= src->byte[3];
	payload->xid.byte[3]	= src->byte[2];
	payload->flagsb			= 0x01;
	payload->chaddr			= *src;

	udp4_add_datagram_header(&composite_header->udp4_header, bp_port_bootpc, bp_port_bootps,
			sizeof(bootp_packet_t), &ipv4_addr_zero, &ipv4_addr_broadcast);

	ipv4_add_packet_header(&composite_header->ipv4_header, &ipv4_addr_zero, &ipv4_addr_broadcast,
			ip4_udp, sizeof(*payload) + sizeof(udp4_datagram_t));

	ethernet_add_frame_header(&composite_header->ether_header, et_ipv4, src, &mac_addr_broadcast);

	return(sizeof(udp4_composite_header) + sizeof(bootp_packet_t));
}
