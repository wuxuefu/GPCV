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
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
#include <mach/module.h>
#include <mach/kernel.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <linux/cdev.h>
#include <mach/hal/hal_lbp.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_cache.h>
#include <mach/gp_lbp.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define LBP_TIMEOUT   3000
#define DEBUG_DUMP      0
#define POLLING_TEST	0

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define DERROR	printk
#if DEBUG_DUMP == 1
#define DEBUG	printk
#else
#define DEBUG(...)
#endif

#define DOWN(sem)	if(down_interruptible(sem) != 0) { DERROR("LbpDOWNError\r\n"); return -ERESTARTSYS; }

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct lbp_info_s {
	struct miscdevice dev;          /*!< @brief lbp device */
	struct semaphore sem;           /*!< @brief mutex semaphore for lbp ops */
	wait_queue_head_t done_wait;    /*!< @brief lbp done wait queue */
	bool done;                      /*!< @brief lbp done flag */
	unsigned int open_count;        /*!< @brief lbp device open count */
} lbp_info_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static lbp_info_t *lbp = NULL;

static void
gp_lbp_module_clk_en (
	int en
)
{
	gp_enable_clock( (int*)"LBP", en );
}

static int
lbp_gen_mode(
	lbp_content_t *ctx
)
{
	int ret;
	DOWN(&lbp->sem);
	lbp->done = false;
	ret = gpHalLBPGenMode((unsigned int)gp_user_va_to_pa((void*)ctx->inImg),
							(unsigned int)gp_user_va_to_pa((void*)ctx->outImg),
							(short)ctx->imageWidth, (short)ctx->imageHeight,
							(short)ctx->threshold, ctx->format_mode);
		
	if(ret == 0 && wait_event_timeout(lbp->done_wait, lbp->done, (LBP_TIMEOUT * HZ) / 1000) == 0) {
		DERROR("LBP Timeout (waiting %dms) !!!!!!!!!!!!\n", LBP_TIMEOUT);
		ret = -1;
	}
	
	if (ret == 0)
		gp_invalidate_dcache_range(ctx->outImg, ctx->imageWidth * ctx->imageHeight);
	
	up(&lbp->sem);
	return ret;
}

static int
lbp_auto_mode(
	lbp_content_t *ctx
)
{
	int ret;
	DOWN(&lbp->sem);
	lbp->done = false;
	ret = gpHalLBPAutoMode((unsigned int)gp_user_va_to_pa((void*)ctx->inImg),
							(unsigned int)gp_user_va_to_pa((void*)ctx->outImg),
							(unsigned int)gp_user_va_to_pa((void*)ctx->HammImg),
							(short)ctx->imageWidth, (short)ctx->imageHeight,
							(short)ctx->threshold, ctx->format_mode);

	if(ret == 0 && wait_event_timeout(lbp->done_wait, lbp->done, (LBP_TIMEOUT * HZ) / 1000) == 0) {
		DERROR("LBP Timeout (waiting %dms) !!!!!!!!!!!!\n", LBP_TIMEOUT);
		ret = -1;
	}
	
	if (ret == 0) 
	{
		gp_invalidate_dcache_range(ctx->outImg, ctx->imageWidth * ctx->imageHeight);
		gp_invalidate_dcache_range(ctx->HammImg, ctx->imageWidth * ctx->imageHeight);
	}
	up(&lbp->sem);
	return ret;
}

static int
lbp_cmp_mode(
	lbp_cmp_t *cmp
)
{
	int ret, sum;
	DOWN(&lbp->sem);
	lbp->done = false;
	
	//printk("lbp cmp, src1=%x src2=%x, x=%d y=%d w=%d h=%d x=%d y=%d w=%d h=%d\n", 
	//	cmp->srcImg.pData, cmp->dstImg.pData, cmp->srcRoi.x, cmp->srcRoi.y, cmp->srcRoi.width, cmp->srcRoi.height,
	//	cmp->dstRoi.x, cmp->dstRoi.y, cmp->dstRoi.width, cmp->dstRoi.height);
	ret = gpHalLBPCmpMode((unsigned int)gp_user_va_to_pa((void*)cmp->srcImg.pData),
							(unsigned int)gp_user_va_to_pa((void*)cmp->dstImg.pData),
							cmp->srcImg.width, cmp->srcImg.height, cmp->srcImg.bpl, cmp->srcImg.type,
							cmp->srcRoi, cmp->dstRoi);
	
	if(ret == 0 && wait_event_timeout(lbp->done_wait, lbp->done, (LBP_TIMEOUT * HZ) / 1000) == 0) {
		DERROR("LBP Timeout (waiting %dms) !!!!!!!!!!!!\n", LBP_TIMEOUT);
		ret = -1;
	}
	
	if (ret == 0)
	{
		sum = gpHalLBPGetTotalSum();
		copy_to_user((void __user*)(cmp->sum), &sum, sizeof(sum));
	}
	
	up(&lbp->sem);			
	return ret;
							
}


