#include "stats.h"
#include "util.h"

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

void stats_generate(uint16_t size, uint8_t *dst)
{
	static uint8_t conv[8];

	xstrncat((uint8_t *)"wd int: ", size, dst);
	int_to_str(wd_interrupts, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"e int: ", size, dst);
	int_to_str(eth_interrupts, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"e rx: ", size, dst);
	int_to_str(eth_pkt_rx, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"e tx: ", size, dst);
	int_to_str(eth_pkt_tx, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"e rxerr: ", size, dst);
	int_to_str(eth_rxerr, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"e txerr: ", size, dst);
	int_to_str(eth_txerr, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"e rxbuf: ", size, dst);
	int_to_str(eth_pkts_buffered, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"arp: ", size, dst);
	int_to_str(ip_arp_pkt_in, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"/", size, dst);
	int_to_str(ip_arp_pkt_out, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"ipv4: ", size, dst);
	int_to_str(ip_ipv4_pkt_in, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"/", size, dst);
	int_to_str(ip_ipv4_pkt_out, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"icmp4: ", size, dst);
	int_to_str(ip_icmp4_pkt_in, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"/", size, dst);
	int_to_str(ip_icmp4_pkt_out, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"udp4: ", size, dst);
	int_to_str(ip_udp4_pkt_in, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"/", size, dst);
	int_to_str(ip_udp4_pkt_out, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"tcp4: ", size, dst);
	int_to_str(ip_tcp4_pkt_in, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"/", size, dst);
	int_to_str(ip_tcp4_pkt_out, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"other: ", size, dst);
	int_to_str(ip_other_pkt, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);

	xstrncat((uint8_t *)"stray: ", size, dst);
	int_to_str(ip_stray_pkt, sizeof(conv), conv);
	xstrncat(conv, size, dst);
	xstrncat((uint8_t *)"\n", size, dst);
}
