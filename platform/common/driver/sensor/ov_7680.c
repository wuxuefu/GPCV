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
#define	OV7680_ID	0x42

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

static int select_fmt = 0;

static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x12 ,0x80);
	sensor_write(0x1e ,0x91);
	sensor_write(0x14 ,0x81);
	sensor_write(0x63 ,0x0b);
	sensor_write(0x64 ,0x0f);
	sensor_write(0x65 ,0x07);
	sensor_write(0x12 ,0x01);
	sensor_write(0x0c ,0x06);
	sensor_write(0x82 ,0x9A);
	sensor_write(0x22 ,0x40);
	sensor_write(0x23 ,0x20);
	sensor_write(0x31 ,0x11);
	sensor_write(0x40 ,0x10);
	sensor_write(0x4a ,0x42);
	sensor_write(0x4b ,0x64);
	sensor_write(0x67 ,0x50);
	sensor_write(0x6b ,0x00);
	sensor_write(0x6c ,0x00);
	sensor_write(0x6d ,0x00);
	sensor_write(0x6e ,0x00);
	sensor_write(0x42 ,0x4a);
	sensor_write(0x45 ,0x40);
	sensor_write(0x3f ,0x46);
	sensor_write(0x48 ,0x20);
	sensor_write(0x21 ,0x23);
	sensor_write(0x5a ,0x10);
	sensor_write(0x5c ,0xe0);
	sensor_write(0x27 ,0x33);
	sensor_write(0x4d ,0x1d);
	sensor_write(0x42 ,0x4a);
	sensor_write(0x4e ,0x54);
	sensor_write(0x65 ,0x87);
	sensor_write(0x66 ,0x02);
	sensor_write(0x67 ,0x5c);
	sensor_write(0x32 ,0x20);
	sensor_write(0x33 ,0x00);
	sensor_write(0x34 ,0x20);
	sensor_write(0x35 ,0x28);
	sensor_write(0x36 ,0x20);
	sensor_write(0x37 ,0x24);
	sensor_write(0x2f ,0x0c);
	sensor_write(0x30 ,0x10);
	sensor_write(0x38 ,0x84);
	sensor_write(0x31 ,0x0f);
	sensor_write(0x55 ,0x86);
	sensor_write(0x58 ,0x83);
	sensor_write(0x38 ,0x84);
	sensor_write(0x59 ,0x0e);
	sensor_write(0x5f ,0x20);
	sensor_write(0x56 ,0x40);
	sensor_write(0x51 ,0x00);
	sensor_write(0x57 ,0x22);
	sensor_write(0x59 ,0x07);
	sensor_write(0x5f ,0xa3);
	sensor_write(0x59 ,0x08);
	sensor_write(0x5f ,0x54);
	sensor_write(0x52 ,0x30);
	sensor_write(0x56 ,0x29);
	sensor_write(0x53 ,0x20);
	sensor_write(0x54 ,0x30);
	sensor_write(0x59 ,0x00);
	sensor_write(0x5f ,0xf0);
	sensor_write(0x59 ,0x01);
	sensor_write(0x5f ,0xF0);
	sensor_write(0x59 ,0x0F);
	sensor_write(0x5f ,0x20);
	sensor_write(0x5f ,0x00);
	sensor_write(0x59 ,0x10);
	sensor_write(0x5f ,0x7E);
	sensor_write(0x59 ,0x0A);
	sensor_write(0x5f ,0x80);
	sensor_write(0x59 ,0x0B);
	sensor_write(0x5f ,0x01);
	sensor_write(0x59 ,0x0C);
	sensor_write(0x5f ,0x07);
	sensor_write(0x5f ,0x0F);
	sensor_write(0x59 ,0x0D);
	sensor_write(0x5f ,0x20);
	sensor_write(0x59 ,0x09);
	sensor_write(0x5f ,0x30);
	sensor_write(0x59 ,0x02);
	sensor_write(0x5f ,0x80);
	sensor_write(0x59 ,0x03);
	sensor_write(0x5f ,0x60);
	sensor_write(0x59 ,0x04);
	sensor_write(0x5f ,0xF0);
	sensor_write(0x59 ,0x05);
	sensor_write(0x5f ,0x80);
	sensor_write(0x59 ,0x06);
	sensor_write(0x5f ,0x04);
	sensor_write(0x59 ,0x26);
	sensor_write(0x59 ,0x0b);
	sensor_write(0x5f ,0x31);
	sensor_write(0x55 ,0xa8);
	sensor_write(0x27 ,0xb3);
	sensor_write(0x40 ,0x23);
	sensor_write(0x4d ,0x1d);
	sensor_write(0x12 ,0x00);
	sensor_write(0x0c ,0x06);
	sensor_write(0x82 ,0x98);
	sensor_write(0x3e ,0x30);
	sensor_write(0x81 ,0x06);
	sensor_write(0xb7 ,0x98);
	sensor_write(0xb8 ,0x98);
	sensor_write(0xb9 ,0x00);
	sensor_write(0xba ,0x28);
	sensor_write(0xbb ,0x70);
	sensor_write(0xbc ,0x98);
	sensor_write(0xbd ,0x5a);
	sensor_write(0xbe ,0xb0);
	sensor_write(0xbf ,0x9d);
	sensor_write(0xc0 ,0x13);
	sensor_write(0xc1 ,0x16);
	sensor_write(0xc2 ,0x7b);
	sensor_write(0xc3 ,0x91);
	sensor_write(0xc4 ,0x1e);
	sensor_write(0xc5 ,0x9d);
	sensor_write(0xc6 ,0x9a);
	sensor_write(0xc7 ,0x03);
	sensor_write(0xc8 ,0x2e);
	sensor_write(0xc9 ,0x91);
	sensor_write(0xca ,0xbf);
	sensor_write(0xcb ,0x1e);
	sensor_write(0xaf ,0x1a);
	sensor_write(0xa0 ,0x05);
	sensor_write(0xa1 ,0x10);
	sensor_write(0xa2 ,0x28);
	sensor_write(0xa3 ,0x58);
	sensor_write(0xa4 ,0x6a);
	sensor_write(0xa5 ,0x7c);
	sensor_write(0xa6 ,0x88);
	sensor_write(0xa7 ,0x94);
	sensor_write(0xa8 ,0x9c);
	sensor_write(0xa9 ,0xa6);
	sensor_write(0xaa ,0xb4);
	sensor_write(0xab ,0xc1);
	sensor_write(0xac ,0xd0);
	sensor_write(0xad ,0xdd);
	sensor_write(0xae ,0xec);
	sensor_write(0x89 ,0x5c);
	sensor_write(0x8a ,0x11);
	sensor_write(0x8b ,0x92);
	sensor_write(0x8c ,0x11);
	sensor_write(0x8d ,0x52);
	sensor_write(0x96 ,0xff);
	sensor_write(0x97 ,0x00);
	sensor_write(0x9c ,0xf0);
	sensor_write(0x9d ,0xf0);
	sensor_write(0x9e ,0xf0);
	sensor_write(0xb2 ,0x06);
	sensor_write(0xb3 ,0x03);
	sensor_write(0xb4 ,0x05);
	sensor_write(0xb5 ,0x04);
	sensor_write(0xb6 ,0x02);
	sensor_write(0xd5 ,0x02);
	sensor_write(0xdb ,0x40);
	sensor_write(0xdc ,0x40);
	sensor_write(0x11 ,0x01);
	sensor_write(0x24 ,0x94);
	sensor_write(0x25 ,0x84);
	sensor_write(0x26 ,0xb3);
	sensor_write(0x2c ,0x67);
	sensor_write(0x4f ,0x9a);//0x4c
	sensor_write(0x50 ,0x80);//0x3f
	sensor_write(0x21 ,0x79);
	sensor_write(0x5a ,0x14);
	sensor_write(0x5b ,0xe3);
	sensor_write(0x5c ,0x70);
	sensor_write(0x5d ,0x30);
	sensor_write(0x81 ,0x07);
	sensor_write(0x84 ,0x02);
	return sensor_write(0x13 ,0xff);
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);
	select_fmt = 0;
	sensor_write(0x11 ,0x01);
	return sensor_write(0x0e ,0x02);
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);
	select_fmt = 1;
	return 	sensor_write(0x11 ,0x03);
}

