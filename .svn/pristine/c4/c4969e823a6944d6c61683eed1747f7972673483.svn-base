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
#define	SP0838_ID		0x30

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
		.pixelformat = V4L2_PIX_FMT_UYVY,
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
		.pixelformat = V4L2_PIX_FMT_UYVY,
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
		.pixelformat = V4L2_PIX_FMT_UYVY,
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
	.sensor_timing_mode = MODE_CCIR_HREF,
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
	g_ti2c_handle.pDeviceString = "SP0838";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("SP0838 ti2c request failed\n");
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

static int dv_flag = 0;

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
 
static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
  sensor_write(0xfd,0x00);  //P0
  sensor_write(0x1B,0x02);
  sensor_write(0x27,0xe8);
  sensor_write(0x28,0x0B);
  sensor_write(0x32,0x00);
  sensor_write(0x22,0xc0);
  sensor_write(0x26,0x10);
  sensor_write(0x31,0x70);  //Upside/mirr/Pclk inv/sub
  sensor_write(0x5f,0x11);  //Bayer order
  sensor_write(0xfd,0x01);  //P1
  sensor_write(0x25,0x1a);  //Awb start
  sensor_write(0x26,0xfb);
  sensor_write(0x28,0x61);
  sensor_write(0x29,0x49);
  sensor_write(0x31,0x60); //64
  sensor_write(0x32,0x18);
  sensor_write(0x4d,0xdc);
  sensor_write(0x4e,0x6b);
  sensor_write(0x41,0x8c);
  sensor_write(0x42,0x66);
  sensor_write(0x55,0xff);
  sensor_write(0x56,0x00);
  sensor_write(0x59,0x82);
  sensor_write(0x5a,0x00);
  sensor_write(0x5d,0xff);
  sensor_write(0x5e,0x6f);
  sensor_write(0x57,0xff);
  sensor_write(0x58,0x00);
  sensor_write(0x5b,0xff);
  sensor_write(0x5c,0xa8);
  sensor_write(0x5f,0x75);
  sensor_write(0x60,0x00);
  sensor_write(0x2d,0x00);
  sensor_write(0x2e,0x00);
  sensor_write(0x2f,0x00);
  sensor_write(0x30,0x00);
  sensor_write(0x33,0x00);
  sensor_write(0x34,0x00);
  sensor_write(0x37,0x00);
  sensor_write(0x38,0x00);  //awb end
  sensor_write(0xfd,0x00);  //P0
  sensor_write(0x33,0x6f);  //LSC BPC EN
  sensor_write(0x51,0x3f);  //BPC debug start
  sensor_write(0x52,0x09);  
  sensor_write(0x53,0x00);  
  sensor_write(0x54,0x00);
  sensor_write(0x55,0x10);  //BPC debug end
  sensor_write(0x4f,0xff);  //blueedge
  sensor_write(0x50,0xff);  
  sensor_write(0x57,0x40);  //Raw filter debut start
  sensor_write(0x58,0x40);  //4
  sensor_write(0x59,0x10);  //04
  sensor_write(0x56,0x70);
  sensor_write(0x5a,0x02);
  sensor_write(0x5b,0x02);
            
  sensor_write(0x5c,0x20);  //Raw filter debut end 
  sensor_write(0x65,0x06);  //Sharpness debug start
  sensor_write(0x66,0x01);
  sensor_write(0x67,0x03);
  sensor_write(0x68,0xc6);
  sensor_write(0x69,0x7f);

  sensor_write(0x6a,0x01);
  sensor_write(0x6b,0x04);
  sensor_write(0x6c,0x01);
  sensor_write(0x6d,0x06);  //Edge gain normal
  sensor_write(0x6e,0xc8);  //Edge gain normal
  sensor_write(0x6f,0x7f);

  sensor_write(0x70,0x01);
  sensor_write(0x71,0x04);
  sensor_write(0x72,0x01);
  sensor_write(0x73,0x06);  //Edge gain lowlight
  sensor_write(0x74,0xc8);  //Edge gain normal
  sensor_write(0x75,0x7f);

  sensor_write(0x76,0x01);  //Sharpness debug end
  sensor_write(0xcb,0x07);  //HEQ&Saturation debug start 
  sensor_write(0xcc,0x04);
  sensor_write(0xce,0xff);
  sensor_write(0xcf,0x10);
  sensor_write(0xd0,0x20);
  sensor_write(0xd1,0x00);
  sensor_write(0xd2,0x1c);
  sensor_write(0xd3,0x16);
  sensor_write(0xd4,0x00);
  sensor_write(0xd6,0x1c);
  sensor_write(0xd7,0x16);
  sensor_write(0xdd,0x70);  //Contrast
  sensor_write(0xde,0xa0);  //HEQ&Saturation debug end
