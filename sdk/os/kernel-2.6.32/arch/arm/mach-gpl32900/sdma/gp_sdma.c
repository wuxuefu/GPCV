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
 * @file    gp_sdma.c
 * @brief   Implement of SDMA module driver.
 * @author
 */

#include <mach/kernel.h>
#include <mach/diag.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/typedef.h>
#include <mach/hal/hal_sdma.h>
#include <mach/hal/regmap/reg_sdma.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_sdma.h>
#include <mach/gp_cache.h>
#include <linux/delay.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

#define SDMA_MAX_SIZE_LIMIT		(0xFFF80)
#define SDMA_MAX_BLOCK			0x100000
#define SDMA_MAX_FRAME			0x100000
#define CHANNEL_NUM 			2

#define SDMA_ALLOC_TIMEOUT		500			/*!< @brief Channel alloc timeout: 5sec */
#define SDMA_TRANS_TIMEOUT		500			/*!< @brief Channel transaction timeout: 5sec */

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/


#define DEBUG(fmt, arg...)		DIAG_DEBUG("[%s][DBG]: "fmt, __FUNCTION__, ##arg)

#define DERROR(fmt, arg...)		DIAG_ERROR("[%s][ERR]: "fmt, __FUNCTION__, ##arg)

#define gp_sdma_protect()	{down(&g_sdma.sem); spin_lock(&g_sdma.lock);}
#define gp_sdma_unprotect()	{spin_unlock(&g_sdma.lock); up(&g_sdma.sem);}

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct gpSDMACHInfo_s
{
	volatile unsigned int		irq;			/*!< @brief IRQ status */
}gpSDMACHInfo_t;

typedef struct gpSDMAInfo_s{
	volatile unsigned long 	ch_status;			/*!< @brief Channel status */
	atomic_t 				wait_usr;			/*!< @brief Wait user number */
	wait_queue_head_t		wait_queue;			/*!< @brief Wait queue */ 
	wait_queue_head_t 		finish_queue;		/*!< @brief Dma finish queue */
	spinlock_t				lock;               /*!< @brief For mutual exclusion */
	struct semaphore		sem;				/*!< @brief For mutual exclusion */
	gpSDMACHInfo_t			ch[CHANNEL_NUM];	/*!< @brief Dma channel parametet */
}gpSDMAInfo_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

static int gp_sdma_open(struct inode *ip, struct file *fp);
static int gp_sdma_release(struct inode *ip, struct file* fp);
static int gp_sdma_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg);
static int gp_sdma_memcpy_user(gp_Sdma_t* sdma );
static int gp_sdma_memcpy_2d_user(gp_memcpy_2d_t cmd );

static int gp_sdma_perTrans(void* src, void* dst, unsigned int blockSize, unsigned int blockCnt);
static int gp_sdma_perTrans_2d(gp_memcpy_2d_t cmd );

static int gp_sdma_probe( struct platform_device *pdev);
static int gp_sdma_remove( struct platform_device *pdev);
static void gp_sdma_device_release(struct device *dev);

/**************************************************************************
*                         G L O B A L    D A T A                         *
***************************************************************************/

static gpSDMAInfo_t g_sdma = {0};

/**
 * @brief   SDMA Class OPS
 */
static struct file_operations gp_sdma_fops = 
{
	.owner = 	THIS_MODULE,
	.open = 	gp_sdma_open,
	.release = 	gp_sdma_release,
    .ioctl = 	gp_sdma_ioctl,
};

/**
 * @brief   SDMA misc device define
 */
static struct miscdevice gp_sdma_misc = 
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sdma",
	.fops = &gp_sdma_fops,
};

/**
 * @brief   SDMA platform device define
 */
static struct platform_driver gp_sdma_driver = 
{
	.probe		= gp_sdma_probe,
	.remove		= gp_sdma_remove,
	.suspend    = NULL,
	.resume     = NULL,
	.driver = {
		.name = "gp_sdma"
	}
};

/**
 * @brief   SDMA platform device driver define
 */
struct platform_device gp_sdma_device = 
{
	.name	= "gp_sdma",
	.id		= -1,
	.dev	= 
	{
		.release = gp_sdma_device_release,
	},
};

/**************************************************************************
*             F U N C T I O N    I M P L E M E N T A T I O N S            *
**************************************************************************/

