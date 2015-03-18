/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Inc.                         *
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
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file    gp_usb_vendor.c
 * @brief   Implement of GP vendor command for File-backed Storage Gadget (FSG).
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

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

static struct usb_vendor_ops *gp_vendor[GP_VENDOR_CMD_NUM] = {0};
spinlock_t usb_vendor_lock = SPIN_LOCK_UNLOCKED;

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

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

/**************************************************************************
*             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/
/**
* @brief 	Set GP vendor command pointer function (Lock protect). 
* @param	cmd[in]: Vendor command number. 0~511
* @param 	ops[in]: Vendor command operation function pointer.
* @return 	0 for success, others number means fail.
*/ 
static int gp_usb_vendor_set(unsigned int cmd, struct usb_vendor_ops *ops)
{
	unsigned long flags;
	int ret = 0;
	spin_lock_irqsave(&usb_vendor_lock, flags);
	if(gp_vendor[cmd]&&ops)
		ret = -EEXIST;
	else
	{
		gp_vendor[cmd] = ops;
	}
	spin_unlock_irqrestore(&usb_vendor_lock, flags);
	return ret;
}

/**
* @brief 	Get GP vendor command pointer function (Lock protect). 
* @param	cmd[in]: Vendor command number. 0~511
* @return 	Operation function pointer.
*/ 
static struct usb_vendor_ops *gp_usb_vendor_get(unsigned int cmd)
{
	unsigned long flags;
	struct usb_vendor_ops* ops;
	
	spin_lock_irqsave(&usb_vendor_lock, flags);
	ops = gp_vendor[cmd];
	spin_unlock_irqrestore(&usb_vendor_lock, flags);
	return ops;
}

/**
* @brief 	Register GP vendor command function.
* @param	cmd[in]: Vendor command number. 0~511
* @param 	ops[in]: Vendor command operation function pointer.
* @return 	0 for success, others number means fail.
*/ 
int gp_usb_register_vendor(unsigned int cmd, struct usb_vendor_ops *ops)
{
	int ret = 0;
	if((cmd<GP_VENDOR_CMD_NUM)&&ops->get_scsi&&ops->set_csw)
	{
		ret = gp_usb_vendor_set(cmd,ops);
	}
	else
		ret = -EINVAL;
	return ret;	
}
EXPORT_SYMBOL(gp_usb_register_vendor);

/**
* @brief 	Unregister GP vendor command function.
* @param	cmd[in]: Vendor command number. 0~511
* @return 	None.
*/ 
void gp_usb_unregister_vendor(unsigned int cmd)
{
	gp_usb_vendor_set(cmd,0);
}
EXPORT_SYMBOL(gp_usb_unregister_vendor);

/**
* @brief 	List all vendor command.
* @return 	None.
*/ 
void gp_usb_vendor_list(void)
{
	char *non = "No Name";
	unsigned int i;
	/* ----- Print all vendor command ----- */
	for(i=0;i<GP_VENDOR_CMD_NUM;i++)
	{
		if(gp_vendor[i])
			printk("USB Vendor Cmd: 0x%x, name = %s\n", i, (gp_vendor[i]->name)? gp_vendor[i]->name:non);
	}
}
EXPORT_SYMBOL(gp_usb_vendor_list);

/**
* @brief 	Check vendor command function.
* @param	fsg[in]: fsg device struct pointer.
* @return 	0 for success, others number means fail.
*/ 
int gp_usb_vendor_check_cmd(struct fsg_dev *fsg)
{
	int ret = 0;
	unsigned int cmd = (fsg->cmnd[0]==0xf6)?fsg->cmnd[1]+256:fsg->cmnd[1];
	struct usb_vendor_ops   *ops =  gp_usb_vendor_get(cmd);
	
	if(ops)
	{
		if(ops->get_scsi(ops->priv, &fsg->cmnd, fsg->data_dir, fsg->data_size ,&fsg->data_size_from_cmnd)<0)
		{	
			ret = ops->set_csw(ops->priv, &fsg->curlun->sense_data);
			if(ret==2)
				fsg->phase_error = 1;
			if(ret!=0)
				ret = -EINVAL;
		}
		fsg->residue = fsg->usb_amount_left = fsg->data_size;
	}
	else
		ret = -ESRCH;
	return ret;
}

