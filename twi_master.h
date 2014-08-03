#ifndef _twi_master_h_
#define _twi_master_h_
#include <stdint.h>

typedef enum
{
	tms_start_sent		= 0x01,		//	0x08
	tms_re_start_sent	= 0x02,		//	0x10
	tms_addr_w_ack		= 0x03,		//	0x18
	tms_addr_w_nack		= 0x04,		//	0x20
	tms_data_sent_ack	= 0x05,		//	0x28
	tms_data_send_nack	= 0x06,		//	0x30
	tms_arbitration_lost= 0x07,		//	0x38
	tms_addr_r_ack		= 0x08,		//	0x40
	tms_addr_r_nack		= 0x09,		//	0x48
	tms_data_recvd_ack	= 0x0a,		//	0x50
	tms_data_recvd_nack	= 0x0b,		//	0x58
} twi_master_state;

typedef enum
{
	tme_ok				= 0x00,
	tme_send_start 		= 0x01,
	tme_send_address_w	= 0x02,
	tme_send_address_r	= 0x03,
	tme_send_byte		= 0x04,
	tme_receive_byte	= 0x05,
	tme_send_nack		= 0x06,
} twi_master_error_t;

void	twi_master_init(void);
void	twi_master_recover(void);
uint8_t	twi_master_send(uint8_t address, uint8_t length, const uint8_t *buffer);
uint8_t twi_master_receive(uint8_t address, uint8_t size, uint8_t *buffer);

#endif
