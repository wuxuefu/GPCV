/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2014 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park                       *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file    gp_usb_color_cal_cmd.c
 * @brief   Implement of GP vendor command for GP's color calibration.
 * @author
 */
 
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/freezer.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/gp_usb_vendor.h>

#include <mach/gp_cache.h>

#include "s_file_storage.h"
#include "gp_usb_color_cal_cmd.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/
unsigned int g_bit_lfsr[32];
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/
extern void start_transfer(struct fsg_dev *, struct usb_ep *,
		struct usb_request *, int *,
		enum fsg_buffer_state *);
		
extern int sleep_thread(struct fsg_dev *);

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

/**************************************************************************
*                         G L O B A L    D A T A                         *
**************************************************************************/
u8 SCSIInquiryDataForColorCal[COLOR_CAL_SCSI_INQUIRY_LEN] =
{
	0x00,
	0x80,
	0x00,
	0x00,
	0x1F,
	0x00,
	0x00,
	0x00,
	'G','E','N','P','L','U','S',
	0x20,
	'U','S','B','-','M','S','D','C',' ','D','I','S','K',
	' ',
	'A',
	0x20,
	'1','.','0','0','G','P','-','P','R','O','D','.',
};
/**************************************************************************
*             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/
static void scram_init(unsigned int value)
{
	unsigned int i;
	
	for (i=0; i<32; i++) {
 		g_bit_lfsr[i] = (value>> i) & 1;
 	}
}

static void scram_lfsr(void)
{
	unsigned int i;
	unsigned int bit_scram[32];
	
	// Now advance the LFSR 1 serial clocks
	bit_scram[ 0] = g_bit_lfsr[31];
	bit_scram[ 1] = g_bit_lfsr[ 0];
	bit_scram[ 2] = g_bit_lfsr[ 1];
	bit_scram[ 3] = g_bit_lfsr[ 2]^g_bit_lfsr[31];
	bit_scram[ 4] = g_bit_lfsr[ 3]^g_bit_lfsr[31];
	bit_scram[ 5] = g_bit_lfsr[ 4]^g_bit_lfsr[31];
	bit_scram[ 6] = g_bit_lfsr[ 5];
	bit_scram[ 7] = g_bit_lfsr[ 6];
	bit_scram[ 8] = g_bit_lfsr[ 7];
	bit_scram[ 9] = g_bit_lfsr[ 8];
	bit_scram[10] = g_bit_lfsr[ 9];
	bit_scram[11] = g_bit_lfsr[10];
	bit_scram[12] = g_bit_lfsr[11];
	bit_scram[13] = g_bit_lfsr[12];
	bit_scram[14] = g_bit_lfsr[13];
	bit_scram[15] = g_bit_lfsr[14];
	bit_scram[16] = g_bit_lfsr[15];
	bit_scram[17] = g_bit_lfsr[16]^g_bit_lfsr[31];
	bit_scram[18] = g_bit_lfsr[17]^g_bit_lfsr[31];
	bit_scram[19] = g_bit_lfsr[18];
	bit_scram[20] = g_bit_lfsr[19];
	bit_scram[21] = g_bit_lfsr[20];
	bit_scram[22] = g_bit_lfsr[21];
	bit_scram[23] = g_bit_lfsr[22];
	bit_scram[24] = g_bit_lfsr[23];
	bit_scram[25] = g_bit_lfsr[24];
	bit_scram[26] = g_bit_lfsr[25];
	bit_scram[27] = g_bit_lfsr[26];
	bit_scram[28] = g_bit_lfsr[27];
	bit_scram[29] = g_bit_lfsr[28];
	bit_scram[30] = g_bit_lfsr[29];
	bit_scram[31] = g_bit_lfsr[30];

	for (i=0; i<32; i++) {
		g_bit_lfsr[i] = bit_scram[i];
	}
}

static unsigned int scram_checkval(unsigned int val)
{
	unsigned int i, j;
	unsigned int bit_tmp[32], bit_out[32];
	unsigned int tmpval;
	
	
	for (i=0; i<32; i++) {	//convert val to bit array for legibility
		bit_tmp[i] = (val >> i) & 1;
	}
	
	for (i=0; i<32; i++) {	//value scramble
		scram_lfsr();
		bit_tmp[i] = bit_tmp[i]^g_bit_lfsr[31];
	}
	
	j = 0;
	for (i=0; i<32; i=i+2) {
		bit_out[i] = bit_tmp[31-j];
		bit_out[i+1] = bit_tmp[15-j];
		j++;
	}
	
	tmpval = 0;
	for(i=0; i<32; i++) {	// convert data back to unsigned long
		tmpval = tmpval + (bit_out[i] << i);
	}
	return tmpval;
}

static void _do_show_color(int value)
{
	/* value is RGB565  */
	printk("_do_show_color: RGB value = 0x%x\n", value);
}	

