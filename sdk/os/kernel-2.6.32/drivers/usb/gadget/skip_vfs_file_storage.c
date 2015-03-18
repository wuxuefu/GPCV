#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <linux/utsname.h>
#include <linux/random.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/workqueue.h>
#include <mach/gp_cache.h>
#include <mach/hardware.h>
#include <mach/general.h>
#include <mach/gp_usb.h>
#include <mach/gp_board.h>
#include "gadget_chips.h"
#include <mach/hal/hal_sd.h>
#include <mach/typedef.h>
#include <mach/general.h>


/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "s_file_storage.h"

sync_queue_t *nandsync = NULL;

/*
#define TEST_WRITE
#define TEST_READ
*/

extern void gadget_sbull_transfer(struct block_device *bdev, unsigned long sector,	unsigned long nsect, char *buffer, int write);								
extern void FlushWorkbuffer(void);
															
static int do_nand_read(struct fsg_dev *fsg)
{
	struct para_tab tab;	
	unsigned int		dev_id;
	loff_t	file_offset;
	loff_t	usb_offset;
	unsigned int			partial_page;
	int			rc, ret;
	struct gp_board_nand_s *pConfig = gp_board_get_config("nand",gp_board_nand_t);
	tab.curlun = fsg->curlun;

	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	if (fsg->cmnd[0] == SC_READ_6)
		tab.start = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
	else {
		tab.start = get_be32(&fsg->cmnd[2]);

		/* We allow DPO (Disable Page Out = don't save data in the
		 * cache) and FUA (Force Unit Access = don't read from the
		 * cache), but we don't implement them. */
		if ((fsg->cmnd[1] & ~0x18) != 0) {
			tab.curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
	}
	
	tab.offset = tab.start;
	
	if (tab.start >= tab.curlun->num_sectors) {
		tab.curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		printk("out of range\n");
		return -EINVAL;
	}
	file_offset = ((loff_t) tab.start) << 9;

	/* Carry out the file reads */
	tab.data_left = fsg->data_size_from_cmnd;
	if (unlikely(tab.data_left == 0))
		return -EIO;		// No default reply

	for (;;) {

		/* Figure out how much we need to read:
		 * Try to read the remaining tab.data_length.
		 * But don't read more than the buffer size.
		 * And don't try to read past the end of the file.
		 * Finally, if we're not at a page boundary, don't read past
		 *	the next page.
		 * If this means reading 0 then we were asked/* Queue a request for more data from the host */
		tab.data_length = min((unsigned int) tab.data_left, mod_data.buflen);
		tab.data_length = min((loff_t) tab.data_length,
				tab.curlun->file_length - file_offset);
		partial_page = file_offset & (PAGE_CACHE_SIZE - 1);
		if (partial_page > 0)
			tab.data_length = min(tab.data_length, (unsigned int) PAGE_CACHE_SIZE -
					partial_page);

		/* Wait for the next buffer to become available */
		tab.bh = fsg->next_buffhd_to_fill;
		while (tab.bh->state != BUF_STATE_EMPTY) {
			printk("%s buffer not empty so sleep\n", __FUNCTION__);
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		
		/* If we were asked to read past the end of file,
		 * end with an empty buffer. */
		if (tab.data_length == 0) {
			tab.curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
			tab.curlun->sense_data_info = file_offset >> 9;
			tab.curlun->info_valid = 1;
			tab.bh->inreq->length = 0;
			tab.bh->state = BUF_STATE_FULL;
			printk("[%s][%d]state[%d]\n", __FUNCTION__, __LINE__, tab.bh->state);
			break;
		}
		if(pConfig->sbull_transfer != NULL){
			pConfig->sbull_transfer(tab.curlun->bdev, tab.offset, tab.data_length>>9, (char*)tab.bh->buf, 0);
		}	
		tab.offset += (tab.data_length>>9);
		
				
		if (signal_pending(current))
			return -EINTR;

		file_offset  += tab.data_length;
		tab.data_left  -= tab.data_length;
		fsg->residue -= tab.data_length;
		tab.bh->inreq->length = tab.data_length;
		tab.bh->state = BUF_STATE_FULL;

		if (tab.data_left == 0)
			break;		// No more left to read

		/* Send this buffer and go read some more */
		tab.bh->inreq->zero = 0;
		
		gp_clean_dcache_range((unsigned long)tab.bh->inreq->buf, tab.data_length);

		start_transfer(fsg, fsg->bulk_in, tab.bh->inreq, &tab.bh->inreq_busy, &tab.bh->state); 
		fsg->next_buffhd_to_fill = tab.bh->next;
				
	}
	return -EIO;		// No default reply
}

static int do_nand_write(struct fsg_dev *fsg)
{	
	struct para_tab tab;	
	unsigned int		dev_id;
	unsigned int		partial_page;
	loff_t	file_offset;
	loff_t	usb_offset;
	int			rc, ret;
	u32 data_remain;
	struct gp_board_nand_s *pConfig = gp_board_get_config("nand",gp_board_nand_t);
	tab.curlun = fsg->curlun;

	if (tab.curlun->ro) {
		tab.curlun->sense_data = SS_WRITE_PROTECTED;
		return -EINVAL;
	}
	tab.curlun->filp->f_flags &= ~O_SYNC;	// Default is not to wait

	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	if (fsg->cmnd[0] == SC_WRITE_6)
		tab.start = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
	else {
		tab.start = get_be32(&fsg->cmnd[2]);

		/* We allow DPO (Disable Page Out = don't save data in the
		 * cache) and FUA (Force Unit Access = write directly to the
		 * medium).  We don't implement DPO; we implement FUA by
		 * performing synchronous output. */
		if ((fsg->cmnd[1] & ~0x18) != 0) {
			tab.curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
	}
	
	tab.offset = tab.start;
	
	if (tab.start >= tab.curlun->num_sectors) {
		tab.curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}
	
	/* Carry out the file writes */
	usb_offset = ((loff_t) tab.start) << 9;
	tab.data_left = data_remain = fsg->data_size_from_cmnd; 
		
	while(tab.data_left > 0){		
		tab.bh = fsg->next_buffhd_to_fill;
		
		while (tab.bh->state != BUF_STATE_EMPTY) {
			printk("%s buffer not empty so sleep\n", __FUNCTION__);
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		
		if(tab.bh->state == BUF_STATE_EMPTY && data_remain != 0){

			tab.data_length = min(tab.data_left, mod_data.buflen);
			tab.data_length = min((loff_t) tab.data_length, tab.curlun->file_length - usb_offset);
			partial_page = usb_offset & (PAGE_CACHE_SIZE - 1);
				
			if (partial_page > 0)
				tab.data_length = min(tab.data_length, (unsigned int) PAGE_CACHE_SIZE - partial_page);

			if (tab.data_length == 0) {
				tab.curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
				tab.curlun->sense_data_info = usb_offset >> 9;
				tab.curlun->info_valid = 1;
			}
		
			tab.bh->outreq->length = tab.bh->bulk_out_intended_length = tab.data_length;
			tab.bh->outreq->short_not_ok = 1;
					
			start_transfer(fsg, fsg->bulk_out, tab.bh->outreq, &tab.bh->outreq_busy, &tab.bh->state);
			gp_invalidate_dcache_range((unsigned long)tab.bh->outreq->buf, tab.data_length);	
		
			fsg->next_buffhd_to_fill = tab.bh->next;
			
			/* Get the next buffer */
			usb_offset += tab.data_length;
			fsg->usb_amount_left -= tab.data_length;
			data_remain -= tab.data_length;
		}
		
		tab.bh = fsg->next_buffhd_to_drain;
				
		if (tab.bh->state == BUF_STATE_FULL){
			smp_rmb();
			fsg->next_buffhd_to_drain = tab.bh->next;
			
			if(pConfig->sbull_transfer != NULL){
				pConfig->sbull_transfer(tab.curlun->bdev, tab.offset, tab.data_length>>9, (char*)tab.bh->buf, 1);
			}	
			
			tab.offset += (tab.data_length>>9);
			tab.bh->state = BUF_STATE_EMPTY;
			tab.data_left -= tab.bh->outreq->length;
		}
		
		if (signal_pending(current))
			return -EINTR;		// Interrupted!
		
	}
	
	return -EIO;
}

static int do_sd_read(struct fsg_dev *fsg)
{
	struct para_tab tab;	
	unsigned int		dev_id;
	loff_t	file_offset;
	loff_t	usb_offset;
	unsigned int			partial_page;
	int			rc, ret;
	struct gp_board_sd_s *pConfig;
	tab.curlun = fsg->curlun;

	if(tab.curlun == TYPE_SD0)
			pConfig = gp_board_get_config("sd0",gp_board_sd_t);
	else
			pConfig = gp_board_get_config("sd1",gp_board_sd_t);

	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	if (fsg->cmnd[0] == SC_READ_6)
		tab.start = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
	else {
		tab.start = get_be32(&fsg->cmnd[2]);

		/* We allow DPO (Disable Page Out = don't save data in the
		 * cache) and FUA (Force Unit Access = don't read from the
		 * cache), but we don't implement them. */
		if ((fsg->cmnd[1] & ~0x18) != 0) {
			tab.curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
	}
	if (tab.start >= tab.curlun->num_sectors) {
		tab.curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		printk("out of range\n");
		return -EINVAL;
	}
	tab.offset =  tab.curlun->bdev->bd_part->start_sect;
	file_offset = ((loff_t) tab.start) << 9;
	tab.start += tab.offset;

	/* Carry out the file reads */
	tab.data_left = fsg->data_size_from_cmnd;
	if (unlikely(tab.data_left == 0))
		return -EIO;		// No default reply
		
	dev_id = *(unsigned char*)(tab.curlun->bdev->bd_disk->private_data);
	
	ret = (pConfig->read_cmd != NULL) ? pConfig->read_cmd(dev_id, tab.start) : -1;
	if(ret == SP_FALSE){
			printk("Read Command Error\n");
			return -EIO;
	}

	for (;;) {

		/* Figure out how much we need to read:
		 * Try to read the remaining tab.data_length.
		 * But don't read more than the buffer size.
		 * And don't try to read past the end of the file.
		 * Finally, if we're not at a page boundary, don't read past
		 *	the next page.
		 * If this means reading 0 then we were asked/* Queue a request for more data from the host */
		tab.data_length = min((unsigned int) tab.data_left, mod_data.buflen);
		tab.data_length = min((loff_t) tab.data_length,
				tab.curlun->file_length - file_offset);
		partial_page = file_offset & (PAGE_CACHE_SIZE - 1);
		if (partial_page > 0)
			tab.data_length = min(tab.data_length, (unsigned int) PAGE_CACHE_SIZE -
					partial_page);

		/* Wait for the next buffer to become available */
		tab.bh = fsg->next_buffhd_to_fill;
		while (tab.bh->state != BUF_STATE_EMPTY) {
			printk("%s buffer not empty so sleep\n", __FUNCTION__);
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		
		/* If we were asked to read past the end of file,
		 * end with an empty buffer. */
		if (tab.data_length == 0) {
			tab.curlun->sense_data =
					SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
			tab.curlun->sense_data_info = file_offset >> 9;
			tab.curlun->info_valid = 1;
			tab.bh->inreq->length = 0;
			tab.bh->state = BUF_STATE_FULL;
			printk("[%s][%d]state[%d]\n", __FUNCTION__, __LINE__, tab.bh->state);
			break;
		}
		
		/* Perform the read */
		ret = (pConfig->dma_enable != NULL) ? pConfig->dma_enable(dev_id, (unsigned char *)virt_to_phys(tab.bh->buf), tab.data_length, 0) : -1;
		if(ret == SP_FALSE){
			printk("[%d]:DMA Enable error\n", dev_id);
			return -EIO;
		}

		ret = (pConfig->dma_finish != NULL) ? pConfig->dma_finish(dev_id, (tab.data_length>>9)*15) : -1;
		
		if(ret != 0){
			printk("[%d][%s]:DMA Timeout: %d\n", dev_id,__FUNCTION__, ret);
			(pConfig->dma_stop != NULL) ? pConfig->dma_stop(tab.curlun->bdev) : -1;
			break;
		}

		if (signal_pending(current))
			return -EINTR;
		

		file_offset  += tab.data_length;;
		tab.data_left  -= tab.data_length;;
		fsg->residue -= tab.data_length;;
		tab.bh->inreq->length = tab.data_length;;
		tab.bh->state = BUF_STATE_FULL;

		if (tab.data_left == 0)
			break;		// No more left to read

		/* Send this buffer and go read some more */
		tab.bh->inreq->zero = 0;
		
		gp_clean_dcache_range((unsigned long)tab.bh->inreq->buf, tab.data_length);

		start_transfer(fsg, fsg->bulk_in, tab.bh->inreq,
				&tab.bh->inreq_busy, &tab.bh->state); 
		fsg->next_buffhd_to_fill = tab.bh->next;
				
	}

	ret = (pConfig->transfer_stop != NULL) ? pConfig->transfer_stop(dev_id) : -1;
	if(ret == SP_FALSE){
		printk("[%d]:Transfer error\n", dev_id);
		return -EIO;
	}
	
	return -EIO;		// No default reply
}


static int do_sd_write(struct fsg_dev *fsg)
{
	struct para_tab tab;	
	unsigned int 		dev_id;
	loff_t	file_offset;
	loff_t	usb_offset;
	int			rc, ret;
	unsigned int 		dma_en;
	unsigned int		partial_page;
	struct gp_board_sd_s *pConfig;
	
	tab.curlun = fsg->curlun;
	dma_en = 0;

	if(tab.curlun == TYPE_SD0)
			pConfig = gp_board_get_config("sd0", gp_board_sd_t);
	else
			pConfig = gp_board_get_config("sd1", gp_board_sd_t);

	if (tab.curlun->ro) {
		tab.curlun->sense_data = SS_WRITE_PROTECTED;
		return -EINVAL;
	}
	tab.curlun->filp->f_flags &= ~O_SYNC;	

	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	if (fsg->cmnd[0] == SC_WRITE_6)
		tab.start = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
	else {
		tab.start = get_be32(&fsg->cmnd[2]);

		/* We allow DPO (Disable Page Out = don't save data in the
		 * cache) and FUA (Force Unit Access = write directly to the
		 * medium).  We don't implement DPO; we implement FUA by
		 * performing synchronous output. */
		if ((fsg->cmnd[1] & ~0x18) != 0) {
			tab.curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}		
	}
	if (tab.start >= tab.curlun->num_sectors) {
		tab.curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}
	
	tab.offset =  tab.curlun->bdev->bd_part->start_sect;
	file_offset = usb_offset = ((loff_t) tab.start) << 9;
	
	tab.start += tab.offset;
	/* Carry out the file writes */
	
	tab.data_left = fsg->data_size_from_cmnd; 

	dev_id = *(unsigned char*)(tab.curlun->bdev->bd_disk->private_data);

	ret = (pConfig->write_cmd != NULL) ? pConfig->write_cmd(dev_id, tab.start) : -1;
	if(ret == SP_FALSE){
			printk("Write Command Error\n");
			return -EIO;
	}

	while(tab.data_left > 0){
	
		tab.bh = fsg->next_buffhd_to_fill;
		
		while (tab.bh->state != BUF_STATE_EMPTY) {
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		
		if(tab.bh->state == BUF_STATE_EMPTY){
			tab.data_length = min(tab.data_left, mod_data.buflen);
			tab.data_length = min((loff_t) tab.data_length, tab.curlun->file_length - usb_offset);
			partial_page = usb_offset & (PAGE_CACHE_SIZE - 1);
				
			if (partial_page > 0)
				tab.data_length = min(tab.data_length, (unsigned int) PAGE_CACHE_SIZE - partial_page);

			if (tab.data_length == 0) {
				tab.curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
				tab.curlun->sense_data_info = usb_offset >> 9;
				tab.curlun->info_valid = 1;
			}
		
			tab.bh->outreq->length = tab.bh->bulk_out_intended_length = tab.data_length;
			tab.bh->outreq->short_not_ok = 1;
				
			start_transfer(fsg, fsg->bulk_out, tab.bh->outreq, &tab.bh->outreq_busy, &tab.bh->state);
			gp_invalidate_dcache_range((unsigned long)tab.bh->outreq->buf, tab.data_length);
			
			/* Get the next buffer */
			fsg->next_buffhd_to_fill = tab.bh->next;

			usb_offset += tab.data_length;
			fsg->usb_amount_left -= tab.data_length;
			tab.data_left -= tab.data_length;

		}
		
		tab.bh = fsg->next_buffhd_to_drain;
		
		if(dma_en == 1){
				ret = (pConfig->dma_finish != NULL) ? pConfig->dma_finish(dev_id, (tab.data_length>>9) * 55) : -1;			
				if(ret != 0){
					printk("[%d][%s]:DMA Timeout: %d", dev_id,__FUNCTION__, ret);
					(pConfig->dma_stop != NULL) ? pConfig->dma_stop(tab.curlun->bdev) : -1;
					break;
				}
				dma_en = 0;	
				tab.bh->state = BUF_STATE_EMPTY;
				fsg->next_buffhd_to_drain = tab.bh->next;			
		}
		
		tab.bh = fsg->next_buffhd_to_drain;
				
		if (tab.bh->state == BUF_STATE_FULL){
			smp_rmb();
			ret = (pConfig->dma_enable != NULL) ? pConfig->dma_enable(dev_id, (unsigned char *)virt_to_phys(tab.bh->buf), tab.data_length, 1) : -1;					
			if(ret == SP_FALSE){
				printk("[%d]:DMA Enable error\n", dev_id);
			}
			dma_en = 1;
		}		
	}
	
	if(dma_en == 1){
			ret = (pConfig->dma_finish != NULL) ? pConfig->dma_finish(dev_id, (tab.data_length>>9) * 55) : -1;				
			if(ret != 0){
				printk("[%d][%s]:DMA Timeout: %d", dev_id,__FUNCTION__, ret);
				(pConfig->dma_stop != NULL) ? pConfig->dma_stop(tab.curlun->bdev) : -1;
			}
			dma_en = 0;
	}

	ret = (pConfig->WaitDataComplete != NULL) ? pConfig->WaitDataComplete(dev_id) : -1;				
	if(ret == SP_FALSE){
				printk("[%d]: wait complete error\n", dev_id);
				return -EIO;
	}


	ret = (pConfig->transfer_stop != NULL) ? pConfig->transfer_stop(dev_id) : -1;
	if(ret == SP_FALSE){
		printk("[%d]:Transfer error\n", dev_id);
		return -EIO;
	}
	return -EIO;
}

int gp_do_read(struct fsg_dev *fsg)
{
	struct lun		*curlun = fsg->curlun;
	if(curlun->type == TYPE_SD0 || curlun->type == TYPE_SD1)
		return do_sd_read(fsg);
	else if(curlun->type == TYPE_NAND)
		return do_nand_read(fsg);
}

int gp_do_write(struct fsg_dev *fsg)
{
	struct lun		*curlun = fsg->curlun;
	if(curlun->type == TYPE_SD0 || curlun->type == TYPE_SD1)
		return do_sd_write(fsg);
	else if(curlun->type == TYPE_NAND)
		return do_nand_write(fsg);
}

void nandsyncHandler(struct work_struct *data)
{	
	struct gp_board_nand_s *pConfig = gp_board_get_config("nand",gp_board_nand_t);
	printk("[%s] Time out, trigger nand flush !\n", __FUNCTION__);
	nandsync->state = SYNC_RUNNING;
	nandsync->response = 0;
	if(pConfig->flush != NULL)
		pConfig->flush();
	nandsync->state = SYNC_READY;
}

static DECLARE_DELAYED_WORK(work, nandsyncHandler);

int initSyncWork(void)
{
	nandsync = kzalloc(sizeof(nandsync), GFP_KERNEL);
	if (nandsync == NULL)
		return -ENOMEM;
	nandsync->workqueue = create_workqueue("nandsync");
    if (nandsync->workqueue == NULL)
        return -1;
	nandsync->state = SYNC_IDLE;
	return 0;
}

void delSyncwork(u8 cmd)
{
	if(cmd && (nandsync->state == SYNC_READY)){
		cancel_delayed_work_sync(&work);
		nandsync->state = SYNC_IDLE;
	}
}

void addSyncwork(u8 cmd)
{
	if(cmd == 0x2a){
		nandsync->response = 1;
	}
	if(nandsync->response){
		queue_delayed_work(nandsync->workqueue, &work, 30);
		nandsync->state = SYNC_READY;
	}
}


int exitSyncWork(void)
{	
	if(nandsync->state == SYNC_READY)
	{
		cancel_delayed_work_sync(&work);
		nandsync->state = SYNC_IDLE;
	}
	destroy_workqueue(nandsync->workqueue);
	nandsync->state = SYNC_STOP;
	kfree(nandsync);
	nandsync = NULL;
	return 0;
}