//Color Correction start 
  sensor_write(0x7f,0xbe);
  sensor_write(0x80,0xe6);
  sensor_write(0x81,0xf2);
  sensor_write(0x82,0xde);
  sensor_write(0x83,0xa3);
  sensor_write(0x84,0xff);
  sensor_write(0x85,0xe7);
  sensor_write(0x86,0xc0);
  sensor_write(0x87,0xd9);
  sensor_write(0x88,0x3c);
  sensor_write(0x89,0x33);
  sensor_write(0x8a,0xf );
//Color Correction end
  sensor_write(0x8b,0x0 );  //gamma start
  sensor_write(0x8c,0x1a);
  sensor_write(0x8d,0x29);
  sensor_write(0x8e,0x41);
  sensor_write(0x8f,0x62);
  sensor_write(0x90,0x7c);
  sensor_write(0x91,0x90);
  sensor_write(0x92,0xa2);
  sensor_write(0x93,0xaf);
  sensor_write(0x94,0xbc);
  sensor_write(0x95,0xc5);
  sensor_write(0x96,0xcd);
  sensor_write(0x97,0xd5);
  sensor_write(0x98,0xdd);
  sensor_write(0x99,0xe5);
  sensor_write(0x9a,0xed);
  sensor_write(0x9b,0xf5);
  sensor_write(0xfd,0x01);  //P1
  sensor_write(0x8d,0xfd);
  sensor_write(0x8e,0xff);  //gamma end
  sensor_write(0xfd,0x00);  //P0
  sensor_write(0xca,0xcf);
  sensor_write(0xd8,0x65);  //UV outdoor
  sensor_write(0xd9,0x65);  //UV indoor 
  sensor_write(0xda,0x65);  //UV dummy
  sensor_write(0xdb,0x50);  //UV lowlight
  sensor_write(0xb9,0x00);  //Ygamma start
  sensor_write(0xba,0x04);
  sensor_write(0xbb,0x08);
  sensor_write(0xbc,0x10);
  sensor_write(0xbd,0x20);
  sensor_write(0xbe,0x30);
  sensor_write(0xbf,0x40);
  sensor_write(0xc0,0x50);
  sensor_write(0xc1,0x60);
  sensor_write(0xc2,0x70);
  sensor_write(0xc3,0x80);
  sensor_write(0xc4,0x90);
  sensor_write(0xc5,0xA0);
  sensor_write(0xc6,0xB0);
  sensor_write(0xc7,0xC0);
  sensor_write(0xc8,0xD0);
  sensor_write(0xc9,0xE0);
  sensor_write(0xfd,0x01);  //P1
  sensor_write(0x89,0xf0);
  sensor_write(0x8a,0xff);  //Ygamma end
  sensor_write(0xfd,0x00);  //P0
  sensor_write(0xe8,0x30);  //AEdebug start
  sensor_write(0xe9,0x30);
  sensor_write(0xea,0x40);  //0x40  //Alc Window sel
  sensor_write(0xf4,0x1b);  //outdoor mode sel
  sensor_write(0xf5,0x80);
  sensor_write(0xf7,0x78);  //AE target
  sensor_write(0xf8,0x63);  
  sensor_write(0xf9,0x68);  //AE target
  sensor_write(0xfa,0x53);
  sensor_write(0xfd,0x01);  //P1
  sensor_write(0x09,0x31);  //AE Step 3.0
  sensor_write(0x0a,0x85);
  sensor_write(0x0b,0x0b);  //AE Step 3.0
  sensor_write(0x14,0x20);
  sensor_write(0x15,0x0f);
  sensor_write(0xfd,0x00);  //p0
  sensor_write(0x05,0x0 );
  sensor_write(0x06,0x0 );
  sensor_write(0x09,0x1 );
  sensor_write(0x0a,0x76);
  sensor_write(0xf0,0x62);
  sensor_write(0xf1,0x0 );
  sensor_write(0xf2,0x5f);
  sensor_write(0xf5,0x78);
  sensor_write(0xfd,0x01);//P1
  sensor_write(0x00,0xb2);
  sensor_write(0x0f,0x60);
  sensor_write(0x16,0x60);
  sensor_write(0x17,0xa2);
  sensor_write(0x18,0xaa);
  sensor_write(0x1b,0x60);
  sensor_write(0x1c,0xaa);
  sensor_write(0xb4,0x20);
  sensor_write(0xb5,0x3a);
  sensor_write(0xb6,0x5e);
  sensor_write(0xb9,0x40);
  sensor_write(0xba,0x4f);
  sensor_write(0xbb,0x47);
  sensor_write(0xbc,0x45);
  sensor_write(0xbd,0x43);
  sensor_write(0xbe,0x42);
  sensor_write(0xbf,0x42);
  sensor_write(0xc0,0x42);
  sensor_write(0xc1,0x41);
  sensor_write(0xc2,0x41);
  sensor_write(0xc3,0x41);
  sensor_write(0xc4,0x41);
  sensor_write(0xc5,0x70);
  sensor_write(0xc6,0x41);
  sensor_write(0xca,0x70);
  sensor_write(0xcb,0xc );  //AEdebug end
  sensor_write(0xfd,0x00);  //P0
  sensor_write(0x32,0x15);  //Auto_mode set
  sensor_write(0x34,0x66);  //Isp_mode set
  sensor_write(0x35,0x00);  //out format
	 return 0;
}
  

