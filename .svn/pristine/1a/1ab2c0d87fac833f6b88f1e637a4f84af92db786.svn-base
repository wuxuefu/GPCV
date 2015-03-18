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
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/hardware.h>

#include <mach/hal/hal_i2c_bus.h>
#include <mach/gp_i2c_bus.h>
#include <mach/gp_board.h>

//----------------------------------
#include <linux/init.h>
#include <linux/fs.h> 
#include <mach/gp_display.h>
#include <mach/general.h>

#include <mach/gp_gpio.h>
#include <linux/delay.h> 
#include <mach/hal/hal_gpio.h>

#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#endif
#define TI2C_MODE 0
#define I2C_MODE 1
#define I2C_DEV  I2C_MODE
/*************************************************
MAC define
*************************************************/
#define ERROR(fmt, arg...) printk( "[%s:%d] Error! "fmt, __FUNCTION__, __LINE__, ##arg)
#define RETURN(x)		{ret = x; goto Return;}
#define CHECK_(x, msg, errid) if(!(x)) {ERROR("%s, %s\n", msg, #x); RETURN(errid);}
#define CHECK(x)		CHECK_(x, "Check failed", -1)
#define CHECK_PRG(x)	CHECK_(x, "Program Error", -EIO)
#define CHECK_VAL(x)	CHECK_(x, "Value Error", -1)



#define ASENSOR_IOCTL_ID	'G'
#define ASENSOR_IOCTL_GET_INT_ACTIVE	_IOR(ASENSOR_IOCTL_ID, 0x01, int)
#define ASENSOR_IOCTL_SET_SENSITIVE	_IOW(ASENSOR_IOCTL_ID, 0x02, int)
#define ASENSOR_IOCTL_PARK_MODE		_IOW(ASENSOR_IOCTL_ID, 0x04, int)
#define ASENSOR_IOCTL_PARK_MODE_INT	_IOR(ASENSOR_IOCTL_ID, 0x08, int)
#define ASENSOR_IOCTL_GET_G_VALUE	_IOR(ASENSOR_IOCTL_ID, 0x10, int)

#define DA380_SLAVE_ID 0x4E
#define A_SENSOR_ADDR DA380_SLAVE_ID
#define A_SENSOR_CLK 10


#define SOFT_RESET    0x00
#define CHIPID        0x01

#define ACC_X_LSB     0x02
#define ACC_X_MSB     0x03

#define ACC_Y_LSB     0x04
#define ACC_Y_MSB     0x05

#define ACC_Z_LSB     0x06
#define ACC_Z_MSB     0x07

#define MOTION_FLAG   0x09
#define NEWDATA_FLAG  0x0A

#define TAP_ACTIVE_STATUS 0x0B  

#define RESOLUTION_RANGE 0x0F  // Resolution bit[3:2] -- 00:14bit 
                               //                        01:12bit 
                               //                        10:10bit
                               //                        11:8bit 
                               
                               // FS bit[1:0]         -- 00:+/-2g 
                               //                        01:+/-4g 
                               //                        10:+/-8g
                               //                        11:+/-16g 
#define ODR_AXIS      0x10 
#define MODE_BW       0x11                             
#define SWAP_POLARITY 0x12
#define INT_SET1      0x16
#define INT_SET2      0x17
#define INT_MAP1      0x19 
#define INT_MAP2      0x1A
#define INT_CONFIG    0x20 
#define INT_LATCH     0x21
#define FREEFALL_DUR  0x22
#define FREEFALL_THS  0x23
#define FREEFALL_HYST 0x24
#define ACTIVE_DUR    0x27
#define ACTIVE_THS    0x28
#define TAP_DUR       0x2A
#define TAP_THS       0x2B
#define ORIENT_HYST   0x2C
#define Z_BLOCK       0x2D
#define SELF_TEST     0x32
#define ENGINEERING_MODE   0x7f 


#define DMT_SENSITIVE_LOW	 0xB0
#define DMT_SENSITIVE_MID	 0x60
#define DMT_SENSITIVE_HIGH	 0x20

int test_int=0;

#if I2C_DEV == TI2C_MODE
typedef struct gsensor_info_s 
{		
	struct miscdevice dev;	
	struct ti2c_set_value_s g_ti2c_handle;
}
gsensor_info_t;

static gsensor_info_t gsensor_data;


