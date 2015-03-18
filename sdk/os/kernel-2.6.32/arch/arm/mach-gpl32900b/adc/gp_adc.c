/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2011 by Generalplus Technology Co., Ltd.         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Technology Co., Ltd. All rights are reserved by Generalplus Technology*
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Technology Co., Ltd. reserves the right to modify this    *
 *  software without notice.                                              *
 *                                                                        *
 *  Generalplus Technology Co., Ltd.                                      *
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/

/**
 * @file arch/arm/mach-spmp8050/gp_adc.c
 * @brief ADC device driver
 * @author
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/gp_adc.h>
#include <mach/hal/hal_adc.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/hal/regmap/reg_adc.h>
#include <mach/hal/regmap/reg_scu.h>

/* This driver is designed to control the usage of the ADC block between
 * the touchscreen and any other drivers that may need to use it, such as
 * the sar key driver.
 *
 * Each user registers to get a client block which uniquely identifies it
 * and stores information such as the necessary functions to callback when
 * action is required.
 * SAR ADC can generate 125ksps
 */
/**************************************************************************
 *                          C O N S T A N T S                          *
 **************************************************************************/
#define DELAY_US	50 /* 50us to make ADC stable, and readable */

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
/**
 * @brife the ADC client data struct
 */
typedef struct gp_adc_client_s {
	unsigned int		channel;/*!<current channel of this client*/
	unsigned int		is_ts;	/*!<is this client an touch screen?*/
	/*!<conversion end callback function, need for touch screen client*/
	void	(*convert_cb)(int handle, unsigned val, unsigned event);
	void *dev;
	int value;
} gp_adc_client_t;

/**
 *@brife the ADC device data struct
 */
typedef struct gp_adc_device_s {
	struct miscdevice dev;      /*!< @brief gpio device */
	struct platform_device	*pdev;
	struct gp_adc_client_s	*ts;	/*!< touch screen client*/
	struct semaphore lock;
	int aux_ch;
	int aux_data;
	struct semaphore aux_sem;           /*!< @brief mutex semaphore for scale ops */
	unsigned int open_cnt;
	unsigned int close_cnt;
} gp_adc_device_t;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

static struct gp_adc_device_s *g_adc_dev = 0;
static struct semaphore g_adc_prepare;



/*************************************************************************/

static unsigned long get_open_count( gp_adc_device_t *padc )
{
	return padc->open_cnt - padc->close_cnt;
}

/**
 * @brief   Adc clock enable/disable
 * @param   enable [in] 0:disable, 1:enable
 * @return  None
 * @see
 */
static void adc_clock_enable(int enable)
{
	gp_enable_clock( (int*)"SAACC", enable );
}

static void gp_adc_en ( int en )
{
	adc_init_t adc_init;
	
	adc_clock_enable(en);

	if( en ) {
		/*setup adc default settings*/
		adc_init.clk_tog_en = 0;	/* 1-always toggling clock,0-toggling only in measurement */
		adc_init.conv_dly = 0x40;	/* 2 frame time */
		adc_init.chkdly = 0;
		adc_init.x2y_dly = 0x20;	/* a frame time */
		adc_init.interval_dly = 0x800*10; /* about 2msx5 */
		adc_init.debounce_dly = 0x1000;   /* about 2ms */
		adc_init.clock_rate = 800000ul; /* 800KHz */
		adc_init.pclk_rate = 189000000;
		gpHalAdcInit(&adc_init);
	}
}

static int gp_adc_start_aux(int handle, unsigned int channel)
{
	struct gp_adc_client_s *client = (struct gp_adc_client_s *) handle;
	gp_adc_device_t *dev = (gp_adc_device_t *)client->dev;
	client->channel = channel;

	down(&dev->lock);
	//if (dev->aux_ch != channel)
	//gpHalAdcStartConv(MODE_AUX, channel); /* start aux mode */
	dev->aux_ch = channel;
	up(&dev->lock);

	return 0;
}

