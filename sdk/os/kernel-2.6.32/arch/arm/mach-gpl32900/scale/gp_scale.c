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
 * @file    gp_scale.c
 * @brief   Implement of scale driver.
 * @author  qinjian
 * @since   2010/10/9
 * @date    2010/10/9
 */
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/gp_scale.h>
#include <mach/gp_cache.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_cache.h>
#include <mach/gp_line_buffer.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/hal/hal_scale.h>
#include <mach/hal/hal_clock.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/* Scale default timeout (ms) */
#define SCALE_DEFAULT_TIMEOUT   3000

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

#define POLLING_TEST    0
#define DEBUG_DUMP      0

//#define DOWN(sem)   down(sem)
#define DOWN(sem)   if (down_killable(sem) != 0) return -ERESTARTSYS
//#define DOWN(sem)   if (down_interruptible(sem) != 0) return -ERESTARTSYS

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct scale_info_s {
	struct miscdevice dev;          /*!< @brief scale device */
	struct semaphore sem;           /*!< @brief mutex semaphore for scale ops */
	wait_queue_head_t done_wait;    /*!< @brief scaling done wait queue */
	bool done;                      /*!< @brief scaling done flag */
	unsigned int open_count;        /*!< @brief scale device open count */
} scale_info_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

static scale_info_t *scale = NULL;

#if DEBUG_DUMP
/**
 * @brief   Scale parameter dump function
 * @param   ctx [in] scale parameter
 */
static void scale_param_dump(scale_content_t *ctx)
{
	DIAG_PRINTF("src_img.type     = %d\n", ctx->src_img.type);
	DIAG_PRINTF("src_img.width    = %d\n", ctx->src_img.width);
	DIAG_PRINTF("src_img.height   = %d\n", ctx->src_img.height);
	DIAG_PRINTF("src_img.bpl      = %d\n", ctx->src_img.bpl);
	DIAG_PRINTF("src_img.strideUV = %d\n", ctx->src_img.strideUV);
	DIAG_PRINTF("clip_rgn.x       = %d\n", ctx->clip_rgn.x);
	DIAG_PRINTF("clip_rgn.y       = %d\n", ctx->clip_rgn.y);
	DIAG_PRINTF("clip_rgn.width   = %d\n", ctx->clip_rgn.width);
	DIAG_PRINTF("clip_rgn.height  = %d\n", ctx->clip_rgn.height);
	DIAG_PRINTF("dst_img.type     = %d\n", ctx->dst_img.type);
	DIAG_PRINTF("dst_img.width    = %d\n", ctx->dst_img.width);
	DIAG_PRINTF("dst_img.height   = %d\n", ctx->dst_img.height);
	DIAG_PRINTF("dst_img.bpl      = %d\n", ctx->dst_img.bpl);
	DIAG_PRINTF("scale_rgn.x      = %d\n", ctx->scale_rgn.x);
	DIAG_PRINTF("scale_rgn.y      = %d\n", ctx->scale_rgn.y);
	DIAG_PRINTF("scale_rgn.width  = %d\n", ctx->scale_rgn.width);
	DIAG_PRINTF("scale_rgn.height = %d\n", ctx->scale_rgn.height);
}
#endif

static void
gp_scalar_module_clk_en (
	int en
)
{
#ifdef CONFIG_PM
	if( en ) {
		//gpHalScuClkEnable( SCU_C_PERI_SCALING | SCU_C_PERI_2DSCALEABT, SCU_C, 1);
		gp_enable_clock((int*)"2DSCAABT", 1);
		gpHalScuClkEnable( SCU_C_PERI_SCALING, SCU_C, 1);
	}
	else{
		gpHalScuClkEnable( SCU_C_PERI_SCALING, SCU_C, 0);
		gp_enable_clock((int*)"2DSCAABT", 0);
	}
#else
	if( en ) {
		gpHalScuClkEnable( SCU_C_PERI_SCALING | SCU_C_PERI_2DSCALEABT, SCU_C, 1);
	}
	else{
		gpHalScuClkEnable( SCU_C_PERI_SCALING, SCU_C, 0);
	}
#endif
}

