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
#include <linux/fs.h> /* everything... */
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <mach/gp_mipi.h>
#include <mach/gp_i2c_bus.h>
#include <mach/sensor_mgr.h>

#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define OV9655_ID		0x60
#define	OV9655_WIDTH	1280
#define	OV9655_HEIGHT	1024

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
typedef struct regval_list_s 
{
	unsigned char reg_num;
	unsigned char value;
}regval8_t;

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
		.pixelformat = V4L2_PIX_FMT_YUYV,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 640,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
	/* preview mode */
	{
		.desc		= "capture=640*480",
		.pixelformat = V4L2_PIX_FMT_RGB565,
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

static const regval8_t ov9655_default_regs[] = 
{
	{ 0x12, 0x80 },
	{ 0x00, 0x00 },
//	{ 0x00, 0x1a },
	{ 0x01, 0x80 },
	{ 0x02, 0x80 },
	{ 0x03, 0x00 }, //0x12,
	{ 0x04, 0x03 },
	{ 0x0b, 0x57 },
	{ 0x0e, 0x61 },
	{ 0x0f, 0x43 }, //0x42,
    
	{ 0x11, 0x00 }, //0x80,
	{ 0x12, 0x62 },
	{ 0x13, 0xe7 },
	{ 0x14, 0x2e }, //0x3e,
	{ 0x15, 0x04 },
	{ 0x16, 0x14 }, //0x24, //0x04,
	{ 0x17, 0x16 },
	{ 0x18, 0x02 },
	{ 0x19, 0x01 },
	{ 0x1a, 0x3d },
	{ 0x1e, 0x34 },
//	{ 0x1e, 0x04 },
    
	{ 0x24, 0x43 }, //0x3c,
	{ 0x25, 0x33 },
	{ 0x26, 0x92 }, //0x82, //0xf2, //0x72,
	{ 0x27, 0x08 },
	{ 0x28, 0x08 },
	{ 0x29, 0x00 },
	{ 0x2a, 0x00 }, //dummy pixel number=0x2a[7:4]x256+0x2b[7:0]
	{ 0x2b, 0x19 }, //dummy pixel number=0x2a[7:4]x256+0x2b[7:0]
	{ 0x2c, 0x08 },
    
	{ 0x32, 0x80 }, //0xff,
	{ 0x33, 0x00 },
	{ 0x34, 0x3F },
	{ 0x35, 0x00 },
	{ 0x36, 0xfa },
	{ 0x37, 0x08 },
	{ 0x38, 0x72 },
	{ 0x39, 0x57 },
	{ 0x3a, 0x80 }, //0x82, //0x80,
//	{ 0x3b, 0xa4 }, //0x44,
	{ 0x3b, 0x44 },
	{ 0x3d, 0x99 },
	{ 0x3e, 0x0c },
	{ 0x3f, 0xc1 },
//	{ 0x3f, 0x84 },
    
	{ 0x40, 0xc0 },
	{ 0x41, 0x00 },
	{ 0x42, 0xd1 }, //0xc0,
	{ 0x43, 0x0a },
	{ 0x44, 0xf0 },
	{ 0x45, 0x46 },
	{ 0x46, 0x62 },
	{ 0x47, 0x2a },
	{ 0x48, 0x3c },
	{ 0x4a, 0xfc },
	{ 0x4b, 0xfc },
	{ 0x4c, 0x7f },
	{ 0x4d, 0x7f },
	{ 0x4e, 0x7f },
	             
	// 0.8x saturation
	{ 0x4f, 0x79 },
	{ 0x50, 0x79 },
	{ 0x51, 0x00 },
	{ 0x52, 0x20 },
	{ 0x54, 0x8c },
	{ 0x58, 0x1a },

	{ 0x59, 0x85 },
	{ 0x5a, 0xa9 },
	{ 0x5b, 0x64 },
	{ 0x5c, 0x84 },
	{ 0x5d, 0x53 },
	{ 0x5e, 0x0e },
	{ 0x5f, 0xf0 },

	{ 0x60, 0xf0 },
	{ 0x61, 0xf0 },
	{ 0x62, 0x00 },
	{ 0x63, 0x00 },
	{ 0x64, 0x04 },
	{ 0x65, 0x20 },
	{ 0x66, 0x00 },
	{ 0x69, 0x0a },
	{ 0x6a, 0x02 }, // 50Hz Banding Max AEC Step
	{ 0x6b, 0x4a },
	{ 0x6c, 0x04 },
	{ 0x6d, 0x55 },
	{ 0x6e, 0x00 },
	{ 0x6f, 0x9d },

	{ 0x70, 0x10 }, //0x21,
//	{ 0x70, 0x09 },
	{ 0x71, 0x78 },
	{ 0x72, 0xcc }, //0x00,
	{ 0x73, 0x00 },
	{ 0x74, 0x3a },
	{ 0x75, 0x35 },
	{ 0x76, 0x01 },
	{ 0x77, 0x03 }, //0x02,

	//OV 2
	{ 0x7a, 0x20 },
	{ 0x7b, 0x09 },
	{ 0x7c, 0x18 },
	{ 0x7d, 0x30 },
	{ 0x7e, 0x58 },
	{ 0x7f, 0x66 },
	
	{ 0x80, 0x72 },
	{ 0x81, 0x7d },
	{ 0x82, 0x86 },
	{ 0x83, 0x8f },
	{ 0x84, 0x97 },
	{ 0x85, 0xa5 },
	{ 0x86, 0xb2 },
	{ 0x87, 0xc7 },
	{ 0x88, 0xd8 },
	{ 0x89, 0xe8 },
	
	{ 0x8a, 0x45 }, //0x03,
	{ 0x8c, 0x8d },
	
	{ 0x90, 0x7d },
	{ 0x91, 0x7b },
	{ 0x9d, 0x02 },
	{ 0x9e, 0x02 },
	{ 0x9f, 0x7a },
	
	{ 0xa0, 0x79 },
	{ 0xa1, 0x40 },
	{ 0xa2, 0x96 }, //50Hz Fliker for 30FPS
	{ 0xa3, 0x7d }, //60Hz Fliker for 30FPS
	{ 0xa4, 0x50 },
	{ 0xa5, 0x68 },
	{ 0xa6, 0x4a },
//	{ 0xa6, 0x40 },
	{ 0xa8, 0xc1 },
	{ 0xa9, 0xef },
	{ 0xaa, 0x92 },
	{ 0xab, 0x04 },
	{ 0xac, 0x80 },
	{ 0xad, 0x80 },
	{ 0xae, 0x80 },
	{ 0xaf, 0x80 },
	
	{ 0xb2, 0xf2 },
	{ 0xb3, 0x20 },
	{ 0xb4, 0x03 }, //0x00,
	{ 0xb5, 0x00 },
	{ 0xb6, 0xaf },
	{ 0xbb, 0xae },
	{ 0xbc, 0x7f },
	{ 0xbd, 0x7f },
	{ 0xbe, 0x7f },
	{ 0xbf, 0x7f },

	{ 0xc0, 0xaa },
	{ 0xc1, 0xc0 },
	{ 0xc2, 0x01 },
	{ 0xc3, 0x4e },
	{ 0xc5, 0x02 }, // 60Hz Banding Max AEC Step
	{ 0xc6, 0x05 },
	{ 0xc7, 0x80 },
	{ 0xc9, 0xe0 },
	{ 0xca, 0xe8 },
	{ 0xcb, 0xf0 },
	{ 0xcc, 0xd8 },
	{ 0xcd, 0x93 },

	{ 0x03, 0x00 }, //0x12,
	{ 0x12, 0x62 },
//	{ 0x12, 0x02 },
	{ 0x17, 0x17 },
	{ 0x18, 0x03 },
	{ 0x1a, 0x3d },
	{ 0x2a, 0x00 }, //dummy pixel number=0x2a[7:4]x256+0x2b[7:0]
	{ 0x2b, 0x19 }, //dummy pixel number=0x2a[7:4]x256+0x2b[7:0]
	{ 0x32, 0x80 }, //0x92, //0xa4,
	{ 0x34, 0x3F },
	{ 0x36, 0xfa },
	{ 0x65, 0x00 }, //0x20,
	{ 0x66, 0x01 }, //0x00,
	{ 0x69, 0x0a },
	{ 0x6a, 0x02 }, // 50Hz Banding Max AEC Step
	{ 0x73, 0x00 },
	{ 0x8c, 0x89 },
	{ 0x9d, 0x03 }, //0x02,
	{ 0x9e, 0x04 }, //0x02,
	{ 0xa2, 0x96 }, //50Hz Fliker for 30FPS
	{ 0xa3, 0x7d }, //60Hz Fliker for 30FPS
	{ 0xc5, 0x02 }, // 60Hz Banding Max AEC Step
	{ 0xc0, 0xaa },
	{ 0x14, 0x2e }, //0x3e,
	{ 0x13, 0xe7 },
	
	{ 0xa1, 0x40 },
	{ 0x10, 0x3e },
//	{ 0x10, 0x70 },
	{ 0x04, 0x02 },
	{ 0x2e, 0x00 },
	{ 0x2d, 0x00 },
	{ 0x01, 0x80 },
//	{ 0x01, 0x78 },
	{ 0xa6, 0x80 },
	{ 0x02, 0x80 },
//	{ 0x02, 0x40 },
	{ 0x05, 0x36 },
	{ 0x06, 0x3b },
	{ 0x07, 0x38 },
	{ 0x08, 0x3f },
	{ 0x09, 0x03 },
	{ 0x2f, 0x3b },

	{ 0xff, 0xff }	//end
};

static const regval8_t ov9655_fmt_yuv422[] = {
	{ 0xff, 0xff }
};

static const regval8_t ov9655_fmt_rgb565[] = {
	{ 0xff, 0xff }
};

static const regval8_t ov9655_resume_regs[] = {
	{ 0xc1, 0x00 },
	{ 0x49, 0x48 },
	{ 0x39, 0x57 },
	{ 0x09, 0x03 },
	{ 0xc1, 0xc0 },
	{ 0xff, 0xff }
};

static const regval8_t ov9655_suspend_regs[] = {
	{ 0x13, 0xe0 },
	{ 0x6b, 0x4a },
	{ 0x39, 0x5f },
	{ 0x38, 0x72 },
	{ 0x10, 0x00 },
	{ 0xa1, 0x00 },
	{ 0x04, 0x00 },
	{ 0x00, 0x00 },
	{ 0xc1, 0xc0 },
	{ 0x49, 0x08 },
	{ 0x09, 0x11 },
	{ 0x09, 0x01 },
	{ 0x09, 0x11 },
	{ 0xff, 0xff }
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
	g_ti2c_handle.pDeviceString = "OV9655";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV9655 ti2c request failed\n");
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
sensor_i2c_read(	
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
sensor_i2c_write(	
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
sensor_write_table(
	regval8_t *vals
)
{
	int i, nRet;
	
	while (vals->reg_num != 0xff || vals->value != 0xff) {
		for(i = 0; i< 10; i++) {
			nRet = sensor_i2c_write(vals->reg_num, vals->value);
			if(nRet >= 0) {
			#if 0
				unsigned char value;
				sensor_i2c_read(vals->reg_num, &value);
				printk("0x%x, 0x%x\n", vals->reg_num, value);
			#endif
				break;
			} else {
				printk("I2C Fail\n");
			}
		}
		if(i == 10) return -1;
		vals++;
	}
	return 0;
}

static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
	return sensor_write_table((regval8_t *)ov9655_default_regs);
}

static int 
sensor_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
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
	case V4L2_CID_BRIGHTNESS:
		qc->minimum = 0;
		qc->maximum = 255;
		qc->step = 1;
		qc->default_value = 128;
		break;
			
	case V4L2_CID_CONTRAST:
		qc->minimum = 0;
		qc->maximum = 127;
		qc->step = 1;
		qc->default_value = 64;
		break;

	case V4L2_CID_VFLIP:
		qc->minimum = 0;
		qc->maximum = 1;
		qc->step = 1;
		qc->default_value = 0;
		break;
	
	case V4L2_CID_HFLIP:
		qc->minimum = 0;
		qc->maximum = 1;
		qc->step = 1;
		qc->default_value = 0;
		break;
	
	case V4L2_CID_SATURATION:
		qc->minimum = 0;
		qc->maximum = 256;
		qc->step = 1;
		qc->default_value = 128;
		break;
	
	case V4L2_CID_HUE:
		qc->minimum = -180;
		qc->maximum = 180;
		qc->step = 5;
		qc->default_value = 0;
		break;
		
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
	printk("%s\n", __FUNCTION__);
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
	printk("%s\n", __FUNCTION__);
	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		break;

	default:
		return -EINVAL;
	}
	
	return 0; 
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
	int nRet = 0;

	printk("%s\n", __FUNCTION__);
	printk("%s\n", g_fmt_table[fmt->fmt.pix.priv].desc);
	if(fmt->fmt.pix.priv == 0) {
		nRet = sensor_write_table((regval8_t *)ov9655_fmt_yuv422);
	} else if(fmt->fmt.pix.priv == 1) {
		nRet = sensor_write_table((regval8_t *)ov9655_fmt_rgb565);
	} else {
		nRet = -1;
	}

	g_sensor_dev.fmt = &g_fmt_table[fmt->fmt.pix.priv];
	return nRet;
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
	printk("%s\n", __FUNCTION__);
	return sensor_write_table((regval8_t *)ov9655_suspend_regs);
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
	printk("%s\n", __FUNCTION__);
	return sensor_write_table((regval8_t *)ov9655_resume_regs);
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
	if(sensor_i2c_open(OV9655_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: sensor ov9655 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov9655");
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
MODULE_DESCRIPTION("Generalplus ov3640 mipi sensor Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");