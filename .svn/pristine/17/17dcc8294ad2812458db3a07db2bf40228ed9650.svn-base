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
#define	SS6AA_ID		0x78

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
		.pixelformat = V4L2_PIX_FMT_YUYV,
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
		.pixelformat = V4L2_PIX_FMT_YUYV,
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
		.desc		= "record=640*480",
		.pixelformat = V4L2_PIX_FMT_YUYV,
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
	g_ti2c_handle.pDeviceString = "SS6AA";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("SS6AA ti2c request failed\n");
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
	unsigned short *value
)
{
#if (I2C_MODE == HW_I2C)
	char addr[2], data[2];
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
sensor_write(	
	unsigned short reg,
	unsigned short value
)
{
#if (I2C_MODE == HW_I2C)
	char data[4];
	
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

static int select_fmt = 0;

static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
//	sensor_write(0x12,0x80); //SW reset sensor
	msleep(100);
//****************************************/
//FCFCD000
//****************************************/
//	{0x0010, 0x0001},	// Reet
//00040000    // Diable Auto Addre Increment : 0 //Chunghwan Park
//	{0x1030, 0x0000},	// Clear hot interrupt o main will wait
//	{0x0014, 0x0001},	// ARM go
//p100	// Wait100mec
// tart T&P part
	sensor_write(0x0028, 0x7000);
	sensor_write(0x002A, 0x0400);
	sensor_write(0x0F12, 0x007F);
	sensor_write(0x002A, 0x03Dc);
	sensor_write(0x0F12, 0x0000);
	sensor_write(0x002A, 0x03DE);
	sensor_write(0x0F12, 0x0000);

//****************************************/
//For AE Window weight
//****************************************/
	
	sensor_write(0x002A, 0x100E);	//01.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1010);	//01.04-03
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1012);	//01.06-05
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1014);	//01.08-07
	sensor_write(0x0F12, 0x0101);

	sensor_write(0x002A, 0x1016);	//02.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1018);	//02.04-03
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x101A);	//02.06-05
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x101C);	//02.08-07
	sensor_write(0x0F12, 0x0101);

	sensor_write(0x002A, 0x101E);	//03.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1020);	//03.04-03
	sensor_write(0x0F12, 0x0305);
	sensor_write(0x002A, 0x1022);	//03.06-05
	sensor_write(0x0F12, 0x0105);
	sensor_write(0x002A, 0x1024);	//03.08-07
	sensor_write(0x0F12, 0x0101);

	sensor_write(0x002A, 0x1026);	//04.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1028);	//04.04-03
	sensor_write(0x0F12, 0x0505);
	sensor_write(0x002A, 0x102A);	//04.06-05
	sensor_write(0x0F12, 0x0305);
	sensor_write(0x002A, 0x102C);	//04.08-07
	sensor_write(0x0F12, 0x0101);

	sensor_write(0x002A, 0x102E);	//05.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1030);	//05.04-03
	sensor_write(0x0F12, 0x0301);
	sensor_write(0x002A, 0x1032);	//05.06-05
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1034);	//05.08-07
	sensor_write(0x0F12, 0x0101);

	sensor_write(0x002A, 0x1036);	//06.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1038);	//06.04-03
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x103A);	//06.06-05
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x103C);	//06.08-07
	sensor_write(0x0F12, 0x0101);

	sensor_write(0x002A, 0x103E);	//07.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1040);	//07.04-03
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1042);	//07.06-05
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1044);	//07.08-07
	sensor_write(0x0F12, 0x0101);

	sensor_write(0x002A, 0x1046);	//08.02-01
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x1048);	//08.04-03
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x104A);	//08.06-05
	sensor_write(0x0F12, 0x0101);
	sensor_write(0x002A, 0x104C);	//08.08-07
	sensor_write(0x0F12, 0x0101);

