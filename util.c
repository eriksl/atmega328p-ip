#include "util.h"

void int_to_str(uint16_t in, uint8_t outlen, uint8_t *out)
{
	static uint16_t divisor;
	static uint8_t current;
	static uint8_t leading;

	divisor = 10000;
	leading = 1;

	while((outlen > 1) && (divisor > 0))
	{
		current = in / divisor;

		if((current > 0) || !leading)
		{
			leading = 0;
			*out++ = '0' + current;
			outlen--;
			in -= current * divisor;
		}
		
		divisor /= 10;
	}

	if(leading && (outlen > 1))
		*out++ = '0';

	*out = 0;
}

void xstrncat(const uint8_t *in, uint16_t outlen, uint8_t *out)
{
	const uint8_t *from;
	uint8_t *to;

	for(to = out; (outlen > 1) && *to; outlen--, to++)
		(void)0;

	for(from = in; (outlen > 1) && *from; outlen--, from++, to++)
		*to = *from;

	*to = 0;
}

uint16_t xstrlen(const uint8_t *str)
{
	static uint16_t length;
	
	length = 0;

	for(;;)
	{
		if(!*str)
			break;

		str++;
		length++;
	}

	return(length);
}
