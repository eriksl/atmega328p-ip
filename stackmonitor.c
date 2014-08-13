#include "stackmonitor.h"

#include <stdint.h>

/* credit: MichaelMcTernan http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=52249 */

static void __attribute__ ((used)) __attribute__ ((naked)) __attribute ((section(".init1"))) stackmonitor_paint_stack(void)
{
	__asm volatile(""
		"		ldi		r30,lo8(_end)\n"	/* Ylo */
		"		ldi		r31,hi8(_end)\n"	/* Yhi */
		"		ldi		r24,lo8(0xfc)\n" /* STACK CANARY = 0xfc */
		"		ldi		r25,hi8(__stack)\n"
		"		rjmp	.cmp\n"
		".loop:\n"
		"		st		Z+,r24\n"			/* r30/r31 */
		".cmp:\n"
		"		cpi		r30,lo8(__stack)\n"	/* r30 = Ylo */
		"		cpc		r31,r25\n"			/* r31 = Yhi, r25 = hi8(__stack) */
		"		brlo	.loop\n"
		"		breq	.loop" ::);
}

uint16_t stackmonitor_free(void)
{
	extern const uint8_t _end;
	extern const uint8_t __stack;

	const	uint8_t 	*cur = &_end;
			uint16_t	free = 0;

	while((*cur == 0xfc /* canary */) && (cur <= &__stack))
	{
		cur++;
		free++;
	}

	return(free);
}