/**
 *@brief  start specified channel ADC conversion
 *@param  handle: handle of the adc client
 *@param  channel: specify which channel as AUX ADC input
 *   This parameter can be one of the following values:
 *     @arg ADC_CHANNEL_TPXP: touch pannle X positive input
 *     @arg ADC_CHANNEL_TPXN: touch pannle X negative input
 *     @arg ADC_CHANNEL_TPYP: touch pannle Y positive input
 *     @arg ADC_CHANNEL_TPYN: touch pannle Y negative input
 *     @arg ADC_CHANNEL_AUX1: AUX1 as ADC input
 *     @arg ADC_CHANNEL_AUX2: AUX2 as ADC input
 *     @arg ADC_CHANNEL_AUX3: AUX3 as ADC input
 *     @arg ADC_CHANNEL_AUX4: AUX4 as ADC input
 *@return 0-OK
 *        -EINVAL invalid parameter
 *        -EAGAIN client have alreay started
 */
int gp_adc_start(int handle, unsigned int channel)
{
	struct gp_adc_client_s *client = (struct gp_adc_client_s *)handle;
	gp_adc_device_t *dev = (gp_adc_device_t *)client->dev;

	//printk("[%s] enter\n", __func__);
	if (NULL==dev || NULL==client) {
		DIAG_ERROR("%s: failed to find adc or client\n", __func__);
		return -EINVAL;
	}

	/*only one touch screen client can be started*/
	if (client->is_ts && dev->ts) {
		DIAG_INFO("adc touchscreen client already started!\n");
		return -EAGAIN;
	}

	if ( get_open_count(dev) == 0 )
		gp_adc_en(1); /* enable ADC */

	dev->open_cnt++;
	if (client->is_ts) { /*ts client*/
		dev->ts = client;
		/* Work around for adc auto mode */
		gpHalAdcStartConv(MODE_AUX, 0x7);

		/* if it is touch panel mode, enable interrupt */
		gpHalAdcSetIntEn(ADC_INTPNL | ADC_INTPENUP | ADC_INTPENDN, ADC_INTPNL);
		gpHalAdcStartConv(MODE_TP_AUTO, 0);
	} else {
		gpHalAdcSetIntEn(ADC_INTAUX, ADC_INTAUX);
		gp_adc_start_aux(handle, channel);
	}

	return 0;
}

EXPORT_SYMBOL(gp_adc_start);

/**
 *@brief  manual stop specified AD client conversion
 *@param  handle: handle of the adc client
 *@return none
 */
int gp_adc_stop(int handle)
{
	struct gp_adc_client_s *client = (struct gp_adc_client_s *)handle;
	gp_adc_device_t *dev = (gp_adc_device_t *)client->dev;

	if (dev->ts == client) { /*touch screen client*/
		dev->ts = NULL;
		gpHalAdcStopConv(MODE_TP_AUTO);
		gpHalAdcSetIntEn(ADC_INTPNL | ADC_INTPENUP | ADC_INTPENDN, 0);
	}

	dev->close_cnt++; /* increase the close count */

	if( get_open_count(dev) == 0 )
		adc_clock_enable(0);

	return 0;
}
EXPORT_SYMBOL(gp_adc_stop);

/**
 *@brief  Returns the last ADC conversion result data for AUX channel.
 *@param  handle: handle of the adc resource
 *@return -ETIMEOUT timeout, no valid result
 *        >=0, AD result
 */