/**
* @brief 	Get free channel.
* @return:	0 for no channel, others mean channel number + 1.
*/
static unsigned int gp_sdma_getfree(void)
{
	unsigned int free_ch;
	gp_sdma_protect();
	free_ch = find_first_zero_bit((const void*)&g_sdma.ch_status,32);
	if(free_ch>=CHANNEL_NUM)
		free_ch = 0;
	else
	{
		/* ----- Initial SDMA ----- */
		if(g_sdma.ch_status==0)
			gpHalSdmaInit();
		set_bit(free_ch, &g_sdma.ch_status);
		free_ch++;
	}
	gp_sdma_unprotect();	
	return free_ch;
}

/**
* @brief 	Check dma finish or not.
* @param 	indexChan[in]: Channel Number.
* @return:	0 for finish, others mean error or not finish.
*/
static int gp_sdma_ckfinish(unsigned int indexChan)
{
	gpSDMACHInfo_t* ch = (gpSDMACHInfo_t*)&g_sdma.ch[indexChan];
	int ret = 0;
	/* ----- Check irq status ----- */
	if(ch->irq)
	{
		if(ch->irq & 0x800080)
		{
			DERROR("Bus error, status = 0x%x\n", ch->irq);
			ret = -	EIO;
		}
	}
	else
	{
		DERROR("ch %d timeout\n", indexChan);
		ret = -EALREADY;
	}
	return ret;
}

/**
* @brief 	Acquire sdma handle.
* @param 	timeout[in]: timeout (unit: 10ms).
* @return:	SDMA handle(non-zero and non-negative)/ERROR_ID(-1).
*/
unsigned int gp_sdma_alloc_chan(unsigned short timeout)
{
	unsigned int free_ch = 0;
	/* ----- Get free channel ----- */
	if((free_ch = gp_sdma_getfree())==0)
	{
		atomic_inc(&g_sdma.wait_usr);
		/* ----- Wait for channel free ----- */
		if(timeout)
			wait_event_interruptible_timeout(g_sdma.wait_queue, (free_ch = gp_sdma_getfree())!=0, timeout*HZ/100 );
		else
			wait_event_interruptible(g_sdma.wait_queue, (free_ch = gp_sdma_getfree())!=0);
		atomic_dec(&g_sdma.wait_usr);
	}
	return free_ch;	
}

/**
* @brief 	Release sdma handle.
* @param 	handle [in]: sdma handle.
* @return:	None.
*/
void gp_sdma_free_chan(unsigned int handle)
{
	handle --;
	/* ----- Check handle ----- */
	if(likely(handle<CHANNEL_NUM))
	{	
		gp_sdma_protect();
		/* ----- Reset channel ----- */
		g_sdma.ch[handle].irq = 0;
		/* ----- Clear Channel status ----- */
		clear_bit(handle, &g_sdma.ch_status);
		/* ----- Uninitial SDMA ----- */
		if(g_sdma.ch_status==0)
			gpHalSdmaUninit();
		gp_sdma_unprotect();
		if(atomic_read(&g_sdma.wait_usr))
			wake_up_interruptible(&g_sdma.wait_queue);
	}
}

/**
* @brief 	Enable SDMA.
* @param 	handle [in]: SDMA handle.
* @param	param [in]: SDMA parameter.
* @return: 	None.
*/
void gp_sdma_en(unsigned int handle, gpSdma_t* param)
{
	unsigned int  indexChan = handle - 1;
	/* ----- Check handle ----- */
	if(unlikely(indexChan >= CHANNEL_NUM))
	{
		DERROR("Handle error\n");
		return ;	
	}
	/* ----- Enable channel ----- */
	gpHalClearIrq(indexChan);
	gpHalMaskIrq(indexChan);
	gpHalSdmaTrriger(indexChan, param);
}

