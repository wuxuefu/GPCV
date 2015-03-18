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
 *                         H E A D E R   F I L E S						  *					*
 **************************************************************************/
#include <linux/module.h>
#include <linux/fs.h> /* everything... */
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <linux/delay.h>
#include <mach/gp_gpio.h>
#include <mach/gp_i2c_bus.h>
#include <mach/gp_mipi.h>
#include <mach/sensor_mgr.h>

#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
/* max resolution: 2048*1536 */
#define	OV3640_ID					0x78
#define OV3640_WIDTH				2048
#define OV3640_HEIGHT				1536

#define OV3640_VYUY 				0x00
#define OV3640_UYVY					0x02
#define OV3640_BGGR					0x18
#define OV3640_GBRG					0x19
#define OV3640_GRBG					0x1A
#define OV3640_RGGB					0x1B

#define GPIO_SCL_CH					2
#define GPIO_SCL_PIN				0
#define GPIO_SCL_FUNC				2
#define GPIO_SCL_GID				4

#define GPIO_SDA_CH					2
#define GPIO_SDA_PIN				1
#define GPIO_SDA_FUNC				2
#define GPIO_SDA_GID				4

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
#define MK_GPIO_INDEX(ch, func, gid, pin) ((ch<<24) | (func<<16) | (gid<<8) | pin)

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct regval_list_s 
{
	unsigned short reg_num;
	unsigned char value;
}regval16_t;

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
#if (I2C_MODE == GPIO_I2C)
static int g_sda_handle;
static int g_scl_handle;
#elif (I2C_MODE == HW_I2C)
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
		.pixelformat = V4L2_PIX_FMT_YVYU,
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
		.desc		= "capture=2048*1536",
		.pixelformat = V4L2_PIX_FMT_YVYU,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 20000000,
		.hpixel = 2048,
		.hoffset = 0,
		.vline = 1536,
		.voffset = 0,
	},
	/* record mode */
	{
		.desc		= "record=640*480",
		.pixelformat = V4L2_PIX_FMT_YVYU,
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
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
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
		.desc		= "capture=2048*1536",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 2048,
		.hoffset = 0,
		.vline = 1536,
		.voffset = 0,
	},
};

#define C_SENSOR_FMT_MAX	sizeof(g_fmt_table)/sizeof(sensor_fmt_t)

static mipi_config_t mipi_setting = 
{
	.mipi_sep_clk_en = ENABLE,
	.mipi_sep_clk = 200000000,
	.mipi_sep_clk_src = MIPI_D_PHY_CLK,
	.byte_clk_edge = D_PHY_SAMPLE_POS,
	.low_power_en = DISABLE,
	.lane_num = MIPI_1_LANE,
	.ecc_check_en = ENABLE,
	.ecc_order = MIPI_ECC_ORDER3,
#ifdef CONFIG_FPGA_TEST	
	.data_mask_time = 270, //ns
#else
	.data_mask_time = 100, //ns
#endif
	.check_hs_seq = MIPI_CHECK_HS_SEQ,
};

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
	.mipi_config = &mipi_setting,
};
 
