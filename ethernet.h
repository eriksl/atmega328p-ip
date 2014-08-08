#ifndef _ethernet_h_
#define _ethernet_h_

#include "ipv4.h"
#include "net.h"

typedef enum
{
	et_ipv4	= htons(0x0800),
	et_arp	= htons(0x0806),
} ether_type_t;

typedef struct
{
	uint8_t byte[6];
} mac_addr_t;

typedef struct
{
	mac_addr_t	destination;
	mac_addr_t	source;
	uint16_t	ethertype;
	uint8_t		payload[];
} etherframe_t;

extern const mac_addr_t mac_addr_broadcast;

uint16_t ethernet_process_frame(const uint8_t *frame_in, uint16_t frame_in_length,
		uint8_t *frame_out, uint16_t frame_out_size,
		const mac_addr_t *my_mac_addr, const ipv4_addr_t *my_ip_addr);
void ethernet_add_frame_header(etherframe_t *ethernet_frame,
		uint16_t ethertype, const mac_addr_t *src, const mac_addr_t *dst);
#endif
