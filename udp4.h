#ifndef _udp4_h_
#define _udp4_h_

#include <stdint.h>

#include "ipv4.h"

uint16_t process_udp4(uint16_t length, const uint8_t *packet,
		uint16_t reply_size, uint8_t *reply,
		const ipv4_addr_t *src_ipv4, const ipv4_addr_t *dst_ipv4,
		uint8_t protocol);

#endif