int gp_adc_read(int handle)
{
	struct gp_adc_client_s *client = (struct gp_adc_client_s *) handle;
	gp_adc_device_t *dev = (gp_adc_device_t *)client->dev;
	short val;
	
	if(down_timeout(&dev->lock,msecs_to_jiffies(300))){
		printk("adc read time out, return last value channel=%d\n",client->channel);
		return (client->value + 512);
	}

	//mutex_lock(&dev->lock);
#if 0
	//printk("channel = %d, aux_ch = %d\n",client->channel, dev->aux_ch);
	//if (client->channel != dev->aux_ch) {
		gpHalAdcStartConv(MODE_AUX, client->channel); /* start aux mode */
		dev->aux_ch = client->channel;
		udelay(DELAY_US);		// add by chenhn  2011/10/24  for the stable of ADC
	//}

	val = (short) gpHalAdcGetAUX(); /* read data */
#else
	down(&g_adc_prepare);
	gpHalAdcStartConv(MODE_AUX, client->channel); /* start aux mode */
	dev->aux_ch = client->channel;
	down_interruptible(&dev->aux_sem);
	val = dev->aux_data;
	client->value = val;
	up(&g_adc_prepare);
#endif

	up(&dev->lock);
	//printk("val = %x\n",val);
	return (val + 512);
}

EXPORT_SYMBOL(gp_adc_read);

/**
 *@brief  Returns the last ADC conversion result data for AUX channel.
 *@param  handle: handle of the adc resource
 *@param  to_jiffies: timeout (number of jiffies)
 *@return -ETIMEOUT timeout, no valid result
 *        >=0, AD result
 */
int gp_adc_read_timeout(int handle, unsigned int to_jiffies)
{
	return gp_adc_read( handle );
}
EXPORT_SYMBOL(gp_adc_read_timeout);

/**
 *@brife request an ADC client
 *@param is_ts[in]: 0-this client is not touch screen, others-touch screen client
 *@param ts_cb[in]: callback function for touch screen conversion end,
 *                  not used for none touch screent client
 *@return -ENOMEM no memory for new client, request failed
 *        others: handle of the new adc client
 */

int gp_adc_request(
	unsigned int is_ts,
	void (*ts_cb)(int client,  unsigned int val, unsigned int event)
)
{
	struct gp_adc_client_s *client;

	client = kzalloc(sizeof(struct gp_adc_client_s), GFP_KERNEL);
	if (!client) {
		DIAG_ERROR("no memory for adc client\n");
		return -ENOMEM;
	}

	/*init client data*/
	client->is_ts = is_ts;
	client->convert_cb = is_ts ? ts_cb : 0;
	client->dev = g_adc_dev;
	client->value = 0;

	return (int)client;
}
EXPORT_SYMBOL(gp_adc_request);

/**
 *@brife release ADC client
 *@param handle[in]: the ADC client to release
 *@return none
 */
void gp_adc_release(int handle)
{
	struct gp_adc_client_s *client = (struct gp_adc_client_s *)handle;

	gp_adc_stop(handle);
	kfree(client);
}
EXPORT_SYMBOL(gp_adc_release);

/*
 *adc interrupt handler
 */
static irqreturn_t gp_adc_irq(int irq, void *pw)
{
	struct gp_adc_device_s *adc = pw;
	unsigned int int_flags, data, x, y;
	static int bIntPendown = 0;

	int_flags = gpHalAdcGetIntFlag();

	if(int_flags & ADC_INTAUX){
		//printk("[%s] ADC_INTAUX occurs.\n", __func__);
		data = gpHalAdcGetAUX();
		adc->aux_data = data;
		gpHalAdcSetIntEn(ADC_INTAUX, ADC_INTAUX);
		up(&adc->aux_sem);
		// down_interruptible
	}
	else{
		data = gpHalAdcGetPNL();
		x = data & 0xffff;
		y = (data & 0xffff0000) >> 16;
		x = x + 512;
		y = y + 512;
		data = (y << 16) | x;
		/* only process touch panel IRQ */
		if(adc->ts && adc->ts->convert_cb) {
			/*PNL interrupt*/
			if(int_flags & ADC_INTPNL) {
				if( !bIntPendown ) {
					bIntPendown = 1;
					gpHalAdcSetIntEn(ADC_INTPENUP, ADC_INTPENUP);
				}
				(adc->ts->convert_cb)((int)adc->ts, data, ADC_INTPNL);
			}
			/*Pen up interrupt*/
			if(int_flags & ADC_INTPENUP) {
				if( bIntPendown ) {
					bIntPendown = 0;
					gpHalAdcSetIntEn(ADC_INTPENUP, 0);
					(adc->ts->convert_cb)((int)adc->ts, 0, ADC_INTPENUP);
				}
			}
			/*pen down, only in touch screen manual mode*/
			if(int_flags & ADC_INTPENDN) {
				gpHalAdcStartConv(MODE_TP_MANUAL, 1);
			}
		}
	}
	return IRQ_HANDLED;
}


