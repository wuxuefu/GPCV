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
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>

#include <mach/gp_i2c_bus.h>
#include <mach/gp_mipi.h>
#include <mach/sensor_mgr.h>

#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define	SS6AA_ID		0x78

#define C_SS6AA_YUYV	(0<<4)
#define C_SS6AA_YVYU	(1<<4)
#define C_SS6AA_FMT		C_SS6AA_YVYU	

#if C_SS6AA_FMT == C_SS6AA_YUYV
#define SS6AA_PIXFMT	V4L2_PIX_FMT_YUYV
#elif C_SS6AA_FMT == C_SS6AA_YVYU
#define SS6AA_PIXFMT	V4L2_PIX_FMT_YVYU
#endif

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
		.pixelformat = SS6AA_PIXFMT,
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
		.desc		= "capture=1280*1024",
		.pixelformat = SS6AA_PIXFMT,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 1024,
		.voffset = 0,
	},
	/* record mode */
	{
		.desc		= "record=1280*720",
		.pixelformat = SS6AA_PIXFMT,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1280,
		.hoffset = 0,
		.vline = 720,
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
	.ecc_check_en = DISABLE,
	.ecc_order = MIPI_ECC_ORDER3,
	.data_mask_time = 100, //ns
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
 
regval16_t ss6AA_init_table[] =
{
// Start T&P part
// DO NOT DELETE T&P SECTION COMMENTS! They are required to debug T&P related issues.
// svn://transrdsrv/svn/svnroot/System/Software/tcevb/SDK+FW/ISP_Oscar/Firmware
// Rev: 33110-33110
// Signature:
// md5 f0ba942df15b96de5c09e6cf13fed9c9 .btp
// md5 8bc59f72129cb36e6f6db4be5ddca1f6 .htp
// md5 954ec97efcabad291d89f63e29f32490 .RegsMap.h
// md5 5c29fe50b51e7e860313f5b3b6452bfd .RegsMap.bin
// md5 6211407baaa234b753431cde4ba32402 .base.RegsMap.h
// md5 90cc21d42cc5f02eb80b2586e5c46d9b .base.RegsMap.bin
//
	{ 0x0028, 0x7000 },
	{ 0x002A, 0x1D60 },
	{ 0x0F12, 0xB570 },
	{ 0x0F12, 0x4936 },
	{ 0x0F12, 0x4836 },
	{ 0x0F12, 0x2205 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA4E },
	{ 0x0F12, 0x4935 },
	{ 0x0F12, 0x2002 },
	{ 0x0F12, 0x83C8 },
	{ 0x0F12, 0x2001 },
	{ 0x0F12, 0x3120 },
	{ 0x0F12, 0x8088 },
	{ 0x0F12, 0x4933 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0x8008 },
	{ 0x0F12, 0x4933 },
	{ 0x0F12, 0x8048 },
	{ 0x0F12, 0x4933 },
	{ 0x0F12, 0x4833 },
	{ 0x0F12, 0x2204 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA3E },
	{ 0x0F12, 0x4932 },
	{ 0x0F12, 0x4833 },
	{ 0x0F12, 0x2206 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA39 },
	{ 0x0F12, 0x4932 },
	{ 0x0F12, 0x4832 },
	{ 0x0F12, 0x2207 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA34 },
	{ 0x0F12, 0x4931 },
	{ 0x0F12, 0x4832 },
	{ 0x0F12, 0x2208 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA2F },
	{ 0x0F12, 0x4931 },
	{ 0x0F12, 0x4831 },
	{ 0x0F12, 0x2209 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA2A },
	{ 0x0F12, 0x4930 },
	{ 0x0F12, 0x4831 },
	{ 0x0F12, 0x220A },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA25 },
	{ 0x0F12, 0x4930 },
	{ 0x0F12, 0x4830 },
	{ 0x0F12, 0x220B },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA20 },
	{ 0x0F12, 0x482F },
	{ 0x0F12, 0x4930 },
	{ 0x0F12, 0x6108 },
	{ 0x0F12, 0x4830 },
	{ 0x0F12, 0x39FF },
	{ 0x0F12, 0x3901 },
	{ 0x0F12, 0x6748 },
	{ 0x0F12, 0x482F },
	{ 0x0F12, 0x1C0A },
	{ 0x0F12, 0x32C0 },
	{ 0x0F12, 0x6390 },
	{ 0x0F12, 0x482E },
	{ 0x0F12, 0x6708 },
	{ 0x0F12, 0x491A },
	{ 0x0F12, 0x482D },
	{ 0x0F12, 0x3108 },
	{ 0x0F12, 0x60C1 },
	{ 0x0F12, 0x6882 },
	{ 0x0F12, 0x1A51 },
	{ 0x0F12, 0x8201 },
	{ 0x0F12, 0x4C2B },
	{ 0x0F12, 0x2607 },
	{ 0x0F12, 0x6821 },
	{ 0x0F12, 0x0736 },
	{ 0x0F12, 0x42B1 },
	{ 0x0F12, 0xDA05 },
	{ 0x0F12, 0x4829 },
	{ 0x0F12, 0x22D8 },
	{ 0x0F12, 0x1C05 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA09 },
	{ 0x0F12, 0x6025 },
	{ 0x0F12, 0x68A1 },
	{ 0x0F12, 0x42B1 },
	{ 0x0F12, 0xDA07 },
	{ 0x0F12, 0x4825 },
	{ 0x0F12, 0x2224 },
	{ 0x0F12, 0x3824 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xFA00 },
	{ 0x0F12, 0x4822 },
	{ 0x0F12, 0x3824 },
	{ 0x0F12, 0x60A0 },
	{ 0x0F12, 0x4D22 },
	{ 0x0F12, 0x6D29 },
	{ 0x0F12, 0x42B1 },
	{ 0x0F12, 0xDA07 },
	{ 0x0F12, 0x481F },
	{ 0x0F12, 0x228F },
	{ 0x0F12, 0x00D2 },
	{ 0x0F12, 0x30D8 },
	{ 0x0F12, 0x1C04 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF9F2 },
	{ 0x0F12, 0x652C },
	{ 0x0F12, 0xBC70 },
	{ 0x0F12, 0xBC08 },
	{ 0x0F12, 0x4718 },
	{ 0x0F12, 0x218B },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x127B },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0398 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x1376 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x2370 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x1F0D },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x890D },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x1F2F },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x27A9 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x1FE1 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x27C5 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x2043 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x285F },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x2003 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x28FF },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x20CD },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x6181 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x20EF },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x6663 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x2123 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x0100 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x1EC1 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x1EAD },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x1F79 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x04AC },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x06CC },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x23A4 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x0704 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0xB510 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF9B9 },
	{ 0x0F12, 0x48C3 },
	{ 0x0F12, 0x49C3 },
	{ 0x0F12, 0x8800 },
	{ 0x0F12, 0x8048 },
	{ 0x0F12, 0xBC10 },
	{ 0x0F12, 0xBC08 },
	{ 0x0F12, 0x4718 },
	{ 0x0F12, 0xB5F8 },
	{ 0x0F12, 0x1C06 },
	{ 0x0F12, 0x4DC0 },
	{ 0x0F12, 0x68AC },
	{ 0x0F12, 0x1C30 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF9B3 },
	{ 0x0F12, 0x68A9 },
	{ 0x0F12, 0x4ABC },
	{ 0x0F12, 0x42A1 },
	{ 0x0F12, 0xD003 },
	{ 0x0F12, 0x4BBD },
	{ 0x0F12, 0x8A1B },
	{ 0x0F12, 0x3301 },
	{ 0x0F12, 0x8013 },
	{ 0x0F12, 0x8813 },
	{ 0x0F12, 0x1C14 },
	{ 0x0F12, 0x2B00 },
	{ 0x0F12, 0xD00F },
	{ 0x0F12, 0x2201 },
	{ 0x0F12, 0x4281 },
	{ 0x0F12, 0xD003 },
	{ 0x0F12, 0x8C2F },
	{ 0x0F12, 0x42B9 },
	{ 0x0F12, 0xD300 },
	{ 0x0F12, 0x2200 },
	{ 0x0F12, 0x60AE },
	{ 0x0F12, 0x2A00 },
	{ 0x0F12, 0xD003 },
	{ 0x0F12, 0x8C28 },
	{ 0x0F12, 0x42B0 },
	{ 0x0F12, 0xD800 },
	{ 0x0F12, 0x1C30 },
	{ 0x0F12, 0x1E59 },
	{ 0x0F12, 0x8021 },
	{ 0x0F12, 0xBCF8 },
	{ 0x0F12, 0xBC08 },
	{ 0x0F12, 0x4718 },
	{ 0x0F12, 0xB510 },
	{ 0x0F12, 0x1C04 },
	{ 0x0F12, 0x48AF },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF997 },
	{ 0x0F12, 0x4AAD },
	{ 0x0F12, 0x4BAE },
	{ 0x0F12, 0x8811 },
	{ 0x0F12, 0x885B },
	{ 0x0F12, 0x8852 },
	{ 0x0F12, 0x4359 },
	{ 0x0F12, 0x1889 },
	{ 0x0F12, 0x4288 },
	{ 0x0F12, 0xD800 },
	{ 0x0F12, 0x1C08 },
	{ 0x0F12, 0x6020 },
	{ 0x0F12, 0xE7C5 },
	{ 0x0F12, 0xB570 },
	{ 0x0F12, 0x1C05 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF98F },
	{ 0x0F12, 0x49A5 },
	{ 0x0F12, 0x8989 },
	{ 0x0F12, 0x4348 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0x0C00 },
	{ 0x0F12, 0x2101 },
	{ 0x0F12, 0x0349 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF98E },
	{ 0x0F12, 0x1C04 },
	{ 0x0F12, 0x489F },
	{ 0x0F12, 0x8F80 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF991 },
	{ 0x0F12, 0x1C01 },
	{ 0x0F12, 0x20FF },
	{ 0x0F12, 0x43C0 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF994 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF998 },
	{ 0x0F12, 0x1C01 },
	{ 0x0F12, 0x4898 },
	{ 0x0F12, 0x8840 },
	{ 0x0F12, 0x4360 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0x0C00 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF97A },
	{ 0x0F12, 0x6028 },
	{ 0x0F12, 0xBC70 },
	{ 0x0F12, 0xBC08 },
	{ 0x0F12, 0x4718 },
	{ 0x0F12, 0xB5F1 },
	{ 0x0F12, 0xB082 },
	{ 0x0F12, 0x4D96 },
	{ 0x0F12, 0x4E91 },
	{ 0x0F12, 0x88A8 },
	{ 0x0F12, 0x1C2C },
	{ 0x0F12, 0x3420 },
	{ 0x0F12, 0x4F90 },
	{ 0x0F12, 0x2800 },
	{ 0x0F12, 0xD018 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF988 },
	{ 0x0F12, 0x9001 },
	{ 0x0F12, 0x9802 },
	{ 0x0F12, 0x6B39 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF974 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF978 },
	{ 0x0F12, 0x9901 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF95F },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0x8871 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF96A },
	{ 0x0F12, 0x0400 },
	{ 0x0F12, 0x0C00 },
	{ 0x0F12, 0x21FF },
	{ 0x0F12, 0x3101 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF97A },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0x88E8 },
	{ 0x0F12, 0x2800 },
	{ 0x0F12, 0xD00A },
	{ 0x0F12, 0x4980 },
	{ 0x0F12, 0x8820 },
	{ 0x0F12, 0x3128 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF979 },
	{ 0x0F12, 0x8D38 },
	{ 0x0F12, 0x8871 },
	{ 0x0F12, 0x4348 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0x0C00 },
	{ 0x0F12, 0x8538 },
	{ 0x0F12, 0xBCFE },
	{ 0x0F12, 0xBC08 },
	{ 0x0F12, 0x4718 },
	{ 0x0F12, 0xB510 },
	{ 0x0F12, 0x1C04 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF974 },
	{ 0x0F12, 0x6821 },
	{ 0x0F12, 0x0409 },
	{ 0x0F12, 0x0C09 },
	{ 0x0F12, 0x1A40 },
	{ 0x0F12, 0x4976 },
	{ 0x0F12, 0x6849 },
	{ 0x0F12, 0x4281 },
	{ 0x0F12, 0xD800 },
	{ 0x0F12, 0x1C08 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF971 },
	{ 0x0F12, 0x6020 },
	{ 0x0F12, 0xE75B },
	{ 0x0F12, 0xB570 },
	{ 0x0F12, 0x6801 },
	{ 0x0F12, 0x040D },
	{ 0x0F12, 0x0C2D },
	{ 0x0F12, 0x6844 },
	{ 0x0F12, 0x486F },
	{ 0x0F12, 0x8981 },
	{ 0x0F12, 0x1C28 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF927 },
	{ 0x0F12, 0x8060 },
	{ 0x0F12, 0x4970 },
	{ 0x0F12, 0x69C9 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF968 },
	{ 0x0F12, 0x1C01 },
	{ 0x0F12, 0x80A0 },
	{ 0x0F12, 0x0228 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF92D },
	{ 0x0F12, 0x0400 },
	{ 0x0F12, 0x0C00 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0x496B },
	{ 0x0F12, 0x2300 },
	{ 0x0F12, 0x5EC9 },
	{ 0x0F12, 0x4288 },
	{ 0x0F12, 0xDA02 },
	{ 0x0F12, 0x20FF },
	{ 0x0F12, 0x3001 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0xE797 },
	{ 0x0F12, 0xB5F8 },
	{ 0x0F12, 0x1C04 },
	{ 0x0F12, 0x4867 },
	{ 0x0F12, 0x4E65 },
	{ 0x0F12, 0x7800 },
	{ 0x0F12, 0x6AB7 },
	{ 0x0F12, 0x2800 },
	{ 0x0F12, 0xD100 },
	{ 0x0F12, 0x6A37 },
	{ 0x0F12, 0x495D },
	{ 0x0F12, 0x2800 },
	{ 0x0F12, 0x688D },
	{ 0x0F12, 0xD100 },
	{ 0x0F12, 0x684D },
	{ 0x0F12, 0x4859 },
	{ 0x0F12, 0x8841 },
	{ 0x0F12, 0x6820 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF94B },
	{ 0x0F12, 0x8DF1 },
	{ 0x0F12, 0x434F },
	{ 0x0F12, 0x0A3A },
	{ 0x0F12, 0x4282 },
	{ 0x0F12, 0xD30C },
	{ 0x0F12, 0x4D5C },
	{ 0x0F12, 0x26FF },
	{ 0x0F12, 0x8829 },
	{ 0x0F12, 0x3601 },
	{ 0x0F12, 0x43B1 },
	{ 0x0F12, 0x8029 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF944 },
	{ 0x0F12, 0x6020 },
	{ 0x0F12, 0x8828 },
	{ 0x0F12, 0x4330 },
	{ 0x0F12, 0x8028 },
	{ 0x0F12, 0xE73B },
	{ 0x0F12, 0x1C0A },
	{ 0x0F12, 0x436A },
	{ 0x0F12, 0x0A12 },
	{ 0x0F12, 0x4282 },
	{ 0x0F12, 0xD304 },
	{ 0x0F12, 0x0200 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF8F3 },
	{ 0x0F12, 0x6020 },
	{ 0x0F12, 0xE7F4 },
	{ 0x0F12, 0x6025 },
	{ 0x0F12, 0xE7F2 },
	{ 0x0F12, 0xB410 },
	{ 0x0F12, 0x4848 },
	{ 0x0F12, 0x4950 },
	{ 0x0F12, 0x89C0 },
	{ 0x0F12, 0x2316 },
	{ 0x0F12, 0x5ECC },
	{ 0x0F12, 0x1C02 },
	{ 0x0F12, 0x42A0 },
	{ 0x0F12, 0xDC00 },
	{ 0x0F12, 0x1C22 },
	{ 0x0F12, 0x82CA },
	{ 0x0F12, 0x2318 },
	{ 0x0F12, 0x5ECA },
	{ 0x0F12, 0x4290 },
	{ 0x0F12, 0xDC00 },
	{ 0x0F12, 0x1C10 },
	{ 0x0F12, 0x8308 },
	{ 0x0F12, 0xBC10 },
	{ 0x0F12, 0x4770 },
	{ 0x0F12, 0xB570 },
	{ 0x0F12, 0x1C06 },
	{ 0x0F12, 0x4C45 },
	{ 0x0F12, 0x2501 },
	{ 0x0F12, 0x8820 },
	{ 0x0F12, 0x02AD },
	{ 0x0F12, 0x43A8 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF91E },
	{ 0x0F12, 0x6030 },
	{ 0x0F12, 0xF7FF },
	{ 0x0F12, 0xFFE0 },
	{ 0x0F12, 0x8820 },
	{ 0x0F12, 0x4328 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0xE741 },
	{ 0x0F12, 0xB570 },
	{ 0x0F12, 0x4C3D },
	{ 0x0F12, 0x2501 },
	{ 0x0F12, 0x8820 },
	{ 0x0F12, 0x02ED },
	{ 0x0F12, 0x43A8 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF916 },
	{ 0x0F12, 0xF7FF },
	{ 0x0F12, 0xFFD1 },
	{ 0x0F12, 0x8820 },
	{ 0x0F12, 0x4328 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0xE732 },
	{ 0x0F12, 0x230D },
	{ 0x0F12, 0x071B },
	{ 0x0F12, 0x18C3 },
	{ 0x0F12, 0x8818 },
	{ 0x0F12, 0x2A00 },
	{ 0x0F12, 0xD001 },
	{ 0x0F12, 0x4308 },
	{ 0x0F12, 0xE000 },
	{ 0x0F12, 0x4388 },
	{ 0x0F12, 0x8018 },
	{ 0x0F12, 0x4770 },
	{ 0x0F12, 0xB570 },
	{ 0x0F12, 0x2402 },
	{ 0x0F12, 0x4932 },
	{ 0x0F12, 0x8809 },
	{ 0x0F12, 0x078A },
	{ 0x0F12, 0xD500 },
	{ 0x0F12, 0x2406 },
	{ 0x0F12, 0x2900 },
	{ 0x0F12, 0xD01F },
	{ 0x0F12, 0x1C02 },
	{ 0x0F12, 0x207D },
	{ 0x0F12, 0x00C0 },
	{ 0x0F12, 0x2600 },
	{ 0x0F12, 0x4D2D },
	{ 0x0F12, 0x2A00 },
	{ 0x0F12, 0xD019 },
	{ 0x0F12, 0x2101 },
	{ 0x0F12, 0x8229 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF8F9 },
	{ 0x0F12, 0x2200 },
	{ 0x0F12, 0x2101 },
	{ 0x0F12, 0x482A },
	{ 0x0F12, 0x0309 },
	{ 0x0F12, 0xF7FF },
	{ 0x0F12, 0xFFDB },
	{ 0x0F12, 0x2008 },
	{ 0x0F12, 0x4304 },
	{ 0x0F12, 0x1C21 },
	{ 0x0F12, 0x4C26 },
	{ 0x0F12, 0x2200 },
	{ 0x0F12, 0x3C14 },
	{ 0x0F12, 0x1C20 },
	{ 0x0F12, 0xF7FF },
	{ 0x0F12, 0xFFD2 },
	{ 0x0F12, 0x2200 },
	{ 0x0F12, 0x2121 },
	{ 0x0F12, 0x1C20 },
	{ 0x0F12, 0xF7FF },
	{ 0x0F12, 0xFFCD },
	{ 0x0F12, 0x802E },
	{ 0x0F12, 0xE6FD },
	{ 0x0F12, 0x822E },
	{ 0x0F12, 0x0789 },
	{ 0x0F12, 0x0FC9 },
	{ 0x0F12, 0x0089 },
	{ 0x0F12, 0x223B },
	{ 0x0F12, 0x4311 },
	{ 0x0F12, 0x8029 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF8DA },
	{ 0x0F12, 0xE7F4 },
	{ 0x0F12, 0xB510 },
	{ 0x0F12, 0x491B },
	{ 0x0F12, 0x8FC8 },
	{ 0x0F12, 0x2800 },
	{ 0x0F12, 0xD007 },
	{ 0x0F12, 0x2000 },
	{ 0x0F12, 0x87C8 },
	{ 0x0F12, 0x8F88 },
	{ 0x0F12, 0x4C19 },
	{ 0x0F12, 0x2800 },
	{ 0x0F12, 0xD002 },
	{ 0x0F12, 0x2008 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0xE689 },
	{ 0x0F12, 0x4815 },
	{ 0x0F12, 0x3060 },
	{ 0x0F12, 0x8900 },
	{ 0x0F12, 0x2800 },
	{ 0x0F12, 0xD103 },
	{ 0x0F12, 0x4814 },
	{ 0x0F12, 0x2101 },
	{ 0x0F12, 0xF000 },
	{ 0x0F12, 0xF8CA },
	{ 0x0F12, 0x2010 },
	{ 0x0F12, 0x8020 },
	{ 0x0F12, 0xE7F2 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x1376 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x2370 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x14D8 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x235C },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0xF4B0 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x1554 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x1AB8 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x0080 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x046C },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x0468 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x1100 },
	{ 0x0F12, 0xD000 },
	{ 0x0F12, 0x198C },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x0AC4 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0xB0A0 },
	{ 0x0F12, 0xD000 },
	{ 0x0F12, 0xB0B4 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x01B8 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x044E },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x0450 },
	{ 0x0F12, 0x7000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x9CE7 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xF004 },
	{ 0x0F12, 0xE51F },
	{ 0x0F12, 0x9FB8 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x14C1 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x27E1 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x88DF },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x275D },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x1ED3 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x27C5 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xF004 },
	{ 0x0F12, 0xE51F },
	{ 0x0F12, 0xA144 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x1F87 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x27A9 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x1ECB },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x28FF },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x26F9 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x4027 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x9F03 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xF004 },
	{ 0x0F12, 0xE51F },
	{ 0x0F12, 0x9D9C },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x285F },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x6181 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x6663 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x85D9 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x4778 },
	{ 0x0F12, 0x46C0 },
	{ 0x0F12, 0xC000 },
	{ 0x0F12, 0xE59F },
	{ 0x0F12, 0xFF1C },
	{ 0x0F12, 0xE12F },
	{ 0x0F12, 0x2001 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0xE848 },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0xE848 },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x0500 },
	{ 0x0F12, 0x0064 },
	{ 0x0F12, 0x0002 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
//
// Parameters Defined in T&P:
// REG_SF_USER_IsoVal                        2 700003EE SHORT
// REG_SF_USER_IsoChanged                    2 700003F0 SHORT
// AWBBTune_EVT4                            20 7000235C STRUCT
// AWBBTune_EVT4_uMinCoarse                  2 7000235C SHORT
// AWBBTune_EVT4_uMinFine                    2 7000235E SHORT
// AWBBTune_EVT4_uMaxExp3                    4 70002360 LONG
// AWBBTune_EVT4_uCapMaxExp3                 4 70002364 LONG
// AWBBTune_EVT4_uMaxAnGain3                 2 70002368 SHORT
// AWBBTune_EVT4_uMinFinalPt                 2 7000236A SHORT
// AWBBTune_EVT4_uInitPostToleranceCnt       2 7000236C SHORT
// AWBB_Mon_EVT3                             4 70002370 STRUCT
// AWBB_Mon_EVT3_uPostToleranceCnt           2 70002370 SHORT
// AWBB_Mon_EVT3_usIsoFixedDigitalGain88     2 70002372 SHORT
//
// End T&P part


// Start tuning part

// Analog Settings
	{ 0xF418, 0x0050 },
	{ 0xF454, 0x0001 },
	{ 0xF43E, 0x0010 },

	{ 0x002A, 0x1106 },
	{ 0x0F12, 0x00F0 },
	{ 0x0F12, 0x00F1 },

	{ 0x002A, 0x112A },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x1132 },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x113E },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x115C },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x1164 },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x1174 },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x1178 },
	{ 0x0F12, 0x0000 },

	{ 0x002A, 0x2170 },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x0090 },

	{ 0x002A, 0x07A2 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0000 },

	{ 0x002A, 0x07B6 },
	{ 0x0F12, 0x0000 },
	{ 0x0F12, 0x0002 },
	{ 0x0F12, 0x0004 },
	{ 0x0F12, 0x0004 },
	{ 0x0F12, 0x0005 },
	{ 0x0F12, 0x0005 },

	{ 0x002A, 0x0104 },   //greg: bypass FW for mode change.
	{ 0x0F12, 0x0C2F },
	{ 0x002A, 0x10EA },
	{ 0x0F12, 0x007C },   

