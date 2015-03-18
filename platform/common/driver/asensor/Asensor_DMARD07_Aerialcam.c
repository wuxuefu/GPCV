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
//#include <linux/input.h>


#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#include <mach/gp_i2c_bus.h>
#endif


/*************************************************
MAC define
*************************************************/
#define TI2C_MODE 0
#define I2C_MODE 1
#define I2C_DEV  I2C_MODE
#define ERROR(fmt, arg...) printk( "[%s:%d] Error! "fmt, __FUNCTION__, __LINE__, ##arg)
#define RETURN(x)		{ret = x; goto Return;}
#define CHECK_(x, msg, errid) if(!(x)) {ERROR("%s, %s\n", msg, #x); RETURN(errid);}
#define CHECK(x)		CHECK_(x, "Check failed", -1)
#define CHECK_PRG(x)	CHECK_(x, "Program Error", -EIO)
#define CHECK_VAL(x)	CHECK_(x, "Value Error", -1)

#define ASENSOR_IOCTL_ID	'G'
#define ASENSOR_IOCTL_GET_G_ACTIVE	_IOR(ASENSOR_IOCTL_ID, 0x01, int)

#define ABSMIN_2G 0
#define ABSMAX_2G 1<<8
#define A_SENSOR_ADDR 0x38
#define A_SENSOR_CLK 100

/*DMT_ARD05 register*/
#define DMT_ARD07_CTRL_REG_1	0x44
#define DMT_ARD07_CTRL_REG_2	0x45
#define DMT_ARD07_CTRL_REG_3	0x46
#define DMT_ARD07_CTRL_REG_4	0x47
#define DMT_ARD07_CTRL_REG_5	0x4A
#define DMT_ARD07_CTRL_REG_6	0x4B
#define DMT_ARD07_CTRL_REG_7	0x4C
#define DMT_ARD07_CTRL_REG_8	0x4D
#define DMT_ARD07_CTRL_REG_9	0x48
#define DMT_ARD07_REG_XOUT	0x41
#define DMT_ARD07_REG_YOUT	0x42
#define DMT_ARD07_REG_ZOUT	0x43
#define DMT_ARD07_REG_STS	0x49


#define DMT_ARD07_LOW		0x60
#define DMT_ARD07_MID		0x38
#define DMT_ARD07_HIGH		0x07

//#define DMT_ARD07_LOW		0x0f
//#define DMT_ARD07_MID		0x08
//#define DMT_ARD07_HIGH		0x05

#define A_sensor_POLL HZ

#define filter_value 10

typedef struct gp_sensor_info_s
{
	struct miscdevice dev;
	#if I2C_DEV == TI2C_MODE
	struct ti2c_set_value_s g_ti2c_handle;
	#else
	int g_ti2c_handle;
	#endif
	unsigned char g_sensor_power_on;
}gp_sensor_info_t;

static struct gp_sensor_info_s *ptr_asensor=NULL;


