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
#define OV4689_MIPI_CLK_NO_STOP_EN		1
#define OV4689_ID	0x42
#define TI2C_RETRY	10

// I2C mode
#define GPIO_I2C		0x00 
#define HW_I2C		0x01
#define HW_TI2C		0x02
#if (defined CONFIG_ARCH_GPL32900)
#define I2C_MODE		HW_I2C
#elif (defined CONFIG_ARCH_GPL32900B)
#define I2C_MODE		HW_TI2C
#else
#define I2C_MODE		GPIO_I2C
#endif

#define OV4689_MAX_EXPOSURE_TIME			0x0438 // depend on 0x350C/0x350D
#define OV4689_MIN_EXPOSURE_TIME			0x0008
#define OV4689_MAX_ANALOG_GAIN			(16*256)
#define OV4689_MIN_ANALOG_GAIN			(1*256)
#define OV4689_MAX_DIGITAL_GAIN			(1*256)
#define OV4689_MIN_DIGITAL_GAIN 			(1*256)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define AEGAIN_MUL(x, y)				((((x) * (y)) + 512)>> 10)
#define ABS(x)          					((x) < (0) ? -(x) : (x))


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
 *                         G L O B A L    D A T A  For CDSP using                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static sensor_exposure_t seInfo; 
static sensor_dev_t	OV4689Dev;
#if (I2C_MODE == HW_I2C)
static int g_i2c_handle;
#elif (I2C_MODE == HW_TI2C)
static ti2c_set_value_t g_ti2c_handle;
#endif

static char *param[] = {"0", "MIPI", "0", "NONE", "0", "NONE"};
static int nstrs = 6;
module_param_array(param, charp, &nstrs, S_IRUGO);

static sensor_fmt_t OV4689FmtTable[] =
{
	{
		.desc		= "preview=1280*720,crop=1280*720",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 720,
		.voffset = 0,
		.cdsp_calibration = NULL,
	},
	{
		.desc		= "capture=1920*1084,crop=1920*1080",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 2304,
		.hoffset = 0,
		.vline = 1296,
		.voffset = 0,
		.cdsp_calibration = NULL,
	},
	{
		.desc		= "record=1280*720,crop=1280*720",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 720,
		.voffset = 0,
		.cdsp_calibration = NULL,
	},	
};

#define C_SENSOR_FMT_MAX	sizeof(OV4689FmtTable)/sizeof(sensor_fmt_t)

static mipi_config_t OV4689_mipi_setting = 
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
#if OV4689_MIPI_CLK_NO_STOP_EN == 1 
	.check_hs_seq = MIPI_CHECK_LP_00,	//for mipi clock no stop 
#else 
	.check_hs_seq = MIPI_CHECK_HS_SEQ,	//for mipi clock auto stop 
#endif
};

static sensor_config_t OV4689_config_table =
{
	.sensor_timing_mode = MODE_CCIR_601,
	.sensor_data_mode = MODE_DATA_YUV,
	.sensor_interlace_mode = MODE_NONE_INTERLACE,
	.sensor_pclk_mode = MODE_POSITIVE_EDGE,
	.sensor_hsync_mode = MODE_ACTIVE_HIGH,
	.sensor_vsync_mode = MODE_ACTIVE_LOW,
	.sensor_fmt_num = C_SENSOR_FMT_MAX,
	.fmt = OV4689FmtTable,
	.mipi_config = &OV4689_mipi_setting,
};

/**************************************************************************
 *             Sensor Setting Table								          *
 **************************************************************************/