// End tuning part
	{ 0x1000,	0x0001 },	// Set host interrupt so main start run

 //image quality turning start                            
	{ 0x002A, 0x0712 },                                         
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x00B0 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x00C8 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x00D8 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },                                             
	{ 0x0F12, 0x0100 },
	//parawrite _end - TVAR_ash_
                            
	//parawrite _start - TVAR_as                                             
	{ 0x002A, 0x074A },                                        
	{ 0x0F12, 0x00FB },                                            
	{ 0x0F12, 0x00FF },                                            
	{ 0x0F12, 0x00F9 },                                            
	{ 0x0F12, 0x0104 },  
	//parawrite _end - TVAR_ash_     
	                                     
	{ 0x002A, 0x075A },                                        
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x0282 },                                            
	{ 0x0F12, 0x0216 },                                            
	{ 0x0F12, 0x000B },                                            
	{ 0x0F12, 0x000E },    
	//parawrite _start - TVAR_as
                                        
	{ 0x002A, 0x247C },                                        
	{ 0x0F12, 0x02B2 },		                                            
	{ 0x0F12, 0x01C3 },                                            
	{ 0x0F12, 0x0138 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x00E5 },                                            
	{ 0x0F12, 0x00D1 },                                            
	{ 0x0F12, 0x00C6 },                                            
	{ 0x0F12, 0x00CA },                                            
	{ 0x0F12, 0x00DA },                                            
	{ 0x0F12, 0x00EE },                                            
	{ 0x0F12, 0x0110 },                                            
	{ 0x0F12, 0x0179 },                                            
	{ 0x0F12, 0x0232 },                                            
	{ 0x0F12, 0x01EC },                                            
	{ 0x0F12, 0x0148 },                                            
	{ 0x0F12, 0x00F3 },                                            
	{ 0x0F12, 0x00C7 },                                            
	{ 0x0F12, 0x00A3 },                                            
	{ 0x0F12, 0x0089 },                                            
	{ 0x0F12, 0x007A },                                            
	{ 0x0F12, 0x0081 },                                            
	{ 0x0F12, 0x0093 },                                            
	{ 0x0F12, 0x00AF },                                            
	{ 0x0F12, 0x00CF },                                            
	{ 0x0F12, 0x010A },                                            
	{ 0x0F12, 0x0181 },                                            
	{ 0x0F12, 0x015D },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x00B5 },                                            
	{ 0x0F12, 0x0083 },                                            
	{ 0x0F12, 0x0058 },                                            
	{ 0x0F12, 0x003F },                                            
	{ 0x0F12, 0x0036 },                                            
	{ 0x0F12, 0x0038 },                                            
	{ 0x0F12, 0x0048 },                                            
	{ 0x0F12, 0x0065 },                                            
	{ 0x0F12, 0x008E },                                            
	{ 0x0F12, 0x00C0 },                                            
	{ 0x0F12, 0x010A },                                            
	{ 0x0F12, 0x0119 },                                            
	{ 0x0F12, 0x00C7 },                                            
	{ 0x0F12, 0x008A },                                            
	{ 0x0F12, 0x0056 },                                            
	{ 0x0F12, 0x0030 },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x0012 },                                            
	{ 0x0F12, 0x0011 },                                            
	{ 0x0F12, 0x001C },                                            
	{ 0x0F12, 0x0036 },                                            
	{ 0x0F12, 0x005F },                                            
	{ 0x0F12, 0x0096 },                                            
	{ 0x0F12, 0x00D2 },                                            
	{ 0x0F12, 0x00FA },                                            
	{ 0x0F12, 0x00B7 },                                            
	{ 0x0F12, 0x0073 },                                            
	{ 0x0F12, 0x003D },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x000C },                                            
	{ 0x0F12, 0x0004 },                                            
	{ 0x0F12, 0x0004 },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x001C },                                            
	{ 0x0F12, 0x0045 },                                            
	{ 0x0F12, 0x0083 },                                            
	{ 0x0F12, 0x00BA },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x00B2 },                                            
	{ 0x0F12, 0x006B },                                            
	{ 0x0F12, 0x0034 },                                            
	{ 0x0F12, 0x0016 },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0018 },                                            
	{ 0x0F12, 0x003F },                                            
	{ 0x0F12, 0x0080 },                                            
	{ 0x0F12, 0x00BA },                                            
	{ 0x0F12, 0x00FD },                                            
	{ 0x0F12, 0x00BE },                                            
	{ 0x0F12, 0x0075 },                                            
	{ 0x0F12, 0x003D },                                            
	{ 0x0F12, 0x001A },                                            
	{ 0x0F12, 0x000B },                                            
	{ 0x0F12, 0x0003 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x000D },                                            
	{ 0x0F12, 0x0022 },                                            
	{ 0x0F12, 0x004C },                                            
	{ 0x0F12, 0x008E },                                            
	{ 0x0F12, 0x00CB },                                            
	{ 0x0F12, 0x0121 },                                            
	{ 0x0F12, 0x00DB },                                            
	{ 0x0F12, 0x0096 },                                            
	{ 0x0F12, 0x0057 },                                            
	{ 0x0F12, 0x002C },                                            
	{ 0x0F12, 0x0016 },                                            
	{ 0x0F12, 0x000F },                                            
	{ 0x0F12, 0x0011 },                                            
	{ 0x0F12, 0x001E },                                            
	{ 0x0F12, 0x003B },                                            
	{ 0x0F12, 0x006D },                                            
	{ 0x0F12, 0x00AE },                                            
	{ 0x0F12, 0x00F0 },                                            
	{ 0x0F12, 0x0163 },                                            
	{ 0x0F12, 0x0107 },                                            
	{ 0x0F12, 0x00C6 },                                            
	{ 0x0F12, 0x0085 },                                            
	{ 0x0F12, 0x0053 },                                            
	{ 0x0F12, 0x0034 },                                            
	{ 0x0F12, 0x0029 },                                            
	{ 0x0F12, 0x002F },                                            
	{ 0x0F12, 0x0042 },                                            
	{ 0x0F12, 0x0066 },                                            
	{ 0x0F12, 0x009E },                                            
	{ 0x0F12, 0x00DC },                                            
	{ 0x0F12, 0x012D },                                            
	{ 0x0F12, 0x01E1 },                                            
	{ 0x0F12, 0x014C },                                            
	{ 0x0F12, 0x0102 },                                            
	{ 0x0F12, 0x00CA },                                            
	{ 0x0F12, 0x0096 },                                            
	{ 0x0F12, 0x0072 },                                            
	{ 0x0F12, 0x0062 },                                            
	{ 0x0F12, 0x0068 },                                            
	{ 0x0F12, 0x007F },                                            
	{ 0x0F12, 0x00A9 },                                            
	{ 0x0F12, 0x00D7 },                                            
	{ 0x0F12, 0x011B },                                            
	{ 0x0F12, 0x0196 },                                            
	{ 0x0F12, 0x029C },                                            
	{ 0x0F12, 0x01C0 },                                            
	{ 0x0F12, 0x0144 },                                            
	{ 0x0F12, 0x0108 },                                            
	{ 0x0F12, 0x00DE },                                            
	{ 0x0F12, 0x00BB },                                            
	{ 0x0F12, 0x00AB },                                            
	{ 0x0F12, 0x00AC },                                            
	{ 0x0F12, 0x00C7 },                                            
	{ 0x0F12, 0x00E8 },                                            
	{ 0x0F12, 0x011A },                                            
	{ 0x0F12, 0x017B },                                            
	{ 0x0F12, 0x0222 },                                            
	{ 0x0F12, 0x0281 },                                            
	{ 0x0F12, 0x019C },                                            
	{ 0x0F12, 0x011A },                                            
	{ 0x0F12, 0x00E7 },                                            
	{ 0x0F12, 0x00CF },                                            
	{ 0x0F12, 0x00BE },                                            
	{ 0x0F12, 0x00B3 },                                            
	{ 0x0F12, 0x00B2 },                                            
	{ 0x0F12, 0x00BA },                                            
	{ 0x0F12, 0x00C7 },                                            
	{ 0x0F12, 0x00E0 },                                            
	{ 0x0F12, 0x0139 },                                            
	{ 0x0F12, 0x01E4 },                                            
	{ 0x0F12, 0x01B4 },                                            
	{ 0x0F12, 0x011D },                                            
	{ 0x0F12, 0x00D8 },                                            
	{ 0x0F12, 0x00B4 },                                            
	{ 0x0F12, 0x0093 },                                            
	{ 0x0F12, 0x007B },                                            
	{ 0x0F12, 0x0070 },                                            
	{ 0x0F12, 0x0072 },                                            
	{ 0x0F12, 0x007F },                                            
	{ 0x0F12, 0x0091 },                                            
	{ 0x0F12, 0x00A9 },                                            
	{ 0x0F12, 0x00D6 },                                            
	{ 0x0F12, 0x0142 },                                            
	{ 0x0F12, 0x013A },                                            
	{ 0x0F12, 0x00D3 },                                            
	{ 0x0F12, 0x00AA },                                            
	{ 0x0F12, 0x007C },                                            
	{ 0x0F12, 0x0055 },                                            
	{ 0x0F12, 0x003D },                                            
	{ 0x0F12, 0x0035 },                                            
	{ 0x0F12, 0x0036 },                                            
	{ 0x0F12, 0x0044 },                                            
	{ 0x0F12, 0x005B },                                            
	{ 0x0F12, 0x007A },                                            
	{ 0x0F12, 0x009E },                                            
	{ 0x0F12, 0x00DF },                                            
	{ 0x0F12, 0x00F9 },                                            
	{ 0x0F12, 0x00B5 },                                            
	{ 0x0F12, 0x0083 },                                            
	{ 0x0F12, 0x0052 },                                            
	{ 0x0F12, 0x002D },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x0013 },                                            
	{ 0x0F12, 0x0012 },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x0031 },                                            
	{ 0x0F12, 0x0055 },                                            
	{ 0x0F12, 0x007F },                                            
	{ 0x0F12, 0x00AF },                                            
	{ 0x0F12, 0x00E0 },                                            
	{ 0x0F12, 0x00A6 },                                            
	{ 0x0F12, 0x006C },                                            
	{ 0x0F12, 0x0039 },                                            
	{ 0x0F12, 0x001A },                                            
	{ 0x0F12, 0x000D },                                            
	{ 0x0F12, 0x0007 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x0018 },                                            
	{ 0x0F12, 0x003D },                                            
	{ 0x0F12, 0x0070 },                                            
	{ 0x0F12, 0x009C },                                            
	{ 0x0F12, 0x00DA },                                            
	{ 0x0F12, 0x00A2 },                                            
	{ 0x0F12, 0x0065 },                                            
	{ 0x0F12, 0x0031 },                                            
	{ 0x0F12, 0x0015 },                                            
	{ 0x0F12, 0x0009 },                                            
	{ 0x0F12, 0x0003 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0014 },                                            
	{ 0x0F12, 0x0038 },                                            
	{ 0x0F12, 0x006D },                                            
	{ 0x0F12, 0x009C },                                            
	{ 0x0F12, 0x00DF },                                            
	{ 0x0F12, 0x00A8 },                                            
	{ 0x0F12, 0x006B },                                            
	{ 0x0F12, 0x0038 },                                            
	{ 0x0F12, 0x0019 },                                            
	{ 0x0F12, 0x000C },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x000B },                                            
	{ 0x0F12, 0x001D },                                            
	{ 0x0F12, 0x0043 },                                            
	{ 0x0F12, 0x0075 },                                            
	{ 0x0F12, 0x00A6 },                                            
	{ 0x0F12, 0x00FA },                                            
	{ 0x0F12, 0x00BE },                                            
	{ 0x0F12, 0x0087 },                                            
	{ 0x0F12, 0x004F },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0016 },                                            
	{ 0x0F12, 0x000F },                                            
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x001A },                                            
	{ 0x0F12, 0x0033 },                                            
	{ 0x0F12, 0x005D },                                            
	{ 0x0F12, 0x008F },                                            
	{ 0x0F12, 0x00C2 },                                            
	{ 0x0F12, 0x0132 },                                            
	{ 0x0F12, 0x00DF },                                            
	{ 0x0F12, 0x00B0 },                                            
	{ 0x0F12, 0x0077 },                                            
	{ 0x0F12, 0x004A },                                            
	{ 0x0F12, 0x0031 },                                            
	{ 0x0F12, 0x0027 },                                            
	{ 0x0F12, 0x002B },                                            
	{ 0x0F12, 0x003A },                                            
	{ 0x0F12, 0x0057 },                                            
	{ 0x0F12, 0x0083 },                                            
	{ 0x0F12, 0x00B0 },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x019B },                                            
	{ 0x0F12, 0x0117 },                                            
	{ 0x0F12, 0x00D9 },                                            
	{ 0x0F12, 0x00B0 },                                            
	{ 0x0F12, 0x0085 },                                            
	{ 0x0F12, 0x0067 },                                            
	{ 0x0F12, 0x0059 },                                            
	{ 0x0F12, 0x005C },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x008D },                                            
	{ 0x0F12, 0x00AE },                                            
	{ 0x0F12, 0x00DE },                                            
	{ 0x0F12, 0x0146 },                                            
	{ 0x0F12, 0x0249 },                                            
	{ 0x0F12, 0x017C },                                            
	{ 0x0F12, 0x010F },                                            
	{ 0x0F12, 0x00DF },                                            
	{ 0x0F12, 0x00C0 },                                            
	{ 0x0F12, 0x00A6 },                                            
	{ 0x0F12, 0x0095 },                                            
	{ 0x0F12, 0x0096 },                                            
	{ 0x0F12, 0x00A8 },                                            
	{ 0x0F12, 0x00C0 },                                            
	{ 0x0F12, 0x00E3 },                                            
	{ 0x0F12, 0x012E },                                            
	{ 0x0F12, 0x01BF },                                            
	{ 0x0F12, 0x0289 },                                            
	{ 0x0F12, 0x019B },                                            
	{ 0x0F12, 0x0116 },                                            
	{ 0x0F12, 0x00DE },                                            
	{ 0x0F12, 0x00C0 },                                            
	{ 0x0F12, 0x00A9 },                                            
	{ 0x0F12, 0x009D },                                            
	{ 0x0F12, 0x00A4 },                                            
	{ 0x0F12, 0x00B8 },                                            
	{ 0x0F12, 0x00D8 },                                            
	{ 0x0F12, 0x0106 },                                            
	{ 0x0F12, 0x0175 },                                            
	{ 0x0F12, 0x0239 },                                            
	{ 0x0F12, 0x01C5 },                                            
	{ 0x0F12, 0x0125 },                                            
	{ 0x0F12, 0x00D9 },                                            
	{ 0x0F12, 0x00B2 },                                            
	{ 0x0F12, 0x008D },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x0062 },                                            
	{ 0x0F12, 0x006A },                                            
	{ 0x0F12, 0x0084 },                                            
	{ 0x0F12, 0x00A8 },                                            
	{ 0x0F12, 0x00CD },                                            
	{ 0x0F12, 0x010D },                                            
	{ 0x0F12, 0x0189 },                                            
	{ 0x0F12, 0x0143 },                                            
	{ 0x0F12, 0x00DF },                                            
	{ 0x0F12, 0x00AF },                                            
	{ 0x0F12, 0x007D },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x0036 },                                            
	{ 0x0F12, 0x002D },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0048 },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x009F },                                            
	{ 0x0F12, 0x00CF },                                            
	{ 0x0F12, 0x0118 },                                            
	{ 0x0F12, 0x010C },                                            
	{ 0x0F12, 0x00C3 },                                            
	{ 0x0F12, 0x008C },                                            
	{ 0x0F12, 0x0056 },                                            
	{ 0x0F12, 0x002D },                                            
	{ 0x0F12, 0x0017 },                                            
	{ 0x0F12, 0x000D },                                            
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x001F },                                            
	{ 0x0F12, 0x0040 },                                            
	{ 0x0F12, 0x0070 },                                            
	{ 0x0F12, 0x00A6 },                                            
	{ 0x0F12, 0x00DB },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x00B6 },                                            
	{ 0x0F12, 0x0078 },                                            
	{ 0x0F12, 0x003E },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x000B },                                            
	{ 0x0F12, 0x0004 },                                            
	{ 0x0F12, 0x0003 },                                            
	{ 0x0F12, 0x0009 },                                            
	{ 0x0F12, 0x001F },                                            
	{ 0x0F12, 0x004B },                                            
	{ 0x0F12, 0x0088 },                                            
	{ 0x0F12, 0x00B6 },                                            
	{ 0x0F12, 0x00EA },                                            
	{ 0x0F12, 0x00B4 },                                            
	{ 0x0F12, 0x0070 },                                            
	{ 0x0F12, 0x0037 },                                            
	{ 0x0F12, 0x0016 },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x0013 },                                            
	{ 0x0F12, 0x0038 },                                            
	{ 0x0F12, 0x0071 },                                            
	{ 0x0F12, 0x00A0 },                                            
	{ 0x0F12, 0x00F1 },                                            
	{ 0x0F12, 0x00B8 },                                            
	{ 0x0F12, 0x0076 },                                            
	{ 0x0F12, 0x003E },                                            
	{ 0x0F12, 0x001C },                                            
	{ 0x0F12, 0x000B },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0004 },                                            
	{ 0x0F12, 0x0014 },                                            
	{ 0x0F12, 0x0037 },                                            
	{ 0x0F12, 0x0068 },                                            
	{ 0x0F12, 0x0095 },                                            
	{ 0x0F12, 0x010B },                                            
	{ 0x0F12, 0x00CC },                                            
	{ 0x0F12, 0x0093 },                                            
	{ 0x0F12, 0x0056 },                                            
	{ 0x0F12, 0x002B },                                            
	{ 0x0F12, 0x0015 },                                            
	{ 0x0F12, 0x000B },                                            
	{ 0x0F12, 0x0009 },                                            
	{ 0x0F12, 0x000E },                                            
	{ 0x0F12, 0x0021 },                                            
	{ 0x0F12, 0x0043 },                                            
	{ 0x0F12, 0x0070 },                                            
	{ 0x0F12, 0x00A0 },                                            
	{ 0x0F12, 0x0143 },                                            
	{ 0x0F12, 0x00EB },                                            
	{ 0x0F12, 0x00B8 },                                            
	{ 0x0F12, 0x007E },                                            
	{ 0x0F12, 0x004E },                                            
	{ 0x0F12, 0x002F },                                            
	{ 0x0F12, 0x0021 },                                            
	{ 0x0F12, 0x0020 },                                            
	{ 0x0F12, 0x0027 },                                            
	{ 0x0F12, 0x003D },                                            
	{ 0x0F12, 0x005D },                                            
	{ 0x0F12, 0x0084 },                                            
	{ 0x0F12, 0x00BD },                                            
	{ 0x0F12, 0x01AD },                                            
	{ 0x0F12, 0x0122 },                                            
	{ 0x0F12, 0x00E3 },                                            
	{ 0x0F12, 0x00B5 },                                            
	{ 0x0F12, 0x0087 },                                            
	{ 0x0F12, 0x0064 },                                            
	{ 0x0F12, 0x0051 },                                            
	{ 0x0F12, 0x004E },                                            
	{ 0x0F12, 0x0057 },                                            
	{ 0x0F12, 0x006A },                                            
	{ 0x0F12, 0x007F },                                            
	{ 0x0F12, 0x00A8 },                                            
	{ 0x0F12, 0x0101 },                                            
	{ 0x0F12, 0x0267 },                                            
	{ 0x0F12, 0x018C },                                            
	{ 0x0F12, 0x0119 },                                            
	{ 0x0F12, 0x00E5 },                                            
	{ 0x0F12, 0x00C2 },                                            
	{ 0x0F12, 0x00A2 },                                            
	{ 0x0F12, 0x008D },                                            
	{ 0x0F12, 0x0086 },                                            
	{ 0x0F12, 0x008C },                                            
	{ 0x0F12, 0x0099 },                                            
	{ 0x0F12, 0x00B0 },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x016C },                                            
	{ 0x0F12, 0x01F3 },                                            
	{ 0x0F12, 0x0136 },                                            
	{ 0x0F12, 0x00D6 },                                            
	{ 0x0F12, 0x00B3 },                                            
	{ 0x0F12, 0x00A1 },                                            
	{ 0x0F12, 0x0095 },                                            
	{ 0x0F12, 0x008E },                                            
	{ 0x0F12, 0x0098 },                                            
	{ 0x0F12, 0x00AD },                                            
	{ 0x0F12, 0x00C5 },                                            
	{ 0x0F12, 0x00ED },                                            
	{ 0x0F12, 0x014D },                                            
	{ 0x0F12, 0x0207 },                                            
	{ 0x0F12, 0x014C },                                            
	{ 0x0F12, 0x00D1 },                                            
	{ 0x0F12, 0x00A4 },                                            
	{ 0x0F12, 0x0091 },                                            
	{ 0x0F12, 0x0077 },                                            
	{ 0x0F12, 0x0062 },                                            
	{ 0x0F12, 0x005E },                                            
	{ 0x0F12, 0x006A },                                            
	{ 0x0F12, 0x0081 },                                            
	{ 0x0F12, 0x009F },                                            
	{ 0x0F12, 0x00BE },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x0162 },                                            
	{ 0x0F12, 0x00DB },                                            
	{ 0x0F12, 0x008C },                                            
	{ 0x0F12, 0x0079 },                                            
	{ 0x0F12, 0x005D },                                            
	{ 0x0F12, 0x003D },                                            
	{ 0x0F12, 0x002B },                                                        
	{ 0x0F12, 0x002B },                                            
	{ 0x0F12, 0x0033 },                                            
	{ 0x0F12, 0x004A },                                            
	{ 0x0F12, 0x006A },                                            
	{ 0x0F12, 0x0092 },                                            
	{ 0x0F12, 0x00B2 },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x00A2 },                                            
	{ 0x0F12, 0x0072 },                                            
	{ 0x0F12, 0x0059 },                                            
	{ 0x0F12, 0x003A },                                            
	{ 0x0F12, 0x001E },                                            
	{ 0x0F12, 0x0011 },                                            
	{ 0x0F12, 0x000F },                                            
	{ 0x0F12, 0x0012 },                                            
	{ 0x0F12, 0x0020 },                                            
	{ 0x0F12, 0x003B },                                            
	{ 0x0F12, 0x005E },                                            
	{ 0x0F12, 0x0084 },                                            
	{ 0x0F12, 0x00AD },                                            
	{ 0x0F12, 0x008B },                                            
	{ 0x0F12, 0x0065 },                                            
	{ 0x0F12, 0x0045 },                                            
	{ 0x0F12, 0x0024 },                                            
	{ 0x0F12, 0x000F },                                            
	{ 0x0F12, 0x0007 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x0017 },                                            
	{ 0x0F12, 0x0037 },                                            
	{ 0x0F12, 0x0060 },                                            
	{ 0x0F12, 0x0083 },                                            
	{ 0x0F12, 0x0081 },                                            
	{ 0x0F12, 0x0060 },                                            
	{ 0x0F12, 0x003D },                                            
	{ 0x0F12, 0x001D },                                            
	{ 0x0F12, 0x000C },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x0009 },                                            
	{ 0x0F12, 0x0022 },                                            
	{ 0x0F12, 0x0047 },                                            
	{ 0x0F12, 0x0068 },                                            
	{ 0x0F12, 0x0084 },                                            
	{ 0x0F12, 0x0064 },                                            
	{ 0x0F12, 0x0042 },                                            
	{ 0x0F12, 0x0023 },                                            
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x0007 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x001C },                                            
	{ 0x0F12, 0x0039 },                                            
	{ 0x0F12, 0x005B },                                            
	{ 0x0F12, 0x009C },                                            
	{ 0x0F12, 0x0076 },                                            
	{ 0x0F12, 0x005B },                                            
	{ 0x0F12, 0x0037 },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x000F },                                            
	{ 0x0F12, 0x0009 },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x0009 },                                            
	{ 0x0F12, 0x0011 },                                            
	{ 0x0F12, 0x0025 },                                            
	{ 0x0F12, 0x003E },                                            
	{ 0x0F12, 0x005F },                                            
	{ 0x0F12, 0x00D0 },                                            
	{ 0x0F12, 0x0095 },                                            
	{ 0x0F12, 0x007E },                                            
	{ 0x0F12, 0x005C },                                            
	{ 0x0F12, 0x003A },                                            
	{ 0x0F12, 0x0025 },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x001B },                                            
	{ 0x0F12, 0x001E },                                            
	{ 0x0F12, 0x0027 },                                            
	{ 0x0F12, 0x003A },                                            
	{ 0x0F12, 0x004F },                                            
	{ 0x0F12, 0x007B },                                            
	{ 0x0F12, 0x012F },                                            
	{ 0x0F12, 0x00C8 },                                            
	{ 0x0F12, 0x00A7 },                                            
	{ 0x0F12, 0x008E },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x0057 },                                            
	{ 0x0F12, 0x0048 },                                            
	{ 0x0F12, 0x0047 },                                            
	{ 0x0F12, 0x0049 },                                            
	{ 0x0F12, 0x004F },                                            
	{ 0x0F12, 0x0058 },                                            
	{ 0x0F12, 0x006E },                                            
	{ 0x0F12, 0x00B9 },                                            
	{ 0x0F12, 0x01CB },                                            
	{ 0x0F12, 0x0123 },                                            
	{ 0x0F12, 0x00D5 },                                            
	{ 0x0F12, 0x00B9 },                                            
	{ 0x0F12, 0x00A2 },                                            
	{ 0x0F12, 0x008E },                                            
	{ 0x0F12, 0x0080 },                                            
	{ 0x0F12, 0x007B },                                            
	{ 0x0F12, 0x0079 },                                            
	{ 0x0F12, 0x0078 },                                            
	{ 0x0F12, 0x0081 },                                            
	{ 0x0F12, 0x00A9 },                                            
	{ 0x0F12, 0x0108 },
	//parawrite _end - TVAR_ash_           
                                           
	{ 0x002A, 0x0C48 },                                         
	{ 0x0F12, 0x0520 },                                            
	{ 0x0F12, 0x0400 },                                            
	{ 0x0F12, 0x0660 },

	//param_start - TVAR_ash_Awb                                             
	{ 0x002A, 0x0704 },                                         
	{ 0x0F12, 0x00ED },                                            
	{ 0x0F12, 0x0124 },                                            
	{ 0x0F12, 0x012B },                                            
	{ 0x0F12, 0x014A },                                            
	{ 0x0F12, 0x0190 },                                            
	{ 0x0F12, 0x01B2 },                                            
	{ 0x0F12, 0x01C4 },  
	//param_end - TVAR_ash_AwbAs  
	                                        
	{ 0x002A, 0x0754 },                                        
	{ 0x0F12, 0x247C },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x002A, 0x0E1A },                                        
	{ 0x0F12, 0x0138 },                                            
	//AWB Speed //                                     
	{ 0x002A, 0x0E7C },                                         
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x0003 },                                            
                                                         
	//param_start - awbb_IndoorG                       
	{ 0x002A, 0x0C50 },                                        
	{ 0x0F12, 0x03BA },                                            
	{ 0x0F12, 0x03E3 },                                            
	{ 0x0F12, 0x039A },                                            
	{ 0x0F12, 0x03DB },                                            
	{ 0x0F12, 0x037B },                                            
	{ 0x0F12, 0x03CD },                                            
	{ 0x0F12, 0x035D },                                            
	{ 0x0F12, 0x03B2 },                                            
	{ 0x0F12, 0x0342 },                                            
	{ 0x0F12, 0x0397 },                                            
	{ 0x0F12, 0x0325 },                                            
	{ 0x0F12, 0x0380 },                                            
	{ 0x0F12, 0x030E },                                            
	{ 0x0F12, 0x0369 },                                            
	{ 0x0F12, 0x02F8 },                                            
	{ 0x0F12, 0x034B },                                            
	{ 0x0F12, 0x02DE },                                            
	{ 0x0F12, 0x0336 },                                            
	{ 0x0F12, 0x02BF },                                            
	{ 0x0F12, 0x031F },                                            
	{ 0x0F12, 0x02A6 },                                            
	{ 0x0F12, 0x0306 },                                            
	{ 0x0F12, 0x028D },                                            
	{ 0x0F12, 0x02F4 },                                            
	{ 0x0F12, 0x027D },                                            
	{ 0x0F12, 0x02DD },                                            
	{ 0x0F12, 0x026C },                                            
	{ 0x0F12, 0x02C2 },                                            
	{ 0x0F12, 0x025C },                                            
	{ 0x0F12, 0x02AE },                                            
	{ 0x0F12, 0x024F },                                            
	{ 0x0F12, 0x029D },                                            
	{ 0x0F12, 0x0245 },                                            
	{ 0x0F12, 0x028B },                                            
	{ 0x0F12, 0x023E },                                            
	{ 0x0F12, 0x027F },                                            
	{ 0x0F12, 0x0235 },                                            
	{ 0x0F12, 0x0272 },                                            
	{ 0x0F12, 0x022B },                                            
	{ 0x0F12, 0x0267 },                                            
	{ 0x0F12, 0x0220 },                                            
	{ 0x0F12, 0x025B },                                            
	{ 0x0F12, 0x0218 },                                            
	{ 0x0F12, 0x0250 },                                            
	{ 0x0F12, 0x020E },                                            
	{ 0x0F12, 0x0246 },                                            
	{ 0x0F12, 0x0206 },                                            
	{ 0x0F12, 0x023D },                                            
	{ 0x0F12, 0x01FB },                                            
	{ 0x0F12, 0x0234 },                                            
	{ 0x0F12, 0x01F1 },                                            
	{ 0x0F12, 0x0229 },                                            
	{ 0x0F12, 0x01E7 },                                            
	{ 0x0F12, 0x0220 },                                            
	{ 0x0F12, 0x01DF },                                            
	{ 0x0F12, 0x0216 },                                            
	{ 0x0F12, 0x01D2 },                                            
	{ 0x0F12, 0x020D },                                            
	{ 0x0F12, 0x01C7 },                                            
	{ 0x0F12, 0x01FD },                                            
	{ 0x0F12, 0x01C4 },                                            
	{ 0x0F12, 0x01EE },                                            
	{ 0x0F12, 0x01D1 },                                            
	{ 0x0F12, 0x01E1 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0004 },                                            
	{ 0x0F12, 0x0000 },
	//param_end - awbb_IndoorGrZ
	                                            
	{ 0x002A, 0x0CF8 },                                        
	{ 0x0F12, 0x010F },                                            
	{ 0x0F12, 0x0000 },   

	//param_start - SARR_usGamma                                          
	{ 0x002A, 0x04C8 },                                        
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x000E },                                            
	{ 0x0F12, 0x002C },                                            
	{ 0x0F12, 0x005D },                                            
	{ 0x0F12, 0x00C5 },                                            
	{ 0x0F12, 0x010F },                                            
	{ 0x0F12, 0x0150 },                                            
	{ 0x0F12, 0x01B8 },                                            
	{ 0x0F12, 0x01FD },                                            
	{ 0x0F12, 0x0269 },                                            
	{ 0x0F12, 0x02C4 },                                            
	{ 0x0F12, 0x031F },                                            
	{ 0x0F12, 0x0364 },                                            
	{ 0x0F12, 0x03AE },                                            
	{ 0x0F12, 0x03FF },
                    
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x000E },                                            
	{ 0x0F12, 0x002C },                                            
	{ 0x0F12, 0x005D },                                            
	{ 0x0F12, 0x00C5 },                                            
	{ 0x0F12, 0x010F },                                            
	{ 0x0F12, 0x0150 },                                            
	{ 0x0F12, 0x01B8 },                                            
	{ 0x0F12, 0x01FD },                                            
	{ 0x0F12, 0x0269 },                                            
	{ 0x0F12, 0x02C4 },                                            
	{ 0x0F12, 0x031F },                                            
	{ 0x0F12, 0x0364 },                                            
	{ 0x0F12, 0x03AE },                                            
	{ 0x0F12, 0x03FF }, 
                           
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0001 },                                            
	{ 0x0F12, 0x000E },                                            
	{ 0x0F12, 0x002C },                                            
	{ 0x0F12, 0x005D },                                            
	{ 0x0F12, 0x00C5 },                                            
	{ 0x0F12, 0x010F },                                            
	{ 0x0F12, 0x0150 },                                            
	{ 0x0F12, 0x01B8 },                                            
	{ 0x0F12, 0x01FD },                                            
	{ 0x0F12, 0x0269 },                                            
	{ 0x0F12, 0x02C4 },                                            
	{ 0x0F12, 0x031F },                                            
	{ 0x0F12, 0x0364 },                                            
	{ 0x0F12, 0x03AE },                                            
	{ 0x0F12, 0x03FF },    
	//param_end - SARR_usGammaLu
	                                        
	{ 0x002A, 0x1000 },                                        
	{ 0x0F12, 0x003F },                                            
	{ 0x002A, 0x0474 },                                        
	{ 0x0F12, 0x0112 },                                            
	{ 0x0F12, 0x00EF },                                            
	{ 0x002A, 0x236C },                                        
	{ 0x0F12, 0x0000 },                                            
	{ 0x002A, 0x1006 },                                        
	{ 0x0F12, 0x001F },                                            
	{ 0x002A, 0x108E },                                        
	{ 0x0F12, 0x00C7 },                                            
	{ 0x0F12, 0x00F7 },                                            
	{ 0x0F12, 0x0107 },                                            
	{ 0x0F12, 0x0142 },                                            
	{ 0x0F12, 0x017A },                                            
	{ 0x0F12, 0x01A0 },                                            
	{ 0x0F12, 0x01B6 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x0100 },

	//param_start - CCM start                                          
	{ 0x002A, 0x23A4 },                                         
	{ 0x0F12, 0x01FA },                                             
	{ 0x0F12, 0xFFB9 }, 	//H                                            
	{ 0x0F12, 0xFFF8 },                                             
	{ 0x0F12, 0x0116 },                                             
	{ 0x0F12, 0x00BD },                                             
	{ 0x0F12, 0xFF38 },                                             
	{ 0x0F12, 0xFF23 },                                             
	{ 0x0F12, 0x01AB },                                             
	{ 0x0F12, 0xFF81 },                                             
	{ 0x0F12, 0xFF0D },                                             
	{ 0x0F12, 0x0169 },                                             
	{ 0x0F12, 0x00DE },                                             
	{ 0x0F12, 0xFFEF },                                             
	{ 0x0F12, 0xFFCA },                                             
	{ 0x0F12, 0x014D },                                             
	{ 0x0F12, 0x01C3 },                                             
	{ 0x0F12, 0xFF7E },                                             
	{ 0x0F12, 0x016F }, 
                               
	{ 0x0F12, 0x01FA }, 	//A                                            
	{ 0x0F12, 0xFFB9 },                                             
	{ 0x0F12, 0xFFF8 },                                             
	{ 0x0F12, 0x0116 },                                             
	{ 0x0F12, 0x00BD },                                             
	{ 0x0F12, 0xFF38 },                                             
	{ 0x0F12, 0xFF23 },                                             
	{ 0x0F12, 0x01AB },                                             
	{ 0x0F12, 0xFF81 },                                             
	{ 0x0F12, 0xFF0D },                                             
	{ 0x0F12, 0x0169 },                                             
	{ 0x0F12, 0x00DE },                                             
	{ 0x0F12, 0xFFEF },                                             
	{ 0x0F12, 0xFFCA },                                             
	{ 0x0F12, 0x014D },                                             
	{ 0x0F12, 0x01C3 },                                             
	{ 0x0F12, 0xFF7E },                                             
	{ 0x0F12, 0x016F },   
                           
	{ 0x0F12, 0x01FA }, 	//WW                                             
	{ 0x0F12, 0xFFB9 },                                             
	{ 0x0F12, 0xFFF8 },                                             
	{ 0x0F12, 0x0116 },                                             
	{ 0x0F12, 0x00BD },                                             
	{ 0x0F12, 0xFF38 },                                             
	{ 0x0F12, 0xFF23 },                                             
	{ 0x0F12, 0x01AB },                                             
	{ 0x0F12, 0xFF81 },                                             
	{ 0x0F12, 0xFF0D },                                             
	{ 0x0F12, 0x0169 },                                             
	{ 0x0F12, 0x00DE },                                             
	{ 0x0F12, 0xFFEF },                                             
	{ 0x0F12, 0xFFCA },                                             
	{ 0x0F12, 0x014D },                                             
	{ 0x0F12, 0x01C3 },                                             
	{ 0x0F12, 0xFF7E },                                             
	{ 0x0F12, 0x016F },
                              
	{ 0x0F12, 0x01FA },	//CW                                              
	{ 0x0F12, 0xFFB9 },                                             
	{ 0x0F12, 0xFFF8 },                                             
	{ 0x0F12, 0x0116 },                                             
	{ 0x0F12, 0x00BD },                                             
	{ 0x0F12, 0xFF38 },                                             
	{ 0x0F12, 0xFF23 },                                             
	{ 0x0F12, 0x01AB },                                             
	{ 0x0F12, 0xFF81 },                                             
	{ 0x0F12, 0xFF0D },                                             
	{ 0x0F12, 0x0169 },                                             
	{ 0x0F12, 0x00DE },                                             
	{ 0x0F12, 0xFFEF },                                             
	{ 0x0F12, 0xFFCA },                                             
	{ 0x0F12, 0x014D },                                             
	{ 0x0F12, 0x01C3 },                                             
	{ 0x0F12, 0xFF7E },                                             
	{ 0x0F12, 0x016F }, 
                                            
	{ 0x0F12, 0x030B },	//d50                                             
	{ 0x0F12, 0xFF9D },                                             
	{ 0x0F12, 0x001C },                                             
	{ 0x0F12, 0x00BD },                                             
	{ 0x0F12, 0x0137 },                                             
	{ 0x0F12, 0xFF22 },                                             
	{ 0x0F12, 0xFDB5 },                                             
	{ 0x0F12, 0x01D2 },                                             
	{ 0x0F12, 0xFF7A },                                             
	{ 0x0F12, 0xFCE1 },                                             
	{ 0x0F12, 0x0273 },                                             
	{ 0x0F12, 0x019A },                                             
	{ 0x0F12, 0x000A },                                             
	{ 0x0F12, 0xFF78 },                                             
	{ 0x0F12, 0x0167 },                                             
	{ 0x0F12, 0x0412 },                                             
	{ 0x0F12, 0xFF60 },                                             
	{ 0x0F12, 0x0215 },
                                             
	{ 0x0F12, 0x0255 },	//D65                                             
	{ 0x0F12, 0xFFA7 },                                            
	{ 0x0F12, 0xFFF4 },                                            
	{ 0x0F12, 0x01D8 },                                            
	{ 0x0F12, 0x0145 },                                            
	{ 0x0F12, 0xFF0B },                                            
	{ 0x0F12, 0xFE8F },                                            
	{ 0x0F12, 0x0216 },                                            
	{ 0x0F12, 0xFF16 },                                            
	{ 0x0F12, 0xFE9F },                                            
	{ 0x0F12, 0x01BA },                                            
	{ 0x0F12, 0x00F5 },                                            
	{ 0x0F12, 0xFF8D },                                            
	{ 0x0F12, 0xFF8B },                                            
	{ 0x0F12, 0x028D },                                            
	{ 0x0F12, 0x0215 },                                            
	{ 0x0F12, 0xFF2E },                                            
	{ 0x0F12, 0x01BD },   
	//param_start - CCM end

	//param_start - TVAR_wbt_pOu                                         
	{ 0x002A, 0x2380 },                                        
	{ 0x0F12, 0x01F2 },                                            
	{ 0x0F12, 0xFFC3 },                                            
	{ 0x0F12, 0xFFE3 },                                            
	{ 0x0F12, 0x00F9 },                                            
	{ 0x0F12, 0x013F },                                            
	{ 0x0F12, 0xFF6E },                                            
	{ 0x0F12, 0xFEBB },                                            
	{ 0x0F12, 0x01F2 },                                            
	{ 0x0F12, 0xFEFA },                                            
	{ 0x0F12, 0xFF37 },                                            
	{ 0x0F12, 0x01A2 },                                            
	{ 0x0F12, 0x0126 },                                            
	{ 0x0F12, 0xFFE0 },                                            
	{ 0x0F12, 0xFFBF },                                            
	{ 0x0F12, 0x01E6 },                                            
	{ 0x0F12, 0x0186 },                                            
	{ 0x0F12, 0xFF4B },                                            
	{ 0x0F12, 0x01B1 }, 
	//param_end - TVAR_wbt_pOutd
	                                           
	{ 0x002A, 0x06D4 },                                        
	{ 0x0F12, 0x2380 },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x002A, 0x06CC },                                        
	{ 0x0F12, 0x23A4 },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x002A, 0x06E8 },                                        
	{ 0x0F12, 0x23A4 },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x0F12, 0x23C8 },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x0F12, 0x23EC },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x0F12, 0x2410 },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x0F12, 0x2434 },                                            
	{ 0x0F12, 0x7000 },                                            
	{ 0x0F12, 0x2458 },                                            
	{ 0x0F12, 0x7000 },
                                          
	{ 0x002A, 0x06DA },                                        
	{ 0x0F12, 0x00BF },                                            
	{ 0x0F12, 0x00E6 },                                            
	{ 0x0F12, 0x00F2 },                                            
	{ 0x0F12, 0x0143 },                                            
	{ 0x0F12, 0x0178 },                                            
	{ 0x0F12, 0x01A3 },  

	//param_start - SARR_uNormBr                                          
	{ 0x002A, 0x07E8 },                                        
	{ 0x0F12, 0x0016 },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0096 },                                            
	{ 0x0F12, 0x01F4 },                                            
	{ 0x0F12, 0x07D0 }, 
	//param_end - SARR_uNormBrIn
	
	//param_start - afit_uNoiseI	                                           
	{ 0x002A, 0x07D0 },                                        
	{ 0x0F12, 0x0030 },                                            
	{ 0x0F12, 0x0046 },                                            
	{ 0x0F12, 0x0088 },                                            
	{ 0x0F12, 0x0205 },                                            
	{ 0x0F12, 0x02BC }, 
	//param_end - afit_uNoiseInd
	                                           
	{ 0x002A, 0x07E6 },                                         
	{ 0x0F12, 0x0000 }, //afit_bUseNoiseI 

	//param_start - TVAR_afit_pB                                           
	{ 0x002A, 0x0828 },                                        
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0021 },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x00FF },                                            
	{ 0x0F12, 0x0129 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0344 },                                            
	{ 0x0F12, 0x033A },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x001E },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0106 },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x0C0F },                                            
	{ 0x0F12, 0x0C1F },                                            
	{ 0x0F12, 0x0303 },                                            
	{ 0x0F12, 0x0303 },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x2828 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x020A },                                            
	{ 0x0F12, 0x0480 },                                            
	{ 0x0F12, 0x0E08 },                                            
	{ 0x0F12, 0x030A },                                            
	{ 0x0F12, 0x0A03 },                                            
	{ 0x0F12, 0x0A11 },                                            
	{ 0x0F12, 0x000F },                                            
	{ 0x0F12, 0x0500 },                                            
	{ 0x0F12, 0x0914 },                                            
	{ 0x0F12, 0x0012 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0A00 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x014C },                                            
	{ 0x0F12, 0x014D },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x6020 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0001 },
                                           
	{ 0x0F12, 0xFFFE },                                            
	{ 0x0F12, 0xFFEC },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x000C },                                            
	{ 0x0F12, 0x000E },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x00FF },                                            
	{ 0x0F12, 0x0129 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0114 },                                            
	{ 0x0F12, 0x020A },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0018 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0106 },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x050F },                                            
	{ 0x0F12, 0x0A1F },                                            
	{ 0x0F12, 0x0203 },                                            
	{ 0x0F12, 0x0303 },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x2828 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x020A },                                            
	{ 0x0F12, 0x0480 },                                            
	{ 0x0F12, 0x0E08 },                                            
	{ 0x0F12, 0x030A },                                            
	{ 0x0F12, 0x1403 },                                            
	{ 0x0F12, 0x0A11 },                                            
	{ 0x0F12, 0x0A0F },                                            
	{ 0x0F12, 0x050A },                                            
	{ 0x0F12, 0x101E },                                            
	{ 0x0F12, 0x101E },                                            
	{ 0x0F12, 0x0A08 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0400 },                                            
	{ 0x0F12, 0x0400 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0A00 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0151 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x6020 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0000 },
                                           
	{ 0x0F12, 0xFFFB },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0014 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x001C },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0106 },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x0205 },                                            
	{ 0x0F12, 0x051E },                                            
	{ 0x0F12, 0x0101 },                                            
	{ 0x0F12, 0x0202 },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x2828 },                                            
	{ 0x0F12, 0x0606 },                                            
	{ 0x0F12, 0x0205 },                                            
	{ 0x0F12, 0x0480 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x1903 },                                            
	{ 0x0F12, 0x1911 },                                            
	{ 0x0F12, 0x0A0F },                                            
	{ 0x0F12, 0x050A },                                            
	{ 0x0F12, 0x2028 },                                            
	{ 0x0F12, 0x2028 },                                            
	{ 0x0F12, 0x0A08 },                                            
	{ 0x0F12, 0x0007 },                                            
	{ 0x0F12, 0x0403 },                                            
	{ 0x0F12, 0x0402 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0203 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0170 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x8050 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0000 },
                                          
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0008 },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0014 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x001C },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0028 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0010 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0106 },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x0205 },                                            
	{ 0x0F12, 0x051E },                                            
	{ 0x0F12, 0x0101 },                                            
	{ 0x0F12, 0x0202 },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x2828 },                                            
	{ 0x0F12, 0x0606 },                                            
	{ 0x0F12, 0x0205 },                                            
	{ 0x0F12, 0x0480 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x1903 },                                            
	{ 0x0F12, 0x1911 },                                            
	{ 0x0F12, 0x0A0F },                                            
	{ 0x0F12, 0x050A },                                            
	{ 0x0F12, 0x2028 },                                            
	{ 0x0F12, 0x2028 },                                            
	{ 0x0F12, 0x0A08 },                                            
	{ 0x0F12, 0x0007 },                                            
	{ 0x0F12, 0x0403 },                                            
	{ 0x0F12, 0x0402 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0203 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0175 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x8070 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0014 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x000E },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0020 },                                            
	{ 0x0F12, 0x0050 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x000A },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x03FF },                                            
	{ 0x0F12, 0x0014 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0000 },
                                          
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0020 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0032 },                                            
	{ 0x0F12, 0x0106 },                                            
	{ 0x0F12, 0x006F },                                            
	{ 0x0F12, 0x0202 },                                            
	{ 0x0F12, 0x051E },                                            
	{ 0x0F12, 0x0101 },                                            
	{ 0x0F12, 0x0202 },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x140A },                                            
	{ 0x0F12, 0x2828 },                                            
	{ 0x0F12, 0x0606 },                                            
	{ 0x0F12, 0x0205 },                                            
	{ 0x0F12, 0x0880 },                                            
	{ 0x0F12, 0x000F },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x1903 },                                            
	{ 0x0F12, 0x1911 },                                            
	{ 0x0F12, 0x0A0F },                                            
	{ 0x0F12, 0x050A },                                            
	{ 0x0F12, 0x2020 },                                            
	{ 0x0F12, 0x2020 },                                            
	{ 0x0F12, 0x0A08 },                                            
	{ 0x0F12, 0x0007 },                                            
	{ 0x0F12, 0x0408 },                                            
	{ 0x0F12, 0x0406 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0608 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0006 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0175 },                                            
	{ 0x0F12, 0x0100 },                                            
	{ 0x0F12, 0x7058 },                                            
	{ 0x0F12, 0x0180 },                                            
	{ 0x0F12, 0x0000 },
