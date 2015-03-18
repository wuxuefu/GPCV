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
#define OV2655_ID		0x60

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
		.desc		= "preview=800*600",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 800,
		.hoffset = 0,
		.vline = 600,
		.voffset = 0,
	},
	/* capature mode */
	{
		.desc		= "capture=1600*1200",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1600,
		.hoffset = 0,
		.vline = 1200,
		.voffset = 0,
	},
	/* record mode */
	{
		.desc		= "record=800*600",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 800,
		.hoffset = 0,
		.vline = 600,
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
	g_ti2c_handle.pDeviceString = "OV2655";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV2655 ti2c request failed\n");
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
	unsigned char addr[2], data[1];
	int nRet;
	
	addr[0] = (reg >> 8) & 0xFF;
	addr[1] = reg & 0xFF;
	nRet = gp_i2c_bus_write(g_i2c_handle, addr, 2);
	if(nRet <= 0) {
		return nRet;
	}
	
	nRet = gp_i2c_bus_read(g_i2c_handle, data, 1);
	*value = data[0];
	return nRet;
	
#elif (I2C_MODE == HW_TI2C)
	unsigned char addr[2], data[1];
	int nRet;
	
	addr[0] = (reg >> 8) & 0xFF;
	addr[1] = reg & 0xFF;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = addr;	
	g_ti2c_handle.dataCnt = 2;	
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
	unsigned char data[3];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = value;	
	return gp_i2c_bus_write(g_i2c_handle, data, 3);
	
#elif (I2C_MODE == HW_TI2C)
	unsigned char data[3];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = value;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 3;	
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
	sensor_write(0x3012, 0x80);	// Soft Reset, Add some dealy or wait a few miliseconds after register reset
	msleep(10);
	sensor_write(0x308c, 0x80);	// IO & Clock & Analog Setup
	sensor_write(0x308d, 0x0e);
	sensor_write(0x360b, 0x00);
	sensor_write(0x30b0, 0xff);
	sensor_write(0x30b1, 0xff);
	sensor_write(0x30b2, 0x24);
	
	sensor_write(0x300e, 0x34);
	sensor_write(0x300f, 0xa6);
	sensor_write(0x3010, 0x81);
	sensor_write(0x3082, 0x01);
	sensor_write(0x30f4, 0x01);
	sensor_write(0x3090, 0x33);
	sensor_write(0x3091, 0xc0);
	sensor_write(0x30ac, 0x42);
	
	sensor_write(0x30d1, 0x08);
	sensor_write(0x30a8, 0x56);
	sensor_write(0x3015, 0x03);
	sensor_write(0x3093, 0x00);
	sensor_write(0x307e, 0xe5);
	sensor_write(0x3079, 0x00);
	sensor_write(0x30aa, 0x42);
	sensor_write(0x3017, 0x40);
	sensor_write(0x30f3, 0x82);
	sensor_write(0x306a, 0x0c);
	sensor_write(0x306d, 0x00);
	sensor_write(0x336a, 0x3c);
	sensor_write(0x3076, 0x6a);
	sensor_write(0x30d9, 0x8c);
	sensor_write(0x3016, 0x82);
	sensor_write(0x3601, 0x30);
	sensor_write(0x304e, 0x88);
	sensor_write(0x30f1, 0x82);
	sensor_write(0x306f, 0x14);
	
	sensor_write(0x3012, 0x10);
	sensor_write(0x3011, 0x01);
	sensor_write(0x302A, 0x02);
	sensor_write(0x302B, 0xE6);
	sensor_write(0x3028, 0x07);
	sensor_write(0x3029, 0x93);
	
	sensor_write(0x3391, 0x06);	// saturation
	sensor_write(0x3394, 0x38);
	sensor_write(0x3395, 0x38);
	
	sensor_write(0x3015, 0x02);
	sensor_write(0x302d, 0x00);
	sensor_write(0x302e, 0x00);
	
	sensor_write(0x3013, 0xf7);	// AEC/AGC
	sensor_write(0x3018, 0x80);
	sensor_write(0x3019, 0x70);
	sensor_write(0x301a, 0xd4);
	
	sensor_write(0x30af, 0x00);	// D5060
	sensor_write(0x3048, 0x1f);
	sensor_write(0x3049, 0x4e);
	sensor_write(0x304a, 0x20);
	sensor_write(0x304f, 0x20);
	sensor_write(0x304b, 0x02);
	sensor_write(0x304c, 0x00);
	sensor_write(0x304d, 0x02);
	sensor_write(0x304f, 0x20);
	sensor_write(0x30a3, 0x10);
	sensor_write(0x3013, 0xf7);
	sensor_write(0x3014, 0x84);
	sensor_write(0x3071, 0x00);
	sensor_write(0x3070, 0x5d);
	sensor_write(0x3073, 0x00);
	sensor_write(0x3072, 0x5d);
	sensor_write(0x301c, 0x07);
	sensor_write(0x301d, 0x07);
	sensor_write(0x304d, 0x42);
	sensor_write(0x304a, 0x40);
	sensor_write(0x304f, 0x40);
	sensor_write(0x3095, 0x07);
	sensor_write(0x3096, 0x16);
	sensor_write(0x3097, 0x1d);
	
	sensor_write(0x3020, 0x01);	// Window Setup
	sensor_write(0x3021, 0x18);
	sensor_write(0x3022, 0x00);
	sensor_write(0x3023, 0x06);
	sensor_write(0x3024, 0x06);
	sensor_write(0x3025, 0x58);
	sensor_write(0x3026, 0x02);
	sensor_write(0x3027, 0x5e);
	sensor_write(0x3088, 0x03);
	sensor_write(0x3089, 0x20);
	sensor_write(0x308a, 0x02);
	sensor_write(0x308b, 0x58);
	sensor_write(0x3316, 0x64);
	sensor_write(0x3317, 0x25);
	sensor_write(0x3318, 0x80);
	sensor_write(0x3319, 0x08);
	sensor_write(0x331a, 0x64);
	sensor_write(0x331b, 0x4b);
	sensor_write(0x331c, 0x00);
	sensor_write(0x331d, 0x38);
	sensor_write(0x3100, 0x00);
	
	sensor_write(0x3320, 0xfa);	// AWB
	sensor_write(0x3321, 0x11);
	sensor_write(0x3322, 0x92);
	sensor_write(0x3323, 0x01);
	sensor_write(0x3324, 0x97);
	sensor_write(0x3325, 0x02);
	sensor_write(0x3326, 0xff);
	sensor_write(0x3327, 0x0c);
	sensor_write(0x3328, 0x10);
	sensor_write(0x3329, 0x10);
	sensor_write(0x332a, 0x54);
	sensor_write(0x332b, 0x52);
	sensor_write(0x332c, 0xbe);
	sensor_write(0x332d, 0xe1);
	sensor_write(0x332e, 0x3a);
	sensor_write(0x332f, 0x36);
	sensor_write(0x3330, 0x4d);
	sensor_write(0x3331, 0x44);
	sensor_write(0x3332, 0xf8);
	sensor_write(0x3333, 0x0a);
	sensor_write(0x3334, 0xf0);
	sensor_write(0x3335, 0xf0);
	sensor_write(0x3336, 0xf0);
	sensor_write(0x3337, 0x40);
	sensor_write(0x3338, 0x40);
	sensor_write(0x3339, 0x40);
	sensor_write(0x333a, 0x00);
	sensor_write(0x333b, 0x00); 
	
	sensor_write(0x3380, 0x28);	// Color Matrix
	sensor_write(0x3381, 0x48);
	sensor_write(0x3382, 0x10);
	sensor_write(0x3383, 0x22);
	sensor_write(0x3384, 0xc0);
	sensor_write(0x3385, 0xe2);
	sensor_write(0x3386, 0xe2);
	sensor_write(0x3387, 0xf2);
	sensor_write(0x3388, 0x10);
	sensor_write(0x3389, 0x98);
	sensor_write(0x338a, 0x00);
	
	sensor_write(0x3340, 0x04);	// Gamma
	sensor_write(0x3341, 0x07);
	sensor_write(0x3342, 0x19);
	sensor_write(0x3343, 0x34);
	sensor_write(0x3344, 0x4a);
	sensor_write(0x3345, 0x5a);
	sensor_write(0x3346, 0x67);
	sensor_write(0x3347, 0x71);
	sensor_write(0x3348, 0x7c);
	sensor_write(0x3349, 0x8c);
	sensor_write(0x334a, 0x9b);
	sensor_write(0x334b, 0xa9);
	sensor_write(0x334c, 0xc0);
	sensor_write(0x334d, 0xd5);
	sensor_write(0x334e, 0xe8);
	sensor_write(0x334f, 0x20);
	
	sensor_write(0x3350, 0x33);	// Lens correction
	sensor_write(0x3351, 0x28);
	sensor_write(0x3352, 0x00);
	sensor_write(0x3353, 0x14);
	sensor_write(0x3354, 0x00);
	sensor_write(0x3355, 0x85);
	sensor_write(0x3356, 0x35);
	sensor_write(0x3357, 0x28);
	sensor_write(0x3358, 0x00);
	sensor_write(0x3359, 0x13);
	sensor_write(0x335a, 0x00);
	sensor_write(0x335b, 0x85);
	sensor_write(0x335c, 0x34);
	sensor_write(0x335d, 0x28);
	sensor_write(0x335e, 0x00);
	sensor_write(0x335f, 0x13);
	sensor_write(0x3360, 0x00);
	sensor_write(0x3361, 0x85);
	sensor_write(0x3363, 0x70);
	sensor_write(0x3364, 0x7f);
	sensor_write(0x3365, 0x00);
	sensor_write(0x3366, 0x00);
	
	sensor_write(0x3362, 0x90);
	
	sensor_write(0x3301, 0xff);	// UVadjust
	sensor_write(0x338B, 0x14);
	sensor_write(0x338c, 0x10);
	sensor_write(0x338d, 0x40);
	
	sensor_write(0x3370, 0xd0);	// Sharpness/De-noise
	sensor_write(0x3371, 0x00);
	sensor_write(0x3372, 0x00);
	sensor_write(0x3373, 0x40);
	sensor_write(0x3374, 0x10);
	sensor_write(0x3375, 0x10);
	sensor_write(0x3376, 0x06);
	sensor_write(0x3377, 0x00);
	sensor_write(0x3378, 0x04);
	sensor_write(0x3379, 0x80);
	
	sensor_write(0x3069, 0x84);	// BLC
	sensor_write(0x307c, 0x10);
	sensor_write(0x3087, 0x02);
	
	sensor_write(0x3300, 0xfc);	// Other functions
	sensor_write(0x3302, 0x11);
	sensor_write(0x3400, 0x00);
	sensor_write(0x3606, 0x20);
	sensor_write(0x3601, 0x30);
	sensor_write(0x30f3, 0x83);
	sensor_write(0x304e, 0x88);
	
	sensor_write(0x30aa, 0x72);
	sensor_write(0x30a3, 0x80);
	sensor_write(0x30a1, 0x41);
	
	sensor_write(0x3086, 0x0f);
	return sensor_write(0x3086, 0x00);
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x3012, 0x10);
	sensor_write(0x302a, 0x02);
	sensor_write(0x302b, 0xE6);
	sensor_write(0x306f, 0x14);
	sensor_write(0x3362, 0x90);
	
	sensor_write(0x3070, 0x5d);
	sensor_write(0x3072, 0x5d);
	sensor_write(0x301c, 0x07);
	sensor_write(0x301d, 0x07);
	
	sensor_write(0x3020, 0x01);
	sensor_write(0x3021, 0x18);
	sensor_write(0x3022, 0x00);
	sensor_write(0x3023, 0x06);
	sensor_write(0x3024, 0x06);
	sensor_write(0x3025, 0x58);
	sensor_write(0x3026, 0x02);
	sensor_write(0x3027, 0x5E);
	sensor_write(0x3088, 0x03);
	sensor_write(0x3089, 0x20);
	sensor_write(0x308A, 0x02);
	sensor_write(0x308B, 0x58);
	sensor_write(0x3316, 0x64);
	sensor_write(0x3317, 0x25);
	sensor_write(0x3318, 0x80);
	sensor_write(0x3319, 0x08);
	sensor_write(0x331A, 0x64);
	sensor_write(0x331B, 0x4B);
	sensor_write(0x331C, 0x00);
	sensor_write(0x331D, 0x38);
	return sensor_write(0x3302, 0x11);
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x3012, 0x00);
	sensor_write(0x302a, 0x05);
	sensor_write(0x302b, 0xCB);
	sensor_write(0x306f, 0x54);
	sensor_write(0x3362, 0x80);
	
	sensor_write(0x3070, 0x5d);
	sensor_write(0x3072, 0x5d);
	sensor_write(0x301c, 0x0f);
	sensor_write(0x301d, 0x0f);
	
	sensor_write(0x3020, 0x01);
	sensor_write(0x3021, 0x18);
	sensor_write(0x3022, 0x00);
	sensor_write(0x3023, 0x0A);
	sensor_write(0x3024, 0x06);
	sensor_write(0x3025, 0x58);
	sensor_write(0x3026, 0x04);
	sensor_write(0x3027, 0xbc);
	sensor_write(0x3088, 0x06);
	sensor_write(0x3089, 0x40);
	sensor_write(0x308A, 0x04);
	sensor_write(0x308B, 0xB0);
	sensor_write(0x3316, 0x64);
	sensor_write(0x3317, 0x4B);
	sensor_write(0x3318, 0x00);
	sensor_write(0x3319, 0x6C);
	sensor_write(0x331A, 0x64);
	sensor_write(0x331B, 0x4B);
	sensor_write(0x331C, 0x00);
	sensor_write(0x331D, 0x6C);
	return sensor_write(0x3302, 0x01);
}