static int A_sensor_write(unsigned char reg,unsigned char data)
{
	int ret=0;	
	
	gsensor_data.g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	gsensor_data.g_ti2c_handle.slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	gsensor_data.g_ti2c_handle.slaveAddr = (unsigned short)A_SENSOR_ADDR;
	gsensor_data.g_ti2c_handle.subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	gsensor_data.g_ti2c_handle.pSubAddr = (unsigned short *)&reg;
	gsensor_data.g_ti2c_handle.pBuf = &data;	
	gsensor_data.g_ti2c_handle.dataCnt = 1;	

	ret=gp_ti2c_bus_xfer(&gsensor_data.g_ti2c_handle);	

	return ret;
}
static int A_sensor_read(unsigned char reg,unsigned char *value)
{
	int ret=0;	

	gsensor_data.g_ti2c_handle.transmitMode = TI2C_BURST_READ_STOP_MODE;	
	
	gsensor_data.g_ti2c_handle.slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	gsensor_data.g_ti2c_handle.slaveAddr = (unsigned short)A_SENSOR_ADDR;
	gsensor_data.g_ti2c_handle.subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;	
	gsensor_data.g_ti2c_handle.pSubAddr =(unsigned short *) &reg;
	gsensor_data.g_ti2c_handle.pBuf = value;
	gsensor_data.g_ti2c_handle.dataCnt = 1;
	gsensor_data.g_ti2c_handle.apbdmaEn =0;	
	ret=gp_ti2c_bus_xfer(&gsensor_data.g_ti2c_handle);

	return ret;
}
#else
typedef struct gsensor_info_s 
{		
	struct miscdevice dev;	
	int g_ti2c_handle;
}
gsensor_info_t;

static gsensor_info_t gsensor_data;

static int A_sensor_write(unsigned char reg,unsigned char data)
{
	int ret=0xff;	
	char buf[2];
	buf[0] = reg;
	buf[1] = data;
	CHECK(gp_i2c_bus_write(gsensor_data.g_ti2c_handle, &buf, 2)>=0);
	printk( "[%s:%d] write reg = 0x%x,data = %d\n ", __FUNCTION__, __LINE__,reg,data);
	return ret;	
	Return:
	printk( "[%s:%d] Error!reg = %d,data = %d\n ", __FUNCTION__, __LINE__,reg,data);	
	
}
static int A_sensor_read(unsigned char reg)
{
	int ret;
	char value;
	CHECK(gp_i2c_bus_write(gsensor_data.g_ti2c_handle, &reg, 1)>=0);
	CHECK(gp_i2c_bus_read(gsensor_data.g_ti2c_handle, &value, 1)>=0);
	//printk( "[%s:%d] read reg = 0x%x,data = %d\n ", __FUNCTION__, __LINE__,reg,value);
	return value;
	Return:
	printk( "[%s:%d] Error!reg = %d\n ", __FUNCTION__, __LINE__,reg);

}
#endif

static char A_Sensor_Init(unsigned char level)
{
	UINT8 temp= 0xff;

	temp = A_sensor_read(MOTION_FLAG);
	if((temp != 0xff) && (temp & 0x04)) 
	{
		test_int = 1;
	}
	else
	{
		test_int = 0;
	}
	A_sensor_write(SOFT_RESET,0x24);
	temp = A_sensor_read(CHIPID);
	printk("[%s][%d]:ID is [%d]\r\n",__FUNCTION__,__LINE__,temp);
	if(temp != 0x13)
	{
		return 0;
	}
	A_sensor_write(RESOLUTION_RANGE,0x02);
	//A_sensor_read(RESOLUTION_RANGE);
	A_sensor_write(MODE_BW,0x1e);
	//A_sensor_read(MODE_BW);
	A_sensor_write(ODR_AXIS,0x07);
	//A_sensor_read(ODR_AXIS);
	A_sensor_write(INT_CONFIG,0x05);
	//A_sensor_read(INT_CONFIG);
	A_sensor_write(INT_LATCH,0x8f);
	//A_sensor_read(INT_LATCH);
	A_sensor_write(ENGINEERING_MODE,0x83);
	//A_sensor_read(ENGINEERING_MODE);
	A_sensor_write(ENGINEERING_MODE,0x69);
	//A_sensor_read(ENGINEERING_MODE);
	A_sensor_write(ENGINEERING_MODE,0xbd);
	//A_sensor_read(ENGINEERING_MODE);
	A_sensor_write(INT_SET1,0x07);			//avtive x,y,z interrupt
	//A_sensor_read(INT_SET1);
	A_sensor_write(ACTIVE_DUR,0x03);
	//A_sensor_read(ACTIVE_DUR);
	A_sensor_write(INT_MAP1,0x00);	//doesn't map to INT
	//A_sensor_read(INT_MAP1);
	A_sensor_write(INT_LATCH, 0x87);		//latch forever
	//A_sensor_read(INT_LATCH);
	switch(level)
	{
		case 3://low
			A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_LOW);
		break;
		case 2://mid
       	 	A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_MID);
		break;
		case 1://high
        		A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_HIGH);
		break;
	}
	return 1;
}