//param_end - TVAR_afit_pBas

//param_start - afit_pConstB                                             
	{ 0x0F12, 0x00FF },                                            
	{ 0x0F12, 0x00FF },                                            
	{ 0x0F12, 0x0800 },                                            
	{ 0x0F12, 0x0600 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0300 },                                            
	{ 0x0F12, 0x0002 },                                            
	{ 0x0F12, 0x0400 },                                            
	{ 0x0F12, 0x0106 },                                            
	{ 0x0F12, 0x0005 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0x0703 },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0xFFD6 },                                            
	{ 0x0F12, 0x53C1 },                                            
	{ 0x0F12, 0xE1FE },                                            
	{ 0x0F12, 0x0001 },
//param_end - afit_pConstBas                                         

	{ 0x002A, 0x0488 },                                        
	{ 0x0F12, 0x416E },	//#lt_uMaxExp	                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0xA316 },	//#lt_uMaxExp2                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x002A, 0x2360 },                                        
	{ 0x0F12, 0xF424 },                                            
	{ 0x0F12, 0x0000 },
                                         
	{ 0x002A, 0x0490 },	//#lt_uCapMaxExp                                        
	{ 0x0F12, 0x416E },                                            
	{ 0x0F12, 0x0000 },                                            
	{ 0x0F12, 0xA316 }, //#lt_uCapMaxExp2                                             
	{ 0x0F12, 0x0000 },                                            
	{ 0x002A, 0x2364 },	//#AWBBTune_EVT4_uCapMaxExp3                                        
	{ 0x0F12, 0xF424 },                                            
	{ 0x0F12, 0x0000 },  
                                         
	{ 0x002A, 0x0498 },                                        
	{ 0x0F12, 0x01E8 },	//#lt_uMaxAnGain1                                            
	{ 0x0F12, 0x0300 },   //#lt_uMaxAnGain2                                         
	{ 0x002A, 0x2368 }, //#AWBBTune_EVT4_uMaxAnGain3                                         
	{ 0x0F12, 0x0300 },                                            
	{ 0x002A, 0x049C }, //lt_uMaxDigGain                                      
	{ 0x0F12, 0x0100 },                                            
	{ 0x002A, 0x235c },                                        
	{ 0x0F12, 0x0001 },  	//#AWBBTune_EVT4_uMinCoarse                                          
	{ 0x0F12, 0x0090 },  	//#AWBBTune_EVT4_uMinFine                                          

	{ 0x002A, 0x100E }, 	//AE Weight //                                          
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0201 },                                             
	{ 0x0F12, 0x0202 },                                             
	{ 0x0F12, 0x0202 },                                             
	{ 0x0F12, 0x0102 },                                             
	{ 0x0F12, 0x0201 },                                             
	{ 0x0F12, 0x0302 },                                             
	{ 0x0F12, 0x0203 },                                             
	{ 0x0F12, 0x0102 },                                             
	{ 0x0F12, 0x0201 },                                             
	{ 0x0F12, 0x0302 },                                             
	{ 0x0F12, 0x0203 },                                             
	{ 0x0F12, 0x0102 },                                             
	{ 0x0F12, 0x0201 },                                             
	{ 0x0F12, 0x0302 },                                             
	{ 0x0F12, 0x0203 },                                             
	{ 0x0F12, 0x0102 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0202 },                                             
	{ 0x0F12, 0x0202 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },                                             
	{ 0x0F12, 0x0101 },
 
	{ 0x002A, 0x0404 },                                             
	{ 0x0f12, 0x0001 },   
