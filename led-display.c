#include "led-display.h"

#include "twi_master.h"

/*
	+--1--+
	|     |
	2     0
	|     |
	+--3--+
	|     |
	7     5
	|     |
	+--6--+ +4
			++

	0	0x01
	1	0x02
	2	0x04
	3	0x08
	4	0x10
	5	0x20
	6	0x40
	7	0x80

*/

static const uint8_t charrom[] =
{
	0x00,		/*	32	SPC	*/
	0x31,		/*	33	!	*/
	0x05,		/*	34	"	*/
	0x00,		/*	35	#	*/
	0x6e,		/*	36	$	*/
	0x24,		/*	37	%	*/
	0x29,		/*	38	&	*/
	0x0f,		/*	39	´	*/
	0xc6,		/*	40	(	*/
	0x63,		/*	41	)	*/
	0x8c,		/*	42	*	*/
	0x29,		/*	43	+	*/
	0x10,		/*	44	,	*/
	0x08,		/*	45	-	*/
	0x10,		/*	46	.	*/
	0x89,		/*	47	/	*/
	0xe7,		/*	48	0	*/
	0x21,		/*	49	1	*/
	0xcb,		/*	50	2	*/
	0x6b,		/*	51	3	*/
	0x2d,		/*	52	4	*/
	0x6e,		/*	53	5	*/
	0xee,		/*	54	6	*/
	0x23,		/*	55	7	*/
	0xef,		/*	56	8	*/
	0x6f,		/*	57	9	*/
	0x21,		/*	58	:	*/
	0x61,		/*	59	;	*/
	0xc6,		/*	60	<	*/
	0x48,		/*	61	=	*/
	0x63,		/*	62	>	*/
	0x33,		/*	63	?	*/
	0xff,		/*	64	@	*/
	0xeb,		/*	65	A	*/
	0xec,		/*	66	B	*/
	0xc8,		/*	67	C	*/
	0xe9,		/*	68	D	*/
	0xcf,		/*	69	E	*/
	0x8e,		/*	70	F	*/
	0x6f,		/*	71	G	*/
	0xac,		/*	72	H	*/
	0x21,		/*	73	I	*/
	0x61,		/*	74	J	*/
	0x8c,		/*	75	K	*/
	0xc4,		/*	76	L	*/
	0xaa,		/*	77	M	*/
	0xa8,		/*	78	N	*/
	0xe8,		/*	79	O	*/
	0x8f,		/*	80	P	*/
	0x2f,		/*	81	Q	*/
	0x88,		/*	82	R	*/
	0x6e,		/*	83	S	*/
	0xcc,		/*	84	T	*/
	0xe5,		/*	85	U	*/
	0xe5,		/*	86	V	*/
	0xed,		/*	87	W	*/
	0xad,		/*	88	X	*/
	0x6d,		/*	89	Y	*/
	0xcb,		/*	90	Z	*/
	0xc6,		/*	91	[	*/
	0x2c,		/*	92	\	*/
	0x63,		/*	93	]	*/
	0x02,		/*	94	^	*/
	0x40,		/*	95	_	*/
};

static uint8_t brightness = 1;

static uint8_t render_char(uint8_t character)
{
	uint8_t add_dot = 0;

	if(character & 0x80)
	{
		add_dot = 0x10;
		character &= ~0x80;
	}

	if(character & 0x40)
		character &= ~0x20;	// maps CAPS characters to lowercase characters

	if(character > 0x60)
		return(0x10);

	if(character < 0x20)
		return(0x10);

	character -= 0x20;		// skip control characters 0x00 - 0x20

	if(character >= sizeof(charrom))
		return(0xff);		// this should never happen

	return(charrom[character] | add_dot);
}

uint8_t display_show(const uint8_t *text)
{
	uint8_t ix, twierror;
	uint8_t twistring[6];

	twistring[0] =	0x00;	// start at control register (0x00),
							// followed by four digits segments registers (0x01-0x04)
	twistring[1] = 0x07;	// multiplex mode, enable all digits, no test mode
	twistring[1] |= brightness << 4;

	for(ix = 0; ix < 4; ix++)
		twistring[2 + ix] = 0x00;

	for(ix = 0; (ix < 4) && text[ix]; ix++)
		twistring[5 - ix] = render_char(text[ix]); // reverse digit's position

	if((twierror = twi_master_send(0x38, 6, twistring)) != tme_ok)
		return(twierror);

	return(0);
}

void display_brightness(uint8_t level)
{
	if(level > 7)
		level = 7;

	brightness = level;
}