void A_sensor_uninit(unsigned char level) //for power off.
{
	printk("[%s][%d]\r\n",__FUNCTION__,__LINE__);

	A_sensor_write(SOFT_RESET,0x24);
	A_sensor_write(RESOLUTION_RANGE,0x02);
	A_sensor_write(MODE_BW,0x1e);
	A_sensor_write(ODR_AXIS,0x07);

	A_sensor_write(INT_CONFIG,0x05);
	A_sensor_write(INT_LATCH,0x8f);

	A_sensor_write(ENGINEERING_MODE,0x83);
	A_sensor_write(ENGINEERING_MODE,0x69);
	A_sensor_write(ENGINEERING_MODE,0xbd);

	A_sensor_write(INT_SET1,0x07);
	A_sensor_write(ACTIVE_DUR,0x03);
	A_sensor_write(INT_MAP1, 0x04);			//map to INT pin
	A_sensor_write(INT_LATCH, 0x87);

	switch(level)
	{
		case 3://low
			A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_LOW);		
		break;
		case 2://mid
        		A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_MID);
		break;
		case 1://high
       	 	A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_HIGH);
		break;
	}
	A_sensor_write(MODE_BW,0x5e);
}

static long A_sensor_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	int temp=0xff;	
	int shift_bits=0;//Æ«ÒÆÎ»-->resolution +-2g ~+-16g
	int x_out=0;
	int y_out=0;
	int z_out=0;
	int g_value=0;
	
	switch(cmd)	
	{	
		case ASENSOR_IOCTL_GET_INT_ACTIVE:	
	
				temp = A_sensor_read(MOTION_FLAG);
				//printk("[%s][%d]:MOTION_FLAG is [%d]\r\n",__FUNCTION__,__LINE__,temp);
				if((temp != 0xff) && (temp & 0x04)) 
				{ 
					//active int flag
					
					A_sensor_write(INT_LATCH, 0x87);
					temp = 1;
					printk("Asensor int actived \n\r");
				}
				else
				{
					temp = 0;
				}
				if(copy_to_user((void __user *)arg,&temp,sizeof(int)))			
					return -EFAULT;			
			break;	
		case ASENSOR_IOCTL_SET_SENSITIVE:	
			printk("[%s][%d]:level is [%d]\r\n",__FUNCTION__,__LINE__,arg);
			switch(arg)
				{
					case 3://low
						A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_LOW);		
					break;
					case 2://mid
			        		A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_MID);
					break;
					case 1://high
			       	 	A_sensor_write(ACTIVE_THS,DMT_SENSITIVE_HIGH);
					break;
				}
			break;
		case ASENSOR_IOCTL_PARK_MODE:
				A_sensor_uninit(arg);
			break;
		case ASENSOR_IOCTL_PARK_MODE_INT:
				if(copy_to_user((void __user *)arg,&test_int,sizeof(int)))			
				return -EFAULT;		
			break;
		case ASENSOR_IOCTL_GET_G_VALUE:
#if 1			
			temp = A_sensor_read(RESOLUTION_RANGE);	
			shift_bits = temp&0x03;//Æ«ÒÆÎ»-->resolution +-2g ~+-16g
							  // Resolution bit[3:2] -- 00:14bit 
                               //                        01:12bit 
                               //                        10:10bit
                               //                        11:8bit 
                               
                               // FS bit[1:0]         -- 00:+/-2g 
                               //                        01:+/-4g 
                               //                        10:+/-8g
                               //                        11:+/-16g 
			x_out = A_sensor_read(ACC_X_MSB);
			x_out<<=8;
			x_out =x_out + A_sensor_read(ACC_X_LSB);
			if(x_out&0x8000) //-g
			{
				x_out<<=(shift_bits);
				x_out &= ~0xFF0000;
				x_out |= 0x8000;				
			}else//+g
			{
				x_out<<=(shift_bits);
			}
			x_out =((x_out<<8)+0x800000)&0xff0000;		
/////////////////////////////////////////////////////
			y_out = A_sensor_read(ACC_Y_MSB);
			y_out<<=8;
			y_out =y_out + A_sensor_read(ACC_Y_LSB);		
			if(y_out&0x8000) //-g
			{
				y_out<<=(shift_bits);
				y_out &= ~0xFF0000;
				y_out |= 0x8000;				
			}else//+g
			{
				y_out<<=(shift_bits);
			}
			y_out =(y_out+0x8000)&0xff00;
////////////////////////////////////////////////////////
			z_out = A_sensor_read(ACC_Z_MSB);
			z_out<<=8;
			z_out =z_out + A_sensor_read(ACC_Z_LSB);				
		
			if(z_out&0x8000) //-g
			{
				z_out<<=(shift_bits);
				z_out &= ~0xFF0000;
				z_out |= 0x8000;				
			}else//+g
			{
				z_out<<=(shift_bits);
			}
			z_out =((z_out>>8)+0x80)&0x00ff;
