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
 
/**************************************************************************
 *                         H E A D E R   F I L E S						  *
 **************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <linux/delay.h>
#include <mach/gp_gpio.h>
#include <mach/gp_i2c_bus.h>
#include <mach/sensor_mgr.h>

#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define GC0308_ID		0x42

// I2C mode
#define	GPIO_I2C		0x00 
#define HW_I2C			0x01
#define HW_TI2C			0x02
#if (defined CONFIG_ARCH_GPL32900)
#define I2C_MODE		HW_I2C
#elif (defined CONFIG_ARCH_GPL32900B)
#define I2C_MODE		HW_TI2C
#else
#define I2C_MODE		GPIO_I2C
#endif

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct sensor_dev_s 
{
	struct v4l2_subdev sd;
	sensor_fmt_t *fmt;	/* Current format */
}sensor_dev_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static sensor_dev_t	g_sensor_dev;
#if (I2C_MODE == HW_I2C)
static int g_i2c_handle;
#elif (I2C_MODE == HW_TI2C)
static ti2c_set_value_t g_ti2c_handle;
#endif

static char *param[] = {"0", "PORT0", "0", "NONE", "0", "NONE"};
static int nstrs = 6;
module_param_array(param, charp, &nstrs, S_IRUGO);

static sensor_fmt_t g_fmt_table[] =
{
	/* preview mode */
	{
		.desc		= "preview=640*480",
		.pixelformat = V4L2_PIX_FMT_VYUY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 640,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
	/* capature mode */
	{
		.desc		= "capture=640*480",
		.pixelformat = V4L2_PIX_FMT_VYUY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 640,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
	/* record mode */
	{
		.desc		= "record=640*480",
		.pixelformat = V4L2_PIX_FMT_VYUY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 640,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
};

#define C_SENSOR_FMT_MAX	sizeof(g_fmt_table)/sizeof(sensor_fmt_t)

static sensor_config_t config_table =
{
	.sensor_timing_mode = MODE_CCIR_601,
	.sensor_data_mode = MODE_DATA_YUV,
	.sensor_interlace_mode = MODE_NONE_INTERLACE,
	.sensor_pclk_mode = MODE_POSITIVE_EDGE,
	.sensor_hsync_mode = MODE_ACTIVE_HIGH,
	.sensor_vsync_mode = MODE_ACTIVE_LOW,
	.sensor_fmt_num = C_SENSOR_FMT_MAX,
	.fmt = g_fmt_table,
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
static int
sensor_i2c_open(
	unsigned int slave_id,
	unsigned int scl_speed
)
{
#if (I2C_MODE == HW_I2C)
	g_i2c_handle = gp_i2c_bus_request(slave_id, scl_speed);
	if((g_i2c_handle == 0) ||(g_i2c_handle == -ENOMEM)) {
		printk(KERN_WARNING "i2cReqFail %d\n", g_i2c_handle);
		return -1;
	}

	return 0;

#elif (I2C_MODE == HW_TI2C)
	g_ti2c_handle.pDeviceString = "GC0308";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("GC0308 ti2c request failed\n");
		return -1;
	}

	return 0;
#endif	
}

static int
sensor_i2c_close(
	void
)
{
#if (I2C_MODE == HW_I2C)
	return gp_i2c_bus_release(g_i2c_handle);
#elif (I2C_MODE == HW_TI2C)
	return gp_ti2c_bus_release(&g_ti2c_handle);	
#endif
}

static int
sensor_read(	
	unsigned short reg,
	unsigned char *value
)
{
#if (I2C_MODE == HW_I2C)
	char addr[0], data[0];
	int nRet;
	
	addr[0] = reg & 0xFF;
	nRet = gp_i2c_bus_write(g_i2c_handle, addr, 1);
	if(nRet <= 0) {
		return nRet;
	}
	
	nRet = gp_i2c_bus_read(g_i2c_handle, data, 1);
	*value = data[0];
	return nRet;
	
#elif (I2C_MODE == HW_TI2C)
	unsigned char addr[0], data[0];
	int nRet;
	
	addr[0] = reg & 0xFF;
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = addr;	
	g_ti2c_handle.dataCnt = 1;	
	nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
	if(nRet <= 0) {
		return nRet;
	}
	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_READ_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 1;	
	nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
	*value = data[0];
	return nRet;
#endif
}

static int
sensor_write(	
	unsigned short reg,
	unsigned char value
)
{
#if (I2C_MODE == HW_I2C)
	unsigned char data[2];

	data[0] = reg & 0xFF;
	data[1] = value;	
	return gp_i2c_bus_write(g_i2c_handle, data, 2);

#elif (I2C_MODE == HW_TI2C)
	unsigned char data[2];

	data[0] = reg & 0xFF;
	data[1] = value;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 2;	
	return gp_ti2c_bus_xfer(&g_ti2c_handle);
#endif
}

void Gp_GC0308_GAMMA_Select(int GammaLvl)
{
	switch(GammaLvl)
	{
		case 1:                                             //smallest gamma curve
			sensor_write( 0x9F, 0x0B ); 
			sensor_write( 0xA0, 0x16 ); 
			sensor_write( 0xA1, 0x29 ); 
			sensor_write( 0xA2, 0x3C ); 
			sensor_write( 0xA3, 0x4F ); 
			sensor_write( 0xA4, 0x5F ); 
			sensor_write( 0xA5, 0x6F ); 
			sensor_write( 0xA6, 0x8A ); 
			sensor_write( 0xA7, 0x9F ); 
			sensor_write( 0xA8, 0xB4 ); 
			sensor_write( 0xA9, 0xC6 ); 
			sensor_write( 0xAA, 0xD3 ); 
			sensor_write( 0xAB, 0xDD );  
			sensor_write( 0xAC, 0xE5 );  
			sensor_write( 0xAD, 0xF1 ); 
			sensor_write( 0xAE, 0xFA ); 
			sensor_write( 0xAF, 0xFF ); 	
			break;
		case 2:			
			sensor_write( 0x9F, 0x0E ); 
			sensor_write( 0xA0, 0x1C ); 
			sensor_write( 0xA1, 0x34 ); 
			sensor_write( 0xA2, 0x48 ); 
			sensor_write( 0xA3, 0x5A ); 
			sensor_write( 0xA4, 0x6B ); 
			sensor_write( 0xA5, 0x7B ); 
			sensor_write( 0xA6, 0x95 ); 
			sensor_write( 0xA7, 0xAB ); 
			sensor_write( 0xA8, 0xBF );
			sensor_write( 0xA9, 0xCE ); 
			sensor_write( 0xAA, 0xD9 ); 
			sensor_write( 0xAB, 0xE4 );  
			sensor_write( 0xAC, 0xEC ); 
			sensor_write( 0xAD, 0xF7 ); 
			sensor_write( 0xAE, 0xFD ); 
			sensor_write( 0xAF, 0xFF ); 
		break;
		case 3:
			sensor_write( 0x9F, 0x10 ); 
			sensor_write( 0xA0, 0x20 ); 
			sensor_write( 0xA1, 0x38 ); 
			sensor_write( 0xA2, 0x4E ); 
			sensor_write( 0xA3, 0x63 ); 
			sensor_write( 0xA4, 0x76 ); 
			sensor_write( 0xA5, 0x87 ); 
			sensor_write( 0xA6, 0xA2 ); 
			sensor_write( 0xA7, 0xB8 ); 
			sensor_write( 0xA8, 0xCA ); 
			sensor_write( 0xA9, 0xD8 ); 
			sensor_write( 0xAA, 0xE3 ); 
			sensor_write( 0xAB, 0xEB ); 
			sensor_write( 0xAC, 0xF0 ); 
			sensor_write( 0xAD, 0xF8 ); 
			sensor_write( 0xAE, 0xFD ); 
			sensor_write( 0xAF, 0xFF ); 

			break;
		case 4:
			sensor_write( 0x9F, 0x14 ); 
			sensor_write( 0xA0, 0x28 ); 
			sensor_write( 0xA1, 0x44 ); 
			sensor_write( 0xA2, 0x5D ); 
			sensor_write( 0xA3, 0x72 ); 
			sensor_write( 0xA4, 0x86 ); 
			sensor_write( 0xA5, 0x95 ); 
			sensor_write( 0xA6, 0xB1 ); 
			sensor_write( 0xA7, 0xC6 ); 
			sensor_write( 0xA8, 0xD5 ); 
			sensor_write( 0xA9, 0xE1 ); 
			sensor_write( 0xAA, 0xEA ); 
			sensor_write( 0xAB, 0xF1 ); 
			sensor_write( 0xAC, 0xF5 ); 
			sensor_write( 0xAD, 0xFB ); 
			sensor_write( 0xAE, 0xFE ); 
			sensor_write( 0xAF, 0xFF );
		break;
		case 5:								// largest gamma curve
			sensor_write( 0x9F, 0x15 ); 
			sensor_write( 0xA0, 0x2A ); 
			sensor_write( 0xA1, 0x4A ); 
			sensor_write( 0xA2, 0x67 ); 
			sensor_write( 0xA3, 0x79 ); 
			sensor_write( 0xA4, 0x8C ); 
			sensor_write( 0xA5, 0x9A ); 
			sensor_write( 0xA6, 0xB3 ); 
			sensor_write( 0xA7, 0xC5 ); 
			sensor_write( 0xA8, 0xD5 ); 
			sensor_write( 0xA9, 0xDF ); 
			sensor_write( 0xAA, 0xE8 ); 
			sensor_write( 0xAB, 0xEE ); 
			sensor_write( 0xAC, 0xF3 ); 
			sensor_write( 0xAD, 0xFA ); 
			sensor_write( 0xAE, 0xFD ); 
			sensor_write( 0xAF, 0xFF );
			break;
		default:
		break;
	}
}

static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);

	sensor_write(0xfe , 0x80);   	
		
	sensor_write(0xfe , 0x00);       // set page0  sensor_write(0xfe, 0x00)

		
	sensor_write(0xd2 , 0x10);   // close AEC
	sensor_write(0x22 , 0x55);   // close AWB

	sensor_write(0x5a , 0x56); 
	sensor_write(0x5b , 0x40);
	sensor_write(0x5c , 0x4a);			

	sensor_write(0x22 , 0x57);  // Open AWB
				
	sensor_write(0x01 , 0xfa); 
	sensor_write(0x02 , 0x70); 
	sensor_write(0x0f , 0x01); 

	sensor_write(0x03 , 0x01); 
	sensor_write(0x04 , 0x2c); 

	sensor_write(0xe2 , 0x00); 	//anti-flicker step [11:8]
	sensor_write(0xe3 , 0x64);   //anti-flicker step [7:0]
		
	sensor_write(0xe4 , 0x02);   //exp level 0  16.67fps
	sensor_write(0xe5 , 0x58); 
	sensor_write(0xe6 , 0x03);   //exp level 1  12.5fps
	sensor_write(0xe7 , 0x20); 
	sensor_write(0xe8 , 0x04);   //exp level 2  8.33fps
	sensor_write(0xe9 , 0xb0); 
	sensor_write(0xea , 0x09);   //exp level 3  4.00fps
	sensor_write(0xeb , 0xc4); 

	sensor_write(0x05 , 0x00);
	sensor_write(0x06 , 0x00);
	sensor_write(0x07 , 0x00);
	sensor_write(0x08 , 0x00);
	sensor_write(0x09 , 0x01);
	sensor_write(0x0a , 0xe8);
	sensor_write(0x0b , 0x02);
	sensor_write(0x0c , 0x88);
	sensor_write(0x0d , 0x02);
	sensor_write(0x0e , 0x02);
	sensor_write(0x10 , 0x22);
	sensor_write(0x11 , 0xfd);
	sensor_write(0x12 , 0x2a);
	sensor_write(0x13 , 0x00);
	sensor_write(0x14 , 0x10);
	sensor_write(0x15 , 0x0a);
	sensor_write(0x16 , 0x05);
	sensor_write(0x17 , 0x01);
	sensor_write(0x18 , 0x44);
	sensor_write(0x19 , 0x44);
	sensor_write(0x1a , 0x1e);
	sensor_write(0x1b , 0x00);
	sensor_write(0x1c , 0xc1);
	sensor_write(0x1d , 0x08);
	sensor_write(0x1e , 0x60);
	sensor_write(0x1f , 0x16);

	
	sensor_write(0x20 , 0xff);
	sensor_write(0x21 , 0xf8);
	sensor_write(0x22 , 0x57);
	sensor_write(0x24 , 0xa0);
	sensor_write(0x25 , 0x0f);
	                         
	//output sync_mode       
	sensor_write(0x26 , 0x03);
	sensor_write(0x2f , 0x01);
	sensor_write(0x30 , 0xf7);
	sensor_write(0x31 , 0x50);
	sensor_write(0x32 , 0x00);
	sensor_write(0x39 , 0x04);
	sensor_write(0x3a , 0x18);
	sensor_write(0x3b , 0x20);
	sensor_write(0x3c , 0x00);
	sensor_write(0x3d , 0x00);
	sensor_write(0x3e , 0x00);
	sensor_write(0x3f , 0x00);
	sensor_write(0x50 , 0x10);
	sensor_write(0x53 , 0x82);
	sensor_write(0x54 , 0x80);
	sensor_write(0x55 , 0x80);
	sensor_write(0x56 , 0x82);
	sensor_write(0x8b , 0x40);
	sensor_write(0x8c , 0x40);
	sensor_write(0x8d , 0x40);
	sensor_write(0x8e , 0x2e);
	sensor_write(0x8f , 0x2e);
	sensor_write(0x90 , 0x2e);
	sensor_write(0x91 , 0x3c);
	sensor_write(0x92 , 0x50);
	sensor_write(0x5d , 0x12);
	sensor_write(0x5e , 0x1a);
	sensor_write(0x5f , 0x24);
	sensor_write(0x60 , 0x07);
	sensor_write(0x61 , 0x15);
	sensor_write(0x62 , 0x08);
	sensor_write(0x64 , 0x03);
	sensor_write(0x66 , 0xe8);
	sensor_write(0x67 , 0x86);
	sensor_write(0x68 , 0xa2);
	sensor_write(0x69 , 0x18);
	sensor_write(0x6a , 0x0f);
	sensor_write(0x6b , 0x00);
	sensor_write(0x6c , 0x5f);
	sensor_write(0x6d , 0x8f);
	sensor_write(0x6e , 0x55);
	sensor_write(0x6f , 0x38);
	sensor_write(0x70 , 0x15);
	sensor_write(0x71 , 0x33);
	sensor_write(0x72 , 0xdc);
	sensor_write(0x73 , 0x80);
	sensor_write(0x74 , 0x02);
	sensor_write(0x75 , 0x3f);
	sensor_write(0x76 , 0x02);
	sensor_write(0x77 , 0x36);
	sensor_write(0x78 , 0x88);
	sensor_write(0x79 , 0x81);
	sensor_write(0x7a , 0x81);
	sensor_write(0x7b , 0x22);
	sensor_write(0x7c , 0xff);
	sensor_write(0x93 , 0x48);
	sensor_write(0x94 , 0x00);
	sensor_write(0x95 , 0x05);
	sensor_write(0x96 , 0xe8);
	sensor_write(0x97 , 0x40);
	sensor_write(0x98 , 0xf0);
	sensor_write(0xb1 , 0x38);
	sensor_write(0xb2 , 0x38);
	sensor_write(0xbd , 0x38);
	sensor_write(0xbe , 0x36);
	sensor_write(0xd0 , 0xc9);
	sensor_write(0xd1 , 0x10);
	//sensor_write(0xd2 , 0x90);
	sensor_write(0xd3 , 0x80);
	sensor_write(0xd5 , 0xf2);
	sensor_write(0xd6 , 0x16);
	sensor_write(0xdb , 0x92);
	sensor_write(0xdc , 0xa5);
	sensor_write(0xdf , 0x23);
	sensor_write(0xd9 , 0x00);
	sensor_write(0xda , 0x00);
	sensor_write(0xe0 , 0x09);
	sensor_write(0xec , 0x20);
	sensor_write(0xed , 0x04);
	sensor_write(0xee , 0xa0);
	sensor_write(0xef , 0x40);
	sensor_write(0x80 , 0x03);
	sensor_write(0x80 , 0x03);
	sensor_write(0x9F , 0x10);
	sensor_write(0xA0 , 0x20);
	sensor_write(0xA1 , 0x38);
	sensor_write(0xA2 , 0x4E);
	sensor_write(0xA3 , 0x63);
	sensor_write(0xA4 , 0x76);
	sensor_write(0xA5 , 0x87);
	sensor_write(0xA6 , 0xA2);
	sensor_write(0xA7 , 0xB8);
	sensor_write(0xA8 , 0xCA);
	sensor_write(0xA9 , 0xD8);
	sensor_write(0xAA , 0xE3);
	sensor_write(0xAB , 0xEB);
	sensor_write(0xAC , 0xF0);
	sensor_write(0xAD , 0xF8);
	sensor_write(0xAE , 0xFD);
	sensor_write(0xAF , 0xFF);
	sensor_write(0xc0 , 0x00);
	sensor_write(0xc1 , 0x10);
	sensor_write(0xc2 , 0x1C);
	sensor_write(0xc3 , 0x30);
	sensor_write(0xc4 , 0x43);
	sensor_write(0xc5 , 0x54);
	sensor_write(0xc6 , 0x65);
	sensor_write(0xc7 , 0x75);
	sensor_write(0xc8 , 0x93);
	sensor_write(0xc9 , 0xB0);
	sensor_write(0xca , 0xCB);
	sensor_write(0xcb , 0xE6);
	sensor_write(0xcc , 0xFF);
	sensor_write(0xf0 , 0x02);
	sensor_write(0xf1 , 0x01);
	sensor_write(0xf2 , 0x01);
	sensor_write(0xf3 , 0x30);
	sensor_write(0xf9 , 0x9f);
	sensor_write(0xfa , 0x78);

	//---------------------------------------------------------------
	sensor_write(0xfe , 0x01);  //sensor_write(0xfe, 0x01)

	sensor_write(0x00 , 0xf5);
	sensor_write(0x02 , 0x1a);
	sensor_write(0x0a , 0xa0);
	sensor_write(0x0b , 0x60);
	sensor_write(0x0c , 0x08);
	sensor_write(0x0e , 0x4c);
	sensor_write(0x0f , 0x39);
	sensor_write(0x11 , 0x3f);
	sensor_write(0x12 , 0x72);
	sensor_write(0x13 , 0x13);
	sensor_write(0x14 , 0x42);
	sensor_write(0x15 , 0x43);
	sensor_write(0x16 , 0xc2);
	sensor_write(0x17 , 0xa8);
	sensor_write(0x18 , 0x18);
	sensor_write(0x19 , 0x40);
	sensor_write(0x1a , 0xd0);
	sensor_write(0x1b , 0xf5);
	sensor_write(0x70 , 0x40);
	sensor_write(0x71 , 0x58);
	sensor_write(0x72 , 0x30);
	sensor_write(0x73 , 0x48);
	sensor_write(0x74 , 0x20);
	sensor_write(0x75 , 0x60);
	sensor_write(0x77 , 0x20);
	sensor_write(0x78 , 0x32);
	sensor_write(0x30 , 0x03);
	sensor_write(0x31 , 0x40);
	sensor_write(0x32 , 0xe0);
	sensor_write(0x33 , 0xe0);
	sensor_write(0x34 , 0xe0);
	sensor_write(0x35 , 0xb0);
	sensor_write(0x36 , 0xc0);
	sensor_write(0x37 , 0xc0);
	sensor_write(0x38 , 0x04);
	sensor_write(0x39 , 0x09);
	sensor_write(0x3a , 0x12);
	sensor_write(0x3b , 0x1C);
	sensor_write(0x3c , 0x28);
	sensor_write(0x3d , 0x31);
	sensor_write(0x3e , 0x44);
	sensor_write(0x3f , 0x57);
	sensor_write(0x40 , 0x6C);
	sensor_write(0x41 , 0x81);
	sensor_write(0x42 , 0x94);
	sensor_write(0x43 , 0xA7);
	sensor_write(0x44 , 0xB8);
	sensor_write(0x45 , 0xD6);
	sensor_write(0x46 , 0xEE);
	sensor_write(0x47 , 0x0d); 
	
	sensor_write(0xfe , 0x00);  //sensor_write(0xfe, 0x00)

    	sensor_write(0xd2 , 0x90);  // Open AEC at last.  

	//Registers of Page0
	sensor_write(0xfe , 0x00); 

	sensor_write(0x10 , 0x26);                                 
	sensor_write(0x11 , 0x0d);  	// fd                                
	sensor_write(0x1a , 0x2a);  	// 1e                                   

	sensor_write(0x1c , 0x49); 	// c1                                  
	sensor_write(0x1d , 0x9a);	// 08                                  
	sensor_write(0x1e , 0x61);	// 60                                  

	sensor_write(0x3a , 0x20);

	sensor_write(0x50 , 0x14);  	// 10                                
	sensor_write(0x53 , 0x80);                                  
	sensor_write(0x56 , 0x80);
	
	sensor_write(0x8b , 0x20); 	//LSC                                 
	sensor_write(0x8c , 0x20);                                  
	sensor_write(0x8d , 0x20);                                  
	sensor_write(0x8e , 0x14);                                  
	sensor_write(0x8f , 0x10);                                  
	sensor_write(0x90 , 0x14);                                  

	sensor_write(0x94 , 0x02);                                  
	sensor_write(0x95 , 0x07);                                  
	sensor_write(0x96 , 0xe0);                                  

	sensor_write(0xb1 , 0x40); 	// YCPT                                 
	sensor_write(0xb2 , 0x40);                                  
	sensor_write(0xb3 , 0x40);
	sensor_write(0xb6 , 0xe0);

	sensor_write(0xd0 , 0xcb); 	// AECT  c9                                 
	sensor_write(0xd3 , 0x48); 	// 80                          

	sensor_write(0xf2 , 0x02);                                  
	sensor_write(0xf7 , 0x12);
	sensor_write(0xf8 , 0x0a);

	//Registers of Page1
	sensor_write(0xfe , 0x01);

	sensor_write(0x02 , 0x20);
	sensor_write(0x04 , 0x10);
	sensor_write(0x05 , 0x08);
	sensor_write(0x06 , 0x20);
	sensor_write(0x08 , 0x0a);

	sensor_write(0x0e , 0x44);                                  
	sensor_write(0x0f , 0x32);
	sensor_write(0x10 , 0x41);                                  
	sensor_write(0x11 , 0x37);                                  
	sensor_write(0x12 , 0x22);                                  
	sensor_write(0x13 , 0x19);                                  
	sensor_write(0x14 , 0x44);                                  
	sensor_write(0x15 , 0x44);  
	
	sensor_write(0x19 , 0x50);                                  
	sensor_write(0x1a , 0xd8); 
	
	sensor_write(0x32 , 0x10); 
	
	sensor_write(0x35 , 0x00);                                  
	sensor_write(0x36 , 0x80);                                  
	sensor_write(0x37 , 0x00); 
	//-----------Update the registers end---------//

    	sensor_write(0xfe , 0x00); 
    	/*Customer can adjust GAMMA, MIRROR & UPSIDEDOWN here!*/	
    	Gp_GC0308_GAMMA_Select(2);

	msleep(10);
	return sensor_write(0x14, 0x10);	// normal:10
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
sensor_record(void)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
sensor_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	return 0;
}

static int 
sensor_queryctrl(
	struct v4l2_subdev *sd,
	struct v4l2_queryctrl *qc
)
{
	/* Fill in min, max, step and default value for these controls. */
	switch(qc->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		qc->minimum = 0;
		qc->maximum = 1;
		qc->step = 1;
		qc->default_value = 1;
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		qc->minimum = 50;
		qc->maximum = 60;
		qc->step = 10;
		qc->default_value = 50;
		break;

	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		qc->minimum = 0;
		qc->maximum = 3;
		qc->step = 1;
		qc->default_value = 0;
		break;

	case V4L2_CID_BACKLIGHT_COMPENSATION:
		qc->minimum = 0;
		qc->maximum = 1;
		qc->step = 1;
		qc->default_value = 0;
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static int 
sensor_g_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{	
	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		break;

	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		break;

	case V4L2_CID_BACKLIGHT_COMPENSATION:	
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int 
sensor_s_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{

	unsigned char data;
	int nRet = 0;

	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		if(ctrl->value) {	// Enable Auto AWB
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x5a, 0x56); //for AWB can adjust back
			nRet = sensor_write(0x5b, 0x40);
			nRet = sensor_write(0x5c, 0x4a);			
			nRet = sensor_write(0x22, data|0x02);	 // Enable AWB
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		}
		else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(0x01  ,0x32); 	
			nRet = sensor_write(0x02  ,0x70); 
			nRet = sensor_write(0x0f  ,0x01);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x78);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x02);   //exp level 0  12.5fps
			nRet = sensor_write(0xe5  ,0x58); 
		
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_write(0x01  ,0x6a); 	
			nRet = sensor_write(0x02  ,0x89); 
			nRet = sensor_write(0x0f  ,0x00);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x7d);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x02);   //exp level 0  12.00fps
			nRet = sensor_write(0xe5  ,0x71); 
		
		}
		  else
		{
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02);   
			nRet = sensor_write(0x5a, 0x74);	// 50 45 40
			nRet = sensor_write(0x5b, 0x52);
			nRet = sensor_write(0x5c, 0x40);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02);	// Disable AWB 
			nRet = sensor_write(0x5a, 0x8c);	// WB_manual_gain // 5a 42 40
			nRet = sensor_write(0x5b, 0x50);
			nRet = sensor_write(0x5c, 0x40);
		}else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02);   
			nRet = sensor_write(0x5a, 0x40);
			nRet = sensor_write(0x5b, 0x42);
			nRet = sensor_write(0x5c, 0x50);
		}else if(ctrl->value == 3) {	// INCANDESCENCE
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02); 
			nRet = sensor_write(0x5a, 0x48);
			nRet = sensor_write(0x5b, 0x40);
			nRet = sensor_write(0x5c, 0x5c);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		nRet = sensor_read(0x20, (unsigned char *)&data);	
		if(ctrl->value) {	// NIGH MODE ON
			nRet = sensor_write(0x01  ,0x32); 	
			nRet = sensor_write(0x02  ,0xc8); 
			nRet = sensor_write(0x0f  ,0x21);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x78);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x04);   //exp level 0  10fps
			nRet = sensor_write(0xe5  ,0xb0); 	

			nRet = sensor_write(0xec, 0x00);
			nRet = sensor_write(0x20, data&0x5f);   // close cc
			nRet = sensor_write(0x3c, 0x08);
			nRet = sensor_write(0x3d, 0x08);
			nRet = sensor_write(0x3e, 0x08);
			nRet = sensor_write(0x3f, 0x08);
		}
		else{	// NIGH MODE OFF
			nRet = sensor_write(0x01  ,0x32); 	
			nRet = sensor_write(0x02  ,0x70); 
			nRet = sensor_write(0x0f  ,0x01);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x78);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x02);   //exp level 0  20fps
			nRet = sensor_write(0xe5  ,0x58); 

			nRet = sensor_write(0xec, 0x00);
			nRet = sensor_write(0x20, data|0x20);
			nRet = sensor_write(0x3c, 0x02);
			nRet = sensor_write(0x3d, 0x02);
			nRet = sensor_write(0x3e, 0x02);
			nRet = sensor_write(0x3f, 0x02);
		}
		break;

	default:
		return -EINVAL;
	}

	return nRet; 
}