//----------------------------------------------
// MCLK: 24Mhz
// resolution: 1280x720
// Mipi : 1lane 
// Mipi data rate: 360Mbps/Lane
// SystemCLK   :120Mhz
// FPS	       :30 ( = SystemCLK / (HTS*VTS) ) 
// HTS		   :4802(R380c:R380d)
// VTS		   :833(R380e:R380f)
// Tline 	   :40.02us
// Max exp line:829 (VTS-4)
//---------------------------------------------
const regval16_t OV4689_Mipi_Raw10_720P_30fps[] =
{
	{ 0x0103, 0x01 }, // 
	{ 0x3638, 0x00 }, // 
	{ 0x0300, 0x01 }, // ; 02 
	{ 0x0302, 0x1e }, // ; 32 
	{ 0x0303, 0x00 }, //
	{ 0x0304, 0x03 }, // 
	{ 0x030b, 0x00 }, // 
	{ 0x030d, 0x1e }, // 
	{ 0x030e, 0x04 }, // 
	{ 0x030f, 0x01 }, // 
	{ 0x0312, 0x01 }, // 
	{ 0x031e, 0x00 }, // 
	{ 0x3000, 0x20 }, // 
	{ 0x3002, 0x00 }, // 
	{ 0x3018, 0x12 }, // ; 12/32/72 1lane/2lane/4lane
	{ 0x3019, 0x0e }, // ; 0e/0c/00 1lane/2lane/4lane 
	{ 0x3020, 0x93 }, // 
	{ 0x3021, 0x03 }, // 
	{ 0x3022, 0x01 }, // 
	{ 0x3031, 0x0a }, // 
	{ 0x303f, 0x0c }, //
	{ 0x3305, 0xf1 }, // 
	{ 0x3307, 0x04 }, // 
	{ 0x3309, 0x29 }, // 
	{ 0x3500, 0x00 }, // 
	{ 0x3501, 0x33 }, // 
	{ 0x3502, 0x00 }, // 
	{ 0x3503, 0x04 }, // 
	{ 0x3504, 0x00 }, // 
	{ 0x3505, 0x00 }, // 
	{ 0x3506, 0x00 }, // 
	{ 0x3507, 0x00 }, // 
	{ 0x3508, 0x00 }, // 
	{ 0x3509, 0x80 }, // 
	{ 0x350a, 0x00 }, // 
	{ 0x350b, 0x00 }, // 
	{ 0x350c, 0x00 }, // 
	{ 0x350d, 0x00 }, // 
	{ 0x350e, 0x00 }, // 
	{ 0x350f, 0x80 }, // 
	{ 0x3510, 0x00 }, // 
	{ 0x3511, 0x00 }, // 
	{ 0x3512, 0x00 }, // 
	{ 0x3513, 0x00 }, // 
	{ 0x3514, 0x00 }, // 
	{ 0x3515, 0x80 }, // 
	{ 0x3516, 0x00 }, // 
	{ 0x3517, 0x00 }, // 
	{ 0x3518, 0x00 }, // 
	{ 0x3519, 0x00 }, // 
	{ 0x351a, 0x00 }, // 
	{ 0x351b, 0x80 }, // 
	{ 0x351c, 0x00 }, // 
	{ 0x351d, 0x00 }, // 
	{ 0x351e, 0x00 }, // 
	{ 0x351f, 0x00 }, // 
	{ 0x3520, 0x00 }, // 
	{ 0x3521, 0x80 }, // 
	{ 0x3522, 0x08 }, // 
	{ 0x3524, 0x08 }, // 
	{ 0x3526, 0x08 }, // 
	{ 0x3528, 0x08 }, // 
	{ 0x352a, 0x08 }, // 
	{ 0x3602, 0x00 }, // 
	{ 0x3603, 0x40 }, // 
	{ 0x3604, 0x02 }, // 
	{ 0x3605, 0x00 }, // 
	{ 0x3606, 0x00 }, // 
	{ 0x3607, 0x00 }, // 
	{ 0x3609, 0x12 }, // 
	{ 0x360a, 0x40 }, // 
	{ 0x360c, 0x08 }, // 
	{ 0x360f, 0xe5 }, // 
	{ 0x3608, 0x8f }, // 
	{ 0x3611, 0x00 }, // 
	{ 0x3613, 0xf7 }, // 
	{ 0x3616, 0x58 }, // 
	{ 0x3619, 0x99 }, // 
	{ 0x361b, 0x60 }, // 
	{ 0x361c, 0x7a }, // 
	{ 0x361e, 0x79 }, // 
	{ 0x361f, 0x02 }, // 
	{ 0x3632, 0x05 }, // 
	{ 0x3633, 0x10 }, // 
	{ 0x3634, 0x10 }, // 
	{ 0x3635, 0x10 }, // 
	{ 0x3636, 0x15 }, // 
	{ 0x3646, 0x86 }, // 
	{ 0x364a, 0x0b }, // 
	{ 0x3700, 0x17 }, // 
	{ 0x3701, 0x22 }, // 
	{ 0x3703, 0x10 }, // 
	{ 0x370a, 0x37 }, // 
	{ 0x3705, 0x00 }, // 
	{ 0x3706, 0x63 }, // 
	{ 0x3709, 0x3c }, // 
	{ 0x370b, 0x01 }, // 
	{ 0x370c, 0x30 }, // 
	{ 0x3710, 0x24 }, // 
	{ 0x3711, 0x0c }, // 
	{ 0x3716, 0x00 }, // 
	{ 0x3720, 0x28 }, // 
	{ 0x3729, 0x7b }, // 
	{ 0x372a, 0x84 }, // 
	{ 0x372b, 0xbd }, // 
	{ 0x372c, 0xbc }, // 
	{ 0x372e, 0x52 }, // 
	{ 0x373c, 0x0e }, // 
	{ 0x373e, 0x33 }, // 
	{ 0x3743, 0x10 }, // 
	{ 0x3744, 0x88 }, // 
	{ 0x3745, 0xc0 }, // 
	{ 0x374a, 0x43 }, // 
	{ 0x374c, 0x00 }, // 
	{ 0x374e, 0x23 }, // 
	{ 0x3751, 0x7b }, // 
	{ 0x3752, 0x84 }, // 
	{ 0x3753, 0xbd }, // 
	{ 0x3754, 0xbc }, // 
	{ 0x3756, 0x52 }, // 
	{ 0x375c, 0x00 }, // 
	{ 0x3760, 0x00 }, // 
	{ 0x3761, 0x00 }, // 
	{ 0x3762, 0x00 }, // 
	{ 0x3763, 0x00 }, // 
	{ 0x3764, 0x00 }, // 
	{ 0x3767, 0x04 }, // 
	{ 0x3768, 0x04 }, // 
	{ 0x3769, 0x08 }, // 
	{ 0x376a, 0x08 }, // 
	{ 0x376b, 0x40 }, // 
	{ 0x376c, 0x00 }, // 
	{ 0x376d, 0x00 }, // 
	{ 0x376e, 0x00 }, // 
	{ 0x3773, 0x00 }, // 
	{ 0x3774, 0x51 }, // 
	{ 0x3776, 0xbd }, // 
	{ 0x3777, 0xbd }, // 
	{ 0x3781, 0x18 }, // 
	{ 0x3783, 0x25 }, // 
	{ 0x3798, 0x1b }, // 
	{ 0x3800, 0x00 }, //
	{ 0x3801, 0x48 }, // 
	{ 0x3802, 0x00 }, // 
	{ 0x3803, 0x2C }, // 
	{ 0x3804, 0x0a }, // 
	{ 0x3805, 0x57 }, // 
	{ 0x3806, 0x05 }, // 
	{ 0x3807, 0xD3 }, // 
	{ 0x3808, 0x05 }, // 
	{ 0x3809, 0x00 }, // 
	{ 0x380a, 0x02 }, // 
	{ 0x380b, 0xD0 }, // 
	{ 0x380c, 0x12 }, //  ; 03 
	{ 0x380d, 0xc2 }, // ; 5C 
	{ 0x380e, 0x03 }, // 
	{ 0x380f, 0x41 }, //
	{ 0x3810, 0x00 }, // 
	{ 0x3811, 0x04 }, // 
	{ 0x3812, 0x00 }, // 
	{ 0x3813, 0x02 }, // 
	{ 0x3814, 0x03 }, // 
	{ 0x3815, 0x01 }, // 
	{ 0x3819, 0x01 }, // 
	{ 0x3820, 0x10 }, // 
	{ 0x3821, 0x07 }, // 
	{ 0x3829, 0x00 }, // 
	{ 0x382a, 0x03 }, // 
	{ 0x382b, 0x01 }, // 
	{ 0x382d, 0x7f }, // 
	{ 0x3830, 0x08 }, // 
	{ 0x3836, 0x02 }, // 
	{ 0x3837, 0x00 }, // 
	{ 0x3841, 0x02 }, // 
	{ 0x3846, 0x08 }, // 
	{ 0x3847, 0x07 }, // 
	{ 0x3d85, 0x36 }, // 
	{ 0x3d8c, 0x71 }, // 
	{ 0x3d8d, 0xcb }, // 
	{ 0x3f0a, 0x00 }, // 
	{ 0x4000, 0x71 }, // 
	{ 0x4001, 0x50 }, // 
	{ 0x4002, 0x04 }, // 
	{ 0x4003, 0x14 }, // 
	{ 0x400e, 0x00 }, // 
	{ 0x4011, 0x00 }, // 
	{ 0x401a, 0x00 }, // 
	{ 0x401b, 0x00 }, // 
	{ 0x401c, 0x00 }, // 
	{ 0x401d, 0x00 }, // 
	{ 0x401f, 0x00 }, // 
	{ 0x4020, 0x00 }, // 
	{ 0x4021, 0x10 }, // 
	{ 0x4022, 0x03 }, // 
	{ 0x4023, 0x93 }, // 
	{ 0x4024, 0x04 }, //
	{ 0x4025, 0xC0 }, // 
	{ 0x4026, 0x04 }, // 
	{ 0x4027, 0xD0 }, // 
	{ 0x4028, 0x00 }, // 
	{ 0x4029, 0x02 }, // 
	{ 0x402a, 0x06 }, // 
	{ 0x402b, 0x04 }, // 
	{ 0x402c, 0x02 }, // 
	{ 0x402d, 0x02 }, // 
	{ 0x402e, 0x0e }, // 
	{ 0x402f, 0x04 }, // 
	{ 0x4302, 0xff }, // 
	{ 0x4303, 0xff }, // 
	{ 0x4304, 0x00 }, // 
	{ 0x4305, 0x00 }, // 
	{ 0x4306, 0x00 }, // 
	{ 0x4308, 0x02 }, // 
	{ 0x4500, 0x6c }, // 
	{ 0x4501, 0xc4 }, // 
	{ 0x4502, 0x44 }, // 
	{ 0x4503, 0x01 }, // 
	{ 0x4601, 0x4F }, // 
	{ 0x4800, 0x04 }, // 
	{ 0x4813, 0x08 }, // 
	{ 0x481f, 0x40 }, // 
	{ 0x4829, 0x78 }, // 
	{ 0x4837, 0x2c }, // 
	{ 0x4b00, 0x2a }, // 
	{ 0x4b0d, 0x00 }, // 
	{ 0x4d00, 0x04 }, // 
	{ 0x4d01, 0x42 }, // 
	{ 0x4d02, 0xd1 }, // 
	{ 0x4d03, 0x93 }, // 
	{ 0x4d04, 0xf5 }, // 
	{ 0x4d05, 0xc1 }, // 
	{ 0x5000, 0xf3 }, // 
	{ 0x5001, 0x11 }, // 
	{ 0x5004, 0x00 }, // 
	{ 0x500a, 0x00 }, // 
	{ 0x500b, 0x00 }, // 
	{ 0x5032, 0x00 }, // 
	{ 0x5040, 0x00 }, // 
	{ 0x5050, 0x3c }, // 
	{ 0x5500, 0x00 }, // 
	{ 0x5501, 0x10 }, // 
	{ 0x5502, 0x01 }, // 
	{ 0x5503, 0x0f }, // 
	{ 0x8000, 0x00 }, // 
	{ 0x8001, 0x00 }, // 
	{ 0x8002, 0x00 }, // 
	{ 0x8003, 0x00 }, // 
	{ 0x8004, 0x00 }, // 
	{ 0x8005, 0x00 }, // 
	{ 0x8006, 0x00 }, // 
	{ 0x8007, 0x00 }, // 
	{ 0x8008, 0x00 }, // 
	{ 0x3638, 0x00 }, // 
	{ 0x0100, 0x01 }, // 
	{ 0, 0 },	        //End
}; 
 