///////////////////////////////
			g_value=x_out+y_out+z_out;
						
			if(copy_to_user((void __user *)arg,&g_value,sizeof(UINT32)))			
					return -EFAULT;	
//			printk("DA380 g_value=%x\n",g_value);
#endif
			break;
		default:	
			
			return -EINVAL;		
	}	
	return 0;
}

struct file_operations gsensor_fops = 
{	
	.owner		= THIS_MODULE,	
	.unlocked_ioctl = A_sensor_dev_ioctl,
};

static int __init gp_asensor_probe(struct platform_device *pdev)
{
	int ret = 0;
	#if I2C_DEV == TI2C_MODE
	gsensor_data.g_ti2c_handle.pDeviceString = "Asensor_DA380";	
	gsensor_data.g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;
	gsensor_data.g_ti2c_handle.slaveAddr = (unsigned short)A_SENSOR_ADDR;
	gsensor_data.g_ti2c_handle.clockRate = A_SENSOR_CLK;
	gsensor_data.g_ti2c_handle.apbdmaEn = 0;	/* open ti2c */	
	ret = gp_ti2c_bus_request(&gsensor_data.g_ti2c_handle);
	#else
	gsensor_data.g_ti2c_handle = gp_i2c_bus_request(A_SENSOR_ADDR,A_SENSOR_CLK);
	if(gsensor_data.g_ti2c_handle == -ENOMEM)
	{
		printk(KERN_ALERT"gsensor_data.g_ti2c_handle = 0x%x fail\n",gsensor_data.g_ti2c_handle);	
		return;
	}
	printk(KERN_ALERT"gsensor_data.g_ti2c_handle = 0x%x\n",gsensor_data.g_ti2c_handle);	
	#endif
	if(A_Sensor_Init(2) == 0)
	{
		goto err_reg;
	}
	
	gsensor_data.dev.name = "gp_asensor";	
	gsensor_data.dev.fops = &gsensor_fops;	
	gsensor_data.dev.minor = MISC_DYNAMIC_MINOR;
	
	ret = misc_register(&gsensor_data.dev);	
	if(ret)	
	{		
		printk(KERN_ALERT"gsensor probe register faile\n");		
		goto err_reg;	
	}
	platform_set_drvdata(pdev,&gsensor_data);
	
	printk(KERN_ALERT"gsensor kxtf9 probe ok\n");	
	return 0;
err_reg:	
	#if I2C_DEV == TI2C_MODE
	gp_ti2c_bus_release(&gsensor_data.g_ti2c_handle);
	#else
	gp_i2c_bus_release(gsensor_data.g_ti2c_handle);
	#endif
	return ret;	
}

static int gp_asensor_remove(struct platform_device *pdev)
{

	int ret;
	printk("[%s][%d]\r\n",__FUNCTION__,__LINE__);	
	//A_sensor_uninit(2);
	A_sensor_write(MODE_BW,0x5e);
	#if I2C_DEV == TI2C_MODE
	gp_ti2c_bus_release(&gsensor_data.g_ti2c_handle);
	#else
	gp_i2c_bus_release(gsensor_data.g_ti2c_handle);
	#endif
	ret = misc_deregister(&gsensor_data.dev);	
	if(ret)	
	{		
		printk(KERN_ALERT"gsensor rmmod register faile\n");		
		return ret;
	}
	return 0;
}

static int gp_asensor_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("[%s][%d]\r\n",__FUNCTION__,__LINE__);
	return 0;
}
static int gp_asensor_resume(struct platform_device *pdev)
{
	printk("[%s][%d]\r\n",__FUNCTION__,__LINE__);
	return 0;
}

static void gp_asensor_device_release(struct device *dev)
{
	printk("[%s][%d]\r\n",__FUNCTION__,__LINE__);
}



static struct platform_device gp_asensor_device = {
	.name = "gp_asensor",
	.id   = -1,
	.dev	= {
		.release = gp_asensor_device_release,
	}
};

static struct platform_driver gp_asensor_driver = {
       .driver         = {
	       .name   = "gp_asensor",
	       .owner  = THIS_MODULE,
       },
       .probe          = gp_asensor_probe,
       .remove         = gp_asensor_remove,
       .suspend        = gp_asensor_suspend,
       .resume         = gp_asensor_resume,

};

static int __init gp_gsensor_module_init(void)
{
	int rc;
	printk("gp_asensor_module_init \n");
	platform_device_register(&gp_asensor_device);
	rc = platform_driver_register(&gp_asensor_driver);
	printk("gp_asensor_module_init  end\n");
	return rc;
}

static void __exit gp_gsensor_module_exit(void)
{
	platform_device_unregister(&gp_asensor_device);
	platform_driver_unregister(&gp_asensor_driver);
}

module_init(gp_gsensor_module_init);
module_exit(gp_gsensor_module_exit);
MODULE_LICENSE_GP;