static irqreturn_t 
lbp_irq_handler(
	int irq, 
	void *dev_id
)
{
	if(gpHalLBPGetIRQStatus()) {
		DEBUG("LBP Done\n");
		lbp->done = true;
		wake_up(&lbp->done_wait);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

/**
 * @brief   LBP device open function
 */
static int 
lbp_open(
	struct inode *ip, 
	struct file *fp
)
{
	DOWN(&lbp->sem);

	if (lbp->open_count == 0) {
		gp_lbp_module_clk_en(1);
	}
	lbp->open_count++;

	up(&lbp->sem);
	return 0;
}

/**
 * @brief  LBP device release function
 */
static int 
lbp_release(
	struct inode *ip, 
	struct file* fp
)
{
	DOWN(&lbp->sem);

	lbp->open_count--;
	if (lbp->open_count == 0) {
		//gp_lbp_module_clk_en(0);
	}
	
	if (fp->private_data) {
		kfree(fp->private_data);
	}

	up(&lbp->sem);
	return 0;
}

/**
 * @brief   LBP device ioctl function
 */
static long 
lbp_ioctl(
	struct file* file, 
	unsigned int cmd, 
	unsigned long arg
)
{
	long ret = 0;
	lbp_content_t ctx;
	lbp_cmp_t cmp;

	switch(cmd) 
	{
		case LBP_IOCTL_GEN:
			DEBUG("LBP_IOCTL_GEN\n");
			if (copy_from_user(&ctx, (void __user*)arg, sizeof(ctx))) {
				ret = -EFAULT;
				break;
			}
			
			ret = lbp_gen_mode(&ctx);
			break;

		case LBP_IOCTL_AUTO:
			DEBUG("LBP_IOCTL_AUTO\n");
			if (copy_from_user(&ctx, (void __user*)arg, sizeof(ctx))) {
				ret = -EFAULT;
				break;
			}
			
			ret = lbp_auto_mode(&ctx);
			break;

		case LBP_IOCTL_CMP:
			DEBUG("LBP_IOCTL_CMP\n");
			if (copy_from_user(&cmp, (void __user*)arg, sizeof(cmp))) {
				ret = -EFAULT;
				break;
			}
			
			ret = lbp_cmp_mode(&cmp);
			break;

		default:
			DEBUG("LBP_IOCTL ERR\n");
			ret = -ENOTTY; /* Inappropriate ioctl for device */
			break;
	}

	return ret;
}

static const struct file_operations lbp_fops = {
	.owner          = THIS_MODULE,
	.open           = lbp_open,
	.release        = lbp_release,
	.unlocked_ioctl = lbp_ioctl,
};


#ifdef CONFIG_PM
static int gp_lbp_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (lbp->open_count > 0) {
		if (down_killable(&lbp->sem) != 0) {
			return 0;
		}
		gp_lbp_module_clk_en(0);
		up(&lbp->sem);
	}
	return 0;
}

static int gp_lbp_resume(struct platform_device *pdev)
{
	if (lbp->open_count > 0) {
		gp_lbp_module_clk_en(1);
	}
	return 0;
}
#else
#define gp_lbp_suspend NULL
#define gp_lbp_resume NULL
#endif

/**
 * @brief   scalar device release
 */
static void gp_lbp_device_release(struct device *dev)
{
	DIAG_INFO("remove lbp device ok\n");
}

static struct platform_device gp_lbp_device = {
	.name	= "gp-lbp",
	.id	= -1,
	.dev	= {
		.release = gp_lbp_device_release,
	}
};

static struct platform_driver gp_lbp_driver = {
	.driver		= {
		.name	= "gp-lbp",
		.owner	= THIS_MODULE,
	},
	.suspend	= gp_lbp_suspend,
	.resume		= gp_lbp_resume,
};

/**
 * @brief   LBP driver init function
 */
static int __init lbp_init(void)
{
	int ret = -ENXIO;

	DEBUG(KERN_WARNING "ModuleInit: lbp \n");
	lbp = (lbp_info_t *)kzalloc(sizeof(lbp_info_t), GFP_KERNEL);
	if (lbp == NULL) {
		ret = -ENOMEM;
		DERROR("LBP kmalloc fail\n");
		goto fail_kmalloc;
	}

	ret = request_irq(IRQ_LBP,
					  lbp_irq_handler,
					  IRQF_SHARED,
					  "LBP_IRQ",
					  lbp);
	if (ret < 0) {
		DERROR("LBP request irq fail\n");
		goto fail_request_irq;
	}

	/* initialize */
	init_MUTEX(&lbp->sem);
	init_waitqueue_head(&lbp->done_wait);

	lbp->dev.name  = "lbp";
	lbp->dev.minor = MISC_DYNAMIC_MINOR;
	lbp->dev.fops  = &lbp_fops;

	/* register device */
	ret = misc_register(&lbp->dev);
	if (ret != 0) {
		DERROR("LBP device register fail\n");
		goto fail_device_register;
	}

	platform_device_register(&gp_lbp_device);
	platform_driver_register(&gp_lbp_driver);

	return 0;

	/* error rollback */
fail_device_register:
	free_irq(IRQ_LBP, lbp);
fail_request_irq:
	kfree(lbp);
	lbp = NULL;
fail_kmalloc:
	return ret;
}

/**
 * @brief   LBP driver exit function
 */
static void __exit lbp_exit(void)
{
	misc_deregister(&lbp->dev);
	free_irq(IRQ_LBP, lbp);
	kfree(lbp);
	lbp = NULL;

	platform_device_unregister(&gp_lbp_device);
	platform_driver_unregister(&gp_lbp_driver);
}

module_init(lbp_init);
module_exit(lbp_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus LBP Driver");
MODULE_LICENSE_GP;
MODULE_VERSION("1.0");
