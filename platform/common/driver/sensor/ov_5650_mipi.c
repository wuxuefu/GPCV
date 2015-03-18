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
#define OV5650_MIPI_CLK_NO_STOP_EN	1

#define OV5650_ID		0x6C

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
static sensor_dev_t	gOV5650Dev;
#if (I2C_MODE == HW_I2C)
static int g_i2c_handle;
#elif (I2C_MODE == HW_TI2C)
static ti2c_set_value_t g_ti2c_handle;
#endif

static char *param[] = {"0", "PORT0", "0", "NONE", "0", "NONE"};
static int nstrs = 6;
module_param_array(param, charp, &nstrs, S_IRUGO);

static sensor_fmt_t gOV5650FmtTable[] =
{
	{
		.desc		= "preview=640*480",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 640,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
	{
		.desc		= "capture=1280*720",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 720,
		.voffset = 0,
	},
	{
		.desc		= "record=1280*720",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 720,
		.voffset = 0,
	},
};

#define C_SENSOR_FMT_MAX	sizeof(gOV5650FmtTable)/sizeof(sensor_fmt_t)

static mipi_config_t ov5650_mipi_setting = 
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
#if OV5650_MIPI_CLK_NO_STOP_EN == 1 
	.check_hs_seq = MIPI_CHECK_LP_00,	//for mipi clock no stop 
#else 
	.check_hs_seq = MIPI_CHECK_HS_SEQ,	//for mipi clock auto stop 
#endif
};

static sensor_config_t ov5650_config_table =
{
	.sensor_timing_mode = MODE_CCIR_601,
	.sensor_data_mode = MODE_DATA_YUV,
	.sensor_interlace_mode = MODE_NONE_INTERLACE,
	.sensor_pclk_mode = MODE_POSITIVE_EDGE,
	.sensor_hsync_mode = MODE_ACTIVE_HIGH,
	.sensor_vsync_mode = MODE_ACTIVE_LOW,
	.sensor_fmt_num = C_SENSOR_FMT_MAX,
	.fmt = gOV5650FmtTable,
	.mipi_config = &ov5650_mipi_setting,
};

/**************************************************************************
 *             Sensor Setting Table								          *
 **************************************************************************/