regval16_t ov3640_init_table[] =
{
	{ 0x3012, 0x80 },
	{ 0x304d, 0x41 }, 
	{ 0x3087, 0x16 }, 
	{ 0x30aa, 0x45 }, 
	{ 0x30b0, 0xff }, 
	{ 0x30b1, 0xff }, 
	{ 0x30b2, 0x10 }, 
	{ 0x30d7, 0x10 }, 
	
	{ 0x309e, 0x00 }, 
	{ 0x3602, 0x26 }, /* SOL/EOL on */	
	{ 0x3603, 0x4D }, /* ecc */
	{ 0x364c, 0x04 }, /* ecc */
	{ 0x360c, 0x12 }, /* irtual channel 0 */
	{ 0x361e, 0x00 }, 
	{ 0x361f, 0x11 }, /* pclk_period, terry */
	{ 0x3633, 0x32 }, /* increase hs_prepare */
	{ 0x3629, 0x3c }, /* increase clk_prepare */

#ifndef CONFIG_FPGA_TEST
	{ 0x300e, 0x38 },
	{ 0x300f, 0xa2 }, /*a1:1.5 , a2:2 */
#else
	#if 0
	{ 0x300e, 0x39 }, 
	{ 0x300f, 0xa1 }, 
	#elif 0
	{ 0x300e, 0x37 }, //15fps
	{ 0x300f, 0xa3 },	 
	#else
	{ 0x300e, 0x3b }, //bypass PLL
	{ 0x300f, 0xa8 }, 
	#endif
#endif
#if 1
	{ 0x3010, 0x80 }, /* high mipi spd, 81 */
#else
	{ 0x3010, 0xa0 }, /* 2 lane */
#endif
	{ 0x3011, 0x00 }, 
	{ 0x304c, 0x81 }, 
	
	{ 0x3018, 0x38 }, /* aec */
	{ 0x3019, 0x30 }, 
	{ 0x301a, 0x61 }, 
	{ 0x307d, 0x00 }, 
	{ 0x3087, 0x02 }, 
	{ 0x3082, 0x20 },
	
	{ 0x303c, 0x08 }, /* aec weight */
	{ 0x303d, 0x18 }, 
	{ 0x303e, 0x06 }, 
	{ 0x303f, 0x0c }, 
	{ 0x3030, 0x62 }, 	
	{ 0x3031, 0x26 }, 
	{ 0x3032, 0xe6 }, 
	{ 0x3033, 0x6e }, 
	{ 0x3034, 0xea }, 
	{ 0x3035, 0xae }, 
	{ 0x3036, 0xa6 }, 
	{ 0x3037, 0x6a },
	
	{ 0x3015, 0x12 },	
	{ 0x3014, 0x04 },	
	{ 0x3013, 0xf7 },
	
	{ 0x3104, 0x02 }, 
	{ 0x3105, 0xfd }, 
	{ 0x3106, 0x00 }, 
	{ 0x3107, 0xff }, 
	{ 0x3308, 0xa5 }, 
	{ 0x3316, 0xff }, 
	{ 0x3317, 0x00 }, 
	{ 0x3087, 0x02 }, 
	{ 0x3082, 0x20 }, 
	{ 0x3300, 0x13 }, 
	{ 0x3301, 0xde }, 
	{ 0x3302, 0xef },
	
	{ 0x30b8, 0x20 },  
	{ 0x30b9, 0x17 }, 
	{ 0x30ba, 0x04 },	
	{ 0x30bb, 0x08 },     

	{ 0x3100, 0x02 }, /* set raw format */
	{ 0x3304, 0x00 },
	{ 0x3400, 0x00 },
	{ 0x3404, OV3640_UYVY},
 	
	{ 0x3020, 0x01 }, /* Size, 2048x1536, QXGA */
	{ 0x3021, 0x1d },
	{ 0x3022, 0x00 },
	{ 0x3023, 0x0a },
	{ 0x3024, 0x08 },
	{ 0x3025, 0x18 },
	{ 0x3026, 0x06 },
	{ 0x3027, 0x0c },

	{ 0x335f, 0x68 },
	{ 0x3360, 0x18 },
	{ 0x3361, 0x0c },
	{ 0x3362, 0x68 },
	{ 0x3363, 0x08 },
	{ 0x3364, 0x04 },
	{ 0x3403, 0x42 },

	{ 0x3088, 0x08 },
	{ 0x3089, 0x00 },
	{ 0x308a, 0x06 },
	{ 0x308b, 0x00 },

	{ 0x3507, 0x06 },
	{ 0x350a, 0x4f },
	{ 0x3600, 0xc4 },
	
#ifdef CONFIG_FPGA_TEST
	{ 0x307B, 0x4a },   //color bar[1:0]
	{ 0x307D, 0xa0 },   //color bar[7]
	{ 0x306C, 0x00 },   //color bar[4]
	{ 0x3080, 0x11 },   //color bar[7] enable
#endif
	{ 0xffff, 0xff },
};

regval16_t ov3640_raw_fmt_table[] =
{
	{ 0x3100, 0x22 },
	{ 0x3304, 0x01 }, 
 	{ 0x3400, 0x03 },
 	{ 0x3600, 0xC4 },
 	{ 0x3404, OV3640_BGGR },
 	{ 0xffff, 0xff },
};