/**
* @brief 	Wait until SDMA finish. (blocking)
* @param 	timeout [in]: timeout (unit: 10ms).
* @param 	handle [in]: SDMA handle.
* @return: 	SUCCESS(0)/ERROR_ID
*/
int gp_sdma_wait(unsigned int handle, unsigned short timeout)
{
	unsigned int indexChan = handle - 1;
	int ret = 0;
	/* ----- Check handle ----- */
	if(unlikely(indexChan >= CHANNEL_NUM))
	{
		DERROR("Handle error\n");
		return -EINVAL;	
	}
	/* ----- Wait dma finish ----- */	
	if(timeout)
		wait_event_interruptible_timeout(g_sdma.finish_queue, g_sdma.ch[indexChan].irq, timeout*HZ/100);
	else
		wait_event_interruptible(g_sdma.finish_queue, g_sdma.ch[indexChan].irq);
	/* ----- Check dma status ----- */
	ret = gp_sdma_ckfinish(indexChan);
	g_sdma.ch[indexChan].irq = 0;
	
	if(ret<0)
		gpHalDump(indexChan);
		
	if(ret == -EALREADY)
		ret = -ETIMEDOUT;
	
	return ret;
}

/**
* @brief 	Check for SDMA finish or not. (non-blocking)
* @param 	handle [in]: SDMA handle.
* @return: 	SUCCESS(0)/ERROR_ID
*/
int gp_sdma_trywait(unsigned int handle)
{
	unsigned int indexChan = handle - 1;
	int ret = 0;
	/* ----- Check handle ----- */
	if(unlikely(indexChan >= CHANNEL_NUM))
	{
		DERROR("Handle error\n");
		return -EINVAL;	
	}
	ret = gp_sdma_ckfinish(indexChan);
	
	if(ret == 0)
		g_sdma.ch[indexChan].irq = 0;
	
	return ret;
}

/**
* @brief 	SDMA transfer function. 
* @param 	src [in]: Source address.
* @param 	dst [in]: Destination address.
* @param 	blockSize [in]: Block size.
* @param 	blockCnt [in]: Block count. ( 2D Packet size)
* @return: 	SUCCESS(0)/ERROR_ID
*/
static int gp_sdma_perTrans(void* src, void* dst, unsigned int blockSize, unsigned int blockCnt)
{
	unsigned int handle;
	gpSdma_t param = {0};
	int ret = 0;
	/* ----- Alloc channel ----- */
	handle = gp_sdma_alloc_chan(SDMA_ALLOC_TIMEOUT);
	if(handle == 0)
	{
		DERROR("No channel\n");
		return -EBUSY;
	}
	/* ----- Set SDMA parameter ----- */
	param.srcAddr = src;
	param.dstAddr = dst;
	param.blockSize = blockSize;
	param.bStepSize = 0;
	param.frameSize = blockCnt;
	param.fStepSize = 0;
	if (blockCnt != 0)
		param.packetSize = 1;
	else 
		param.packetSize = 0;
	/* ----- Enable SDMA channel ----- */
	gp_sdma_en(handle, &param);
	/* ----- Wait SDMA finish ----- */
	ret = gp_sdma_wait(handle, SDMA_TRANS_TIMEOUT);
	/* ----- Free SDMA channel ----- */
	gp_sdma_free_chan(handle);
	
	return ret;
}

/**
* @brief 	SDMA 2D transfer function. 
* @param 	cmd [in]: SDMA 2D parameter.
* @return: 	SUCCESS(0)/ERROR_ID
*/
static int gp_sdma_perTrans_2d( gp_memcpy_2d_t cmd )
{
	unsigned int handle;
	gpSdma_t param = {0};
	int ret = 0;
	/* ----- Alloc channel ----- */
	handle = gp_sdma_alloc_chan(SDMA_ALLOC_TIMEOUT);
	if(handle == 0)
	{
		DERROR("No channel\n");
		return -EBUSY;
	}
	/* ----- Set SDMA parameter ----- */
	param.srcAddr = cmd.src;
	param.dstAddr = cmd.dst;
	param.blockSize = cmd.width;
	param.bStepSize = cmd.src_width - cmd.width;
	param.frameSize = cmd.height;
	param.packetSize = 1;
	param.dbStepSize = cmd.dst_width - cmd.width;
	/* ----- Enable SDMA channel ----- */
	gp_sdma_en(handle, &param);
	/* ----- Wait SDMA finish ----- */
	ret = gp_sdma_wait(handle, SDMA_TRANS_TIMEOUT);
	/* ----- Free SDMA channel ----- */
	gp_sdma_free_chan(handle);
	
	return ret;
}

