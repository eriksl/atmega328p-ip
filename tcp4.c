#include <avr/io.h>

#include "net.h"
#include "tcp4.h"
#include "ipv4.h"
#include "application.h"
#include "stats.h"

typedef enum
{
	state_listen,
	state_connected,
	state_close,
} state_t;

typedef struct
{
			uint8_t		state;
			uint16_t	sport;
	const	uint16_t	dport;
			ipv4_addr_t	src;
			uint32_t	lseq;
			uint32_t	rseq;
} state_entry_t;

static state_entry_t state[] =
{
	{ state_listen, 0, 23, {{ 0, 0, 0, 0 }}, 0, 0 },
	{ 0, 0, 0, {{ 0, 0, 0, 0 }}, 0, 0 },
};

static void reset_state(state_entry_t *st)
{
	st->state		= state_listen;
	st->sport		= 0;
	st->src.byte[0]	= 0;
	st->src.byte[1]	= 0;
	st->src.byte[2]	= 0;
	st->src.byte[3]	= 0;
	st->lseq		= 0;
	st->rseq		= 0;
}

static uint16_t init_segment(const state_entry_t *state_entry, uint8_t add_syn_options,
							uint16_t mss, const segment_t *src, segment_t *dst)
{
	dst->sport = src->dport;
	dst->dport = src->sport;

	if(state_entry)
	{
		dst->sequence_number = htonl(state_entry->lseq);
		dst->ack_number	     = htonl(state_entry->rseq);
	}
	else
	{
		dst->sequence_number = src->ack_number;
		dst->ack_number	     = htonl(ntohl(src->sequence_number) + 1);
	}

	dst->ns				= 0;
	dst->res			= 0;
	dst->data_offset	= (sizeof(segment_t)) >> 2;

	if(add_syn_options)
		dst->data_offset += (sizeof(tcp4_options_t)) >> 2;

	dst->cwr = 0;
	dst->ece = 0;
	dst->urg = 0;
	dst->ack = 0;
	dst->psh = 0;
	dst->rst = 0;
	dst->syn = 0;
	dst->fin = 0;

	dst->window_size		= htons(mss);
	dst->checksum			= 0;
	dst->urgent_pointer		= 0;

	if(add_syn_options)
	{
		tcp4_options_t *options = (tcp4_options_t *)&dst->payload[0];

		options->mss.kind	= t4o_kind_mss;
		options->mss.length	= sizeof(mss_option_t);
		options->mss.value	= htons(mss);
		options->ws.kind	= t4o_kind_ws;
		options->ws.length	= sizeof(ws_option_t);
		options->ws.value	= 0;
		options->end.kind	= t4o_kind_end;

		return(sizeof(*dst) + sizeof(*options));
	}

	return(sizeof(*dst));
}

static void finish_segment(uint16_t header_plus_data_length, const segment_t *src, segment_t *dst,
		const ipv4_addr_t *src_ipv4, const ipv4_addr_t *dst_ipv4)
{
	segment_checksum_t checksum_header;

	checksum_header.src			= *src_ipv4;
	checksum_header.dst			= *dst_ipv4;
	checksum_header.zero		= 0;
	checksum_header.protocol	= ip4_tcp;
	checksum_header.length		= htons(header_plus_data_length);

	dst->checksum = ipv4_checksum(sizeof(checksum_header), (uint8_t *)&checksum_header,
				header_plus_data_length, (uint8_t *)dst);
}

static uint16_t empty_segment(const state_entry_t *state_entry, uint16_t mss, const segment_t *src, segment_t *dst,
		const ipv4_addr_t *src_ipv4, const ipv4_addr_t *dst_ipv4, uint8_t segment_type)
{
	uint16_t total_length;

	switch(segment_type)
	{
		case(est_rst):
		{
			total_length = init_segment(state_entry, 0, 0, src, dst);
			dst->ack = 1;
			dst->rst = 1;
			break;
		}

		case(est_syn):
		{
			total_length = init_segment(state_entry, 1, mss, src, dst);
			dst->syn = 1;
			break;
		}

		case(est_synack):
		{
			total_length = init_segment(state_entry, 1, mss, src, dst);
			dst->syn = 1;
			dst->ack = 1;
			break;
		}

		case(est_ack):
		{
			total_length = init_segment(state_entry, 1, mss, src, dst);
			dst->ack = 1;
			dst->psh = 1;
			break;
		}

		case(est_fin):
		{
			total_length = init_segment(state_entry, 1, mss, src, dst);
			dst->fin = 1;
			break;
		}

		case(est_finack):
		{
			total_length = init_segment(state_entry, 1, mss, src, dst);
			dst->fin = 1;
			dst->ack = 1;
			break;
		}
	}

	finish_segment(total_length, src, dst, src_ipv4, dst_ipv4);

	return(total_length);
}