/*
 *@brife /dev/adc handling (file operations)
 */
ssize_t gp_adc_fops_read(struct file* file, char __user *user,size_t len,loff_t* pos)
{
	int client = (int)(file->private_data);
	int temp;

	if(len<2) {/*user buffer size too small */
		return -EFAULT;
	}
	/*may blocking*/
	temp = gp_adc_read(client);

	if( temp<0 )
		return temp;

	len = 2;
	if(copy_to_user(user, &temp, len)) {
		return -EFAULT;
	}
	return len;
}

static int gp_adc_fops_open(struct inode *inode, struct file *file)
{
	int client;

	client = gp_adc_request(0,NULL);
	if(IS_ERR((void*)client))
		return client;

	file->private_data = (void*)client;
	return nonseekable_open(inode, file);
}

static int gp_adc_fops_release(struct inode *inode, struct file *file)
{
	int client = (int)file->private_data;

	gp_adc_release(client);
	return 0;
}

static long gp_adc_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int client = (int)file->private_data;
	long ret = -ENOTTY;

	switch (cmd) {
	case IOCTL_GP_ADC_START:
		ret = gp_adc_start(client, arg);	/* 'arg' is used as channel*/
		break;
	case IOCTL_GP_ADC_STOP:
		ret = gp_adc_stop(client);
		break;
	default:
		break;
	}
	return ret;
}

static struct file_operations gp_adc_fops = {
	.owner		= THIS_MODULE,
	.read		= gp_adc_fops_read,
	.open		= gp_adc_fops_open,
	.release		= gp_adc_fops_release,
	.unlocked_ioctl = gp_adc_fops_ioctl,
};
/*
 *device driver probe
 */
int *gAdcRegBackup = NULL;
static int gp_adc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gp_adc_device_s *adc;
	int ret;

	adc = kzalloc(sizeof(struct gp_adc_device_s), GFP_KERNEL);
	if (adc == NULL) {
		DIAG_ERROR("failed to allocate adc device\n");
		return -ENOMEM;
	}
	g_adc_dev = adc;

	sema_init(&adc->lock,1);
	sema_init(&g_adc_prepare,1);
	sema_init(&adc->aux_sem, 0);
	adc->pdev = pdev;
	adc->aux_ch = -1; /* not starting aux */
	ret = request_irq( IRQ_SAACC, gp_adc_irq, 0, dev_name(dev), adc);
	if (ret < 0) {
		DIAG_ERROR( "failed to attach adc irq\n");
		goto err_alloc;
	}

	/* register misc device */
	adc->dev.name  = "adc";
	adc->dev.minor = MISC_DYNAMIC_MINOR;
	adc->dev.fops  = &gp_adc_fops;
	adc->open_cnt = 0;
	ret = misc_register(&adc->dev);
	if (ret != 0) {
		DIAG_ERROR("gp_adc device register fail\n");
		goto err_irq;
	}

	platform_set_drvdata(pdev, adc);

	if(gAdcRegBackup == NULL)
		gAdcRegBackup = (int *)kmalloc(sizeof(regADC_t), GFP_KERNEL);
	return 0;

err_irq:
	free_irq(IRQ_SAACC, adc);

err_alloc:
	kfree(adc);
	return ret;
}

