#ifndef _enc_private_h_
#define _enc_private_h_

#define _RCR		0x00
#define _RBM		0x3a
#define _WCR		0x40
#define _WBM		0x7a
#define _BFS		0x80
#define _BFC		0xa0
#define _PAD		0xc0
#define _SC			0xff

#define	REG_ETH		0x0000
#define REG_MAC		0x1000
#define REG_PHY		0x2000
#define REG_RES		0x3000

#define BANK0		0x0000
#define BANK1		0x0100
#define BANK2		0x0200
#define BANK3		0x0300

#define ERDPTL		(0x00 | BANK0 | REG_ETH)
#define ERDPTH		(0x01 | BANK0 | REG_ETH)
#define EWRPTL		(0x02 | BANK0 | REG_ETH)
#define EWRPTH		(0x03 | BANK0 | REG_ETH)
#define ETXSTL		(0x04 | BANK0 | REG_ETH)
#define ETXSTH		(0x05 | BANK0 | REG_ETH)
#define ETXNDL		(0x06 | BANK0 | REG_ETH)
#define ETXNDH		(0x07 | BANK0 | REG_ETH)
#define ERXSTL		(0x08 | BANK0 | REG_ETH)
#define ERXSTH		(0x09 | BANK0 | REG_ETH)
#define ERXNDL		(0x0a | BANK0 | REG_ETH)
#define ERXNDH		(0x0b | BANK0 | REG_ETH)
#define ERXRDPTL	(0x0c | BANK0 | REG_ETH)
#define ERXRDPTH	(0x0d | BANK0 | REG_ETH)
#define ERXWRPTL	(0x0e | BANK0 | REG_ETH)
#define ERXWRPTH	(0x0f | BANK0 | REG_ETH)
#define EDMASTL		(0x10 | BANK0 | REG_ETH)
#define EDMASTH		(0x11 | BANK0 | REG_ETH)
#define EDMANDL		(0x12 | BANK0 | REG_ETH)
#define EDMANDH		(0x13 | BANK0 | REG_ETH)
#define EDMADSTL	(0x14 | BANK0 | REG_ETH)
#define EDMADSTH	(0x15 | BANK0 | REG_ETH)
#define EDMACSL		(0x16 | BANK0 | REG_ETH)
#define EDMACSH		(0x17 | BANK0 | REG_ETH)
#define RES00		(0x18 | BANK0 | REG_RES)
#define RES01		(0x19 | BANK0 | REG_RES)
#define RES02		(0x1a | BANK0 | REG_RES)
#define EIE			(0x1b | BANK0 | REG_ETH)
#define EIR			(0x1c | BANK0 | REG_ETH)
#define ESTAT		(0x1d | BANK0 | REG_ETH)
#define ECON2		(0x1e | BANK0 | REG_ETH)
#define ECON1		(0x1f | BANK0 | REG_ETH)

#define EHT0		(0x00 | BANK1 | REG_ETH)
#define EHT1		(0x01 | BANK1 | REG_ETH)
#define EHT2		(0x02 | BANK1 | REG_ETH)
#define EHT3		(0x03 | BANK1 | REG_ETH)
#define EHT4		(0x04 | BANK1 | REG_ETH)
#define EHT5		(0x05 | BANK1 | REG_ETH)
#define EHT6		(0x06 | BANK1 | REG_ETH)
#define EHT7		(0x07 | BANK1 | REG_ETH)
#define EPMM0		(0x08 | BANK1 | REG_ETH)
#define EPMM1		(0x09 | BANK1 | REG_ETH)
#define EPMM2		(0x0a | BANK1 | REG_ETH)
#define EPMM3		(0x0b | BANK1 | REG_ETH)
#define EPMM4		(0x0c | BANK1 | REG_ETH)
#define EPMM5		(0x0d | BANK1 | REG_ETH)
#define EPMM6		(0x0e | BANK1 | REG_ETH)
#define EPMM7		(0x0f | BANK1 | REG_ETH)
#define EPMCSL		(0x10 | BANK1 | REG_ETH)
#define EPMCSH		(0x11 | BANK1 | REG_ETH)
#define RES10		(0x12 | BANK1 | REG_RES)
#define RES11		(0x13 | BANK1 | REG_RES)
#define EPMOL		(0x14 | BANK1 | REG_ETH)
#define EPMOH		(0x15 | BANK1 | REG_ETH)
#define EPWOLIE		(0x16 | BANK1 | REG_ETH)
#define EPWOLIR		(0x17 | BANK1 | REG_ETH)
#define ERXFCON		(0x18 | BANK1 | REG_ETH)
#define EPKTCNT		(0x19 | BANK1 | REG_ETH)
#define RES12		(0x1a | BANK1 | REG_RES)
#define EIE_1		(0x1b | BANK1 | REG_ETH)
#define EIR_1		(0x1c | BANK1 | REG_ETH)
#define ESTAT_1		(0x1d | BANK1 | REG_ETH)
#define ECON2_1		(0x1e | BANK1 | REG_ETH)
#define ECON1_1		(0x1f | BANK1 | REG_ETH)