//@@MIPI ISP Raw VGA 30fps MIPI One-lane 10Bit
regval16_t ov5650_Mipi_Raw10_VGA[] =
{
	{ 0x3008, 0x82 },    
	{ 0x3008, 0x42 },    
	{ 0x3103, 0x93 },    
	{ 0x3b07, 0x0c },    
	{ 0x3017, 0xff },    
	{ 0x3018, 0xfc },    
	{ 0x3706, 0x41 },    
	{ 0x3613, 0xc4 },    
	{ 0x370d, 0x42 },    
	{ 0x3703, 0x9a },    
	{ 0x3630, 0x22 },    
	{ 0x3605, 0x04 },    
	{ 0x3606, 0x3f },    
	{ 0x3712, 0x13 },    
	{ 0x370e, 0x00 },    
	{ 0x370b, 0x40 },    
	{ 0x3600, 0x54 },    
	{ 0x3601, 0x05 },    
	{ 0x3713, 0x22 },    
	{ 0x3714, 0x27 },    
	{ 0x3631, 0x22 },    
	{ 0x3612, 0x1a },    
	{ 0x3604, 0x40 },    
	{ 0x3705, 0xdc },  
#if 0	
	{ 0x3709, 0x40 },    
	{ 0x370a, 0x41 }, //;81
#else
	{ 0x370a, 0x81 },
#endif
	{ 0x370c, 0xc8 },    
	{ 0x3710, 0x28 },    
	{ 0x3702, 0x3a },    
	{ 0x3704, 0x18 },    
	{ 0x3a18, 0x00 },    
	{ 0x3a19, 0xf8 },    
	{ 0x3a00, 0x38 },    
	{ 0x3800, 0x02 },    
	{ 0x3801, 0x54 },    
	{ 0x3803, 0x0c },    
	{ 0x380c, 0x0c },    
	{ 0x380d, 0xb4 },    
	{ 0x380e, 0x07 },    
	{ 0x380f, 0xb0 },    
	{ 0x3830, 0x50 },    
	{ 0x3a08, 0x12 },    
	{ 0x3a09, 0x70 },    
	{ 0x3a0a, 0x0f },    
	{ 0x3a0b, 0x60 },    
	{ 0x3a0d, 0x06 },    
	{ 0x3a0e, 0x06 },    
	{ 0x3a13, 0x54 },    
	{ 0x3815, 0x82 },    
	{ 0x5059, 0x80 },    
	{ 0x3615, 0x52 },    
	{ 0x505a, 0x0a },    
	{ 0x505b, 0x2e },    
	{ 0x3703, 0x9a },    
	{ 0x3010, 0x20 },    
	{ 0x3011, 0x10 },    
	{ 0x3713, 0x92 },    
	{ 0x3714, 0x17 },    
	{ 0x3804, 0x05 },    
	{ 0x3805, 0x00 },    
	{ 0x3806, 0x01 },    
	{ 0x3807, 0xe0 },    
	{ 0x3808, 0x02 },    
	{ 0x3809, 0x80 },    
	{ 0x380a, 0x01 },    
	{ 0x380b, 0xe0 },    
	{ 0x380c, 0x08 },    
	{ 0x380d, 0x78 },    
	{ 0x3a08, 0x12 },    
	{ 0x3a09, 0x70 },    
	{ 0x3a0a, 0x0f },    
	{ 0x3a0b, 0x60 },    
	{ 0x3a0d, 0x01 },    
	{ 0x3a0e, 0x01 },    
	{ 0x380e, 0x01 },    
	{ 0x380f, 0xec },    
	{ 0x3815, 0x81 },    
	{ 0x3824, 0x23 },    
	{ 0x3825, 0x20 },    
	{ 0x3803, 0x08 },    
	{ 0x3826, 0x00 },    
	{ 0x3827, 0x08 },    
	{ 0x3a1a, 0x06 },    
	{ 0x3503, 0x00 },    
	{ 0x3623, 0x01 },    
	{ 0x3633, 0x24 },    
	{ 0x3c01, 0x34 },    
	{ 0x3c04, 0x28 },    
	{ 0x3c05, 0x98 },    
	{ 0x3c07, 0x07 },    
	{ 0x3c09, 0xc2 },    
	{ 0x4000, 0x05 },    
	{ 0x401d, 0x28 },    
	{ 0x4001, 0x02 },    
	{ 0x401c, 0x42 },    
	{ 0x5046, 0x09 },    
	{ 0x3810, 0x40 },    
	{ 0x3836, 0x41 },    
	{ 0x505f, 0x04 },    
	{ 0x5000, 0xfe },    
	{ 0x5001, 0x01 },    
	{ 0x5002, 0x02 },    
	{ 0x503d, 0x00 },    
	{ 0x5901, 0x04 },    
	{ 0x585a, 0x01 },    
	{ 0x585b, 0x2c },    
	{ 0x585c, 0x01 },    
	{ 0x585d, 0x93 },    
	{ 0x585e, 0x01 },    
	{ 0x585f, 0x90 },    
	{ 0x5860, 0x01 },    
	{ 0x5861, 0x0d }, //agc
	{ 0x5180, 0xc0 },    
	{ 0x5184, 0x00 },    
	{ 0x470a, 0x00 },    
	{ 0x470b, 0x00 },    
	{ 0x470c, 0x00 },    
	{ 0x300f, 0x8e },    
	{ 0x3603, 0xa7 },    
	{ 0x3632, 0x55 },    
	{ 0x3620, 0x56 },    
	{ 0x3621, 0xaf },    
	{ 0x3818, 0xc2 },    
	{ 0x3631, 0x36 },    
	{ 0x3632, 0x5f },    
	{ 0x3711, 0x24 },    
	{ 0x401f, 0x03 },    
	{ 0x3008, 0x02 },    
	     
	{ 0x3011, 0x14 },    
	{ 0x3007, 0x3B },    
	{ 0x3010, 0x21 },    
	{ 0x4801, 0x0f },    
	{ 0x3003, 0x03 },    
	{ 0x300e, 0x0c },    
	{ 0x4803, 0x50 },    
	{ 0x4800, 0x04 },    
	{ 0x300f, 0x8b },    
	{ 0x3815, 0x82 },    
	{ 0x3003, 0x01 }, 
	
	{ 0x0000, 0x00 }, //end
};