//****************************************/
//For SATURATION
//****************************************/
	sensor_write(0x002A, 0x082C);
	sensor_write(0x0F12, 0x0000);
	sensor_write(0x002A, 0x08AA);
	sensor_write(0x0F12, 0x0000);
	sensor_write(0x002A, 0x0928);
	sensor_write(0x0F12, 0x0000);
	sensor_write(0x002A, 0x09A6);
	sensor_write(0x0F12, 0x0008);
	sensor_write(0x002A, 0x0A24);
	sensor_write(0x0F12, 0x0016);

//****************************************/
//For Sharp Blur
//****************************************/
	sensor_write(0x002A, 0x082E);
	sensor_write(0x0F12, 0x0000);
	sensor_write(0x002A, 0x08AC);
	sensor_write(0x0F12, 0x0000);
	sensor_write(0x002A, 0x092A);
	sensor_write(0x0F12, 0x0037);
	sensor_write(0x002A, 0x09A8);
	sensor_write(0x0F12, 0x005A);
	sensor_write(0x002A, 0x0A26);
	sensor_write(0x0F12, 0x0064);

//****************************************/
//For Low frame rate
//****************************************/
	sensor_write(0x0028, 0x7000);
	sensor_write(0x002A, 0x048C);
	sensor_write(0x0F12, 0xd090);
	sensor_write(0x002A, 0x048E);
	sensor_write(0x0F12, 0x0005);

   
///////////////////////////////////////////
//clk Settings

	sensor_write(0x002A,0x01B8);
	sensor_write(0x0F12,0x5dc0);	//2ee0	// 24MHz input clock
	sensor_write(0x0F12,0x0000);
	sensor_write(0x002A,0x01C6);
	sensor_write(0x0F12,0x0001);	// PLL configurations
	sensor_write(0x002A,0x01CC);
	sensor_write(0x0F12,0x2710);	// 1st system CLK 28MHz
	sensor_write(0x0F12,0x2710);	// 24MHz output clock
	sensor_write(0x0F12,0x2710);	// 2nd system CLK
	sensor_write(0x0F12,0x1770);
	sensor_write(0x0F12,0x1770);
	sensor_write(0x0F12,0x1770);
	sensor_write(0x002A,0x01E0);
	sensor_write(0x0F12,0x0001);


//p100
	//sensor_write(0xFFFF,100);//Note:program can identify the address 0xffff and do waiting operation
	msleep(100);
///////////////////////////////////////////
//PREVIEW CONFIGURATION 0 (SXGA, YUV, 7.5fps)
	sensor_write(0x002A,0x0242);
	sensor_write(0x0F12,0x0500);	// 0320 					//1280
	sensor_write(0x0F12,0x0400);	//0258					//1024
	sensor_write(0x0F12,0x0005);	//YUV
	sensor_write(0x002A,0x024E);
	sensor_write(0x0F12,0x0000);
	sensor_write(0x002A,0x0248);
	sensor_write(0x0F12,0x1770);
	sensor_write(0x0F12,0x1770);
	sensor_write(0x0F12,0x0042);
	sensor_write(0x002A,0x0252);
	sensor_write(0x0F12,0x0002);
	sensor_write(0x002A,0x0250);
	sensor_write(0x0F12,0x0002);
	sensor_write(0x002A,0x0254);
	sensor_write(0x0F12,0x0594);	//029a
	sensor_write(0x0F12,0x0594);

//PREVIEW CONFIGURATION 1 (SXGA, YUV, 1.8-15fps)
	sensor_write(0x002A,0x0268);
	sensor_write(0x0F12,0x0500);	// 0320 					//1280
	sensor_write(0x0F12,0x0400);	//0258					//1024
	sensor_write(0x0F12,0x0005);	//YUV
	sensor_write(0x002A,0x0274);
	sensor_write(0x0F12,0x0000);
	sensor_write(0x002A,0x026E);
	sensor_write(0x0F12,0x2710);	//0x36B0-56Mhz
	sensor_write(0x0F12,0x1770);	//0x36B0-56Mhz
	sensor_write(0x0F12,0x0042);
	sensor_write(0x002A,0x0278);
	sensor_write(0x0F12,0x0002);
	sensor_write(0x002A,0x0276);
	sensor_write(0x0F12,0x0000);
	sensor_write(0x002A,0x027A);
	sensor_write(0x0F12,0x18F5);	//1.8 fps
	sensor_write(0x0F12,0x029A);	//8.5 fps