#define MACON1		(0x00 | BANK2 | REG_MAC)
#define MACON2		(0x01 | BANK2 | REG_MAC)
#define	MACON3		(0x02 | BANK2 | REG_MAC)
#define	MACON4		(0x03 | BANK2 | REG_MAC)
#define MABBIPG		(0x04 | BANK2 | REG_MAC)
#define RES20		(0x05 | BANK2 | REG_RES)
#define MAIPGL		(0x06 | BANK2 | REG_MAC)
#define MAIPGH		(0x07 | BANK2 | REG_MAC)
#define MACLCON1	(0x08 | BANK2 | REG_MAC)
#define MACLCON2	(0x09 | BANK2 | REG_MAC)
#define MAMXFLL		(0x0a | BANK2 | REG_MAC)
#define MAMXFLH		(0x0b | BANK2 | REG_MAC)
#define RES21		(0x0c | BANK2 | REG_RES)
#define MAPHSUP		(0x0d | BANK2 | REG_MAC)
#define RES22		(0x0e | BANK2 | REG_RES)
#define RES23		(0x0f | BANK2 | REG_RES)
#define RES24		(0x10 | BANK2 | REG_RES)
#define MICON		(0x11 | BANK2 | REG_MAC)
#define MICMD		(0x12 | BANK2 | REG_MAC)
#define RES25		(0x13 | BANK2 | REG_RES)
#define MIREGADR	(0x14 | BANK2 | REG_MAC)
#define RES26		(0x15 | BANK2 | REG_RES)
#define MIWRL		(0x16 | BANK2 |	REG_MAC)
#define MIWRH		(0x17 | BANK2 |	REG_MAC)
#define MIRDL		(0x18 | BANK2 |	REG_MAC)
#define MIRDH		(0x19 | BANK2 |	REG_MAC)
#define RES27		(0x1a | BANK2 | REG_RES)
#define EIE_2		(0x1b | BANK2 | REG_ETH)
#define EIR_2		(0x1c | BANK2 | REG_ETH)
#define ESTAT_2		(0x1d | BANK2 | REG_ETH)
#define ECON2_2		(0x1e | BANK2 | REG_ETH)
#define ECON1_2		(0x1f | BANK2 | REG_ETH)

#define MAADR1		(0x00 | BANK3 | REG_MAC)
#define MAADR0		(0x01 | BANK3 | REG_MAC)
#define MAADR3		(0x02 | BANK3 | REG_MAC)
#define MAADR2		(0x03 | BANK3 | REG_MAC)
#define MAADR5		(0x04 | BANK3 | REG_MAC)
#define MAADR4		(0x05 | BANK3 | REG_MAC)
#define EBSTSD		(0x06 | BANK3 | REG_ETH)
#define EBSTCON		(0x07 | BANK3 | REG_ETH)
#define EBSTCSL		(0x08 | BANK3 | REG_ETH)
#define EBSTCSH		(0x09 | BANK3 | REG_ETH)
#define MISTAT		(0x0a | BANK3 |	REG_MAC)
#define RES30		(0x0b | BANK3 | REG_RES)
#define RES31		(0x0c | BANK3 | REG_RES)
#define RES32		(0x0d | BANK3 | REG_RES)
#define RES33		(0x0e | BANK3 | REG_RES)
#define RES34		(0x0f | BANK3 | REG_RES)
#define RES35		(0x10 | BANK3 | REG_RES)
#define RES36		(0x11 | BANK3 | REG_RES)
#define EREVID		(0x12 | BANK3 | REG_ETH)
#define RES37		(0x13 | BANK3 | REG_RES)
#define RES38		(0x14 | BANK3 | REG_RES)
#define ECOCON		(0x15 | BANK3 | REG_ETH)
#define RES3_9		(0x16 | BANK3 | REG_RES)
#define EFLOCON		(0x17 | BANK3 | REG_ETH)
#define EPAUSL		(0x18 | BANK3 | REG_ETH)
#define EPAUSH		(0x19 | BANK3 | REG_ETH)
#define RES3a		(0x1a | BANK3 | REG_RES)
#define EIE_3		(0x1b | BANK3 | REG_ETH)
#define EIR_3		(0x1c | BANK3 | REG_ETH)
#define ESTAT_3		(0x1d | BANK3 | REG_ETH)
#define ECON2_3		(0x1e | BANK3 | REG_ETH)
#define ECON1_3		(0x1f | BANK3 | REG_ETH)