//----------------------------------------------
// MCLK: 24Mhz
// resolution: 1280x720
// Mipi : 1lane 
// Mipi data rate: 720Mbps/Lane
// SystemCLK   :120Mhz
// FPS	       :60 ( = SystemCLK / (HTS*VTS) ) 
// HTS		   :2588(R380c:R380d)
// VTS		   :773(R380e:R380f)
// Tline 	   :21.57us
// Max exp line:769 (VTS-4)
//---------------------------------------------
const regval16_t OV4689_Mipi_Raw10_720P_60fps[] =
{
	{ 0x0103, 0x01 }, // 
	{ 0x3638, 0x00 }, // 
	{ 0x0300, 0x00 }, // ; 02 
	{ 0x0302, 0x1e }, // ; 32 
	{ 0x0303, 0x00 }, //
	{ 0x0304, 0x03 }, // 
	{ 0x030b, 0x00 }, // 
	{ 0x030d, 0x1e }, // 
	{ 0x030e, 0x04 }, // 
	{ 0x030f, 0x01 }, // 
	{ 0x0312, 0x01 }, // 
	{ 0x031e, 0x00 }, // 
	{ 0x3000, 0x20 }, // 
	{ 0x3002, 0x00 }, // 
	{ 0x3018, 0x12 }, // ; 12/32/72 1lane/2lane/4lane
	{ 0x3019, 0x0e }, // ; 0e/0c/00 1lane/2lane/4lane 
	{ 0x3020, 0x93 }, // 
	{ 0x3021, 0x03 }, // 
	{ 0x3022, 0x01 }, // 
	{ 0x3031, 0x0a }, // 
	{ 0x303f, 0x0c }, //
	{ 0x3305, 0xf1 }, // 
	{ 0x3307, 0x04 }, // 
	{ 0x3309, 0x29 }, // 
	{ 0x3500, 0x00 }, // 
	{ 0x3501, 0x30 }, // 
	{ 0x3502, 0x00 }, // 
	{ 0x3503, 0x04 }, // 
	{ 0x3504, 0x00 }, // 
	{ 0x3505, 0x00 }, // 
	{ 0x3506, 0x00 }, // 
	{ 0x3507, 0x00 }, // 
	{ 0x3508, 0x00 }, // 
	{ 0x3509, 0x80 }, // 
	{ 0x350a, 0x00 }, // 
	{ 0x350b, 0x00 }, // 
	{ 0x350c, 0x00 }, // 
	{ 0x350d, 0x00 }, // 
	{ 0x350e, 0x00 }, // 
	{ 0x350f, 0x80 }, // 
	{ 0x3510, 0x00 }, // 
	{ 0x3511, 0x00 }, // 
	{ 0x3512, 0x00 }, // 
	{ 0x3513, 0x00 }, // 
	{ 0x3514, 0x00 }, // 
	{ 0x3515, 0x80 }, // 
	{ 0x3516, 0x00 }, // 
	{ 0x3517, 0x00 }, // 
	{ 0x3518, 0x00 }, // 
	{ 0x3519, 0x00 }, // 
	{ 0x351a, 0x00 }, // 
	{ 0x351b, 0x80 }, // 
	{ 0x351c, 0x00 }, // 
	{ 0x351d, 0x00 }, // 
	{ 0x351e, 0x00 }, // 
	{ 0x351f, 0x00 }, // 
	{ 0x3520, 0x00 }, // 
	{ 0x3521, 0x80 }, // 
	{ 0x3522, 0x08 }, // 
	{ 0x3524, 0x08 }, // 
	{ 0x3526, 0x08 }, // 
	{ 0x3528, 0x08 }, // 
	{ 0x352a, 0x08 }, // 
	{ 0x3602, 0x00 }, // 
	{ 0x3603, 0x40 }, // 
	{ 0x3604, 0x02 }, // 
	{ 0x3605, 0x00 }, // 
	{ 0x3606, 0x00 }, // 
	{ 0x3607, 0x00 }, // 
	{ 0x3609, 0x12 }, // 
	{ 0x360a, 0x40 }, // 
	{ 0x360c, 0x08 }, // 
	{ 0x360f, 0xe5 }, // 
	{ 0x3608, 0x8f }, // 
	{ 0x3611, 0x00 }, // 
	{ 0x3613, 0xf7 }, // 
	{ 0x3616, 0x58 }, // 
	{ 0x3619, 0x99 }, // 
	{ 0x361b, 0x60 }, // 
	{ 0x361c, 0x7a }, // 
	{ 0x361e, 0x79 }, // 
	{ 0x361f, 0x02 }, // 
	{ 0x3632, 0x05 }, // 
	{ 0x3633, 0x10 }, // 
	{ 0x3634, 0x10 }, // 
	{ 0x3635, 0x10 }, // 
	{ 0x3636, 0x15 }, // 
	{ 0x3646, 0x86 }, // 
	{ 0x364a, 0x0b }, // 
	{ 0x3700, 0x17 }, // 
	{ 0x3701, 0x22 }, // 
	{ 0x3703, 0x10 }, // 
	{ 0x370a, 0x37 }, // 
	{ 0x3705, 0x00 }, // 
	{ 0x3706, 0x63 }, // 
	{ 0x3709, 0x3c }, // 
	{ 0x370b, 0x01 }, // 
	{ 0x370c, 0x30 }, // 
	{ 0x3710, 0x24 }, // 
	{ 0x3711, 0x0c }, // 
	{ 0x3716, 0x00 }, // 
	{ 0x3720, 0x28 }, // 
	{ 0x3729, 0x7b }, // 
	{ 0x372a, 0x84 }, // 
	{ 0x372b, 0xbd }, // 
	{ 0x372c, 0xbc }, // 
	{ 0x372e, 0x52 }, // 
	{ 0x373c, 0x0e }, // 
	{ 0x373e, 0x33 }, // 
	{ 0x3743, 0x10 }, // 
	{ 0x3744, 0x88 }, // 
	{ 0x3745, 0xc0 }, // 
	{ 0x374a, 0x43 }, // 
	{ 0x374c, 0x00 }, // 
	{ 0x374e, 0x23 }, // 
	{ 0x3751, 0x7b }, // 
	{ 0x3752, 0x84 }, // 
	{ 0x3753, 0xbd }, // 
	{ 0x3754, 0xbc }, // 
	{ 0x3756, 0x52 }, // 
	{ 0x375c, 0x00 }, // 
	{ 0x3760, 0x00 }, // 
	{ 0x3761, 0x00 }, // 
	{ 0x3762, 0x00 }, // 
	{ 0x3763, 0x00 }, // 
	{ 0x3764, 0x00 }, // 
	{ 0x3767, 0x04 }, // 
	{ 0x3768, 0x04 }, // 
	{ 0x3769, 0x08 }, // 
	{ 0x376a, 0x08 }, // 
	{ 0x376b, 0x40 }, // 
	{ 0x376c, 0x00 }, // 
	{ 0x376d, 0x00 }, // 
	{ 0x376e, 0x00 }, // 
	{ 0x3773, 0x00 }, // 
	{ 0x3774, 0x51 }, // 
	{ 0x3776, 0xbd }, // 
	{ 0x3777, 0xbd }, // 
	{ 0x3781, 0x18 }, // 
	{ 0x3783, 0x25 }, // 
	{ 0x3798, 0x1b }, // 
	{ 0x3800, 0x00 }, //
	{ 0x3801, 0x48 }, // 
	{ 0x3802, 0x00 }, // 
	{ 0x3803, 0x2C }, // 
	{ 0x3804, 0x0a }, // 
	{ 0x3805, 0x57 }, // 
	{ 0x3806, 0x05 }, // 
	{ 0x3807, 0xD3 }, // 
	{ 0x3808, 0x05 }, // 
	{ 0x3809, 0x00 }, // 
	{ 0x380a, 0x02 }, // 
	{ 0x380b, 0xD0 }, // 
	{ 0x380c, 0x0a }, //  ; 03 
	{ 0x380d, 0x1c }, // ; 5C 
	{ 0x380e, 0x03 }, // 
	{ 0x380f, 0x05 }, //
	{ 0x3810, 0x00 }, // 
	{ 0x3811, 0x04 }, // 
	{ 0x3812, 0x00 }, // 
	{ 0x3813, 0x02 }, // 
	{ 0x3814, 0x03 }, // 
	{ 0x3815, 0x01 }, // 
	{ 0x3819, 0x01 }, // 
	{ 0x3820, 0x10 }, // 
	{ 0x3821, 0x07 }, // 
	{ 0x3829, 0x00 }, // 
	{ 0x382a, 0x03 }, // 
	{ 0x382b, 0x01 }, // 
	{ 0x382d, 0x7f }, // 
	{ 0x3830, 0x08 }, // 
	{ 0x3836, 0x02 }, // 
	{ 0x3837, 0x00 }, // 
	{ 0x3841, 0x02 }, // 
	{ 0x3846, 0x08 }, // 
	{ 0x3847, 0x07 }, // 
	{ 0x3d85, 0x36 }, // 
	{ 0x3d8c, 0x71 }, // 
	{ 0x3d8d, 0xcb }, // 
	{ 0x3f0a, 0x00 }, // 
	{ 0x4000, 0x71 }, // 
	{ 0x4001, 0x50 }, // 
	{ 0x4002, 0x04 }, // 
	{ 0x4003, 0x14 }, // 
	{ 0x400e, 0x00 }, // 
	{ 0x4011, 0x00 }, // 
	{ 0x401a, 0x00 }, // 
	{ 0x401b, 0x00 }, // 
	{ 0x401c, 0x00 }, // 
	{ 0x401d, 0x00 }, // 
	{ 0x401f, 0x00 }, // 
	{ 0x4020, 0x00 }, // 
	{ 0x4021, 0x10 }, // 
	{ 0x4022, 0x03 }, // 
	{ 0x4023, 0x93 }, // 
	{ 0x4024, 0x04 }, //
	{ 0x4025, 0xC0 }, // 
	{ 0x4026, 0x04 }, // 
	{ 0x4027, 0xD0 }, // 
	{ 0x4028, 0x00 }, // 
	{ 0x4029, 0x02 }, // 
	{ 0x402a, 0x06 }, // 
	{ 0x402b, 0x04 }, // 
	{ 0x402c, 0x02 }, // 
	{ 0x402d, 0x02 }, // 
	{ 0x402e, 0x0e }, // 
	{ 0x402f, 0x04 }, // 
	{ 0x4302, 0xff }, // 
	{ 0x4303, 0xff }, // 
	{ 0x4304, 0x00 }, // 
	{ 0x4305, 0x00 }, // 
	{ 0x4306, 0x00 }, // 
	{ 0x4308, 0x02 }, // 
	{ 0x4500, 0x6c }, // 
	{ 0x4501, 0xc4 }, // 
	{ 0x4502, 0x44 }, // 
	{ 0x4503, 0x01 }, // 
	{ 0x4601, 0x4F }, // 
	{ 0x4800, 0x04 }, // 
	{ 0x4813, 0x08 }, // 
	{ 0x481f, 0x40 }, // 
	{ 0x4829, 0x78 }, // 
	{ 0x4837, 0x16 }, // 
	{ 0x4b00, 0x2a }, // 
	{ 0x4b0d, 0x00 }, // 
	{ 0x4d00, 0x04 }, // 
	{ 0x4d01, 0x42 }, // 
	{ 0x4d02, 0xd1 }, // 
	{ 0x4d03, 0x93 }, // 
	{ 0x4d04, 0xf5 }, // 
	{ 0x4d05, 0xc1 }, // 
	{ 0x5000, 0xf3 }, // 
	{ 0x5001, 0x11 }, // 
	{ 0x5004, 0x00 }, // 
	{ 0x500a, 0x00 }, // 
	{ 0x500b, 0x00 }, // 
	{ 0x5032, 0x00 }, // 
	{ 0x5040, 0x00 }, // 
	{ 0x5050, 0x3c }, // 
	{ 0x5500, 0x00 }, // 
	{ 0x5501, 0x10 }, // 
	{ 0x5502, 0x01 }, // 
	{ 0x5503, 0x0f }, // 
	{ 0x8000, 0x00 }, // 
	{ 0x8001, 0x00 }, // 
	{ 0x8002, 0x00 }, // 
	{ 0x8003, 0x00 }, // 
	{ 0x8004, 0x00 }, // 
	{ 0x8005, 0x00 }, // 
	{ 0x8006, 0x00 }, // 
	{ 0x8007, 0x00 }, // 
	{ 0x8008, 0x00 }, // 
	{ 0x3638, 0x00 }, // 
	{ 0x0100, 0x01 }, // 
	{ 0, 0 },			  //End
}; 

