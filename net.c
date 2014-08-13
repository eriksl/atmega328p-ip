#include "net.h"

uint32_t htonl(uint32_t in)
{
	uint32_t out =	(((in >> 24) & 0xff) <<  0) |
			(((in >> 16) & 0xff) <<  8) |
			(((in >>  8) & 0xff) << 16) |
			(((in >>  0) & 0xff) << 24);

	return(out);
}

uint32_t ntohl(uint32_t in)
{
	uint32_t out =	(((in >> 24) & 0xff) <<  0) |
			(((in >> 16) & 0xff) <<  8) |
			(((in >>  8) & 0xff) << 16) |
			(((in >>  0) & 0xff) << 24);

	return(out);
}
