#ifndef _ethernet_h_
#define _ethernet_h_

#include <stdint.h>

#include "ethernet_macaddr.h"
#include "ipv4_addr.h"
#include "net.h"

typedef enum
{
	et_ipv4	= 0x0800,
	et_arp	= 0x0806,
} ether_type_t;

typedef struct
{
	mac_addr_t	destination;
	mac_addr_t	source;
	uint16_t	ethertype;
	uint8_t		payload[];
} etherframe_t;

extern const mac_addr_t mac_addr_broadcast;

uint8_t ethernet_address_match(const mac_addr_t *address1, const mac_addr_t *address2);
uint16_t ethernet_process_frame(const uint8_t *frame_in, uint16_t frame_in_length,
		uint8_t *frame_out, uint16_t frame_out_size,
		const mac_addr_t *my_mac_addr, ipv4_addr_t *my_ip_addr);
void ethernet_add_frame_header(etherframe_t *ethernet_frame,
		const mac_addr_t *src, const mac_addr_t *dst,
		uint16_t ethertype);
#endif