static int gp_adc_remove(struct platform_device *pdev)
{
	struct gp_adc_device_s *adc = platform_get_drvdata(pdev);

	adc_clock_enable(0);
	kfree(adc);

	return 0;
}

#ifdef CONFIG_PM
static void gp_adc_reg_dump(void)
{
	regADC_t *pAdc = (regADC_t *)(LOGI_ADDR_ADC_REG);

	printk("pAdc->AUTODLY=%x \n",pAdc->AUTODLY);
	printk("pAdc->SARCTRL =%x \n",pAdc->SARCTRL);	/** +00 SARADC control register*/
	printk("pAdc->CONDLY =%x \n",pAdc->CONDLY);		/** conversion delay time*/
	printk("pAdc->AUTODLY =%x \n",pAdc->AUTODLY);	/** auto conversion mode x-y delay and sample interval*/
	printk("pAdc->DEBTIME =%x \n",pAdc->DEBTIME);	/** pen down interrupt de-bounce time*/
	printk("pAdc->PNL =%x \n",pAdc->PNL);		/** touch panel x/y position*/
	printk("pAdc->AUX =%x \n",pAdc->AUX);		/** auxiliary measurement data*/
	printk("pAdc->INTEN =%x \n",pAdc->INTEN);		/** interrupt enable*/
	printk("pAdc->INTF =%x \n",pAdc->INTF);		/** interrupt flags*/

}

static void gp_adc_reg_copy(int *pDst, int *pSrc, int regNum)
{
	int idx;
	for(idx = 0; idx<regNum; idx++){
		*pDst = *pSrc;
		pSrc++;
		pDst++;
	}
	//gp_adc_reg_dump();
}

static void gp_adc_reg_backup(void)
{
	regADC_t *pAdc = (regADC_t *)(LOGI_ADDR_ADC_REG);
	gp_adc_reg_copy((int *)gAdcRegBackup,(int *)pAdc,sizeof(regADC_t)/4);
}

static void gp_adc_reg_restore(void)
{
	regADC_t *pAdc = (regADC_t *)(LOGI_ADDR_ADC_REG);
	gp_adc_reg_copy((int *)pAdc,(int *)gAdcRegBackup,sizeof(regADC_t)/4);

}

static int gp_adc_suspend(struct platform_device *pdev, pm_message_t state)
{
	down(&g_adc_prepare);
	gp_adc_reg_backup();
	//gp_adc_en(0); /* stop adc clock */
	return 0;
}

static int gp_adc_resume(struct platform_device *pdev)
{
	//gp_adc_en(1); /* start adc clock */
	gp_adc_en(1);
	gp_adc_reg_restore();
	up(&g_adc_prepare);
	return 0;
}

#else
#define gp_adc_suspend NULL
#define gp_adc_resume NULL
#endif

/**
 * @brief adc device release
 */
static void gp_adc_device_release(struct device *dev)
{
	DIAG_INFO("remove adc device ok\n");
}

static struct platform_device gp_adc_device = {
	.name	= "gp-adc",
	.id	= -1,
	.dev	= {
		.release = gp_adc_device_release,
	}
};

static struct platform_driver gp_adc_driver = {
	.probe		= gp_adc_probe,
	.remove		= __devexit_p(gp_adc_remove),
	.suspend		= gp_adc_suspend,
	.resume		= gp_adc_resume,
	.driver		= {
		.name	= "gp-adc",
		.owner	= THIS_MODULE,
	},
};

static int __init gp_adc_module_init(void)
{
	int ret;

	platform_device_register(&gp_adc_device);
	ret = platform_driver_register(&gp_adc_driver);
	if (ret)
		DIAG_ERROR("%s: failed to add adc driver\n", __func__);

	return ret;
}

static void __exit gp_adc_module_exit(void)
{
	platform_device_unregister(&gp_adc_device);
	platform_driver_unregister(&gp_adc_driver);
}

module_init(gp_adc_module_init);
module_exit(gp_adc_module_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP ADC Driver");
MODULE_LICENSE_GP;