static int 
sensor_record(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x3012, 0x10);
	sensor_write(0x302a, 0x02);
	sensor_write(0x302b, 0xE6);
	sensor_write(0x306f, 0x14);
	sensor_write(0x3362, 0x90);
	
	sensor_write(0x3070, 0x5d);
	sensor_write(0x3072, 0x5d);
	sensor_write(0x301c, 0x07);
	sensor_write(0x301d, 0x07);
	
	sensor_write(0x3020, 0x01);
	sensor_write(0x3021, 0x18);
	sensor_write(0x3022, 0x00);
	sensor_write(0x3023, 0x06);
	sensor_write(0x3024, 0x06);
	sensor_write(0x3025, 0x58);
	sensor_write(0x3026, 0x02);
	sensor_write(0x3027, 0x5E);
	sensor_write(0x3088, 0x03);
	sensor_write(0x3089, 0x20);
	sensor_write(0x308A, 0x02);
	sensor_write(0x308B, 0x58);
	sensor_write(0x3316, 0x64);
	sensor_write(0x3317, 0x25);
	sensor_write(0x3318, 0x80);
	sensor_write(0x3319, 0x08);
	sensor_write(0x331A, 0x64);
	sensor_write(0x331B, 0x4B);
	sensor_write(0x331C, 0x00);
	sensor_write(0x331D, 0x38);
	return sensor_write(0x3302, 0x11);
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
			nRet = sensor_write(0x3306, 0x00);
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		nRet = sensor_read(0x3014, &data);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			data |= 0x80;
			nRet = sensor_write(0x3014, data);
			nRet = sensor_write(0x3070, 0x5d);
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			data &= 0x7f;
			nRet = sensor_write(0x3014, data);
			nRet = sensor_write(0x3072, 0x4d);
		}else{
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_write(0x3306, 0x02);
			nRet = sensor_write(0x3337, 0x5e);
			nRet = sensor_write(0x3338, 0x40);
			nRet = sensor_write(0x3339, 0x46);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_write(0x3306, 0x02);
			nRet = sensor_write(0x3337, 0x68);
			nRet = sensor_write(0x3338, 0x40);
			nRet = sensor_write(0x3339, 0x4e);
		}else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_write(0x3306, 0x02);
			nRet = sensor_write(0x3337, 0x68);
			nRet = sensor_write(0x3338, 0x40);
			nRet = sensor_write(0x3339, 0x4e);
		}else if(ctrl->value == 3) {	// INCANDESCENCE
			nRet = sensor_write(0x3306, 0x02);
			nRet = sensor_write(0x3337, 0x44);
			nRet = sensor_write(0x3338, 0x40);
			nRet = sensor_write(0x3339, 0x70);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value) {	// NIGH MODE ON
			nRet = sensor_write(0x3014, 0x8c);
			nRet = sensor_write(0x3015, 0x52);
		}else{	// NIGH MODE OFF
			nRet = sensor_write(0x3014, 0x84);
			nRet = sensor_write(0x3015, 0x02);
			nRet = sensor_write(0x302d, 0x00);
			nRet = sensor_write(0x302e, 0x00);
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
	if(sensor_i2c_open(OV2655_ID, 100) < 0) {
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: ov2655 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov2655");
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
MODULE_DESCRIPTION("Generalplus ov2655 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