static int 
sensor_record(void)
{
	return 0;
	dv_flag= 1;
	sensor_write(0xfd,0x00);
	sensor_write(0x05,0x0 );
	sensor_write(0x06,0x0 );
	sensor_write(0x09,0x1 );
	sensor_write(0x0a,0x76);
	sensor_write(0xf0,0x62);
	sensor_write(0xf1,0x0 );
	sensor_write(0xf2,0x5f);
	sensor_write(0xf5,0x78);
	sensor_write(0xfd,0x01);
	sensor_write(0x00,0xa9);
	sensor_write(0x0f,0x60);
	sensor_write(0x16,0x60);
	sensor_write(0x17,0x91);
	sensor_write(0x18,0x99);
	sensor_write(0x1b,0x60);
	sensor_write(0x1c,0x99);
	sensor_write(0xb4,0x20);
	sensor_write(0xb5,0x3a);
	sensor_write(0xb6,0x5e);
	sensor_write(0xb9,0x40);
	sensor_write(0xba,0x4f);
	sensor_write(0xbb,0x47);
	sensor_write(0xbc,0x45);
	sensor_write(0xbd,0x43);
	sensor_write(0xbe,0x78);
	sensor_write(0xbf,0x42);
	sensor_write(0xc0,0x42);
	sensor_write(0xc1,0x41);
	sensor_write(0xc2,0x41);
	sensor_write(0xc3,0x41);
	sensor_write(0xc4,0x41);
	sensor_write(0xc5,0x41);
	sensor_write(0xc6,0x41);
	sensor_write(0xca,0x78);
	sensor_write(0xcb,0x5 );
	sensor_write(0x14,0x20);
	sensor_write(0x15,0x0f);		
	sensor_write(0xfd,0x00);			   
	return 0;
}

static int 
sensor_preview(void)
{

	return 0;
}

static int 
sensor_capture(void)
{
return 0;
}

static int 
sensor_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	 dv_flag= 0;

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
static unsigned int ExpStatus = 0;