static int 
sensor_querystd(
	struct v4l2_subdev *sd,
	v4l2_std_id *std
)
{
	return 0;
}

static int 
sensor_enum_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_fmtdesc *fmtdesc
)
{
	printk("%s\n", __FUNCTION__);
	if(fmtdesc->index >= C_SENSOR_FMT_MAX)
		return -EINVAL;

	fmtdesc->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	memcpy((void *)fmtdesc->description, (void *)g_fmt_table[fmtdesc->index].desc, 32);
	fmtdesc->pixelformat = g_fmt_table[fmtdesc->index].pixelformat;
	return 0;
}

static int 
sensor_g_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	printk("%s\n", __FUNCTION__);
	fmt->fmt.pix.width = g_sensor_dev.fmt->hpixel;
	fmt->fmt.pix.height = g_sensor_dev.fmt->vline;
	fmt->fmt.pix.pixelformat = g_sensor_dev.fmt->pixelformat;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = g_sensor_dev.fmt->hpixel * g_sensor_dev.fmt->bpp;
	fmt->fmt.pix.sizeimage = fmt->fmt.pix.bytesperline * g_sensor_dev.fmt->vline;

	return 0;
}

static int 
sensor_try_fmt(
	struct v4l2_subdev *sd,
	struct v4l2_format *fmt
)
{
	return 0;
}

