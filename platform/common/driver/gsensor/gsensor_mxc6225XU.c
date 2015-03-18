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
#include <linux/delay.h>
#include "gsensor_mxc6225XU.h"


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
static unsigned char detection_val;	//huanggq
static unsigned char state_val;	//huanggq

/*
static void mxc6225XU_write(unsigned char *pdata, int len)
{
	gp_i2c_bus_write(gsensor_data.i2c_handle, pdata, len);
}
*/
static int mxc6225XU_read(unsigned char reg)
{
	unsigned char data;

	data = 0;
	gp_i2c_bus_write(gsensor_data.i2c_handle, &reg, 1);
	gp_i2c_bus_read(gsensor_data.i2c_handle, &data, 1);
	return data;
}

// huanggq
static int mxc6225XU_get_serial_ID(void)
{
	unsigned char reg;
	unsigned char data[3];

	reg = CMD_RD_ID;
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	
	gp_i2c_bus_write(gsensor_data.i2c_handle, &reg, 1);
	gp_i2c_bus_read(gsensor_data.i2c_handle, data, 3);
	printk( "gsensor id: %X-%X-%X", data[2]>>4, data[1], data[0] );
	
	return 0;
	
}
#if 0 //fix warning: 'mxc6225XU_set_detection' defined but not used

//huanggq
static void mxc6225XU_set_detection(unsigned char detection, unsigned char ctrl_code )
{
	unsigned char data[2];

	//clear bits to be updated
	detection_val &= ~ctrl_code;
	//update corresponding bits
	detection_val |= (detection & ctrl_code);

	data[0] = CMD_WR_DET;
	data[1] = detection_val;
	gp_i2c_bus_write(gsensor_data.i2c_handle, data, 2 );
}
#endif

//huanggq
static int mxc6225XU_init(void)
{
/*
ORC[1:0] -- orientation hysteresis time
SHC[3:2] -- sets the shake events time window
SHTH[5:4] -- sets the shake threshold
SHM[6] -- the shake mode
PD[7] -- powers down
*/
	unsigned char data[2];
	
	detection_val = 0x59;	//0101 1001
	data[0] = CMD_WR_DET;
	data[1] = detection_val;
	gp_i2c_bus_write(gsensor_data.i2c_handle, data, 2 ); 
	
	mdelay( 50 );
	mxc6225XU_get_serial_ID();
		
	return 0;
}

//huanggq
static int is_data_ready(void)
{
	state_val = mxc6225XU_read(CMD_RD_STATE);
	if( state_val & 0x80 )
		return 1;

	return 0;
}
static long gsensor_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int temp;
	int ret[2];
	
	switch(cmd)
	{
	case GSENSOR_IOCTL_G_X:
		if(is_data_ready() == 0)
		{
			//printk("gsensor not ready!\n");
			return -EAGAIN;
		}
		temp = mxc6225XU_read(CMD_RD_X);
		//temp = 1;
		if(copy_to_user((void __user *)arg,&temp,sizeof(int)))
			return -EFAULT;
		break;

	case GSENSOR_IOCTL_G_Y:
		if(is_data_ready() == 0)
		{
			//printk("gsensor not ready!\n");
			return -EAGAIN;
		}
		temp = mxc6225XU_read(CMD_RD_Y);  
		//temp = 2;
		if(copy_to_user((void __user *)arg,&temp,sizeof(int)))
			return -EFAULT;
		break;

	//huanggq
	case GSENSOR_IOCTL_G_STATE:
		if(is_data_ready() == 0)
		{
			//printk("gsensor not ready!\n");
			return -EAGAIN;
		}
		
		ret[0] = (state_val & 0x60)>>5;	// shake
		if( (state_val & 0x10) != 0 )
			ret[1] = state_val & 0x03;	// orientation
		else
		  ret[1] = 0;

		//ret[0] = 5;
		//ret[1] = 3;
		if( copy_to_user((void __user *)arg, ret, sizeof(int)*2) )
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
	mxc6225XU_init();
	printk(KERN_ALERT"gsensor mxc6225XU probe ok\n");
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
