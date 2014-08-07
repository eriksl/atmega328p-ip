#ifndef _tcp4_h_
#define _tcp4_h_

#include "ipv4.h"

typedef enum
{
	est_rst,
	est_syn,
	est_synack,
	est_ack,
	est_fin,
	est_finack,
} empty_segment_t;

typedef enum
{
	t4o_kind_end	= 0,
	t4o_kind_nop	= 1,
	t4o_kind_mss	= 2,
	t4o_kind_ws		= 3,
} option_kind_t;

typedef struct
{
	uint16_t		sport;
	uint16_t		dport;
	uint32_t		sequence_number;
	uint32_t		ack_number;
	unsigned int	ns:1;
	unsigned int	res:3;
	unsigned int	data_offset:4;
	unsigned int	fin:1;
	unsigned int	syn:1;
	unsigned int	rst:1;
	unsigned int	psh:1;
	unsigned int	ack:1;
	unsigned int	urg:1;
	unsigned int	ece:1;
	unsigned int	cwr:1;
	uint16_t		window_size;
	uint16_t		checksum;
	uint16_t		urgent_pointer;
	uint8_t			payload[];
} segment_t;

typedef struct
{
	ipv4_addr_t		src;
	ipv4_addr_t		dst;
	uint8_t			zero;
	uint8_t			protocol;
	uint16_t		length;
} segment_checksum_t;

typedef struct
{
	uint8_t			kind;
} option_t;

typedef struct
{
	uint8_t			kind;
	uint8_t			length;
	uint8_t			value;
} ws_option_t;

typedef struct
{
	uint8_t			kind;
	uint8_t			length;
	uint16_t		value;
} mss_option_t;

typedef struct
{
	mss_option_t	mss;
	ws_option_t		ws;
	option_t		end;
} tcp4_options_t;


uint16_t process_tcp4(uint16_t length, const uint8_t *packet,
		uint16_t reply_size, uint8_t *reply,
		const ipv4_addr_t *src_ipv4, const ipv4_addr_t *dst_ipv4,
		uint8_t protocol);

#endif
