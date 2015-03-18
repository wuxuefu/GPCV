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
#define AR0330_MIPI_CLK_NO_STOP_EN		1

#define AR0330_ID		0x20
#define AR0330_FULLHD	0


// 30fps
#define AR0330_30FPS_50HZ_INIT_EV_IDX 			135
#define AR0330_30FPS_50HZ_DAY_EV_IDX 			114
#define AR0330_30FPS_50HZ_NIGHT_EV_IDX		174
#define AR0330_30FPS_50HZ_EXP_TIME_TOTAL		224
#define AR0330_30FPS_50HZ_MAX_EV_IDX			(AR0330_30FPS_50HZ_EXP_TIME_TOTAL - 27) //15)


#define AR0330_30FPS_60HZ_INIT_EV_IDX 			135
#define AR0330_30FPS_60HZ_DAY_EV_IDX 			114
#define AR0330_30FPS_60HZ_NIGHT_EV_IDX		174
#define AR0330_30FPS_60HZ_EXP_TIME_TOTAL		226
#define AR0330_30FPS_60HZ_MAX_EV_IDX			(AR0330_30FPS_60HZ_EXP_TIME_TOTAL - 27) //15)


// 25fps ldw
#define AR0330_25FPS_50HZ_LDW_INIT_EV_IDX 			135
#define AR0330_25FPS_50HZ_LDW_DAY_EV_IDX 			110
#define AR0330_25FPS_50HZ_LDW_NIGHT_EV_IDX		168
#define AR0330_25FPS_50HZ_LDW_EXP_TIME_TOTAL		219
#define AR0330_25FPS_50HZ_LDW_MAX_EV_IDX			(AR0330_25FPS_50HZ_LDW_EXP_TIME_TOTAL - 20)//9)


#define AR0330_25FPS_60HZ_LDW_INIT_EV_IDX 			135
#define AR0330_25FPS_60HZ_LDW_DAY_EV_IDX 			110
#define AR0330_25FPS_60HZ_LDW_NIGHT_EV_IDX		168
#define AR0330_25FPS_60HZ_LDW_EXP_TIME_TOTAL		221
#define AR0330_25FPS_60HZ_LDW_MAX_EV_IDX			(AR0330_25FPS_60HZ_LDW_EXP_TIME_TOTAL - 20)//9)


// 25fps
#define AR0330_25FPS_50HZ_INIT_EV_IDX 			150
#define AR0330_25FPS_50HZ_DAY_EV_IDX 			131
#define AR0330_25FPS_50HZ_NIGHT_EV_IDX		191
#define AR0330_25FPS_50HZ_EXP_TIME_TOTAL		251
#define AR0330_25FPS_50HZ_MAX_EV_IDX			(AR0330_25FPS_50HZ_EXP_TIME_TOTAL - 20)//9)
#define AR0330_25FPS_50HZ_SWITCH_TIME		1540

#define AR0330_25FPS_60HZ_INIT_EV_IDX 			150
#define AR0330_25FPS_60HZ_DAY_EV_IDX 			132
#define AR0330_25FPS_60HZ_NIGHT_EV_IDX		185
#define AR0330_25FPS_60HZ_EXP_TIME_TOTAL		244
#define AR0330_25FPS_60HZ_MAX_EV_IDX			(AR0330_25FPS_60HZ_EXP_TIME_TOTAL - 20)//9)
#define AR0330_25FPS_60HZ_SWITCH_TIME		1288		


// 60fps
#define AR0330_60FPS_50HZ_INIT_EV_IDX 			135
#define AR0330_60FPS_50HZ_DAY_EV_IDX 			118
#define AR0330_60FPS_50HZ_NIGHT_EV_IDX		147
#define AR0330_60FPS_50HZ_EXP_TIME_TOTAL		197
#define AR0330_60FPS_50HZ_MAX_EV_IDX			(AR0330_60FPS_50HZ_EXP_TIME_TOTAL - 1)


#define AR0330_60FPS_60HZ_INIT_EV_IDX 			135
#define AR0330_60FPS_60HZ_DAY_EV_IDX 			112
#define AR0330_60FPS_60HZ_NIGHT_EV_IDX		159
#define AR0330_60FPS_60HZ_EXP_TIME_TOTAL		211
#define AR0330_60FPS_60HZ_MAX_EV_IDX			(AR0330_60FPS_60HZ_EXP_TIME_TOTAL - 5)


#define AR0330_MIN_D_GAIN						(1.50)



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

#if AR0330_FULLHD
//#define AR0330_MAX_EXPOSURE_TIME			0x2c0//0x0438
#define AR0330_MAX_EXPOSURE_TIME			658 // 50Hz
//#define AR0330_MAX_EXPOSURE_TIME			548 // 60Hz
#define AR0330_MIN_EXPOSURE_TIME			0x0008
#define AR0330_MAX_ANALOG_GAIN			(int)(6.5*256)
#define AR0330_MIN_ANALOG_GAIN			(1*256)
#define AR0330_MAX_DIGITAL_GAIN			0x0100//0x02c0//0x00E0//0x07ff
#define AR0330_MIN_DIGITAL_GAIN 			0X0080
#else
#define AR0330_MAX_EXPOSURE_TIME			1182 // 30ms
//#define AR0330_MAX_EXPOSURE_TIME			1310 // 33ms
//#define AR0330_MAX_EXPOSURE_TIME			792 // 20ms = 50Hz
//#define AR0330_MAX_EXPOSURE_TIME			660 // 60Hz
#define AR0330_MIN_EXPOSURE_TIME			0x0004
#define AR0330_MAX_ANALOG_GAIN			(int)(6*256)
#define AR0330_MIN_ANALOG_GAIN			(1*256)
#define AR0330_MAX_DIGITAL_GAIN			(int)(4*128)	//0x02c0//0x00E0//0x07ff
#define AR0330_MIN_DIGITAL_GAIN 			0X0080
#endif


#define TI2C_RETRY		5
#define	FLIP_MIRROR		0

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
	unsigned short value;
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
 static int current_frame_rate;

static int *p_expTime_table;
static int sensor_fps, sensor_total_ev, sensor_swith_time;
static int pre_sensor_time, pre_sensor_a_gain, pre_sensor_d_gain;
static int pre_sensor_time_b, pre_sensor_a_gain_b, pre_sensor_d_gain_b;
static sensor_exposure_t 	seInfo;
static sensor_dev_t	gAR0330Dev;
#if (I2C_MODE == HW_I2C)
static int g_i2c_handle;
#elif (I2C_MODE == HW_TI2C)
static ti2c_set_value_t g_ti2c_handle;
#endif

static char *param[] = {"0", "MIPI", "0", "NONE", "0", "NONE"};
static int nstrs = 6;
module_param_array(param, charp, &nstrs, S_IRUGO);

static sensor_fmt_t gAR0330FmtTable[] =
{
	{
		.desc		= "preview=1280*722,crop=1280*720",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 27000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 722,
		.voffset = 0,
		.cdsp_calibration = NULL,//&ar0330_cdsp_calibration,
	},
	{
#if AR0330_FULLHD
		.desc		= "capture=1920*1082,crop=1920*1080",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_320M,
		.mclk = 27000000,
		.hpixel = 1920,
		.hoffset = 0,
		.vline = 1082,
		.voffset = 0,
#else
		
		.desc		= "capture=1920*1084,crop=1920*1080",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_320M,
		.mclk = 27000000,
		.hpixel = 2304,
		.hoffset = 0,
		.vline = 1296,
		.voffset = 0,		
#endif
		.cdsp_calibration = NULL, //&ar0330_cdsp_calibration,
	},
	{
		.desc		= "record=1280*720",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 720,
		.voffset = 0,
		.cdsp_calibration = NULL, //&ar0330_cdsp_calibration,
	},
	{
		.desc		= "ldw=1920*1084,crop=1920*1080",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_320M,
		.mclk = 27000000,
		.hpixel = 2304,
		.hoffset = 0,
		.vline = 1296,
		.voffset = 0,
	},
	{ // for 720p @ 60fps
		.desc		= "capture=1280*722,crop=1280*720",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_320M,
		.mclk = 27000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 722,
		.voffset = 0,
	},
	{
		// 2240x1260 for 25fps to 30fps
		.desc		= "c25_30=1920*1084,crop=1920*1080",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_320M,
		.mclk = 27000000,
		.hpixel = 2240,
		.hoffset = 0,
		.vline = 1260,
		.voffset = 0,
		.cdsp_calibration = NULL,
	},
	{
		// 2240x1260 for 25fps
		.desc		= "cap25=1920*1084,crop=1920*1080",
		.pixelformat = V4L2_PIX_FMT_SGRBG8,
		.bpp 		= 1,
		.mclk_src = MODE_MCLK_SRC_320M,
		.mclk = 27000000,
		.hpixel = 2240,
		.hoffset = 0,
		.vline = 1260,
		.voffset = 0,
		.cdsp_calibration = NULL,
	}
};

#define C_SENSOR_FMT_MAX	sizeof(gAR0330FmtTable)/sizeof(sensor_fmt_t)

static mipi_config_t AR0330_mipi_setting = 
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
#if AR0330_MIPI_CLK_NO_STOP_EN == 1 
	.check_hs_seq = MIPI_CHECK_LP_00,	//for mipi clock no stop 
#else 
	.check_hs_seq = MIPI_CHECK_HS_SEQ,	//for mipi clock auto stop 
#endif
};

static sensor_config_t AR0330_config_table =
{
	.sensor_timing_mode = MODE_CCIR_601,
	.sensor_data_mode = MODE_DATA_YUV,
	.sensor_interlace_mode = MODE_NONE_INTERLACE,
	.sensor_pclk_mode = MODE_POSITIVE_EDGE,
	.sensor_hsync_mode = MODE_ACTIVE_HIGH,
	.sensor_vsync_mode = MODE_ACTIVE_LOW,
	.sensor_fmt_num = C_SENSOR_FMT_MAX,
	.fmt = gAR0330FmtTable,
	.mipi_config = &AR0330_mipi_setting,
};

static const int ar0330_analog_gain_table[29] = 
{
	// coarse gain = 0
	(int)(1.00*256+0.5), (int)(1.03*256+0.5), (int)(1.07*256+0.5), (int)(1.10*256+0.5), 
	(int)(1.14*256+0.5), (int)(1.19*256+0.5), (int)(1.23*256+0.5), (int)(1.28*256+0.5), 
	(int)(1.33*256+0.5), (int)(1.39*256+0.5), (int)(1.45*256+0.5), (int)(1.52*256+0.5), 
	(int)(1.60*256+0.5), (int)(1.68*256+0.5), (int)(1.78*256+0.5), (int)(1.88*256+0.5),
	
	// coarse gain = 1
	(int)(2.00*256+0.5), (int)(2.13*256+0.5), (int)(2.29*256+0.5), (int)(2.46*256+0.5), 
	(int)(2.67*256+0.5), (int)(2.91*256+0.5), (int)(3.20*256+0.5), (int)(3.56*256+0.5),

	// coarse gain = 2
	(int)(4.00*256+0.5), (int)(4.57*256+0.5), (int)(5.33*256+0.5), (int)(6.40*256+0.5),

	// coarse gain = 3
	(int)(8.00*256+0.5)
};

