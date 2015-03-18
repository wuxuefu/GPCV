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
#define	MT9D112_ID					0x78

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
		.pixelformat = V4L2_PIX_FMT_UYVY,
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
		.desc		= "capture=1600*1200",
		.pixelformat = V4L2_PIX_FMT_VYUY,
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
		.desc		= "record=640*480",
		.pixelformat = V4L2_PIX_FMT_UYVY,
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
	g_ti2c_handle.pDeviceString = "MT9D112";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("MT9D112 ti2c request failed\n");
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
	unsigned int reg,
	unsigned int *value
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
	*value = ((data[0]<<8) |data[1]);
	return nRet;
	
#elif (I2C_MODE == HW_TI2C)
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
	*value = ((data[0]<<8) |data[1]);
	return nRet;
#endif
}


static int 
sensor_write(
	unsigned int reg,
	unsigned int value
)
{
#if (I2C_MODE == HW_I2C)
	char data[4];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = (value>>8)& 0xFF;	
	data[3] = value& 0xFF;	
    return  gp_i2c_bus_write(g_i2c_handle, data, 4);

#elif (I2C_MODE == HW_TI2C)
	unsigned char data[4];

	data[0] = (reg >> 8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = (value>>8)& 0xFF;	
	data[3] = value& 0xFF;		
	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;	
	g_ti2c_handle.pBuf = data;	
	g_ti2c_handle.dataCnt = 4;	
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
///common 

sensor_write(0x3386, 0x2501); 		//MCU_BOOT_MODE
sensor_write(0x3386, 0x2500); 		//MCU_BOOT_MODE
msleep(10);			//DELAY = 10
sensor_write(0x301A, 0x0ACC); 		//RESET_REGISTER
sensor_write(0x3202, 0x0008); 		//STANDBY_CONTROL
msleep(10);					//DELAY = 10
sensor_write(0x338C, 0xA215);      //AE maxADChi 
sensor_write(0x3390, 0x0006);      //gain_thd , by jiujian
sensor_write(0x338C, 0xA206);	// MCU_ADDRESS
sensor_write(0x3390, 0x0036);	// AE_TARGET
sensor_write(0x338C, 0xA207);	// MCU_ADDRESS
sensor_write(0x3390, 0x0004);	// AE_GATE
sensor_write(0x3278, 0x0050);	// first black level
sensor_write(0x327a, 0x0050);	// first black level,red
sensor_write(0x327c, 0x0050);	// green_1
sensor_write(0x327e, 0x0050);	// green_2
sensor_write(0x3280, 0x0050);	// blue
msleep(10);	//DELAY = 10 
sensor_write(0x337e, 0x2000);	// Y/RGB offset
sensor_write(0x338C, 0xA34A); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0059); 		// AWB_GAIN_MIN
sensor_write(0x338C, 0xA34B); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00A6); 		// AWB_GAIN_MAX
sensor_write(0x338C, 0x235F); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0040); 		// AWB_CNT_PXL_TH
sensor_write(0x338C, 0xA361); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00D2); 		// AWB_TG_MIN0
sensor_write(0x338C, 0xA362); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00E6); 		// AWB_TG_MAX0
sensor_write(0x338C, 0xA363); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0010); 		// AWB_X0
sensor_write(0x338C, 0xA364); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00A0); 		// AWB_KR_L
sensor_write(0x338C, 0xA365); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0096); 		// AWB_KG_L
sensor_write(0x338C, 0xA366); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0080); 		// AWB_KB_L
sensor_write(0x338C, 0xA367); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0080); 		// AWB_KR_R
sensor_write(0x338C, 0xA368); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0080); 		// AWB_KG_R
sensor_write(0x338C, 0xA369); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0080); 		// AWB_KB_R
sensor_write(0x32A2, 0x3640); 		// RESERVED_SOC1_32A2  //fine tune color setting
sensor_write(0x338C, 0x2306); 		// MCU_ADDRESS
sensor_write(0x3390, 0x02FF); 		// AWB_CCM_L_0
sensor_write(0x338C, 0x2308); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFE6E); 		// AWB_CCM_L_1
sensor_write(0x338C, 0x230A); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFFC2); 		// AWB_CCM_L_2
sensor_write(0x338C, 0x230C); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFF4A); 		// AWB_CCM_L_3
sensor_write(0x338C, 0x230E); 		// MCU_ADDRESS
sensor_write(0x3390, 0x02D7); 		// AWB_CCM_L_4
sensor_write(0x338C, 0x2310); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFF30); 		// AWB_CCM_L_5
sensor_write(0x338C, 0x2312); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFF6E); 		// AWB_CCM_L_6
sensor_write(0x338C, 0x2314); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFDEE); 		// AWB_CCM_L_7
sensor_write(0x338C, 0x2316); 		// MCU_ADDRESS
sensor_write(0x3390, 0x03CF); 		// AWB_CCM_L_8
sensor_write(0x338C, 0x2318); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0020); 		// AWB_CCM_L_9
sensor_write(0x338C, 0x231A); 		// MCU_ADDRESS
sensor_write(0x3390, 0x003C); 		// AWB_CCM_L_10
sensor_write(0x338C, 0x231C); 		// MCU_ADDRESS
sensor_write(0x3390, 0x002C); 		// AWB_CCM_RL_0
sensor_write(0x338C, 0x231E); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFFBC); 		// AWB_CCM_RL_1
sensor_write(0x338C, 0x2320); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0016); 		// AWB_CCM_RL_2
sensor_write(0x338C, 0x2322); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0037); 		// AWB_CCM_RL_3
sensor_write(0x338C, 0x2324); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFFCD); 		// AWB_CCM_RL_4
sensor_write(0x338C, 0x2326); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFFF3); 		// AWB_CCM_RL_5
sensor_write(0x338C, 0x2328); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0077); 		// AWB_CCM_RL_6
sensor_write(0x338C, 0x232A); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00F4); 		// AWB_CCM_RL_7
sensor_write(0x338C, 0x232C); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFE95); 		// AWB_CCM_RL_8
sensor_write(0x338C, 0x232E); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0014); 		// AWB_CCM_RL_9
sensor_write(0x338C, 0x2330); 		// MCU_ADDRESS
sensor_write(0x3390, 0xFFE8); 		// AWB_CCM_RL_10  //end
sensor_write(0x338C, 0xA348); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0008); 		// AWB_GAIN_BUFFER_SPEED
sensor_write(0x338C, 0xA349); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0002); 		// AWB_JUMP_DIVISOR
sensor_write(0x338C, 0xA34A); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0059); 		// AWB_GAIN_MIN
sensor_write(0x338C, 0xA34B); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00A6); 		// AWB_GAIN_MAX
sensor_write(0x338C, 0xA34F); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0000); 		// AWB_CCM_POSITION_MIN
sensor_write(0x338C, 0xA350); 		// MCU_ADDRESS
sensor_write(0x3390, 0x007F); 		// AWB_CCM_POSITION_MAX
sensor_write(0x338C, 0xA352); 		// MCU_ADDRESS
sensor_write(0x3390, 0x001E); 		// AWB_SATURATION
sensor_write(0x338C, 0xA353); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0002); 		// AWB_MODE
sensor_write(0x338C, 0xA35B); 		// MCU_ADDRESS
sensor_write(0x3390, 0x007E); 		// AWB_STEADY_BGAIN_OUT_MIN
sensor_write(0x338C, 0xA35C); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0086); 		// AWB_STEADY_BGAIN_OUT_MAX
sensor_write(0x338C, 0xA35D); 		// MCU_ADDRESS
sensor_write(0x3390, 0x007F); 		// AWB_STEADY_BGAIN_IN_MIN
sensor_write(0x338C, 0xA35E); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0082); 		// AWB_STEADY_BGAIN_IN_MAX
sensor_write(0x338C, 0x235F); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0040); 		// AWB_CNT_PXL_TH
sensor_write(0x338C, 0xA361); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00D2); 		// AWB_TG_MIN0
sensor_write(0x338C, 0xA362); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00E6); 		// AWB_TG_MAX0
sensor_write(0x338C, 0xA302); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0000); 		// AWB_WINDOW_POS
sensor_write(0x338C, 0xA303); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00EF); 		// AWB_WINDOW_SIZE
sensor_write(0x338C, 0xAB05); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0000); 		// HG_PERCENT
sensor_write(0x338C, 0xA782); 		// MCU_ADDRESS
sensor_write(0x35A4, 0x0596); 		// BRIGHT_COLOR_KILL_CONTROLS
sensor_write(0x338C, 0xA118); 		// MCU_ADDRESS
sensor_write(0x3390, 0x001E); 		// SEQ_LLSAT1
sensor_write(0x338C, 0xA119); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0004); 		// SEQ_LLSAT2
sensor_write(0x338C, 0xA11A); 		// MCU_ADDRESS
sensor_write(0x3390, 0x000A); 		// SEQ_LLINTERPTHRESH1
sensor_write(0x338C, 0xA11B); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0020); 		// SEQ_LLINTERPTHRESH2
sensor_write(0x338C, 0xA13E); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_R
sensor_write(0x338C, 0xA13F); 		// MCU_ADDRESS
sensor_write(0x3390, 0x000E); 		// SEQ_NR_TH1_G
sensor_write(0x338C, 0xA140); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_B
sensor_write(0x338C, 0xA141); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_OL
sensor_write(0x338C, 0xA142); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_R
sensor_write(0x338C, 0xA143); 		// MCU_ADDRESS
sensor_write(0x3390, 0x000F); 		// SEQ_NR_TH2_G
sensor_write(0x338C, 0xA144); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_B
sensor_write(0x338C, 0xA145); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_OL
sensor_write(0x338C, 0xA146); 		// MCU_ADDRESS
sensor_write(0x3390, 0x0005); 		// SEQ_NR_GAINTH1
sensor_write(0x338C, 0xA147); 		// MCU_ADDRESS
sensor_write(0x3390, 0x003A); 		// SEQ_NR_GAINTH2
sensor_write(0x338C, 0xA404); 		// MCU_ADDRESS
sensor_write(0x3390, 0x00c0); 		// 50HZ  



