#include "content.h"
#include "stats.h"
#include "util.h"

int16_t content(uint16_t port, uint16_t length, const uint8_t *src, uint16_t size, uint8_t *dst)
{
	static uint8_t conv[8];
	static uint8_t cmd;

	if(port == 28022)
	{
		dst[0] = 't';
		dst[1] = 'e';
		dst[2] = 's';
		dst[3] = 't';
		dst[4] = '\n';

		return(5);
	}
	else
	{
		if(size == 0)
			return(0);

		*dst = 0;

		if(length == 0)
			cmd = '?';
		else
			cmd = src[0];

		switch(cmd)
		{
			case('e'):
			{
				if(length > size)
					length = size;

				xstrncat(src, length, dst);

				break;
			}

			case('q'):
			{
				return(-1);
				break;
			}

			case('r'):
			{
				// let watchdog do it's job
				for(;;)
					(void)0;

				break;
			}

			case('s'):
			{
				xstrncat((uint8_t *)"wd int: ", size, dst);
				int_to_str(wd_interrupts, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"eth int: ", size, dst);
				int_to_str(eth_interrupts, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"eth rx: ", size, dst);
				int_to_str(eth_pkt_rx, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"eth tx: ", size, dst);
				int_to_str(eth_pkt_tx, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"eth rx err: ", size, dst);
				int_to_str(eth_rxerr, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"eth tx err: ", size, dst);
				int_to_str(eth_txerr, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"eth rx buf: ", size, dst);
				int_to_str(eth_pkts_buffered, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"arp: ", size, dst);
				int_to_str(ip_arp_pkt_in, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)" / ", size, dst);
				int_to_str(ip_arp_pkt_out, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"ipv4: ", size, dst);
				int_to_str(ip_ipv4_pkt_in, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)" / ", size, dst);
				int_to_str(ip_ipv4_pkt_out, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"icmp4: ", size, dst);
				int_to_str(ip_icmp4_pkt_in, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)" / ", size, dst);
				int_to_str(ip_icmp4_pkt_out, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)"\n", size, dst);

				xstrncat((uint8_t *)"tcp4: ", size, dst);
				int_to_str(ip_tcp4_pkt_in, sizeof(conv), conv);
				xstrncat(conv, size, dst);
				xstrncat((uint8_t *)" / ", size, dst);
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

				break;
			}

			default:
			{
				xstrncat((uint8_t *)"command:\n\n  e)echo\n\n  ?/h)help\n  q)uit\n  r)eset\n  s)tats\n", size, dst);
				break;
			}
		}
	}

	return((int16_t)xstrlen(dst));
}