#if I2C_DEV == TI2C_MODE
static int A_sensor_write(unsigned char reg,unsigned char data,ti2c_set_value_t*g_ti2c_handle)
{
	int ret=0;	
	g_ti2c_handle->transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle->slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	g_ti2c_handle->slaveAddr = (unsigned short)A_SENSOR_ADDR;
	g_ti2c_handle->subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	g_ti2c_handle->pSubAddr = (unsigned short *)&reg;
	g_ti2c_handle->pBuf = &data;	
	g_ti2c_handle->dataCnt = 1;	

	ret=gp_ti2c_bus_xfer(g_ti2c_handle);	
	//printk( "[%s:%d] wirte reg = 0x%x,data = 0x%x\n ", __FUNCTION__, __LINE__,reg,data);		
	return ret;
}
static int A_sensor_read(unsigned char reg,unsigned char *value,ti2c_set_value_t*g_ti2c_handle)
{
	int ret=0;	

	g_ti2c_handle->transmitMode = TI2C_BURST_READ_STOP_MODE;	
	
	g_ti2c_handle->slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	g_ti2c_handle->slaveAddr = (unsigned short)A_SENSOR_ADDR;
	g_ti2c_handle->subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;	
	g_ti2c_handle->pSubAddr =(unsigned short *) &reg;
	g_ti2c_handle->pBuf = value;
	g_ti2c_handle->dataCnt = 1;
	g_ti2c_handle->apbdmaEn =0;	
	ret=gp_ti2c_bus_xfer(g_ti2c_handle);
	ret = *value;
	//printk( "[%s:%d] read reg = 0x%x,data = 0x%x\n ", __FUNCTION__, __LINE__,reg,ret);	
	return ret;
}
#else
static int A_sensor_write(unsigned char reg,unsigned char data,int* g_ti2c_handle)
{
	int ret=0xff;		
	unsigned char buf[2];
	buf[0] = reg;
	buf[1] = data;
	CHECK(gp_i2c_bus_write(*g_ti2c_handle, (void*)&buf, 2)>=0);	
	printk( "[%s:%d] wirte reg = 0x%x,data = %d\n ", __FUNCTION__, __LINE__,reg,data);				
	return ret;
Return:	
	printk( "[%s:%d] Error!reg = 0x%x,data = %d\n ", __FUNCTION__, __LINE__,reg,data);	
	

}
static int A_sensor_read(unsigned char reg,unsigned char *value,int* g_ti2c_handle)
{
	
	int ret=0xff;	
	char value1 = 0xff;
	CHECK(gp_i2c_bus_write(*g_ti2c_handle, (void*)&reg, 1)>=0);	
	CHECK(gp_i2c_bus_read(*g_ti2c_handle, &value1, 1)>=0);	
	*value = value1;
	//printk( "[%s:%d] read reg,=0x%x,value = %d\n ", __FUNCTION__, __LINE__,reg,*value);
	return ret;
Return:	
	printk( "[%s:%d] Error!reg = 0x%x\n ", __FUNCTION__, __LINE__,reg);
}
#endif

#if I2C_DEV == TI2C_MODE
static int A_Sensor_Init(unsigned char *status,ti2c_set_value_t *g_ti2c_handle)
{
	unsigned char value;
	unsigned char x_out,y_out,z_out;
	
	A_sensor_read(0x0f,&value,g_ti2c_handle);
	printk("version 0x%x \n\r",value);
	if(value != 0x07) {
		return -1;
	}	
	A_sensor_read(0x53,&value,g_ti2c_handle);
	A_sensor_read(0x0f,&value,g_ti2c_handle);
	printk("version 0x%x \n\r",value);
	
	A_sensor_write(DMT_ARD07_CTRL_REG_1, 0x47,g_ti2c_handle);//0xa7 low power 4Hz // 0x27 normal 342hz
	//A_sensor_read(DMT_ARD07_CTRL_REG_1,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_2,0x24,g_ti2c_handle);//0x10 + - 2g data low filter
	//A_sensor_read(DMT_ARD07_CTRL_REG_2,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_3,0x00,g_ti2c_handle);
	//A_sensor_read(DMT_ARD07_CTRL_REG_3,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_4,0x2c,g_ti2c_handle); //0x4c 0x0c data ready int,activity high
	//A_sensor_read(DMT_ARD07_CTRL_REG_4,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_9,0x00,g_ti2c_handle);
	//A_sensor_read(DMT_ARD07_CTRL_REG_9,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_5,0x2a,g_ti2c_handle); //0x00
	//A_sensor_read(DMT_ARD07_CTRL_REG_5,&value,g_ti2c_handle);
//	A_sensor_write(DMT_ARD07_CTRL_REG_7,0x10,g_ti2c_handle); //0x00
	A_sensor_write(DMT_ARD07_CTRL_REG_7,DMT_ARD07_LOW,g_ti2c_handle);
	//A_sensor_read(DMT_ARD07_CTRL_REG_7,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_8, 0x08,g_ti2c_handle); //0x00
	//A_sensor_read(DMT_ARD07_CTRL_REG_8,&value,g_ti2c_handle);

	A_sensor_read(0x41,&x_out,g_ti2c_handle);
	A_sensor_read(0x42,&y_out,g_ti2c_handle);
	A_sensor_read(0x43,&z_out,g_ti2c_handle);
	printk("[%d,%d,%d]\r\n",x_out,y_out,z_out);

	return 0;
}