/**
 * @brief   Scale do scaling function
 * @param   ctx [in] scale parameter
 * @param   dither [in] scale dither parameter
 * @return  success: 0,  fail: errcode
 * @see
 */
static int scale_trigger(scale_content_t *ctx, scale_dither_t *dither)
{
	int ret = 0;
#ifndef GP_SYNC_OPTION
	unsigned int y_addr, u_addr, v_addr, output_addr;
	unsigned int format = gpHalScaleGetFmtByBmpType(ctx->src_img.type);
#endif

	DOWN(&scale->sem);

#ifndef GP_SYNC_OPTION
	y_addr = (unsigned int)ctx->src_img.pData;
	u_addr = (unsigned int)ctx->src_img.pDataU;
	v_addr = (unsigned int)ctx->src_img.pDataV;
	output_addr = (unsigned int)ctx->dst_img.pData;

	/* translate address from user_va to pa */
	ctx->src_img.pData  = (void *)gp_user_va_to_pa(ctx->src_img.pData);
	if (ctx->src_img.pDataU != NULL) {
		ctx->src_img.pDataU = (void *)gp_user_va_to_pa(ctx->src_img.pDataU);
		ctx->src_img.pDataV = (void *)gp_user_va_to_pa(ctx->src_img.pDataV);
	}

	ctx->dst_img.pData  = (void *)gp_user_va_to_pa(ctx->dst_img.pData);
	DIAG_VERB("[SCALE_IOCTL_TRIGGER] src:%08X  dst:%08X\n", ctx->src_imgpData, ctx->dst_img.pData);
#endif
	/* start scaling */
	ret = gp_line_buffer_request(LINE_BUFFER_MODULE_SCALER, ctx->scale_rgn.width);
	if (ret != 0) {
		DIAG_ERROR("Scalar request line buffer fail: %d\n", ret);
		goto out;
	}

	/* clean dcache */
#ifndef GP_SYNC_OPTION
	gp_clean_dcache_range(y_addr, ctx->src_img.height * ctx->src_img.bpl);
	if ((ctx->src_img.pDataU != NULL)
		&& (format == SE_CFMT_YCbCr420 || format == SE_CFMT_YCbCr422 || format == SE_CFMT_YCbCr444)) {
		unsigned int uv_size = ctx->src_img.height / ((format == SE_CFMT_YCbCr420)?2:1) * ctx->src_img.strideUV;
		gp_clean_dcache_range(u_addr, uv_size);
		gp_clean_dcache_range(v_addr, uv_size);
	}
#else
	GP_SYNC_CACHE();
#endif
	scale->done = false;
	ret = gpHalScaleExec(&ctx->src_img, &ctx->clip_rgn, &ctx->dst_img, &ctx->scale_rgn, dither);
	if (ret != SP_OK) {
		gp_line_buffer_release(LINE_BUFFER_MODULE_SCALER);
		DIAG_ERROR("gpHalScaleExec fail: %d\n", ret);
		ret = -ret;
		goto out;
	}

	if (ctx->timeout == 0) {
		ctx->timeout = SCALE_DEFAULT_TIMEOUT;
	}
#if POLLING_TEST
	DIAG_VERB("Waiting for Scaling Done\n");
	if (HAL_BUSY_WAITING(gpHalScaleDone(), ctx->timeout) >= 0) {
		gpHalScaleClearDone();
		DIAG_VERB("Scaling Done\n");
	}
	else {
		DIAG_ERROR("---------------------------------> Scaler1 Timeout (polling %dms) !!!!!!!!!!!!\n", ctx->timeout);
#if DEBUG_DUMP
		scale_param_dump(ctx);
		gpHalScaleRegDump();
#endif
	}
	gp_line_buffer_release(LINE_BUFFER_MODULE_SCALER);
#else
	if (ctx->timeout != 0xFFFFFFFF) {
		/* waiting for done */
		if (wait_event_timeout(scale->done_wait, scale->done, (ctx->timeout * HZ) / 1000) == 0) {
			ret = -ETIMEDOUT;
			gp_line_buffer_release(LINE_BUFFER_MODULE_SCALER);

			DIAG_ERROR("---------------------------------> Scaler1 Timeout (IRQ %dms) !!!!!!!!!!!!\n", ctx->timeout);
#if DEBUG_DUMP
			scale_param_dump(ctx);
			gpHalScaleRegDump();
#endif
		}
		else {
			/* invalidate dcache */
#ifndef GP_SYNC_OPTION
			#if 1
			gp_invalidate_dcache_range(output_addr + ctx->dst_img.bpl * ctx->scale_rgn.y,
									   ctx->dst_img.bpl * ctx->scale_rgn.height);
			#else
			gp_invalidate_dcache_range(output_addr, ctx->dst_img.bpl * ctx->dst_img.height);
			#endif
#else
			GP_SYNC_CACHE();
#endif
		}
	}
#endif
out:
	up(&scale->sem);
	return ret;
}