/**
* @brief 	SDMA memcpy transfer function. 
* @param 	src [in]: Source address.
* @param 	dst [in]: Destination address.
* @param 	size [in]: Transfer length.
* @return: 	SUCCESS(0)/ERROR_ID
*/
int gp_sdma_run(void* src, void* dst, unsigned int size) 
{
	int ret = 0;
	unsigned int blockCnt = size / SDMA_MAX_SIZE_LIMIT;
	unsigned int leaveSize = size % SDMA_MAX_SIZE_LIMIT;
	
#if 0	/* ----- One-dimension transfer ----- */
	while (blockCnt--) {
		ret = gp_sdma_perTrans(src, dst, SDMA_MAX_SIZE_LIMIT, 0);
		src += SDMA_MAX_SIZE_LIMIT;
		dst += SDMA_MAX_SIZE_LIMIT;
	}
	ret = gp_sdma_perTrans(src, dst, leaveSize, 0);
	
#else	/* ----- two-dimension transfer ----- */
	if (blockCnt > 0) {
		ret = gp_sdma_perTrans(src, dst, SDMA_MAX_SIZE_LIMIT, blockCnt);
	}

	if (leaveSize > 0) {
		src += (blockCnt * SDMA_MAX_SIZE_LIMIT);
		dst += (blockCnt * SDMA_MAX_SIZE_LIMIT);
		ret = gp_sdma_perTrans(src, dst, leaveSize, 0);
	}
#endif
	return ret;
}

/**
* @brief 	SDMA memcpy function for kernel space.
* @param 	sdma [in]: SDMA parameter.
* @return: 	SUCCESS(0)/ERROR_ID
*/
int gp_sdma_memcpy_kernel(gp_Sdma_t* sdma )
{
	void* srcPhyAddr;
	void* dstPhyAddr;
	int ret = 0;
	
	/* ----- convert address vir to phy ----- */
	srcPhyAddr = (void*)gp_chunk_pa(sdma->srcAdress);
	if (srcPhyAddr == NULL) 
	{
		DERROR("source address convert to phy error\n");
		return -EFAULT;
	}
	dstPhyAddr = (void*)gp_chunk_pa(sdma->dstAdress);
	if (dstPhyAddr == NULL) 
	{
		DERROR("destination address convert to phy error\n");
		return -EFAULT;
	}
	if (sdma->size <= 0) 
	{
		DERROR("invalid data size\n");
		return -EINVAL;
	}
	/* ----- Clean source address cache ----- */
	gp_clean_dcache_range((unsigned int)sdma->srcAdress, sdma->size);
	
	ret = gp_sdma_run(srcPhyAddr, dstPhyAddr, sdma->size);
	/* ----- Invalidate destination address cache ----- */
	if(ret==0)
		gp_invalidate_dcache_range((unsigned int)sdma->dstAdress, sdma->size);
	return ret;
}
EXPORT_SYMBOL(gp_sdma_memcpy_kernel);

/**
* @brief 	SDMA memcpy function for user space.
* @param 	sdma [in]: SDMA parameter.
* @return: 	SUCCESS(0)/ERROR_ID
*/
static int gp_sdma_memcpy_user( gp_Sdma_t* sdma )
{
	void* srcPhyAddr;
	void* dstPhyAddr;
	int ret = 0;
	
	/* ----- Convert address vir to phy ----- */
	srcPhyAddr = (void*)gp_user_va_to_pa(sdma->srcAdress);
	if (srcPhyAddr == NULL) 
	{
		DERROR("source address convert to phy error\n");
		return -EFAULT;
	}
	dstPhyAddr = (void*)gp_user_va_to_pa(sdma->dstAdress);
	if (dstPhyAddr == NULL)
	{
		DERROR("destination address convert to phy error\n");
		return -EFAULT;
	}
	if (sdma->size <= 0) 
	{
		DERROR("invalid data size\n");
		return -EINVAL;
	}
	/* ----- Clean source address cache ----- */
	gp_clean_dcache_range((unsigned int)sdma->srcAdress, sdma->size);
	
	ret = gp_sdma_run(srcPhyAddr, dstPhyAddr, sdma->size);
	/* ----- Invalidate destination address cache ----- */
	if(ret==0)
		gp_invalidate_dcache_range((unsigned int)sdma->dstAdress, sdma->size);
	
	return ret;
}