return 0;
}

static int 
sensor_preview(void)
{
	//sensor_write(0x338c, 0xa20c);	// 
	//sensor_write(0x3390, 0x0005);//100 /4 ==zhenlv 
	#if  0
	////SVGA  800 X 600, V4L2_PIX_FMT_VYUY
	sensor_write(0x341E, 0x8F09);        //PLL/ Clk_in control: BYPASS PLL = 0x8F09
	sensor_write(0x341C, 0x0120);        //PLL Control 1 = 0x120
	msleep(10);						 // delay 10ms
	sensor_write(0x341E, 0x8F09);        //PLL/ Clk_in control: PLL ON, bypassed = 0x8F09
	sensor_write(0x341E, 0x8F08);        //PLL/ Clk_in control: USE PLL = 0x8F08
	sensor_write(0x3044, 0x0540); 		//Reserved = 1344
	sensor_write(0x3216, 0x02CF); 		//Internal Clock Control = 719
	sensor_write(0x321C, 0x0402); 		//OF Control Status = 1026
	sensor_write(0x3212, 0x0001); 		//Factory Bypass = 1	
	sensor_write(0x338C, 0x2703);        //Output Width (A)
	sensor_write(0x3390, 0x0322); 		//      = 800+2
	sensor_write(0x338C, 0x2705);        //Output Height (A)
	sensor_write(0x3390, 0x0258);        //      = 600
	sensor_write(0x338C, 0x270D);        //Row Start (A)
	sensor_write(0x3390, 0x0000); 		//      = 0
	sensor_write(0x338C, 0x270F);        //Column Start (A)
	sensor_write(0x3390, 0x0000); 		//      = 0
	sensor_write(0x338C, 0x2711);        //Row End (A)
	sensor_write(0x3390, 0x04BD); 		//      = 1213
	sensor_write(0x338C, 0x2713);        //Column End (A)
	sensor_write(0x3390, 0x0651);        //      = 1613+4
	sensor_write(0x338C, 0x2715);        //Extra Delay (A)
	sensor_write(0x3390, 0x026C); 		//      = 620
	sensor_write(0x338C, 0x2717);        //Row Speed (A)
	sensor_write(0x3390, 0x2111);        //      = 8465
	sensor_write(0x338C, 0x2719);        //Read Mode (A)
	sensor_write(0x3390, 0x046C);        //      = 1132
	sensor_write(0x338C, 0x271B);        //sensor_sample_time_pck (A)
	sensor_write(0x3390, 0x024F);        //      = 591
	sensor_write(0x338C, 0x271D);        //sensor_fine_correction (A)
	sensor_write(0x3390, 0x0102);        //      = 258
	sensor_write(0x338C, 0x271F);        //sensor_fine_IT_min (A)
	sensor_write(0x3390, 0x0279);        //      = 633
	sensor_write(0x338C, 0x2721);        //sensor_fine_IT_max_margin (A)
	sensor_write(0x3390, 0x0155);        //      = 341
	sensor_write(0x338C, 0x2723);        //Frame Lines (A)
	sensor_write(0x3390, 0x0293); 		//      = 659
	sensor_write(0x338C, 0x2725);        //Line Length (A)
	sensor_write(0x3390, 0x060F);        //      = 1551
	sensor_write(0x338C, 0x2727);        //sensor_dac_id_4_5 (A)
	sensor_write(0x3390, 0x2020);        //      = 8224
	sensor_write(0x338C, 0x2729);        //sensor_dac_id_6_7 (A)
	sensor_write(0x3390, 0x2020);        //      = 8224
	sensor_write(0x338C, 0x272B);        //sensor_dac_id_8_9 (A)
	sensor_write(0x3390, 0x1020);        //      = 4128
	sensor_write(0x338C, 0x272D);        //sensor_dac_id_10_11 (A)
	sensor_write(0x3390, 0x2007);        //      = 8199
	sensor_write(0x338C, 0x2751);        //Crop_X0 (A)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2753);        //Crop_X1 (A)
	sensor_write(0x3390, 0x0322); 		//      = 800+2
	sensor_write(0x338C, 0x2755);        //Crop_Y0 (A)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2757);        //Crop_Y1 (A)
	sensor_write(0x3390, 0x0258);        //      = 600
	sensor_write(0x338C, 0x222E);        //R9 Step
	sensor_write(0x3390, 0x006E); 		//      = 110
	sensor_write(0x338C, 0xA408);        //search_f1_50
	sensor_write(0x3390, 0x0018); 		//      = 24
	sensor_write(0x338C, 0xA409);        //search_f2_50
	sensor_write(0x3390, 0x001B); 		//      = 27
	sensor_write(0x338C, 0xA40A);        //search_f1_60
	sensor_write(0x3390, 0x0014); 		//      = 20
	sensor_write(0x338C, 0xA40B);        //search_f2_60
	sensor_write(0x3390, 0x0017); 		//      = 23
	sensor_write(0x338C, 0x2411); 		//R9_Step_60 (A)
	sensor_write(0x3390, 0x006E); 		//      = 110
	sensor_write(0x338C, 0x2413); 		//R9_Step_50 (A)
	sensor_write(0x3390, 0x0084); 		//      = 132
	sensor_write(0x338C, 0xA40D);        //Stat_min
	sensor_write(0x3390, 0x0002);        //      = 2
	sensor_write(0x338C, 0xA410);        //Min_amplitude
	sensor_write(0x3390, 0x0001);        //      = 1
	msleep(10);					//delay 10ms
	sensor_write(0x338C, 0xA103);        //Refresh Sequencer Mode
	sensor_write(0x3390, 0x0006);        //      = 6
	msleep(500);	//DELAY = 500
	sensor_write(0x338C, 0xA103);        //Refresh Sequencer
	sensor_write(0x3390, 0x0005);        //      = 5
	msleep(100); 	//DELAY = 100
	sensor_write(0x338C, 0xA120);	// MCU_ADDRESS
	sensor_write(0x3390, 0x0000);	// SEQ_CAP_MODE
	sensor_write(0x338C, 0xA103);	// MCU_ADDRESS
	sensor_write(0x3390, 0x0001);	// SEQ_CMD
	sensor_write(0x33f4, 0x031d);		//defect
	sensor_write(0x338c, 0xa118);     //saturation 
	sensor_write(0x3390, 0x0026);
	sensor_write(0x300c, 0x071c);
	sensor_write(0x338c, 0xa207);	// AE sensitivity
	sensor_write(0x3390, 0x0040);
	#else 
	//V4L2_PIX_FMT_UYVY
	
	sensor_write(0x341E, 0x8F09);        //PLL/ Clk_in control: BYPASS PLL = 0x8F09
	sensor_write(0x341C, 0x0120);        //PLL Control 1 = 0x120
	msleep(10);//DELAY = 1 Allow PLL to lock
	sensor_write(0x341E, 0x8F09);        //PLL/ Clk_in control: PLL ON, bypassed = 0x8F09
	sensor_write(0x341E, 0x8F08);        //PLL/ Clk_in control: USE PLL = 0x8F08
	sensor_write(0x338C, 0x2703); 		//Output Width (A)
	sensor_write(0x3390, 0x0280);        //      = 640
	sensor_write(0x338C, 0x2705);        //Output Height (A)
	sensor_write(0x3390, 0x01E0);        //      = 480
	sensor_write(0x338C, 0x2707);        //Output Width (B)
	sensor_write(0x3390, 0x0640);        //      = 1600
	sensor_write(0x338C, 0x2709);       //Output Height (B)
	sensor_write(0x3390, 0x04B0);        //      = 1200
	sensor_write(0x338C, 0x270D);        //Row Start (A)
	sensor_write(0x3390, 0x0078);        //      = 120
	sensor_write(0x338C, 0x270F);        //Column Start (A)
	sensor_write(0x3390, 0x00A0);        //      = 160
	sensor_write(0x338C, 0x2711);        //Row End (A)
	sensor_write(0x3390, 0x044d);        //      = 1101
	sensor_write(0x338C, 0x2713);        //Column End (A)
	sensor_write(0x3390, 0x05b5);        //      = 1461
	sensor_write(0x338C, 0x2715);        //Extra Delay (A)
	sensor_write(0x3390, 0x00AF);        //      = 175
	sensor_write(0x338C, 0x2717);        //Row Speed (A)
	sensor_write(0x3390, 0x2111);        //      = 8465
	sensor_write(0x338C, 0x2719);        //Read Mode (A)
	sensor_write(0x3390, 0x046C);        //      = 1132
	sensor_write(0x338C, 0x271B);        //sensor_sample_time_pck (A)
	sensor_write(0x3390, 0x024F);        //      = 591
	sensor_write(0x338C, 0x271D);        //sensor_fine_correction (A)
	sensor_write(0x3390, 0x0102);        //      = 258
	sensor_write(0x338C, 0x271F);        //sensor_fine_IT_min (A)
	sensor_write(0x3390, 0x0279);        //      = 633
	sensor_write(0x338C, 0x2721);        //sensor_fine_IT_max_margin (A)
	sensor_write(0x3390, 0x0155);        //      = 341
	sensor_write(0x338C, 0x2723);        //Frame Lines (A)
	sensor_write(0x3390, 0x0205);        //      = 575
	sensor_write(0x338C, 0x2725);        //Line Length (A)
	sensor_write(0x3390, 0x056F);        //      = 1391
	sensor_write(0x338C, 0x2727);        //sensor_dac_id_4_5 (A)
	sensor_write(0x3390, 0x2020);        //      = 8224
	sensor_write(0x338C, 0x2729);        //sensor_dac_id_6_7 (A)
	sensor_write(0x3390, 0x2020);        //      = 8224
	sensor_write(0x338C, 0x272B);        //sensor_dac_id_8_9 (A)
	sensor_write(0x3390, 0x1020);        //      = 4128
	sensor_write(0x338C, 0x272D);        //sensor_dac_id_10_11 (A)
	sensor_write(0x3390, 0x2007);        //      = 8199
	sensor_write(0x338c, 0x2795);		   // Natural , Swaps chrominance byte
	sensor_write(0x3390, 0x0002);
	sensor_write(0x338C, 0x272F);        //Row Start (B)
	sensor_write(0x3390, 0x0004);        //      = 4
	sensor_write(0x338C, 0x2731);        //Column Start (B)
	sensor_write(0x3390, 0x0004);        //      = 4
	sensor_write(0x338C, 0x2733);        //Row End (B)
	sensor_write(0x3390, 0x04BB);        //      = 1211
	sensor_write(0x338C, 0x2735);        //Column End (B)
	sensor_write(0x3390, 0x064B);        //      = 1611
	sensor_write(0x338C, 0x2737);        //Extra Delay (B)
	sensor_write(0x3390, 0x007C);        //      = 124
	sensor_write(0x338C, 0x2739);        //Row Speed (B)
	sensor_write(0x3390, 0x2111);        //      = 8465
	sensor_write(0x338C, 0x273B);        //Read Mode (B)
	sensor_write(0x3390, 0x0024);        //      = 36
	sensor_write(0x338C, 0x273D);        //sensor_sample_time_pck (B)
	sensor_write(0x3390, 0x0120);        //      = 288
	sensor_write(0x338C, 0x2741);        //sensor_fine_IT_min (B)
	sensor_write(0x3390, 0x0169);        //      = 361
	sensor_write(0x338C, 0x2745);        //Frame Lines (B)
	sensor_write(0x3390, 0x04FC);        //      = 1276
	sensor_write(0x338C, 0x2747);        //Line Length (B)
	sensor_write(0x3390, 0x092F);        //      = 2351
	sensor_write(0x338C, 0x2751);        //Crop_X0 (A)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2753);        //Crop_X1 (A)
	sensor_write(0x3390, 0x0280);        //      = 640
	sensor_write(0x338C, 0x2755);        //Crop_Y0 (A)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2757);        //Crop_Y1 (A)
	sensor_write(0x3390, 0x01E0);        //      = 480
	sensor_write(0x338C, 0x275F);        //Crop_X0 (B)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2761);        //Crop_X1 (B)
	sensor_write(0x3390, 0x0640);        //      = 1600
	sensor_write(0x338C, 0x2763);        //Crop_Y0 (B)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2765);        //Crop_Y1 (B)
	sensor_write(0x3390, 0x04B0);        //      = 1200
	sensor_write(0x338C, 0x222E);        //R9 Step
	sensor_write(0x3390, 0x0090);        //      = 144
	sensor_write(0x338C, 0xA408);        //search_f1_50
	sensor_write(0x3390, 0x001A);        //      = 26
	sensor_write(0x338C, 0xA409);        //search_f2_50
	sensor_write(0x3390, 0x001D);        //      = 29
	sensor_write(0x338C, 0xA40A);        //search_f1_60
	sensor_write(0x3390, 0x0020);        //      = 32
	sensor_write(0x338C, 0xA40B);        //search_f2_60
	sensor_write(0x3390, 0x0023);        //      = 35
	sensor_write(0x338C, 0x2411);        //R9_Step_60_A
	sensor_write(0x3390, 0x0090);        //      = 144
	sensor_write(0x338C, 0x2413);        //R9_Step_50_A
	sensor_write(0x3390, 0x00AD);        //      = 173
	sensor_write(0x338C, 0x2415);        //R9_Step_60_B
	sensor_write(0x3390, 0x0055);        //      = 85
	sensor_write(0x338C, 0x2417);        //R9_Step_50_B
	sensor_write(0x3390, 0x0066);        //      = 102
	sensor_write(0x34CE, 0x21A0);			/////////////LSC 
	sensor_write(0x34D0, 0x6633);
	sensor_write(0x34D2, 0x3299);
	sensor_write(0x34D4, 0x9563);
	sensor_write(0x34D6, 0x4B26);
	sensor_write(0x34D8, 0x2671);
	sensor_write(0x34DA, 0x714C);
	sensor_write(0x34DC, 0x0003);
	sensor_write(0x34DE, 0x019A);
	sensor_write(0x34E6, 0x00F9);
	sensor_write(0x34EE, 0x0B6B);
	sensor_write(0x34F6, 0x0D0D);
	sensor_write(0x3500, 0xAE3A);
	sensor_write(0x3508, 0x4A34);
	sensor_write(0x3510, 0x1C2A);
	sensor_write(0x3518, 0x282B);
	sensor_write(0x3520, 0x2F2E);
	sensor_write(0x3528, 0x3A3A);
	sensor_write(0x3530, 0x2931);
	sensor_write(0x3538, 0x34D8);
	sensor_write(0x354C, 0x0457);
	sensor_write(0x3544, 0x04A2);
	sensor_write(0x355C, 0x01CB);
	sensor_write(0x3554, 0x0021);
	sensor_write(0x34E0, 0x0182);
	sensor_write(0x34E8, 0x00C7);
	sensor_write(0x34F0, 0x0CE4);
	sensor_write(0x34F8, 0x0BD4);
	sensor_write(0x3502, 0x0614);
	sensor_write(0x350A, 0x2812);
	sensor_write(0x3512, 0x152B);
	sensor_write(0x351A, 0x2224);
	sensor_write(0x3522, 0x2827);
	sensor_write(0x352A, 0x2626);
	sensor_write(0x3532, 0x2A37);
	sensor_write(0x353A, 0x3A39);
	sensor_write(0x354E, 0x0508);
	sensor_write(0x3546, 0x000C);
	sensor_write(0x355E, 0x0035);
	sensor_write(0x3556, 0x0286);
	sensor_write(0x34E4, 0x013D);
	sensor_write(0x34EC, 0x0090);
	sensor_write(0x34F4, 0x0D87);
	sensor_write(0x34FC, 0x0CE9);
	sensor_write(0x3506, 0xF211);
	sensor_write(0x350E, 0x300C);
	sensor_write(0x3516, 0x1D26);
	sensor_write(0x351E, 0x1F1B);
	sensor_write(0x3526, 0x191F);
	sensor_write(0x352E, 0x161E);
	sensor_write(0x3536, 0x1E3A);
	sensor_write(0x353E, 0x4040);
	sensor_write(0x3552, 0x04F5);
	sensor_write(0x354A, 0x0200);
	sensor_write(0x3562, 0x049C);
	sensor_write(0x355A, 0x01B6);
	sensor_write(0x34E2, 0x0173);
	sensor_write(0x34EA, 0x00B2);
	sensor_write(0x34F2, 0x0B5F);
	sensor_write(0x34FA, 0x0B4B);
	sensor_write(0x3504, 0x144E);
	sensor_write(0x350C, 0x322C);
	sensor_write(0x3514, 0x1C24);
	sensor_write(0x351C, 0x2A28);
	sensor_write(0x3524, 0x2124);
	sensor_write(0x352C, 0x1D2C);
	sensor_write(0x3534, 0x1C14);
	sensor_write(0x353C, 0x21E7);
	sensor_write(0x3550, 0x0146);
	sensor_write(0x3548, 0x0477);
	sensor_write(0x3560, 0x00D9);
	sensor_write(0x3558, 0x047E);
	sensor_write(0x3540, 0x0000);
	sensor_write(0x3542, 0x0000);
	sensor_write(0x3210, 0x01FC);    ///////////end LSC
	sensor_write(0x338C, 0xA34A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0059); 		// AWB_GAIN_MIN
	sensor_write(0x338C, 0xA34B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00A6); 		// AWB_GAIN_MAX
	sensor_write(0x338C, 0x235F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0040); 		// AWB_CNT_PXL_TH
	sensor_write(0x338C, 0xA361); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00D2); 		// AWB_TG_MIN0
	sensor_write(0x338C, 0xA362); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00E6); 		// AWB_TG_MAX0
	sensor_write(0x338C, 0xA363); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0010); 		// AWB_X0
	sensor_write(0x338C, 0xA364); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00A0); 		// AWB_KR_L
	sensor_write(0x338C, 0xA365); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0096); 		// AWB_KG_L
	sensor_write(0x338C, 0xA366); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KB_L
	sensor_write(0x338C, 0xA367); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KR_R
	sensor_write(0x338C, 0xA368); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KG_R
	sensor_write(0x338C, 0xA369); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KB_R
	sensor_write(0x32A2, 0x3640); 		// RESERVED_SOC1_32A2  //fine tune color setting
	sensor_write(0x338C, 0x2306); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x02FF); 		// AWB_CCM_L_0
	sensor_write(0x338C, 0x2308); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFE6E); 		// AWB_CCM_L_1
	sensor_write(0x338C, 0x230A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFC2); 		// AWB_CCM_L_2
	sensor_write(0x338C, 0x230C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFF4A); 		// AWB_CCM_L_3
	sensor_write(0x338C, 0x230E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x02D7); 		// AWB_CCM_L_4
	sensor_write(0x338C, 0x2310); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFF30); 		// AWB_CCM_L_5
	sensor_write(0x338C, 0x2312); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFF6E); 		// AWB_CCM_L_6
	sensor_write(0x338C, 0x2314); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFDEE); 		// AWB_CCM_L_7
	sensor_write(0x338C, 0x2316); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x03CF); 		// AWB_CCM_L_8
	sensor_write(0x338C, 0x2318); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0020); 		// AWB_CCM_L_9
	sensor_write(0x338C, 0x231A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x003C); 		// AWB_CCM_L_10
	sensor_write(0x338C, 0x231C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x002C); 		// AWB_CCM_RL_0
	sensor_write(0x338C, 0x231E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFBC); 		// AWB_CCM_RL_1
	sensor_write(0x338C, 0x2320); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0016); 		// AWB_CCM_RL_2
	sensor_write(0x338C, 0x2322); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0037); 		// AWB_CCM_RL_3
	sensor_write(0x338C, 0x2324); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFCD); 		// AWB_CCM_RL_4
	sensor_write(0x338C, 0x2326); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFF3); 		// AWB_CCM_RL_5
	sensor_write(0x338C, 0x2328); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0077); 		// AWB_CCM_RL_6
	sensor_write(0x338C, 0x232A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00F4); 		// AWB_CCM_RL_7
	sensor_write(0x338C, 0x232C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFE95); 		// AWB_CCM_RL_8
	sensor_write(0x338C, 0x232E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0014); 		// AWB_CCM_RL_9
	sensor_write(0x338C, 0x2330); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFE8); 		// AWB_CCM_RL_10  //end
	sensor_write(0x338C, 0xA348); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0008); 		// AWB_GAIN_BUFFER_SPEED
	sensor_write(0x338C, 0xA349); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0002); 		// AWB_JUMP_DIVISOR
	sensor_write(0x338C, 0xA34A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0059); 		// AWB_GAIN_MIN
	sensor_write(0x338C, 0xA34B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00A6); 		// AWB_GAIN_MAX
	sensor_write(0x338C, 0xA34F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0000); 		// AWB_CCM_POSITION_MIN
	sensor_write(0x338C, 0xA350); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x007F); 		// AWB_CCM_POSITION_MAX
	sensor_write(0x338C, 0xA352); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x001E); 		// AWB_SATURATION
	sensor_write(0x338C, 0xA353); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0002); 		// AWB_MODE
	sensor_write(0x338C, 0xA35B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x007E); 		// AWB_STEADY_BGAIN_OUT_MIN
	sensor_write(0x338C, 0xA35C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0086); 		// AWB_STEADY_BGAIN_OUT_MAX
	sensor_write(0x338C, 0xA35D); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x007F); 		// AWB_STEADY_BGAIN_IN_MIN
	sensor_write(0x338C, 0xA35E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0082); 		// AWB_STEADY_BGAIN_IN_MAX
	sensor_write(0x338C, 0x235F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0040); 		// AWB_CNT_PXL_TH
	sensor_write(0x338C, 0xA361); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00D2); 		// AWB_TG_MIN0
	sensor_write(0x338C, 0xA362); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00E6); 		// AWB_TG_MAX0
	sensor_write(0x338C, 0xA302); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0000); 		// AWB_WINDOW_POS
	sensor_write(0x338C, 0xA303); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00EF); 		// AWB_WINDOW_SIZE
	sensor_write(0x338C, 0xAB05); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0000); 		// HG_PERCENT
	sensor_write(0x338C, 0xA782); 		// MCU_ADDRESS
	sensor_write(0x35A4, 0x0596); 		// BRIGHT_COLOR_KILL_CONTROLS
	sensor_write(0x338C, 0xA118); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x001E); 		// SEQ_LLSAT1
	sensor_write(0x338C, 0xA119); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_LLSAT2
	sensor_write(0x338C, 0xA11A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x000A); 		// SEQ_LLINTERPTHRESH1
	sensor_write(0x338C, 0xA11B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0020); 		// SEQ_LLINTERPTHRESH2
	sensor_write(0x338C, 0xA13E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_R
	sensor_write(0x338C, 0xA13F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x000E); 		// SEQ_NR_TH1_G
	sensor_write(0x338C, 0xA140); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_B
	sensor_write(0x338C, 0xA141); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_OL
	sensor_write(0x338C, 0xA142); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_R
	sensor_write(0x338C, 0xA143); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x000F); 		// SEQ_NR_TH2_G
	sensor_write(0x338C, 0xA144); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_B
	sensor_write(0x338C, 0xA145); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_OL
	sensor_write(0x338C, 0xA146); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0005); 		// SEQ_NR_GAINTH1
	sensor_write(0x338C, 0xA147); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x003A); 		// SEQ_NR_GAINTH2
	sensor_write(0x338C, 0xA215);      //AE maxADChi 
	sensor_write(0x3390, 0x0006);      //gain_thd , by ethan 
	sensor_write(0x338c, 0xa207);	// AE sensitivity
	sensor_write(0x3390, 0x0040);
	sensor_write(0x338C, 0xA40D);        //Stat_min
	sensor_write(0x3390, 0x0002);        //      = 2
	sensor_write(0x338C, 0xA410);        //Min_amplitude
	sensor_write(0x3390, 0x0001);        //      = 1
	msleep(10);
	sensor_write(0x338C, 0xA103);        //Refresh Sequencer Mode
	sensor_write(0x3390, 0x0006);        //      = 6
	msleep(500); //DELAY = 500
	sensor_write(0x338C, 0xA103);        //Refresh Sequencer
	sensor_write(0x3390, 0x0005);        //      = 5
	msleep(100);	//DELAY = 100
	sensor_write(0x338C, 0xA120);	// MCU_ADDRESS
	sensor_write(0x3390, 0x0000);	// SEQ_CAP_MODE
	sensor_write(0x338C, 0xA103);	// MCU_ADDRESS
	sensor_write(0x3390, 0x0001);	// SEQ_CMD
	sensor_write(0x33f4, 0x031d);		//defect
	sensor_write(0x338c, 0xa118);     //saturation 
	sensor_write(0x3390, 0x0026);


	#endif
	
	return 0;

}

