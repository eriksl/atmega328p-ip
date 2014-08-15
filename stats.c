#include "stats.h"
#include "util.h"

#include <stdio.h>

stats_t eth_interrupts = 0;
stats_t eth_pkt_rx = 0;
stats_t eth_pkt_tx = 0;
stats_t eth_txerr = 0;
stats_t eth_rxerr = 0;
stats_t eth_pkts_buffered = 0;

stats_t ip_arp_pkt_in = 0;
stats_t ip_arp_pkt_out = 0;
stats_t ip_ipv4_pkt_in = 0;
stats_t ip_ipv4_pkt_out = 0;
stats_t ip_icmp4_pkt_in = 0;
stats_t ip_icmp4_pkt_out = 0;
stats_t ip_tcp4_pkt_in = 0;
stats_t ip_tcp4_pkt_out = 0;
stats_t ip_udp4_pkt_in = 0;
stats_t ip_udp4_pkt_out = 0;
stats_t ip_other_pkt = 0;
stats_t ip_stray_pkt = 0;
stats_t ip_bad_checksum = 0;

stats_t wd_interrupts = 0;

static const __flash char format_string[] =
{
	"int wd: %d\n"
	"int eth: %d\n"
	"pkt rx: %d\n"
	"pkt tx: %d\n"
	"err rx: %d\n"
	"err tx: %d\n"
	"buf rx: %d\n"
	"arp: %d/%d\n"
	"ipv4: %d/%d\n"
	"icmp4: %d/%d\n"
	"udp4: %d/%d\n"
	"tcp4: %d/%d\n"
	"other: %d\n"
	"stray: %d\n"
	"c/s err: %d\n"
};

void stats_generate(uint16_t size, uint8_t *dst)
{
	snprintf_P((char *)dst, (size_t)size, (const char *)format_string,
			wd_interrupts,
			eth_interrupts,
			eth_pkt_rx,
			eth_pkt_tx,
			eth_rxerr,
			eth_txerr,
			eth_pkts_buffered,
			ip_arp_pkt_in, ip_arp_pkt_out,
			ip_ipv4_pkt_in, ip_ipv4_pkt_out,
			ip_icmp4_pkt_in, ip_icmp4_pkt_out,
			ip_udp4_pkt_in, ip_udp4_pkt_out,
			ip_tcp4_pkt_in, ip_tcp4_pkt_out,
			ip_other_pkt,
			ip_stray_pkt,
			ip_bad_checksum);
}
