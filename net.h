#ifndef _net_h_
#define _net_h_
#include <stdint.h>

#define htons(s) (\
	((((s) >> 8) & 0xff) << 0) | \
	((((s) >> 0) & 0xff) << 8)   \
) 

#define ntohs(s) (\
	((((s) >> 8) & 0xff) << 0) | \
	((((s) >> 0) & 0xff) << 8)   \
) 

uint32_t htonl(uint32_t in);
uint32_t ntohl(uint32_t in);

#endif
