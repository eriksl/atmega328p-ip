#include <stdint.h>
#include <string.h>

#include "arp.h"

uint16_t process_arp(const uint8_t *payload_in, uint16_t payload_in_length,
		uint8_t *payload_out,
		const mac_addr_t *mac, const ipv4_addr_t *ipv4)
{
	const	ether_arp_pkt_t *src;
			ether_arp_pkt_t *dst;

	src = (ether_arp_pkt_t *)payload_in;
	dst = (ether_arp_pkt_t *)payload_out;

	if((src->htype == ah_ether) &&
			(ntohs(src->ptype) == et_ipv4) &&
			(src->oper == ao_request) &&
			ipv4_address_match(&src->tpa, ipv4))
	{
		dst->htype	= src->htype;
		dst->ptype	= src->ptype;
		dst->hlen	= src->hlen;
		dst->plen	= src->plen;
		dst->oper	= ao_reply;
		dst->sha	= *mac;
		dst->spa	= *ipv4;
		dst->tha	= src->sha;
		dst->tpa	= src->spa;

		return(sizeof(ether_arp_pkt_t));
	}

	return(0);
}