//image quality turning end  
///////////////////////////////////////////
//clk Settings
 
	{ 0x002A, 0x03FA },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x00C3 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01B8 },
	{ 0x0F12, 0x5DC0 },           	// 24MHz input clock  
	{ 0x0F12, 0x0000 },                                         
	{ 0x002A, 0x01C6 },                                     
	{ 0x0F12, 0x0000 },                                         
	{ 0x0F12, 0x0002 },                   //1 PLL configurations
	{ 0x002A, 0x01CC },                                     
	{ 0x0F12, 0x1B58 },                   //1st system CLK 28MHz
	{ 0x0F12, 0x30D4 },                                         
	{ 0x0F12, 0x30D4 },                   // 50MHz output clock 
	{ 0x0F12, 0x1B58 },                   //2nd system CLK 28MHz
	{ 0x0F12, 0x30D4 },                   // 50MHz output clock 
	{ 0x0F12, 0x30D4 },
	{ 0x002A, 0x01E0 },
	{ 0x0F12, 0x0001 },

////////////////////////////////////
//PREVIEW CONFIGURATION 3 (VGA, YUV)

	{ 0x002A, 0x02B4 }, 
	{ 0x0F12, 0x0280 }, 
	{ 0x0F12, 0x01E0 }, 
	{ 0x0F12, 0x0005 }, 
	{ 0x002A, 0x02C0 }, 
	{ 0x0F12, 0x0000 }, 
	{ 0x002A, 0x02BA }, 
	{ 0x0F12, 0x30D4 }, 
	{ 0x0F12, 0x30D4 }, 
	{ 0x0F12, 0x0040 | C_SS6AA_FMT }, //yuv
	{ 0x002A, 0x02C4 }, 
	{ 0x0F12, 0x0001 }, 
	{ 0x002A, 0x02C2 }, 