//@@MIPI ISP Raw 720P 30fps MIPI One-lane 10Bit
regval16_t ov5650_Mipi_Raw10_720P[] =
{
	{ 0x3008, 0x82 },	{ 0x3008, 0x42 },	{ 0x3103, 0x93 },	{ 0x3b07, 0x0c },	{ 0x3017, 0xff },	{ 0x3018, 0xfc },	{ 0x3706, 0x41 },	{ 0x3613, 0xc4 },	{ 0x370d, 0x42 },	{ 0x3703, 0x9a },	{ 0x3630, 0x22 },	{ 0x3605, 0x04 },	{ 0x3606, 0x3f },	{ 0x3712, 0x13 },	{ 0x370e, 0x00 },	{ 0x370b, 0x40 },	{ 0x3600, 0x54 },	{ 0x3601, 0x05 },	{ 0x3713, 0x22 },	{ 0x3714, 0x27 },	{ 0x3631, 0x22 },	{ 0x3612, 0x1a },	{ 0x3604, 0x40 },	{ 0x3705, 0xdb },	{ 0x3709, 0x40 },	{ 0x370a, 0x41 }, //81	{ 0x370c, 0x00 },	{ 0x3710, 0x28 },	{ 0x3702, 0x3a },	{ 0x3704, 0x18 },	{ 0x3a18, 0x00 },	{ 0x3a19, 0xf8 },	{ 0x3a00, 0x38 },	{ 0x3800, 0x02 },	{ 0x3801, 0x54 },	{ 0x3803, 0x0c },	{ 0x380c, 0x0c },	{ 0x380d, 0xb4 },	{ 0x380e, 0x07 },	{ 0x380f, 0xb0 },	{ 0x3830, 0x50 },	{ 0x3a08, 0x12 },	{ 0x3a09, 0x70 },	{ 0x3a0a, 0x0f },	{ 0x3a0b, 0x60 },	{ 0x3a0d, 0x06 },	{ 0x3a0e, 0x06 },	{ 0x3a13, 0x54 },	{ 0x3815, 0x82 },	{ 0x5059, 0x80 },	{ 0x3615, 0x52 },	{ 0x505a, 0x0a },	{ 0x505b, 0x2e },	{ 0x3713, 0x92 },	{ 0x3714, 0x17 },	{ 0x3804, 0x05 },	{ 0x3805, 0x00 },	{ 0x3806, 0x02 },	{ 0x3807, 0xd0 },	{ 0x3808, 0x05 },	{ 0x3809, 0x00 },	{ 0x380a, 0x02 },	{ 0x380b, 0xd0 },	{ 0x380c, 0x08 },	{ 0x380d, 0x72 },	{ 0x380e, 0x02 },	{ 0x380f, 0xe4 },	{ 0x3815, 0x81 },	{ 0x381c, 0x10 },	{ 0x381d, 0x82 },	{ 0x381e, 0x05 },	{ 0x381f, 0xc0 },	{ 0x3821, 0x20 },	{ 0x3824, 0x23 },	{ 0x3825, 0x2c },	{ 0x3826, 0x00 },	{ 0x3827, 0x0c },	{ 0x3a08, 0x1b },	{ 0x3a09, 0xc0 },	{ 0x3a0a, 0x17 },	{ 0x3a0b, 0x20 },	{ 0x3a0d, 0x01 },	{ 0x3a0e, 0x01 },	{ 0x3a1a, 0x06 },	{ 0x3503, 0x00 },	{ 0x3623, 0x01 },	{ 0x3633, 0x24 },	{ 0x3c01, 0x34 },	{ 0x3c04, 0x28 },	{ 0x3c05, 0x98 },	{ 0x3c07, 0x07 },	{ 0x3c09, 0xc2 },	{ 0x4000, 0x05 },	{ 0x401d, 0x28 },	{ 0x4001, 0x02 },	{ 0x401c, 0x42 },	{ 0x5046, 0x09 },	{ 0x3810, 0x40 },	{ 0x3836, 0x41 },	{ 0x505f, 0x04 },	{ 0x5000, 0xfe },	{ 0x5001, 0x01 },	{ 0x5002, 0x00 },	{ 0x503d, 0x00 },	{ 0x5901, 0x00 },	{ 0x585a, 0x01 },	{ 0x585b, 0x2c },	{ 0x585c, 0x01 },	{ 0x585d, 0x93 },	{ 0x585e, 0x01 },	{ 0x585f, 0x90 },	{ 0x5860, 0x01 },	{ 0x5861, 0x0d },	{ 0x5180, 0xc0 },	{ 0x5184, 0x00 },	{ 0x470a, 0x00 },	{ 0x470b, 0x00 },	{ 0x470c, 0x00 },	{ 0x300f, 0x8e },	{ 0x3603, 0xa7 },	{ 0x3632, 0x55 },	{ 0x3620, 0x56 },	{ 0x3621, 0xaf },	{ 0x3818, 0xc1 },	{ 0x3631, 0x36 },	{ 0x3632, 0x5f },	{ 0x3711, 0x24 },	{ 0x401f, 0x03 },	{ 0x3008, 0x02 },	
	{ 0x3011, 0x14 },	{ 0x3007, 0x3B },	{ 0x4801, 0x0f },	{ 0x3003, 0x03 },	{ 0x300e, 0x0c },	{ 0x4803, 0x50 },	{ 0x4800, 0x04 },	{ 0x300f, 0x8b }, //8f	{ 0x3815, 0x82 },	{ 0x3003, 0x01 },
#if 0	
	{ 0x3503, 0x03 }, //manual AE AGC 	{ 0x3501, 0x11 },	{ 0x3502, 0x90 },	{ 0x350b, 0x7f },	{ 0x5001, 0x01 }, //manual AWB 	{ 0x5046, 0x09 },  	{ 0x3406, 0x01 },  	{ 0x3400, 0x04 },  	{ 0x3401, 0x00 },  	{ 0x3402, 0x04 },  	{ 0x3403, 0x00 },  	{ 0x3404, 0x04 },  	{ 0x3405, 0x00 },  	{ 0x5000, 0x06 }, //lenc off,wbc on  
#endif
	{ 0x0000, 0x00 }, //end
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
	g_ti2c_handle.pDeviceString = "OV5650_MIPI";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV5650_MIPI ti2c request failed\n");
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
ov5650_read(
	unsigned short reg, 
	unsigned char *value
)
{
#if (I2C_MODE == HW_I2C)
	char addr[2], data[0];
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
ov5650_write(
	unsigned short reg, 
	unsigned char value
)
{
#if (I2C_MODE == HW_I2C)
	char data[3];
	
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
OV5650WrTable(
	regval16_t *pTable
)
{
	int ret = 0;

	while(1) {
		if(pTable->reg_num == 0 && pTable->value == 0) {
			break;
		}
		
		ret = ov5650_write(pTable->reg_num, pTable->value);
		if(ret < 0) {
			return -1;
		}

		printk("[0x%x] = 0x%x\n", pTable->reg_num, pTable->value);
		pTable ++;
	}
	
	return ret;
}

static int 
ov5650_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
ov5650_preview(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return OV5650WrTable(ov5650_Mipi_Raw10_VGA);
}

static int 
ov5650_capture(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return OV5650WrTable(ov5650_Mipi_Raw10_720P);
}

static int 
ov5650_record(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return OV5650WrTable(ov5650_Mipi_Raw10_720P);
}

static int 
ov5650_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	return 0;
}

static int 
ov5650_queryctrl(
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
ov5650_g_ctrl(
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
ov5650_s_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{
	int nRet = 0;

	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		} else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			
		} else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			
		} else {
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			
		} else if(ctrl->value == 1) {	// CLOUDY
			
		} else if(ctrl->value == 2) {	// FLUORESCENCE
			
		} else if(ctrl->value == 3) {	// INCANDESCENCE

		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value) {	// NIGH MODE ON
			
		} else {	// NIGH MODE OFF
			
		}
		break;

	default:
		return -EINVAL;
	}
	
	return nRet; 
}