void A_sensor_uninit(UINT32 level,struct ti2c_set_value_s *g_ti2c_handle) //for power off.
{
	unsigned char data;
}
#else
static int A_Sensor_Init(unsigned char *status,int *g_ti2c_handle)
{
	unsigned char value;
	unsigned char x_out,y_out,z_out;
	int i;
	A_sensor_read(0x0f,&value,g_ti2c_handle);
	printk("version 0x%x \n\r",value);
	if(value != 0x07) {
		for(i=0;i<5;i++)
		{
			A_sensor_read(0x0f,&value,g_ti2c_handle);
			printk("version 0x%x \n\r",value);	
			if(value == 0x07)	
			{
				break;
			}
		}
		if(i==5)
		{
			return -1;
		}
	}

	A_sensor_read(0x53,&value,g_ti2c_handle);
	A_sensor_read(0x0f,&value,g_ti2c_handle);
	printk("version 0x%x \n\r",value);
	
	A_sensor_write(DMT_ARD07_CTRL_REG_1, 0x27,g_ti2c_handle);//0xa7 low power 4Hz //0x27 normal 342hz
	//A_sensor_read(DMT_ARD07_CTRL_REG_1,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_2,0x24,g_ti2c_handle);//0x10 + - 2g data low filter
	//A_sensor_read(DMT_ARD07_CTRL_REG_2,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_3,0x00,g_ti2c_handle);
	//A_sensor_read(DMT_ARD07_CTRL_REG_3,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_4,0x2c,g_ti2c_handle); //0x4c 0x0c data ready int,activity high
	//A_sensor_read(DMT_ARD07_CTRL_REG_4,&value,g_ti2c_handle);

	A_sensor_write(DMT_ARD07_CTRL_REG_9,0x00,g_ti2c_handle);
	//A_sensor_read(DMT_ARD07_CTRL_REG_9,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_5,0x2a,g_ti2c_handle); //0x00
	//A_sensor_read(DMT_ARD07_CTRL_REG_5,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_7,0x10,g_ti2c_handle); //0x00
	//A_sensor_read(DMT_ARD07_CTRL_REG_7,&value,g_ti2c_handle);
	A_sensor_write(DMT_ARD07_CTRL_REG_8, 0x08,g_ti2c_handle); //0x00
	//A_sensor_read(DMT_ARD07_CTRL_REG_8,&value,g_ti2c_handle);

	A_sensor_read(0x41,&x_out,g_ti2c_handle);
	A_sensor_read(0x42,&y_out,g_ti2c_handle);
	A_sensor_read(0x43,&z_out,g_ti2c_handle);
	printk("[%d,%d,%d]\r\n",x_out,y_out,z_out);

	return 0;
}