/**
 * @brief   Scale irq handler
 */
static irqreturn_t scale_irq_handler(int irq, void *dev_id)
{
	if (gpHalScaleDone()) {
		DIAG_VERB("Scaling Done\n");
		gpHalScaleClearDone();
		scale->done = true;
		wake_up(&scale->done_wait);
		gp_line_buffer_release(LINE_BUFFER_MODULE_SCALER);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

/**
 * @brief   Scale device open function
 */
static int scale_open(struct inode *ip, struct file *fp)
{
	DOWN(&scale->sem);

	if (scale->open_count == 0) {
		//gpHalScaleClkEnable(1);
		gp_scalar_module_clk_en(1);
#if !POLLING_TEST
		gpHalScaleEnableIrq(1);
#endif
	}
	scale->open_count++;

	up(&scale->sem);
	return 0;
}

/**
 * @brief   Scale device release function
 */
static int scale_release(struct inode *ip, struct file* fp)
{
	DOWN(&scale->sem);

	scale->open_count--;
	if (scale->open_count == 0) {
#if !POLLING_TEST
		gpHalScaleEnableIrq(0);
#endif
		gp_scalar_module_clk_en(0);
	}
	if (fp->private_data) {
		kfree(fp->private_data);
	}

	up(&scale->sem);
	return 0;
}

/**
 * @brief   Scale device ioctl function
 */
static long scale_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	switch (cmd) {
	case SCALE_IOCTL_TRIGGER:
		{
			scale_content_t ctx;

			if (copy_from_user(&ctx, (void __user*)arg, sizeof(ctx))) {
				ret = -EFAULT;
				break;
			}

#ifdef GP_SYNC_OPTION
			/* translate address from user_va to pa */
			ctx.src_img.pData  = (void *)gp_user_va_to_pa(ctx.src_img.pData);
			if (ctx.src_img.pDataU != NULL) {
				ctx.src_img.pDataU = (void *)gp_user_va_to_pa(ctx.src_img.pDataU);
				ctx.src_img.pDataV = (void *)gp_user_va_to_pa(ctx.src_img.pDataV);
			}
			ctx.dst_img.pData  = (void *)gp_user_va_to_pa(ctx.dst_img.pData);
			DIAG_VERB("[SCALE_IOCTL_TRIGGER] src:%08X  dst:%08X\n", ctx.src_img.pData, ctx.dst_img.pData);
#endif
			ret = scale_trigger(&ctx, (scale_dither_t *)file->private_data);
		}
		break;

	case SCALE_IOCTL_DITHER:
		{
			scale_dither_t dither;

			if (copy_from_user(&dither, (void __user*)arg, sizeof(dither))) {
				ret = -EFAULT;
				break;
			}

			/* set dither param */
			if (dither.mode > SCALE_DITHER_MODE_HERRDIFF) {
				ret = -EINVAL;
				break;
			}
			if (file->private_data == NULL) {
				file->private_data = kmalloc(sizeof(scale_dither_t), GFP_KERNEL);
				if (file->private_data == NULL) {
					ret = -ENOMEM;
					break;
				}
			}
			memcpy(file->private_data, &dither, sizeof(scale_dither_t));
		}
		break;

	default:
		ret = -ENOTTY; /* Inappropriate ioctl for device */
		break;
	}

	return ret;
}

static const struct file_operations scale_fops = {
	.owner          = THIS_MODULE,
	.open           = scale_open,
	.release        = scale_release,
	.unlocked_ioctl = scale_ioctl,
};


#ifdef CONFIG_PM
static void gp_scalar_suspend_set(void)
{
	if (down_killable(&scale->sem) != 0) {
		return;
	}
	gp_scalar_module_clk_en(0);
	up(&scale->sem);
}

static void gp_scalar_resume_set(void)
{
	gp_scalar_module_clk_en(1);
}

static int gp_scalar_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (scale->open_count > 0) {
		gp_scalar_suspend_set();
	}
	return 0;
}

