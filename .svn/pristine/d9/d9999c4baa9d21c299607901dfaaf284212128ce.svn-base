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
 *                         H E A D E R   F I L E S												*
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
#define OV7670_ID			0x42
#define OV7670_WIDTH		640
#define OV7670_HEIGHT		480
#define OV7670_YUYV			0x00
#define OV7670_BAYER_RAW	0x01
#define COLOR_BAR			0x02
#define COLOR_BAR_EN 		0

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
	/* capature mode */
	{
		.desc		= "capture=640*480",
		.pixelformat = V4L2_PIX_FMT_YUYV,
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
		.desc		= "preview=640*480",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
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
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
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
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
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

static regval8_t ov7670_init_table[] = 
{
	{ 0x11, 0x02 },	
	{ 0x3a, 0x04 },	
	{ 0x12, OV7670_YUYV },
	{ 0x17, 0x13 }, /* h set */	
	{ 0x18, 0x01 },
	{ 0x32, 0xb6 },
	{ 0x19, 0x02 }, /* v set */
	{ 0x1a, 0x7a },
	{ 0x03, 0x0a },
	{ 0x0c, 0x00 },
	{ 0x3e, 0x00 },
	{ 0x70, 0x3a },
	{ 0x71, 0x35 },
	{ 0x72, 0x11 },
	{ 0x73, 0xf0 },
	{ 0xa2, 0x02 },
	
	{ 0x7a, 0x24 },
	{ 0x7b, 0x04 },
	{ 0x7c, 0x0a },
	{ 0x7d, 0x17 },
	{ 0x7e, 0x32 },
	{ 0x7f, 0x3f },
	{ 0x80, 0x4c },
	{ 0x81, 0x58 },
	{ 0x82, 0x64 },
	{ 0x83, 0x6f },
	{ 0x84, 0x7a },
	{ 0x85, 0x8c },
	{ 0x86, 0x9e },
	{ 0x87, 0xbb },
	{ 0x88, 0xd2 },
	{ 0x89, 0xe5 },
	
	{ 0x13, 0xe0 },
	{ 0x00, 0x00 },
	{ 0x10, 0x00 },
	{ 0x0d, 0x40 },
	{ 0x14, 0x18 },
	{ 0xa5, 0x02 },
	{ 0xab, 0x03 },
	{ 0x24, 0x95 },
	{ 0x25, 0x33 },
	{ 0x26, 0xe3 },
	{ 0x9f, 0x78 },
	{ 0xa0, 0x68 },
	{ 0xa1, 0x03 },
	{ 0xa6, 0xd8 },
	{ 0xa7, 0xd8 },
	{ 0xa8, 0xf0 },
	{ 0xa9, 0x90 },
	{ 0xaa, 0x94 },
	{ 0x13, 0xe5 },
	
	{ 0x0e, 0x61 },
	{ 0x0f, 0x4b },
	{ 0x16, 0x02 },
	{ 0x1e, 0x3f },	/* Flip & Mirror */
	{ 0x21, 0x02 },
	{ 0x22, 0x91 },
	{ 0x29, 0x07 },
	{ 0x33, 0x0b },
	{ 0x35, 0x0b },
	{ 0x37, 0x1d },
	{ 0x38, 0x71 },
	{ 0x39, 0x2a },
	{ 0x3c, 0x78 },
	{ 0x4d, 0x40 },
	{ 0x4e, 0x20 },
	{ 0x69, 0x00 },	

	{ 0x6b, 0x8a },	/* pclk*6 */	
	{ 0x2d, 0x00 },	/* dummy byte */
	{ 0x2e, 0x00 },
	
	{ 0x74, 0x10 },
	{ 0x8d, 0x4f },
	{ 0x8e, 0x00 },
	{ 0x8f, 0x00 },
	{ 0x90, 0x00 },
	{ 0x91, 0x00 },
	
	{ 0x96, 0x00 },
	{ 0x9a, 0x80 },
	{ 0xb0, 0x84 },
	{ 0xb1, 0x0c },
	{ 0xb2, 0x0e },
	{ 0xb3, 0x82 },
	{ 0xb8, 0x0a },
	
	{ 0x43, 0x0a },
	{ 0x44, 0xf0 },
	{ 0x45, 0x44 },
	{ 0x46, 0x7a },
	{ 0x47, 0x27 },
	{ 0x48, 0x3c },
	{ 0x59, 0xbc },
	{ 0x5a, 0xde },
	{ 0x5b, 0x54 },
	{ 0x5c, 0x8a },
	{ 0x5d, 0x4b },
	{ 0x5e, 0x0f },
	{ 0x6c, 0x0a },
	{ 0x6d, 0x55 },
	{ 0x6e, 0x11 },
	{ 0x6f, 0x9e },
	
	{ 0x6a, 0x40 },
	{ 0x01, 0x40 },
	{ 0x02, 0x40 },
	{ 0x13, 0xe7 },
	
	{ 0x4f, 0x80 },
	{ 0x50, 0x80 },
	{ 0x51, 0x00 },
	{ 0x52, 0x22 },
	{ 0x53, 0x5e },
	{ 0x54, 0x80 },
	{ 0x58, 0x9e },
	
	{ 0x62, 0x08 },
	{ 0x63, 0x8f },
	{ 0x65, 0x00 },
	{ 0x64, 0x08 },
	{ 0x94, 0x08 },
	{ 0x95, 0x0c },
	{ 0x66, 0x05 },
	
	{ 0x41, 0x08 },
	{ 0x3f, 0x00 },
	{ 0x75, 0x05 },
	{ 0x76, 0xe1 },
	{ 0x4c, 0x00 },
	{ 0x77, 0x01 },
	{ 0x3d, 0x08 },	/* disable gamma and uv saturation */
	{ 0x4b, 0x09 },
	{ 0xc9, 0x60 },
	{ 0x41, 0x38 },
	{ 0x56, 0x40 },
	
	{ 0x34, 0x11 },
	{ 0x3b, 0x4a },	/* disable night mode */
	{ 0xa4, 0x88 },
	{ 0x96, 0x00 },
	{ 0x97, 0x30 },
	{ 0x98, 0x20 },
	{ 0x99, 0x30 },
	{ 0x9a, 0x84 },
	{ 0x9b, 0x29 },
	{ 0x9c, 0x03 },
	{ 0x9d, 0x98 },
	{ 0x9e, 0x7f },
	{ 0x78, 0x04 },
	
	{ 0x79, 0x01 },
	{ 0xc8, 0xf0 },
	{ 0x79, 0x0f },
	{ 0xc8, 0x00 },
	{ 0x79, 0x10 },
	{ 0xc8, 0x7e },
	{ 0x79, 0x0a },
	{ 0xc8, 0x80 },
	{ 0x79, 0x0b },
	{ 0xc8, 0x01 },
	{ 0x79, 0x0c },
	{ 0xc8, 0x0f },
	{ 0x79, 0x0d },
	{ 0xc8, 0x20 },
	{ 0x79, 0x09 },
	{ 0xc8, 0x80 },
	{ 0x79, 0x02 },
	{ 0xc8, 0xc0 },
	{ 0x79, 0x03 },
	{ 0xc8, 0x40 },
	{ 0x79, 0x05 },
	{ 0xc8, 0x30 },
	{ 0x79, 0x26 },
	{ 0x3b, 0x08 },/* 60Hz = 0x00, 50Hz = 0x08 */
	{ 0xff, 0xff },
};

static regval8_t ov7670_raw_fmt[] = 
{
#if COLOR_BAR_EN == 1
	{ 0x42, 0x08 },
	{ 0x12, OV7670_BAYER_RAW | COLOR_BAR },
#else
	{ 0x12, OV7670_BAYER_RAW },
#endif
	{ 0xff, 0xff },
};

static regval8_t ov7670_yuv_fmt[] = 
{
#if COLOR_BAR_EN == 1
	{ 0x42, 0x08 },
	{ 0x12, OV7670_YUYV | COLOR_BAR },
#else
	{ 0x12, OV7670_YUYV },  /* Selects YUV mode */
#endif
	{ 0xff, 0xff },
};

static regval8_t ov7670_resume_table[] = 
{
	{0xff, 0xff},	//end
};

static regval8_t ov7670_suspend_table[] = 
{
	{0xff, 0xff},	//end
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
	g_ti2c_handle.pDeviceString = "OV7670";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV7670 ti2c request failed\n");
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
	return sensor_write_table(ov7670_init_table);
}

static int 
sensor_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
	return sensor_i2c_write(0x12, 0x80);
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
	unsigned char data = 0;
	int nRet = 0;

	printk("%s\n", __FUNCTION__);
	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		nRet = sensor_i2c_read(0x3b, &data);
		if(data & 0x08) {
			ctrl->value = 50;
		} else {
			ctrl->value = 60;
		}
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
	int nRet = 0;

	printk("%s\n", __FUNCTION__);
	printk("%s\n", g_fmt_table[fmt->fmt.pix.priv].desc);
	if(fmt->fmt.pix.priv < 3) {
		nRet = sensor_write_table(ov7670_yuv_fmt);
	} else if(fmt->fmt.pix.priv < 6) {
		nRet = sensor_write_table(ov7670_raw_fmt);
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
	return sensor_write_table(ov7670_suspend_table);
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
	printk("%s\n", __FUNCTION__);
	return sensor_write_table(ov7670_resume_table);
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
	if(sensor_i2c_open(OV7670_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: sensor ov7670 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov7670");
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