static int 
sensor_capture(void)
{
	////UXGA  1600X1200
	sensor_write(0x301a,0x0acc);
	sensor_write(0x3202,0x0008);
	                    
	sensor_write(0x338c,0x2707);
	sensor_write(0x3390,0x0640);
	sensor_write(0x338c,0x2709);
	sensor_write(0x3390,0x04B0);
	sensor_write(0x338c,0x275f);
	sensor_write(0x3390,0x0000);
	sensor_write(0x338c,0x2761);
	sensor_write(0x3390,0x0640);
	sensor_write(0x338c,0x2763);
	sensor_write(0x3390,0x0000);
	sensor_write(0x338c,0x2765);
	sensor_write(0x3390,0x04b0);
	                    
	sensor_write(0x338c,0xa120);
	sensor_write(0x3390,0x0072);

	sensor_write(0x338c,0xa103);
	sensor_write(0x3390,0x0002);

	sensor_write(0x338C, 0xA102);	
	sensor_write(0x3390, 0x000f);//DISABLE AE
	
	msleep(500);
	return 0;
}

static int 
sensor_record(void)
{
//	dv_flag= 1;
	////VGA
//	sensor_write(0x338c, 0xa20c);	// 
//	sensor_write(0x3390, 0x0004);//100 /4 ==zhenlv 

	
	sensor_write(0x341E, 0x8F09);        //PLL/ Clk_in control: BYPASS PLL = 0x8F09
	sensor_write(0x341C, 0x0120);        //PLL Control 1 = 0x120
	msleep(10);//DELAY = 1 Allow PLL to lock
	sensor_write(0x341E, 0x8F09);        //PLL/ Clk_in control: PLL ON, bypassed = 0x8F09
	sensor_write(0x341E, 0x8F08);        //PLL/ Clk_in control: USE PLL = 0x8F08
	sensor_write(0x338C, 0x2703); 		//Output Width (A)
	sensor_write(0x3390, 0x0280);        //      = 640
	sensor_write(0x338C, 0x2705);        //Output Height (A)
	sensor_write(0x3390, 0x01E0);        //      = 480
	sensor_write(0x338C, 0x2707);        //Output Width (B)
	sensor_write(0x3390, 0x0640);        //      = 1600
	sensor_write(0x338C, 0x2709);       //Output Height (B)
	sensor_write(0x3390, 0x04B0);        //      = 1200
	sensor_write(0x338C, 0x270D);        //Row Start (A)
	sensor_write(0x3390, 0x0078);        //      = 120
	sensor_write(0x338C, 0x270F);        //Column Start (A)
	sensor_write(0x3390, 0x00A0);        //      = 160
	sensor_write(0x338C, 0x2711);        //Row End (A)
	sensor_write(0x3390, 0x044d);        //      = 1101
	sensor_write(0x338C, 0x2713);        //Column End (A)
	sensor_write(0x3390, 0x05b5);        //      = 1461
	sensor_write(0x338C, 0x2715);        //Extra Delay (A)
	sensor_write(0x3390, 0x00AF);        //      = 175
	sensor_write(0x338C, 0x2717);        //Row Speed (A)
	sensor_write(0x3390, 0x2111);        //      = 8465
	sensor_write(0x338C, 0x2719);        //Read Mode (A)
	sensor_write(0x3390, 0x046C);        //      = 1132
	sensor_write(0x338C, 0x271B);        //sensor_sample_time_pck (A)
	sensor_write(0x3390, 0x024F);        //      = 591
	sensor_write(0x338C, 0x271D);        //sensor_fine_correction (A)
	sensor_write(0x3390, 0x0102);        //      = 258
	sensor_write(0x338C, 0x271F);        //sensor_fine_IT_min (A)
	sensor_write(0x3390, 0x0279);        //      = 633
	sensor_write(0x338C, 0x2721);        //sensor_fine_IT_max_margin (A)
	sensor_write(0x3390, 0x0155);        //      = 341
	sensor_write(0x338C, 0x2723);        //Frame Lines (A)
	sensor_write(0x3390, 0x0205);        //      = 575
	sensor_write(0x338C, 0x2725);        //Line Length (A)
	sensor_write(0x3390, 0x056F);        //      = 1391
	sensor_write(0x338C, 0x2727);        //sensor_dac_id_4_5 (A)
	sensor_write(0x3390, 0x2020);        //      = 8224
	sensor_write(0x338C, 0x2729);        //sensor_dac_id_6_7 (A)
	sensor_write(0x3390, 0x2020);        //      = 8224
	sensor_write(0x338C, 0x272B);        //sensor_dac_id_8_9 (A)
	sensor_write(0x3390, 0x1020);        //      = 4128
	sensor_write(0x338C, 0x272D);        //sensor_dac_id_10_11 (A)
	sensor_write(0x3390, 0x2007);        //      = 8199
	sensor_write(0x338c, 0x2795);		   // Natural , Swaps chrominance byte
	sensor_write(0x3390, 0x0002);
	sensor_write(0x338C, 0x272F);        //Row Start (B)
	sensor_write(0x3390, 0x0004);        //      = 4
	sensor_write(0x338C, 0x2731);        //Column Start (B)
	sensor_write(0x3390, 0x0004);        //      = 4
	sensor_write(0x338C, 0x2733);        //Row End (B)
	sensor_write(0x3390, 0x04BB);        //      = 1211
	sensor_write(0x338C, 0x2735);        //Column End (B)
	sensor_write(0x3390, 0x064B);        //      = 1611
	sensor_write(0x338C, 0x2737);        //Extra Delay (B)
	sensor_write(0x3390, 0x007C);        //      = 124
	sensor_write(0x338C, 0x2739);        //Row Speed (B)
	sensor_write(0x3390, 0x2111);        //      = 8465
	sensor_write(0x338C, 0x273B);        //Read Mode (B)
	sensor_write(0x3390, 0x0024);        //      = 36
	sensor_write(0x338C, 0x273D);        //sensor_sample_time_pck (B)
	sensor_write(0x3390, 0x0120);        //      = 288
	sensor_write(0x338C, 0x2741);        //sensor_fine_IT_min (B)
	sensor_write(0x3390, 0x0169);        //      = 361
	sensor_write(0x338C, 0x2745);        //Frame Lines (B)
	sensor_write(0x3390, 0x04FC);        //      = 1276
	sensor_write(0x338C, 0x2747);        //Line Length (B)
	sensor_write(0x3390, 0x092F);        //      = 2351
	sensor_write(0x338C, 0x2751);        //Crop_X0 (A)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2753);        //Crop_X1 (A)
	sensor_write(0x3390, 0x0280);        //      = 640
	sensor_write(0x338C, 0x2755);        //Crop_Y0 (A)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2757);        //Crop_Y1 (A)
	sensor_write(0x3390, 0x01E0);        //      = 480
	sensor_write(0x338C, 0x275F);        //Crop_X0 (B)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2761);        //Crop_X1 (B)
	sensor_write(0x3390, 0x0640);        //      = 1600
	sensor_write(0x338C, 0x2763);        //Crop_Y0 (B)
	sensor_write(0x3390, 0x0000);        //      = 0
	sensor_write(0x338C, 0x2765);        //Crop_Y1 (B)
	sensor_write(0x3390, 0x04B0);        //      = 1200
	sensor_write(0x338C, 0x222E);        //R9 Step
	sensor_write(0x3390, 0x0090);        //      = 144
	sensor_write(0x338C, 0xA408);        //search_f1_50
	sensor_write(0x3390, 0x001A);        //      = 26
	sensor_write(0x338C, 0xA409);        //search_f2_50
	sensor_write(0x3390, 0x001D);        //      = 29
	sensor_write(0x338C, 0xA40A);        //search_f1_60
	sensor_write(0x3390, 0x0020);        //      = 32
	sensor_write(0x338C, 0xA40B);        //search_f2_60
	sensor_write(0x3390, 0x0023);        //      = 35
	sensor_write(0x338C, 0x2411);        //R9_Step_60_A
	sensor_write(0x3390, 0x0090);        //      = 144
	sensor_write(0x338C, 0x2413);        //R9_Step_50_A
	sensor_write(0x3390, 0x00AD);        //      = 173
	sensor_write(0x338C, 0x2415);        //R9_Step_60_B
	sensor_write(0x3390, 0x0055);        //      = 85
	sensor_write(0x338C, 0x2417);        //R9_Step_50_B
	sensor_write(0x3390, 0x0066);        //      = 102
	sensor_write(0x34CE, 0x21A0);			/////////////LSC 
	sensor_write(0x34D0, 0x6633);
	sensor_write(0x34D2, 0x3299);
	sensor_write(0x34D4, 0x9563);
	sensor_write(0x34D6, 0x4B26);
	sensor_write(0x34D8, 0x2671);
	sensor_write(0x34DA, 0x714C);
	sensor_write(0x34DC, 0x0003);
	sensor_write(0x34DE, 0x019A);
	sensor_write(0x34E6, 0x00F9);
	sensor_write(0x34EE, 0x0B6B);
	sensor_write(0x34F6, 0x0D0D);
	sensor_write(0x3500, 0xAE3A);
	sensor_write(0x3508, 0x4A34);
	sensor_write(0x3510, 0x1C2A);
	sensor_write(0x3518, 0x282B);
	sensor_write(0x3520, 0x2F2E);
	sensor_write(0x3528, 0x3A3A);
	sensor_write(0x3530, 0x2931);
	sensor_write(0x3538, 0x34D8);
	sensor_write(0x354C, 0x0457);
	sensor_write(0x3544, 0x04A2);
	sensor_write(0x355C, 0x01CB);
	sensor_write(0x3554, 0x0021);
	sensor_write(0x34E0, 0x0182);
	sensor_write(0x34E8, 0x00C7);
	sensor_write(0x34F0, 0x0CE4);
	sensor_write(0x34F8, 0x0BD4);
	sensor_write(0x3502, 0x0614);
	sensor_write(0x350A, 0x2812);
	sensor_write(0x3512, 0x152B);
	sensor_write(0x351A, 0x2224);
	sensor_write(0x3522, 0x2827);
	sensor_write(0x352A, 0x2626);
	sensor_write(0x3532, 0x2A37);
	sensor_write(0x353A, 0x3A39);
	sensor_write(0x354E, 0x0508);
	sensor_write(0x3546, 0x000C);
	sensor_write(0x355E, 0x0035);
	sensor_write(0x3556, 0x0286);
	sensor_write(0x34E4, 0x013D);
	sensor_write(0x34EC, 0x0090);
	sensor_write(0x34F4, 0x0D87);
	sensor_write(0x34FC, 0x0CE9);
	sensor_write(0x3506, 0xF211);
	sensor_write(0x350E, 0x300C);
	sensor_write(0x3516, 0x1D26);
	sensor_write(0x351E, 0x1F1B);
	sensor_write(0x3526, 0x191F);
	sensor_write(0x352E, 0x161E);
	sensor_write(0x3536, 0x1E3A);
	sensor_write(0x353E, 0x4040);
	sensor_write(0x3552, 0x04F5);
	sensor_write(0x354A, 0x0200);
	sensor_write(0x3562, 0x049C);
	sensor_write(0x355A, 0x01B6);
	sensor_write(0x34E2, 0x0173);
	sensor_write(0x34EA, 0x00B2);
	sensor_write(0x34F2, 0x0B5F);
	sensor_write(0x34FA, 0x0B4B);
	sensor_write(0x3504, 0x144E);
	sensor_write(0x350C, 0x322C);
	sensor_write(0x3514, 0x1C24);
	sensor_write(0x351C, 0x2A28);
	sensor_write(0x3524, 0x2124);
	sensor_write(0x352C, 0x1D2C);
	sensor_write(0x3534, 0x1C14);
	sensor_write(0x353C, 0x21E7);
	sensor_write(0x3550, 0x0146);
	sensor_write(0x3548, 0x0477);
	sensor_write(0x3560, 0x00D9);
	sensor_write(0x3558, 0x047E);
	sensor_write(0x3540, 0x0000);
	sensor_write(0x3542, 0x0000);
	sensor_write(0x3210, 0x01FC);    ///////////end LSC
	sensor_write(0x338C, 0xA34A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0059); 		// AWB_GAIN_MIN
	sensor_write(0x338C, 0xA34B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00A6); 		// AWB_GAIN_MAX
	sensor_write(0x338C, 0x235F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0040); 		// AWB_CNT_PXL_TH
	sensor_write(0x338C, 0xA361); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00D2); 		// AWB_TG_MIN0
	sensor_write(0x338C, 0xA362); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00E6); 		// AWB_TG_MAX0
	sensor_write(0x338C, 0xA363); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0010); 		// AWB_X0
	sensor_write(0x338C, 0xA364); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00A0); 		// AWB_KR_L
	sensor_write(0x338C, 0xA365); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0096); 		// AWB_KG_L
	sensor_write(0x338C, 0xA366); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KB_L
	sensor_write(0x338C, 0xA367); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KR_R
	sensor_write(0x338C, 0xA368); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KG_R
	sensor_write(0x338C, 0xA369); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0080); 		// AWB_KB_R
	sensor_write(0x32A2, 0x3640); 		// RESERVED_SOC1_32A2  //fine tune color setting
	sensor_write(0x338C, 0x2306); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x02FF); 		// AWB_CCM_L_0
	sensor_write(0x338C, 0x2308); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFE6E); 		// AWB_CCM_L_1
	sensor_write(0x338C, 0x230A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFC2); 		// AWB_CCM_L_2
	sensor_write(0x338C, 0x230C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFF4A); 		// AWB_CCM_L_3
	sensor_write(0x338C, 0x230E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x02D7); 		// AWB_CCM_L_4
	sensor_write(0x338C, 0x2310); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFF30); 		// AWB_CCM_L_5
	sensor_write(0x338C, 0x2312); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFF6E); 		// AWB_CCM_L_6
	sensor_write(0x338C, 0x2314); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFDEE); 		// AWB_CCM_L_7
	sensor_write(0x338C, 0x2316); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x03CF); 		// AWB_CCM_L_8
	sensor_write(0x338C, 0x2318); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0020); 		// AWB_CCM_L_9
	sensor_write(0x338C, 0x231A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x003C); 		// AWB_CCM_L_10
	sensor_write(0x338C, 0x231C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x002C); 		// AWB_CCM_RL_0
	sensor_write(0x338C, 0x231E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFBC); 		// AWB_CCM_RL_1
	sensor_write(0x338C, 0x2320); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0016); 		// AWB_CCM_RL_2
	sensor_write(0x338C, 0x2322); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0037); 		// AWB_CCM_RL_3
	sensor_write(0x338C, 0x2324); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFCD); 		// AWB_CCM_RL_4
	sensor_write(0x338C, 0x2326); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFF3); 		// AWB_CCM_RL_5
	sensor_write(0x338C, 0x2328); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0077); 		// AWB_CCM_RL_6
	sensor_write(0x338C, 0x232A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00F4); 		// AWB_CCM_RL_7
	sensor_write(0x338C, 0x232C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFE95); 		// AWB_CCM_RL_8
	sensor_write(0x338C, 0x232E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0014); 		// AWB_CCM_RL_9
	sensor_write(0x338C, 0x2330); 		// MCU_ADDRESS
	sensor_write(0x3390, 0xFFE8); 		// AWB_CCM_RL_10  //end
	sensor_write(0x338C, 0xA348); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0008); 		// AWB_GAIN_BUFFER_SPEED
	sensor_write(0x338C, 0xA349); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0002); 		// AWB_JUMP_DIVISOR
	sensor_write(0x338C, 0xA34A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0059); 		// AWB_GAIN_MIN
	sensor_write(0x338C, 0xA34B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00A6); 		// AWB_GAIN_MAX
	sensor_write(0x338C, 0xA34F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0000); 		// AWB_CCM_POSITION_MIN
	sensor_write(0x338C, 0xA350); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x007F); 		// AWB_CCM_POSITION_MAX
	sensor_write(0x338C, 0xA352); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x001E); 		// AWB_SATURATION
	sensor_write(0x338C, 0xA353); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0002); 		// AWB_MODE
	sensor_write(0x338C, 0xA35B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x007E); 		// AWB_STEADY_BGAIN_OUT_MIN
	sensor_write(0x338C, 0xA35C); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0086); 		// AWB_STEADY_BGAIN_OUT_MAX
	sensor_write(0x338C, 0xA35D); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x007F); 		// AWB_STEADY_BGAIN_IN_MIN
	sensor_write(0x338C, 0xA35E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0082); 		// AWB_STEADY_BGAIN_IN_MAX
	sensor_write(0x338C, 0x235F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0040); 		// AWB_CNT_PXL_TH
	sensor_write(0x338C, 0xA361); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00D2); 		// AWB_TG_MIN0
	sensor_write(0x338C, 0xA362); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00E6); 		// AWB_TG_MAX0
	sensor_write(0x338C, 0xA302); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0000); 		// AWB_WINDOW_POS
	sensor_write(0x338C, 0xA303); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x00EF); 		// AWB_WINDOW_SIZE
	sensor_write(0x338C, 0xAB05); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0000); 		// HG_PERCENT
	sensor_write(0x338C, 0xA782); 		// MCU_ADDRESS
	sensor_write(0x35A4, 0x0596); 		// BRIGHT_COLOR_KILL_CONTROLS
	sensor_write(0x338C, 0xA118); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x001E); 		// SEQ_LLSAT1
	sensor_write(0x338C, 0xA119); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_LLSAT2
	sensor_write(0x338C, 0xA11A); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x000A); 		// SEQ_LLINTERPTHRESH1
	sensor_write(0x338C, 0xA11B); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0020); 		// SEQ_LLINTERPTHRESH2
	sensor_write(0x338C, 0xA13E); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_R
	sensor_write(0x338C, 0xA13F); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x000E); 		// SEQ_NR_TH1_G
	sensor_write(0x338C, 0xA140); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_B
	sensor_write(0x338C, 0xA141); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0004); 		// SEQ_NR_TH1_OL
	sensor_write(0x338C, 0xA142); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_R
	sensor_write(0x338C, 0xA143); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x000F); 		// SEQ_NR_TH2_G
	sensor_write(0x338C, 0xA144); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_B
	sensor_write(0x338C, 0xA145); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0032); 		// SEQ_NR_TH2_OL
	sensor_write(0x338C, 0xA146); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x0005); 		// SEQ_NR_GAINTH1
	sensor_write(0x338C, 0xA147); 		// MCU_ADDRESS
	sensor_write(0x3390, 0x003A); 		// SEQ_NR_GAINTH2
	sensor_write(0x338C, 0xA215);      //AE maxADChi 
	sensor_write(0x3390, 0x0006);      //gain_thd , by ethan 
	sensor_write(0x338c, 0xa207);	// AE sensitivity
	sensor_write(0x3390, 0x0040);
	sensor_write(0x338C, 0xA40D);        //Stat_min
	sensor_write(0x3390, 0x0002);        //      = 2
	sensor_write(0x338C, 0xA410);        //Min_amplitude
	sensor_write(0x3390, 0x0001);        //      = 1
	msleep(10);
	sensor_write(0x338C, 0xA103);        //Refresh Sequencer Mode
	sensor_write(0x3390, 0x0006);        //      = 6
	msleep(500); //DELAY = 500
	sensor_write(0x338C, 0xA103);        //Refresh Sequencer
	sensor_write(0x3390, 0x0005);        //      = 5
	msleep(100);	//DELAY = 100
	sensor_write(0x338C, 0xA120);	// MCU_ADDRESS
	sensor_write(0x3390, 0x0000);	// SEQ_CAP_MODE
	sensor_write(0x338C, 0xA103);	// MCU_ADDRESS
	sensor_write(0x3390, 0x0001);	// SEQ_CMD
	sensor_write(0x33f4, 0x031d);		//defect
	sensor_write(0x338c, 0xa118);     //saturation 
	sensor_write(0x3390, 0x0026);