regval16_t ov3640_yuv_fmt_table[] =
{
	{ 0x3100, 0x02 },
	{ 0x3304, 0x00 },
	{ 0x3400, 0x00 },
	{ 0x3404, OV3640_UYVY},
	{ 0xffff, 0xff },
};

regval16_t ov3640_scale_vga_table[] =
{
	{ 0x3012, 0x10 }, //xga
		
	{ 0x3020, 0x01 },
	{ 0x3021, 0x1d },
	{ 0x3022, 0x00 },
	{ 0x3023, 0x06 },
	{ 0x3024, 0x08 },
	{ 0x3025, 0x18 },
	{ 0x3026, 0x03 },
	{ 0x3027, 0x04 },
	{ 0x302a, 0x03 },
	{ 0x302b, 0x10 },
	{ 0x3075, 0x24 },
	{ 0x300d, 0x01 },
	{ 0x30d7, 0x90 },
	{ 0x3069, 0x04 },
	
	{ 0x3302, 0xef },
	{ 0x335f, 0x34 },
	{ 0x3360, 0x0c },
	{ 0x3361, 0x04 },
	{ 0x3362, 0x12 },
	{ 0x3363, 0x88 },
	{ 0x3364, 0xe4 },
	{ 0x3403, 0x42 },

	{ 0x302c, 0x0e }, /* EXHTS */
	{ 0x302d, 0x00 }, /* EXVTS[15:8] */
	{ 0x302e, 0x10 }, /* EXVTS[7:0] */
	
	{ 0x3088, 0x02 },
	{ 0x3089, 0x80 },
	{ 0x308a, 0x01 },
#if 0
	{ 0x308b, 0xe0 },
#else
	{ 0x308b, 0xe2 }, /* if mipi clock will auto stop, must add 1 vertical line. */ 
#endif
	{ 0xffff, 0xff },
};

regval16_t ov3640_qxga_table[] =
{
	{ 0x3012, 0x00 }, //qxga mode

	{ 0x3020, 0x01 }, /* Size, 2048x1536, QXGA */
	{ 0x3021, 0x1d },
	{ 0x3022, 0x00 },
	{ 0x3023, 0x0a },
	{ 0x3024, 0x08 },
	{ 0x3025, 0x18 },
	{ 0x3026, 0x06 },
	{ 0x3027, 0x0c },

	{ 0x335f, 0x68 },
	{ 0x3360, 0x18 },
	{ 0x3361, 0x0c },
	{ 0x3362, 0x68 },
	{ 0x3363, 0x08 },
	{ 0x3364, 0x04 },
	{ 0x3403, 0x42 },

	{ 0x302c, 0x00 }, /* EXHTS */
	{ 0x302d, 0x00 }, /* EXVTS[15:8] */
	{ 0x302e, 0x00 }, /* EXVTS[7:0] */

	{ 0x3088, 0x08 },
	{ 0x3089, 0x00 },
	{ 0x308a, 0x06 },
#if 0
	{ 0x308b, 0x00 },
#else
	{ 0x308b, 0x02 }, /* if mipi clock will auto stop, must add 1 vertical line. */	
#endif
	{ 0xffff, 0xff },
};

regval16_t ov3640_suspend_table[] = 
{
	{0x300e, 0xb2},
	{0x308d, 0x14},
	{0x3086, 0x0f},
	{0xffff, 0xff},
};
 