static const  int ar0330_30fps_exp_time_gain_50Hz[AR0330_30FPS_50HZ_EXP_TIME_TOTAL][3] = 
{ // {time, analog gain, digital gain}
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{35, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{36, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{37, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{39, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{40, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{41, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{43, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{44, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{46, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{48, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{49, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{51, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{53, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{55, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{57, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{59, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{61, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{63, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{65, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{67, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{70, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{72, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{75, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{77, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{80, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{83, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{86, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{89, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{92, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{95, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{98, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{102, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{106, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{109, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{113, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{117, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{121, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{126, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{130, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{135, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{139, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{144, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{149, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{155, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{160, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{166, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{171, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{178, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{184, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{190, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{197, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{204, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{211, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{219, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{226, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{234, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{243, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{251, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{260, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{269, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{279, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{288, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{299, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{309, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{320, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{331, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{343, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{355, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{368, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{381, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1182, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1182, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1182, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.00 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.04 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.07 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.11 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.15 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.19 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.23 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.27 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.32 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.37 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.41 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.46 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.52 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.57 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.62 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.68 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.74 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.80 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.87 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(1.93 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.00 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.07 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.14 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.22 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.30 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.38 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.46 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.55 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.64 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.73 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.83 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(2.93 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.03 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.14 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.25 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.36 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.48 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.61 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.73 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(3.86 *AR0330_MIN_D_GAIN*256)},
{1182, (int)(4.0*256), (int)(4.00 *AR0330_MIN_D_GAIN*256)}
};

// for 2304x1296 8-bit setting (30fps)
static const  int ar0330_30fps_exp_time_gain_60Hz[AR0330_30FPS_60HZ_EXP_TIME_TOTAL][3] = 
{	// {time, analog gain, digital gain}
	{8,	(int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9,	(int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9,	(int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9,	(int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{33, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{36, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{37, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{38, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{40, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{41, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{42, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{44, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{45, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{47, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{49, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{50, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{52, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{54, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{56, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{58, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{60, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{62, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{64, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{67, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{69, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{71, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{74, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{77, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{79, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{82, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{85, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{88, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{91, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{94, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{98, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{101, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{105, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{108, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{112, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{116, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{120, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{124, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{129, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{133, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{138, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{143, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{148, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{153, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{158, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{164, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{170, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{176, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{182, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{188, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{195, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{202, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{209, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{216, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{224, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{232, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{240, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{249, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{257, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{266, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{276, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{286, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{296, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{306, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{317, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1311, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1311, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.07 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.11 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.15 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.19 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.23 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.27 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.32 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.37 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.41 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.46 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.52 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.57 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.62 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.68 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.74 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.80 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.87 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(1.93 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.00 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.07 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.14 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.22 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.30 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.38 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.46 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.55 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.64 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.73 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.83 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(2.93 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.03 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.14 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.25 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.36 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.48 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.61 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.73 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(3.86 *AR0330_MIN_D_GAIN*256)},
{1311, (int)(4.0*256), (int)(4.00 *AR0330_MIN_D_GAIN*256)}
};




// for 2304x1296 8-bit setting (25fps)
static const  int ar0330_25fps_exp_time_gain_50Hz_ldw[AR0330_25FPS_50HZ_LDW_EXP_TIME_TOTAL][3] = 
{ // {time, analog gain, digital gain}
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{33, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{36, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{37, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{38, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{40, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{41, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{42, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{44, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{45, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{47, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{49, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{50, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{52, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{54, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{56, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{58, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{60, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{62, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{64, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{67, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{69, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{71, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{74, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{77, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{79, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{82, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{85, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{88, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{91, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{94, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{98, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{101, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{105, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{108, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{112, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{116, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{120, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{124, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{129, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{133, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{138, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{143, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{148, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{153, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{158, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{164, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{170, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{176, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{182, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{188, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{195, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{202, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{209, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{216, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{224, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{232, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{240, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{249, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{257, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{266, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{276, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{286, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{296, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{306, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{317, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{328, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{656, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{984, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{984, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.07 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.11 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.15 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.19 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.23 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.27 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.32 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.37 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.41 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.46 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.52 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.57 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.62 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.68 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.74 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.80 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.87 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(1.93 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.00 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.07 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.14 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.22 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.30 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.38 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.46 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.55 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.64 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.73 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.83 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(2.93 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.03 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.14 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.25 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.36 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.48 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.61 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.73 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(3.86 *AR0330_MIN_D_GAIN*256)},
{984, (int)(4.0*256), (int)(4.00 *AR0330_MIN_D_GAIN*256)}
};


// for 2304x1296 8-bit setting (25fps ldw)
static const  int ar0330_25fps_exp_time_gain_60Hz_ldw[AR0330_25FPS_60HZ_LDW_EXP_TIME_TOTAL][3] = 
{	// {time, analog gain, digital gain}
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{33, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{35, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{37, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{38, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{39, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{41, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{42, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{44, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{45, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{47, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{48, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{50, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{52, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{54, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{56, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{58, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{60, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{62, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{64, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{66, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{68, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{71, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{73, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{76, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{79, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{81, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{84, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{87, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{90, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{94, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{97, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{100, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{104, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{107, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{111, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{115, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{119, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{123, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{128, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{132, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{137, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{142, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{147, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{152, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{157, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{163, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{169, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{175, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{181, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{187, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{194, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{201, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{208, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{215, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{223, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{230, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{239, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{247, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{256, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{265, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{274, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{548, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{822, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{822, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.07 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.11 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.15 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.19 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.23 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.27 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.32 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.37 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.41 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.46 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.52 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.57 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.62 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.68 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.74 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.80 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.87 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(1.93 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.00 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.07 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.14 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.22 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.30 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.38 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.46 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.55 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.64 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.73 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.83 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(2.93 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.03 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.14 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.25 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.36 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.48 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.61 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.73 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(3.86 *AR0330_MIN_D_GAIN*256)},
{822, (int)(4.0*256), (int)(4.00 *AR0330_MIN_D_GAIN*256)}	
};



// for 2304x1296 8-bit setting (25fps)
static const  int ar0330_25fps_exp_time_gain_50Hz[AR0330_25FPS_50HZ_EXP_TIME_TOTAL][3] = 
{ // {time, analog gain, digital gain}
#if 1
{4, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{4, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{33, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{35, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{37, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{38, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{39, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{41, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{42, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{43, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{45, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{47, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{48, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{50, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{52, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{54, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{55, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{57, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{59, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{61, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{64, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{66, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{68, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{71, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{73, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{76, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{78, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{81, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{84, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{87, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{90, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{93, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{96, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{100, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{103, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{107, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{111, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{115, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{119, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{123, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{127, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{132, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{136, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{141, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{146, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{151, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{157, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{162, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{168, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{174, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{180, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{186, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{193, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{200, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{207, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{214, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{222, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{230, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{238, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{246, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{255, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{264, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{273, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{283, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{293, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{303, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{314, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{325, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{336, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{348, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{360, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{373, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{386, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{772, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1158, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1542, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1542, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.11*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.15*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.19*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.23*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.27*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.32*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.37*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.41*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.46*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.52*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.57*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.62*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.68*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.74*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.80*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.87*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(1.93*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.00*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.07*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.14*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.22*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.30*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.38*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.46*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.55*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.64*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.73*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.83*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(2.93*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.03*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.14*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.25*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.36*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.48*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.61*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.73*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(3.86*AR0330_MIN_D_GAIN*256)},
{1542, (int)(4.0*256), (int)(4.00*AR0330_MIN_D_GAIN*256)}
#else
{4, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{35, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{36, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{37, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{39, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{40, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{41, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{43, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{44, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{46, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{48, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{49, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{51, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{53, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{55, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{57, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{59, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{61, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{63, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{65, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{67, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{70, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{72, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{75, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{77, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{80, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{83, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{86, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{89, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{92, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{95, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{98, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{102, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{106, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{109, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{113, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{117, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{121, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{126, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{130, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{135, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{139, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{144, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{149, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{155, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{160, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{166, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{171, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{178, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{184, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{190, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{197, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{204, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{211, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{219, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{226, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{234, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{243, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{251, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{260, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{269, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{279, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{288, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{299, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{309, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{320, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{331, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{343, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{355, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{368, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{381, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{394, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{788, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1182, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1576, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1576, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.11*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.15*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.19*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.23*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.27*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.32*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.37*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.41*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.46*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.52*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.57*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.62*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.68*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.74*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.80*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.87*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(1.93*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.00*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.07*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.14*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.22*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.30*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.38*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.46*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.55*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.64*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.73*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.83*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(2.93*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.03*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.14*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.25*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.36*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.48*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.61*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.73*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(3.86*AR0330_MIN_D_GAIN*256)},
{1576, (int)(4.0*256), (int)(4.00*AR0330_MIN_D_GAIN*256)}
#endif
};

// for 2240x1260 8-bit setting (25fps)
static const  int ar0330_25fps_exp_time_gain_60Hz[AR0330_25FPS_60HZ_EXP_TIME_TOTAL][3] = 
{	// {time, analog gain, digital gain}
{4, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{5, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{6, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{7, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{33, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{35, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{36, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{38, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{39, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{40, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{42, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{43, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{45, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{46, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{48, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{50, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{51, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{53, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{55, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{57, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{59, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{61, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{63, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{65, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{68, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{70, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{73, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{75, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{78, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{80, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{83, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{86, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{89, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{92, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{96, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{99, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{103, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{106, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{110, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{114, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{118, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{122, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{126, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{131, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{135, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{140, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{145, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{150, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{156, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{161, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{167, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{173, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{179, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{185, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{191, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{198, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{205, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{212, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{220, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{228, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{236, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{244, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{253, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{262, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{271, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{280, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{290, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{300, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{311, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{322, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{644, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{966, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1288, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1288, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.11*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.15*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.19*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.23*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.27*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.32*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.37*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.41*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.46*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.52*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.57*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.62*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.68*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.74*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.80*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.87*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(1.93*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.00*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.07*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.14*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.22*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.30*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.38*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.46*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.55*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.64*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.73*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.83*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(2.93*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.03*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.14*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.25*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.36*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.48*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.61*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.73*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(3.86*AR0330_MIN_D_GAIN*256)},
{1288, (int)(4.0*256), (int)(4.00*AR0330_MIN_D_GAIN*256)}
};


static const  int ar0330_60fps_exp_time_gain_50Hz[AR0330_60FPS_50HZ_EXP_TIME_TOTAL][3] = 
{	// {time, analog gain, digital gain}
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{32, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{33, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{35, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{36, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{38, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{39, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{40, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{42, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{43, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{45, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{46, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{48, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{50, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{52, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{53, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{55, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{57, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{59, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{61, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{64, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{66, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{68, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{70, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{73, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{76, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{78, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{81, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{84, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{87, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{90, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{93, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{96, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{100, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{103, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{107, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{111, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{114, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{118, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{123, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{127, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{131, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{136, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{141, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{146, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{151, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{156, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{162, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{168, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{173, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{180, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{186, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{193, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{199, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{206, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{214, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{221, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{229, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{237, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{245, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{254, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{263, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{272, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{282, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{292, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{302, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{313, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{324, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{335, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{347, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{359, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{372, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{385, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{399, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{413, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{427, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{442, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{458, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{474, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{474, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.11*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.15*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.19*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.23*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.27*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.32*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.37*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.41*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.46*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.52*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.57*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.62*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.68*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.74*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.80*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.87*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(1.93*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.00*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.07*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.14*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.22*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.30*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.38*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.46*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.55*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.64*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.73*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.83*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(2.93*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.03*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.14*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.25*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.36*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.48*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.61*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.73*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(3.86*AR0330_MIN_D_GAIN*256)},
{474, (int)(4.0*256), (int)(4.00*AR0330_MIN_D_GAIN*256)}
};



static const  int ar0330_60fps_exp_time_gain_60Hz[AR0330_60FPS_60HZ_EXP_TIME_TOTAL][3] = 
{	// {time, analog gain, digital gain}
{8, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{9, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{10, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{11, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{12, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{13, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{14, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{15, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{16, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{17, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{18, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{19, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{20, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{21, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{22, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{23, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{24, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{25, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{26, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{27, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{28, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{29, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{30, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{31, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{33, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{34, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{35, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{36, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{37, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{39, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{40, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{42, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{43, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{44, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{46, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{48, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{49, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{51, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{53, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{55, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{57, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{59, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{61, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{63, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{65, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{67, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{70, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{72, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{75, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{77, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{80, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{83, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{86, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{89, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{92, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{95, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{99, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{102, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{106, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{110, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{113, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{117, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{122, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{126, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{130, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{135, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{140, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{145, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{150, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{155, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{160, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{166, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{172, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{178, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{184, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{191, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{197, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{204, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{212, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{219, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{227, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{235, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{243, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{252, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{261, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{270, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{279, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{289, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{299, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{310, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{321, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{332, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{344, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{356, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{369, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{382, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{395, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.00*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.03*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.07*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.10*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.14*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.19*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.23*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.28*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.33*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.39*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.45*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.52*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.60*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.68*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.78*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(1.88*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.13*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.13*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.29*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.29*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.46*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.46*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.67*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.67*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.67*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.91*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.91*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(2.91*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{789, (int)(3.20*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(3.20*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(3.20*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{789, (int)(3.56*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(3.56*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(3.56*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.04*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.07*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.11*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.15*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.19*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.23*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.27*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.32*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.37*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.41*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.46*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.52*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.57*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.62*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.68*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.74*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.80*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.87*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(1.93*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.00*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.07*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.14*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.22*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.30*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.38*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.46*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.55*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.64*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.73*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.83*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(2.93*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.03*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.14*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.25*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.36*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.48*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.61*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.73*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(3.86*AR0330_MIN_D_GAIN*256)},
{789, (int)(4.0*256), (int)(4.00*AR0330_MIN_D_GAIN*256)}
};

/**************************************************************************
 *             Sensor Setting Table								          *
 **************************************************************************/
//@@ MIPI-10bit_1280x720_60fps
const regval16_t AR0330_Mipi_Raw10_720P[] =
{
	{0x301A, 0x0058}, //RESET_REGISTER = 88
	{-2,-2},
	{0x3052, 0xA114},
	{0x304A, 0x0070},
	{-2,-2},	
	
	{0x302A, 0x0004},		//
	{0x302C, 0x0004},		//
	{0x302E, 0x0007},		//
	{0x3030, 0x00C7},		//
	{0x3036, 0x0008},		//
	{0x3038, 0x0001},		//
	{0x31AC, 0x0808},		//
	{0x31AE, 0x0201},		//		
	
	//MIPI Port Timing
	{0x31B0, 0x0049},		//
	{0x31B2, 0x001C},		//
	{0x31B4, 0x5F77},		//
	{0x31B6, 0x5299},		//
	{0x31B8, 0x408E},		//
	{0x31BA, 0x030C},		//
	{0x31BC, 0x800A},		//
	{0x31BE, 0x2003},		//
	
	//Step 2: PLL_settings	
	//Timing_settings
	{0x3002, 0x019E},		//Y_ADDR_START = 414
	{0x3004, 0x0206},		//X_ADDR_START = 518
	{0x3006, 0x046F},		//Y_ADDR_END = 1133 + 2
	{0x3008, 0x0705},		//X_ADDR_END = 1797
	{0x300A, 0x0316},		//FRAME_LENGTH_LINES = 788 + 2
	{0x300C, 0x03F4},		//LINE_LENGTH_PCK = 1014 - 2
	{0x3012, 0x0313},		//COARSE_INTEGRATION_TIME = 473
	{0x3014, 0x0000},		//FINE_INTEGRATION_TIME = 0	
	{0x30A2, 0x0001},		//X_ODD_INC = 1
	{0x30A6, 0x0001},		//Y_ODD_INC = 1	
	{0x3040, 0x0000},		//READ_MODE = 0
	{0x3042, 0x0000},		//EXTRA_DELAY = 521
	{0x30BA, 0x002C},		//DIGITAL_CTRL = 44	
	{0x3ED4, 0x8F6C},
	{0x3ED6, 0x66CC},

	{0x3088, 0x80BA},
	{0x3086, 0x0253},

	{0x308C, 0x0006}, //Y_ADDR_START_CB = 6
	{0x308A, 0x0006}, //X_ADDR_START_CB = 6
	{0x3090, 0x0605}, //Y_ADDR_END_CB = 1541
	{0x308E, 0x0905}, //X_ADDR_END_CB = 2309

	{0x30AA, 0x0687}, //FRAME_LENGTH_LINES_CB = 1671
	{0x303E, 0x04E0}, //LINE_LENGTH_PCK_CB = 1248
	{0x3016, 0x0686}, //COARSE_INTEGRATION_TIME_CB = 1670
	{0x3018, 0x0000}, //FINE_INTEGRATION_TIME_CB = 0
	{0x30AE, 0x0001}, //X_ODD_INC_CB = 1
	{0x30A8, 0x0001}, //Y_ODD_INC_CB = 1
	{0x3040, 0x0000}, //READ_MODE = 0
	{0x3042, 0x0000}, //EXTRA_DELAY = 0; //1183
	{0x30BA, 0x002C}, //DIGITAL_CTRL = 44

	{0x30CE, 0x0020}, // The maximum integration time can be limited to the frame time by setting R0x30CE[5] to 1.
	
	// Context A
	{0x3014, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3012, 10},//AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time
	
	// Context B
	{0x3018, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3016, 10},//AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time
	
	// Analog gain
	{0x3060, 0x100},	// analog gain = 1x

	// Digital gain
	{0x305E, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context A
	{0x30C4, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context B

	//Flip & Mirror
#if FLIP_MIRROR
	{0x3040, 0xC000},
#endif
	{0x30BA, 0x002C}, 	// DIGITAL_CTRL// BITFIELD=0x30BA,0x0040,0	// Digital_Ctrl_Adc_High_Speed	
	
	{0x30B0, 0x8000},  // use context A

	//Recommended Configuration
	{0x31E0, 0x0703},
	{0x3064, 0x1802},
	{0x3ED2, 0x0146},
	{0x3ED4, 0x8F6C},
	{0x3ED6, 0x66CC},
	{0x3ED8, 0x8C42},
	{0x3EDA, 0x88BC},
	{0x3EDC, 0xAA63},
	{0x305E, 0x00A0},
	{0x301A, 0x025C}, //Enable Streaming
	{-1,-1},
	{0, 0},			  //End

}; 
 
//@@ MIPI-10bit_1920x1080_30fps 
const regval16_t AR0330_Mipi_Raw10_1080P[] =
{
#if 0
	//[1920x1080 19fps one lane MIPI 10bit]
	//Step 1: RESET	
	{0x301A, 0x0058}, 	// RESET_REGISTER
	//delay = 20
	{-1,-1},
	{0x301A, 0x0058}, 	// RESET_REGISTER
	{-1,-1},
	//delay = 20
	//Step 2: PLL_settings	
	{0x302A, 0x0005}, 	// VT_PIX_CLK_DIV
	{0x302C, 0x0004}, 	// VT_SYS_CLK_DIV
	{0x302E, 0x0003}, 	// PRE_PLL_CLK_DIV
	{0x3030, 0x0050}, 	// PLL_MULTIPLIER
	{0x3036, 0x000A}, 	// OP_PIX_CLK_DIV
	{0x3038, 0x0001}, 	// OP_SYS_CLK_DIV
	{0x31AC, 0x0A0A}, 	// DATA_FORMAT_BITS
	{0x301A, 0x005C}, 	// RESET_REGISTER
	
	//Step 3:Timing_settings
	{0x31AE, 0x0201}, 	// SERIAL_FORMAT
	{0x3002, 0x00EA}, 	// Y_ADDR_START
	{0x3004, 0x00C6}, 	// X_ADDR_START
	{0x3006, 0x0521}, 	// Y_ADDR_END
	{0x3008, 0x0845}, 	// X_ADDR_END
	{0x300A, 0x054C}, 	// FRAME_LENGTH_LINES
	{0x300C, 0x04DA}, 	// LINE_LENGTH_PCK
	{0x3012, 0x054A}, 	// COARSE_INTEGRATION_TIME
	{0x3014, 0x0000}, 	// FINE_INTEGRATION_TIME
	{0x30A2, 0x0001}, 	// X_ODD_INC
	{0x30A6, 0x0001}, 	// Y_ODD_INC
	{0x308C, 0x00EA}, 	// Y_ADDR_START_CB
	{0x308A, 0x00C6}, 	// X_ADDR_START_CB
	{0x3090, 0x0521}, 	// Y_ADDR_END_CB
	{0x308E, 0x0845}, 	// X_ADDR_END_CB
	{0x30AA, 0x0730}, 	// FRAME_LENGTH_LINES_CB
	{0x303E, 0x04DA}, 	// LINE_LENGTH_PCK_CB
	{0x3016, 0x072E}, 	// COARSE_INTEGRATION_TIME_CB
	{0x3018, 0x0000}, 	// FINE_INTEGRATION_TIME_CB
	{0x30AE, 0x0001}, 	// X_ODD_INC_CB
	{0x30A8, 0x0001}, 	// Y_ODD_INC_CB
	{0x3040, 0x0000}, 	// READ_MODE
	{0x3042, 0x003A}, 	// EXTRA_DELAY
	{0x30BA, 0x002C}, 	// DIGITAL_CTRL
	{0x3088, 0x80BA}, 	// SEQ_CTRL_PORT
	{0x3086, 0xE653}, 	// SEQ_DATA_PORT
	//IMAGE=1920,1080,BAYER-10
	//delay = 10
	
	//Step 4: Sequencer			
	{0x301A, 0x0058}, 	// RESET_REGISTER
	{0x3088, 0x8000}, 	// SEQ_CTRL_PORT
	{0x3086, 0x4A03}, 	// SEQ_DATA_PORT
	{0x3086, 0x4316}, 	// SEQ_DATA_PORT
	{0x3086, 0x0443}, 	// SEQ_DATA_PORT
	{0x3086, 0x1645}, 	// SEQ_DATA_PORT
	{0x3086, 0x4045}, 	// SEQ_DATA_PORT
	{0x3086, 0x6017}, 	// SEQ_DATA_PORT
	{0x3086, 0x2045}, 	// SEQ_DATA_PORT
	{0x3086, 0x404B}, 	// SEQ_DATA_PORT
	{0x3086, 0x1244}, 	// SEQ_DATA_PORT
	{0x3086, 0x6134}, 	// SEQ_DATA_PORT
	{0x3086, 0x4A31}, 	// SEQ_DATA_PORT
	{0x3086, 0x4342}, 	// SEQ_DATA_PORT
	{0x3086, 0x4560}, 	// SEQ_DATA_PORT
	{0x3086, 0x2714}, 	// SEQ_DATA_PORT
	{0x3086, 0x3DFF}, 	// SEQ_DATA_PORT
	{0x3086, 0x3DFF}, 	// SEQ_DATA_PORT
	{0x3086, 0x3DEA}, 	// SEQ_DATA_PORT
	{0x3086, 0x2704}, 	// SEQ_DATA_PORT
	{0x3086, 0x3D10}, 	// SEQ_DATA_PORT
	{0x3086, 0x2705}, 	// SEQ_DATA_PORT
	{0x3086, 0x3D10}, 	// SEQ_DATA_PORT
	{0x3086, 0x2715}, 	// SEQ_DATA_PORT
	{0x3086, 0x3527}, 	// SEQ_DATA_PORT
	{0x3086, 0x053D}, 	// SEQ_DATA_PORT
	{0x3086, 0x1045}, 	// SEQ_DATA_PORT
	{0x3086, 0x4027}, 	// SEQ_DATA_PORT
	{0x3086, 0x0427}, 	// SEQ_DATA_PORT
	{0x3086, 0x143D}, 	// SEQ_DATA_PORT
	{0x3086, 0xFF3D}, 	// SEQ_DATA_PORT
	{0x3086, 0xFF3D}, 	// SEQ_DATA_PORT
	{0x3086, 0xEA62}, 	// SEQ_DATA_PORT
	{0x3086, 0x2728}, 	// SEQ_DATA_PORT
	{0x3086, 0x3627}, 	// SEQ_DATA_PORT
	{0x3086, 0x083D}, 	// SEQ_DATA_PORT
	{0x3086, 0x6444}, 	// SEQ_DATA_PORT
	{0x3086, 0x2C2C}, 	// SEQ_DATA_PORT
	{0x3086, 0x2C2C}, 	// SEQ_DATA_PORT
	{0x3086, 0x4B01}, 	// SEQ_DATA_PORT
	{0x3086, 0x432D}, 	// SEQ_DATA_PORT
	{0x3086, 0x4643}, 	// SEQ_DATA_PORT
	{0x3086, 0x1647}, 	// SEQ_DATA_PORT
	{0x3086, 0x435F}, 	// SEQ_DATA_PORT
	{0x3086, 0x4F50}, 	// SEQ_DATA_PORT
	{0x3086, 0x2604}, 	// SEQ_DATA_PORT
	{0x3086, 0x2684}, 	// SEQ_DATA_PORT
	{0x3086, 0x2027}, 	// SEQ_DATA_PORT
	{0x3086, 0xFC53}, 	// SEQ_DATA_PORT
	{0x3086, 0x0D5C}, 	// SEQ_DATA_PORT
	{0x3086, 0x0D57}, 	// SEQ_DATA_PORT
	{0x3086, 0x5417}, 	// SEQ_DATA_PORT
	{0x3086, 0x0955}, 	// SEQ_DATA_PORT
	{0x3086, 0x5649}, 	// SEQ_DATA_PORT
	{0x3086, 0x5307}, 	// SEQ_DATA_PORT
	{0x3086, 0x5302}, 	// SEQ_DATA_PORT
	{0x3086, 0x4D28}, 	// SEQ_DATA_PORT
	{0x3086, 0x6C4C}, 	// SEQ_DATA_PORT
	{0x3086, 0x0928}, 	// SEQ_DATA_PORT
	{0x3086, 0x2C28}, 	// SEQ_DATA_PORT
	{0x3086, 0x294E}, 	// SEQ_DATA_PORT
	{0x3086, 0x5C09}, 	// SEQ_DATA_PORT
	{0x3086, 0x6045}, 	// SEQ_DATA_PORT
	{0x3086, 0x0045}, 	// SEQ_DATA_PORT
	{0x3086, 0x8026}, 	// SEQ_DATA_PORT
	{0x3086, 0xA627}, 	// SEQ_DATA_PORT
	{0x3086, 0xF817}, 	// SEQ_DATA_PORT
	{0x3086, 0x0227}, 	// SEQ_DATA_PORT
	{0x3086, 0xFA5C}, 	// SEQ_DATA_PORT
	{0x3086, 0x0B17}, 	// SEQ_DATA_PORT
	{0x3086, 0x1826}, 	// SEQ_DATA_PORT
	{0x3086, 0xA25C}, 	// SEQ_DATA_PORT
	{0x3086, 0x0317}, 	// SEQ_DATA_PORT
	{0x3086, 0x4427}, 	// SEQ_DATA_PORT
	{0x3086, 0xF25F}, 	// SEQ_DATA_PORT
	{0x3086, 0x2809}, 	// SEQ_DATA_PORT
	{0x3086, 0x1714}, 	// SEQ_DATA_PORT
	{0x3086, 0x2808}, 	// SEQ_DATA_PORT
	{0x3086, 0x1701}, 	// SEQ_DATA_PORT
	{0x3086, 0x4D1A}, 	// SEQ_DATA_PORT
	{0x3086, 0x2683}, 	// SEQ_DATA_PORT
	{0x3086, 0x1701}, 	// SEQ_DATA_PORT
	{0x3086, 0x27FA}, 	// SEQ_DATA_PORT
	{0x3086, 0x45A0}, 	// SEQ_DATA_PORT
	{0x3086, 0x1707}, 	// SEQ_DATA_PORT
	{0x3086, 0x27FB}, 	// SEQ_DATA_PORT
	{0x3086, 0x1729}, 	// SEQ_DATA_PORT
	{0x3086, 0x4580}, 	// SEQ_DATA_PORT
	{0x3086, 0x1708}, 	// SEQ_DATA_PORT
	{0x3086, 0x27FA}, 	// SEQ_DATA_PORT
	{0x3086, 0x1728}, 	// SEQ_DATA_PORT
	{0x3086, 0x5D17}, 	// SEQ_DATA_PORT
	{0x3086, 0x0E26}, 	// SEQ_DATA_PORT
	{0x3086, 0x8153}, 	// SEQ_DATA_PORT
	{0x3086, 0x0117}, 	// SEQ_DATA_PORT
	{0x3086, 0xE653}, 	// SEQ_DATA_PORT
	{0x3086, 0x0217}, 	// SEQ_DATA_PORT
	{0x3086, 0x1026}, 	// SEQ_DATA_PORT
	{0x3086, 0x8326}, 	// SEQ_DATA_PORT
	{0x3086, 0x8248}, 	// SEQ_DATA_PORT
	{0x3086, 0x4D4E}, 	// SEQ_DATA_PORT
	{0x3086, 0x2809}, 	// SEQ_DATA_PORT
	{0x3086, 0x4C0B}, 	// SEQ_DATA_PORT
	{0x3086, 0x6017}, 	// SEQ_DATA_PORT
	{0x3086, 0x2027}, 	// SEQ_DATA_PORT
	{0x3086, 0xF217}, 	// SEQ_DATA_PORT
	{0x3086, 0x535F}, 	// SEQ_DATA_PORT
	{0x3086, 0x2808}, 	// SEQ_DATA_PORT
	{0x3086, 0x164D}, 	// SEQ_DATA_PORT
	{0x3086, 0x1A17}, 	// SEQ_DATA_PORT
	{0x3086, 0x0127}, 	// SEQ_DATA_PORT
	{0x3086, 0xFA26}, 	// SEQ_DATA_PORT
	{0x3086, 0x035C}, 	// SEQ_DATA_PORT
	{0x3086, 0x0145}, 	// SEQ_DATA_PORT
	{0x3086, 0x4027}, 	// SEQ_DATA_PORT
	{0x3086, 0x9817}, 	// SEQ_DATA_PORT
	{0x3086, 0x2A4A}, 	// SEQ_DATA_PORT
	{0x3086, 0x0A43}, 	// SEQ_DATA_PORT
	{0x3086, 0x160B}, 	// SEQ_DATA_PORT
	{0x3086, 0x4327}, 	// SEQ_DATA_PORT
	{0x3086, 0x9C45}, 	// SEQ_DATA_PORT
	{0x3086, 0x6017}, 	// SEQ_DATA_PORT
	{0x3086, 0x0727}, 	// SEQ_DATA_PORT
	{0x3086, 0x9D17}, 	// SEQ_DATA_PORT
	{0x3086, 0x2545}, 	// SEQ_DATA_PORT
	{0x3086, 0x4017}, 	// SEQ_DATA_PORT
	{0x3086, 0x0827}, 	// SEQ_DATA_PORT
	{0x3086, 0x985D}, 	// SEQ_DATA_PORT
	{0x3086, 0x2645}, 	// SEQ_DATA_PORT
	{0x3086, 0x4B17}, 	// SEQ_DATA_PORT
	{0x3086, 0x0A28}, 	// SEQ_DATA_PORT
	{0x3086, 0x0853}, 	// SEQ_DATA_PORT
	{0x3086, 0x0D52}, 	// SEQ_DATA_PORT
	{0x3086, 0x5112}, 	// SEQ_DATA_PORT
	{0x3086, 0x4460}, 	// SEQ_DATA_PORT
	{0x3086, 0x184A}, 	// SEQ_DATA_PORT
	{0x3086, 0x0343}, 	// SEQ_DATA_PORT
	{0x3086, 0x1604}, 	// SEQ_DATA_PORT
	{0x3086, 0x4316}, 	// SEQ_DATA_PORT
	{0x3086, 0x5843}, 	// SEQ_DATA_PORT
	{0x3086, 0x1659}, 	// SEQ_DATA_PORT
	{0x3086, 0x4316}, 	// SEQ_DATA_PORT
	{0x3086, 0x5A43}, 	// SEQ_DATA_PORT
	{0x3086, 0x165B}, 	// SEQ_DATA_PORT
	{0x3086, 0x4327}, 	// SEQ_DATA_PORT
	{0x3086, 0x9C45}, 	// SEQ_DATA_PORT
	{0x3086, 0x6017}, 	// SEQ_DATA_PORT
	{0x3086, 0x0727}, 	// SEQ_DATA_PORT
	{0x3086, 0x9D17}, 	// SEQ_DATA_PORT
	{0x3086, 0x2545}, 	// SEQ_DATA_PORT
	{0x3086, 0x4017}, 	// SEQ_DATA_PORT
	{0x3086, 0x1027}, 	// SEQ_DATA_PORT
	{0x3086, 0x9817}, 	// SEQ_DATA_PORT
	{0x3086, 0x2022}, 	// SEQ_DATA_PORT
	{0x3086, 0x4B12}, 	// SEQ_DATA_PORT
	{0x3086, 0x442C}, 	// SEQ_DATA_PORT
	{0x3086, 0x2C2C}, 	// SEQ_DATA_PORT
	{0x3086, 0x2C00}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	{0x3086, 0x0000}, 	// SEQ_DATA_PORT
	//delay = 20
	{-1,-1},
	{0x301A, 0x005C}, 	// RESET_REGISTER
	{-1,-1},			//delay = 100
	{0, 0},
#else

#if AR0330_FULLHD

	//[1080p@27.7fps 10bit - MIPI - 64 MPixels - 1lane MIPI]
	{0x301A, 0x0059},	// RESET_REGISTER // BITFIELD=0x301A,0x0001,1	//Reset Sensor
	{-1,-1},			// delay 100
	{0x31AE, 0x0201}, 	// SERIAL_FORMAT  // REG=0x31AE,0x201			//MIPI 1 lane Configured
	{0x301A, 0x0058},	// RESET_REGISTER // BITFIELD=0x301A,0x0004,0 	//Disable Streaming
	{-1,-1},			// delay 100
	{0x301A, 0x0058},	// RESET_REGISTER //BITFIELD=0x301A,0x0040,1 	//Drive Pins
	{0x301A, 0x0058},	// RESET_REGISTER //BITFIELD=0x301A,0x0080,0 	//Parallel Enable
	{0x301A, 0x0058},	// RESET_REGISTER //BITFIELD=0x301A,0x1000,0 	//SMIA Serializer Disable
	{-1,-1},//add by ytliao 2014
	{0x3064, 0x1802}, 	// SMIA_TEST
	{0x31E0, 0x0003}, 	// PIX_DEF_ID
	
	{0x301A, 0x0058}, 	// RESET_REGISTER
	{-1,-1},//add by ytliao 2014
	{0x31AE, 0x0201}, 	// SERIAL_FORMAT
	
	// LOAD=MIPI Timing - 10bit 640Mbps
	{0x31AC, 0x0A0A}, 	// DATA_FORMAT_BITS
	{0x31B0, 0x0043}, 	// FRAME_PREAMBLE
	{0x31B2, 0x0028}, 	// LINE_PREAMBLE
	{0x31B4, 0x1A54}, 	// MIPI_TIMING_0
	{0x31B6, 0x21D2}, 	// MIPI_TIMING_1
	{0x31B8, 0x284A}, 	// MIPI_TIMING_2
	{0x31BA, 0x0208}, 	// MIPI_TIMING_3
	{0x31BC, 0x8005}, 	// MIPI_TIMING_4
	{0x31BE, 0x2003}, 	// MIPI_CONFIG_STATUS
	
	//STATE= Master Clock, 64000000 	//64 MHz
	{0x3036, 0x000A}, 	// OP_PIX_CLK_DIV 			// REG=0x3036, 10				// op_pix_clk_div 
	{0x3038, 0x0001}, 	// OP_SYS_CLK_DIV 			// REG=0x3038, 1				// op_sys_clk_div
	
	
	
	
	//ARRAY READOUT SETTINGS
	#if AR0330_FULLHD
	// 1920x1082
	// original
	{0x302A, 0x0005}, 	// VT_PIX_CLK_DIV 			// REG=0x302A, 5				// vt_pix_clk_div
	{0x302C, 0x0004}, 	// VT_SYS_CLK_DIV 			// REG=0x302C, 4				// vt_sys_clk_div
	{0x302E, 3},		// PRE_PLL_CLK_DIV			// REG=0x302E, 2		      	// pre_pll_clk_div
	
	// modified by Sam for 27MHz 2014.05.08 
	{0x3030, 77},		// PLL_MULTIPLIER 			// REG=0x3030, 64				// pll_multiplier

	
	{0x31AC, 0x0A0A},	// data_format is 10-bit
	
	{0x3004, 0x00C0}, 	// X_ADDR_START			// REG=0x3004, 192		// X_ADDR_START
	{0x3008, 0x083F}, 	// X_ADDR_END  			// REG=0x3008, 2111		// X_ADDR_END
	{0x3002, 0x00E8}, 	// Y_ADDR_START			// REG=0x3002, 232    	// Y_ADDR_START
	{0x3006, 0x0521}, 	// Y_ADDR_END  			// REG=0x3006, 1313		// Y_ADDR_END

	//Frame-Timing
	{0x300C, 0x41E},		// LINE_LENGTH_PCK    for Context A    		
	{0x303E, 0x41E},		// LINE_LENGTH_PCK    for Context B
	
	{0x300A, 0x0446},	// FRAME_LENGTH_LINES  for Context A
	{0x30AA, 0x0446},	// FRAME_LENGTH_LINES  for Context B

	{0x3042, 0x00},	// EXTRA_DELAY            		// REG=0x3042, 0			// EXTRA_DELAY
/*
	// comi modified 20140627
	{0x302A, 0x0006}, 	// VT_PIX_CLK_DIV 			// REG=0x302A, 5				// vt_pix_clk_div
	{0x302C, 0x0002}, 	// VT_SYS_CLK_DIV 			// REG=0x302C, 4				// vt_sys_clk_div
	{0x302E, 4},		// PRE_PLL_CLK_DIV			// REG=0x302E, 2		      	// pre_pll_clk_div
	
	// modified by Sam for 27MHz 2014.05.08 
	{0x3030, 73},		// PLL_MULTIPLIER 			// REG=0x3030, 64				// pll_multiplier

	
	{0x31AC, 0x0A0A},	// data_format is 10-bit
	
	{0x3004, 0x00C0}, 	// X_ADDR_START			// REG=0x3004, 192		// X_ADDR_START
	{0x3008, 0x083F}, 	// X_ADDR_END  			// REG=0x3008, 2111		// X_ADDR_END
	{0x3002, 0x00E8}, 	// Y_ADDR_START			// REG=0x3002, 232    	// Y_ADDR_START
	{0x3006, 0x0521}, 	// Y_ADDR_END  			// REG=0x3006, 1313		// Y_ADDR_END

	//Frame-Timing
	{0x300C, 1248},		// LINE_LENGTH_PCK    for Context A    		
	{0x303E, 1248},		// LINE_LENGTH_PCK    for Context B
	
	{0x300A, 1094},	// FRAME_LENGTH_LINES  for Context A
	{0x30AA, 1094},	// FRAME_LENGTH_LINES  for Context B

	{0x3042, 0},	// EXTRA_DELAY            		// REG=0x3042, 0			// EXTRA_DELAY
	*/
	#else
	#if 0
	// 2304x1296

	{0x302A, 0x0005}, 	// VT_PIX_CLK_DIV 			// REG=0x302A, 5				// vt_pix_clk_div
	{0x302C, 0x0002}, 	// VT_SYS_CLK_DIV 			// REG=0x302C, 4				// vt_sys_clk_div
	{0x302E, 4},		// PRE_PLL_CLK_DIV			// REG=0x302E, 2		      	// pre_pll_clk_div
	{0x3030, 73},		// PLL_MULTIPLIER 			// REG=0x3030, 64				// pll_multiplier

	
	{0x31AC, 0x0808},		// data_format is 8-bit
	
	{0x3004, 0x0006}, 	// X_ADDR_START
	{0x3008, 0x0905}, 	// X_ADDR_END  
	{0x3002, 0x007C}, 	// Y_ADDR_START
	{0x3006, 0x058B}, 	// Y_ADDR_END 
	
	{0x300A, 1316},		// FRAME_LENGTH_LINES 
	{0x300C, 1248},		// LINE_LENGTH_PCK

	{0x3042, 132},	// EXTRA_DELAY
	#else
	// 2130x1200
	{0x302A, 0x0005}, 	// VT_PIX_CLK_DIV
	{0x302C, 0x0003}, 	// VT_SYS_CLK_DIV
	{0x302E, 4},		// PRE_PLL_CLK_DIV		
	{0x3030, 94},		// PLL_MULTIPLIER 		

	
	{0x31AC, 0x0a0a},		// data_format is 10-bit
	
	{0x3004, 0x0030}, 	// X_ADDR_START
	{0x3008, 0x881}, 	// X_ADDR_END  
	{0x3002, 0x0090}, 	// Y_ADDR_START
	{0x3006, 0x53F}, 	// Y_ADDR_END 
	
	{0x300A, 1212},		// FRAME_LENGTH_LINES 
	{0x300C, 1162},		// LINE_LENGTH_PCK

	{0x3042, 1656},	// EXTRA_DELAY
	#endif

	
	#endif
	
	//Sub-sampling
	{0x30A2, 0x0001}, 	// X_ODD_INC			// REG=0x30A2,1			// X_ODD_INCREMENT
	{0x30A6, 0x0001}, 	// Y_ODD_INC			// REG=0x30A6,1			// Y_ODD_INCREMENT
	{0x3040, 0x0000}, 	// READ_MODE			// BITFIELD=0x3040,0x1000,0 	// Row Bin
	{0x3040, 0x0000}, 	// READ_MODE			// BITFIELD=0x3040,0x2000,0 	// Column Bin
	{0x3040, 0x0000}, 	// READ_MODE			// BITFIELD=0x3040,0x0200,0 	// Column SF Bin

	
	{0x30CE, 0x0020}, // The maximum integration time can be limited to the frame time by setting R0x30CE[5] to 1.
	
	// Context A
	{0x3014, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3012, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time
	
	// Context B
	{0x3018, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3016, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time

	
	
	// Analog gain
	{0x3060, 0x0},	// analog gain = 1x

	// Digital gain
	{0x305E, 0x0080}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context A
	{0x30C4, 0x0080}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context B

	//Flip & Mirror
#if	FLIP_MIRROR
	{0x3040, 0xC000},
#endif
	{0x30BA, 0x002C}, 	// DIGITAL_CTRL// BITFIELD=0x30BA,0x0040,0	// Digital_Ctrl_Adc_High_Speed	


	{0x30B0, 0x8000},  // use context A

	//IMAGE=1920,1080,BAYER-10
	//STATE= True Black Level, 42
	//STATE= Auto Exposure Minimum FPS, 30

  	{0x301A, 0x0058}, 	// RESET_REGISTER // BITFIELD=0x301A,0x0004,0  //Disable Streaming
	{-1, -1},			// delay
	{0x3088, 0x8000},
	{0x3086, 0x4A03},
	{0x3086, 0x4316},
	{0x3086, 0x0443},
	{0x3086, 0x1645},
	{0x3086, 0x4045},
	{0x3086, 0x6017},
	{0x3086, 0x2045}, 	//June 9th seq changed to 0x2045 from 0x5045
	{0x3086, 0x404B},
	{0x3086, 0x1244},
	{0x3086, 0x6134},
	{0x3086, 0x4A31},
	{0x3086, 0x4342},
	{0x3086, 0x4560},
	{0x3086, 0x2714},
	{0x3086, 0x3DFF},
	{0x3086, 0x3DFF},
	{0x3086, 0x3DEA},
	{0x3086, 0x2704},
	{0x3086, 0x3D10},
	{0x3086, 0x2705},
	{0x3086, 0x3D10},
	{0x3086, 0x2715},
	{0x3086, 0x3527},
	{0x3086, 0x053D},
	{0x3086, 0x1045},
	{0x3086, 0x4027},
	{0x3086, 0x0427},
	{0x3086, 0x143D},
	{0x3086, 0xFF3D},
	{0x3086, 0xFF3D},
	{0x3086, 0xEA62},
	{0x3086, 0x2728},
	{0x3086, 0x3627},
	{0x3086, 0x083D},
	{0x3086, 0x6444},
	{0x3086, 0x2C2C},
	{0x3086, 0x2C2C},
	{0x3086, 0x4B01},
	{0x3086, 0x432D},
	{0x3086, 0x4643},
	{0x3086, 0x1647},
	{0x3086, 0x435F},
	{0x3086, 0x4F50},
	{0x3086, 0x2604},
	{0x3086, 0x2684},
	{0x3086, 0x2027},
	{0x3086, 0xFC53},
	{0x3086, 0x0D5C},
	{0x3086, 0x0D57},
	{0x3086, 0x5417},
	{0x3086, 0x0955},
	{0x3086, 0x5649},
	{0x3086, 0x5307},
	{0x3086, 0x5302},
	{0x3086, 0x4D28},
	{0x3086, 0x6C4C},
	{0x3086, 0x0928},
	{0x3086, 0x2C28},
	{0x3086, 0x294E},
	{0x3086, 0x5C09},
	{0x3086, 0x6045},
	{0x3086, 0x0045},
	{0x3086, 0x8026},
	{0x3086, 0xA627},
	{0x3086, 0xF817},
	{0x3086, 0x0227},
	{0x3086, 0xFA5C},
	{0x3086, 0x0B17},
	{0x3086, 0x1826},
	{0x3086, 0xA25C},
	{0x3086, 0x0317},
	{0x3086, 0x4427},
	{0x3086, 0xF25F},
	{0x3086, 0x2809},
	{0x3086, 0x1714},
	{0x3086, 0x2808},
	{0x3086, 0x1701},
	{0x3086, 0x4D1A},
	{0x3086, 0x2683},
	{0x3086, 0x1701},
	{0x3086, 0x27FA},
	{0x3086, 0x45A0},
	{0x3086, 0x1707},
	{0x3086, 0x27FB},
	{0x3086, 0x1729},
	{0x3086, 0x4580},
	{0x3086, 0x1708},
	{0x3086, 0x27FA},
	{0x3086, 0x1728},
	{0x3086, 0x5D17},
	{0x3086, 0x0E26},
	{0x3086, 0x8153},
	{0x3086, 0x0117},
	{0x3086, 0xE653},
	{0x3086, 0x0217},
	{0x3086, 0x1026},
	{0x3086, 0x8326},
	{0x3086, 0x8248},
	{0x3086, 0x4D4E},
	{0x3086, 0x2809},
	{0x3086, 0x4C0B},
	{0x3086, 0x6017},
	{0x3086, 0x2027},
	{0x3086, 0xF217},
	{0x3086, 0x535F},
	{0x3086, 0x2808},
	{0x3086, 0x164D},
	{0x3086, 0x1A17},
	{0x3086, 0x0127},
	{0x3086, 0xFA26},
	{0x3086, 0x035C},
	{0x3086, 0x0145},
	{0x3086, 0x4027},
	{0x3086, 0x9817},
	{0x3086, 0x2A4A},
	{0x3086, 0x0A43},
	{0x3086, 0x160B},
	{0x3086, 0x4327},
	{0x3086, 0x9C45},
	{0x3086, 0x6017},
	{0x3086, 0x0727},
	{0x3086, 0x9D17},
	{0x3086, 0x2545},
	{0x3086, 0x4017},
	{0x3086, 0x0827},
	{0x3086, 0x985D},
	{0x3086, 0x2645},
	{0x3086, 0x4B17},
	{0x3086, 0x0A28},
	{0x3086, 0x0853},
	{0x3086, 0x0D52},
	{0x3086, 0x5112},
	{0x3086, 0x4460},
	{0x3086, 0x184A},
	{0x3086, 0x0343},
	{0x3086, 0x1604},
	{0x3086, 0x4316},
	{0x3086, 0x5843},
	{0x3086, 0x1659},
	{0x3086, 0x4316},
	{0x3086, 0x5A43},
	{0x3086, 0x165B},
	{0x3086, 0x4327},
	{0x3086, 0x9C45},
	{0x3086, 0x6017},
	{0x3086, 0x0727},
	{0x3086, 0x9D17},
	{0x3086, 0x2545},
	{0x3086, 0x4017},
	{0x3086, 0x1027},
	{0x3086, 0x9817},
	{0x3086, 0x2022},
	{0x3086, 0x4B12},
	{0x3086, 0x442C},
	{0x3086, 0x2C2C},
	{0x3086, 0x2C00},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	{0x3086, 0x0000},
	
	// [Sequencer Normal Length]
	{0x301A, 0x0078}, 	// RESET_REGISTER
	{-1,-1},//add by ytliao 2014
	{0x3088, 0x80BA}, 	// SEQ_CTRL_PORT
	{0x3086, 0x0253},	//0xE653 	// SEQ_DATA_PORT using short sequencer to increase frame rate
	{0x301A, 0x005C}, 	// RESET_REGISTER // BITFIELD=0x301A,0x0004,1  //Enable Streaming
	
	//[MIPI Timing - 12bit 588Mbps]
	//{0x31AC, 0xC0C},		// DATA FORMAT BITS
	//{0x31B0, 36},    		// FRAME PREAMBLE
	//{0x31B2, 12},			// LINE PREAMBLE
	//{0x31B4, 0x2643},   	// MIPI TIMING 0
	//{0x31B6, 0x114E},		// MIPI TIMING 1
	//{0x31B8, 0x2048},    	// MIPI TIMING 2
	//{0x31BA, 0x0186},		// MIPI TIMING 3
	//{0x31BC, 0x8005},    	// MIPI TIMING 4
	//{0x31BE, 0x2003},		// MIPI CONFIG STATUS
	{-1,-1},				// delay
	{0, 0}
	#else

	// 2304x1296
	{0x301A, 0x0059}, //RESET_REGISTER = 88
	{-2,-2},
	{0x3052, 0xA114},
	{0x304A, 0x0070},
	{-2,-2},
	{0x31AE, 0x0201}, //SERIAL_FORMAT = 513
	{0x301A, 0x0058}, 
	{-3,-3},

	{0x302A, 0x0004}, //VT_PIX_CLK_DIV = 4
	{0x302C, 0x0004}, //VT_SYS_CLK_DIV = 4
	{0x302E, 0x0007}, //PRE_PLL_CLK_DIV = 7
	//{0x3030, 0x00C7}, //PLL_MULTIPLIER = 199
	{0x3030, 0x00CC}, //PLL_MULTIPLIER = 204
	{0x3036, 0x0008}, //OP_PIX_CLK_DIV = 8
	{0x3038, 0x0001}, //OP_SYS_CLK_DIV = 1
	{0x31AC, 0x0808}, //DATA_FORMAT_BITS = 2056
	
	

	//MIPI Port Timing
	{0x31B0, 0x0049}, //FRAME_PREAMBLE = 73
	{0x31B2, 0x001C}, //LINE_PREAMBLE = 28
	{0x31B4, 0x5F77}, //MIPI_TIMING_0 = 24439
	{0x31B6, 0x5299}, //MIPI_TIMING_1 = 21145
	{0x31B8, 0x408E}, //MIPI_TIMING_2 = 16526
	{0x31BA, 0x030C}, //MIPI_TIMING_3 = 780
	{0x31BC, 0x800A}, //MIPI_TIMING_4 = 32778
	{0x31BE, 0x2003},	 // MIPI CONFIG STATUS


	//Timing_settings

	// CA
	{0x3002, 0x007E}, //Y_ADDR_START = 126
	{0x3004, 0x0006}, //X_ADDR_START = 6
	{0x3006, 0x058D}, //Y_ADDR_END = 1421
	{0x3008, 0x0905}, //X_ADDR_END = 2309
	{0x300A, 0x051F}, //FRAME_LENGTH_LINES = 1311(30fps)
	//{0x300A, 0x0629}, //FRAME_LENGTH_LINES = 1577 (25fps)
	{0x300C, 0x04E0}, //LINE_LENGTH_PCK = 1248
	{0x3012, 0x0501}, //COARSE_INTEGRATION_TIME = 1281
	{0x3014, 0x0000}, //FINE_INTEGRATION_TIME = 0
	{0x30A2, 0x0001}, //X_ODD_INC = 1
	{0x30A6, 0x0001}, //Y_ODD_INC = 1

	//CB
	//{0x308C, 0x0006}, //Y_ADDR_START_CB = 6
	{0x308C, 0x007E}, //Y_ADDR_START_CB = 126
	{0x308A, 0x0006}, //X_ADDR_START_CB = 6
	//{0x3090, 0x0605}, //Y_ADDR_END_CB = 1541
	{0x3090, 0x058D}, //Y_ADDR_END_CB = 1421
	{0x308E, 0x0905}, //X_ADDR_END_CB = 2309

	/*
	{0x30AA, 0x0687}, //FRAME_LENGTH_LINES_CB = 1671	
	{0x3016, 0x0686}, //COARSE_INTEGRATION_TIME_CB = 1670*/
	{0x30AA, 0x051F}, //FRAME_LENGTH_LINES_CB = 1311	
	{0x3016, 0x0501}, //COARSE_INTEGRATION_TIME_CB = 1281
	{0x303E, 0x04E0}, //LINE_LENGTH_PCK_CB = 1248
	{0x3018, 0x0000}, //FINE_INTEGRATION_TIME_CB = 0
	{0x30AE, 0x0001}, //X_ODD_INC_CB = 1
	{0x30A8, 0x0001}, //Y_ODD_INC_CB = 1

	
	{0x3040, 0x0000}, //READ_MODE = 0
	{0x3042, 0x0000}, //EXTRA_DELAY = 0; //1183
	{0x30BA, 0x002C}, //DIGITAL_CTRL = 44


	//{0x30CE, 0x0020}, // The maximum integration time can be limited to the frame time by setting R0x30CE[5] to 1.
	{0x30CE, 0x0000}, 
	
	// Context A
	{0x3014, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3012, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time
	
	
	// Context B
	{0x3018, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3016, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time

	
	
	// Analog gain
	{0x3060, 0x100},	// analog gain = 1x

	// Digital gain
	{0x305E, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context A
	{0x30C4, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context B

	//Flip & Mirror
#if FLIP_MIRROR
	{0x3040, 0xC000},
#endif
	{0x30BA, 0x002C}, 	// DIGITAL_CTRL// BITFIELD=0x30BA,0x0040,0	// Digital_Ctrl_Adc_High_Speed	


	{0x30B0, 0x8000},  // use context A

	// r, g, b gain
	//CA
	{0x3056, 0x80}, // green1 gain
	{0x3058, 0x80}, // blue gain
	{0x305A, 0x80}, // red gain
	{0x305C, 0x80}, // green2 gain

	//CB
	{0x30BC, 0x80}, // green1 gain
	{0x30BE, 0x80}, // blue gain
	{0x30C0, 0x80}, // red gain
	{0x30C2, 0x80}, // green2 gain


	//Recommended Configuration
	{0x3064, 0x1802},
	{0x3078, 0x0001},
	{0x31E0, 0x0703},
	//{0x31E0, 0x0203}, // Disable on chip noise correction
	{0x3ED2, 0x0146},
	{0x3ED4, 0x8F6C},
	{0x3ED6, 0x66CC},
	{0x3ED8, 0x8C42},
	{0x3EDA, 0x88BC},
	{0x3EDC, 0xAA63},
	{0x305E, 0x00A0},

	{0x3088, 0x80BA}, 	// SEQ_CTRL_PORT
	{0x3086, 0xE653},	


	{0x301A, 0x025C}, //Enable Streaming
	{-1,-1},
	{0, 0},			  //End
	
#endif
#endif             
};  

const regval16_t AR0330_Mipi_Raw10_1080P_25fps[] = 
{
	// 2240x1260
	{0x301A, 0x0059}, //RESET_REGISTER = 88
	{-2,-2},
	{0x3052, 0xA114},
	{0x304A, 0x0070},
	{-2,-2},
	{0x31AE, 0x0201}, //SERIAL_FORMAT = 513
	{0x301A, 0x0058}, 
	{-3,-3},

	{0x301A, 0x0018}, 
	
	{0x302A, 0x0004}, //VT_PIX_CLK_DIV = 4
	{0x302C, 0x0004}, //VT_SYS_CLK_DIV = 4
	{0x302E, 0x0005}, //PRE_PLL_CLK_DIV = 5
	{0x3030, 0x008E}, //PLL_MULTIPLIER = 142
	{0x3036, 0x0008}, //OP_PIX_CLK_DIV = 8
	{0x3038, 0x0001}, //OP_SYS_CLK_DIV = 1
	{0x31AC, 0x0808}, //DATA_FORMAT_BITS = 2056
	
	

	//MIPI Port Timing
	{0x31B0, 0x0049}, //FRAME_PREAMBLE = 73
	{0x31B2, 0x001C}, //LINE_PREAMBLE = 28
	{0x31B4, 0x5F77}, //MIPI_TIMING_0 = 24439
	{0x31B6, 0x5299}, //MIPI_TIMING_1 = 21145
	{0x31B8, 0x408E}, //MIPI_TIMING_2 = 16526
	{0x31BA, 0x030C}, //MIPI_TIMING_3 = 780
	{0x31BC, 0x800A}, //MIPI_TIMING_4 = 32778
	{0x31BE, 0x2003},	 // MIPI CONFIG STATUS

	{0x301A, 0x0058},
	
	//Timing_settings

	// CA
	{0x3002, 0x0090}, //Y_ADDR_START = 144
	{0x3004, 0x0026}, //X_ADDR_START = 38
	{0x3006, 0x057B}, //Y_ADDR_END = 1403
	{0x3008, 0x08E5}, //X_ADDR_END = 2277
	{0x300A, 0x0507}, //FRAME_LENGTH_LINES = 1287(30fps)
	//{0x300A, 0x0607}, //FRAME_LENGTH_LINES = 1543 (25fps)
	{0x300C, 0x04DA}, //LINE_LENGTH_PCK = 1242
	{0x3012, 0x0100}, //COARSE_INTEGRATION_TIME = 1281
	{0x3014, 0x0000}, //FINE_INTEGRATION_TIME = 0
	{0x30A2, 0x0001}, //X_ODD_INC = 1
	{0x30A6, 0x0001}, //Y_ODD_INC = 1

	//CB
	{0x308C, 0x0090}, //Y_ADDR_START_CB = 144
	{0x308A, 0x0026}, //X_ADDR_START_CB = 38
	{0x3090, 0x057B}, //Y_ADDR_END_CB = 1403
	{0x308E, 0x08E5}, //X_ADDR_END_CB = 2277

	{0x30AA, 0x0507}, //FRAME_LENGTH_LINES_CB = 1287	
	{0x3016, 0x0501}, //COARSE_INTEGRATION_TIME_CB = 1281
	{0x303E, 0x04DA}, //LINE_LENGTH_PCK_CB = 1242
	{0x3018, 0x0000}, //FINE_INTEGRATION_TIME_CB = 0
	{0x30AE, 0x0001}, //X_ODD_INC_CB = 1
	{0x30A8, 0x0001}, //Y_ODD_INC_CB = 1

	
	{0x3040, 0x0000}, //READ_MODE = 0
	{0x3042, 0x0000}, //EXTRA_DELAY = 0; //1183
	{0x30BA, 0x002C}, //DIGITAL_CTRL = 44


	//{0x30CE, 0x0020}, // The maximum integration time can be limited to the frame time by setting R0x30CE[5] to 1.
	{0x30CE, 0x0000}, 
	
	// Context A
	{0x3014, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3012, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time
	
	
	// Context B
	{0x3018, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3016, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time

	
	
	// Analog gain
	{0x3060, 0x100},	// analog gain = 1x

	// Digital gain
	{0x305E, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context A
	{0x30C4, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context B

	//Flip & Mirror
#if FLIP_MIRROR
	{0x3040, 0xC000},
#endif
	{0x30BA, 0x002C}, 	// DIGITAL_CTRL// BITFIELD=0x30BA,0x0040,0	// Digital_Ctrl_Adc_High_Speed	


	{0x30B0, 0x8000},  // use context A

	// r, g, b gain
	//CA
	{0x3056, 0x80}, // green1 gain
	{0x3058, 0x80}, // blue gain
	{0x305A, 0x80}, // red gain
	{0x305C, 0x80}, // green2 gain

	//CB
	{0x30BC, 0x80}, // green1 gain
	{0x30BE, 0x80}, // blue gain
	{0x30C0, 0x80}, // red gain
	{0x30C2, 0x80}, // green2 gain


	//Recommended Configuration
	{0x3064, 0x1802},
	{0x3078, 0x0001},
	{0x31E0, 0x0703},
	//{0x31E0, 0x0203}, // Disable on chip noise correction
	{0x3ED2, 0x0146},
	{0x3ED4, 0x8F6C},
	{0x3ED6, 0x66CC},
	{0x3ED8, 0x8C42},
	{0x3EDA, 0x88BC},
	{0x3EDC, 0xAA63},
	{0x305E, 0x00A0},

	{0x3088, 0x80BA}, 	// SEQ_CTRL_PORT
	{0x3086, 0xE653},	


	{0x301A, 0x025C}, //Enable Streaming
	{-1,-1},
	{0, 0},			  //End
};

const regval16_t AR0330_Mipi_Raw10_1080P_LDW[] =
{	
	{0x301A, 0x0059}, //RESET_REGISTER = 88
	{-2,-2},
	{0x3052, 0xA114},
	{0x304A, 0x0070},
	{-2,-2},
	{0x31AE, 0x0201}, //SERIAL_FORMAT = 513
	{0x301A, 0x0058}, 
	{-3,-3},
	
	{0x302A, 0x0004}, //VT_PIX_CLK_DIV = 4
	{0x302C, 0x0004}, //VT_SYS_CLK_DIV = 4
	{0x302E, 0x0007}, //PRE_PLL_CLK_DIV = 7
	//{0x3030, 0x00C7}, //PLL_MULTIPLIER = 199
	{0x3030, 0x00AA}, //PLL_MULTIPLIER = 204
	{0x3036, 0x0008}, //OP_PIX_CLK_DIV = 8
	{0x3038, 0x0001}, //OP_SYS_CLK_DIV = 1
	{0x31AC, 0x0808}, //DATA_FORMAT_BITS = 2056
	{0x31AE, 0x0201}, //SERIAL_FORMAT = 513

	//MIPI Port Timing
	{0x31B0, 0x0049}, //FRAME_PREAMBLE = 73
	{0x31B2, 0x001C}, //LINE_PREAMBLE = 28
	{0x31B4, 0x5F77}, //MIPI_TIMING_0 = 24439
	{0x31B6, 0x5299}, //MIPI_TIMING_1 = 21145
	{0x31B8, 0x408E}, //MIPI_TIMING_2 = 16526
	{0x31BA, 0x030C}, //MIPI_TIMING_3 = 780
	{0x31BC, 0x800A}, //MIPI_TIMING_4 = 32778


	//Timing_settings
	{0x3002, 0x007E}, //Y_ADDR_START = 126
	{0x3004, 0x0006}, //X_ADDR_START = 6
	{0x3006, 0x058D}, //Y_ADDR_END = 1421
	{0x3008, 0x0905}, //X_ADDR_END = 2309
	{0x300A, 0x051F}, //FRAME_LENGTH_LINES = 1311
	{0x300C, 0x04E0}, //LINE_LENGTH_PCK = 1248
	{0x3012, 0x0501}, //COARSE_INTEGRATION_TIME = 1281
	{0x3014, 0x0000}, //FINE_INTEGRATION_TIME = 0
	{0x30A2, 0x0001}, //X_ODD_INC = 1
	{0x30A6, 0x0001}, //Y_ODD_INC = 1
	{0x308C, 0x0006}, //Y_ADDR_START_CB = 6
	{0x308A, 0x0006}, //X_ADDR_START_CB = 6
	{0x3090, 0x0605}, //Y_ADDR_END_CB = 1541
	{0x308E, 0x0905}, //X_ADDR_END_CB = 2309

	
	{0x30AA, 0x0687}, //FRAME_LENGTH_LINES_CB = 1671
	{0x303E, 0x04E0}, //LINE_LENGTH_PCK_CB = 1248
	{0x3016, 0x0686}, //COARSE_INTEGRATION_TIME_CB = 1670
	{0x3018, 0x0000}, //FINE_INTEGRATION_TIME_CB = 0
	{0x30AE, 0x0001}, //X_ODD_INC_CB = 1
	{0x30A8, 0x0001}, //Y_ODD_INC_CB = 1
	{0x3040, 0x0000}, //READ_MODE = 0
	{0x3042, 0x0000}, //EXTRA_DELAY = 0; //1183
	{0x30BA, 0x002C}, //DIGITAL_CTRL = 44


	{0x30CE, 0x0020}, // The maximum integration time can be limited to the frame time by setting R0x30CE[5] to 1.
	
	// Context A
	{0x3014, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3012, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time
	
	
	// Context B
	{0x3018, 0x0000},	// FINE_INTEGRATION_TIME  		// REG=0x3014, 0			// FINE_INTEGRATION_TIME
	{0x3016, AR0330_MAX_EXPOSURE_TIME},	// COARSE_INTEGRATION_TIME		// REG=0x3012, 1092			// Coarse_Integration_Time

	
	
	// Analog gain
	{0x3060, 0x100},	// analog gain = 1x

	// Digital gain
	{0x305E, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context A
	{0x30C4, 0x80}, 	// Digital GLOBAL_GAIN, max = 0x7ff, default = 0x80, Context B

	//Flip & Mirror
#if FLIP_MIRROR
	{0x3040, 0xC000},
#endif
	{0x30BA, 0x002C}, 	// DIGITAL_CTRL// BITFIELD=0x30BA,0x0040,0	// Digital_Ctrl_Adc_High_Speed	


	{0x30B0, 0x8000},  // use context A



	//Recommended Configuration
	{0x31E0, 0x0703},
	{0x3064, 0x1802},
	{0x3ED2, 0x0146},
	{0x3ED4, 0x8F6C},
	{0x3ED6, 0x66CC},
	{0x3ED8, 0x8C42},
	{0x3EDA, 0x88BC},
	{0x3EDC, 0xAA63},
	{0x305E, 0x00A0},
	{0x301A, 0x025C}, //Enable Streaming
	{-1,-1},
	{0, 0},			  //End            
};  


static int ob_cnt = 0;
    
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
	g_ti2c_handle.pDeviceString = "AR0330_MIPI";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("AR0330_MIPI ti2c request failed\n");
		return -1;
	}
	//gp_ti2c_bus_int_mode_en(0);
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
AR0330_read(
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
	unsigned char addr[2], data[2];
	int nRet;
	int retry = 0;
	
	addr[0] = (reg >> 8) & 0xFF;
	addr[1] = reg & 0xFF;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = addr;	
	g_ti2c_handle.dataCnt = 2;	
	nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
	while(nRet<0) {
		retry++;
		if(nRet == -88)
			return nRet;
		nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
		if(nRet<0 && retry>TI2C_RETRY) {
			printk("retry too much arr\n");
			return nRet;
		}
	}
//	if(nRet <= 0) {
//		return nRet;
//	}
	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_READ_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 2;	
	nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
	value[0] = data[1];
	value[1] = data[0];
#endif
}

static int 
AR0330_write(
	unsigned short reg, 
	unsigned short value
)
{
#if (I2C_MODE == HW_I2C)
	char data[3];
	
	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = value;	
	return gp_i2c_bus_write(g_i2c_handle, data, 3);
	
#elif (I2C_MODE == HW_TI2C)

#if 0
	unsigned char data[3];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = value;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 3;	
	return gp_ti2c_bus_xfer(&g_ti2c_handle);
#else
	unsigned char data[4];
	int ret;
	int retry = 0;

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = (value >> 8) & 0xFF;
	data[3] = value & 0xFF;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 4;
	ret = gp_ti2c_bus_xfer(&g_ti2c_handle);
	while(ret<0) {
		retry++;
		if(ret == -88)
			return ret;
		ret = gp_ti2c_bus_xfer(&g_ti2c_handle);
		if(ret<0 && retry>TI2C_RETRY) {
			printk("retry too much aw\n");
			return ret;
		}
	}
	return ret;
//	return gp_ti2c_bus_xfer(&g_ti2c_handle);

#endif

#endif	
}

#if 0
static int 
AR0330_read(
	unsigned short reg,
	unsigned char *value
)
{
	unsigned char addr[2], data[2];
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
	g_ti2c_handle.dataCnt = 2;	
	nRet = gp_ti2c_bus_xfer(&g_ti2c_handle);
	value[0] = data[1];
	value[1] = data[0];
	return nRet;
}



static int 
AR0330_write(
	unsigned short reg,
	unsigned short value
)
{
	unsigned char data[4];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = (value >> 8) & 0xFF;
	data[3] = value & 0xFF;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 4;	
	return gp_ti2c_bus_xfer(&g_ti2c_handle);
}
#endif






static int ar0330_cvt_analog_gain(int analog_gain)
{
	int i;
	int coarse_gain, fine_gain;
	int *p_ar0330_analog_gain_table = ar0330_analog_gain_table;

	for( i = 0 ; i < 29 ; i++)
	{
		if(analog_gain >= p_ar0330_analog_gain_table[i] && analog_gain < p_ar0330_analog_gain_table[i+1])
			break;
	} 

	if( i < 16 )
	{
		coarse_gain = 0;
		fine_gain = i;
	}
	else if(i < (16+8))
	{
		coarse_gain = 1;
		fine_gain = (i - 16) << 1;
	}
	else if(i < (16 + 8 + 4))
	{
		coarse_gain = 2;
		fine_gain = (i - 16 - 8) << 2;
	}
	else
	{
		coarse_gain = 3;
		fine_gain = 0;
	}

	
	return ((coarse_gain << 4) | fine_gain);
}

static int ar0330_get_real_analog_gain(int analog_gain, int context)
{
	int real_analog_gain;
	int coarse_gain, fine_gain;

	if(context == 0)
	{	// context A
		coarse_gain = (analog_gain >> 4) & 0x3;
		fine_gain = (analog_gain & 0xf);
	}
	else
	{	// context B
		coarse_gain = (analog_gain >> 12) & 0x3;
		fine_gain = (analog_gain >> 8) & 0xf;
	}

	if(coarse_gain == 0)
	{
		real_analog_gain = ar0330_analog_gain_table[fine_gain];
	}
	else	if(coarse_gain == 1)
	{
		real_analog_gain = ar0330_analog_gain_table[16 + (fine_gain >> 1)];
	}
	else	if(coarse_gain == 2)
	{
		real_analog_gain = ar0330_analog_gain_table[16 + 8 + (fine_gain >> 2)];
	}
	else	if(coarse_gain == 3)
	{
		real_analog_gain = ar0330_analog_gain_table[16+8+4];
	}
	else
	{
		printk("AR0330 analog gain Err!\r\n");
	}
	
	
	return real_analog_gain;
}





int ar0330_set_exposure_time(sensor_exposure_t *si)
{
	int ret;
	unsigned short tmp;
	int analog_gain;
	int context;
	
	ret = AR0330_read( 0x30B0, &tmp );  
	if(ret==-88)
		return -1;
	context = (tmp >> 13) & 0x1;

	analog_gain = ar0330_cvt_analog_gain(si->analog_gain);

	//if(context == 0)
	{			
		// set Analog gain	
		tmp = analog_gain << 8;
		tmp |= analog_gain;
		ret = AR0330_write(0x3060, tmp ); 
		if(ret==-88)
			return -1;

		// set Digital gain
		AR0330_write(0x305E, si->digital_gain);

		// set exposure time
		AR0330_write(0x3012 , si->time );
	}
	//else
	{		
		// set Analog gain	
		//analog_gain <<= 8;
		//AR0330_write(0x3060, analog_gain);  

		// set Digital gain
		AR0330_write(0x30C4, si->digital_gain);
		
		// set exposure time
		AR0330_write(0x3016 , si->time );
	}
}


//int dir = 0;
static int ar0330_change_fps(int fps)
{
	int ret = 0;
	
	if(fps == 30)
		ret = AR0330_write(0x300A, 1287);
	else if(fps == 25)
		ret = AR0330_write(0x300A, 1543);

	return ret;
}

int ar0330_set_xfps_exposure_time(sensor_exposure_t *si)
{
	int ret = 0;
	unsigned short tmp;
	int analog_gain;
	int idx;
	int context;

	
/*	if(dir == 0) si->ae_ev_idx = -3;
	else si->ae_ev_idx = 3;
	
	if(si->sensor_ev_idx > 151) dir = 0;
	else if(si->sensor_ev_idx < 112) dir = 1;*/
	
	si->sensor_ev_idx += si->ae_ev_idx;
	if(si->sensor_ev_idx >= si->max_ev_idx) si->sensor_ev_idx = si->max_ev_idx;
	if(si->sensor_ev_idx < 0) si->sensor_ev_idx = 0;

	idx = si->sensor_ev_idx * 3;
	si ->time = p_expTime_table[idx];
	si ->analog_gain = p_expTime_table[idx+1];
	si ->digital_gain = p_expTime_table[idx+2] >> 1;

	//printk("new: sensor_ev_idx = %d, ae_ev_idx = %d\n", si->sensor_ev_idx, si->ae_ev_idx);
	//printk("%s [%d]: time = 0x%x, a_gain = 0x%x, d_gain = 0x%x\n", __FUNCTION__, si->sensor_ev_idx, si->time, si->analog_gain, si->digital_gain);	

	
	
	if(sensor_fps == V4L2_TC_TYPE_25FPS_30FPS)
	{
		int fps;

		if(si ->time >= sensor_swith_time)	fps = 25;
		else	fps = 30;
		
		if(current_frame_rate != fps)
		{
			ret = ar0330_change_fps(fps);
			if(ret < 0) return ret;

			printk("switch to %dfps\r\n", fps);
			
			current_frame_rate = fps;
		}
		//ret = AR0330_write(0x30AA, tmp); // CB
		//if(ret < 0) return ret;
	}
	
	analog_gain = ar0330_cvt_analog_gain(si->analog_gain);

	
	// Context Switch
	ret = AR0330_read( 0x30B0, &tmp );  
	if(ret < 0) return ret;
		
	
	#if 1
	// set exposure time
	if(pre_sensor_time != si->time)
	{
		ret = AR0330_write(0x3012 , si->time );
		if(ret < 0) return ret;

		pre_sensor_time = si->time;
	}
	
	// set Analog gain		
	if(pre_sensor_a_gain != analog_gain)
	{
		ret = AR0330_write(0x3060, analog_gain ); 
		if(ret < 0) return ret;

		pre_sensor_a_gain = analog_gain;
	}

	
	// set Digital gain
	if(pre_sensor_d_gain != si->digital_gain)
	{
		ret = AR0330_write(0x305E, si->digital_gain);
		if(ret < 0) return ret;

		pre_sensor_d_gain = si->digital_gain;
	}
	
	if((tmp | 0x2000) != 0)
	{
		tmp = tmp & (~0x2000);
		ret = AR0330_write(0x30B0, tmp);
	}
	
	#else
	context = (tmp >> 13) & 0x1;
	//local_irq_disable();
	if(context == 0)
	{	// to set Context B
		// set exposure time
		if(pre_sensor_time_b != si->time)
		{
			ret = AR0330_write(0x3016 , si->time );
			if(ret < 0) return ret;

			pre_sensor_time_b = si->time;
		}
		
		// set Analog gain		
		if(pre_sensor_a_gain_b != analog_gain)
		{
			ret = AR0330_write(0x3060, (analog_gain << 8) ); 
			if(ret < 0) return ret;

			pre_sensor_a_gain_b = analog_gain;
		}

		
		// set Digital gain
		if(pre_sensor_d_gain_b != si->digital_gain)
		{
			ret = AR0330_write(0x30c4, si->digital_gain);
			if(ret < 0) return ret;

			pre_sensor_d_gain_b = si->digital_gain;
		}
				
		tmp = tmp | 0x2000;

		printk("CB = 0x%x\r\n", tmp);
	}		
	else
	{	// to set Context A	
		// set exposure time
		if(pre_sensor_time != si->time)
		{
			ret = AR0330_write(0x3012 , si->time );
			if(ret < 0) return ret;

			pre_sensor_time = si->time;
		}
		
		// set Analog gain		
		if(pre_sensor_a_gain != analog_gain)
		{
			ret = AR0330_write(0x3060, analog_gain ); 
			if(ret < 0) return ret;

			pre_sensor_a_gain = analog_gain;
		}

		
		// set Digital gain
		if(pre_sensor_d_gain != si->digital_gain)
		{
			ret = AR0330_write(0x305E, si->digital_gain);
			if(ret < 0) return ret;

			pre_sensor_d_gain = si->digital_gain;
		}

		tmp = tmp & (~0x2000);
		printk("CA = 0x%x\r\n", tmp);
	}
	AR0330_write(0x30B0, tmp);
	//local_irq_enable();
	#endif

	return ret;
}



void ar0330_reset_ob(void)
{
	unsigned short tmp;
	
	ob_cnt++;
	if(ob_cnt >= 20)
	{
		int ret;
		ob_cnt = 0;

		// ob calibration
		ret = AR0330_read( 0x3180, &tmp );
		tmp |= 0x2000;
		AR0330_write(0x3180, tmp);
	}
}

int ar0330_get_exposure_time(sensor_exposure_t *se)
{
	int ret;
	unsigned short tmp;
	int context;

	int testing_cnt;

//	testing_cnt = 4;
//	do {
		int real_dgain;
		
		testing_cnt--;
		
		ret = AR0330_read( 0x30B0, &tmp );  
		context = (tmp >> 13) & 0x1;

		// Analog gain
		ret = AR0330_read( 0x3060, &tmp );  
		se->analog_gain= ar0330_get_real_analog_gain(tmp, context);

		if(context == 0)
		{	// Context A
		
			// coase time
			ret = AR0330_read( 0x3012, &tmp );
			se->time = tmp;

			// digital gain
			ret = AR0330_read( 0x305E, &tmp);
			se->digital_gain = tmp;

			//printk("context A\n");
		}
		else
		{	// Context B
		
			// coase time
			ret = AR0330_read( 0x3016, &tmp);
			se->time = tmp;

			// digital gain
			ret = AR0330_read( 0x30C4, &tmp);
			se->digital_gain = tmp;
			//printk("xxxxxxxxxxxxxxx  context B xxxxxxxxxxxxxxxxx\n");
		}	

#if 0
		// to check the information
		real_dgain = se->digital_gain << 1;
		if(se->time >= se->min_time && se->time <= se->max_time
			&& real_dgain >= se->min_digital_gain && real_dgain <= se->max_digital_gain
			&& se->analog_gain >= se->min_analog_gain && se->analog_gain <= se->max_analog_gain)
		{
			testing_cnt = 0;
		}
		else if(testing_cnt == 0) 
		{
			
			printk("Err: Get Sensor Info!\n\n");
			printk("Digital Gain: %x, %x, %x\n", real_dgain, se->max_digital_gain, se->min_digital_gain);
			printk("Analog Gain: %x, %x, %x\n", se->analog_gain, se->max_analog_gain, se->min_analog_gain);
			printk("Time: %x, %x, %x\n", se->time, se->max_time, se->min_time);

			if(se->time < se->min_time) se->time = se->min_time;
			if(se->time > se->max_time) se->time = se->max_time;
			
			if(real_dgain < se->min_digital_gain) real_dgain = se->min_digital_gain;
			if(real_dgain > se->max_digital_gain) real_dgain = se->max_digital_gain;
			se->digital_gain = real_dgain >> 1;

			if(se->analog_gain < se->min_analog_gain) se->analog_gain = se->min_analog_gain;
			if(se->analog_gain > se->max_analog_gain) se->analog_gain = se->max_analog_gain;

		}
		
			
		
	}while(testing_cnt > 0);
	#endif
	
	return ret;
}




static int
AR0330WrTable(
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
		}
		else if (pTable->reg_num == 0xfffe && pTable->value == 0xfffe) {
			mdelay(25);//comi
			pTable++;
			continue;	
		}
		else if (pTable->reg_num == 0xfffd && pTable->value == 0xfffd) {
			mdelay(35);//comi
			pTable++;
			continue;	
		}
		
		if ( AR0330_write(pTable->reg_num, pTable->value) < 0)
			return -1;
		//ytliao: don't do printk to speed up initialization
		//printk("[0x%x] = 0x%x\n", pTable->reg_num, pTable->value);
		pTable ++;
	}

	//color bar
	//AR0330_write(0x3070, 0x02);
	return 0;
}

static int 
AR0330_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	seInfo.max_time = AR0330_MAX_EXPOSURE_TIME;
	seInfo.min_time = AR0330_MIN_EXPOSURE_TIME;

	seInfo.max_digital_gain = AR0330_MAX_DIGITAL_GAIN << 1;
	seInfo.min_digital_gain = AR0330_MIN_DIGITAL_GAIN << 1;

	seInfo.max_analog_gain = AR0330_MAX_ANALOG_GAIN;
	seInfo.min_analog_gain = AR0330_MIN_ANALOG_GAIN;

	seInfo.analog_gain = seInfo.min_analog_gain;
	seInfo.digital_gain = seInfo.min_digital_gain;
	seInfo.time = seInfo.max_time >> 1;

	seInfo.sensor_ev_idx = AR0330_30FPS_50HZ_INIT_EV_IDX;
	seInfo.ae_ev_idx = 0;
	seInfo.daylight_ev_idx= AR0330_30FPS_50HZ_DAY_EV_IDX;
	seInfo.night_ev_idx= AR0330_30FPS_50HZ_NIGHT_EV_IDX;
	seInfo.max_ev_idx = AR0330_30FPS_50HZ_MAX_EV_IDX;
	p_expTime_table = ar0330_30fps_exp_time_gain_50Hz;
	sensor_fps = V4L2_TC_TYPE_30FPS;
	sensor_total_ev = AR0330_30FPS_50HZ_EXP_TIME_TOTAL;
	sensor_swith_time = 2000;
	current_frame_rate= 30;
		
	pre_sensor_time = pre_sensor_a_gain = pre_sensor_d_gain = -1;
	pre_sensor_time_b = pre_sensor_a_gain_b = pre_sensor_d_gain_b = -1;

	ob_cnt = 0;
	printk("%s\n", __FUNCTION__);
	return 0;
}


static int 
AR0330_preview(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return AR0330WrTable((regval16_t *)AR0330_Mipi_Raw10_720P);
}

static int 
AR0330_capture(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return AR0330WrTable((regval16_t *)AR0330_Mipi_Raw10_1080P);
}


static int 
AR0330_capture_25fps(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return AR0330WrTable((regval16_t *)AR0330_Mipi_Raw10_1080P_25fps);
}


static int 
AR0330_record(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return AR0330WrTable((regval16_t *)AR0330_Mipi_Raw10_720P);
}

static int
AR0330_ldw(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return AR0330WrTable((regval16_t *)AR0330_Mipi_Raw10_1080P_LDW);	
}


static int
AR0330_720p_60fps(
	void
)
{
	printk("%s\n", __FUNCTION__);
	return AR0330WrTable((regval16_t *)AR0330_Mipi_Raw10_720P);	
}


static int 
AR0330_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	return 0;
}

static int 
AR0330_queryctrl(
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
AR0330_g_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{	
	unsigned short value, ret = 0;
	
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
		if(p_expTime_table == 0)
		{
			ar0330_get_exposure_time(&seInfo);
			seInfo.digital_gain = seInfo.digital_gain << 1;
		}
		ar0330_reset_ob();
		ctrl->value = (int)&seInfo;
		break;
		
	case V4L2_CID_VFLIP:
		AR0330_read(0x3040, &value);
		ctrl->value = (value & 0x8000) >> 15;
		break;
		
	case V4L2_CID_HFLIP:
		AR0330_read(0x3040, &value);
		ctrl->value = (value & 0x4000) >> 14;
		break;
		
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = 64 - (sensor_total_ev - seInfo.max_ev_idx);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int 
AR0330_s_ctrl(
	struct v4l2_subdev *sd, 
	struct v4l2_control *ctrl
)
{
	int nRet = 0;
	unsigned short value;

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
			// set exposure time
			sensor_exposure_t *si;
			si = (sensor_exposure_t *) ctrl->value;
			
			seInfo.userISO = si->userISO;
				
			if(p_expTime_table != 0)
			{
				seInfo.ae_ev_idx = si->ae_ev_idx;
				nRet = ar0330_set_xfps_exposure_time(&seInfo);
			}
			else
			{
				//printk("%s: time = %x, a_gain = %x, d_gain = %x\n", __FUNCTION__, si->time, si->analog_gain, si->digital_gain);			
				seInfo.time = si->time;
				seInfo.digital_gain = si->digital_gain >> 1;
				seInfo.analog_gain = si->analog_gain;
				nRet = ar0330_set_exposure_time(&seInfo);
			}
			break;
		}

	case V4L2_CID_POWER_LINE_FREQUENCY:
		{
			enum v4l2_power_line_frequency line_freq;
			line_freq = ctrl->value;
			sensor_swith_time = 2000;
			current_frame_rate= 30;
			
			if(sensor_fps == V4L2_TC_TYPE_30FPS)
			{
				if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED 
					|| line_freq == V4L2_CID_POWER_LINE_FREQUENCY_50HZ)
				{
					p_expTime_table = ar0330_30fps_exp_time_gain_50Hz;		
					
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_30FPS_50HZ_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_30FPS_50HZ_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_30FPS_50HZ_MAX_EV_IDX;
					sensor_total_ev = AR0330_30FPS_50HZ_EXP_TIME_TOTAL;
					
					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
				}
				else if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_60HZ)
				{
					p_expTime_table = ar0330_30fps_exp_time_gain_60Hz;
					
					seInfo.sensor_ev_idx = AR0330_30FPS_60HZ_INIT_EV_IDX;
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_30FPS_60HZ_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_30FPS_60HZ_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_30FPS_50HZ_MAX_EV_IDX;
					sensor_total_ev = AR0330_30FPS_60HZ_EXP_TIME_TOTAL;
					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
				}
				else	
					return -EINVAL;
			}
			else if(sensor_fps == V4L2_TC_TYPE_25FPS || sensor_fps == V4L2_TC_TYPE_25FPS_30FPS)
			{
				if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED 
					|| line_freq == V4L2_CID_POWER_LINE_FREQUENCY_50HZ)
				{
					p_expTime_table = ar0330_25fps_exp_time_gain_50Hz;		
					sensor_swith_time = AR0330_25FPS_50HZ_SWITCH_TIME;
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_25FPS_50HZ_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_25FPS_50HZ_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_25FPS_50HZ_MAX_EV_IDX;
					sensor_total_ev = AR0330_25FPS_50HZ_EXP_TIME_TOTAL;

					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
					
				}
				else if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_60HZ)
				{
					p_expTime_table = ar0330_25fps_exp_time_gain_60Hz;
					sensor_swith_time = AR0330_25FPS_60HZ_SWITCH_TIME;
					seInfo.sensor_ev_idx = AR0330_25FPS_60HZ_INIT_EV_IDX;
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_25FPS_60HZ_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_25FPS_60HZ_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_25FPS_60HZ_MAX_EV_IDX;
					sensor_total_ev = AR0330_25FPS_60HZ_EXP_TIME_TOTAL;
					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
					
				}
				else
					return -EINVAL;

				printk("*******  set 25fps: sensor_swith_time = %d **********\r\n", sensor_swith_time);
			}
			else if(sensor_fps == V4L2_TC_TYPE_60FPS)
			{
				if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED
					|| line_freq == V4L2_CID_POWER_LINE_FREQUENCY_50HZ)
				{
					p_expTime_table = ar0330_60fps_exp_time_gain_50Hz;		
					
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_60FPS_50HZ_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_60FPS_50HZ_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_60FPS_50HZ_MAX_EV_IDX;
					sensor_total_ev = AR0330_60FPS_50HZ_EXP_TIME_TOTAL;

					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
					
				}
				else if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_60HZ)
				{
					p_expTime_table = ar0330_60fps_exp_time_gain_60Hz;

					seInfo.sensor_ev_idx = AR0330_60FPS_60HZ_INIT_EV_IDX;
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_60FPS_60HZ_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_60FPS_60HZ_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_60FPS_50HZ_MAX_EV_IDX;
					sensor_total_ev = AR0330_60FPS_60HZ_EXP_TIME_TOTAL;
					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
					
				}				
				else
					return -EINVAL;
			}			
			else if(sensor_fps == V4L2_TC_TYPE_25FPS_LDW)
			{
				// for ldw use only
				if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED 
					|| line_freq == V4L2_CID_POWER_LINE_FREQUENCY_50HZ)
				{
					p_expTime_table = ar0330_25fps_exp_time_gain_50Hz_ldw;		
					
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_25FPS_50HZ_LDW_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_25FPS_50HZ_LDW_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_25FPS_50HZ_LDW_MAX_EV_IDX;
					sensor_total_ev = AR0330_25FPS_50HZ_LDW_EXP_TIME_TOTAL;

					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
					
				}
				else if(line_freq == V4L2_CID_POWER_LINE_FREQUENCY_60HZ)
				{
					p_expTime_table = ar0330_25fps_exp_time_gain_60Hz_ldw;
					
					seInfo.sensor_ev_idx = AR0330_25FPS_60HZ_LDW_INIT_EV_IDX;
					seInfo.ae_ev_idx = 0;
					seInfo.daylight_ev_idx= AR0330_25FPS_60HZ_LDW_DAY_EV_IDX;
					seInfo.night_ev_idx= AR0330_25FPS_60HZ_LDW_NIGHT_EV_IDX;
					seInfo.max_ev_idx= AR0330_25FPS_50HZ_LDW_MAX_EV_IDX;
					sensor_total_ev = AR0330_25FPS_60HZ_LDW_EXP_TIME_TOTAL;
					if(seInfo.sensor_ev_idx > seInfo.max_ev_idx) seInfo.sensor_ev_idx = seInfo.max_ev_idx;
					
				}
				else 
					return -EINVAL;
			}
			else 
				return -EINVAL;

			
			break;
		}
	case V4L2_CID_VFLIP:
		AR0330_read(0x3040, &value);
		value &= ~(0x1 << 15);
		value = value | (ctrl->value << 15);
		AR0330_write(0x3040, value);
		break;
	case V4L2_CID_HFLIP:
		AR0330_read(0x3040, &value);
		value &= ~(0x1 << 14);
		value = value | (ctrl->value << 14);
		AR0330_write(0x3040, value);
		break;

	case V4L2_CID_BRIGHTNESS:
		seInfo.max_ev_idx = sensor_total_ev - (64 - ctrl->value);
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
AR0330_querystd(
	struct v4l2_subdev *sd,
	v4l2_std_id *std
)
{
	return 0;
}

static int 
AR0330_enum_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_fmtdesc *fmtdesc
)
{
	printk("%s\n", __FUNCTION__);
	if(fmtdesc->index >= C_SENSOR_FMT_MAX)
		return -EINVAL;

	fmtdesc->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	memcpy((void *)fmtdesc->description, (void *)gAR0330FmtTable[fmtdesc->index].desc, 32);
	fmtdesc->pixelformat = gAR0330FmtTable[fmtdesc->index].pixelformat;
	return 0;
}

static int 
AR0330_g_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	printk("%s\n", __FUNCTION__);
	fmt->fmt.pix.width = gAR0330Dev.fmt->hpixel;
	fmt->fmt.pix.height = gAR0330Dev.fmt->vline;
	fmt->fmt.pix.pixelformat = gAR0330Dev.fmt->pixelformat;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = gAR0330Dev.fmt->hpixel * gAR0330Dev.fmt->bpp;
	fmt->fmt.pix.sizeimage = fmt->fmt.pix.bytesperline * gAR0330Dev.fmt->vline;

	return 0;
}

static int 
AR0330_try_fmt(
	struct v4l2_subdev *sd,
	struct v4l2_format *fmt
)
{
	return 0;
}

static int 
AR0330_s_fmt(
	struct v4l2_subdev *sd, 
	struct v4l2_format *fmt
)
{
	int ret;

	printk("%s = %d\n", __FUNCTION__, fmt->fmt.pix.priv);
	switch(fmt->fmt.pix.priv)
	{
	case 0: 
		ret = AR0330_preview();
		break;

	case 1: 		
		ret = AR0330_capture();		
		break;

	case 2: 
		ret = AR0330_record();
		break;	
	
	case 3:
		sensor_fps = V4L2_TC_TYPE_25FPS_LDW;
		ret = AR0330_ldw();
		break;
		
	case 4:
		sensor_fps = V4L2_TC_TYPE_60FPS;
		ret = AR0330_720p_60fps();
		break;
		
	case 5:
		// for 25fps in night, 30fps in daytime
		sensor_fps = V4L2_TC_TYPE_25FPS_30FPS;
		ret = AR0330_capture_25fps();
		break;
		
	case 6:
		// const fps = 25
		sensor_fps = V4L2_TC_TYPE_25FPS;
		ret = AR0330_capture_25fps();
		ar0330_change_fps(25);
		break;
		
	default:
		ret = -1;
	}

	gAR0330Dev.fmt = &gAR0330FmtTable[fmt->fmt.pix.priv];

	if(ret == 0)
		printk("%s SUCCESS\n", __FUNCTION__);
	else
		printk("%s fail\n", __FUNCTION__);
	
	return ret;
}

static int 
AR0330_cropcap(
	struct v4l2_subdev *sd,
	struct v4l2_cropcap *cc
)
{
	return 0;
}

static int 
AR0330_g_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
AR0330_s_crop(
	struct v4l2_subdev *sd,
	struct v4l2_crop *crop
)
{
	return 0;
}

static int 
AR0330_g_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *parms
)
{
	return 0;
}

static int 
AR0330_s_parm(
	struct v4l2_subdev *sd,
	struct v4l2_streamparm *param
)
{
	return 0;
}

static int 
AR0330_s_interface(
	struct v4l2_subdev *sd,
	struct v4l2_interface *interface
)
{
	return 0;
}

static int 
AR0330_suspend(
	struct v4l2_subdev *sd
)
{
	return 0;
}

static int 
AR0330_resume(
	struct v4l2_subdev *sd
)
{
	return 0;
}

static const struct v4l2_subdev_core_ops AR0330_core_ops = 
{
	.init = AR0330_init,
	.reset = AR0330_reset,
	.queryctrl = AR0330_queryctrl,
	.g_ctrl = AR0330_g_ctrl,
	.s_ctrl = AR0330_s_ctrl,
};

static const struct v4l2_subdev_video_ops AR0330_video_ops = 
{
	.querystd = AR0330_querystd,
	.enum_fmt = AR0330_enum_fmt,
	.g_fmt = AR0330_g_fmt,
	.try_fmt = AR0330_try_fmt,
	.s_fmt = AR0330_s_fmt,
	.cropcap = AR0330_cropcap,
	.g_crop = AR0330_g_crop,
	.s_crop = AR0330_s_crop,
	.g_parm = AR0330_g_parm,
	.s_parm = AR0330_s_parm,
};

static const struct v4l2_subdev_ext_ops AR0330_ext_ops = 
{
	.s_interface = AR0330_s_interface,
	.suspend = AR0330_suspend,
	.resume = AR0330_resume,
};

static const struct v4l2_subdev_ops AR0330_ops = 
{
	.core = &AR0330_core_ops,
	.video = &AR0330_video_ops,
	.ext = &AR0330_ext_ops
};

static int __init 
AR0330_module_init(
		void
)
{
	if(sensor_i2c_open(AR0330_ID, 50) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: AR0330 mipi\n");
	gAR0330Dev.fmt = &gAR0330FmtTable[0];
	v4l2_subdev_init(&(gAR0330Dev.sd), &AR0330_ops);
	strcpy(gAR0330Dev.sd.name, "sensor_ar0330_mipi");
	register_sensor(&gAR0330Dev.sd, (int *)&param[0], &AR0330_config_table);
	return 0;
}

static void __exit
AR0330_module_exit(
		void
)
{
	sensor_i2c_close();
	unregister_sensor(&(gAR0330Dev.sd));
}

module_init(AR0330_module_init);
module_exit(AR0330_module_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus AR0330 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
