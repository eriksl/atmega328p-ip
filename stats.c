#include "stats.h"
#include "util.h"

#include <stdio.h>

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
stats_t eth_interrupts = 0;
stats_t adc_interrupts = 0;
stats_t t1_interrupts = 0;
stats_t t1_unhandled = 0;
stats_t t1_unhandled_max = 0;

static const __flash char format_string[] =
{
	"int adc: %u\n"
	"int wd: %u\n"
	"int eth: %u\n"
	"int t1: %u\n"
	"int t1um: %u\n"
	"pkt rx: %u\n"
	"pkt tx: %u\n"
	"err rx: %u\n"
	"err tx: %u\n"
	"buf rx: %u\n"
	"arp: %u/%u\n"
	"ipv4: %u/%u\n"
	"icmp4: %u/%u\n"
	"udp4: %u/%u\n"
	"tcp4: %u/%u\n"
	"other: %u\n"
	"stray: %u\n"
	"c/s err: %u\n"
};

void stats_generate(uint16_t size, uint8_t *dst)
{
	snprintf_P((char *)dst, (size_t)size, (const char *)format_string,
			adc_interrupts,
			wd_interrupts,
			eth_interrupts,
			t1_interrupts,
			t1_unhandled_max,
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
