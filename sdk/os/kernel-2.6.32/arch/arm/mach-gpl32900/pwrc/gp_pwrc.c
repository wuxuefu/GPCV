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
 *  Generalplus Inc.                                                      *
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/

 /**
 * @file gp_pwrc.c
 * @brief pwrc driver interface 
 * @author Daniel Huang
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>

#include <mach/hal/hal_pwrc.h>
#include <mach/gp_pwrc.h>

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define PWRC_NAME	"pwrc"

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gp_pwrc_s {
	struct miscdevice dev;     	 /*!< @brief pwrc device */
	unsigned long isOpened;
} gp_pwrc_t;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_pwrc_t gp_pwrc_data;


/**
 * @brief pwrc handle is valid
 * @return success: 1,  erro: 0
 */
static int valid_handle(int handle)
{
	return (handle == (int)&gp_pwrc_data);
}

/**
 * @brief pwrc request function
 * @return success: pwrc handle,  erro: NULL
 */
int gp_pwrc_request(void)
{
	if(test_and_set_bit(0,&gp_pwrc_data.isOpened))
		return 0;

	return (int)&gp_pwrc_data;
}
EXPORT_SYMBOL(gp_pwrc_request);

/**
 * @brief pwrc release function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_release(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	clear_bit(0,&gp_pwrc_data.isOpened);
	return 0;
}
EXPORT_SYMBOL(gp_pwrc_release);

/**
 * @brief pwrc battery detect enable function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_enable_battery_detect(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	gpHalBatDetEnable(1);
	return 0;
}
EXPORT_SYMBOL(gp_pwrc_enable_battery_detect);

/**
 * @brief pwrc battery detect disable function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_disable_battery_detect(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	gpHalBatDetEnable(0);
	return 0;
}
EXPORT_SYMBOL(gp_pwrc_disable_battery_detect);

/**
 * @brief pwrc battery detect disable function
 * @param handle [in] pwrc handle, [in] battery type
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_battery_select(int handle, int type)
{
	if(!valid_handle(handle))
		return -EINVAL;
	
	if(type==0){
	    gpHalBatSelect(0);
	}else{
	    gpHalBatSelect(1);	
	}
	return 0;
}
EXPORT_SYMBOL(gp_pwrc_battery_select);

/**
 * @brief pwrc battery detect disable function
 * @param handle [in] pwrc handle, [in] operation mode
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_operation_mode_select(int handle, int mode)
{
	if(!valid_handle(handle))
		return -EINVAL;
	
	if(mode==0){
	    gpHalOperationModeSet(0);
	}else{
	    gpHalOperationModeSet(1);	
	}
	return 0;
}
EXPORT_SYMBOL(gp_pwrc_operation_mode_select);

/**
 * @brief pwrc enble function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_enable_dcdc(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	gpHalDcdcEnable(1);
	return 0;
}
EXPORT_SYMBOL(gp_pwrc_enable_dcdc);

/**
 * @brief pwrc disable function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_disable_dcdc(int handle)
{
	if(!valid_handle(handle))
		return -EINVAL;

	gpHalDcdcEnable(0);
	return 0;
}
EXPORT_SYMBOL(gp_pwrc_disable_dcdc);

/**
 * @brief   pwrc device open
 */
static int gp_pwrc_drv_open(struct inode *inode, struct file *file)
{
	if(test_and_set_bit(0,&gp_pwrc_data.isOpened))
		return -EBUSY;
	else
		return 0;
}

/**
 * @brief   pwrc driver release
 */
static int gp_pwrc_drv_release(struct inode *inode, struct file *file)
{
	clear_bit(0,&gp_pwrc_data.isOpened);
	return 0;
}

/**
 * @brief   pwrc driver ioctl
 */