#if 1 // 15/30fps
	{ 0x0F12, 0x0000 }, 
	{ 0x002A, 0x02C6 }, 
	{ 0x0F12, 0x0535 }, 
//	{ 0x0F12, 0x0168 }, // 15fps
	{ 0x0F12, 0x014D },	// 1/33.3 = 30fps 
#else // fix 30fps
	{ 0x0F12, 0x0002 }, 
	{ 0x002A, 0x02C6 }, 
	{ 0x0F12, 0x0168 }, 
	{ 0x0F12, 0x0168 }, 
#endif

	{ 0x002A, 0x02D4 }, 
	{ 0x0F12, 0x0003 }, 
	{ 0x0F12, 0x0003 },
	
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//PREVIEW

	{ 0x002A, 0x03B6 },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x03FA },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x00C3 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021C },
	{ 0x0F12, 0x0003 }, //1: full size 3: VGA
	{ 0x002A, 0x0220 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F8 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021E },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F0 },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x0001 }, 

////////////////////////////////////////////////////////
	// MIPI Non_continous enable
        
	{ 0x0028, 0xD000 }, 
	{ 0x002A, 0xB0CC }, 
	{ 0x0F12, 0x000B },

	//MIPI timing control
	{ 0x002A, 0xB0A2 },	
	{ 0x0F12, 0x001E },	//86		//VCM & VOD
                  
	{ 0x002A, 0xB0C0 },	
	{ 0x0F12, 0x2EE0 },
	{ 0x0F12, 0x0960 },
	{ 0x002A, 0xB0c8 },    
	{ 0x0F12, 0x0005 },
	{ 0x0F12, 0x0003 },
	{ 0x0F12, 0x000A },
	{ 0x002A, 0xB0EA }, 
	{ 0x0F12, 0x0004 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0xB0E6 },
	{ 0x0F12, 0x0004 },
	{ 0x0F12, 0x0000 },	//PCLK prepare. 
	{ 0x002A, 0xB0F0 },    
	{ 0x0F12, 0x0004 }, 
	
	{ 0xffff, 0xffff },
};

