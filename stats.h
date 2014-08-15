#ifndef _stats_h_
#define _stats_h_

#include <stdint.h>

typedef uint16_t stats_t;

extern stats_t eth_interrupts;
extern stats_t eth_pkt_rx;
extern stats_t eth_pkt_tx;
extern stats_t eth_txerr;
extern stats_t eth_rxerr;
extern stats_t eth_pkts_buffered;

extern stats_t ip_arp_pkt_in;
extern stats_t ip_arp_pkt_out;
extern stats_t ip_ipv4_pkt_in;
extern stats_t ip_ipv4_pkt_out;
extern stats_t ip_icmp4_pkt_in;
extern stats_t ip_icmp4_pkt_out;
extern stats_t ip_tcp4_pkt_in;
extern stats_t ip_tcp4_pkt_out;
extern stats_t ip_udp4_pkt_in;
extern stats_t ip_udp4_pkt_out;
extern stats_t ip_other_pkt;
extern stats_t ip_stray_pkt;
extern stats_t ip_bad_checksum;

extern stats_t wd_interrupts;

void stats_generate(uint16_t size, uint8_t *dst);

#endif