static long gp_pwrc_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	
	switch(cmd)
	{
            	case PWRC_IOCTL_ENABLE_BATTERY_DETECT:
            		if(arg)
            		{
            			gp_pwrc_enable_battery_detect((int)&gp_pwrc_data);
            		}
            		else
            		{
            			gp_pwrc_disable_battery_detect((int)&gp_pwrc_data);
            		}
            		break;
            		
            	case PWRC_IOCTL_BATTERY_SELECT:
            		if(arg)
            		{
            			gp_pwrc_battery_select((int)&gp_pwrc_data, 1);
            		}
            		else
            		{
            			gp_pwrc_battery_select((int)&gp_pwrc_data, 0);
            		}
            		break;            		
            		
            	case PWRC_IOCTL_OPERATION_MODE:      		
            		if(arg)
            		{
            			gp_pwrc_operation_mode_select((int)&gp_pwrc_data, 1);
            		}
            		else
            		{
            			gp_pwrc_operation_mode_select((int)&gp_pwrc_data, 0);
            		}
            		break;
            		
            	case PWRC_IOCTL_ENABLE_DCDC:
            		if(arg)
            		{
            			gp_pwrc_enable_dcdc((int)&gp_pwrc_data);
            		}
            		else
            		{
            			gp_pwrc_disable_dcdc((int)&gp_pwrc_data);
            		}
            		break;            		
	}
	return ret;
}

struct file_operations gp_pwrc_fops = {
	.owner          = THIS_MODULE,
	.open           = gp_pwrc_drv_open,
	.release        = gp_pwrc_drv_release,
	.unlocked_ioctl = gp_pwrc_drv_ioctl,
};

/**
 * @brief   pwrc driver probe
 */
static int __init gp_pwrc_probe(struct platform_device *pdev)
{
	int ret;	
	
	gp_pwrc_data.dev.name = PWRC_NAME;
	gp_pwrc_data.dev.minor  = MISC_DYNAMIC_MINOR;
	gp_pwrc_data.dev.fops  = &gp_pwrc_fops;

	ret = misc_register(&gp_pwrc_data.dev);
	if(ret != 0){
		DIAG_ERROR("watchdog probe register fail\n");
		goto err_reg;
	}

	return 0;
err_reg:
	return ret;
}

/**
 * @brief pwrc dev infomation register function
 * @param pwrc_dev [in] pwrc dev handle
 * @return success: 0,  erro: ERROR_ID
 */
static int gp_pwrc_register_device(void)
{
	return 0;
}

/**
 * @brief pwrc dev infomation unregister function
 * @param pwrc_dev [in] pwrc dev handle
 * @return success: 0,  erro: ERROR_ID
 */
static int gp_pwrc_unregister_device(void)
{
	return 0;
}

/**
 * @brief   pwrc device register
 */
int gp_pwrc_device_register(void)
{
	return gp_pwrc_register_device();
}
EXPORT_SYMBOL(gp_pwrc_device_register);

/**
 * @brief   pwrc device unregister
 */
void gp_pwrc_device_unregister(void)
{
	gp_pwrc_unregister_device();
}
EXPORT_SYMBOL(gp_pwrc_device_unregister);

/**
 * @brief   pwrc driver remove
 */
static int gp_pwrc_remove(struct platform_device *pdev)
{
	misc_deregister(&gp_pwrc_data.dev);
	return 0;
}

/**
 * @brief   pwrc driver define
 */
static struct platform_driver gp_pwrc_driver = {
	.probe	= gp_pwrc_probe,
	.remove	= gp_pwrc_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "gp-pwrc"
	},
};


/**
 * @brief   pwrc device release
 */
static void gp_pwrc_device_release(struct device *dev)
{
	DIAG_INFO("remove PWRC device ok\n");
}

/**
 * @brief   pwrc device resource
 */
static struct platform_device gp_pwrc_device = {
	.name	= "gp-pwrc",
	.id	= -1,
	.dev	= {
			.release = gp_pwrc_device_release,
	},
};

/**
 * @brief   pwrc driver init
 */
static int __init gp_pwrc_drv_init(void)
{
	platform_device_register(&gp_pwrc_device);
	return platform_driver_register(&gp_pwrc_driver);
}

/**
 * @brief   pwrc driver exit
 */
static void __exit gp_pwrc_drv_exit(void)
{
	platform_device_unregister(&gp_pwrc_device);
	platform_driver_unregister(&gp_pwrc_driver);	
}

module_init(gp_pwrc_drv_init);
module_exit(gp_pwrc_drv_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP PWRC Driver");
MODULE_LICENSE_GP;