static int 
ov5650_querystd(
	struct v4l2_subdev *sd,
	v4l2_std_id *std
)
{
	return 0;
}

static int 
ov5650_enum_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_fmtdesc *fmtdesc
)
{
	printk("%s\n", __FUNCTION__);
	if(fmtdesc->index >= C_SENSOR_FMT_MAX)
		return -EINVAL;

	fmtdesc->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	memcpy((void *)fmtdesc->description, (void *)gOV5650FmtTable[fmtdesc->index].desc, 32);
	fmtdesc->pixelformat = gOV5650FmtTable[fmtdesc->index].pixelformat;
	return 0;
}

static int 
ov5650_g_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	printk("%s\n", __FUNCTION__);
	fmt->fmt.pix.width = gOV5650Dev.fmt->hpixel;
	fmt->fmt.pix.height = gOV5650Dev.fmt->vline;
	fmt->fmt.pix.pixelformat = gOV5650Dev.fmt->pixelformat;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = gOV5650Dev.fmt->hpixel * gOV5650Dev.fmt->bpp;
	fmt->fmt.pix.sizeimage = fmt->fmt.pix.bytesperline * gOV5650Dev.fmt->vline;

	return 0;
}

static int 
ov5650_try_fmt(
	struct v4l2_subdev *sd,
	struct v4l2_format *fmt
)
{
	return 0;
}