//----------------------------------------------
// MCLK: 24Mhz
// resolution: 2304x1296
// Mipi : 1lane 
// Mipi data rate: 960Mbps/Lane
// SystemCLK   :120Mhz
// FPS	       :30 ( = SystemCLK / (HTS*VTS) ) 
// HTS		   :2998(R380c:R380d)
// VTS		   :1334(R380e:R380f)
// Tline 	   :24.98us
// Max exp line:1330 (VTS-4)
//---------------------------------------------
const regval16_t OV4689_Mipi_Raw10_2304x1296_30fps[] =
{
	{ 0x0103, 0x01 }, //
	{ 0x3638, 0x00 }, //
	{ 0x0300, 0x00 }, //
	{ 0x0302, 0x28 }, //; 2a 
	{ 0x0303, 0x00 }, //
	{ 0x0304, 0x03 }, //
	{ 0x030b, 0x00 }, //
	{ 0x030d, 0x1e }, //
	{ 0x030e, 0x04 }, //
	{ 0x030f, 0x01 }, //
	{ 0x0312, 0x01 }, //
	{ 0x031e, 0x00 }, //
	{ 0x3000, 0x20 }, //
	{ 0x3002, 0x00 }, //
	{ 0x3018, 0x12 }, //; 12/32/72 1lane/2lane/4lane
	{ 0x3019, 0x0e }, //; 0e/0c/00 1lane/2lane/4lane 
	{ 0x3020, 0x93 }, //
	{ 0x3021, 0x03 }, //
	{ 0x3022, 0x01 }, //
	{ 0x3031, 0x0a }, //
	{ 0x303f, 0x0c }, //
	{ 0x3305, 0xf1 }, //
	{ 0x3307, 0x04 }, //
	{ 0x3309, 0x29 }, //
	{ 0x3500, 0x00 }, //
	{ 0x3501, 0x50 }, //
	{ 0x3502, 0x00 }, //
	{ 0x3503, 0x04 }, //
	{ 0x3504, 0x00 }, //
	{ 0x3505, 0x00 }, //
	{ 0x3506, 0x00 }, //
	{ 0x3507, 0x00 }, //
	{ 0x3508, 0x00 }, //
	{ 0x3509, 0x80 }, //
	{ 0x350a, 0x00 }, //
	{ 0x350b, 0x00 }, //
	{ 0x350c, 0x00 }, //
	{ 0x350d, 0x00 }, //
	{ 0x350e, 0x00 }, //
	{ 0x350f, 0x80 }, //
	{ 0x3510, 0x00 }, //
	{ 0x3511, 0x00 }, //
	{ 0x3512, 0x00 }, //
	{ 0x3513, 0x00 }, //
	{ 0x3514, 0x00 }, //
	{ 0x3515, 0x80 }, //
	{ 0x3516, 0x00 }, //
	{ 0x3517, 0x00 }, //
	{ 0x3518, 0x00 }, //
	{ 0x3519, 0x00 }, //
	{ 0x351a, 0x00 }, //
	{ 0x351b, 0x80 }, //
	{ 0x351c, 0x00 }, //
	{ 0x351d, 0x00 }, //
	{ 0x351e, 0x00 }, //
	{ 0x351f, 0x00 }, //
	{ 0x3520, 0x00 }, //
	{ 0x3521, 0x80 }, //
	{ 0x3522, 0x08 }, //
	{ 0x3524, 0x08 }, //
	{ 0x3526, 0x08 }, //
	{ 0x3528, 0x08 }, //
	{ 0x352a, 0x08 }, //
	{ 0x3602, 0x00 }, //
	{ 0x3603, 0x40 }, //
	{ 0x3604, 0x02 }, //
	{ 0x3605, 0x00 }, //
	{ 0x3606, 0x00 }, //
	{ 0x3607, 0x00 }, //
	{ 0x3609, 0x12 }, //
	{ 0x360a, 0x40 }, //
	{ 0x360c, 0x08 }, //
	{ 0x360f, 0xe5 }, //
	{ 0x3608, 0x8f }, //
	{ 0x3611, 0x00 }, //
	{ 0x3613, 0xf7 }, //
	{ 0x3616, 0x58 }, //
	{ 0x3619, 0x99 }, //
	{ 0x361b, 0x60 }, //
	{ 0x361c, 0x7a }, //
	{ 0x361e, 0x79 }, //
	{ 0x361f, 0x02 }, //
	{ 0x3632, 0x00 }, //
	{ 0x3633, 0x10 }, //
	{ 0x3634, 0x10 }, //
	{ 0x3635, 0x10 }, //
	{ 0x3636, 0x15 }, //
	{ 0x3646, 0x86 }, //
	{ 0x364a, 0x0b }, //
	{ 0x3700, 0x17 }, //
	{ 0x3701, 0x22 }, //
	{ 0x3703, 0x10 }, //
	{ 0x370a, 0x37 }, //
	{ 0x3705, 0x00 }, //
	{ 0x3706, 0x63 }, //
	{ 0x3709, 0x3c }, //
	{ 0x370b, 0x01 }, //
	{ 0x370c, 0x30 }, //
	{ 0x3710, 0x24 }, //
	{ 0x3711, 0x0c }, //
	{ 0x3716, 0x00 }, //
	{ 0x3720, 0x28 }, //
	{ 0x3729, 0x7b }, //
	{ 0x372a, 0x84 }, //
	{ 0x372b, 0xbd }, //
	{ 0x372c, 0xbc }, //
	{ 0x372e, 0x52 }, //
	{ 0x373c, 0x0e }, //
	{ 0x373e, 0x33 }, //
	{ 0x3743, 0x10 }, //
	{ 0x3744, 0x88 }, //
	{ 0x3745, 0xc0 }, //
	{ 0x374a, 0x43 }, //
	{ 0x374c, 0x00 }, //
	{ 0x374e, 0x23 }, //
	{ 0x3751, 0x7b }, //
	{ 0x3752, 0x84 }, //
	{ 0x3753, 0xbd }, //
	{ 0x3754, 0xbc }, //
	{ 0x3756, 0x52 }, //
	{ 0x375c, 0x00 }, //
	{ 0x3760, 0x00 }, //
	{ 0x3761, 0x00 }, //
	{ 0x3762, 0x00 }, //
	{ 0x3763, 0x00 }, //
	{ 0x3764, 0x00 }, //
	{ 0x3767, 0x04 }, //
	{ 0x3768, 0x04 }, //
	{ 0x3769, 0x08 }, //
	{ 0x376a, 0x08 }, //
	{ 0x376b, 0x20 }, //
	{ 0x376c, 0x00 }, //
	{ 0x376d, 0x00 }, //
	{ 0x376e, 0x00 }, //
	{ 0x3773, 0x00 }, //
	{ 0x3774, 0x51 }, //
	{ 0x3776, 0xbd }, //
	{ 0x3777, 0xbd }, //
	{ 0x3781, 0x18 }, //
	{ 0x3783, 0x25 }, //
	{ 0x3798, 0x1b }, //
	{ 0x3800, 0x00 }, //
	{ 0x3801, 0xc8 }, //; 08 
	{ 0x3802, 0x00 }, //
	{ 0x3803, 0x74 }, //; 04 
	{ 0x3804, 0x0a }, //
	{ 0x3805, 0xd7 }, //; 97 
	{ 0x3806, 0x05 }, //
	{ 0x3807, 0x8b }, //; fb 
	{ 0x3808, 0x09 }, //; 0a 
	{ 0x3809, 0x00 }, //; 80 
	{ 0x380a, 0x05 }, //
	{ 0x380b, 0x10 }, //; f0 
	{ 0x380c, 0x0b }, //; 0f 
	{ 0x380d, 0xb6 }, //; 16 
	{ 0x380e, 0x05 }, //; 06 
	{ 0x380f, 0x36 }, //; 12 
	{ 0x3810, 0x00 }, //
	{ 0x3811, 0x08 }, //
	{ 0x3812, 0x00 }, //
	{ 0x3813, 0x04 }, //
	{ 0x3814, 0x01 }, //
	{ 0x3815, 0x01 }, //
	{ 0x3819, 0x01 }, //
	{ 0x3820, 0x00 }, //
	{ 0x3821, 0x06 }, //
	{ 0x3829, 0x00 }, //
	{ 0x382a, 0x01 }, //
	{ 0x382b, 0x01 }, //
	{ 0x382d, 0x7f }, //
	{ 0x3830, 0x04 }, //
	{ 0x3836, 0x01 }, //
	{ 0x3837, 0x00 }, //
	{ 0x3841, 0x02 }, //
	{ 0x3846, 0x08 }, //
	{ 0x3847, 0x07 }, //
	{ 0x3d85, 0x36 }, //
	{ 0x3d8c, 0x71 }, //
	{ 0x3d8d, 0xcb }, //
	{ 0x3f0a, 0x00 }, //
	{ 0x4000, 0x71 }, //
	{ 0x4001, 0x40 }, //
	{ 0x4002, 0x04 }, //
	{ 0x4003, 0x14 }, //
	{ 0x400e, 0x00 }, //
	{ 0x4011, 0x00 }, //
	{ 0x401a, 0x00 }, //
	{ 0x401b, 0x00 }, //
	{ 0x401c, 0x00 }, //
	{ 0x401d, 0x00 }, //
	{ 0x401f, 0x00 }, //
	{ 0x4020, 0x00 }, //
	{ 0x4021, 0x10 }, //
	{ 0x4022, 0x07 }, //
	{ 0x4023, 0x93 }, //; cf 
	{ 0x4024, 0x08 }, //; 09 
	{ 0x4025, 0xc0 }, //; 60 
	{ 0x4026, 0x08 }, //; 09 
	{ 0x4027, 0xd0 }, //; 6f 
	{ 0x4028, 0x00 }, //
	{ 0x4029, 0x02 }, //
	{ 0x402a, 0x06 }, //
	{ 0x402b, 0x04 }, //
	{ 0x402c, 0x02 }, //
	{ 0x402d, 0x02 }, //
	{ 0x402e, 0x0e }, //
	{ 0x402f, 0x04 }, //
	{ 0x4302, 0xff }, //
	{ 0x4303, 0xff }, //
	{ 0x4304, 0x00 }, //
	{ 0x4305, 0x00 }, //
	{ 0x4306, 0x00 }, //
	{ 0x4308, 0x02 }, //
	{ 0x4500, 0x6c }, //
	{ 0x4501, 0xc4 }, //
	{ 0x4502, 0x40 }, //
	{ 0x4503, 0x01 }, //
	{ 0x4601, 0x8f }, //; A7
	{ 0x4800, 0x04 }, //
	{ 0x4813, 0x08 }, //
	{ 0x481f, 0x40 }, //
	{ 0x4829, 0x78 }, //
	{ 0x4837, 0x10 }, //; 12 
	{ 0x4b00, 0x2a }, //
	{ 0x4b0d, 0x00 }, //
	{ 0x4d00, 0x04 }, //
	{ 0x4d01, 0x42 }, //
	{ 0x4d02, 0xd1 }, //
	{ 0x4d03, 0x93 }, //
	{ 0x4d04, 0xf5 }, //
	{ 0x4d05, 0xc1 }, //
	{ 0x5000, 0xf3 }, //
	{ 0x5001, 0x11 }, //
	{ 0x5004, 0x00 }, //
	{ 0x500a, 0x00 }, //
	{ 0x500b, 0x00 }, //
	{ 0x5032, 0x00 }, //
	{ 0x5040, 0x00 }, //
	{ 0x5050, 0x0c }, //
	{ 0x5500, 0x00 }, //
	{ 0x5501, 0x10 }, //
	{ 0x5502, 0x01 }, //
	{ 0x5503, 0x0f }, //
	{ 0x8000, 0x00 }, //
	{ 0x8001, 0x00 }, //
	{ 0x8002, 0x00 }, //
	{ 0x8003, 0x00 }, //
	{ 0x8004, 0x00 }, //
	{ 0x8005, 0x00 }, //
	{ 0x8006, 0x00 }, //
	{ 0x8007, 0x00 }, //
	{ 0x8008, 0x00 }, //
	{ 0x3638, 0x00 }, //
	{ 0x0100, 0x01 }, //
	{ 0, 0 },			  //End
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
	g_ti2c_handle.pDeviceString = "OV4689_MIPI";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV4689_MIPI ti2c request failed\n");
		return -1;
	}
	gp_ti2c_bus_int_mode_en(1);
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
OV4689_read(
	unsigned short reg, 
	unsigned char *value
)
{
#if (I2C_MODE == HW_I2C)
	char addr[2], data[1];
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
	int nRet = 0;
	int retry = 0;
	
	addr[0] = (reg >> 8) & 0xFF;
	addr[1] = reg & 0xFF;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = addr;	
	g_ti2c_handle.dataCnt = 2;	
	nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
	while(nRet<0) {
		retry++;
		nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
		if(nRet<0 && retry>TI2C_RETRY) {
			printk("retry too much\n");
			return nRet;
		}
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
OV4689_write(
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
	int ret;
	int retry = 0;

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = value;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 3;
	ret = gp_ti2c_bus_xfer(&g_ti2c_handle);
	while(ret<0) {
		retry++;
		ret = gp_ti2c_bus_xfer(&g_ti2c_handle);
		if(ret<0 && retry>TI2C_RETRY) {
			printk("retry too much\n");
			return ret;
		}
	}
	return ret;
#endif	
}

static int ov4689_get_real_agc_gain(int agc_gain)
{
	int real_agc_gain;

	real_agc_gain = 0x10 + (agc_gain & 0x0f);
	real_agc_gain = real_agc_gain * (1 + ((agc_gain >> 4) & 1)) * (1 + ((agc_gain >> 5) & 1))
			* (1 + ((agc_gain >> 6) & 1)) * (1 + ((agc_gain >> 7) & 1)) * (1 + ((agc_gain >> 8) & 1));
	
	return real_agc_gain;
}

static int ov4689_cvt_agc_gain(int agc_gain)
{
	int sensor_agc_gain, i;

	sensor_agc_gain = 0;
	i = 5;
	do {
		if(agc_gain <= 0x1f) 
			break;
		
		agc_gain >>= 1;
		sensor_agc_gain <<= 1;
		sensor_agc_gain |= 0x10;
		
		i--;
	} while(i != 0);

	agc_gain -= 0x10;
	if(agc_gain < 0) agc_gain = 0;
	sensor_agc_gain += agc_gain;
	
	return sensor_agc_gain;
}

static int ov4689_set_exposure_time(sensor_exposure_t *se)
{
	unsigned char t1, t2, t3;
	int cvt_gain;
	float Gain;

	cvt_gain = ov4689_cvt_agc_gain(se->analog_gain >> 4);
	Gain = (float)cvt_gain / 16;

	OV4689_write(0x3208, 0x00); // Group hold enable 
	
	// AEC Gain
	if(Gain >= 1 && Gain < 2) {
		OV4689_write(0x3508, 0);
		OV4689_write(0x3509, Gain * 128);
	}
	else if(Gain >= 2 && Gain < 4) {
		OV4689_write(0x3508, 1);
		OV4689_write(0x3509, Gain * 64 - 8);
	}
	else if(Gain >= 4 && Gain < 8) {
		OV4689_write(0x3508, 3);
		OV4689_write(0x3509, Gain * 32 - 12);
	}
	else if(Gain >= 8 && Gain < 15.5) {
		OV4689_write(0x3508, 7);
		OV4689_write(0x3509, Gain * 16 - 8);
	}

	// exposure time
	t1 = (se->time & 0x0f) << 4;
	t2 = (se->time >> 4) & 0x00ff;
	t3 = (se->time >> 12) & 0x000f;

	OV4689_write(0x3502, t1);
	OV4689_write(0x3501, t2);
	OV4689_write(0x3500, t3);

	OV4689_write(0x3208, 0x10); //Group latch end
	OV4689_write(0x3208, 0xa0); // Group Latch Launch

	//printk("%s: time = 0x%x, analog_gain = 0x%x, cvt_gain = 0x%x\n", __FUNCTION__, se->time, se->analog_gain, cvt_gain);
	return 0;
}

static int ov4689_get_exposure_time(sensor_exposure_t *se)
{
	unsigned char t1, t2, t3;
	int cvt_gain = 0;

	// AEC Gain
	OV4689_read(0x3508, &t1);
	OV4689_read(0x3509, &t2);
	
	if(t1 == 0) {
		cvt_gain = (16 * t2) / 128; 
	}
	else if(t1 == 1) {
		cvt_gain = (16 * t2 + 8) / 64; 
	}
	else if(t1 == 3) {
		cvt_gain = (16 * t2 + 12) / 32; 
	}
	else if(t1 == 2) {
		cvt_gain = (16 * t2 + 8) / 16;
	}

	se->analog_gain = ov4689_get_real_agc_gain(cvt_gain) << 4;

	// exposure time
	OV4689_read(0x3502, &t1);
	OV4689_read(0x3501, &t2);
	OV4689_read(0x3500, &t3);
	se->time = ((t1 >> 4) & 0x0F) | (t2 << 4) | (t3 << 12);
	
	//printk("%s: time = 0x%x, analog_gain = 0x%x, cvt_gain = 0x%x\n", __FUNCTION__, se->time, se->analog_gain, cvt_gain);
	return 0;
}

static int
OV4689WrTable(
	regval16_t *pTable
)
{
	while(1) {
		if (pTable->reg_num == 0 && pTable->value == 0) {
			break;
		} else if (pTable->reg_num == 0xffff && pTable->value == 0xffff) {
			udelay(1000);//ytliao: delay 100 will make sensor fail, increase to 1000
			pTable++;
			continue;	
		} else if (pTable->reg_num == 0xfffe && pTable->value == 0xfffe) {
			mdelay(22);//comi
			pTable++;
			continue;	
		}
		
		if ( OV4689_write(pTable->reg_num, pTable->value) < 0) {
			return -1;
		}

		pTable ++;
	}

	return 0;
}

static int 
OV4689_preview(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return OV4689WrTable((regval16_t *)OV4689_Mipi_Raw10_720P_30fps);
}

static int 
OV4689_capture(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return OV4689WrTable((regval16_t *)OV4689_Mipi_Raw10_2304x1296_30fps);
}

static int 
OV4689_record(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return OV4689WrTable((regval16_t *)OV4689_Mipi_Raw10_720P_60fps);
}

static int 
OV4689_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	seInfo.time = 0x10;
	seInfo.analog_gain = OV4689_MIN_ANALOG_GAIN;
	seInfo.digital_gain = OV4689_MIN_DIGITAL_GAIN;

	seInfo.max_time = OV4689_MAX_EXPOSURE_TIME;
	seInfo.max_analog_gain = OV4689_MAX_ANALOG_GAIN;
	seInfo.max_digital_gain = OV4689_MAX_DIGITAL_GAIN;


	seInfo.min_time = OV4689_MIN_EXPOSURE_TIME;
	seInfo.min_analog_gain = OV4689_MIN_ANALOG_GAIN;
	seInfo.min_digital_gain = OV4689_MIN_DIGITAL_GAIN;

	seInfo.userISO = DISABLE;


	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
OV4689_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	return 0;
}

static int 
OV4689_queryctrl(
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
OV4689_g_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{	
	unsigned char value;
	
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
		
	case V4L2_CID_EXPOSURE:
		ov4689_get_exposure_time(&seInfo);
		ctrl->value = (int)&seInfo;
		break;
		
	case V4L2_CID_VFLIP:
		OV4689_read(0x3820, &value);
		ctrl->value = (value >> 1) & 0x01;
		break;
		
	case V4L2_CID_HFLIP:
		OV4689_read(0x3821, &value);
		ctrl->value = (value >> 1) & 0x01;
		break;
		
	case V4L2_CID_BRIGHTNESS:
		break;
		
	default:
		return -EINVAL;
	}

	return 0;
}

static int 
OV4689_s_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{
	int nRet = 0;
	unsigned char value;

	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
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

	case V4L2_CID_EXPOSURE:
		{
			sensor_exposure_t *si;
			si = (sensor_exposure_t *) ctrl->value;
			seInfo.time = si->time;
			seInfo.analog_gain = si->analog_gain;
			seInfo.userISO = si->userISO;
			nRet = ov4689_set_exposure_time(&seInfo);
		}	
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:		
		break;
		
	case V4L2_CID_VFLIP:
		OV4689_read(0x3820, &value);
		value &= ~(1 << 1);
		value |= (ctrl->value << 1);
		OV4689_write(0x3820, value);
		break;
		
	case V4L2_CID_HFLIP:
		OV4689_read(0x3281, &value);
		value &= ~(0x1 << 1);
		value |= (ctrl->value << 1);
		OV4689_write(0x3281, value);
		break;

	case V4L2_CID_BRIGHTNESS:
		break;

	case V4L2_CID_AUTOGAIN:
	case V4L2_CID_GAIN:
		break;
		
	default:
		return -EINVAL;
	}
	
	return nRet; 
}

