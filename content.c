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
				length++; // room for null byte

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
				stats_generate(size, dst);
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
