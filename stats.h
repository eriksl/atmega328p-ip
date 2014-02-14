#ifndef _stats_h_
#define _stats_h_

#include <stdint.h>

extern uint16_t eth_interrupts;
extern uint16_t eth_pkt_rx;
extern uint16_t eth_pkt_tx;
extern uint16_t eth_txerr;
extern uint16_t eth_rxerr;
extern uint16_t eth_pkts_buffered;

extern uint16_t ip_arp_pkt_in;
extern uint16_t ip_arp_pkt_out;
extern uint16_t ip_ipv4_pkt_in;
extern uint16_t ip_ipv4_pkt_out;
extern uint16_t ip_icmp4_pkt_in;
extern uint16_t ip_icmp4_pkt_out;
extern uint16_t ip_tcp4_pkt_in;
extern uint16_t ip_tcp4_pkt_out;
extern uint16_t ip_other_pkt;
extern uint16_t ip_stray_pkt;
extern uint16_t ip_bad_checksum;

extern uint16_t wd_interrupts;

#endif