static int 
OV4689_querystd(
	struct v4l2_subdev *sd,
	v4l2_std_id *std
)
{
	return 0;
}

static int 
OV4689_enum_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_fmtdesc *fmtdesc
)
{
	printk("%s\n", __FUNCTION__);
	if(fmtdesc->index >= C_SENSOR_FMT_MAX) {
		return -EINVAL;
	}

	fmtdesc->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	memcpy((void *)fmtdesc->description, (void *)OV4689FmtTable[fmtdesc->index].desc, 32);
	fmtdesc->pixelformat = OV4689FmtTable[fmtdesc->index].pixelformat;
	return 0;
}

static int 
OV4689_g_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	printk("%s\n", __FUNCTION__);
	fmt->fmt.pix.width = OV4689Dev.fmt->hpixel;
	fmt->fmt.pix.height = OV4689Dev.fmt->vline;
	fmt->fmt.pix.pixelformat = OV4689Dev.fmt->pixelformat;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = OV4689Dev.fmt->hpixel * OV4689Dev.fmt->bpp;
	fmt->fmt.pix.sizeimage = fmt->fmt.pix.bytesperline * OV4689Dev.fmt->vline;
	return 0;
}

static int 
OV4689_try_fmt(
	struct v4l2_subdev *sd,
	struct v4l2_format *fmt
)
{
	return 0;
}