static int 
sensor_s_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{
	int nRet = 0;
	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		if(ctrl->value) {	// Enable Auto AWB
			nRet = sensor_write(0x32,0x15);
			nRet = sensor_write(0xfd,0x00);			  
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {

		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			ExpStatus = ExpStatus & 0xf0;
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			ExpStatus = ExpStatus & 0xf0;
			ExpStatus = ExpStatus | 0x01;
		}else {
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	//SUNSHINE
			nRet = sensor_write(0xfd,0x00);//6500K
			nRet = sensor_write(0x32,0x05);
			nRet = sensor_write(0xfd,0x01);
			nRet = sensor_write(0x28,0x6b);
			nRet = sensor_write(0x29,0x48);
			nRet = sensor_write(0xfd,0x00);

		}else if(ctrl->value == 1) {	//CLOUDY
			nRet = sensor_write(0xfd,0x00); //7000K
			nRet = sensor_write(0x32,0x05);
			nRet = sensor_write(0xfd,0x01);
			nRet = sensor_write(0x28,0x71);
			nRet = sensor_write(0x29,0x41);
			nRet = sensor_write(0xfd,0x00);
			 
		}else if(ctrl->value == 2) {	//FLUORESCENCE
			nRet = sensor_write(0xfd,0x00); //4200K-5000K
			nRet = sensor_write(0x32,0x05);
			nRet = sensor_write(0xfd,0x01);
			nRet = sensor_write(0x28,0x5a);
			nRet = sensor_write(0x29,0x62);
			nRet = sensor_write(0xfd,0x00);
		}else if(ctrl->value == 3) {	//INCANDESCENCE
			nRet = sensor_write(0xfd,0x00); //2800K-3000K
			nRet = sensor_write(0x32,0x05);
			nRet = sensor_write(0xfd,0x01);
			nRet = sensor_write(0x28,0x41);
			nRet = sensor_write(0x29,0x71);
			nRet = sensor_write(0xfd,0x00);		
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value) {//night mode
			ExpStatus = ExpStatus & 0x0f;
			ExpStatus = ExpStatus | 0x10;
		}else {//normal
			ExpStatus = ExpStatus & 0x0f;
		}
		break;

	default:
		return -EINVAL;
	}

	switch(ExpStatus){
		case 0x00://50hz in normal light 
		//sp0838 24M 1分频 50Hz VGA 15.11-15.11fps 2xgain AE_Table
		sensor_write(0xfd,0x00);
		sensor_write(0x05,0x00);
		sensor_write(0x06,0x00);
		sensor_write(0x09,0x03);
		sensor_write(0x0a,0x03);
		sensor_write(0xf0,0x4a);
		sensor_write(0xf1,0x00);
		sensor_write(0xf2,0x59);
		sensor_write(0xf5,0x72);
		sensor_write(0xfd,0x01);
		sensor_write(0x00,0x9e);
		sensor_write(0x0f,0x5a);
		sensor_write(0x16,0x5a);
		sensor_write(0x17,0x8e);
		sensor_write(0x18,0x96);
		sensor_write(0x1b,0x5a);
		sensor_write(0x1c,0x96);
		sensor_write(0xb4,0x20);
		sensor_write(0xb5,0x3a);
		sensor_write(0xb6,0x46);
		sensor_write(0xb9,0x40);
		sensor_write(0xba,0x4f);
		sensor_write(0xbb,0x47);
		sensor_write(0xbc,0x45);
		sensor_write(0xbd,0x43);
		sensor_write(0xbe,0x42);
		sensor_write(0xbf,0x70);
		sensor_write(0xc0,0x42);
		sensor_write(0xc1,0x41);
		sensor_write(0xc2,0x41);
		sensor_write(0xc3,0x41);
		sensor_write(0xc4,0x41);
		sensor_write(0xc5,0x41);
		sensor_write(0xc6,0x41);
		sensor_write(0xca,0x70);
		sensor_write(0xcb,0x06);
		sensor_write(0xfd,0x00);
		break;

		case 0x01://60hz in normal light
		//sp0838 24M 1分频 60Hz VGA 15.18-15.18fps 2xgain AE_Table
		sensor_write(0xfd,0x00);
		sensor_write(0x05,0x00);
		sensor_write(0x06,0x00);
		sensor_write(0x09,0x02);
		sensor_write(0x0a,0xfb);
		sensor_write(0xf0,0x3e);
		sensor_write(0xf1,0x00);
		sensor_write(0xf2,0x56);
		sensor_write(0xf5,0x6f);
		sensor_write(0xfd,0x01);
		sensor_write(0x00,0xa1);
		sensor_write(0x0f,0x57);
		sensor_write(0x16,0x57);
		sensor_write(0x17,0x91);
		sensor_write(0x18,0x99);
		sensor_write(0x1b,0x57);
		sensor_write(0x1c,0x99);
		sensor_write(0xb4,0x20);
		sensor_write(0xb5,0x3a);
		sensor_write(0xb6,0x3a);
		sensor_write(0xb9,0x40);
		sensor_write(0xba,0x4f);
		sensor_write(0xbb,0x47);
		sensor_write(0xbc,0x45);
		sensor_write(0xbd,0x43);
		sensor_write(0xbe,0x42);
		sensor_write(0xbf,0x42);
		sensor_write(0xc0,0x42);
		sensor_write(0xc1,0x70);
		sensor_write(0xc2,0x41);
		sensor_write(0xc3,0x41);
		sensor_write(0xc4,0x41);
		sensor_write(0xc5,0x41);
		sensor_write(0xc6,0x41);
		sensor_write(0xca,0x70);
		sensor_write(0xcb,0x08);
		sensor_write(0xfd,0x00);
		break;

		case 0x10://50hz in nightmode
		//sp0838 24M 1分频 50Hz VGA 8.16-8.16fps 2xgain AE_Table
		sensor_write(0xfd,0x00);
		sensor_write(0x05,0x00);
		sensor_write(0x06,0x00);
		sensor_write(0x09,0x08);
		sensor_write(0x0a,0x66);
		sensor_write(0xf0,0x28);
		sensor_write(0xf1,0x00);
		sensor_write(0xf2,0x4c);
		sensor_write(0xf5,0x65);
		sensor_write(0xfd,0x01);
		sensor_write(0x00,0x9f);
		sensor_write(0x0f,0x4d);
		sensor_write(0x16,0x4d);
		sensor_write(0x17,0x8f);
		sensor_write(0x18,0x97);
		sensor_write(0x1b,0x4d);
		sensor_write(0x1c,0x97);
		sensor_write(0xb4,0x20);
		sensor_write(0xb5,0x26);
		sensor_write(0xb6,0x26);
		sensor_write(0xb9,0x40);
		sensor_write(0xba,0x4f);
		sensor_write(0xbb,0x47);
		sensor_write(0xbc,0x45);
		sensor_write(0xbd,0x43);
		sensor_write(0xbe,0x42);
		sensor_write(0xbf,0x42);
		sensor_write(0xc0,0x42);
		sensor_write(0xc1,0x41);
		sensor_write(0xc2,0x41);
		sensor_write(0xc3,0x41);
		sensor_write(0xc4,0x41);
		sensor_write(0xc5,0x70);
		sensor_write(0xc6,0x41);
		sensor_write(0xca,0x70);
		sensor_write(0xcb,0x0c);
		sensor_write(0xfd,0x00);
		break;

		case 0x11://60hz in nightmode
		//sp0838 24M 1分频 60Hz VGA 8.33-8.33fps 2xgain AE_Table
		sensor_write(0xfd,0x00);
		sensor_write(0x05,0x00);
		sensor_write(0x06,0x00);
		sensor_write(0x09,0x08);
		sensor_write(0x0a,0x2b);
		sensor_write(0xf0,0x22);
		sensor_write(0xf1,0x00);
		sensor_write(0xf2,0x49);
		sensor_write(0xf5,0x62);
		sensor_write(0xfd,0x01);
		sensor_write(0x00,0xa1);
		sensor_write(0x0f,0x4a);
		sensor_write(0x16,0x4a);
		sensor_write(0x17,0x91);
		sensor_write(0x18,0x99);
		sensor_write(0x1b,0x4a);
		sensor_write(0x1c,0x99);
		sensor_write(0xb4,0x20);
		sensor_write(0xb5,0x20);
		sensor_write(0xb6,0x20);
		sensor_write(0xb9,0x40);
		sensor_write(0xba,0x4f);
		sensor_write(0xbb,0x47);
		sensor_write(0xbc,0x45);
		sensor_write(0xbd,0x43);
		sensor_write(0xbe,0x42);
		sensor_write(0xbf,0x42);
		sensor_write(0xc0,0x42);
		sensor_write(0xc1,0x41);
		sensor_write(0xc2,0x41);
		sensor_write(0xc3,0x41);
		sensor_write(0xc4,0x41);
		sensor_write(0xc5,0x41);
		sensor_write(0xc6,0x41);
		sensor_write(0xca,0x70);
		sensor_write(0xcb,0x0f);
		sensor_write(0xfd,0x00);
		break;
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
	return 0;
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
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
	if(sensor_i2c_open(SP0838_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: sp0838 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_sp0838");
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
MODULE_DESCRIPTION("Generalplus sp0838 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

