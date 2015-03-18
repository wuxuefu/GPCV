/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Technology Co., Ltd.         *
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
 * @file gp_wdt.c
 * @brief watchdog driver interface 
 * @author zaimingmeng
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>

#include <mach/hal/hal_wdt.h>
#include <mach/hal/hal_clock.h>
#include <mach/gp_wdt.h>

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define WDT_NAME	"watchdog"

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gp_wdt_s {
	struct miscdevice dev;     	 /*!< @brief wdt device */

	unsigned int period;		 /*!< @brief timeout period */

	unsigned long isOpened;

} gp_wdt_t;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_wdt_t gp_wdt_data;
static int g_wdt_regSave = 0;

/**
 * @brief wdt handle is valid
 * @return success: 1,  erro: 0
 */
static int valid_handle(int handle)
{
	return (handle == (int)&gp_wdt_data);
}

/**
 * @brief wdt request function
 * @return success: wdt handle,  erro: NULL
 */
int gp_wdt_request(void)
{
	if(test_and_set_bit(0,&gp_wdt_data.isOpened))
		return 0;
		
	gpHalScuClkEnable(SCU_B_PERI_WDT, SCU_B, 1);
	return (int)&gp_wdt_data;
}
EXPORT_SYMBOL(gp_wdt_request);

/**
 * @brief wdt release function
 * @param handle [in] wdt handle
 * @return success: 0,  erro: erro_id
 */
int gp_wdt_release(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	clear_bit(0,&gp_wdt_data.isOpened);
	gp_wdt_disable((int)&gp_wdt_data);
	gpHalScuClkEnable(SCU_B_PERI_WDT, SCU_B, 0);
	return 0;
}
EXPORT_SYMBOL(gp_wdt_release);

/**
 * @brief wdt enable function
 * @param handle [in] wdt handle
 * @return success: 0,  erro: erro_id
 */
int gp_wdt_enable(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;
	gpHalWdtEnable(1);
	return 0;
}
EXPORT_SYMBOL(gp_wdt_enable);

/**
 * @brief wdt disable function
 * @param handle [in] wdt handle
 * @return success: 0,  erro: erro_id
 */
int gp_wdt_disable(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	gpHalWdtEnable(0);
	return 0;
}
EXPORT_SYMBOL(gp_wdt_disable);

/**
 * @brief set watchdog timer timeout period function
 * @param handle [in] wdt handle
 * @param period [in] wdt reset period in seconds
 * @return success: 0,  erro: erro_id
 */
int gp_wdt_set_timeout(int handle,int period)
{
	unsigned long freq;
	unsigned long count;
	unsigned long prescale = 1;

	if(!valid_handle(handle))
		return -EINVAL;

	freq = gpHalWdtGetBaseClk();

	count = 0xfffffffful/freq;

	if((period < 1)||(period > count))
		return -EINVAL;
	gp_wdt_data.period = period;

	count = period*freq;

	prescale = (uint16_t)(count>>16);
	
	if(prescale){
		count = count/(prescale + 1);
	}

	gpHalWdtSetPrescale(prescale);

	gpHalWdtSetLoad(count);

	return 0;
}
EXPORT_SYMBOL(gp_wdt_set_timeout);

/**
 * @brief get watchdog timer timeout period function
 * @param handle [in] wdt handle
 * @param period [out] wdt reset period in seconds
 * @return success: 0,  erro: erro_id
 */
int gp_wdt_get_timeout(int handle,int *period)
{
	if(!valid_handle(handle))
		return -EINVAL;

	*period = gp_wdt_data.period;
	return 0;
}
EXPORT_SYMBOL(gp_wdt_get_timeout);

/**
 * @brief feed watchdog,prevent watchdog reset
 * @param handle [in] wdt handle
 * @return success: 0,  erro: erro_id
 */
int gp_wdt_keep_alive(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	gpHalWdtKeepAlive();
	return 0;
}
EXPORT_SYMBOL(gp_wdt_keep_alive);

/**
 * @brief watchdog force reset
 * @param handle [in] wdt handle
 * @return success: 0,  erro: erro_id
 */
int gp_wdt_force_reset(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	gpHalWdtForceReset();
	return 0;
}
EXPORT_SYMBOL(gp_wdt_force_reset);

/**
 * @brief   wdt device open
 */
static int gp_wdt_drv_open(struct inode *inode, struct file *file)
{
	if(gp_wdt_request()==0)
		return -EBUSY;
	else
		return 0;
}

/**
 * @brief   wdt driver release
 */
static int gp_wdt_drv_release(struct inode *inode, struct file *file)
{
	gp_wdt_release((int)&gp_wdt_data);
	return 0;
}

/**
 * @brief   wdt driver ioctl
 */