static int 
OV4689_s_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	int ret;

	printk("%s = %d\n", __FUNCTION__, fmt->fmt.pix.priv);
	switch(fmt->fmt.pix.priv)
	{
	case 0: 
		ret = OV4689_preview();
		break;

	case 1: 
		ret = OV4689_capture();
		break;

	case 2: 
		ret = OV4689_record();
		break;	
	
	default:
		ret = -1;
	}

	OV4689Dev.fmt = &OV4689FmtTable[fmt->fmt.pix.priv];

	if(ret == 0) {
		printk("%s SUCCESS\n", __FUNCTION__);
	} else {
		printk("%s fail\n", __FUNCTION__);
	}
	
	return ret;
}

static int 
OV4689_cropcap(
	struct v4l2_subdev *sd,
	struct v4l2_cropcap *cc
)
{
	return 0;
}

static int 
OV4689_g_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
OV4689_s_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
OV4689_g_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *parms
)
{
	return 0;
}

static int 
OV4689_s_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *param
)
{
	return 0;
}

static int 
OV4689_s_interface(
	struct v4l2_subdev *sd,
	struct v4l2_interface *interface
)
{
	return 0;
}

static int 
OV4689_suspend(
	struct v4l2_subdev *sd
)
{
	return 0;
}

static int 
OV4689_resume(
	struct v4l2_subdev *sd
)
{
	return 0;
}

