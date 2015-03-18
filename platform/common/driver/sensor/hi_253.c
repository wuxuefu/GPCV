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
#define HI253_ID		0x40
 
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
		.desc		= "capture=1600*960",
		.pixelformat = V4L2_PIX_FMT_YVYU,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1600,
		.hoffset = 0,
		.vline = 960,
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
	g_ti2c_handle.pDeviceString = "HI253";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("HI253 ti2c request failed\n");
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

static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	
	printk("%s\n", __FUNCTION__);

	/*[SENSOR_INITIALIZATION]
	DISP_DATE = "2009-09-25 21:14:45"
	DISP_WIDTH = 800
	DISP_HEIGHT = 600
	DISP_FORMAT = YUV422
	DISP_DATAORDER = YUYV
	MCLK = 24.00
	PLL = 2.00

	BEGIN*/
	sensor_write(0x01, 0xf9); // Sleep ON
	sensor_write(0x08, 0x0f); // Hi-Z ON
	sensor_write(0x01, 0xf8); // Sleep OFF

	sensor_write(0x03, 0x00); // Dummy 750us START
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00); // Dummy 750us END

	sensor_write(0x0e, 0x00); // PLL 

	sensor_write(0x03, 0x00); // Dummy 750us START
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00); // Dummy 750us END

	sensor_write(0x0e, 0x00); // PLL OFF
	sensor_write(0x01, 0xf1); // Sleep ON
	sensor_write(0x08, 0x00); // Hi-Z OFF
	sensor_write(0x01, 0xf3);
	sensor_write(0x01, 0xf1);

	sensor_write(0x03, 0x20); // Page 20
	sensor_write(0x10, 0x0c); // AE OFF
	sensor_write(0x03, 0x22); // Page 22
	sensor_write(0x10, 0x69); // AWB OFF

	sensor_write(0x03, 0x00); // Page 0
	sensor_write(0x10, 0x13); // Sub1/2_Preview2
	sensor_write(0x11, 0x93); //0x11, 0x93); // Windowing ON); 1Frame Skip****test*****
	sensor_write(0x12, 0x04); // 00:Rinsing edge 04:fall edge
	sensor_write(0x0b, 0xaa); // ESD Check Register
	sensor_write(0x0c, 0xaa); // ESD Check Register
	sensor_write(0x0d, 0xaa); // ESD Check Register
	sensor_write(0x20, 0x00); // WINROWH
	sensor_write(0x21, 0x04); // WINROWL
	sensor_write(0x22, 0x00); // WINCOLH
	sensor_write(0x23, 0x07); // WINCOLL
	sensor_write(0x24, 0x03); // WINHGTH
	sensor_write(0x25, 0xc0); // WINHGTL
	sensor_write(0x26, 0x06); // WINWIDH
	sensor_write(0x27, 0x40); // WINWIDL

	sensor_write(0x40, 0x01); // HBLANKH 424
	sensor_write(0x41, 0xa8); // HBLANKL
	sensor_write(0x42, 0x00); // VSYNCH 8
	sensor_write(0x43, 0x08); // VSYNCL

	sensor_write(0x45, 0x04); // VSCTL1
	sensor_write(0x46, 0x18); // VSCTL2
	sensor_write(0x47, 0xd8); // VSCTL3

	sensor_write(0xe1, 0x0f);

	//BLC
	sensor_write(0x80, 0x2e); // BLCCTL
	sensor_write(0x81, 0x7e);
	sensor_write(0x82, 0x90);
	sensor_write(0x83, 0x00);
	sensor_write(0x84, 0x0c);
	sensor_write(0x85, 0x00);
	sensor_write(0x90, 0x0a); //0x90, 0x08// BLCTIMETH ON****test*****
	sensor_write(0x91, 0x0a); //0x91, 0x08// BLCTIMETH OFF****test*****
	sensor_write(0x92, 0x78); // BLCAGTH ON
	sensor_write(0x93, 0x70); // BLCAGTH OFF
	sensor_write(0x94, 0x75); // BLCDGTH ON
	sensor_write(0x95, 0x70); // BLCDGTH OFF
	sensor_write(0x96, 0xdc);
	sensor_write(0x97, 0xfe);
	sensor_write(0x98, 0x20);

	//OutDoor BLC
	sensor_write(0x99, 0x42); // B
	sensor_write(0x9a, 0x42); // Gb
	sensor_write(0x9b, 0x42); // R
	sensor_write(0x9c, 0x42); // Gr

	//Dark BLC
	sensor_write(0xa0, 0x00); // BLCOFS DB
	sensor_write(0xa2, 0x00); // BLCOFS DGb
	sensor_write(0xa4, 0x00); // BLCOFS DR
	sensor_write(0xa6, 0x00); // BLCOFS DGr

	//Normal BLC
	sensor_write(0xa8, 0x43);
	sensor_write(0xaa, 0x43);
	sensor_write(0xac, 0x43);
	sensor_write(0xae, 0x43);

	sensor_write(0x03, 0x02); // Page 2
	sensor_write(0x12, 0x03);
	sensor_write(0x13, 0x03);
	sensor_write(0x16, 0x00);
	sensor_write(0x17, 0x8C);
	sensor_write(0x18, 0x4c); // 0x28->0x2c->4c [20100919 update]
	sensor_write(0x19, 0x00); // 0x40->0x00 [20100912 update]
	sensor_write(0x1a, 0x39);
	sensor_write(0x1c, 0x09);
	sensor_write(0x1d, 0x40);
	sensor_write(0x1e, 0x30);
	sensor_write(0x1f, 0x10);
	sensor_write(0x20, 0x77);
	sensor_write(0x21, 0xde); // 0xdd->0xde [20100919 update]
	sensor_write(0x22, 0xa7);
	sensor_write(0x23, 0x30); // 0xb0->0x30 [20100912 update]
	sensor_write(0x27, 0x3c);
	sensor_write(0x2b, 0x80);
	sensor_write(0x2e, 0x00); // 100913 power saving Hy gou 11);
	sensor_write(0x2f, 0x00); // 100913 power saving Hy gou a1);
	sensor_write(0x30, 0x05);
	sensor_write(0x50, 0x20);
	sensor_write(0x52, 0x01);
	sensor_write(0x53, 0xc1);
	sensor_write(0x55, 0x1c);
	sensor_write(0x56, 0x11); // 0x00->0x11 [20100912 update]
	sensor_write(0x5d, 0xA2);
	sensor_write(0x5e, 0x5a);					   
	sensor_write(0x60, 0x87);
	sensor_write(0x61, 0x99);
	sensor_write(0x62, 0x88);
	sensor_write(0x63, 0x97);
	sensor_write(0x64, 0x88);
	sensor_write(0x65, 0x97);
	sensor_write(0x67, 0x0c);
	sensor_write(0x68, 0x0c);
	sensor_write(0x69, 0x0c);
	sensor_write(0x72, 0x89);
	sensor_write(0x73, 0x96); // 0x97->0x96 [20100919 update]
	sensor_write(0x74, 0x89);
	sensor_write(0x75, 0x96); // 0x97->0x96 [20100919 update]
	sensor_write(0x76, 0x89);
	sensor_write(0x77, 0x96); // 0x97->0x96 [20100912 update]
	sensor_write(0x7C, 0x85);
	sensor_write(0x7d, 0xaf);
	sensor_write(0x80, 0x01);
	sensor_write(0x81, 0x7f); // 0x81->0x7f [20100919 update]
	sensor_write(0x82, 0x13); // 0x23->0x13 [20100912 update]
	sensor_write(0x83, 0x24); // 0x2b->0x24 [20100912 update]
	sensor_write(0x84, 0x7d);
	sensor_write(0x85, 0x81);
	sensor_write(0x86, 0x7d);
	sensor_write(0x87, 0x81);
	sensor_write(0x92, 0x48); // 0x53->0x48 [20100912 update]
	sensor_write(0x93, 0x54); // 0x5e->0x54 [20100912 update]
	sensor_write(0x94, 0x7d);
	sensor_write(0x95, 0x81);
	sensor_write(0x96, 0x7d);
	sensor_write(0x97, 0x81);
	sensor_write(0xa0, 0x02);
	sensor_write(0xa1, 0x7b);
	sensor_write(0xa2, 0x02);
	sensor_write(0xa3, 0x7b);
	sensor_write(0xa4, 0x7b);
	sensor_write(0xa5, 0x02);
	sensor_write(0xa6, 0x7b);
	sensor_write(0xa7, 0x02);
	sensor_write(0xa8, 0x85);
	sensor_write(0xa9, 0x8c);
	sensor_write(0xaa, 0x85);
	sensor_write(0xab, 0x8c);
	sensor_write(0xac, 0x10); // 0x20->0x10 [20100912 update]
	sensor_write(0xad, 0x16); // 0x26->0x16 [20100912 update]
	sensor_write(0xae, 0x10); // 0x20->0x10 [20100912 update]
	sensor_write(0xaf, 0x16); // 0x26->0x16 [20100912 update]
	sensor_write(0xb0, 0x99);
	sensor_write(0xb1, 0xa3);
	sensor_write(0xb2, 0xa4);
	sensor_write(0xb3, 0xae);
	sensor_write(0xb4, 0x9b);
	sensor_write(0xb5, 0xa2);
	sensor_write(0xb6, 0xa6);
	sensor_write(0xb7, 0xac);
	sensor_write(0xb8, 0x9b);
	sensor_write(0xb9, 0x9f);
	sensor_write(0xba, 0xa6);
	sensor_write(0xbb, 0xaa);
	sensor_write(0xbc, 0x9b);
	sensor_write(0xbd, 0x9f);
	sensor_write(0xbe, 0xa6);
	sensor_write(0xbf, 0xaa);
	sensor_write(0xc4, 0x2c); // 0x36->0x2c [20100912 update]
	sensor_write(0xc5, 0x43); // 0x4e->0x43 [20100912 update]
	sensor_write(0xc6, 0x63); // 0x61->0x63 [20100912 update]
	sensor_write(0xc7, 0x79); // 0x78->0x79 [20100919 update]
	sensor_write(0xc8, 0x2d); // 0x36->0x2d [20100912 update]
	sensor_write(0xc9, 0x42); // 0x4d->0x42 [20100912 update]
	sensor_write(0xca, 0x2d); // 0x36->0x2d [20100912 update]
	sensor_write(0xcb, 0x42); // 0x4d->0x42 [20100912 update]
	sensor_write(0xcc, 0x64); // 0x62->0x64 [20100912 update]
	sensor_write(0xcd, 0x78);
	sensor_write(0xce, 0x64); // 0x62->0x64 [20100912 update]
	sensor_write(0xcf, 0x78);
	sensor_write(0xd0, 0x0a);
	sensor_write(0xd1, 0x09);
	sensor_write(0xd4, 0x0a); //0xd4, 0x08 // DCDCTIMETHON****test*****
	sensor_write(0xd5, 0x0a); //0xd5, 0x08 // DCDCTIMETHOFF****test*****
	sensor_write(0xd6, 0x78); // DCDCAGTHON
	sensor_write(0xd7, 0x70); // DCDCAGTHOFF
	sensor_write(0xe0, 0xc4);
	sensor_write(0xe1, 0xc4);
	sensor_write(0xe2, 0xc4);
	sensor_write(0xe3, 0xc4);
	sensor_write(0xe4, 0x00);
	sensor_write(0xe8, 0x80); // 0x00->0x80 [20100919 update]
	sensor_write(0xe9, 0x40);
	sensor_write(0xea, 0x7f); // 0x82->0x7f [20100919 update]
	sensor_write(0xf0, 0x01); // 100810 memory delay
	sensor_write(0xf1, 0x01); // 100810 memory delay
	sensor_write(0xf2, 0x01); // 100810 memory delay
	sensor_write(0xf3, 0x01); // 100810 memory delay
	sensor_write(0xf4, 0x01); // 100810 memory delay

	sensor_write(0x03, 0x03);
	sensor_write(0x10, 0x10);

	sensor_write(0x03, 0x10); // Page 10
	sensor_write(0x10, 0x03); // YUYV
	sensor_write(0x12, 0x30); // ISPCTL3
	sensor_write(0x20, 0x00); // ITUCTL
	sensor_write(0x30, 0x00);
	sensor_write(0x31, 0x00);
	sensor_write(0x32, 0x00);
	sensor_write(0x33, 0x00);
	sensor_write(0x34, 0x30);
	sensor_write(0x35, 0x00);
	sensor_write(0x36, 0x00);
	sensor_write(0x38, 0x00);
	sensor_write(0x3e, 0x58);
	sensor_write(0x3f, 0x02); // 0x02 for preview and 0x00 for capture
	//0x40, 0x80); // YOFS
	sensor_write(0x40, 0x85); // YOFS modify brightness
	sensor_write(0x41, 0x00); // DYOFS

	//Saturation
	sensor_write(0x60, 0x6f); // SATCTL
	sensor_write(0x61, 0x76); // SATB
	sensor_write(0x62, 0x76); // SATR
	sensor_write(0x63, 0x30); // AGSAT
	sensor_write(0x64, 0x41);
	sensor_write(0x66, 0x33); // SATTIMETH
	sensor_write(0x67, 0x00); // SATOUTDEL
	sensor_write(0x6a, 0x90); // UPOSSAT
	sensor_write(0x6b, 0x80); // UNEGSAT
	sensor_write(0x6c, 0x80); // VPOSSAT
	sensor_write(0x6d, 0xa0); // VNEGSAT
	sensor_write(0x76, 0x01); // white protection ON
	sensor_write(0x74, 0x66);
	sensor_write(0x79, 0x06);

	sensor_write(0x03, 0x11); // Page 11
	sensor_write(0x10, 0x3f); // DLPFCTL1
	sensor_write(0x11, 0x40);
	sensor_write(0x12, 0xba);
	sensor_write(0x13, 0xcb);
	sensor_write(0x26, 0x20); // LPFAGTHL
	sensor_write(0x27, 0x22); // LPFAGTHH
	sensor_write(0x28, 0x0f); // LPFOUTTHL
	sensor_write(0x29, 0x10); // LPFOUTTHH
	sensor_write(0x2b, 0x30); // LPFYMEANTHL
	sensor_write(0x2c, 0x32); // LPFYMEANTHH

	//Out2 D-LPF th
	sensor_write(0x30, 0x70); // OUT2YBOUNDH
	sensor_write(0x31, 0x10); // OUT2YBOUNDL
	sensor_write(0x32, 0x65); // OUT2RATIO
	sensor_write(0x33, 0x09); // OUT2THH
	sensor_write(0x34, 0x06); // OUT2THM
	sensor_write(0x35, 0x04); // OUT2THL

	//Out1 D-LPF th
	sensor_write(0x36, 0x70); // OUT1YBOUNDH
	sensor_write(0x37, 0x18); // OUT1YBOUNDL
	sensor_write(0x38, 0x65); // OUT1RATIO
	sensor_write(0x39, 0x09); // OUT1THH
	sensor_write(0x3a, 0x06); // OUT1THM
	sensor_write(0x3b, 0x04); // OUT1THL

	//Indoor D-LPF th
	sensor_write(0x3c, 0x80); // INYBOUNDH
	sensor_write(0x3d, 0x18); // INYBOUNDL
	sensor_write(0x3e, 0x80); // INRATIO
	sensor_write(0x3f, 0x0c); // INTHH
	sensor_write(0x40, 0x09); // INTHM
	sensor_write(0x41, 0x06); // INTHL

	sensor_write(0x42, 0x80); // DARK1YBOUNDH
	sensor_write(0x43, 0x18); // DARK1YBOUNDL
	sensor_write(0x44, 0x80); // DARK1RATIO
	sensor_write(0x45, 0x12); // DARK1THH
	sensor_write(0x46, 0x10); // DARK1THM
	sensor_write(0x47, 0x10); // DARK1THL
	sensor_write(0x48, 0x90); // DARK2YBOUNDH
	sensor_write(0x49, 0x40); // DARK2YBOUNDL
	sensor_write(0x4a, 0x80); // DARK2RATIO
	sensor_write(0x4b, 0x13); // DARK2THH
	sensor_write(0x4c, 0x10); // DARK2THM
	sensor_write(0x4d, 0x11); // DARK2THL
	sensor_write(0x4e, 0x80); // DARK3YBOUNDH
	sensor_write(0x4f, 0x30); // DARK3YBOUNDL
	sensor_write(0x50, 0x80); // DARK3RATIO
	sensor_write(0x51, 0x13); // DARK3THH
	sensor_write(0x52, 0x10); // DARK3THM
	sensor_write(0x53, 0x13); // DARK3THL
	sensor_write(0x54, 0x11);
	sensor_write(0x55, 0x17);
	sensor_write(0x56, 0x20);
	sensor_write(0x57, 0x20);
	sensor_write(0x58, 0x20);
	sensor_write(0x59, 0x30);
	sensor_write(0x5a, 0x18);
	sensor_write(0x5b, 0x00);
	sensor_write(0x5c, 0x00);
	sensor_write(0x60, 0x3f);
	sensor_write(0x62, 0x50);
	sensor_write(0x70, 0x06);

	sensor_write(0x03, 0x12); // Page 12
	sensor_write(0x20, 0x00); // YCLPFCTL1 0x00 for preview and 0x0f for capture
	sensor_write(0x21, 0x00); // YCLPFCTL2 0x00 for preview and 0x0f for capture
	sensor_write(0x25, 0x30);
	sensor_write(0x28, 0x00); // Out1 BP t
	sensor_write(0x29, 0x00); // Out2 BP t
	sensor_write(0x2a, 0x00);
	sensor_write(0x30, 0x50);
	sensor_write(0x31, 0x18); // YCUNI1TH
	sensor_write(0x32, 0x32); // YCUNI2TH
	sensor_write(0x33, 0x40); // YCUNI3TH
	sensor_write(0x34, 0x50); // YCNOR1TH
	sensor_write(0x35, 0x70); // YCNOR2TH
	sensor_write(0x36, 0xa0); // YCNOR3TH
	sensor_write(0x3b, 0x06);
	sensor_write(0x3c, 0x06);

	//Out2 th
	sensor_write(0x40, 0xa0); // YCOUT2THH
	sensor_write(0x41, 0x40); // YCOUT2THL
	sensor_write(0x42, 0xa0); // YCOUT2STDH
	sensor_write(0x43, 0x90); // YCOUT2STDM
	sensor_write(0x44, 0x90); // YCOUT2STDL
	sensor_write(0x45, 0x80); // YCOUT2RAT

	//Out1 th
	sensor_write(0x46, 0xb0); // YCOUT1THH
	sensor_write(0x47, 0x55); // YCOUT1THL
	sensor_write(0x48, 0xa0); // YCOUT1STDH
	sensor_write(0x49, 0x90); // YCOUT1STDM
	sensor_write(0x4a, 0x90); // YCOUT1STDL
	sensor_write(0x4b, 0x80); // YCOUT1RAT

	//In door th
	sensor_write(0x4c, 0xb0); // YCINTHH
	sensor_write(0x4d, 0x40); // YCINTHL
	sensor_write(0x4e, 0x90); // YCINSTDH
	sensor_write(0x4f, 0x90); // YCINSTDM
	sensor_write(0x50, 0xe6); // YCINSTDL
	sensor_write(0x51, 0x80); // YCINRAT

	//Dark1 th
	sensor_write(0x52, 0xb0); // YCDARK1THH
	sensor_write(0x53, 0x60); // YCDARK1THL
	sensor_write(0x54, 0xc0); // YCDARK1STDH
	sensor_write(0x55, 0xc0); // YCDARK1STDM
	sensor_write(0x56, 0xc0); // YCDARK1STDL
	sensor_write(0x57, 0x80); // YCDARK1RAT

	//Dark2 th
	sensor_write(0x58, 0x90); // YCDARK2THH
	sensor_write(0x59, 0x40); // YCDARK2THL
	sensor_write(0x5a, 0xd0); // YCDARK2STDH
	sensor_write(0x5b, 0xd0); // YCDARK2STDM
	sensor_write(0x5c, 0xe0); // YCDARK2STDL
	sensor_write(0x5d, 0x80); // YCDARK2RAT

	//Dark3 th
	sensor_write(0x5e, 0x88); // YCDARK3THH
	sensor_write(0x5f, 0x40); // YCDARK3THL
	sensor_write(0x60, 0xe0); // YCDARK3STDH
	sensor_write(0x61, 0xe6); // YCDARK3STDM
	sensor_write(0x62, 0xe6); // YCDARK3STDL
	sensor_write(0x63, 0x80); // YCDARK3RAT

	sensor_write(0x70, 0x15);
	sensor_write(0x71, 0x01); //Don't Touch register

	sensor_write(0x72, 0x18);
	sensor_write(0x73, 0x01); //Don't Touch register

	sensor_write(0x74, 0x25);
	sensor_write(0x75, 0x15);
	sensor_write(0x80, 0x30);
	sensor_write(0x81, 0x50);
	sensor_write(0x82, 0x80);
	sensor_write(0x85, 0x1a);
	sensor_write(0x88, 0x00);
	sensor_write(0x89, 0x00);
	sensor_write(0x90, 0x00); // DPCCTL 0x00 For Preview and 0x5d for capture

	sensor_write(0xc5, 0x30);
	sensor_write(0xc6, 0x2a);

	//Dont Touch register
	sensor_write(0xD0, 0x0c);
	sensor_write(0xD1, 0x80);
	sensor_write(0xD2, 0x67);
	sensor_write(0xD3, 0x00);
	sensor_write(0xD4, 0x00);
	sensor_write(0xD5, 0x02);
	sensor_write(0xD6, 0xff);
	sensor_write(0xD7, 0x18);

	sensor_write(0x03, 0x13); // Page 13
	sensor_write(0x10, 0xcb); // EDGECTL1
	sensor_write(0x11, 0x7b); // EDGECTL2
	sensor_write(0x12, 0x07); // EDGECTL3
	sensor_write(0x14, 0x00); // EDGECTL5

	sensor_write(0x20, 0x15); // EDGENGAIN
	sensor_write(0x21, 0x13); // EDGEPGAIN
	sensor_write(0x22, 0x33);
	sensor_write(0x23, 0x04); // EDGEHCLIP1TH
	sensor_write(0x24, 0x09); // EDGEHCLIP2TH
	sensor_write(0x25, 0x08); // EDGELCLIPTH
	sensor_write(0x26, 0x18); // EDGELCLIPLMT
	sensor_write(0x27, 0x30);
	sensor_write(0x29, 0x12); // EDGETIMETH
	sensor_write(0x2a, 0x50); // EDGEAGTH

	//Low clip th
	sensor_write(0x2b, 0x06);
	sensor_write(0x2c, 0x06);
	sensor_write(0x25, 0x08);
	sensor_write(0x2d, 0x0c);
	sensor_write(0x2e, 0x12);
	sensor_write(0x2f, 0x12);

	//Out2 Edge
	sensor_write(0x50, 0x10);
	sensor_write(0x51, 0x14);
	sensor_write(0x52, 0x10);
	sensor_write(0x53, 0x0c);
	sensor_write(0x54, 0x0f);
	sensor_write(0x55, 0x0c);

	//Out1 Edge
	sensor_write(0x56, 0x10);
	sensor_write(0x57, 0x13);
	sensor_write(0x58, 0x10);
	sensor_write(0x59, 0x0c);
	sensor_write(0x5a, 0x0f);
	sensor_write(0x5b, 0x0c);

	//Indoor Edge
	sensor_write(0x5c, 0x0a);
	sensor_write(0x5d, 0x0b);
	sensor_write(0x5e, 0x0a);
	sensor_write(0x5f, 0x08);
	sensor_write(0x60, 0x09);
	sensor_write(0x61, 0x08);

	//Dark1 Edge
	sensor_write(0x62, 0x08);
	sensor_write(0x63, 0x08);
	sensor_write(0x64, 0x08);
	sensor_write(0x65, 0x06);
	sensor_write(0x66, 0x06);
	sensor_write(0x67, 0x06);

	//Dark2 Edge
	sensor_write(0x68, 0x07);
	sensor_write(0x69, 0x07);
	sensor_write(0x6a, 0x07);
	sensor_write(0x6b, 0x05);
	sensor_write(0x6c, 0x05);
	sensor_write(0x6d, 0x05);

	//Dark3 Edge
	sensor_write(0x6e, 0x07);
	sensor_write(0x6f, 0x07);
	sensor_write(0x70, 0x07);
	sensor_write(0x71, 0x05);
	sensor_write(0x72, 0x05);
	sensor_write(0x73, 0x05);

	////2DY
	sensor_write(0x80, 0x00); // EDGE2DCTL1 00 for preview); must turn on 2DY 0xfd when capture
	sensor_write(0x81, 0x1f); // EDGE2DCTL2
	sensor_write(0x82, 0x05); // EDGE2DCTL3
	sensor_write(0x83, 0x01); // EDGE2DCTL4

	sensor_write(0x90, 0x05); // EDGE2DNGAIN
	sensor_write(0x91, 0x05); // EDGE2DPGAIN
	sensor_write(0x92, 0x33); // EDGE2DLCLIPLMT
	sensor_write(0x93, 0x30);
	sensor_write(0x94, 0x03); // EDGE2DHCLIP1TH
	sensor_write(0x95, 0x14); // EDGE2DHCLIP2TH
	sensor_write(0x97, 0x30);
	sensor_write(0x99, 0x30);

	sensor_write(0xa0, 0x04); // EDGE2DLCOUT2N
	sensor_write(0xa1, 0x05); // EDGE2DLCOUT2P
	sensor_write(0xa2, 0x04); // EDGE2DLCOUT1N
	sensor_write(0xa3, 0x05); // EDGE2DLCOUT1P
	sensor_write(0xa4, 0x07); // EDGE2DLCINN
	sensor_write(0xa5, 0x08); // EDGE2DLCINP
	sensor_write(0xa6, 0x07); // EDGE2DLCD1N
	sensor_write(0xa7, 0x08); // EDGE2DLCD1P
	sensor_write(0xa8, 0x07); // EDGE2DLCD2N
	sensor_write(0xa9, 0x08); // EDGE2DLCD2P
	sensor_write(0xaa, 0x07); // EDGE2DLCD3N
	sensor_write(0xab, 0x08); // EDGE2DLCD3P

	//Out2 
	sensor_write(0xb0, 0x22);
	sensor_write(0xb1, 0x2a);
	sensor_write(0xb2, 0x28);
	sensor_write(0xb3, 0x22);
	sensor_write(0xb4, 0x2a);
	sensor_write(0xb5, 0x28);

	//Out1 
	sensor_write(0xb6, 0x22);
	sensor_write(0xb7, 0x2a);
	sensor_write(0xb8, 0x28);
	sensor_write(0xb9, 0x22);
	sensor_write(0xba, 0x2a);
	sensor_write(0xbb, 0x28);

	sensor_write(0xbc, 0x17);
	sensor_write(0xbd, 0x17);
	sensor_write(0xbe, 0x17);
	sensor_write(0xbf, 0x17);
	sensor_write(0xc0, 0x17);
	sensor_write(0xc1, 0x17);

	//Dark1
	sensor_write(0xc2, 0x1e);
	sensor_write(0xc3, 0x12);
	sensor_write(0xc4, 0x10);
	sensor_write(0xc5, 0x1e);
	sensor_write(0xc6, 0x12);
	sensor_write(0xc7, 0x10);

	//Dark2
	sensor_write(0xc8, 0x18);
	sensor_write(0xc9, 0x05);
	sensor_write(0xca, 0x05);
	sensor_write(0xcb, 0x18);
	sensor_write(0xcc, 0x05);
	sensor_write(0xcd, 0x05);

	//Dark3 
	sensor_write(0xce, 0x18);
	sensor_write(0xcf, 0x05);
	sensor_write(0xd0, 0x05);
	sensor_write(0xd1, 0x18);
	sensor_write(0xd2, 0x05);
	sensor_write(0xd3, 0x05);

	sensor_write(0x03, 0x14); // Page 14
	sensor_write(0x10, 0x11); // LENSCTL1
	sensor_write(0x20, 0x40); // XCEN
	sensor_write(0x21, 0x80); // YCEN
	sensor_write(0x22, 0x80); // LENSRGAIN
	sensor_write(0x23, 0x80); // LENSGGAIN
	sensor_write(0x24, 0x80); // LENSBGAIN

	sensor_write(0x30, 0xc8);
	sensor_write(0x31, 0x2b);
	sensor_write(0x32, 0x00);
	sensor_write(0x33, 0x00);
	sensor_write(0x34, 0x90);

	sensor_write(0x40, 0x65); // LENSRP0
	sensor_write(0x50, 0x42); // LENSGrP0
	sensor_write(0x60, 0x3a); // LENSBP0
	sensor_write(0x70, 0x42); // LENSGbP0

	sensor_write(0x03, 0x15); // Page 15 
	sensor_write(0x10, 0x0f); // CMCCTL
	sensor_write(0x14, 0x52); // CMCOFSGH
	sensor_write(0x15, 0x42); // CMCOFSGM
	sensor_write(0x16, 0x32); // CMCOFSGL
	sensor_write(0x17, 0x2f); // CMCSIGN

	//CMC
	sensor_write(0x30, 0x8f); // CMC11
	sensor_write(0x31, 0x59); // CMC12
	sensor_write(0x32, 0x0a); // CMC13
	sensor_write(0x33, 0x15); // CMC21
	sensor_write(0x34, 0x5b); // CMC22
	sensor_write(0x35, 0x06); // CMC23
	sensor_write(0x36, 0x07); // CMC31
	sensor_write(0x37, 0x40); // CMC32
	sensor_write(0x38, 0x86); // CMC33

	//CMC OFS
	sensor_write(0x40, 0x95); // CMCOFSL11
	sensor_write(0x41, 0x1f); // CMCOFSL12
	sensor_write(0x42, 0x8a); // CMCOFSL13
	sensor_write(0x43, 0x86); // CMCOFSL21
	sensor_write(0x44, 0x0a); // CMCOFSL22
	sensor_write(0x45, 0x84); // CMCOFSL23
	sensor_write(0x46, 0x87); // CMCOFSL31
	sensor_write(0x47, 0x9b); // CMCOFSL32
	sensor_write(0x48, 0x23); // CMCOFSL33

	//CMC POFS
	sensor_write(0x50, 0x8c); // CMCOFSH11
	sensor_write(0x51, 0x0c); // CMCOFSH12
	sensor_write(0x52, 0x00); // CMCOFSH13
	sensor_write(0x53, 0x07); // CMCOFSH21
	sensor_write(0x54, 0x17); // CMCOFSH22
	sensor_write(0x55, 0x9d); // CMCOFSH23
	sensor_write(0x56, 0x00); // CMCOFSH31
	sensor_write(0x57, 0x0b); // CMCOFSH32
	sensor_write(0x58, 0x89); // CMCOFSH33

	sensor_write(0x80, 0x03);
	sensor_write(0x85, 0x40);
	sensor_write(0x87, 0x02);
	sensor_write(0x88, 0x00);
	sensor_write(0x89, 0x00);
	sensor_write(0x8a, 0x00);

	sensor_write(0x03, 0x16); // Page 16
	sensor_write(0x10, 0x31); // GMACTL
	sensor_write(0x18, 0x37);
	sensor_write(0x19, 0x36);
	sensor_write(0x1a, 0x0e);
	sensor_write(0x1b, 0x01);
	sensor_write(0x1c, 0xdc);
	sensor_write(0x1d, 0xfe);

	sensor_write(0x30, 0x00); // GGMA0
	sensor_write(0x31, 0x06); // GGMA1
	sensor_write(0x32, 0x1d); // GGMA2
	sensor_write(0x33, 0x33); // GGMA3
	sensor_write(0x34, 0x53); // GGMA4
	sensor_write(0x35, 0x6c); // GGMA5
	sensor_write(0x36, 0x81); // GGMA6
	sensor_write(0x37, 0x94); // GGMA7
	sensor_write(0x38, 0xa4); // GGMA8
	sensor_write(0x39, 0xb3); // GGMA9
	sensor_write(0x3a, 0xc0); // GGMA10
	sensor_write(0x3b, 0xcb); // GGMA11
	sensor_write(0x3c, 0xd5); // GGMA12
	sensor_write(0x3d, 0xde); // GGMA13
	sensor_write(0x3e, 0xe6); // GGMA14
	sensor_write(0x3f, 0xee); // GGMA15
	sensor_write(0x40, 0xf5); // GGMA16
	sensor_write(0x41, 0xfc); // GGMA17
	sensor_write(0x42, 0xff); // GGMA18

	sensor_write(0x50, 0x00); // RGMA0
	sensor_write(0x51, 0x03); // RGMA1
	sensor_write(0x52, 0x19); // RGMA2
	sensor_write(0x53, 0x34); // RGMA3
	sensor_write(0x54, 0x58); // RGMA4
	sensor_write(0x55, 0x75); // RGMA5
	sensor_write(0x56, 0x8d); // RGMA6
	sensor_write(0x57, 0xa1); // RGMA7
	sensor_write(0x58, 0xb2); // RGMA8
	sensor_write(0x59, 0xbe); // RGMA9
	sensor_write(0x5a, 0xc9); // RGMA10
	sensor_write(0x5b, 0xd2); // RGMA11
	sensor_write(0x5c, 0xdb); // RGMA12
	sensor_write(0x5d, 0xe3); // RGMA13
	sensor_write(0x5e, 0xeb); // RGMA14
	sensor_write(0x5f, 0xf0); // RGMA15
	sensor_write(0x60, 0xf5); // RGMA16
	sensor_write(0x61, 0xf7); // RGMA17
	sensor_write(0x62, 0xf8); // RGMA18

	sensor_write(0x70, 0x00); // BGMA0
	sensor_write(0x71, 0x08); // BGMA1
	sensor_write(0x72, 0x17); // BGMA2
	sensor_write(0x73, 0x2f); // BGMA3
	sensor_write(0x74, 0x53); // BGMA4
	sensor_write(0x75, 0x6c); // BGMA5
	sensor_write(0x76, 0x81); // BGMA6
	sensor_write(0x77, 0x94); // BGMA7
	sensor_write(0x78, 0xa4); // BGMA8
	sensor_write(0x79, 0xb3); // BGMA9
	sensor_write(0x7a, 0xc0); // BGMA10
	sensor_write(0x7b, 0xcb); // BGMA11
	sensor_write(0x7c, 0xd5); // BGMA12
	sensor_write(0x7d, 0xde); // BGMA13
	sensor_write(0x7e, 0xe6); // BGMA14
	sensor_write(0x7f, 0xee); // BGMA15
	sensor_write(0x80, 0xf4); // BGMA16
	sensor_write(0x81, 0xfa); // BGMA17
	sensor_write(0x82, 0xff); // BGMA18

	sensor_write(0x03, 0x17); // Page 17
	sensor_write(0x10, 0xf7);
	sensor_write(0xc4, 0x60); // FLK200
	sensor_write(0xc5, 0x50); // FLK240

	sensor_write(0x03, 0x20); // Page 20
	sensor_write(0x11, 0x1c);
	sensor_write(0x18, 0x30);
	sensor_write(0x1a, 0x08);
	sensor_write(0x20, 0x01); //05_lowtemp Y Mean off
	sensor_write(0x21, 0x30);
	sensor_write(0x22, 0x10);
	sensor_write(0x23, 0x00);
	sensor_write(0x24, 0x00); //Uniform Scene Off

	sensor_write(0x28, 0xe7);
	sensor_write(0x29, 0x0d); //20100305 ad->0d
	sensor_write(0x2a, 0xff);
	sensor_write(0x2b, 0x04); //f4->Adaptive off

	sensor_write(0x2c, 0xc2);
	sensor_write(0x2d, 0xcf);  //fe->AE Speed option
	sensor_write(0x2e, 0x33);
	sensor_write(0x30, 0x78); //f8
	sensor_write(0x32, 0x03);
	sensor_write(0x33, 0x2e);
	sensor_write(0x34, 0x30);
	sensor_write(0x35, 0xd4);
	sensor_write(0x36, 0xfe);
	sensor_write(0x37, 0x32);
	sensor_write(0x38, 0x04);
	sensor_write(0x39, 0x22); //AE_escapeC10
	sensor_write(0x3a, 0xde); //AE_escapeC11
	sensor_write(0x3b, 0x22); //AE_escapeC1
	sensor_write(0x3c, 0xde); //AE_escapeC2

	//Y_Frame TH
	sensor_write(0x50, 0x45);
	sensor_write(0x51, 0x88);

	sensor_write(0x56, 0x03);
	sensor_write(0x57, 0xf7);
	sensor_write(0x58, 0x14);
	sensor_write(0x59, 0x88);
	sensor_write(0x5a, 0x04);

	sensor_write(0x60, 0x55); // AEWGT1
	sensor_write(0x61, 0x55); // AEWGT2
	sensor_write(0x62, 0x6a); // AEWGT3
	sensor_write(0x63, 0xa9); // AEWGT4
	sensor_write(0x64, 0x6a); // AEWGT5
	sensor_write(0x65, 0xa9); // AEWGT6
	sensor_write(0x66, 0x6a); // AEWGT7
	sensor_write(0x67, 0xa9); // AEWGT8
	sensor_write(0x68, 0x6b); // AEWGT9
	sensor_write(0x69, 0xe9); // AEWGT10
	sensor_write(0x6a, 0x6a); // AEWGT11
	sensor_write(0x6b, 0xa9); // AEWGT12
	sensor_write(0x6c, 0x6a); // AEWGT13
	sensor_write(0x6d, 0xa9); // AEWGT14
	sensor_write(0x6e, 0x55); // AEWGT15
	sensor_write(0x6f, 0x55); // AEWGT16
	sensor_write(0x70, 0x76); // YLVL
	sensor_write(0x71, 0x00); //82(+8)->+0

	// haunting control
	sensor_write(0x76, 0x43);
	sensor_write(0x77, 0xe2); //04 //f2
	sensor_write(0x78, 0x23); // YTH1
	sensor_write(0x79, 0x42); // YTH2HI //46
	sensor_write(0x7a, 0x23);
	sensor_write(0x7b, 0x22);
	sensor_write(0x7d, 0x23);
	sensor_write(0x83, 0x01); // EXPTIMEH 30fps 
	sensor_write(0x84, 0x5f); // EXPTIMEM 
	sensor_write(0x85, 0x00); // EXPTIMEL 
	sensor_write(0x86, 0x01); // EXPMINH
	sensor_write(0x87, 0x38); // EXPMINL
	sensor_write(0x88, 0x04); //0x88, 0x03 // EXPMAXH 10fps****test*****
	sensor_write(0x89, 0x92); //0x89, 0xa8 // EXPMAXM ****test*****
	sensor_write(0x8a, 0x00); // EXPMAXL 
	sensor_write(0x8b, 0x75); // EXP100H
	sensor_write(0x8c, 0x00); // EXP100L 
	sensor_write(0x8d, 0x61); // EXP120H
	sensor_write(0x8e, 0x80); // EXP120L 
	sensor_write(0x9c, 0x0e); // EXPLMTH
	sensor_write(0x9d, 0xa0); // EXPLMTL 
	sensor_write(0x9e, 0x01); // EXPUNITH
	sensor_write(0x9f, 0x38); // EXPUNITL 
	sensor_write(0xb0, 0x18); // AG
	sensor_write(0xb1, 0x14); // AGMIN
	sensor_write(0xb2, 0xe0); // AGMAX
	sensor_write(0xb3, 0x18); // AGLVLH
	sensor_write(0xb4, 0x1a); // AGTH1
	sensor_write(0xb5, 0x44); // AGTH2
	sensor_write(0xb6, 0x2f); // AGBTH1
	sensor_write(0xb7, 0x28); // AGBTH2
	sensor_write(0xb8, 0x25); // AGBTH3
	sensor_write(0xb9, 0x22); // AGBTH4
	sensor_write(0xba, 0x21); // AGBTH5
	sensor_write(0xbb, 0x20); // AGBTH6
	sensor_write(0xbc, 0x1f); // AGBTH7
	sensor_write(0xbd, 0x1f); // AGBTH8
	sensor_write(0xc8, 0x80); // DGMAX
	sensor_write(0xc9, 0x60); // DGMIN

	/////// PAGE 22 START ///////
	sensor_write(0x03, 0x22);
	sensor_write(0x10, 0xfd);
	sensor_write(0x11, 0x2e);
	sensor_write(0x19, 0x01); // Low On //
	sensor_write(0x20, 0x30);
	sensor_write(0x21, 0x80);
	sensor_write(0x24, 0x01);

	sensor_write(0x30, 0x80);
	sensor_write(0x31, 0x80);
	sensor_write(0x38, 0x11);
	sensor_write(0x39, 0x34);

	sensor_write(0x40, 0xf7); 
	sensor_write(0x41, 0x55); 
	sensor_write(0x42, 0x33); 

	sensor_write(0x43, 0xf7);
	sensor_write(0x44, 0x55); 
	sensor_write(0x45, 0x33); 

	sensor_write(0x46, 0x00);
	sensor_write(0x50, 0xb2);
	sensor_write(0x51, 0x81);
	sensor_write(0x52, 0x98);

	sensor_write(0x80, 0x30); 
	sensor_write(0x81, 0x20);
	sensor_write(0x82, 0x3e);

	sensor_write(0x83, 0x5e);
	sensor_write(0x84, 0x0e); 
	sensor_write(0x85, 0x5e); 
	sensor_write(0x86, 0x22);

	sensor_write(0x87, 0x40);
	sensor_write(0x88, 0x30);
	sensor_write(0x89, 0x37);
	sensor_write(0x8a, 0x28);

	sensor_write(0x8b, 0x40);
	sensor_write(0x8c, 0x33);
	sensor_write(0x8d, 0x39);
	sensor_write(0x8e, 0x30);

	sensor_write(0x8f, 0x4e);
	sensor_write(0x90, 0x4b);
	sensor_write(0x91, 0x47);
	sensor_write(0x92, 0x44);
	sensor_write(0x93, 0x40);
	sensor_write(0x94, 0x34);
	sensor_write(0x95, 0x2f);
	sensor_write(0x96, 0x28);
	sensor_write(0x97, 0x24);
	sensor_write(0x98, 0x22);
	sensor_write(0x99, 0x20);
	sensor_write(0x9a, 0x20);

	sensor_write(0x9b, 0xbb);
	sensor_write(0x9c, 0xbb);
	sensor_write(0x9d, 0x48);
	sensor_write(0x9e, 0x38);
	sensor_write(0x9f, 0x30);

	sensor_write(0xa0, 0x60);
	sensor_write(0xa1, 0x34);
	sensor_write(0xa2, 0x6f);
	sensor_write(0xa3, 0xff);

	sensor_write(0xa4, 0x14); //1500fps
	sensor_write(0xa5, 0x2c); // 700fps
	sensor_write(0xa6, 0xcf);

	sensor_write(0xad, 0x40);
	sensor_write(0xae, 0x4a);

	sensor_write(0xaf, 0x28);  // low temp Rgain
	sensor_write(0xb0, 0x26);  // low temp Rgain

	sensor_write(0xb1, 0x00); //0x20 -> 0x00 0405 modify
	sensor_write(0xb4, 0xea);
	sensor_write(0xb8, 0xa0); //a2: b-2); R+2  //b4 B-3); R+4 lowtemp
	sensor_write(0xb9, 0x00);

	sensor_write(0x03, 0x24); // Page 24
	sensor_write(0x10, 0x01); // AFCTL1
	sensor_write(0x18, 0x06);
	sensor_write(0x30, 0x06);
	sensor_write(0x31, 0x90);
	sensor_write(0x32, 0x25);
	sensor_write(0x33, 0xa2);
	sensor_write(0x34, 0x26);
	sensor_write(0x35, 0x58);
	sensor_write(0x36, 0x60);
	sensor_write(0x37, 0x00);
	sensor_write(0x38, 0x50);
	sensor_write(0x39, 0x00);

	sensor_write(0x03, 0x20); // Page 20
	sensor_write(0x10, 0x9c); // AE ON 50Hz
	sensor_write(0x03, 0x22); // Page 22
	sensor_write(0x10, 0xe9); // AWB ON

	sensor_write(0x03, 0x00); // Page 0
	sensor_write(0x0e, 0x03); // PLL
	sensor_write(0x0e, 0x73); // PLL ON x2

	sensor_write(0x03, 0x00); // Dummy 750us START
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00);
	sensor_write(0x03, 0x00); // Dummy 750us END

	sensor_write(0x03, 0x00); // Page 0
	sensor_write(0x01, 0xf0); // Sleep OFF

	/*END
	[END]*/

	return 0;
		
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);

	sensor_write(0x03, 0x00);
	sensor_write(0x10, 0x13); 

	//modify preview start x); y
	sensor_write(0x20, 0x00); // WINROWH
	sensor_write(0x21, 0x04); // WINROWL
	sensor_write(0x22, 0x00); // WINCOLH
	sensor_write(0x23, 0x07); // WINCOLL

	sensor_write(0x3f, 0x00);
	
	sensor_write(0x3f, 0x02);

	//Page12
	sensor_write(0x03, 0x12); //Function
	sensor_write(0x20, 0x00);
	sensor_write(0x21, 0x00);
	sensor_write(0x90, 0x00);  
	//Page13
	sensor_write(0x03, 0x13); //Function
	sensor_write(0x80, 0x00); //Function

	
	return 0;
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);

	sensor_write(0x03, 0x00);	
	sensor_write(0x10, 0x00);//change size to 1600*1200
	
	sensor_write(0x3f, 0x00);
	
	//Page12
	sensor_write(0x03, 0x12); //Function
	sensor_write(0x20, 0x0f);
	sensor_write(0x21, 0x0f);
	sensor_write(0x90, 0x5d);  
	
	//Page13
	sensor_write(0x03, 0x13); //Function
	sensor_write(0x80, 0xfd); //Function
	
	/*capture 1600*1200 start x); y*/
	sensor_write(0x03, 0x00);
	sensor_write(0x20, 0x00); // WINROWH
	sensor_write(0x21, 0x0f); // WINROWL
	sensor_write(0x22, 0x00); // WINCOLH
	sensor_write(0x23, 0x19); // WINCOLL	
	
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
	unsigned char data;
	int nRet = 0;

	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		if(ctrl->value) {	// Enable Auto AWB
			nRet = sensor_write(0x03, 0x22);
			nRet = sensor_read(0x10, (unsigned char *)&data);						
			nRet = sensor_write(0x10, data|0x80);	 // Enable AWB
			nRet = sensor_write(0x80, 0x30);
			nRet = sensor_write(0x81, 0x20);
			nRet = sensor_write(0x82, 0x3e);
			nRet = sensor_write(0x83, 0x5e);
			nRet = sensor_write(0x84, 0x0e);
			nRet = sensor_write(0x85, 0x5e);
			nRet = sensor_write(0x86, 0x22);
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(0x03, 0x20); 	
			nRet = sensor_write(0x10, 0x9c); 
 		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_write(0x03, 0x20); 	
			nRet = sensor_write(0x10, 0x8c); 
 		}else{
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_write(0x03, 0x22);
			nRet = sensor_read(0x10, (unsigned char *)&data);						
			nRet = sensor_write(0x10, data&~0x80);	 // Disable AWB
			nRet = sensor_write(0x80, 0x45);
			nRet = sensor_write(0x81, 0x20);
			nRet = sensor_write(0x82, 0x27);
			nRet = sensor_write(0x83, 0x44);
			nRet = sensor_write(0x84, 0x3f);
			nRet = sensor_write(0x85, 0x29);
			nRet = sensor_write(0x86, 0x23);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_write(0x03, 0x22);
			nRet = sensor_read(0x10, (unsigned char *)&data);						
			nRet = sensor_write(0x10, data&~0x80);	 // Disable AWB
			nRet = sensor_write(0x80, 0x49);
			nRet = sensor_write(0x81, 0x20);
			nRet = sensor_write(0x82, 0x24);
			nRet = sensor_write(0x83, 0x50);
			nRet = sensor_write(0x84, 0x45);
			nRet = sensor_write(0x85, 0x24);
			nRet = sensor_write(0x86, 0x1e);
		}else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_write(0x03, 0x22);
			nRet = sensor_read(0x10, (unsigned char *)&data);						
			nRet = sensor_write(0x10, data&~0x80);	 // Disable AWB
			nRet = sensor_write(0x80, 0x25);
			nRet = sensor_write(0x81, 0x20);
			nRet = sensor_write(0x82, 0x44);
			nRet = sensor_write(0x83, 0x22);
			nRet = sensor_write(0x84, 0x1e);
			nRet = sensor_write(0x85, 0x50);
			nRet = sensor_write(0x86, 0x45);
		}else if(ctrl->value == 3) {	// INCANDESCENCE
			nRet = sensor_write(0x03, 0x22);
			nRet = sensor_read(0x10, (unsigned char *)&data);						
			nRet = sensor_write(0x10, data&~0x80);	 // Disable AWB
			nRet = sensor_write(0x80, 0x45);
			nRet = sensor_write(0x81, 0x20);
			nRet = sensor_write(0x82, 0x2f);
			nRet = sensor_write(0x83, 0x38);
			nRet = sensor_write(0x84, 0x32);
			nRet = sensor_write(0x85, 0x39);
			nRet = sensor_write(0x86, 0x33);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
 		if(ctrl->value) {	// NIGH MODE ON
 			nRet = sensor_write(0x03, 0x00);
			nRet = sensor_write(0x11, 0x97);
		}else{	// NIGH MODE OFF
 			nRet = sensor_write(0x03, 0x00);
			nRet = sensor_write(0x11, 0x93);
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
	if(sensor_i2c_open(HI253_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: hi253 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_hi253");
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
MODULE_DESCRIPTION("Generalplus hi253 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

