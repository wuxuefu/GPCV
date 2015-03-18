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
#define	OV3640_ID		0x78

#define GPIO_SCL_CH		2
#define GPIO_SCL_PIN	0
#define GPIO_SCL_FUNC	2
#define GPIO_SCL_GID	4

#define GPIO_SDA_CH		2
#define GPIO_SDA_PIN	1
#define GPIO_SDA_FUNC	2
#define GPIO_SDA_GID	4

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
static int g_scl_handle, g_sda_handle;
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
	/* 0: preview mode */
	{
		.desc		= "preview=1024*768,crop=1024*768",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1024,
		.hoffset = 0,
		.vline = 768,
		.voffset = 0,
	},
	/* 1: capature mode */
	{
		.desc		= "capture=2048*1536,crop=2048*1536",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 2048,
		.hoffset = 0,
		.vline = 1536,
		.voffset = 0,
	},
	/* 2: record mode */
	{
		.desc		= "record=1024*768,crop=1024*768",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1024,
		.hoffset = 0,
		.vline = 768,
		.voffset = 0,
	},
	/* 3: preview mode + hsync */
	{
		.desc		= "preview=1024*768,crop=1024*768",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1024,
		.hoffset = 0x9E,
		.vline = 768,
		.voffset = 0x0B,
	},
	/* 4: capature mode + hsync */
	{
		.desc		= "capture=2048*1536,crop=2048*1536",
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 2048,
		.hoffset = 0x142,
		.vline = 1536,
		.voffset = 0x10,
	},
	/* 5: preview mode + hsync */
	{
		.desc		= "preview=1024*768,crop=1024*768",
		.pixelformat = V4L2_PIX_FMT_SGBRG8,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 1024,
		.hoffset = 0x147,
		.vline = 768,
		.voffset = 0x10,
	},
	/* 6: capature mode + hsync */
	{
		.desc		= "capture=2048*1536,crop=2048*1536",
		.pixelformat = V4L2_PIX_FMT_SGBRG8,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 24000000,
		.hpixel = 2048,
		.hoffset = 0x143,
		.vline = 1536,
		.voffset = 0x10,
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
	g_ti2c_handle.pDeviceString = "OV3640";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV3640 ti2c request failed\n");
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
sensor_read(
	unsigned short reg,
	unsigned char *value
)
{
#if (I2C_MODE == GPIO_I2C)
	return sccb_read(OV3640_ID, reg, value);

#elif (I2C_MODE == HW_I2C)
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
#if (I2C_MODE == GPIO_I2C)
	return sccb_write(OV3640_ID, reg, value);

#elif (I2C_MODE == HW_I2C)
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

static int sensor_set_hsync_mode(void)
{
	unsigned char data;

	//hsync mode
	sensor_read(0x3646, &data);
	data |= 0x40;
	sensor_write(0x3646, data);	
	printk("[0x3646]=%x\n", data);
	
	//vsync reverse
	//sensor_write(0x3600, 0xC4);			
	return 0;
}

static int sensor_increase_output_size(int hsize, int vsize)
{
	unsigned char data; 
	unsigned int temp;	

	if(hsize) {
		// x_output_size
		sensor_read(0x3088, &data); //XH
		temp = data;
		temp <<= 8;
		sensor_read(0x3089, &data); //XL
		temp |= data;
		temp += hsize;

		data = temp >> 8;
		sensor_write(0x3088, data);	//XH
		data = temp & 0xFF;
		sensor_write(0x3089, data);	//XL
		printk("width = %d\n", temp);
	}

	if(vsize) {
		// y_output_size
		sensor_read(0x308a, &data); //YH
		temp = data;
		temp <<= 8;
		sensor_read(0x308b, &data); //YL
		temp |= data;
		temp += hsize;

		data = temp >> 8;
		sensor_write(0x308a, data);	//YH
		data = temp & 0xFF;
		sensor_write(0x308b, data);	//YL
		printk("height = %d\n", temp);
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
	sensor_write(0x3012, 0x90);	// [7]:Reset; [6:4]=001->XGA mode
	sensor_write(0x30a9, 0xdb);	// for 1.5V
	sensor_write(0x304d, 0x45);
	sensor_write(0x3087, 0x16);
	sensor_write(0x309c, 0x1a);
	sensor_write(0x30a2, 0xe4);
	sensor_write(0x30aa, 0x42);
	sensor_write(0x30b0, 0xff);
	sensor_write(0x30b1, 0xff);
	sensor_write(0x30b2, 0x10);
	sensor_write(0x300e, 0x32);
	sensor_write(0x300f, 0x21);
	sensor_write(0x3010, 0x20);
	sensor_write(0x3011, 0x01);
	sensor_write(0x304c, 0x82);
	sensor_write(0x30d7, 0x10);
	sensor_write(0x30d9, 0x0d);
	sensor_write(0x30db, 0x08);
	sensor_write(0x3016, 0x82);
	sensor_write(0x3018, 0x48);	// Luminance High Range=72 after Gamma=0x86=134; 0x40->134
	sensor_write(0x3019, 0x40);	// Luminance Low Range=64 after Gamma=0x8f=143; 0x38->125
	sensor_write(0x301a, 0x82);
	sensor_write(0x307d, 0x00);
	sensor_write(0x307c, 0x01);
	sensor_write(0x3087, 0x02);
	sensor_write(0x3082, 0x20);
	sensor_write(0x3070, 0x00);	// 50Hz Banding MSB
	sensor_write(0x3071, 0xbb);	// 50Hz Banding LSB
	sensor_write(0x3072, 0x00);	// 60Hz Banding MSB
	sensor_write(0x3073, 0xa6);	// 60Hz Banding LSB
	sensor_write(0x301c, 0x07);	// max_band_step_50hz
	sensor_write(0x301d, 0x08);	// max_band_step_60hz
	sensor_write(0x3015, 0x12);	// [6:4]:1 dummy frame; [2:0]:AGC gain 8x
	sensor_write(0x3014, 0x84);	// [7]:50hz; [6]:auto banding detection disable; [3]:night modedisable
	sensor_write(0x3013, 0xf7);	// AE_en
	sensor_write(0x3030, 0x11);	// Avg_win_Weight0
	sensor_write(0x3031, 0x11);	// Avg_win_Weight1
	sensor_write(0x3032, 0x11);	// Avg_win_Weight2
	sensor_write(0x3033, 0x11);	// Avg_win_Weight3
	sensor_write(0x3034, 0x11);	// Avg_win_Weight4
	sensor_write(0x3035, 0x11);	// Avg_win_Weight5
	sensor_write(0x3036, 0x11);	// Avg_win_Weight6
	sensor_write(0x3037, 0x11);	// Avg_win_Weight7
	sensor_write(0x3038, 0x01);	// Avg_Win_Hstart=285
	sensor_write(0x3039, 0x1d);	// Avg_Win_Hstart=285
	sensor_write(0x303a, 0x00);	// Avg_Win_Vstart=10
	sensor_write(0x303b, 0x0a);	// Avg_Win_Vstart=10
	sensor_write(0x303c, 0x02);	// Avg_Win_Width=512x4=2048
	sensor_write(0x303d, 0x00);	// Avg_Win_Width=512x4=2048
	sensor_write(0x303e, 0x01);	// Avg_Win_Height=384x4=1536
	sensor_write(0x303f, 0x80);	// Avg_Win_Height=384x4=1536
	sensor_write(0x3047, 0x00);	// [7]:avg_based AE
	sensor_write(0x30b8, 0x20);
	sensor_write(0x30b9, 0x17);
	sensor_write(0x30ba, 0x04);
	sensor_write(0x30bb, 0x08);
	sensor_write(0x30a9, 0xdb);	// for 1.5V

	sensor_write(0x3104, 0x02);
	sensor_write(0x3105, 0xfd);
	sensor_write(0x3106, 0x00);
	sensor_write(0x3107, 0xff);
	sensor_write(0x3100, 0x02);

	sensor_write(0x3300, 0x13);	// [0]: LENC disable; [1]: AF enable
	sensor_write(0x3301, 0xde);	// [1]: BC_en; [2]: WC_en; [4]: CMX_en
	sensor_write(0x3302, 0xcf);	// [0]: AWB_en; [1]: AWB_gain_en; [2]: Gamma_en; [7]: Special_Effect_en
	sensor_write(0x3304, 0xfc);	// [4]: Add bias to gamma result; [5]: Enable Gamma bias function
	sensor_write(0x3306, 0x5c);	// Reserved ???
	sensor_write(0x3307, 0x11);	// Reserved ???
	sensor_write(0x3308, 0x00);	// [7]: AWB_mode=advanced
	sensor_write(0x330b, 0x1c);	// Reserved ???
	sensor_write(0x330c, 0x18);	// Reserved ???
	sensor_write(0x330d, 0x18);	// Reserved ???
	sensor_write(0x330e, 0x56);	// Reserved ???
	sensor_write(0x330f, 0x5c);	// Reserved ???
	sensor_write(0x3310, 0xd0);	// Reserved ???
	sensor_write(0x3311, 0xbd);	// Reserved ???
	sensor_write(0x3312, 0x26);	// Reserved ???
	sensor_write(0x3313, 0x2b);	// Reserved ???
	sensor_write(0x3314, 0x42);	// Reserved ???
	sensor_write(0x3315, 0x42);	// Reserved ???
	sensor_write(0x331b, 0x09);	// Gamma YST1
	sensor_write(0x331c, 0x18);	// Gamma YST2
	sensor_write(0x331d, 0x30);	// Gamma YST3
	sensor_write(0x331e, 0x58);	// Gamma YST4
	sensor_write(0x331f, 0x66);	// Gamma YST5
	sensor_write(0x3320, 0x72);	// Gamma YST6
	sensor_write(0x3321, 0x7d);	// Gamma YST7
	sensor_write(0x3322, 0x86);	// Gamma YST8
	sensor_write(0x3323, 0x8f);	// Gamma YST9
	sensor_write(0x3324, 0x97);	// Gamma YST10
	sensor_write(0x3325, 0xa5);	// Gamma YST11
	sensor_write(0x3326, 0xb2);	// Gamma YST12
	sensor_write(0x3327, 0xc7);	// Gamma YST13
	sensor_write(0x3328, 0xd8);	// Gamma YST14
	sensor_write(0x3329, 0xe8);	// Gamma YST15
	sensor_write(0x332a, 0x20);	// Gamma YSLP15
	sensor_write(0x332b, 0x00);	// [3]: WB_mode=auto
	sensor_write(0x332d, 0x64);	// [6]:de-noise auto mode; [5]:edge auto mode; [4:0]:edge threshold
	sensor_write(0x3355, 0x06);	// Special_Effect_CTRL: [1]:Sat_en; [2]: Cont_Y_en
	sensor_write(0x3358, 0x40);	// Special_Effect_Sat_U
	sensor_write(0x3359, 0x40);	// Special_Effect_Sat_V
	sensor_write(0x336a, 0x52);	// LENC R_A1
	sensor_write(0x3370, 0x46);	// LENC G_A1
	sensor_write(0x3376, 0x38);	// LENC B_A1

	sensor_write(0x3400, 0x00);	// [2:0];Format input source=DSP TUV444
	sensor_write(0x3403, 0x42);	// DVP Win Addr
	sensor_write(0x3404, 0x00);	// [5:0]: yuyv

	sensor_write(0x3507, 0x06);	// ???
	sensor_write(0x350a, 0x4f);	// ???

	sensor_write(0x3600, 0xc0);	// VSYNC_CTRL

	sensor_write(0x3302, 0xcf);	// [0]: AWB_enable
	sensor_write(0x300d, 0x01);	// PCLK/2
	sensor_write(0x3012, 0x10);	// [6:4]=001->XGA mode
	sensor_write(0x3013, 0xf7);	// AE_enable
	sensor_write(0x3020, 0x01);	// HS=285
	sensor_write(0x3021, 0x1d);	// HS=285
	sensor_write(0x3022, 0x00);	// VS = 6
	sensor_write(0x3023, 0x06);	// VS = 6
	sensor_write(0x3024, 0x08);	// HW=2072
	sensor_write(0x3025, 0x18);	// HW=2072
	sensor_write(0x3026, 0x03);	// VW=772
	sensor_write(0x3027, 0x04);	// VW=772
	sensor_write(0x3028, 0x09);	// HTotalSize=2375
	sensor_write(0x3029, 0x47);	// HTotalSize=2375
	sensor_write(0x302a, 0x03);	// VTotalSize=784
	sensor_write(0x302b, 0x10);	// VTotalSize=784
	sensor_write(0x304c, 0x82);
	sensor_write(0x3075, 0x24);	// VSYNCOPT
	sensor_write(0x3086, 0x00);	// Sleep/Wakeup
	sensor_write(0x3088, 0x04);	// x_output_size=1024
	sensor_write(0x3089, 0x00);	// x_output_size=1024
	sensor_write(0x308a, 0x03);	// y_output_size=768
	sensor_write(0x308b, 0x00);	// y_output_size=768
	sensor_write(0x308d, 0x04);
	sensor_write(0x30d7, 0x90);	// ???
	sensor_write(0x3302, 0xef);	// [5]: Scale_en, [0]: AWB_enable
	sensor_write(0x335f, 0x34);	// Scale_VH_in
	sensor_write(0x3360, 0x0c);	// Scale_H_in = 0x40c = 1036
	sensor_write(0x3361, 0x04);	// Scale_V_in = 0x304 = 772
	sensor_write(0x3362, 0x34);	// Scale_VH_out
	sensor_write(0x3363, 0x08);	// Scale_H_out = 0x408 = 1032
	sensor_write(0x3364, 0x04);	// Scale_V_out = 0x304 = 772
	sensor_write(0x300e, 0x32);
	sensor_write(0x300f, 0x21);
	sensor_write(0x3011, 0x00);	// for 30 FPS
	return sensor_write(0x304c, 0x82);

}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x3302, 0xcf);	// [0]: AWB_enable
	sensor_write(0x300d, 0x01);	// PCLK/2
	sensor_write(0x3012, 0x10);	// [6:4]=001->XGA mode
	sensor_write(0x3013, 0xf7);	// AE_enable
	sensor_write(0x3020, 0x01);	// HS=285
	sensor_write(0x3021, 0x1d);	// HS=285
	sensor_write(0x3022, 0x00);	// VS = 6
	sensor_write(0x3023, 0x06);	// VS = 6
	sensor_write(0x3024, 0x08);	// HW=2072
	sensor_write(0x3025, 0x18);	// HW=2072
	sensor_write(0x3026, 0x03);	// VW=772
	sensor_write(0x3027, 0x04);	// VW=772
	sensor_write(0x3028, 0x09);	// HTotalSize=2375
	sensor_write(0x3029, 0x47);	// HTotalSize=2375
	sensor_write(0x302a, 0x03);	// VTotalSize=784
	sensor_write(0x302b, 0x10);	// VTotalSize=784
	sensor_write(0x304c, 0x82);
	sensor_write(0x3075, 0x24);	// VSYNCOPT
	sensor_write(0x3086, 0x00);	// Sleep/Wakeup
	sensor_write(0x3088, 0x04);	// x_output_size=1024
	sensor_write(0x3089, 0x00);	// x_output_size=1024
	sensor_write(0x308a, 0x03);	// y_output_size=768
	sensor_write(0x308b, 0x00);	// y_output_size=768
	sensor_write(0x308d, 0x04);
	sensor_write(0x30d7, 0x90);	// ???
	sensor_write(0x3302, 0xef);	// [5]: Scale_en, [0]: AWB_enable
	sensor_write(0x335f, 0x34);	// Scale_VH_in
	sensor_write(0x3360, 0x0c);	// Scale_H_in = 0x40c = 1036
	sensor_write(0x3361, 0x04);	// Scale_V_in = 0x304 = 772
	sensor_write(0x3362, 0x34);	// Scale_VH_out
	sensor_write(0x3363, 0x08);	// Scale_H_out = 0x408 = 1032
	sensor_write(0x3364, 0x04);	// Scale_V_out = 0x304 = 772
	sensor_write(0x300e, 0x32);
	sensor_write(0x300f, 0x21);
	sensor_write(0x3011, 0x00);	// for 30 FPS
	return sensor_write(0x304c, 0x82);
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x3302, 0xce);	//[0]: AWB_disable
	sensor_write(0x300d, 0x00);	// PCLK/1
	sensor_write(0x300e, 0x39);
	sensor_write(0x300f, 0x21);
	sensor_write(0x3010, 0x20);
	sensor_write(0x3011, 0x00);
	sensor_write(0x3012, 0x00);	// [6:4]=000->QXGA mode
	sensor_write(0x3013, 0xf2);	// AE_disable
	sensor_write(0x3020, 0x01);	// HS=285
	sensor_write(0x3021, 0x1d);	// HS=285
	sensor_write(0x3022, 0x00);	// VS = 10
	sensor_write(0x3023, 0x0a);	// VS = 10
	sensor_write(0x3024, 0x08);	// HW=2072
	sensor_write(0x3025, 0x18);	// HW=2072
	sensor_write(0x3026, 0x06);	// VW=1548
	sensor_write(0x3027, 0x0c);	// VW=1548
	sensor_write(0x3028, 0x09);	// HTotalSize=2375
	sensor_write(0x3029, 0x47);	// HTotalSize=2375
	sensor_write(0x302a, 0x06);	// VTotalSize=1568
	sensor_write(0x302b, 0x20);	// VTotalSize=1568
	sensor_write(0x304c, 0x81);	// ???
	sensor_write(0x3075, 0x44);	// VSYNCOPT
	sensor_write(0x3088, 0x08);	// x_output_size=2048
	sensor_write(0x3089, 0x00);	// x_output_size=2048
	sensor_write(0x308a, 0x06);	// y_output_size=1536
	sensor_write(0x308b, 0x00);	// y_output_size=1536
	sensor_write(0x30d7, 0x10);	// ???
	sensor_write(0x3302, 0xee);	// [5]: Scale_en, [0]: AWB_disable
	sensor_write(0x335f, 0x68);	// Scale_VH_in
	sensor_write(0x3360, 0x18);	// Scale_H_in = 0x818 = 2072
	sensor_write(0x3361, 0x0c);	// Scale_V_in = 0x60c = 1548
	sensor_write(0x3362, 0x68);	// Scale_VH_out
	sensor_write(0x3363, 0x08);	// Scale_H_out = 0x808 = 2056
	return sensor_write(0x3364, 0x04);	// Scale_V_out = 0x604 = 1540
}