return 0;

}

static int 
sensor_reset(
	struct v4l2_subdev *sd, 
	u32 val
)
{
	//dv_flag = 0;
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
	unsigned int data;
	int nRet = 0;
	switch(ctrl->id)
	{
	case V4L2_CID_BRIGHTNESS:
		break;
		
	case V4L2_CID_CONTRAST:
		break;
		
	case V4L2_CID_VFLIP:
		break;
		
	case V4L2_CID_HFLIP:
		break;
		
	case V4L2_CID_SATURATION:
		break;
		
	case V4L2_CID_HUE:
		break;
		
	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("WBAUTO = %d\n", ctrl->value);

		if(ctrl->value)
		{
			nRet = sensor_write(0x338c, 0xa102);
			nRet = sensor_read(0x3390, (unsigned int*)&data );
			nRet = sensor_write(0x3390, data|= 0x0004);		
		}
		else
		{
		}
		break;
	
	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED)
		{

		}
		else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ)
		{
			nRet = sensor_write(0x338C, 0xA404);
			nRet = sensor_read(0x3390, (unsigned int *)&data );
			nRet = sensor_write(0x3390, data|= 0x40);		
		}
		else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ)
		{
			nRet = sensor_write(0x338C, 0xA404);
			nRet = sensor_read(0x3390, (unsigned int *)&data);
			nRet = sensor_write(0x3390, data &= 0xFFBF);		
		}
		else 
		{
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) //SUNSHINE
		{
			nRet = sensor_write(0x338c, 0xa102);
			nRet = sensor_read(0x3390, (unsigned int)&data );
			nRet = sensor_write(0x3390, data&= 0xFFFB);		

			nRet = sensor_write(0x338C, 0xa34c); 	
			nRet = sensor_write(0x3390, 0x0040); 	
			nRet = sensor_write(0x338C, 0xa34e); 	
			nRet = sensor_write(0x3390, 0x007d); 	
		}
		else if(ctrl->value == 1)//CLOUDY
		{
			nRet = sensor_write(0x338c, 0xa102);
			nRet = sensor_read(0x3390, (unsigned int *)&data );
			nRet = sensor_write(0x3390, data&= 0xFFFB);		

			nRet = sensor_write(0x338C, 0xa34c); 	
			nRet = sensor_write(0x3390, 0x007f); 	
			nRet = sensor_write(0x338C, 0xa34e); 	
			nRet = sensor_write(0x3390, 0x0040); 	
		}
		else if(ctrl->value == 2)//FLUORESCENCE
		{
			nRet = sensor_write(0x338c, 0xa102);
			nRet = sensor_read(0x3390, (unsigned int *)&data );
			nRet = sensor_write(0x3390, data&= 0xFFFB);		

			nRet = sensor_write(0x338C, 0xa34c); 	
			nRet = sensor_write(0x3390, 0x009b); 	
			nRet = sensor_write(0x338C, 0xa34e); 	
			nRet = sensor_write(0x3390, 0x00d1); 	
		}
		else if(ctrl->value == 3)//INCANDESCENCE
		{
			nRet = sensor_write(0x338c, 0xa102);
			nRet = sensor_read(0x3390, (unsigned int *)&data );
			nRet = sensor_write(0x3390, data&= 0xFFFB);		

			nRet = sensor_write(0x338C, 0xa34c); 	
			nRet = sensor_write(0x3390, 0x00c0); 	
			nRet = sensor_write(0x338C, 0xa34e); 	
			nRet = sensor_write(0x3390, 0x007e); 	
		}
		break;

	case V4L2_CID_BACKLIGHT_COMPENSATION:
		
		printk("NightMode = %d\n", ctrl->value);
		if(ctrl->value)
		{
		       sensor_write(0x338C, 0xA20c); 
		       sensor_write(0x3390, 0x000a); 
			sensor_write(0x338C, 0xA102); 	// MCU_ADDRESS [SEQ_MODE]
			sensor_write(0x3390, 0x000F); 	// MCU_DATA_0
			sensor_write(0x338C, 0xA206); 	// MCU_ADDRESS [AE_TARGET]
			sensor_write(0x3390, 0x0068); 	// MCU_DATA_0
			sensor_write(0x338C, 0xA207); 	// MCU_ADDRESS [AE_GATE]
			sensor_write(0x3390, 0x0018); 	// MCU_DATA_0
			sensor_write(0x338C, 0xA103); 	// MCU_ADDRESS 
			sensor_write(0x3390, 0x0005); 	// MCU_DATA_0
			

		}
		else
		{	
			//if (dv_flag ==0){
				sensor_write(0x338C, 0xA102); 	// MCU_ADDRESS [SEQ_MODE]
				sensor_write(0x3390, 0x000F); 	// MCU_DATA_0
				sensor_write(0x338C, 0xA206); 	// MCU_ADDRESS [AE_TARGET]
				sensor_write(0x3390, 0x0030); 	// MCU_DATA_0
				sensor_write(0x338C, 0xA207); 	// MCU_ADDRESS [AE_GATE]
				sensor_write(0x3390, 0x0018); 	// MCU_DATA_0
			       sensor_write(0x338C, 0xA20c); 
			       sensor_write(0x3390, 0x0003); 
				sensor_write(0x338C, 0xA103); 	// MCU_ADDRESS 
				sensor_write(0x3390, 0x0005); 	// MCU_DATA_0
			//}


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
	if(sensor_i2c_open(MT9D112_ID, 100) < 0) {
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: mt_9d112 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_mt_9d112");
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
MODULE_DESCRIPTION("Generalplus mt_9d112 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

