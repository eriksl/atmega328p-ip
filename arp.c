#include <stdint.h>
#include <string.h>

#include "arp.h"

uint16_t process_arp(uint16_t length, const uint8_t *packet, uint16_t reply_size, uint8_t *reply,
		const mac_addr_t *mac, const ipv4_addr_t *ipv4)
{
	const	ether_arp_pkt_t *src = (ether_arp_pkt_t *)packet;
			ether_arp_pkt_t *dst = (ether_arp_pkt_t *)reply;

	if(reply_size <= sizeof(*dst))
		return(0);

	if((src->htype == ah_ether) &&
			(src->ptype == et_ipv4) &&
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

		return(sizeof(*dst));
	}

	return(0);
}