regval16_t ss6AA_preview_640x480_table[] =
{
////////////////////////////////////
//PREVIEW CONFIGURATION 3 (VGA, YUV)
	{ 0x0028, 0x7000 }, 

	{ 0x002A, 0x02B4 }, 
	{ 0x0F12, 0x0280 }, 
	{ 0x0F12, 0x01E0 }, 
	{ 0x0F12, 0x0005 }, 
	{ 0x002A, 0x02C0 }, 
	{ 0x0F12, 0x0000 }, 
	{ 0x002A, 0x02BA }, 
	{ 0x0F12, 0x30D4 }, 
	{ 0x0F12, 0x30D4 }, 
	{ 0x0F12, 0x0040 | C_SS6AA_FMT }, //yuv
	{ 0x002A, 0x02C4 }, 
	{ 0x0F12, 0x0001 }, 
	{ 0x002A, 0x02C2 }, 
#if 0 //15 or 30 fps
	{ 0x0F12, 0x0000 }, 
	{ 0x002A, 0x02C6 }, 
	{ 0x0F12, 0x0535 }, // 1/133.3 = 7.5fps
//	{ 0x0F12, 0x0168 }, // 15fps
	{ 0x0F12, 0x014D },	// 1/33.3 = 30fps 
#else // fix 30fps
	{ 0x0F12, 0x0002 }, 
	{ 0x002A, 0x02C6 }, 
	{ 0x0F12, 0x0168 }, 
	{ 0x0F12, 0x0168 }, 
#endif
	{ 0x002A, 0x02D4 }, 
	{ 0x0F12, 0x0003 }, 
	{ 0x0F12, 0x0003 },
	
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//PREVIEW

	{ 0x002A, 0x03B6 },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x03FA },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x00C3 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021C },
	{ 0x0F12, 0x0003 }, //1: full size 3: VGA
	{ 0x002A, 0x0220 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F8 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021E },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F0 },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x0001 }, 
	
	{ 0xffff, 0xffff },
};

