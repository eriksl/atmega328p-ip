#include <stdint.h>
#include <string.h>

#include "application.h"
#include "ipv4.h"
#include "icmp4.h"

uint16_t process_icmp4(uint16_t length, const uint8_t *message,
		uint16_t reply_size, uint8_t *reply)
		
{
	const				icmp4_echo_message_t *src = (icmp4_echo_message_t *)message;
						icmp4_echo_message_t *dst = (icmp4_echo_message_t *)reply;
	static const char 	*cmd = "beep 3 0";

	if((src->type != ic4_echo_request) || (src->code != 0x00))
		return(0);

	if(ipv4_checksum(length, message, 0, 0) != 0)
		return(0);

	application_content(strlen(cmd), (const uint8_t *)cmd, reply_size, (uint8_t *)reply);

	memcpy(dst, src, length);

	dst->type		= ic4_echo_reply;
	dst->checksum	= 0;
	dst->checksum	= ipv4_checksum(length, reply, 0, 0);

	return(length);
}