static int 
sensor_record(void)
{
	printk("%s\n", __FUNCTION__);
	sensor_write(0x3302, 0xcf);	// [0]: AWB_enable
	sensor_write(0x300d, 0x01);	// PCLK/2
	sensor_write(0x3012, 0x10);	// [6:4]=001->XGA mode
	sensor_write(0x3013, 0xf7);	// AE_enable
	sensor_write(0x3020, 0x01);	// HS=285
	sensor_write(0x3021, 0x1d);	// HS=285
	sensor_write(0x3022, 0x00);	// VS = 6
	sensor_write(0x3023, 0x06);	// VS = 6
	sensor_write(0x3024, 0x08);	// HW=2072
	sensor_write(0x3025, 0x18);	// HW=2072
	sensor_write(0x3026, 0x03);	// VW=772
	sensor_write(0x3027, 0x04);	// VW=772
	sensor_write(0x3028, 0x09);	// HTotalSize=2375
	sensor_write(0x3029, 0x47);	// HTotalSize=2375
	sensor_write(0x302a, 0x03);	// VTotalSize=784
	sensor_write(0x302b, 0x10);	// VTotalSize=784
	sensor_write(0x304c, 0x82);
	sensor_write(0x3075, 0x24);	// VSYNCOPT
	sensor_write(0x3086, 0x00);	// Sleep/Wakeup
	sensor_write(0x3088, 0x04);	// x_output_size=1024
	sensor_write(0x3089, 0x00);	// x_output_size=1024
	sensor_write(0x308a, 0x03);	// y_output_size=768
	sensor_write(0x308b, 0x00);	// y_output_size=768
	sensor_write(0x308d, 0x04);
	sensor_write(0x30d7, 0x90);	// ???
	sensor_write(0x3302, 0xef);	// [5]: Scale_en, [0]: AWB_enable
	sensor_write(0x335f, 0x34);	// Scale_VH_in
	sensor_write(0x3360, 0x0c);	// Scale_H_in = 0x40c = 1036
	sensor_write(0x3361, 0x04);	// Scale_V_in = 0x304 = 772
	sensor_write(0x3362, 0x34);	// Scale_VH_out
	sensor_write(0x3363, 0x08);	// Scale_H_out = 0x408 = 1032
	sensor_write(0x3364, 0x04);	// Scale_V_out = 0x304 = 772
	sensor_write(0x300e, 0x32);
	sensor_write(0x300f, 0x21);
	sensor_write(0x3011, 0x00);	// for 30 FPS
	return sensor_write(0x304c, 0x82);
}