/**
* @brief 	SDMA 2D memcpy function for user space.
* @param 	cmd [in]: SDMA 2D parameter.
* @return: 	SUCCESS(0)/ERROR_ID
*/
static int gp_sdma_memcpy_2d_user(gp_memcpy_2d_t cmd )
{
	void* srcPhyAddr;
	void* dstPhyAddr;
	int ret = 0;
	
	/* ----- convert address vir to phy ----- */
	srcPhyAddr = (void*)gp_user_va_to_pa(cmd.src);
	if (srcPhyAddr == NULL) {
		DERROR("source address convert to phy error\n");
		return -EFAULT;
	}
	dstPhyAddr = (void*)gp_user_va_to_pa(cmd.dst);
	if (dstPhyAddr == NULL) {
		DERROR("destination address convert to phy error\n");
		return -EFAULT;
	}
	/* ------ Check block size & frame size ----- */
	if((cmd.width >= SDMA_MAX_BLOCK) || (cmd.height >= SDMA_MAX_FRAME))
	{
		DERROR("Block size error\n");
		return -EINVAL;
	}
	/* ------ Check block step size ----- */
	if((cmd.width > cmd.src_width)||(cmd.width > cmd.dst_width)||
		(cmd.src_width - cmd.width > SDMA_MAX_BLOCK)||(cmd.dst_width - cmd.width > SDMA_MAX_BLOCK))
	{
		DERROR("Block step size error\n");
		return -EINVAL;
	}
	
	cmd.src = srcPhyAddr;
	cmd.dst = dstPhyAddr;
	ret = gp_sdma_perTrans_2d(cmd);
	
	return ret;
}

/**
* @brief 	SDMA open function.
* @param 	ip [in]: Inode pointer.
* @param 	fp [in]: File pointer.
* @return: 	SUCCESS(0)
*/
static int gp_sdma_open( struct inode *ip, struct file *fp )
{
	return 0;
}

/**
* @brief 	SDMA release function.
* @param 	ip [in]: Inode pointer.
* @param 	fp [in]: File pointer.
* @return: 	SUCCESS(0)
*/
static int gp_sdma_release( struct inode *ip, struct file* fp ) 
{
	return 0;
}

/**
* @brief 	SDMA io control function.
* @param 	inode [in]: Inode pointer.
* @param 	flip [in]: File pointer.
* @param 	cmd [in]: Command.
* @param 	arg [in]: Argument.
* @return: 	SUCCESS(0)/ERROR_ID
*/
static int gp_sdma_ioctl( struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	switch(cmd)
	{
		case SDMA_IOCTL_TRIGGER:
			{
				gp_Sdma_t sdma ;
				
				copy_from_user(&sdma, (void __user*)arg, sizeof(gp_Sdma_t));
				
				ret = gp_sdma_memcpy_user(&sdma);
				
			}
		    break;
		case SDMA_IOCTL_MEMCPY_2D:
			{
				gp_memcpy_2d_t cpy ;
				copy_from_user(&cpy, (void __user*)arg, sizeof(gp_memcpy_2d_t));
				gp_clean_dcache_range((unsigned int)cpy.src, cpy.src_width*cpy.height);
				ret = gp_sdma_memcpy_2d_user(cpy);
				if(ret==0)
					gp_invalidate_dcache_range((unsigned int)cpy.dst, cpy.dst_width*cpy.height);
			}
			break;
		default:
			DERROR("Unsupport cmd:%d\n", cmd);
			ret = -EINVAL;
			break;
	}
	return ret;
}

/**
* @brief 	SDMA interrupt function.
* @param 	irq [in]: Interrupt number.
* @param 	dev_id [in]: Private data.
* @return: 	IRQ handle state
*/
static irqreturn_t gp_sdma_irq(int irq,void *dev_id)
{
	unsigned int ch_num = (int) dev_id;
	
	g_sdma.ch[ch_num].irq = gpHalGetIrq(ch_num);
	gpHalClearIrq(ch_num);
	wake_up_interruptible(&g_sdma.finish_queue);
	
	return IRQ_HANDLED;
}