void A_sensor_uninit(UINT32 level, int *g_ti2c_handle) //for power off.
{

}
#endif
static long A_sensor_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	unsigned char temp[3];	
	unsigned char  x_out, y_out,z_out;

	switch(cmd)	
	{	
		case ASENSOR_IOCTL_GET_G_ACTIVE:	
					A_sensor_read(0x41,&x_out,&ptr_asensor->g_ti2c_handle);
					A_sensor_read(0x42,&y_out,&ptr_asensor->g_ti2c_handle);
					A_sensor_read(0x43,&z_out,&ptr_asensor->g_ti2c_handle);	
					
					temp[0] = x_out;
					temp[1] = y_out;
					temp[2] = z_out;	
				/*if((x_out&0x80) != 0)
				{
					x_out = 0xff - x_out;
					printk("[x=-%d,",x_out); 
				}
				else
				{
					printk("[x=%d,",x_out); 
				}
				if((y_out&0x80) != 0)
				{
					y_out = 0xff - y_out;
					printk("y=-%d,",y_out); 
				}
				else
				{
					printk("y=%d,",y_out); 
				}	
				if((z_out&0x80) != 0)
				{
					z_out = 0xff - z_out;
					printk("z=-%d]\r\n",z_out); 
				}
				else
				{
					printk("z=%d]\r\n",z_out); 
				}					*/	
				if(copy_to_user((void __user *)arg,&temp,sizeof(int)))			
					return -EFAULT;			
			break;	
		
		default:	
		break;	
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

// 1. malloc mem for gsensor
	ptr_asensor =kmalloc(sizeof(gp_sensor_info_t),GFP_KERNEL);
	if(ptr_asensor==NULL)
	{
		printk("kmalloc the g_sensor info fail\r\n");
		ret = -ENOMEM;
		goto __err_alloc0;
	}
		
// 2. clear the a sensor value	
	memset(ptr_asensor,0,sizeof(gp_sensor_info_t));

// 3. init the i2c, and init the DMARD07
	#if I2C_DEV == TI2C_MODE
	ptr_asensor->g_ti2c_handle.pDeviceString = "Asensor_ARD07";	
	ptr_asensor->g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;
	ptr_asensor->g_ti2c_handle.slaveAddr = (unsigned short)A_SENSOR_ADDR;
	ptr_asensor->g_ti2c_handle.clockRate = A_SENSOR_CLK;
	ptr_asensor->g_ti2c_handle.apbdmaEn = 0;	/* open ti2c */	
	ret = gp_ti2c_bus_request(&ptr_asensor->g_ti2c_handle);
	#else
		ptr_asensor->g_ti2c_handle = gp_i2c_bus_request(A_SENSOR_ADDR,A_SENSOR_CLK);
	#endif

	if(A_Sensor_Init(&ptr_asensor->g_sensor_power_on,&ptr_asensor->g_ti2c_handle) == -1) {
		goto __err_alloc;
	}

// 4. register the misc dev.
	ptr_asensor->dev.name ="gp_asensor";
	ptr_asensor->dev.fops = &gsensor_fops;
	ptr_asensor->dev.minor = MISC_DYNAMIC_MINOR;

	ret = misc_register(&ptr_asensor->dev);	
	if (ret)
	{		
		ret = -EIO;		
		goto __err_uninit_asensor;	
	}
	
//6.set the device private
	platform_set_drvdata(pdev, ptr_asensor);

	return ret;
// init work
__err_uninit_asensor:
	A_sensor_uninit(1,&ptr_asensor->g_ti2c_handle);
__err_alloc:
	#if I2C_DEV == TI2C_MODE
	gp_ti2c_bus_release(&ptr_asensor->g_ti2c_handle);
	#else
	gp_i2c_bus_release(ptr_asensor->g_ti2c_handle);
	#endif
	kfree(ptr_asensor);
	ptr_asensor = NULL;
__err_alloc0:	
	return ret;
	
}

static int gp_asensor_remove(struct platform_device *pdev)
{
	int ret;
	printk("[%s][%d]\r\n",__FUNCTION__,__LINE__);
	
	A_sensor_uninit(1,&ptr_asensor->g_ti2c_handle);

	#if I2C_DEV == TI2C_MODE
	gp_ti2c_bus_release(&ptr_asensor->g_ti2c_handle);
	#else
	gp_i2c_bus_release(ptr_asensor->g_ti2c_handle);
	#endif
	ret = misc_deregister(&ptr_asensor->dev);	
	if(ret)	
	{		
		printk(KERN_ALERT"gsensor rmmod register faile\n");		
		return ret;
	}

	kfree(ptr_asensor);
	ptr_asensor = NULL;
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