//PREVIEW CONFIGURATION 2 (VGA, YUV, 1-30 fps)
	sensor_write(0x002A,0x028E);
	sensor_write(0x0F12,0x0280);	//640
	sensor_write(0x0F12,0x0200);	//512
	sensor_write(0x0F12,0x0005);	//YUV
	sensor_write(0x002A,0x02C0);
	sensor_write(0x0F12,0x0000);
	sensor_write(0x002A,0x0294);
	sensor_write(0x0F12,0x1770);
	sensor_write(0x0F12,0x1770);
	sensor_write(0x0F12,0x0042);
	sensor_write(0x002A,0x029E);
	sensor_write(0x0F12,0x0001);
	sensor_write(0x002A,0x029C);
	sensor_write(0x0F12,0x0000);
	sensor_write(0x002A,0x02A0);
	//{0x0F12,0x2710},	//Min-1fps
	sensor_write(0x0F12,0x1388);	//Min-2fps
	//{0x0F12,0x0D05},	//Min-3fps
	//{0x0F12,0x09C4},	//Min-4fps
	//{0x0F12,0x07D0},	//Min-5fps
	//{0x0F12,0x0682},	//Min-6fps
	sensor_write(0x0F12,0x0168);

//PREVIEW CONFIGURATION 3 (VGA, YUV, 7.5-30fps)
	sensor_write(0x002A,0x02B4);                                
	sensor_write(0x0F12,0x0280);	//640
	sensor_write(0x0F12,0x0200);	//512                   
	sensor_write(0x0F12,0x0005);	//YUV                       
	sensor_write(0x002A,0x02C0);                                
	sensor_write(0x0F12,0x0000);	//PLL config                
	sensor_write(0x002A,0x02BA);                                
	sensor_write(0x0F12,0x1770);                                
	sensor_write(0x0F12,0x1770);                                
	sensor_write(0x0F12,0x0042);                                
	sensor_write(0x002A,0x02C4);                                
	sensor_write(0x0F12,0x0001);	//1b: Avg S.S 2b: SXGA      
	sensor_write(0x002A,0x02C2);                                
	sensor_write(0x0F12,0x0000);                                
	sensor_write(0x002A,0x02C6);                                
	sensor_write(0x0F12,0x0535);                                
	sensor_write(0x0F12,0x0168);   

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//PREVIEW
	sensor_write(0x002A,0x021C);
	sensor_write(0x0F12,0x0003);	// 0: fixed sxga 1: dynamic sxga 2: fixed vga 3: dynamic vga

	sensor_write(0x002A,0x0220);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F8);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x021E);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F0);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x0F12,0x0001);
	return 0;
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);

	select_fmt = 0;
//	sensor_write(0x12,0x80); //SW reset sensor
	msleep(100);

#if 1
	sensor_write(0x002A,0x021C);
	sensor_write(0x0F12,0x0002);	// 0: fixed sxga 1: dynamic sxga 2: fixed vga 3: dynamic vga

	sensor_write(0x002A,0x0220);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F8);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x021E);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F0);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x0F12,0x0001);
#endif
	return select_fmt;	
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);

	select_fmt = 1;
//	sensor_write(0x12,0x80); //SW reset sensor
	msleep(100);	
#if 1
	sensor_write(0x002A,0x021C);
	sensor_write(0x0F12,0x0001);	// 0: fixed sxga 1: dynamic sxga 2: fixed vga 3: dynamic vga

	sensor_write(0x002A,0x0220);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F8);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x021E);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F0);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x0F12,0x0001);
#endif
	return select_fmt;

}

static int 
sensor_record(void)
{
	printk("%s\n", __FUNCTION__);
	select_fmt = 2;
//	sensor_write(0x12,0x80); //SW reset sensor
	msleep(100);
#if 1
	sensor_write(0x002A,0x021C);
	sensor_write(0x0F12,0x0003);	// 0: fixed sxga 1: dynamic sxga 2: fixed vga 3: dynamic vga

	sensor_write(0x002A,0x0220);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F8);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x021E);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x002A,0x01F0);
	sensor_write(0x0F12,0x0001);

	sensor_write(0x0F12,0x0001);