regval16_t ss6AA_capture_1280x1024_table[] =
{
	{ 0x0028, 0x7000 }, 

///////////////////////////////////////////   	
//PREVIEW CONFIGURATION 1 (1280x1024 YUV, 7.5 or 15fps)
	{ 0x002A, 0x0268 }, //REG_PrevConfigControls_1
	{ 0x0F12, 0x0500 }, //REG_1TC_PCFG_usWidth
	{ 0x0F12, 0x0400 }, //REG_1TC_PCFG_usHeight
	{ 0x0F12, 0x0005 }, //REG_1TC_PCFG_Format
	{ 0x002A, 0x0274 }, // REG_1TC_PCFG_uClockInd
	{ 0x0F12, 0x0000 }, 
	{ 0x002A, 0x026E }, //REG_1TC_PCFG_usMaxOut4KHzRate
	{ 0x0F12, 0x30D4 }, 
	{ 0x0F12, 0x30D4 }, //REG_1TC_PCFG_usMinOut4KHzRate
	{ 0x0F12, 0x0042 | C_SS6AA_FMT }, //yuv // REG_1TC_PCFG_PVIMask
	{ 0x002A, 0x0278 }, //REG_1TC_PCFG_FrRateQualityType
	{ 0x0F12, 0x0002 }, 
	{ 0x002A, 0x0276 }, //REG_1TC_PCFG_usFrTimeType
	{ 0x0F12, 0x0002 }, 
	{ 0x002A, 0x027A }, //REG_1TC_PCFG_usMaxFrTimeMsecMult10
#if 0
	{ 0x0F12, 0x0535 }, // 7.5fps
#else
	{ 0x0F12, 0x029A }, // 1/66.6 = 15fps
#endif
	{ 0x0F12, 0x0000 }, //REG_1TC_PCFG_usMinFrTimeMsecMult10
	{ 0x002A, 0x0288 }, //REG_1TC_PCFG_uPrevMirror
	{ 0x0F12, 0x0003 }, 
	{ 0x0F12, 0x0003 }, //REG_1TC_PCFG_uCaptureMirror

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//PREVIEW

	{ 0x002A, 0x03B6 },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x03FA },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x00C3 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021C },
	{ 0x0F12, 0x0001 },//1: full size 3: VGA
	{ 0x002A, 0x0220 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F8 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021E },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F0 },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x0001 }, 
	
	{ 0xffff, 0xffff },
};