static int 
ov5650_s_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	int ret;

	printk("%s = %d\n", __FUNCTION__, fmt->fmt.pix.priv);
	switch(fmt->fmt.pix.priv)
	{
	case 0: 
		ret = ov5650_preview();
		break;

	case 1: 
		ret = ov5650_capture();
		break;

	case 2: 
		ret = ov5650_record();
		break;
		
	default:
		ret = -1;
	}

	gOV5650Dev.fmt = &gOV5650FmtTable[fmt->fmt.pix.priv];
	return ret;
}

static int 
ov5650_cropcap(
	struct v4l2_subdev *sd,
	struct v4l2_cropcap *cc
)
{
	return 0;
}

static int 
ov5650_g_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
ov5650_s_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
ov5650_g_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *parms
)
{
	return 0;
}

static int 
ov5650_s_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *param
)
{
	return 0;
}

static int 
ov5650_s_interface(
	struct v4l2_subdev *sd,
	struct v4l2_interface *interface
)
{
	return 0;
}

static int 
ov5650_suspend(
	struct v4l2_subdev *sd
)
{
	return 0;
}

static int 
ov5650_resume(
	struct v4l2_subdev *sd
)
{
	return 0;
}

static const struct v4l2_subdev_core_ops ov5650_core_ops = 
{
	.init = ov5650_init,
	.reset = ov5650_reset,
	.queryctrl = ov5650_queryctrl,
	.g_ctrl = ov5650_g_ctrl,
	.s_ctrl = ov5650_s_ctrl,
};

static const struct v4l2_subdev_video_ops ov5650_video_ops = 
{
	.querystd = ov5650_querystd,
	.enum_fmt = ov5650_enum_fmt,
	.g_fmt = ov5650_g_fmt,
	.try_fmt = ov5650_try_fmt,
	.s_fmt = ov5650_s_fmt,
	.cropcap = ov5650_cropcap,
	.g_crop = ov5650_g_crop,
	.s_crop = ov5650_s_crop,
	.g_parm = ov5650_g_parm,
	.s_parm = ov5650_s_parm,
};

static const struct v4l2_subdev_ext_ops ov5650_ext_ops = 
{
	.s_interface = ov5650_s_interface,
	.suspend = ov5650_suspend,
	.resume = ov5650_resume,
};

static const struct v4l2_subdev_ops ov5650_ops = 
{
	.core = &ov5650_core_ops,
	.video = &ov5650_video_ops,
	.ext = &ov5650_ext_ops
};

static int __init 
ov5650_module_init(
		void
)
{
	if(sensor_i2c_open(OV5650_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: ov5650 \n");
	gOV5650Dev.fmt = &gOV5650FmtTable[0];
	v4l2_subdev_init(&(gOV5650Dev.sd), &ov5650_ops);
	strcpy(gOV5650Dev.sd.name, "sensor_ov5650_mipi");
	register_sensor(&gOV5650Dev.sd, (int *)&param[0], &ov5650_config_table);
	return 0;
}

static void __exit
ov5650_module_exit(
		void
)
{
	sensor_i2c_close();
	unregister_sensor(&(gOV5650Dev.sd));
}

module_init(ov5650_module_init);
module_exit(ov5650_module_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus ov5650 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