uint16_t process_tcp4(uint16_t length, const uint8_t *packet,
		uint16_t reply_size, uint8_t *reply,
		const ipv4_addr_t *src_ipv4, const ipv4_addr_t *dst_ipv4,
		uint8_t protocol)
{
	const				segment_t *src;
						segment_t *dst;
	uint16_t			max_content_length;
	int16_t				received_content_length, send_content_length;
	uint8_t				src_header_length, dst_header_length;
	state_entry_t		*state_entry;
	uint16_t			rv;
	segment_checksum_t	checksum_header;

	src = (segment_t *)packet;
	dst = (segment_t *)reply;

	max_content_length	= reply_size - sizeof(*dst) - sizeof(tcp4_options_t);
	src_header_length	= src->data_offset << 2;

	checksum_header.src			= *src_ipv4;
	checksum_header.dst			= *dst_ipv4;
	checksum_header.zero		= 0;
	checksum_header.protocol	= protocol;
	checksum_header.length		= htons(length);

	if(ipv4_checksum(sizeof(checksum_header), (uint8_t *)&checksum_header, length, packet) != 0)
	{
		ip_bad_checksum++;
		return(0);
	}

	for(state_entry = &state[0]; state_entry->dport != 0; state_entry++)
		if(ntohs(src->dport) == state_entry->dport)
			break;

	if(state_entry->dport == 0)
	{
		if(src->rst) // break RST loop
			return(0);
		else
			return(empty_segment(0, 0, src, dst, src_ipv4, dst_ipv4, est_rst));
	}

	if(src->rst)	// break RST loop
	{
		reset_state(state_entry);
		return(0);
	}

	switch(state_entry->state)
	{
		//   <- nothing
		// * -> syn
		// * <- syn/ack
		//   -> ack

		case(state_listen):
		{
			if(src->ack || src->fin)	// no fin or ack allowed before connection, reset
			{
				reset_state(state_entry);
				return(empty_segment(0, 0, src, dst, src_ipv4, dst_ipv4, est_rst));
			}

			if(src->syn)
			{
				state_entry->state	= state_connected;
				state_entry->sport	= ntohs(src->sport);
				state_entry->src	= *src_ipv4;
				state_entry->rseq	= ntohl(src->sequence_number) + 1;	// count SYN as 1 byte

				rv = empty_segment(state_entry, max_content_length, src, dst, src_ipv4, dst_ipv4, est_synack);

				state_entry->lseq++; // count SYN/ACK as 1 byte

				return(rv);
			}

			break;
		}

		//   <- syn/ack		syn/ack got lost, try again
		// * -> syn
		// * <- syn/ack
		//   -> ack

		//   <- syn/ack		data
		// * -> ack
		// * <- ack + data
		//   -> ack

		//   remote shuts down
		//   <- syn/ack
		//   -> ack
		//   <- ack
		// * -> fin/ack		seq=remote		ack=local
		// * <- fin/ack		seq=local		ack=remote+1
		//   -> ack			seq=remote+1	ack=local+1

		//   we shut down
		//   <- syn/ack
		// * -> ack
		// * <- fin/ack		seq=local		ack=remote
		//   -> fin/ack		seq=remote		ack=local+1
		//   <- ack			seq=local+1		ack=remote+1

		case(state_connected):
		{
			if(!ipv4_address_match(&state_entry->src, src_ipv4)) // refuse other host while connected
				return(empty_segment(0, 0, src, dst, src_ipv4, dst_ipv4, est_rst));

			if(state_entry->sport != ntohs(src->sport)) // refuse second connection while connected
				return(empty_segment(0, 0, src, dst, src_ipv4, dst_ipv4, est_rst));

			if(src->syn)		// our syn/ack got lost, try again
			{
				if(src->ack)	// syn/ack from remote is impossible, reset
				{
					reset_state(state_entry);
					return(empty_segment(0, 0, src, dst, src_ipv4, dst_ipv4, est_rst));
				}

				state_entry->rseq = ntohl(src->sequence_number);
				return(empty_segment(state_entry, max_content_length, src, dst, src_ipv4, dst_ipv4, est_synack));
			}

			if(src->fin)	// remote wants to shutdown.
			{				// note: this can be either fin or fin/ack from the remote in this state
				state_entry->rseq = ntohl(src->sequence_number) + 1;
				state_entry->state = state_close;
				return(empty_segment(state_entry, max_content_length, src, dst, src_ipv4, dst_ipv4, est_finack));
			}

			if(src->ack)
			{
				received_content_length = length - src_header_length;

				if(received_content_length == 0)	// pure ACK (no data)
					return(0);						// ignore

				state_entry->rseq	= ntohl(src->sequence_number) + received_content_length;
				dst_header_length	= init_segment(state_entry, 0, max_content_length, src, dst);
				send_content_length	= application_content(	received_content_length,				&packet[src_header_length],
															max_content_length - dst_header_length,	&reply[dst_header_length]);

				if(send_content_length >= 0)
				{
					state_entry->lseq += send_content_length;
					dst->psh = 1;
					dst->ack = 1;
					finish_segment(dst_header_length + send_content_length, src, dst, src_ipv4, dst_ipv4);
					return(dst_header_length + send_content_length);
				}
				else
				{
					state_entry->state = state_close;
					rv = empty_segment(state_entry, max_content_length, src, dst, src_ipv4, dst_ipv4, est_finack);
					state_entry->lseq++;
					state_entry->rseq++;
					return(rv);
				}
			}

			break;
		}

		//   remote shuts down
		//   <- syn/ack
		//   -> ack
		//   <- ack
		//   -> fin/ack		seq=remote		ack=local
		// * <- fin/ack		seq=local		ack=remote+1
		// * -> ack			seq=remote+1	ack=local+1

		//   we shut down
		//   <- syn/ack
		//   -> ack
		//   <- fin/ack		seq=local		ack=remote
		//   -> fin/ack		seq=remote		ack=local+1
		// * <- ack			seq=local+1		ack=remote+1
		// * -> nothing

		case(state_close):
		{
			if(src->ack && !src->fin)		// ACK on FIN/ACK, wait for FIN
				rv = 0;
			else
			{
				if(src->fin)				// FIN or FIN/ACK
					rv = empty_segment(state_entry, max_content_length, src, dst, src_ipv4, dst_ipv4, est_ack);
				else
					rv = empty_segment(0, 0, src, dst, src_ipv4, dst_ipv4, est_rst);

				reset_state(state_entry);
			}

			return(rv);
		}
	}

	reset_state(state_entry);
	return(empty_segment(0, 0, src, dst, src_ipv4, dst_ipv4, est_rst));
}