static const struct v4l2_subdev_core_ops OV4689_core_ops = 
{
	.init = OV4689_init,
	.reset = OV4689_reset,
	.queryctrl = OV4689_queryctrl,
	.g_ctrl = OV4689_g_ctrl,
	.s_ctrl = OV4689_s_ctrl,
};

static const struct v4l2_subdev_video_ops OV4689_video_ops = 
{
	.querystd = OV4689_querystd,
	.enum_fmt = OV4689_enum_fmt,
	.g_fmt = OV4689_g_fmt,
	.try_fmt = OV4689_try_fmt,
	.s_fmt = OV4689_s_fmt,
	.cropcap = OV4689_cropcap,
	.g_crop = OV4689_g_crop,
	.s_crop = OV4689_s_crop,
	.g_parm = OV4689_g_parm,
	.s_parm = OV4689_s_parm,
};

static const struct v4l2_subdev_ext_ops OV4689_ext_ops = 
{
	.s_interface = OV4689_s_interface,
	.suspend = OV4689_suspend,
	.resume = OV4689_resume,
};

static const struct v4l2_subdev_ops OV4689_ops = 
{
	.core = &OV4689_core_ops,
	.video = &OV4689_video_ops,
	.ext = &OV4689_ext_ops
};

static int __init 
OV4689_module_init(
		void
)
{
	if(sensor_i2c_open(OV4689_ID, 50) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: OV4689 mipi\n");
	OV4689Dev.fmt = &OV4689FmtTable[0];
	v4l2_subdev_init(&(OV4689Dev.sd), &OV4689_ops);
	strcpy(OV4689Dev.sd.name, "sensor_ov4689_mipi");
	register_sensor(&OV4689Dev.sd, (int *)&param[0], &OV4689_config_table);
	return 0;
}

static void __exit
OV4689_module_exit(
		void
)
{
	sensor_i2c_close();
	unregister_sensor(&(OV4689Dev.sd));
}

module_init(OV4689_module_init);
module_exit(OV4689_module_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus OV4689 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