static int 
sensor_record(void)
{
	printk("%s\n", __FUNCTION__);
	select_fmt = 2;
	
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
		nRet = sensor_read(0x13,(unsigned char *)&data); 					
		if(ctrl->value) {	// Enable Auto AWB
			nRet = sensor_write(0x13,data|0x02);
			nRet = sensor_write(0x01, 0x40);
			nRet = sensor_write(0x02, 0x40);
			nRet = sensor_write(0x03, 0x40);					  
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {

		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(0x4f ,0x9a);
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_write(0x50 ,0x80);
		}else {
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		nRet = sensor_read(0x13, (unsigned char *)&data);
		if(ctrl->value == 0) {	//SUNSHINE
			nRet = sensor_write(0x13,data& ~02);	
			nRet = sensor_write(0x01, 0x4e);
			nRet = sensor_write(0x02, 0xc8);
			nRet = sensor_write(0x03, 0x80);
		}else if(ctrl->value == 1) {	//CLOUDY
			nRet = sensor_write(0x13,data& ~02);  // Manual AWB mode
			nRet = sensor_write(0x01, 0x58);
			nRet = sensor_write(0x02, 0x60);
			nRet = sensor_write(0x03, 0x40);		
		}else if(ctrl->value == 2) {	//FLUORESCENCE
			nRet = sensor_write(0x13,data& ~02);  // Manual AWB mode
			nRet = sensor_write(0x01, 0x98);
			nRet = sensor_write(0x02, 0xb3);
			nRet = sensor_write(0x03, 0x40);		
		}else if(ctrl->value == 3) {	//INCANDESCENCE
			nRet = sensor_write(0x13,data& ~02);  
 			nRet = sensor_write(0x01, 0x5a);
			nRet = sensor_write(0x02, 0x5c);
			nRet = sensor_write(0x03, 0x40);		
		}
		break; 
		
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		nRet = sensor_read(0x15, (unsigned char *)&data);		
		if(ctrl->value) {
		nRet = sensor_write(0x15, data|0xa0);
		nRet = sensor_write(0x24, 0xa4);
		nRet = sensor_write(0x25, 0x94);
		}else {
		sensor_init(sd, 0 );
		//nRet = sensor_write(0x15, 0x00);
		//nRet = sensor_write(0x24, 0x94);
		//nRet = sensor_write(0x25, 0x84);
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
	sensor_write(0x09, 0x11);
	return sensor_write(0x39, 0x4b);
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
	sensor_write(0x09, 0x01);
	return sensor_write(0x39, 0x43);
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
	if(sensor_i2c_open(OV7680_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: ov7680 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov7680");
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
MODULE_DESCRIPTION("Generalplus ov7680 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

