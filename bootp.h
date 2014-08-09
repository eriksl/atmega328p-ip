#ifndef _bootp_h_
#define _bootp_h_

#include <stdint.h>

#include "net.h"
#include "ethernet.h"
#include "ipv4.h"

enum
{
	bo_bootrequest = 0x01,
	bo_bootreply = 0x02,
};

enum
{
	bh_ether = 0x01,
	bh_802 = 0x06,
};

enum
{
	bp_port_bootps = 67,
	bp_port_bootpc = 68,
};

typedef struct
{
	uint8_t byte[4];
} bootp_xid_t;

typedef struct
{
	uint8_t			op;			// 0-0
	uint8_t			htype;		// 1-1
	uint8_t			hlen;		// 2-2
	uint8_t			hops;		// 3-3
	bootp_xid_t		xid;		// 4-7
	uint16_t		secs;		// 8-9

	uint8_t			flagsh:7;	// 10-10
	unsigned int	flagsb:1;	// 10-10
	uint8_t			flagsl;		// 11-11

	ipv4_addr_t		ciaddr;		// 12-15
	ipv4_addr_t		yiaddr;		// 16-19
	ipv4_addr_t		siaddr;		// 20-23
	ipv4_addr_t		giaddr;		// 24-27
	mac_addr_t		chaddr;		// 28-33
	uint8_t			chaddrf[10];// 34-43
	uint8_t			sname[64];	// 44-107
	uint8_t			file[128];	// 108-235
	uint8_t			vend[64];	// 236-300
} bootp_packet_t;

uint16_t bootp_create_request(uint8_t *frame, const mac_addr_t *src);
void bootp_process_reply(const uint8_t *payload, uint16_t length,
		const mac_addr_t *my_mac_addr, ipv4_addr_t *my_ipv4_addr);
#endif
