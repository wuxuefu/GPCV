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
#include <mach/module.h>
#include <mach/kernel.h>
#include <mach/cdev.h>
#include <mach/gp_i2c_bus.h>
#include "gsensor_mxc6226.h"


/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gsensor_info_s 
{	
	struct miscdevice dev;
	int i2c_handle;
}gsensor_info_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gsensor_info_t gsensor_data;

/*
static void mxc6226_write(unsigned char *pdata, int len)
{
	gp_i2c_bus_write(gsensor_data.i2c_handle, pdata, len);
}
*/
static int mxc6226_read(unsigned char reg)
{
	unsigned char data;

	data = 0;
	gp_i2c_bus_write(gsensor_data.i2c_handle, &reg, 1);
	gp_i2c_bus_read(gsensor_data.i2c_handle, &data, 1);
	return data;
}

static int mxc6226_init(void)
{

	return 0;
}

static int is_data_ready(void)
{
	if(mxc6226_read(CMD_RD_STATE) & 0x80)
		return 1;

	return 0;
}
static long gsensor_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int temp;

	switch(cmd)
	{
	case GSENSOR_IOCTL_G_X:
		if(is_data_ready() == 0)
		{
			//printk("gsensor not ready!\n");
			return -EAGAIN;
		}
		temp = mxc6226_read(CMD_RD_X);
		if(copy_to_user((void __user *)arg,&temp,sizeof(int)))
			return -EFAULT;
		break;

	case GSENSOR_IOCTL_G_Y:
		if(is_data_ready() == 0)
		{
			//printk("gsensor not ready!\n");
			return -EAGAIN;
		}
		temp = mxc6226_read(CMD_RD_Y);  
		if(copy_to_user((void __user *)arg,&temp,sizeof(int)))
			return -EFAULT;
		break;

	default:
		return -EINVAL;	
	}
	return 0;
}

struct file_operations gsensor_fops = 
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl = gsensor_dev_ioctl,
};

static int gp_gsensor_probe(struct platform_device *pdev)
{
	int nRet;

	gsensor_data.i2c_handle = gp_i2c_bus_request(SLAVE_ADDR,I2C_FREQ);
	if(!gsensor_data.i2c_handle)
		return -ENOMEM;

	gsensor_data.dev.name = "gsensor";
	gsensor_data.dev.fops = &gsensor_fops;
	gsensor_data.dev.minor = MISC_DYNAMIC_MINOR;
	
	nRet = misc_register(&gsensor_data.dev);
	if(nRet)
	{
		printk(KERN_ALERT"gsensor probe register faile\n");
		goto err_reg;
	}
	
	platform_set_drvdata(pdev,&gsensor_data);
	mxc6226_init();
	printk(KERN_ALERT"gsensor mxc6226 probe ok\n");
	return 0;

err_reg:
	gp_i2c_bus_release(gsensor_data.i2c_handle);
	return nRet;
}

static int gp_gsensor_remove(struct platform_device *pdev)
{
	misc_deregister(&gsensor_data.dev);
	gp_i2c_bus_release(gsensor_data.i2c_handle);
	return 0;
}


static struct platform_device gp_gsensor_device = 
{
	.name	= "gsensor",
	.id	= -1,
};

static struct platform_driver gp_gsensor_driver = 
{
	.probe		= gp_gsensor_probe,
	.remove		= __devexit_p(gp_gsensor_remove),
	.driver		= 
	{
		.name	= "gsensor",
		.owner	= THIS_MODULE,
	}
};

static int gp_gsensor_init(void)
{
	platform_device_register(&gp_gsensor_device);
	return platform_driver_register(&gp_gsensor_driver);
}

static void gp_gsensor_free(void)
{	
	platform_driver_unregister(&gp_gsensor_driver);	
}

module_init(gp_gsensor_init);
module_exit(gp_gsensor_free);
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus G-Sensor Driver");
MODULE_LICENSE_GP;
MODULE_VERSION("1.0");