/**
* @brief 	SDMA probe function for platform device driver.
* @param 	pdev [in]: Platform device pointer.
* @return: 	IRQ handle state
*/
static int gp_sdma_probe( struct platform_device *pdev)
{
	int ret= -ENOENT;
	struct device *dev = &pdev->dev;
	/* ----- Initial spinlock ----- */
	spin_lock_init(&g_sdma.lock);
	/* ----- Initial semaphore ----- */
	init_MUTEX(&g_sdma.sem);
	/* ----- Initial queue ----- */
	init_waitqueue_head(&g_sdma.wait_queue);
	init_waitqueue_head(&g_sdma.finish_queue);
	/* ----- Register interrupt function ----- */
	ret = request_irq(IRQ_DMAC0_M410_CH0, gp_sdma_irq, IRQF_DISABLED, "SDMA_CH0_IRQ", (void*)0);
	if (ret < 0) 
	{
		DERROR("sdma can't get irq %i, err %d\n", IRQ_DMAC0_M410_CH0, ret);
		return ret;
	}
	ret = request_irq(IRQ_DMAC0_M410_CH1, gp_sdma_irq, IRQF_DISABLED, "SDMA_CH1_IRQ", (void*)1);
	if (ret < 0) 
	{
		DERROR("sdma can't get irq %i, err %d\n", IRQ_DMAC0_M410_CH0, ret);
		return ret;
	}
	/* ----- Register misc device ----- */
	gp_sdma_misc.parent = dev;
	if((ret = misc_register(&gp_sdma_misc))) 
	{
		DERROR("misc_register returned %d in goldfish_audio_init\n", ret);
		free_irq(IRQ_DMAC0_M410_CH0, (void*)0);
		free_irq(IRQ_DMAC0_M410_CH1, (void*)1);
		return ret;
	}
	return 0;
}

/**
* @brief 	SDMA remove function for platform device driver.
* @param 	pdev [in]: Platform device pointer.
* @return: 	IRQ handle state
*/
static int gp_sdma_remove( struct platform_device *pdev)
{
	/* ----- Un-register misc device ----- */
	misc_deregister(&gp_sdma_misc);
	/* ----- Un-register interrupt function ----- */
	free_irq(IRQ_DMAC0_M410_CH0, (void*)0);
	free_irq(IRQ_DMAC0_M410_CH1, (void*)1);
	return 0;
}

/**
 * @brief   device release function 
 * @return  None
 * @see
 */
static void gp_sdma_device_release(struct device *dev)
{
	/* ----- Add your device release procees here ----- */
	return ;
}

#if 0
static SINT32 
driver_use_test(
	void
)
{
	SINT8* src = NULL;
	SINT8* dst = NULL;
	gp_Sdma_t sdma;
	
	src = gp_chunk_malloc(current->tgid, 860 * 480 * 4);
	if (src == NULL) {
		DIAG_ERROR("src alloc error\n");
	}
	src[65536] = 119;
	dst = gp_chunk_malloc(current->tgid, 860 * 480 * 4);
	if (dst == NULL) {
		DIAG_ERROR("src alloc error\n");
	}
	
	sdma.srcAdress = src;
	sdma.dstAdress = dst;
	sdma.size = 860 * 480 * 4;
	if (gp_sdma_memcpy_kernel(&sdma) < 0) {
		DIAG_ERROR("kernel sdma error\n");
	}
	
	DIAG_INFO("data:%d\n", dst[65536]);
	if (src != NULL) {
		gp_chunk_free(src);
	}
	if (dst != NULL) {
		gp_chunk_free(dst);
	}

	return 0;
}
#endif

/**
* @brief 	SDMA driver initial function.
* @return 	SUCCESS/ERROR_ID.
*/
static int __init  gp_sdma_init(void)
{
	int ret;

	ret = platform_driver_register(&gp_sdma_driver);
	if (ret < 0) {
		DERROR("platform_driver_register returned %d\n", ret);
		return ret;
	}

	ret = platform_device_register(&gp_sdma_device);
	if (ret) {
		dev_err(&(gp_sdma_device.dev), "unable to register device: %d\n", ret);
	}
	
	return ret;
}

/**
* @brief 	SDMA driver exit function.
* @return 	None.
*/
static void __exit gp_sdma_exit(void)
{
	platform_device_unregister(&gp_sdma_device);
	platform_driver_unregister(&gp_sdma_driver);
}

module_init(gp_sdma_init);
module_exit(gp_sdma_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
**************************************************************************/

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GPL32900 SDMA Driver");
MODULE_LICENSE_GP;