static int 
sensor_set_qxga_raw(void) 
{
	#define OV3640_BGGR		0x18
	#define OV3640_GBRG		0x19
	#define OV3640_GRBG		0x1A	
	#define OV3640_RGGB		0x1B

	sensor_write( 0x3012, 0x80 );// [7]:Reset;
	sensor_write( 0x304d, 0x45 );//44 ;Rev2A
	sensor_write( 0x30a7, 0x5e );//Rev2C mi
	sensor_write( 0x3087, 0x16 );//Rev2A
	sensor_write( 0x309C, 0x1a );//Rev2C 18
	sensor_write( 0x30a2, 0xe4 );//Rev2C E8
	sensor_write( 0x30aa, 0x42 );//Rev2C 45
	sensor_write( 0x30b0, 0xff );//Rev2A
	sensor_write( 0x30b1, 0xff );
	sensor_write( 0x30b2, 0x10 );//driving

	sensor_write( 0x300e, 0x32 ); //21MHz, 5.8fps
	sensor_write( 0x300f, 0x21 );//051007 (a1)
	sensor_write( 0x3010, 0x20 );//Rev2A 82
	sensor_write( 0x3011, 0x01 );//Rev2A default 7.5fps
	sensor_write( 0x304c, 0x82 );//Rev2A
	sensor_write( 0x30d7, 0x10 );//Rev2A 08212007
	sensor_write( 0x30d9, 0x0d );//Rev2C
	sensor_write( 0x30db, 0x08 );//Rev2C
	sensor_write( 0x3016, 0x82 );//Rev2C

	//aec/agc auto setting
	sensor_write( 0x3018, 0x38 );//aec
	sensor_write( 0x3019, 0x30 );//06142007
	sensor_write( 0x301a, 0x61 );//06142007
	sensor_write( 0x307d, 0x00 );//aec isp 06142007
	sensor_write( 0x3087, 0x02 );//06142007
	sensor_write( 0x3082, 0x20 );//06142007

	sensor_write( 0x3070, 0x00 );//50Hz Banding MSB
	sensor_write( 0x3071, 0xaf );//50Hz Banding LSB
	sensor_write( 0x3072, 0x00 );//60Hz Banding MSB
	sensor_write( 0x3073, 0xa6 );//60Hz Banding LSB
	sensor_write( 0x301c, 0x07 );//max_band_step_50hz
	sensor_write( 0x301d, 0x08 );//max_band_step_60hz

	sensor_write( 0x3015, 0x12 );//07182007 8x gain, auto 1/2
	sensor_write( 0x3014, 0x05 );//0x0c );//06142007 auto frame on
	sensor_write( 0x3013, 0xf7 );//07182007

	//aecweight
	sensor_write( 0x303c, 0x08 );
	sensor_write( 0x303d, 0x18 );
	sensor_write( 0x303e, 0x06 );
	sensor_write( 0x303F, 0x0c );
	
	sensor_write( 0x3030, 0x62 );
	sensor_write( 0x3031, 0x26 );
	sensor_write( 0x3032, 0xe6 );
	sensor_write( 0x3033, 0x6e );
	sensor_write( 0x3034, 0xea );
	sensor_write( 0x3035, 0xae );
	sensor_write( 0x3036, 0xa6 );
	sensor_write( 0x3037, 0x6a );
	
	//ISP Common 
	sensor_write( 0x3104, 0x02 );//isp system control
	sensor_write( 0x3105, 0xfd );
	sensor_write( 0x3106, 0x00 );
	sensor_write( 0x3107, 0xff );
	sensor_write( 0x3300, 0x13 );//052207
	sensor_write( 0x3301, 0xde );//aec gamma- 06142007

	//ISP setting
	sensor_write( 0x3302, 0xcf );//sde, uv_adj, gam, awb

	//AWB
	sensor_write( 0x3312, 0x26 );
	sensor_write( 0x3314, 0x42 );
	sensor_write( 0x3313, 0x2b );
	sensor_write( 0x3315, 0x42 );
	sensor_write( 0x3310, 0xd0 );
	sensor_write( 0x3311, 0xbd );
	sensor_write( 0x330c, 0x18 );
	sensor_write( 0x330d, 0x18 );
	sensor_write( 0x330e, 0x56 );
	sensor_write( 0x330f, 0x5c );
	sensor_write( 0x330b, 0x1c );
	sensor_write( 0x3306, 0x5c );
	sensor_write( 0x3307, 0x11 );
	sensor_write( 0x3308, 0x00 ); // [7]: AWB_mode=advanced
	
	//gamma
	sensor_write( 0x331b, 0x09 ); // Gamma YST1
	sensor_write( 0x331c, 0x18 ); // Gamma YST2
	sensor_write( 0x331d, 0x30 ); // Gamma YST3
	sensor_write( 0x331e, 0x58 ); // Gamma YST4
	sensor_write( 0x331f, 0x66 ); // Gamma YST5
	sensor_write( 0x3320, 0x72 ); // Gamma YST6
	sensor_write( 0x3321, 0x7d ); // Gamma YST7
	sensor_write( 0x3322, 0x86 ); // Gamma YST8
	sensor_write( 0x3323, 0x8f ); // Gamma YST9
	sensor_write( 0x3324, 0x97 ); // Gamma YST10
	sensor_write( 0x3325, 0xa5 ); // Gamma YST11
	sensor_write( 0x3326, 0xb2 ); // Gamma YST12
	sensor_write( 0x3327, 0xc7 ); // Gamma YST13
	sensor_write( 0x3328, 0xd8 ); // Gamma YST14
	sensor_write( 0x3329, 0xe8 ); // Gamma YST15
	sensor_write( 0x332a, 0x20 ); // Gamma YSLP15
	sensor_write( 0x332b, 0x00 ); // [3]: WB_mode=auto
	sensor_write( 0x332d, 0x64 ); // [6]:de-noise auto mode; [5]:edge auto mode; [4:0]:edge threshold
	sensor_write( 0x3355, 0x06 ); // Special_Effect_CTRL: [1]:Sat_en; [2]: Cont_Y_en

	//Sat
    sensor_write( 0x3358, 0x40 ); // Special_Effect_Sat_U
	sensor_write( 0x3359, 0x40 ); // Special_Effect_Sat_V
	
	//Lens correction
	sensor_write( 0x336a, 0x52 );// LENC R_A1
	sensor_write( 0x3370, 0x46 );// LENC G_A1
	sensor_write( 0x3376, 0x38 );// LENC B_A1
	sensor_write( 0x3300, 0x13 ); 

	//UV adjust  
	sensor_write( 0x30b8, 0x20 );   
	sensor_write( 0x30b9, 0x17 );
	sensor_write( 0x30ba, 0x04 );
	sensor_write( 0x30bb, 0x08 );

	//Compression
	sensor_write( 0x3507, 0x06 );
	sensor_write( 0x350a, 0x4f );

	//Output format
	sensor_write( 0x3100, 0x32 );
	sensor_write( 0x3304, 0x00 );
	sensor_write( 0x3400, 0x02 );
	sensor_write( 0x3404, 0x22 );
	sensor_write( 0x3500, 0x00 );
	sensor_write( 0x3600, 0xC0 ); 
	sensor_write( 0x3610, 0x60 );
	sensor_write( 0x350a, 0x4f );
	
	//DVP QXGA
	sensor_write( 0x3088, 0x08 );
	sensor_write( 0x3089, 0x00 );
	sensor_write( 0x308a, 0x06 );
	sensor_write( 0x308b, 0x00 );

	//CIP Raw
	sensor_write( 0x3100, 0x22 );
	sensor_write( 0x3304, 0x01 );
	sensor_write( 0x3400, 0x03 );

	//vsync width 
	sensor_write( 0x302d, 0x00 ); /* EXVTS[15:8] */
	sensor_write( 0x302e, 0x00 ); /* EXVTS[7:0] */
	
	//hsync mode
	//sensor_write( 0x3646, 0x40 );
	
	sensor_write( 0x308d, 0x04 ); //Rev2A
	sensor_write( 0x3086, 0x03 ); //Rev2A
	return sensor_write( 0x3086, 0x00 ); //Rev2A
};