static int 
sensor_s_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	int ret;

	printk("%s\n", __FUNCTION__);
	if(0 == fmt->fmt.pix.priv) {
		ret = sensor_preview();
	}else if (1 == fmt->fmt.pix.priv) {
		ret = sensor_capture();
	}else if (2 == fmt->fmt.pix.priv) {
		ret = sensor_record();
	}else {
		ret = -1;
	}

	g_sensor_dev.fmt = &g_fmt_table[fmt->fmt.pix.priv];
	return ret;
}

static int 
sensor_cropcap(
	struct v4l2_subdev *sd,
	struct v4l2_cropcap *cc
)
{
	return 0;
}

static int 
sensor_g_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
sensor_s_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
sensor_g_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *parms
)
{
	return 0;
}

static int 
sensor_s_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *param
)
{
	return 0;
}

static int 
sensor_s_interface(
	struct v4l2_subdev *sd,
	struct v4l2_interface *interface
)
{
	return 0;
}

static int 
sensor_suspend(
	struct v4l2_subdev *sd
)
{
	// need implement
	//sensor_write(0x3306, 0x02);
	return 0;
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
	// need implement
	//sensor_write(0x3306, 0x02);
	return 0;
}

static const struct v4l2_subdev_core_ops sensor_core_ops = 
{
	.init = sensor_init,
	.reset = sensor_reset,
	.queryctrl = sensor_queryctrl,
	.g_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
};

