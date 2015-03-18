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
#define GT2005_ID					0x78

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
		.mclk_src = MODE_MCLK_SRC_320M,
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
		.mclk_src = MODE_MCLK_SRC_320M,
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
		.mclk_src = MODE_MCLK_SRC_320M,
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
	g_ti2c_handle.pDeviceString = "GT2005";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("GT2005 ti2c request failed\n");
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
sensor_write(
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
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x0101, 0x00);
	sensor_write(0x0103, 0x00);

	sensor_write(0x0105, 0x00);	//Hcount&Vcount
	sensor_write(0x0106, 0xF0);
	sensor_write(0x0107, 0x00);
	sensor_write(0x0108, 0x1C);

	sensor_write(0x0109, 0x00);	//Binning&Resoultion
	sensor_write(0x010A, 0x04);
	sensor_write(0x010B, 0x0f);
	sensor_write(0x010C, 0x00);
	sensor_write(0x010D, 0x08);
	sensor_write(0x010E, 0x00);
	sensor_write(0x010F, 0x08);
	sensor_write(0x0110, 0x03);
	sensor_write(0x0111, 0x20);
	sensor_write(0x0112, 0x02);
	sensor_write(0x0113, 0x58);

	sensor_write(0x0114, 0x04);	//YUV Mode

	sensor_write(0x0115, 0x00);	//Picture Effect

	sensor_write(0x0116, 0x02);	//PLL&Frame Rate
	sensor_write(0x0117, 0x00);
	sensor_write(0x0118, 0xAC);
	sensor_write(0x0119, 0x01);
	sensor_write(0x011A, 0x04);
	sensor_write(0x011B, 0x00);

	sensor_write(0x011C, 0x01);	//DCLK Polarity

	sensor_write(0x011D, 0x02);	//Do not change
	sensor_write(0x011E, 0x00);
	sensor_write(0x011F, 0x00);
	sensor_write(0x0120, 0x1C);
	sensor_write(0x0121, 0x00);
	sensor_write(0x0122, 0x04);
	sensor_write(0x0123, 0x00);
	sensor_write(0x0124, 0x00);
	sensor_write(0x0125, 0x00);
	sensor_write(0x0126, 0x00);
	sensor_write(0x0127, 0x00);
	sensor_write(0x0128, 0x00);

	sensor_write(0x0200, 0x00);	//Contrast

	sensor_write(0x0201, 0x00);	//Brightness

	sensor_write(0x0202, 0x40);	//Saturation

	sensor_write(0x0203, 0x00);	//Do not change
	sensor_write(0x0204, 0x03);
	sensor_write(0x0205, 0x1F);
	sensor_write(0x0206, 0x0B);
	sensor_write(0x0207, 0x20);
	sensor_write(0x0208, 0x00);
	sensor_write(0x0209, 0x2A);
	sensor_write(0x020A, 0x01);

	sensor_write(0x020B, 0x48);	//Sharpness
	sensor_write(0x020C, 0x64);

	sensor_write(0x020D, 0xC8);	//Do not change
	sensor_write(0x020E, 0xBC);
	sensor_write(0x020F, 0x08);
	sensor_write(0x0210, 0xD6);
	sensor_write(0x0211, 0x00);
	sensor_write(0x0212, 0x20);
	sensor_write(0x0213, 0x81);
	sensor_write(0x0214, 0x15);
	sensor_write(0x0215, 0x00);
	sensor_write(0x0216, 0x00);
	sensor_write(0x0217, 0x00);
	sensor_write(0x0218, 0x46);
	sensor_write(0x0219, 0x30);
	sensor_write(0x021A, 0x03);
	sensor_write(0x021B, 0x28);
	sensor_write(0x021C, 0x02);
	sensor_write(0x021D, 0x60);
	sensor_write(0x021E, 0x00);
	sensor_write(0x021F, 0x00);
	sensor_write(0x0220, 0x08);
	sensor_write(0x0221, 0x08);
	sensor_write(0x0222, 0x04);
	sensor_write(0x0223, 0x00);
	sensor_write(0x0224, 0x1F);
	sensor_write(0x0225, 0x1E);
	sensor_write(0x0226, 0x18);
	sensor_write(0x0227, 0x1D);
	sensor_write(0x0228, 0x1F);
	sensor_write(0x0229, 0x1F);
	sensor_write(0x022A, 0x01);
	sensor_write(0x022B, 0x04);
	sensor_write(0x022C, 0x05);
	sensor_write(0x022D, 0x05);
	sensor_write(0x022E, 0x04);
	sensor_write(0x022F, 0x03);
	sensor_write(0x0230, 0x02);
	sensor_write(0x0231, 0x1F);
	sensor_write(0x0232, 0x1A);
	sensor_write(0x0233, 0x19);
	sensor_write(0x0234, 0x19);
	sensor_write(0x0235, 0x1B);
	sensor_write(0x0236, 0x1F);
	sensor_write(0x0237, 0x04);
	sensor_write(0x0238, 0xEE);
	sensor_write(0x0239, 0xFF);
	sensor_write(0x023A, 0x00);
	sensor_write(0x023B, 0x00);
	sensor_write(0x023C, 0x00);
	sensor_write(0x023D, 0x00);
	sensor_write(0x023E, 0x00);
	sensor_write(0x023F, 0x00);
	sensor_write(0x0240, 0x00);
	sensor_write(0x0241, 0x00);
	sensor_write(0x0242, 0x00);
	sensor_write(0x0243, 0x21);
	sensor_write(0x0244, 0x42);
	sensor_write(0x0245, 0x53);
	sensor_write(0x0246, 0x54);
	sensor_write(0x0247, 0x54);
	sensor_write(0x0248, 0x54);
	sensor_write(0x0249, 0x33);
	sensor_write(0x024A, 0x11);
	sensor_write(0x024B, 0x00);
	sensor_write(0x024C, 0x00);
	sensor_write(0x024D, 0xFF);
	sensor_write(0x024E, 0xEE);
	sensor_write(0x024F, 0xDD);
	sensor_write(0x0250, 0x00);
	sensor_write(0x0251, 0x00);
	sensor_write(0x0252, 0x00);
	sensor_write(0x0253, 0x00);
	sensor_write(0x0254, 0x00);
	sensor_write(0x0255, 0x00);
	sensor_write(0x0256, 0x00);
	sensor_write(0x0257, 0x00);
	sensor_write(0x0258, 0x00);
	sensor_write(0x0259, 0x00);
	sensor_write(0x025A, 0x00);
	sensor_write(0x025B, 0x00);
	sensor_write(0x025C, 0x00);
	sensor_write(0x025D, 0x00);
	sensor_write(0x025E, 0x00);
	sensor_write(0x025F, 0x00);
	sensor_write(0x0260, 0x00);
	sensor_write(0x0261, 0x00);
	sensor_write(0x0262, 0x00);
	sensor_write(0x0263, 0x00);
	sensor_write(0x0264, 0x00);
	sensor_write(0x0265, 0x00);
	sensor_write(0x0266, 0x00);
	sensor_write(0x0267, 0x00);
	sensor_write(0x0268, 0x8F);
	sensor_write(0x0269, 0xA3);
	sensor_write(0x026A, 0xB4);
	sensor_write(0x026B, 0x90);
	sensor_write(0x026C, 0x00);
	sensor_write(0x026D, 0xD0);
	sensor_write(0x026E, 0x60);
	sensor_write(0x026F, 0xA0);
	sensor_write(0x0270, 0x40);
	sensor_write(0x0300, 0x81);
	sensor_write(0x0301, 0x80);
	sensor_write(0x0302, 0x22);
	sensor_write(0x0303, 0x06);
	sensor_write(0x0304, 0x03);
	sensor_write(0x0305, 0x83);
	sensor_write(0x0306, 0x00);
	sensor_write(0x0307, 0x22);
	sensor_write(0x0308, 0x00);
	sensor_write(0x0309, 0x55);
	sensor_write(0x030A, 0x55);
	sensor_write(0x030B, 0x55);
	sensor_write(0x030C, 0x54);
	sensor_write(0x030D, 0x1F);
	sensor_write(0x030E, 0x0A);
	sensor_write(0x030F, 0x10);
	sensor_write(0x0310, 0x04);
	sensor_write(0x0311, 0xFF);
	sensor_write(0x0312, 0x08);
	sensor_write(0x0313, 0x26);
	sensor_write(0x0314, 0xFF);
	sensor_write(0x0315, 0x96);
	sensor_write(0x0316, 0x26);
	sensor_write(0x0317, 0x02);
	sensor_write(0x0318, 0x08);
	sensor_write(0x0319, 0x0C);

	sensor_write(0x031A, 0x81);	//A LIGHT CORRECTION
	sensor_write(0x031B, 0x00);
	sensor_write(0x031C, 0x1D);
	sensor_write(0x031D, 0x00);
	sensor_write(0x031E, 0xFD);
	sensor_write(0x031F, 0x00);
	sensor_write(0x0320, 0xE1);
	sensor_write(0x0321, 0x1A);
	sensor_write(0x0322, 0xDE);
	sensor_write(0x0323, 0x11);
	sensor_write(0x0324, 0x1A);
	sensor_write(0x0325, 0xEE);
	sensor_write(0x0326, 0x50);
	sensor_write(0x0327, 0x18);
	sensor_write(0x0328, 0x25);
	sensor_write(0x0329, 0x37);
	sensor_write(0x032A, 0x24);
	sensor_write(0x032B, 0x32);
	sensor_write(0x032C, 0xA9);
	sensor_write(0x032D, 0x32);
	sensor_write(0x032E, 0xFF);
	sensor_write(0x032F, 0x7F);
	sensor_write(0x0330, 0xBA);
	sensor_write(0x0331, 0x7F);
	sensor_write(0x0332, 0x7F);
	sensor_write(0x0333, 0x14);
	sensor_write(0x0334, 0x81);
	sensor_write(0x0335, 0x14);
	sensor_write(0x0336, 0xFF);
	sensor_write(0x0337, 0x20);
	sensor_write(0x0338, 0x46);
	sensor_write(0x0339, 0x04);
	sensor_write(0x033A, 0x04);
	sensor_write(0x033B, 0x00);
	sensor_write(0x033C, 0x00);
	sensor_write(0x033D, 0x00);

	sensor_write(0x033E, 0x03);	//Do not change
	sensor_write(0x033F, 0x28);
	sensor_write(0x0340, 0x02);
	sensor_write(0x0341, 0x60);
	sensor_write(0x0342, 0xAC);
	sensor_write(0x0343, 0x97);
	sensor_write(0x0344, 0x7F);
	sensor_write(0x0400, 0xE8);
	sensor_write(0x0401, 0x40);
	sensor_write(0x0402, 0x00);
	sensor_write(0x0403, 0x00);
	sensor_write(0x0404, 0xF8);
	sensor_write(0x0405, 0x03);
	sensor_write(0x0406, 0x03);
	sensor_write(0x0407, 0x85);
	sensor_write(0x0408, 0x44);
	sensor_write(0x0409, 0x1F);
	sensor_write(0x040A, 0x40);
	sensor_write(0x040B, 0x33);

	sensor_write(0x040C, 0xA0);	//Lens Shading Correction
	sensor_write(0x040D, 0x00);
	sensor_write(0x040E, 0x00);
	sensor_write(0x040F, 0x00);
	sensor_write(0x0410, 0x0D);
	sensor_write(0x0411, 0x0D);
	sensor_write(0x0412, 0x0C);
	sensor_write(0x0413, 0x04);
	sensor_write(0x0414, 0x00);
	sensor_write(0x0415, 0x00);
	sensor_write(0x0416, 0x07);
	sensor_write(0x0417, 0x09);
	sensor_write(0x0418, 0x16);
	sensor_write(0x0419, 0x14);
	sensor_write(0x041A, 0x11);
	sensor_write(0x041B, 0x14);
	sensor_write(0x041C, 0x07);
	sensor_write(0x041D, 0x07);
	sensor_write(0x041E, 0x06);
	sensor_write(0x041F, 0x02);
	sensor_write(0x0420, 0x42);
	sensor_write(0x0421, 0x42);
	sensor_write(0x0422, 0x47);
	sensor_write(0x0423, 0x39);
	sensor_write(0x0424, 0x3E);
	sensor_write(0x0425, 0x4D);
	sensor_write(0x0426, 0x46);
	sensor_write(0x0427, 0x3A);
	sensor_write(0x0428, 0x21);
	sensor_write(0x0429, 0x21);
	sensor_write(0x042A, 0x26);
	sensor_write(0x042B, 0x1C);
	sensor_write(0x042C, 0x25);
	sensor_write(0x042D, 0x25);
	sensor_write(0x042E, 0x28);
	sensor_write(0x042F, 0x20);
	sensor_write(0x0430, 0x3E);
	sensor_write(0x0431, 0x3E);
	sensor_write(0x0432, 0x33);
	sensor_write(0x0433, 0x2E);
	sensor_write(0x0434, 0x54);
	sensor_write(0x0435, 0x53);
	sensor_write(0x0436, 0x3C);
	sensor_write(0x0437, 0x51);
	sensor_write(0x0438, 0x2B);
	sensor_write(0x0439, 0x2B);
	sensor_write(0x043A, 0x38);
	sensor_write(0x043B, 0x22);
	sensor_write(0x043C, 0x3B);
	sensor_write(0x043D, 0x3B);
	sensor_write(0x043E, 0x31);
	sensor_write(0x043F, 0x37);

	sensor_write(0x0440, 0x00);	//PWB Gain
	sensor_write(0x0441, 0x4B);
	sensor_write(0x0442, 0x00);
	sensor_write(0x0443, 0x00);
	sensor_write(0x0444, 0x31);
	sensor_write(0x0445, 0x00);
	sensor_write(0x0446, 0x00);
	sensor_write(0x0447, 0x00);
	sensor_write(0x0448, 0x00);
	sensor_write(0x0449, 0x00);
	sensor_write(0x044A, 0x00);
	sensor_write(0x044D, 0xE0);
	sensor_write(0x044E, 0x05);
	sensor_write(0x044F, 0x07);
	sensor_write(0x0450, 0x00);
	sensor_write(0x0451, 0x00);
	sensor_write(0x0452, 0x00);
	sensor_write(0x0453, 0x00);
	sensor_write(0x0454, 0x00);
	sensor_write(0x0455, 0x00);
	sensor_write(0x0456, 0x00);
	sensor_write(0x0457, 0x00);
	sensor_write(0x0458, 0x00);
	sensor_write(0x0459, 0x00);
	sensor_write(0x045A, 0x00);
	sensor_write(0x045B, 0x00);
	sensor_write(0x045C, 0x00);
	sensor_write(0x045D, 0x00);
	sensor_write(0x045E, 0x00);
	sensor_write(0x045F, 0x00);

	sensor_write(0x0460, 0x80);	//GAMMA Correction
	sensor_write(0x0461, 0x10);
	sensor_write(0x0462, 0x10);
	sensor_write(0x0463, 0x10);
	sensor_write(0x0464, 0x08);
	sensor_write(0x0465, 0x08);
	sensor_write(0x0466, 0x11);
	sensor_write(0x0467, 0x09);
	sensor_write(0x0468, 0x23);
	sensor_write(0x0469, 0x2A);
	sensor_write(0x046A, 0x2A);
	sensor_write(0x046B, 0x47);
	sensor_write(0x046C, 0x52);
	sensor_write(0x046D, 0x42);
	sensor_write(0x046E, 0x36);
	sensor_write(0x046F, 0x46);
	sensor_write(0x0470, 0x3A);
	sensor_write(0x0471, 0x32);
	sensor_write(0x0472, 0x32);
	sensor_write(0x0473, 0x38);
	sensor_write(0x0474, 0x3D);
	sensor_write(0x0475, 0x2F);
	sensor_write(0x0476, 0x29);
	sensor_write(0x0477, 0x48);

	sensor_write(0x0686, 0x6F);

	sensor_write(0x0100, 0x01);	//Output Enable
	sensor_write(0x0102, 0x02);
	sensor_write(0x0104, 0x03);
	return sensor_write(0x0101, 0x00);//GT2005_H_V_Switch
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x0109, 0x00);
	sensor_write(0x010A, 0x04);	// 1/2 binning
	sensor_write(0x010B, 0x03);
	sensor_write(0x010C, 0x00);
	sensor_write(0x010D, 0x08);	// horizontal size = 1600 pixels(default)
	sensor_write(0x010E, 0x00);
	sensor_write(0x010F, 0x08);	// vertical size = 1200 lines (default)

	sensor_write(0x0110, 0x03);
	sensor_write(0x0111, 0x20);	// 800
	sensor_write(0x0112, 0x02);
	return sensor_write(0x0113, 0x58);	// 600 SVGA SETTING
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x0109, 0x01);	//Binning&Resoultion
	sensor_write(0x010A, 0x00);
	sensor_write(0x010B, 0x00);
	sensor_write(0x010C, 0x00);
	sensor_write(0x010D, 0x08);
	sensor_write(0x010E, 0x00);
	sensor_write(0x010F, 0x08);

	sensor_write(0x0110, 0x06);
	sensor_write(0x0111, 0x40);
	sensor_write(0x0112, 0x04);
	return sensor_write(0x0113, 0xB0);	// 1600*1200
}