static int gp_scalar_resume(struct platform_device *pdev)
{
	if (scale->open_count > 0) {
		gp_scalar_resume_set();
	}
	return 0;
}
#else
#define gp_scalar_suspend NULL
#define gp_scalar_resume NULL
#endif

/**
 * @brief   scalar device release
 */
static void gp_scalar_device_release(struct device *dev)
{
	DIAG_INFO("remove scalar device ok\n");
}

static struct platform_device gp_scalar_device = {
	.name	= "gp-scalar",
	.id	= -1,
	.dev	= {
		.release = gp_scalar_device_release,
	}
};

static struct platform_driver gp_scalar_driver = {
	.driver		= {
		.name	= "gp-scalar",
		.owner	= THIS_MODULE,
	},
	.suspend	= gp_scalar_suspend,
	.resume		= gp_scalar_resume,
};

/**
 * @brief   Scale driver init function
 */
static int __init scale_init(void)
{
	int ret = -ENXIO;

	scale = (scale_info_t *)kzalloc(sizeof(scale_info_t), GFP_KERNEL);
	if (scale == NULL) {
		ret = -ENOMEM;
		DIAG_ERROR("scale kmalloc fail\n");
		goto fail_kmalloc;
	}

	ret = request_irq(IRQ_SCALE_ENGINE,
					  scale_irq_handler,
					  IRQF_SHARED,
					  "SCALE_IRQ",
					  scale);
	if (ret < 0) {
		DIAG_ERROR("scale request irq fail\n");
		goto fail_request_irq;
	}

	/* initialize */
	init_MUTEX(&scale->sem);
	init_waitqueue_head(&scale->done_wait);

	scale->dev.name  = "scalar";
	scale->dev.minor = MISC_DYNAMIC_MINOR;
	scale->dev.fops  = &scale_fops;

	/* register device */
	ret = misc_register(&scale->dev);
	if (ret != 0) {
		DIAG_ERROR("scalar device register fail\n");
		goto fail_device_register;
	}

	platform_device_register(&gp_scalar_device);
	platform_driver_register(&gp_scalar_driver);

	return 0;

	/* error rollback */
fail_device_register:
	free_irq(IRQ_SCALE_ENGINE, scale);
fail_request_irq:
	kfree(scale);
	scale = NULL;
fail_kmalloc:
	return ret;
}

/**
 * @brief   Scale driver exit function
 */
static void __exit scale_exit(void)
{
	misc_deregister(&scale->dev);
	free_irq(IRQ_SCALE_ENGINE, scale);
	kfree(scale);
	scale = NULL;

	platform_device_unregister(&gp_scalar_device);
	platform_driver_unregister(&gp_scalar_driver);
}

module_init(scale_init);
module_exit(scale_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus Scaling Engine Driver");
MODULE_LICENSE_GP;