static const struct v4l2_subdev_video_ops sensor_video_ops = 
{
	.querystd = sensor_querystd,
	.enum_fmt = sensor_enum_fmt,
	.g_fmt = sensor_g_fmt,
	.try_fmt = sensor_try_fmt,
	.s_fmt = sensor_s_fmt,
	.cropcap = sensor_cropcap,
	.g_crop = sensor_g_crop,
	.s_crop = sensor_s_crop,
	.g_parm = sensor_g_parm,
	.s_parm = sensor_s_parm,
};

static const struct v4l2_subdev_ext_ops sensor_ext_ops = 
{
	.s_interface = sensor_s_interface,
	.suspend = sensor_suspend,
	.resume = sensor_resume,
};

static const struct v4l2_subdev_ops sensor_ops = 
{
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
	.ext = &sensor_ext_ops
};

static int __init 
sensor_module_init(
		void
)
{
	if(sensor_i2c_open(GC0308_ID, 50) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: gc0308 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_gc0308");
	register_sensor(&g_sensor_dev.sd, (int *)&param[0], &config_table);
	return 0;
}

static void __exit
sensor_module_exit(
		void
)
{
	sensor_i2c_close();
	unregister_sensor(&(g_sensor_dev.sd));
}

module_init(sensor_module_init);
module_exit(sensor_module_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus gc0308 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

