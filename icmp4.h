#ifndef _icmp4_h_
#define _icmp4_h_

#include <stdint.h>

#include "ethernet.h"
#include "ipv4.h"

typedef enum
{
	ic4_echo_reply		= 0x00,
	ic4_echo_request	= 0x08,
} icmp4_type_t;

typedef struct
{
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;
	uint16_t	id;
	uint16_t	sequence;
	uint8_t		payload[];
} icmp4_echo_message_t;

uint16_t process_icmp4(uint16_t length, const uint8_t *message,
		uint16_t reply_size, uint8_t *reply);

#endif
