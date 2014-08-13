#include "ethernet.h"
#include "net.h"
#include "stats.h"
#include "watchdog.h"
#include "arp.h"
#include "ipv4.h"

const mac_addr_t mac_addr_broadcast = {{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }};

uint8_t ethernet_address_match(const mac_addr_t *address1, const mac_addr_t *address2)
{
	uint8_t ix;

	for(ix = 0; ix < sizeof(mac_addr_t); ix++)
		if(address1->byte[ix] != address2->byte[ix])
			return(0);

	return(1);
}

uint16_t ethernet_process_frame(const uint8_t *frame_in, uint16_t frame_in_length,
		uint8_t *frame_out, uint16_t frame_out_size,
		const mac_addr_t *my_mac_address, ipv4_addr_t *my_ipv4_address)
{
	etherframe_t	*etherframe_in;
	etherframe_t	*etherframe_out;
	uint16_t		payload_length_out;

	etherframe_in		= (etherframe_t *)frame_in;
	etherframe_out		= (etherframe_t *)frame_out;
	payload_length_out	= 0;

	switch(ntohs(etherframe_in->ethertype))
	{
		case(et_arp):
		{
			ip_arp_pkt_in++;
			payload_length_out = process_arp(&etherframe_in->payload[0], frame_in_length - sizeof(etherframe_t),
					&etherframe_out->payload[0], my_mac_address, my_ipv4_address);

			if(payload_length_out)
				ip_arp_pkt_out++;

			break;
		}

		case(et_ipv4):
		{
			ip_ipv4_pkt_in++;
			payload_length_out = process_ipv4(&etherframe_in->payload[0], frame_in_length - sizeof(etherframe_t),
					&etherframe_out->payload[0], frame_out_size - sizeof(etherframe_t),
					my_mac_address, my_ipv4_address);

			if(payload_length_out)
				ip_ipv4_pkt_out++;

			break;
		}
	}

	etherframe_out->destination	= etherframe_in->source;
	etherframe_out->source		= *my_mac_address;
	etherframe_out->ethertype	= etherframe_in->ethertype;

	return(sizeof(etherframe_t) + payload_length_out);
}

void ethernet_add_frame_header(etherframe_t *ethernet_frame,
		const mac_addr_t *src, const mac_addr_t *dst,
		uint16_t ethertype)
{
	ethernet_frame->source		= *src;
	ethernet_frame->destination	= *dst;
	ethernet_frame->ethertype	= htons(ethertype);
}