static void _do_reg_read(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	u32 address;
	u32 length;
	u8 *buf = (u8*)bh->buf;
	
	address = (fsg->cmnd[2] << 24) | (fsg->cmnd[3] << 16) | (fsg->cmnd[4] << 8) | (fsg->cmnd[5]);
	length = (fsg->cmnd[7] << 8) | (fsg->cmnd[8]);
	
	/* Get the specified register address from GP12B then fill the value to buf */
	
	buf[0] = 0xAA;
	buf[1] = 0xBB;
	buf[2] = 0xCC;
	buf[3] = 0xDD;
	
	printk("_do_reg_read: address 0x%x\n", address);
}	

static void _do_reg_write(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	u32 address;
	u32 length;
	u32 regdata;
	int	rc = 0;
	u8* buf;
	
	address = (fsg->cmnd[2] << 24) | (fsg->cmnd[3] << 16) | (fsg->cmnd[4] << 8) | (fsg->cmnd[5]);
	length = (fsg->cmnd[7] << 8) | (fsg->cmnd[8]);
	
	bh->bulk_out_intended_length = length;
	bh->outreq->length = length;
	bh->outreq->short_not_ok = 1;
	
	start_transfer(fsg, fsg->bulk_out, bh->outreq,
				&bh->outreq_busy, &bh->state);

	/* We will drain the buffer in software, which means we
	 * can reuse it for the next filling.  No need to advance
	 * next_buffhd_to_fill. */

	/* Wait for the CBW to arrive */
	while (bh->state != BUF_STATE_FULL ) {
		rc = sleep_thread(fsg);
		if (rc) {
			printk("[%s][%d][%d], state[%d]\n", __FUNCTION__, __LINE__, fsg->cbbuf_cmnd_size, bh->state);
			usb_ep_dequeue(fsg->bulk_out, bh->outreq);
			bh->outreq_busy = 0;
			bh->state = BUF_STATE_EMPTY;
			return rc;
		}
	}
	
	buf = bh->outreq->buf;
	
	regdata = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]);
	
	printk("_do_reg_write: write data 0x%x to address 0x%x\n", regdata, address);
	
	/* reset bh state to empty for CSW*/
	bh->state = BUF_STATE_EMPTY;
}	

static unsigned int _get_cmd_key_val(struct fsg_dev *fsg)
{
	unsigned int value = 0, i;
	
	for(i=0; i<4; i++)
	{
		value |= (fsg->cmnd[5-i] << i*8);
	}
	
	return value;
}

int is_spyder_vendor_cmd(int cmd)
{
	if(cmd == CMD_SHOW_COLOR || cmd == CMD_KEY_INIT || cmd == CMD_KEY_PROTECT || cmd == CMD_UI_START || cmd == CMD_REG_READ 
		|| cmd == CMD_REG_WRITE)
		return 1;
	else
		return 0;		
}

int do_gp_color_cal(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	u8	*buf = (u8 *) bh->buf;
	unsigned int check_value;
	int ret = 0;
	int cmd = (fsg->cmnd[0] << 8 | fsg->cmnd[1]);
	
	
	switch(cmd)
	{
		case CMD_SHOW_COLOR:
			_do_show_color(fsg->cmnd[2] << 8 | fsg->cmnd[3]);
			//printk("do_gp_color_cal: CMD_SHOW_COLOR, color value 0x%x\n", fsg->cmnd[2] << 8 | fsg->cmnd[3]); 
			break;
		
		case CMD_KEY_INIT:
			scram_init(_get_cmd_key_val(fsg));
			printk("do_gp_color_cal: CMD_KEY_INIT\n"); 	
			break;
		
		case CMD_KEY_PROTECT:
			check_value = scram_checkval(_get_cmd_key_val(fsg));
			buf[0] = (u8)(check_value >> 24);
			buf[1] = (u8)(check_value >> 16);
			buf[2] = (u8)(check_value >> 8);
			buf[3] = (u8)(check_value);
			ret = 4;
			printk("do_gp_color_cal: CMD_KEY_PROTECT, check_value 0x%x\n", check_value); 	
			break;
		
		case CMD_UI_START:
			/* Nothing to do here */
			printk("do_gp_color_cal: CMD_UI_START\r\n"); 	
			break;
		
		case CMD_REG_READ:
			_do_reg_read(fsg, bh);
			ret = 4;
			//printk("do_gp_color_cal: CMD_REG_READ\r\n"); 	
			break;	
		
		case CMD_REG_WRITE:
			_do_reg_write(fsg, bh);
			//printk("do_gp_color_cal: CMD_REG_WRITE\r\n"); 	
			break;
		
		default:
			printk("do_gp_color_cal: unknown cmd 0x%x\n", cmd);
			break;							
	}	
	
	
	return ret;
}	