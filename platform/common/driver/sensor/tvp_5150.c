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
#define TVP5150_ID					0xB8

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

static char *param[] = {"1", "PORT0", "0", "NONE", "0", "NONE"};
static int nstrs = 6;
module_param_array(param, charp, &nstrs, S_IRUGO);

static sensor_fmt_t g_fmt_table[] =
{
	/* preview mode */
	{
		.desc		= "NTSC=720*480",
		.pixelformat = V4L2_PIX_FMT_VYUY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 27000000,
		.hpixel = 720,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
	/* capature mode */
	{
		.desc		= "PAL=720*576",
		.pixelformat = V4L2_PIX_FMT_VYUY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 27000000,
		.hpixel = 720,
		.hoffset = 0,
		.vline = 576,
		.voffset = 0,
	},
};

#define C_SENSOR_FMT_MAX	sizeof(g_fmt_table)/sizeof(sensor_fmt_t)

static sensor_config_t config_table =
{
	.sensor_timing_mode = MODE_CCIR_656,
	.sensor_data_mode = MODE_DATA_YUV,
	.sensor_interlace_mode = MODE_INTERLACE,
	.sensor_pclk_mode = MODE_POSITIVE_EDGE,
	.sensor_hsync_mode = MODE_ACTIVE_LOW,
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
		return 0;
	}

	return g_i2c_handle;

#elif (I2C_MODE == HW_TI2C)
	g_ti2c_handle.pDeviceString = "TVP5150";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("TVP5150 ti2c request failed\n");
		return 0;
	}

	return 1;
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
	unsigned char reg,
	unsigned char *value
)
{
#if (I2C_MODE == HW_I2C)
	char addr[0], data[0];
	int nRet;
	
	addr[0] = reg ;
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
	unsigned char reg,
	unsigned char value
)
{
#if (I2C_MODE == HW_I2C)
	char data[2];
	data[0] = reg ;
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
	printk("%s\n", __FUNCTION__);
	sensor_write(0x05, 0x01);
	sensor_write(0x05, 0x00);
	sensor_write(0x02, 0x00);
	sensor_write(0xD0, 0xFF);
	sensor_write(0x0f, 0x0a);
	sensor_write(0x03, 0x6d);
	return sensor_write(0x0d, 0x47);
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
sensor_querystd(
	struct v4l2_subdev *sd,
	v4l2_std_id *std
)
{
	int ret = 0;
	unsigned char value;
	ret = sensor_read(0x8c, &value);
	if (ret < 0) {
		ret = -EIO;
	}
	else {
		if (value & 0x06) {
			*std = V4L2_STD_PAL;
		}
		else {
			*std = V4L2_STD_NTSC;
		} 
	}
	return ret;
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
	if(sensor_i2c_open(TVP5150_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: tvp5150 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "tvin_tvp5150");
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
MODULE_DESCRIPTION("Generalplus tvp5150 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

