#ifndef _arp_h_
#define _arp_h_

#include <stdint.h>

#include "net.h"
#include "ethernet.h"
#include "ipv4.h"

typedef enum
{
	ah_ether	= htons(1),
} arp_hw_t;

typedef enum
{
	ao_request	= htons(1),
	ao_reply	= htons(2),
} arp_operation_t;

typedef struct
{
	uint16_t	htype;	//	0
	uint16_t	ptype;	//	2
	uint8_t		hlen;	//	4
	uint8_t		plen;	//	5
	uint16_t	oper;	//	6
	mac_addr_t	sha;	//	8
	ipv4_addr_t	spa;	//	14
	mac_addr_t	tha;	//	18
	ipv4_addr_t	tpa;	//	24
} ether_arp_pkt_t;

uint16_t process_arp(uint16_t length, const uint8_t *packet, uint16_t reply_size, uint8_t *reply,
		const mac_addr_t *mac, const ipv4_addr_t *ipv4);

#endif