regval16_t ss6AA_record_1280x720_table[] =
{
	{ 0x0028, 0x7000 },
	
///////////////////////////////////////////                    
//PREVIEW CONFIGURATION 0 (hd YUV, 15fps)  
 
	{ 0x002A, 0x0242 },
	{ 0x0F12, 0x0500 },
	{ 0x0F12, 0x02D0 },
	{ 0x0F12, 0x0005 },
	{ 0x002A, 0x024E },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x0248 },
	{ 0x0F12, 0x30D4 },
	{ 0x0F12, 0x30D4 },
	{ 0x0F12, 0x0040 | C_SS6AA_FMT }, //yuv
	{ 0x002A, 0x0252 },
	{ 0x0F12, 0x0002 },
	{ 0x002A, 0x0250 },
	{ 0x0F12, 0x0002 },
	{ 0x002A, 0x0254 },
	{ 0x0F12, 0x029A },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x0262 },
	{ 0x0F12, 0x0003 },
	{ 0x0F12, 0x0003 },

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//PREVIEW

	{ 0x002A, 0x03B6 },
	{ 0x0F12, 0x0000 },
	{ 0x002A, 0x03FA },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x00C3 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021C },
	{ 0x0F12, 0x0001 },//1: full size 3: VGA
	{ 0x002A, 0x0220 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F8 },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x021E },
	{ 0x0F12, 0x0001 },
	{ 0x002A, 0x01F0 },
	{ 0x0F12, 0x0001 },
	{ 0x0F12, 0x0001 }, 
	
	{ 0xffff, 0xffff },
};

regval16_t ss6AA_suspend_table[] = 
{
	{0xffff, 0xffff},
};
 
regval16_t ss6AA_resume_table[] = 
{
	{0xffff, 0xffff},
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
	g_ti2c_handle.pDeviceString = "SS6AA_MIPI";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("SS6AA_MIPI ti2c request failed\n");
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
	unsigned short *value
)
{
#if (I2C_MODE == HW_I2C)
	unsigned char addr[2], data[2];
	int nRet;
	
	addr[0] = (reg >> 8) & 0xFF;
	addr[1] = reg & 0xFF;
	nRet = gp_i2c_bus_write(g_i2c_handle, addr, 2);
	if(nRet <= 0) {
		return nRet;
	}
	
	nRet = gp_i2c_bus_read(g_i2c_handle, data, 2);
	*value = (data[1] << 8) | data[0];
	return nRet;

#elif (I2C_MODE == HW_TI2C)
	char addr[2], data[2];
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
	*value = (data[1] << 8) | data[0];
	return nRet;
#endif
}

static int
sensor_i2c_write(	
	unsigned short reg,
	unsigned short value
)
{
#if (I2C_MODE == HW_I2C)
	unsigned char data[4];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = (value >> 8) & 0xFF;
	data[3] = value & 0xFF;
	return gp_i2c_bus_write(g_i2c_handle, data, 4);
	
#elif (I2C_MODE == HW_TI2C)
	unsigned char data[4];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = (value >> 8) & 0xFF;
	data[3] = value & 0xFF;	
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 4;	
	return gp_ti2c_bus_xfer(&g_ti2c_handle);
#endif	
}

static int
sensor_write_table(
	regval16_t *vals
)
{
	int i, nRet;
	
	while (vals->reg_num != 0xffff || vals->value != 0xffff) {
		for(i = 0; i< 10; i++) {
			nRet = sensor_i2c_write(vals->reg_num, vals->value);
			if(nRet >= 0) {
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
	return sensor_write_table(ss6AA_init_table);
}

static int 
sensor_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	int nRet;
	regval16_t reset_table[] =
	{
		{0x0010, 0x0001},	// Reset
		{0x1030, 0x0000},	// Clear host interrupt so main will wait
		{0x0014, 0x0001},	// ARM go
		{0xFFFF, 0xFFFF}
	};

	printk("%s\n", __FUNCTION__);
	nRet = sensor_write_table(reset_table);
	msleep(1);
	return nRet;
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
	unsigned short data;
	printk("%s\n", __FUNCTION__);
	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		sensor_i2c_read(0x002A, &data);
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
		nRet = sensor_write_table(ss6AA_preview_640x480_table);
	} else if (fmt->fmt.pix.priv == 1) {
		sensor_write_table(ss6AA_init_table);
		nRet = sensor_write_table(ss6AA_capture_1280x1024_table);
	} else if (fmt->fmt.pix.priv == 2) {
		nRet = sensor_write_table(ss6AA_record_1280x720_table);	
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
	printk("%s\n", __FUNCTION__);
	return 0;
}	

static int 
sensor_suspend(
	struct v4l2_subdev *sd
)
{
	printk("%s\n", __FUNCTION__);
	return sensor_write_table(ss6AA_suspend_table);
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
	printk("%s\n", __FUNCTION__);
	return sensor_write_table(ss6AA_resume_table);
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
	if(sensor_i2c_open(SS6AA_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: SS_6AA mipi\n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_6AA_mipi");
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
MODULE_DESCRIPTION("Generalplus SS_6AA mipi sensor Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");



