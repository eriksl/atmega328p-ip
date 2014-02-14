#include "stats.h"

uint16_t eth_interrupts = 0;
uint16_t eth_pkt_rx = 0;
uint16_t eth_pkt_tx = 0;
uint16_t eth_txerr = 0;
uint16_t eth_rxerr = 0;
uint16_t eth_pkts_buffered = 0;

uint16_t ip_arp_pkt_in = 0;
uint16_t ip_arp_pkt_out = 0;
uint16_t ip_ipv4_pkt_in = 0;
uint16_t ip_ipv4_pkt_out = 0;
uint16_t ip_icmp4_pkt_in = 0;
uint16_t ip_icmp4_pkt_out = 0;
uint16_t ip_tcp4_pkt_in = 0;
uint16_t ip_tcp4_pkt_out = 0;
uint16_t ip_other_pkt = 0;
uint16_t ip_stray_pkt = 0;
uint16_t ip_bad_checksum = 0;

uint16_t wd_interrupts = 0;