#define PHCON1		(0x00 | REG_PHY)
#define PHSTAT1		(0x01 | REG_PHY)
#define PHID1		(0x02 | REG_PHY)
#define PHID2		(0x03 | REG_PHY)
#define PHCON2		(0x10 | REG_PHY)
#define PHSTAT2		(0x11 | REG_PHY)
#define PHIE		(0x12 | REG_PHY)
#define PHIR		(0x13 | REG_PHY)
#define PHLCON		(0x14 | REG_PHY)

#define TYPE_MASK	0x3000
#define BANK_MASK	0x0300
#define REG_MASK	0x001f

#define TYPE_SHIFT	12
#define BANK_SHIFT	8
#define REG_SHIFT	0

#define	RCR(s)		(_RCR | ((s) & REG_MASK))
#define RBM			(_RBM)
#define WCR(s)		(_WCR | ((s) & REG_MASK))
#define WBM			(_WBM)
#define BFS(s)		(_BFS | ((s) & REG_MASK))
#define BFC(s)		(_BFC | ((s) & REG_MASK))
#define SC			(_SC)
#define PAD			(_PAD)

#define	TYPE(s)		(((s) & TYPE_MASK) >> TYPE_SHIFT)
#define	BANK(s)		(((s) & BANK_MASK) >> BANK_SHIFT)
#define	REG(s)		(((s) & REG_MASK)  >> REG_SHIFT)

#define ECON1_BANK_MASK 0x03
#define ECON1_RXEN		2
#define ECON1_TXRTS		3
#define ECON1_CSUMEN	4
#define ECON1_DMAST		5
#define ECON1_RXRST		6
#define ECON1_TXRST		7

#define ECON2_VRPS		3
#define ECON2_PWRSV		5
#define ECON2_PKTDEC	6
#define	ECON2_AUTOINC	7

#define ERXFCON_BCEN	0
#define ERXFCON_MCEN	1
#define ERXFCON_HTEN	2
#define ERXFCON_MPEN	3
#define	ERXFCON_PMEN	4
#define	ERXFCON_CRCEN	5
#define	ERXFCON_ANDOR	6
#define	ERXFCON_UCEN	7

#define EIE_RXERIE		0
#define EIE_TXERIE		1
#define EIE_WOLIE		2
#define EIE_TXIE		3
#define EIE_LINKIE		4
#define EIE_DMAIE		5
#define EIE_PKTIE		6
#define EIE_INTIE		7

#define	EIR_RXERIF		0
#define EIR_TXERIF		1
#define EIR_WOLIF		2
#define EIR_TXIF		3
#define EIR_LINKIF		4
#define EIR_DMAIF		5
#define EIR_PKTIF		6

#define MICMD_MIIRD		0
#define MICOM_MIISCAN	1

#define	MISTAT_BUSY		0
#define	MISTAT_SCAN		1
#define MISTAT_NVALID	2

#define MACON1_MARXEN	0
#define MACON1_PASSALL	1
#define MACON1_RXPAUS	2
#define MACON1_TXPAUS	3
#define MACON1_LOOPBK	4

#define MACON2_TFUNRST	0
#define MACON2_MATXRST	1
#define MACON2_RFUNRST	2
#define MACON2_MARXRST	3
#define MACON2_RNDRST	6
#define MACON2_MARST	7

#define MACON3_FULDPX		0
#define MACON3_FRMLNEN		1
#define MACON3_HFRMEN		2
#define MACON3_PHDRLEN		3
#define	MACON3_TXCRCEN		4
#define MACON3_PADCFG		5
#define MACON3_PADCFG_NONE	0x00
#define MACON3_PADCFG_PAD60	0x01
#define MACON3_PADCFG_PAD64	0x03
#define MACON3_PADCFG_VLAN	0x05

#define	PHCON1_PDPXMD		8
#define	PHCON1_PPWRSV		11
#define PHCON1_PLOOPBK		14
#define PHCON1_PRST			15

#define PHCON2_HDLDIS		8
#define PHCON2_JABBER		10
#define PHCON2_TXDIS		13
#define PHCON2_FRCLINK		14

/*
 *	1fff
 *	1ffe	TXSTOP_INIT			TXBUFFER + TXBUFLEN - 1		ETXND
 *	...
 *  19ff	TXSTART_INIT		TXBUFFER					ETXST
 *  ...
 *  ...
 *	13fe	RXSTOP_INIT			RXBUFFER + RXBUFLEN - 1		ERXND
 *	...
 *	0000	RXSTART_INIT		RXBUFFER					ERXST
 */

#define TXBUFLEN	0x0600		// 1536
#define	TXBUFFER	0x19ff		// 6655

#define	RXBUFLEN	0x13ff		// 5119
#define	RXBUFFER	0x0000		// 0000
#endif