static long gp_wdt_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int temp;

	switch(cmd){
	case WDT_IOCTL_CTRL:
		if(arg)
			gp_wdt_enable((int)&gp_wdt_data);
		else
			gp_wdt_disable((int)&gp_wdt_data);
		break;

	case WDT_IOCTL_KEEPALIVE:
		gp_wdt_keep_alive((int)&gp_wdt_data);
		break;

	case WDT_IOCTL_FORCERESET:
		gp_wdt_force_reset((int)&gp_wdt_data);
		break;

	case WDT_IOCTL_SETTIMEROUT:
		gp_wdt_set_timeout((int)&gp_wdt_data,arg);
		break;

	case WDT_IOCTL_GETTIMEROUT:
		gp_wdt_get_timeout((int)&gp_wdt_data,&temp);
		if(copy_to_user((void __user *)arg,&temp,sizeof(int))){
			ret = -EFAULT;
		}
		break;
	}

	return ret;
}

struct file_operations gp_wdt_fops = {
	.owner          = THIS_MODULE,
	.open           = gp_wdt_drv_open,
	.release        = gp_wdt_drv_release,
	.unlocked_ioctl = gp_wdt_drv_ioctl,
};

/**
 * @brief   wdt driver probe
 */
static int __init gp_wdt_probe(struct platform_device *pdev)
{
	int ret;


	gp_wdt_data.dev.name = WDT_NAME;
	gp_wdt_data.dev.minor  = MISC_DYNAMIC_MINOR;
	gp_wdt_data.dev.fops  = &gp_wdt_fops;

	ret = misc_register(&gp_wdt_data.dev);
	if(ret != 0){
		DIAG_ERROR("watchdog probe register fail\n");
		goto err_reg;
	}

	return 0;
err_reg:
	return ret;
}

void gp_wdt_suspend_set( void ){
	int* ptr; 
	if( g_wdt_regSave == 0 && gpHalWdtEnGet() ) {
		ptr = kmalloc( WDT_REGISTER_OFFSET, GFP_KERNEL);
		gpHalWdtRegSave( ptr );
		g_wdt_regSave = (int)ptr;
		if( ptr == NULL ) {
			printk("[%s][%d]Suspend Error, it allocates memory fail\n", __FUNCTION__, __LINE__);
		}
	}
	else {
		//printk("[%s][%d], WDT PTR[%x] EN[%d]\n", __FUNCTION__, __LINE__,  
			//   g_wdt_regSave, gpHalWdtEnGet());
	}
}
EXPORT_SYMBOL(gp_wdt_suspend_set);

void gp_wdt_resume_set( void ){
	int* ptr; 
	if( g_wdt_regSave != 0 ) {
		ptr = (int *)g_wdt_regSave;
		gpHalWdtRegRestore( ptr );
		kfree( ptr );
		g_wdt_regSave = 0;
	}
	else {
		//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	}
}
EXPORT_SYMBOL(gp_wdt_resume_set);

#ifdef CONFIG_PM
static int gp_wdt_suspend(struct platform_device *pdev, pm_message_t state){
	gp_wdt_suspend_set();
	return 0;
}

static int gp_wdt_resume(struct platform_device *pdev){
	gp_wdt_resume_set();
	return 0;
}
#else
#define gp_wdt_suspend NULL
#define gp_wdt_resume NULL
#endif

/**
 * @brief   wdt driver remove
 */
static int gp_wdt_remove(struct platform_device *pdev)
{
	gp_wdt_disable((int)&gp_wdt_data);
	misc_deregister(&gp_wdt_data.dev);
	return 0;
}

/**
 * @brief   wdt driver define
 */
static struct platform_driver gp_wdt_driver = {
	.probe	= gp_wdt_probe,
	.remove	= gp_wdt_remove,
	.suspend = gp_wdt_suspend,
	.resume = gp_wdt_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "gp-wdt"
	},
};


/**
 * @brief   wdt device release
 */
static void gp_wdt_device_release(struct device *dev)
{
	DIAG_INFO("remove watchdog device ok\n");
}

/**
 * @brief   wdt device resource
 */
static struct platform_device gp_wdt_device = {
	.name	= "gp-wdt",
	.id	= -1,
	.dev	= {
			.release = gp_wdt_device_release,
	},
};

/**
 * @brief   wdt driver init
 */
static int __init gp_wdt_drv_init(void)
{
	platform_device_register(&gp_wdt_device);
	return platform_driver_register(&gp_wdt_driver);
}

/**
 * @brief   wdt driver exit
 */
static void __exit gp_wdt_drv_exit(void)
{
	platform_device_unregister(&gp_wdt_device);
	platform_driver_unregister(&gp_wdt_driver);	
}

module_init(gp_wdt_drv_init);
module_exit(gp_wdt_drv_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP Watchdog Driver");
MODULE_LICENSE_GP;