/**
* @brief 	Vendor read function.
* @param	fsg[in]: fsg device struct pointer.
* @return 	0 for success, others number means fail.
*/ 
static int do_gp_vendor_read(struct fsg_dev *fsg)
{
	struct lun				*curlun = fsg->curlun;
	unsigned int			amount;
	struct fsg_buffhd		*bh;
	u32						amount_left;
	int						rc;
	ssize_t					nread;
	unsigned int cmd = (fsg->cmnd[0]==0xf6)?fsg->cmnd[1]+256:fsg->cmnd[1];
	struct usb_vendor_ops   *ops = gp_usb_vendor_get(cmd);
	
	/* Carry out the file reads */
	amount_left = fsg->data_size_from_cmnd;
	if (unlikely(amount_left == 0))
		return -EIO;		// No default reply
	
	while(1)
	{
		amount = min(amount_left, mod_data.buflen);
		if(amount>512&&amount&0x1ff)
			amount -= (amount&0x1ff);
		/* Wait for the next buffer to become available */
		bh = fsg->next_buffhd_to_fill;
		while (bh->state != BUF_STATE_EMPTY) 
		{
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		/* Perform the read */
		if(ops&&ops->read)
			nread = ops->read(ops->priv, (char __user *) bh->buf, amount);
		else
			nread = -EINVAL;
		
		if (signal_pending(current))
			return -EINTR;

		if (nread < 0) 
		{
			LDBG(curlun, "error in vendor read: %d\n",(int) nread);
			nread = 0;
		} 
		else if (nread < amount) 
		{
			LDBG(curlun, "partial vendor read: %d/%u\n",(int) nread, amount);
			nread -= (nread & 511);	// Round down to a block
		}
		
		amount_left  -= nread;
		fsg->residue -= nread;
		bh->inreq->length = nread;
		bh->state = BUF_STATE_FULL;
		/* Force write cache data to ram, only for write back mode */
		gp_clean_dcache_range((unsigned long)bh->inreq->buf, amount);
		/* If an error occurred, report it and its position */
		if (nread < amount) {
			curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
			curlun->sense_data_info = 0;
			curlun->info_valid = 0;
			break;
		}

		if (amount_left == 0)
			break;		// No more left to read

		/* Send this buffer and go read some more */
		bh->inreq->zero = 0;

		start_transfer(fsg, fsg->bulk_in, bh->inreq,
				&bh->inreq_busy, &bh->state); 
		fsg->next_buffhd_to_fill = bh->next;
	}

	return -EIO;		// No default reply
}

/**
* @brief 	Vendor write function.
* @param	fsg[in]: fsg device struct pointer.
* @return 	0 for success, others number means fail.
*/ 
static int do_gp_vendor_write(struct fsg_dev *fsg)
{
	struct lun				*curlun = fsg->curlun;
	struct fsg_buffhd		*bh;
	int						get_some_more;
	u32						amount_left_to_req, amount_left_to_write;
	unsigned int			amount;
	ssize_t					nwritten;
	int						rc;
	unsigned int cmd = (fsg->cmnd[0]==0xf6)?fsg->cmnd[1]+256:fsg->cmnd[1];
	struct usb_vendor_ops   *ops = gp_usb_vendor_get(cmd);
	
	/* Carry out the file writes */
	get_some_more = 1;
	amount_left_to_req = amount_left_to_write = fsg->data_size_from_cmnd;

	while (amount_left_to_write > 0) 
	{
		//printk("check 2\n");
		/* Queue a request for more data from the host */
		bh = fsg->next_buffhd_to_fill;
		if (bh->state == BUF_STATE_EMPTY && get_some_more) 
		{
			
			amount = min(amount_left_to_req, mod_data.buflen);
			//printk("check 3\n");
			if (amount == 0) 
			{
				get_some_more = 0;
				curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
				curlun->sense_data_info = 0;
				curlun->info_valid = 0;
				continue;
			}
			//printk("check 3-1\n");
			//amount -= (amount & 511);
			if(amount>512&&amount&0x1ff)
				amount -= (amount&0x1ff);
			if (amount == 0) 
			{
				printk("amount == 0\n");
				/* Why were we were asked to transfer a
				 * partial block? */
				get_some_more = 0;
				continue;
			}

			/* Get the next buffer */
			//usb_offset += amount;
			//printk("check 3-2\n");
			fsg->usb_amount_left -= amount;
			amount_left_to_req -= amount;
			if (amount_left_to_req == 0)
				get_some_more = 0;
			/* amount is always divisible by 512, hence by
			 * the bulk-out maxpacket size */
			bh->outreq->length = bh->bulk_out_intended_length = amount;
			bh->outreq->short_not_ok = 1;

			start_transfer(fsg, fsg->bulk_out, bh->outreq, &bh->outreq_busy, &bh->state);
			/* invalid data cache */
			gp_invalidate_dcache_range((unsigned long)bh->outreq->buf, amount);
			//printk("check 3-3\n");
			fsg->next_buffhd_to_fill = bh->next;
			continue;
		}
		/* Write the received data to the backing file */
		bh = fsg->next_buffhd_to_drain;
		if (bh->state == BUF_STATE_EMPTY && !get_some_more)
			break;			// We stopped early
			
		if (bh->state == BUF_STATE_FULL) 
		{
			//printk("check 4-1\n");
			smp_rmb();
			fsg->next_buffhd_to_drain = bh->next;
			bh->state = BUF_STATE_EMPTY;

			/* Did something go wrong with the transfer? */
			if (bh->outreq->status != 0) {
				curlun->sense_data = SS_COMMUNICATION_FAILURE;
				curlun->sense_data_info = 0;
				curlun->info_valid = 0;
				break;
			}
			//printk("check 4-2\n");
			amount = bh->outreq->actual;
			/* Perform the write */
			if(ops&&ops->write)
				nwritten = ops->write(ops->priv,(char __user *) bh->buf, amount);
			else
				nwritten = -EINVAL;
			
			VLDBG(curlun, "vendor write %u -> %d\n", amount, (int) nwritten);		
			
			if (signal_pending(current))
				return -EINTR;		// Interrupted!

			if (nwritten < 0) 
			{
				LDBG(curlun, "error in file write: %d\n",
						(int) nwritten);
				nwritten = 0;
			} 
			else if (nwritten < amount) 
			{
				LDBG(curlun, "partial file write: %d/%u\n",(int) nwritten, amount);
				nwritten -= (nwritten & 511);	// Round down to a block
			}
			//file_offset += nwritten;
			amount_left_to_write -= nwritten;
			fsg->residue -= nwritten;

			/* If an error occurred, report it and its position */
			if (nwritten < amount) {
				curlun->sense_data = SS_WRITE_ERROR;
				curlun->sense_data_info = 0;
				curlun->info_valid = 0;
				break;
			}

			/* Did the host decide to stop early? */
			if (bh->outreq->actual != bh->outreq->length) {
				fsg->short_packet_received = 1;
				break;
			}
			continue;
		}
		//printk("check 5\n");
		/* Wait for something to happen */
		rc = sleep_thread(fsg);
		//printk("check 6\n");
		if (rc)
			return rc;
	}
		//printk("check 7\n");
	return -EIO;		// No default reply
}

/**
* @brief 	Vendor process function.
* @param	fsg[in]: fsg device struct pointer.
* @return 	0 for success, others number means fail.
*/ 
int do_gp_vendor(struct fsg_dev *fsg)
{
	int ret = 0;
	int csw_ret = 0;
	unsigned int cmd = (fsg->cmnd[0]==0xf6)?fsg->cmnd[1]+256:fsg->cmnd[1];
	struct usb_vendor_ops   *ops =  gp_usb_vendor_get(cmd);
	if(fsg->data_size_from_cmnd)
	{
		if( fsg->data_dir == DATA_DIR_FROM_HOST)	
			ret = do_gp_vendor_write(fsg);
		else
			ret = do_gp_vendor_read(fsg);
	}
	if(ops)	
		csw_ret = ops->set_csw(ops->priv, &fsg->curlun->sense_data);
	else
		ret = -EINVAL;
	if(csw_ret==2)
		fsg->phase_error = 1;
	return ret;
}

/**
* @brief 	Vendor command 0xf0f0.
* @param	fsg[in]: fsg device struct pointer.
* @param	bh[in]: fsg buffer struct pointer.
* @return 	0 for success, others number means fail.
*/ 
int do_gp_setvid(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	u8	*buf = (u8 *) bh->buf;
	unsigned int i;
	unsigned short tmp = 0;
	unsigned short mask = (fsg->cmnd[14]<<8)|(fsg->cmnd[15]);
	
	for(i=0;i<7;i++)
		tmp += fsg->cmnd[i+2];
	
	tmp ^= mask;
	buf[0] = tmp>>8;
	buf[1] = tmp;
	
	return 2;
}	