regval16_t ov3640_resume_table[] = 
{
	{0x3086, 0x08},
	{0x308d, 0x14},
	{0x300e, 0x32},
	{0xffff, 0xff},
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
#if (I2C_MODE == GPIO_I2C)
static void 
sccb_delay (
	int i
) 
{
	udelay(i*10);
}

static void 
sccb_start(
	void
)
{
	gp_gpio_set_value(g_scl_handle, 1);
	sccb_delay(2);
	gp_gpio_set_value(g_sda_handle, 1);
	sccb_delay(2);
	gp_gpio_set_value(g_sda_handle, 0);	
	sccb_delay(2);
}

static void 
sccb_stop(
	void
)
{
	sccb_delay(2);
	gp_gpio_set_value(g_sda_handle, 0);					
	sccb_delay(2);
	gp_gpio_set_value(g_scl_handle, 1);					
	sccb_delay(2);
	gp_gpio_set_value(g_sda_handle, 1);					
	sccb_delay(2);
}

static int 
sccb_w_phase(
	unsigned short value
)
{
	int i, nRet = 0;

	for(i=0;i<8;i++) {
		gp_gpio_set_value(g_scl_handle,0);		/* SCL0 */
		sccb_delay (2);
		if (value & 0x80) {
			gp_gpio_set_value(g_sda_handle, 1);	/* SDA1 */
		} else {
			gp_gpio_set_value(g_sda_handle, 0);	/* SDA0 */
		}
		gp_gpio_set_value(g_scl_handle, 1);		/* SCL1 */
		sccb_delay(2);
		value <<= 1;
	}
	
	/* The 9th bit transmission */
	gp_gpio_set_value(g_scl_handle, 0);				/* SCL0 */
	gp_gpio_set_input(g_sda_handle, GPIO_PULL_HIGH);/* SDA is Hi-Z mode */
	sccb_delay(2);
	gp_gpio_set_value(g_scl_handle,1);				/* SCL1 */
	sccb_delay(2);
	gp_gpio_set_value(g_scl_handle,0);				/* SCL0 */
	gp_gpio_get_value(g_sda_handle, &nRet);			/* check ack */
	gp_gpio_set_output(g_sda_handle, 1, 0);			/* SDA is output */
	return nRet;
}

static int 
sccb_r_phase(
	void
)
{
	int i;
	int data, temp;

	gp_gpio_set_input(g_sda_handle, GPIO_PULL_HIGH);/* SDA is Hi-Z mode */
	data = 0x00;
	for (i=0;i<8;i++) {
		gp_gpio_set_value(g_scl_handle,0);			/* SCL0 */
		sccb_delay(2);
		gp_gpio_set_value(g_scl_handle,1);			/* SCL1 */
		gp_gpio_get_value(g_sda_handle, &temp);
		data <<= 1;
		data |= temp;
		sccb_delay(2);
	}
	
	/* The 9th bit transmission */
	gp_gpio_set_value(g_scl_handle, 0);				/* SCL0 */
	gp_gpio_set_output(g_sda_handle, 1, 0);			/* SDA is output mode */
	gp_gpio_set_value(g_sda_handle, 1);				/* SDA0, the nighth bit is NA must be 1 */
	sccb_delay(2);
	gp_gpio_set_value(g_scl_handle,1);				/* SCL1 */
	sccb_delay(2);
	gp_gpio_set_value(g_scl_handle,0);				/* SCL0 */
	return data;		
}

static int
sccb_read (
	unsigned short id,			
	unsigned short addr,		
	unsigned char *value
) 
{
	int nRet = 0;
	
	/* Data re-verification */
	id &= 0xFF;
	addr &= 0xFFFF;

	/* Serial bus output mode initialization */
	gp_gpio_set_output(g_scl_handle, 1, 0);
	gp_gpio_set_output(g_sda_handle, 1, 0);	

	/* 2-Phase write transmission cycle is starting now ...*/
	gp_gpio_set_value(g_scl_handle, 1);		/* SCL1	*/	
	gp_gpio_set_value(g_sda_handle, 0);		/* SDA0 */
	
	sccb_start();							/* Transmission start */
	nRet = sccb_w_phase (id);				/* Phase 1 */
	if(nRet < 0) {
		goto Return;
	}

	nRet = sccb_w_phase(addr >> 8);			/* Phase 2 */
	if(nRet < 0) {
		goto Return;
	}
	
	nRet = sccb_w_phase(addr & 0xFF);						
	if(nRet < 0) {
		goto Return;
	}
	sccb_stop();							/* Transmission stop */

	/* 2-Phase read transmission cycle is starting now ... */
	sccb_start();							/* Transmission start */
	nRet = sccb_w_phase (id | 0x01);		/* Phase 1 (read) */
	if(nRet < 0) {
		goto Return;
	}
	*value = sccb_r_phase();				/* Phase 2 */

Return:
	sccb_stop();							/* Transmission stop */
	return nRet;
}

static int 
sccb_write (
	unsigned short id,
	unsigned short addr,
	unsigned char data
) 
{
	int nRet = 0;
	
	/* Data re-verification */
	id &= 0xFF;
	addr &= 0xFFFF;
	data &= 0xFF;
	
	/* Serial bus output mode initialization */
	gp_gpio_set_output(g_scl_handle, 1, 0);
	gp_gpio_set_output(g_sda_handle, 1, 0);

	/* 3-Phase write transmission cycle is starting now ... */
	gp_gpio_set_value(g_scl_handle, 1);		/* SCL1 */		
	gp_gpio_set_value(g_sda_handle, 0);		/* SDA0 */
	sccb_start();							/* Transmission start */

	nRet = sccb_w_phase(id);				/* Phase 1 */
	if(nRet < 0) {
		goto Return;
	}
	
	nRet = sccb_w_phase((addr >> 8)& 0xFF);	/* Phase 2 */
	if(nRet < 0) {
		goto Return;
	}
	
	nRet = sccb_w_phase(addr & 0xFF);
	if(nRet < 0) {
		goto Return;
	}

	nRet = sccb_w_phase(data);				/* Phase 3 */
	nRet = 0;
	
Return:
	sccb_stop();							/* Transmission stop */
	return nRet;
}
#endif

static int
sensor_i2c_open(
	unsigned int slave_id,
	unsigned int scl_speed
)
{
#if (I2C_MODE == GPIO_I2C)
	unsigned int pin_index;

	pin_index = MK_GPIO_INDEX(GPIO_SCL_CH, GPIO_SCL_FUNC, GPIO_SCL_GID, GPIO_SCL_PIN);
	g_scl_handle = gp_gpio_request(pin_index, "GPIO_SCL"); 
	pin_index = MK_GPIO_INDEX(GPIO_SDA_CH, GPIO_SDA_FUNC, GPIO_SDA_GID, GPIO_SDA_PIN);
	g_sda_handle = gp_gpio_request(pin_index, "GPIO_SDA");
	if((g_scl_handle == 0) || (g_scl_handle == -EINVAL) || (g_scl_handle == -ENOMEM)||
		(g_sda_handle == 0) || (g_sda_handle == -EINVAL) || (g_sda_handle == -ENOMEM))
	{
		printk(KERN_WARNING "GpioReqFail 0x%x, 0x%x\n", g_scl_handle, g_sda_handle);
		gp_gpio_release(g_scl_handle);
		gp_gpio_release(g_sda_handle);	
		return -1;
	}

	gp_gpio_set_output(g_scl_handle, 1, 0);
	gp_gpio_set_output(g_sda_handle, 1, 0);
	printk(KERN_WARNING "GpioReq %d, %d\n", g_scl_handle, g_sda_handle);
	return 0;

#elif (I2C_MODE == HW_I2C)
	g_i2c_handle = gp_i2c_bus_request(slave_id, scl_speed);
	if((g_i2c_handle == 0) ||(g_i2c_handle == -ENOMEM)) {
		printk(KERN_WARNING "i2cReqFail %d\n", g_i2c_handle);
		return -1;
	}

	return 0;

#elif (I2C_MODE == HW_TI2C)
	g_ti2c_handle.pDeviceString = "OV3640_MIPI";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV3640_MIPI ti2c request failed\n");
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
#if (I2C_MODE == GPIO_I2C)
	gp_gpio_release(g_scl_handle);
	gp_gpio_release(g_sda_handle);	
	return 0;

#elif (I2C_MODE == HW_I2C)
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
#if (I2C_MODE == GPIO_I2C)
	return sccb_read(OV3640_ID, reg, value);

#elif (I2C_MODE == HW_I2C)
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
sensor_i2c_write(	
	unsigned short reg,
	unsigned char value
)
{
#if (I2C_MODE == GPIO_I2C)
	return sccb_write(OV3640_ID, reg, value);

#elif (I2C_MODE == HW_I2C)
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
sensor_write_table(
	regval16_t *vals
)
{
	int i, nRet;
	
	while (vals->reg_num != 0xffff || vals->value != 0xff) {
		for(i = 0; i< 10; i++) {
		#ifdef CONFIG_FPGA_TEST	
			printk("0x%x, 0x%x\n", vals->reg_num, vals->value);
		#endif
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
	return sensor_write_table(ov3640_init_table);
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
#ifdef CONFIG_FPGA_TEST
	printk("%s\n", __FUNCTION__);
	return 0;
#else
	unsigned char data;
	int nRet = 0;

	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		if(ctrl->value) {	// Enable Auto AWB
			nRet = sensor_i2c_read(0x332b, (unsigned char *)&data);			
			nRet = sensor_i2c_write(0x332b, (data &= ~0x8));
		}else{		// Disable Auto AWB
			nRet = sensor_i2c_read(0x332b, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x332b, data |= 0x08);
			nRet = sensor_i2c_write(0x33a7, 0x40);
			nRet = sensor_i2c_write(0x33a8, 0x40);
			nRet = sensor_i2c_write(0x33a9, 0x40);
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		nRet = sensor_i2c_read(0x3014, &data);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {	

		} else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_i2c_read(0x3014, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x3014, data |= 0x80);
		} else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_i2c_read(0x3014, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x3014, data &= ~0x80);
		} else {
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_i2c_read(0x332b, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x332b, data |= 0x08);
			nRet = sensor_i2c_write(0x33a7, 0x5a);
			nRet = sensor_i2c_write(0x33a8, 0x40);
			nRet = sensor_i2c_write(0x33a9, 0x48);
		} else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_i2c_read(0x332b, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x332b, data |= 0x08);
			nRet = sensor_i2c_write(0x33a7, 0x68);
			nRet = sensor_i2c_write(0x33a8, 0x40);
			nRet = sensor_i2c_write(0x33a9, 0x50);
		} else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_i2c_read(0x332b, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x332b, data |= 0x08);
			nRet = sensor_i2c_write(0x33a7, 0x52);
			nRet = sensor_i2c_write(0x33a8, 0x40);
			nRet = sensor_i2c_write(0x33a9, 0x5a);
		} else if(ctrl->value == 3) {	// INCANDESCENCE
			nRet = sensor_i2c_read(0x332b, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x332b, data |= 0x08);
			nRet = sensor_i2c_write(0x33a7, 0x40);
			nRet = sensor_i2c_write(0x33a8, 0x40);
			nRet = sensor_i2c_write(0x33a9, 0x64);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value) {	// NIGH MODE ON
			nRet = sensor_i2c_read(0x3014, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x3014, data |= 0x08);
		} else {	// NIGH MODE OFF
			nRet = sensor_i2c_read(0x3014, (unsigned char *)&data);
			nRet = sensor_i2c_write(0x3014, data &= ~0x08);
		}
		break;

	default:
		return -EINVAL;
	}
	
	return nRet; 
#endif
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
	printk("%s\n", __FUNCTION__);
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
		nRet = sensor_write_table(ov3640_yuv_fmt_table);
		nRet = sensor_write_table(ov3640_scale_vga_table);
	} else if (fmt->fmt.pix.priv == 1) {
		nRet = sensor_write_table(ov3640_yuv_fmt_table);
		nRet = sensor_write_table(ov3640_qxga_table);
	} else if (fmt->fmt.pix.priv == 2) {
		nRet = sensor_write_table(ov3640_yuv_fmt_table);
		nRet = sensor_write_table(ov3640_scale_vga_table);
	} else if(fmt->fmt.pix.priv == 3) {
		nRet = sensor_write_table(ov3640_raw_fmt_table);
		nRet = sensor_write_table(ov3640_scale_vga_table);
	}else if(fmt->fmt.pix.priv == 4) {
		nRet = sensor_write_table(ov3640_raw_fmt_table);
		nRet = sensor_write_table(ov3640_qxga_table);	
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
	printk("%s\n", __FUNCTION__);
	return 0;
}


static int 
sensor_g_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
sensor_s_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
sensor_g_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *parms
)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
sensor_s_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *param
)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}


static int 
sensor_s_interface(
	struct v4l2_subdev *sd,
	struct v4l2_interface *interface
)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}	

static int 
sensor_suspend(
	struct v4l2_subdev *sd
)
{
	printk("%s\n", __FUNCTION__);
#ifdef CONFIG_FPGA_TEST	
	return 0;
#else
	return sensor_write_table(ov3640_suspend_table);
#endif
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
	printk("%s\n", __FUNCTION__);
#ifdef CONFIG_FPGA_TEST	
	return 0;
#else
	return sensor_write_table(ov3640_resume_table);
#endif
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
	if(sensor_i2c_open(OV3640_ID, 100) < 0) {
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: ov3640 mipi\n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov3640_mipi");
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



