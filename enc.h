#ifndef _enc_h_
#define _enc_h_
#include <stdint.h>

#include "ethernet.h"

#define PHLCON_LED_RES0		0x00
#define	PHLCON_LED_XMIT		0x01
#define	PHLCON_LED_RCV		0x02
#define	PHLCON_LED_COL		0x03
#define	PHLCON_LED_LNK		0x04
#define	PHLCON_LED_DPX		0x05
#define PHLCON_LED_RES1		0x06
#define PHLCON_LED_XMRV		0x07
#define PHLCON_LED_ON		0x08
#define PHLCON_LED_OFF		0x09
#define PHLCON_LED_BFAST	0x0a
#define PHLCON_LED_BSLOW	0x0b
#define PHLCON_LED_LNRC		0x0c
#define PHLCON_LED_LNXR		0x0d
#define PHLCON_LED_DXCL		0x0e
#define PHLCON_LED_RES2		0x0f

#define PHLCON_STRCH		1
#define PHLCON_LFRQ0		2
#define PHLCON_LFRQ1		3
#define PHLCON_LEDB			4
#define	PHLCON_LEDA			8

void		enc_init(uint16_t max_frame_size, const mac_addr_t *mac);
void		enc_set_led(uint8_t how1, uint8_t how2);
uint16_t	enc_receive_frame(uint8_t *frame, uint16_t buffer_length);
void		enc_send_frame(const uint8_t *frame, uint16_t length);
#endif