#endif
	return select_fmt;	
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
#if 0
	unsigned char data;
	int nRet = 0;
	switch(ctrl->id)
	{
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);
		nRet = sensor_write(0xff, 0x00);
		nRet = sensor_read(0xc7,(unsigned char *)&data); 					
		if(ctrl->value) {	// Enable Auto AWB
			  nRet = sensor_write(0xc7,data& ~0x40);
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {

		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(0xff, 0x01);	
			nRet = sensor_write(0x0c, 0x38);
			if(select_fmt == 0){	// previre mode and capture mode
				nRet = sensor_write(0x46, 0x3f);
			}else if(select_fmt == 2){	// record mode
				nRet = sensor_write(0x46, 0x87);
			}
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_write(0xff, 0x01);	
			nRet = sensor_write(0x0c, 0x3c);
			nRet = sensor_write(0x46, 0x00);
		}else {
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		nRet = sensor_write(0xff, 0x00);
		nRet = sensor_read(0xc7, (unsigned char *)&data);
		if(ctrl->value == 0) {	//SUNSHINE
			nRet = sensor_write(0xc7, data|0x40);	
			nRet = sensor_write(0xCC, 0x4e);
			nRet = sensor_write(0xCD, 0x40);
			nRet = sensor_write(0xCE, 0x48);		
		}else if(ctrl->value == 1) {	//CLOUDY
			  nRet = sensor_write(0xc7,data|0x40);  // Manual AWB mode
			  nRet = sensor_write(0xCC, 0x38);
			  nRet = sensor_write(0xCD, 0x40);
			  nRet = sensor_write(0xCE, 0x58);
		}else if(ctrl->value == 2) {	//FLUORESCENCE
			  nRet = sensor_write(0xc7,data|0x40);  // Manual AWB mode
			  nRet = sensor_write(0xCC, 0x40);
			  nRet = sensor_write(0xCD, 0x40);
			  nRet = sensor_write(0xCE, 0x50);		
		}else if(ctrl->value == 3) {	//INCANDESCENCE
			nRet = sensor_write(0xc7,data|0x40);  
			nRet = sensor_write(0xCC, 0x30);
			nRet = sensor_write(0xCD, 0x40);
			nRet = sensor_write(0xCE, 0x66);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value) {
			nRet = sensor_write(0xff, 0x01);
			nRet = sensor_write(0x0f, 0x4b);
			nRet = sensor_write(0x03, 0x4f);
		}else {
			nRet = sensor_write(0xff, 0x01);
			nRet = sensor_write(0x0f, 0x43);
			nRet = sensor_write(0x03, 0x0f);
			nRet = sensor_write(0x2d, 0x00);
			nRet = sensor_write(0x2e, 0x00);
		}
		break;

	default:
		return -EINVAL;
	}

	return nRet; 
#endif
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
	int ret,i;

	printk("%s\n", __FUNCTION__);
	for(i=0; i<config_table.sensor_fmt_num; i++) {	
		if( (fmt->fmt.pix.width == config_table.fmt[i].hpixel) && (fmt->fmt.pix.height == config_table.fmt[i].vline) ) {
				printk("sensor mode = %d \n", i);
				if(0 == i) {
					ret = sensor_preview();
				}else if (1 == i) {
					ret = sensor_capture();
				}else if (2 == i) {
					ret = sensor_record();
				}else {
					ret = -1;
			}
	
		//g_sensor_dev.fmt = &g_fmt_table[fmt->fmt.pix.priv];
		g_sensor_dev.fmt = &g_fmt_table[i];
		printk("ret = %d /n",ret);
		return ret;
		}
	}
	return -1;
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
	if(sensor_i2c_open(SS6AA_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: ov2643 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov2643");
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
MODULE_DESCRIPTION("Generalplus SSA66 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