static int 
sensor_record(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x0109, 0x00);
	sensor_write(0x010A, 0x04);	// 1/2 binning
	sensor_write(0x010B, 0x03);
	sensor_write(0x010C, 0x00);
	sensor_write(0x010D, 0x08);	// horizontal size = 1600 pixels(default)
	sensor_write(0x010E, 0x00);
	sensor_write(0x010F, 0x08);	// vertical size = 1200 lines (default)

	sensor_write(0x0110, 0x03);
	sensor_write(0x0111, 0x20);	// 800
	sensor_write(0x0112, 0x02);
	return sensor_write(0x0113, 0x58);	// 600 SVGA SETTING
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
			nRet = sensor_write(0x031a,0x81);   // select Auto WB
			nRet = sensor_write(0x0320,0x24);//  WBGRMAX[7:0];
			nRet = sensor_write(0x0321,0x14);//  WBGRMIN[7:0];
			nRet = sensor_write(0x0322,0x1a);//  WBGBMAX[7:0];
			nRet = sensor_write(0x0323,0x24);//  WBGBMIN[7:0];
			nRet = sensor_write(0x0441,0x56);//  PWBGAINR[7:0];
			nRet = sensor_write(0x0442,0x00);//  PWBGAINGR[7:0];
			nRet = sensor_write(0x0443,0x00);//  PWBGAINGB[7:0];
			nRet = sensor_write(0x0444,0x17);//  PWBGAINB[7:0];
		}else{		// Disable Auto AWB
			nRet = sensor_read(0x031a, (unsigned char *)&data);			
			nRet = sensor_write(0x031a, (data &= ~0x80));
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {

		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(0x0315, 0x16);                  			
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_write(0x0315, 0x56);                  			
		}else{
			return -EINVAL;
		}
		break;

	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		nRet = sensor_read(0x031a, (unsigned char *)&data);			
		nRet = sensor_write(0x031a, (data &= ~0x80));
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_write(0x0320 ,0x02);
			nRet = sensor_write(0x0321 ,0x02);
			nRet = sensor_write(0x0322 ,0x02);
			nRet = sensor_write(0x0323 ,0x02);
			nRet = sensor_write(0x0441 ,0x60);
			nRet = sensor_write(0x0442 ,0x00);
			nRet = sensor_write(0x0443 ,0x00);
			nRet = sensor_write(0x0444 ,0x14);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_write(0x0320 ,0x02);//  WBGRMAX[7:0];
			nRet = sensor_write(0x0321 ,0x02);//  WBGRMIN[7:0];
			nRet = sensor_write(0x0322 ,0x02);//  WBGBMAX[7:0];
			nRet = sensor_write(0x0323 ,0x02);//  WBGBMIN[7:0];
			nRet = sensor_write(0x0441 ,0x80);//  PWBGAINR[7:0];
			nRet = sensor_write(0x0442 ,0x00);//  PWBGAINGR[7:0];
			nRet = sensor_write(0x0443 ,0x00);//  PWBGAINGB[7:0];
			nRet = sensor_write(0x0444 ,0x0D);//  PWBGAINB[7:0];
		}else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_write(0x0320 ,0x02);//  WBGRMAX[7:0];
			nRet = sensor_write(0x0321 ,0x02);//  WBGRMIN[7:0];
			nRet = sensor_write(0x0322 ,0x02);//  WBGBMAX[7:0];
			nRet = sensor_write(0x0323 ,0x02);//  WBGBMIN[7:0];
			nRet = sensor_write(0x0441 ,0x43);//  PWBGAINR[7:0];
			nRet = sensor_write(0x0442 ,0x00);//  PWBGAINGR[7:0];
			nRet = sensor_write(0x0443 ,0x00);//  PWBGAINGB[7:0];
			nRet = sensor_write(0x0444 ,0x4b);//  PWBGAINB[7:0];
		}else if(ctrl->value == 3) {	// INCANDESCENCE
			nRet = sensor_write(0x0320 ,0x02);//  WBGRMAX[7:0];
			nRet = sensor_write(0x0321 ,0x02);//  WBGRMIN[7:0];
			nRet = sensor_write(0x0322 ,0x02);//  WBGBMAX[7:0];
			nRet = sensor_write(0x0323 ,0x02);//  WBGBMIN[7:0];
			nRet = sensor_write(0x0441 ,0x50);//  PWBGAINR[7:0];
			nRet = sensor_write(0x0442 ,0x00);//  PWBGAINGR[7:0];
			nRet = sensor_write(0x0443 ,0x00);//  PWBGAINGB[7:0];
			nRet = sensor_write(0x0444 ,0x30);//  PWBGAINB[7:0];
		}
		break;

	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value) {	// NIGH MODE ON
			nRet = sensor_write(0x0312 , 0x98);//set fps/2
		}else{	// NIGH MODE OFF
			nRet = sensor_write(0x0312 , 0x08); //Disable night mode  Frame rate do not change);//set fps
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
	if(sensor_i2c_open(GT2005_ID, 100) < 0) {
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: gt2005 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_gt2005");
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
MODULE_DESCRIPTION("Generalplus gt2005 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