static int 
sensor_set_qxga_yuv(void)
{
	sensor_write( 0x3012, 0x80 );// [7]:Reset;
	sensor_write( 0x304d, 0x45 );//44 ;Rev2A
	sensor_write( 0x30a7, 0x5e );//Rev2C mi
	sensor_write( 0x3087, 0x16 );//Rev2A
	sensor_write( 0x309C, 0x1a );//Rev2C 18
	sensor_write( 0x30a2, 0xe4 );//Rev2C E8
	sensor_write( 0x30aa, 0x42 );//Rev2C 45
	sensor_write( 0x30b0, 0xff );//Rev2A
	sensor_write( 0x30b1, 0xff );
	sensor_write( 0x30b2, 0x10 );//driving

	sensor_write( 0x300e, 0x32 );//(3c)48Mhz 7.5fps
	//sensor_write( 0x300e, 0x27 );//40Mhz 10fps
	sensor_write( 0x300f, 0x21 );//051007 (a1)
	sensor_write( 0x3010, 0x20 );//Rev2A 82
	sensor_write( 0x3011, 0x01 );//Rev2A default 7.5fps
	sensor_write( 0x304c, 0x82 );//Rev2A
	sensor_write( 0x30d7, 0x10 );//Rev2A 08212007
	sensor_write( 0x30d9, 0x0d );//Rev2C
	sensor_write( 0x30db, 0x08 );//Rev2C
	sensor_write( 0x3016, 0x82 );//Rev2C

	//aec/agc auto setting
	sensor_write( 0x3018, 0x38 );//aec
	sensor_write( 0x3019, 0x30 );//06142007
	sensor_write( 0x301a, 0x61 );//06142007
	sensor_write( 0x307d, 0x00 );//aec isp 06142007
	sensor_write( 0x3087, 0x02 );//06142007
	sensor_write( 0x3082, 0x20 );//06142007

	sensor_write( 0x3070, 0x00 );//50Hz Banding MSB
	sensor_write( 0x3071, 0xaf );//50Hz Banding LSB
	sensor_write( 0x3072, 0x00 );//60Hz Banding MSB
	sensor_write( 0x3073, 0xa6 );//60Hz Banding LSB
	sensor_write( 0x301c, 0x07 );//max_band_step_50hz
	sensor_write( 0x301d, 0x08 );//max_band_step_60hz

	sensor_write( 0x3015, 0x12 );//07182007 8x gain, auto 1/2
	sensor_write( 0x3014, 0x05 );//0x0c );//06142007 auto frame on
	sensor_write( 0x3013, 0xf7 );//07182007

	//aecweight
	sensor_write( 0x303c, 0x08 );
	sensor_write( 0x303d, 0x18 );
	sensor_write( 0x303e, 0x06 );
	sensor_write( 0x303F, 0x0c );
	
	sensor_write( 0x3030, 0x62 );
	sensor_write( 0x3031, 0x26 );
	sensor_write( 0x3032, 0xe6 );
	sensor_write( 0x3033, 0x6e );
	sensor_write( 0x3034, 0xea );
	sensor_write( 0x3035, 0xae );
	sensor_write( 0x3036, 0xa6 );
	sensor_write( 0x3037, 0x6a );
	
	//ISP Common 
	sensor_write( 0x3104, 0x02 );//isp system control
	sensor_write( 0x3105, 0xfd );
	sensor_write( 0x3106, 0x00 );
	sensor_write( 0x3107, 0xff );
	sensor_write( 0x3300, 0x13 );//052207
	sensor_write( 0x3301, 0xde );//aec gamma- 06142007

	//ISP setting
	sensor_write( 0x3302, 0xcf );//sde, uv_adj, gam, awb

	//AWB
	sensor_write( 0x3312, 0x26 );
	sensor_write( 0x3314, 0x42 );
	sensor_write( 0x3313, 0x2b );
	sensor_write( 0x3315, 0x42 );
	sensor_write( 0x3310, 0xd0 );
	sensor_write( 0x3311, 0xbd );
	sensor_write( 0x330c, 0x18 );
	sensor_write( 0x330d, 0x18 );
	sensor_write( 0x330e, 0x56 );
	sensor_write( 0x330f, 0x5c );
	sensor_write( 0x330b, 0x1c );
	sensor_write( 0x3306, 0x5c );
	sensor_write( 0x3307, 0x11 );
	//sensor_write( 0x3308, 0x00 ); // [7]: AWB_mode=advanced
	
	//gamma
	sensor_write( 0x331b, 0x09 ); // Gamma YST1
	sensor_write( 0x331c, 0x18 ); // Gamma YST2
	sensor_write( 0x331d, 0x30 ); // Gamma YST3
	sensor_write( 0x331e, 0x58 ); // Gamma YST4
	sensor_write( 0x331f, 0x66 ); // Gamma YST5
	sensor_write( 0x3320, 0x72 ); // Gamma YST6
	sensor_write( 0x3321, 0x7d ); // Gamma YST7
	sensor_write( 0x3322, 0x86 ); // Gamma YST8
	sensor_write( 0x3323, 0x8f ); // Gamma YST9
	sensor_write( 0x3324, 0x97 ); // Gamma YST10
	sensor_write( 0x3325, 0xa5 ); // Gamma YST11
	sensor_write( 0x3326, 0xb2 ); // Gamma YST12
	sensor_write( 0x3327, 0xc7 ); // Gamma YST13
	sensor_write( 0x3328, 0xd8 ); // Gamma YST14
	sensor_write( 0x3329, 0xe8 ); // Gamma YST15
	sensor_write( 0x332a, 0x20 ); // Gamma YSLP15
	sensor_write( 0x332b, 0x00 ); // [3]: WB_mode=auto
	sensor_write( 0x332d, 0x64 ); // [6]:de-noise auto mode; [5]:edge auto mode; [4:0]:edge threshold
	sensor_write( 0x3355, 0x06 ); // Special_Effect_CTRL: [1]:Sat_en; [2]: Cont_Y_en

	//Sat
    sensor_write( 0x3358, 0x40 ); // Special_Effect_Sat_U
	sensor_write( 0x3359, 0x40 ); // Special_Effect_Sat_V
	
	//Lens correction
	sensor_write( 0x336a, 0x52 );// LENC R_A1
	sensor_write( 0x3370, 0x46 );// LENC G_A1
	sensor_write( 0x3376, 0x38 );// LENC B_A1
	sensor_write( 0x3300, 0x13 ); 

	//UV adjust  
	sensor_write( 0x30b8, 0x20 );   
	sensor_write( 0x30b9, 0x17 );
	sensor_write( 0x30ba, 0x04 );
	sensor_write( 0x30bb, 0x08 );

	//Compression
	sensor_write( 0x3507, 0x06 );
	sensor_write( 0x350a, 0x4f );

	//Output format
	sensor_write( 0x3100, 0x32 );
	sensor_write( 0x3304, 0x00 );
	sensor_write( 0x3400, 0x02 );
	sensor_write( 0x3404, 0x22 );
	sensor_write( 0x3500, 0x00 );
	sensor_write( 0x3600, 0xC0 ); 
	sensor_write( 0x3610, 0x60 );
	sensor_write( 0x350a, 0x4f );

	//DVP QXGA
	sensor_write( 0x3088, 0x08 );
	sensor_write( 0x3089, 0x00 );
	sensor_write( 0x308a, 0x06 );
	sensor_write( 0x308b, 0x00 );

	//SET YUV
	sensor_write( 0x3100, 0x02 );
	sensor_write( 0x3301, 0x10 ); //0x30
	sensor_write( 0x3304, 0x00 ); //0x03
	sensor_write( 0x3400, 0x00 );
	sensor_write( 0x3404, 0x00 );

	//vsync width 
	sensor_write( 0x302d, 0x00 ); /* EXVTS[15:8] */
	sensor_write( 0x302e, 0x00 ); /* EXVTS[7:0] */

	//hsync mode
	//sensor_write( 0x3646, 0x40 );
	 
	sensor_write( 0x304c, 0x81 ); 
	sensor_write( 0x3011, 0x01 ); //3.75fps
	
	sensor_write( 0x308d, 0x04 ); //Rev2A
	sensor_write( 0x3086, 0x03 ); //Rev2A
	return sensor_write( 0x3086, 0x00 ); //Rev2A
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
	case V4L2_CID_EXPOSURE:
		ctrl->value = 0;
		break;
		
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
	case V4L2_CID_EXPOSURE:
		break;
		
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		if(ctrl->value) {	// Enable Auto AWB
			nRet = sensor_read(0x332b, (unsigned char *)&data);			
			nRet = sensor_write(0x332b, (data &= ~0x8));
		}else{		// Disable Auto AWB
			nRet = sensor_read(0x332b, (unsigned char *)&data);
			nRet = sensor_write(0x332b, data |= 0x08);
			nRet = sensor_write(0x33a7, 0x40);
			nRet = sensor_write(0x33a8, 0x40);
			nRet = sensor_write(0x33a9, 0x40);
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		nRet = sensor_read(0x3014, &data);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_read(0x3014, (unsigned char *)&data);
			nRet = sensor_write(0x3014, data |= 0x80);
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_read(0x3014, (unsigned char *)&data);
			nRet = sensor_write(0x3014, data &= ~0x80);
		}else {
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_read(0x332b, (unsigned char *)&data);
			nRet = sensor_write(0x332b, data |= 0x08);
			nRet = sensor_write(0x33a7, 0x5a);
			nRet = sensor_write(0x33a8, 0x40);
			nRet = sensor_write(0x33a9, 0x48);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_read(0x332b, (unsigned char *)&data);
			nRet = sensor_write(0x332b, data |= 0x08);
			nRet = sensor_write(0x33a7, 0x68);
			nRet = sensor_write(0x33a8, 0x40);
			nRet = sensor_write(0x33a9, 0x50);
		}else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_read(0x332b, (unsigned char *)&data);
			nRet = sensor_write(0x332b, data |= 0x08);
			nRet = sensor_write(0x33a7, 0x52);
			nRet = sensor_write(0x33a8, 0x40);
			nRet = sensor_write(0x33a9, 0x5a);
		}else if(ctrl->value == 3) {	// INCANDESCENCE
			nRet = sensor_read(0x332b, (unsigned char *)&data);
			nRet = sensor_write(0x332b, data |= 0x08);
			nRet = sensor_write(0x33a7, 0x40);
			nRet = sensor_write(0x33a8, 0x40);
			nRet = sensor_write(0x33a9, 0x64);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value) {	// NIGH MODE ON
			nRet = sensor_read(0x3014, (unsigned char *)&data);
			nRet = sensor_write(0x3014, data |= 0x08);
		}else{	// NIGH MODE OFF
			nRet = sensor_read(0x3014, (unsigned char *)&data);
			nRet = sensor_write(0x3014, data &= ~0x08);
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

	printk("%s = %d\n", __FUNCTION__, fmt->fmt.pix.priv);
	switch(fmt->fmt.pix.priv)
	{
	case 0: //HREF
		ret = sensor_preview();
		break;

	case 1: //HREF
		ret = sensor_capture();
		break;

	case 2: //HREF
		ret = sensor_record();
		break;

	case 3:	//HSYNC + YUV
		printk("HSYNC + YUV\n");
		ret = sensor_preview();
		ret = sensor_increase_output_size(4, 0);
		ret = sensor_set_hsync_mode();
		break;

	case 4: //HSYNC + YUV
		printk("HSYNC + YUV\n");
		ret = sensor_set_qxga_yuv();
		ret = sensor_set_hsync_mode();
		break;

	case 5:	//HSYNC + RAW
		printk("HSYNC + RAW\n");
		ret = sensor_set_qxga_raw();
		ret = sensor_set_hsync_mode();
		break;

	case 6: //HSYNC + RAW
		printk("HSYNC + RAW\n");
		ret = sensor_set_qxga_raw();
		ret = sensor_set_hsync_mode();
		break;

	default:
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
	sensor_write(0x300e, 0xb2);
	sensor_write(0x308d, 0x14);
	sensor_write(0x3086, 0x0f);
	return 0;
}

static int 
sensor_resume(
	struct v4l2_subdev *sd
)
{
	// need implement
	sensor_write(0x3086, 0x08);
	sensor_write(0x308d, 0x14);
	sensor_write(0x300e, 0x32);
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
	if(sensor_i2c_open(OV3640_ID, 100) < 0) {
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: ov3640 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov3640");
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
MODULE_DESCRIPTION("Generalplus ov3640 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

