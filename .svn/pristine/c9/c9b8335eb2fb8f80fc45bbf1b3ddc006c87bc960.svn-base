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
#define GC2015_ID		0x60

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
		.pixelformat = V4L2_PIX_FMT_VYUY,
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
		.pixelformat = V4L2_PIX_FMT_VYUY,
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
		.pixelformat = V4L2_PIX_FMT_VYUY,
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
	g_ti2c_handle.pDeviceString = "GC2015";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("GC2015 ti2c request failed\n");
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

void Gp_GC2015_Gamma_Select(unsigned int GammaLvl)
{
	switch(GammaLvl)
	{
		case 1:                                             //smallest gamma curve
			sensor_write( 0xbF, 0x0B ); 
			sensor_write( 0xc0, 0x16 ); 
			sensor_write( 0xc1, 0x29 ); 
			sensor_write( 0xc2, 0x3C ); 
			sensor_write( 0xc3, 0x4F ); 
			sensor_write( 0xc4, 0x5F ); 
			sensor_write( 0xc5, 0x6F ); 
			sensor_write( 0xc6, 0x8A ); 
			sensor_write( 0xc7, 0x9F ); 
			sensor_write( 0xc8, 0xB4 ); 
			sensor_write( 0xc9, 0xC6 ); 
			sensor_write( 0xcA, 0xD3 ); 
			sensor_write( 0xcB, 0xDD );  
			sensor_write( 0xcC, 0xE5 );  
			sensor_write( 0xcD, 0xF1 ); 
			sensor_write( 0xcE, 0xFA ); 
			sensor_write( 0xcF, 0xFF ); 	
			break;
		case 2:			
			sensor_write( 0xbF, 0x0E ); 
			sensor_write( 0xc0, 0x1C ); 
			sensor_write( 0xc1, 0x34 ); 
			sensor_write( 0xc2, 0x48 ); 
			sensor_write( 0xc3, 0x5A ); 
			sensor_write( 0xc4, 0x6B ); 
			sensor_write( 0xc5, 0x7B ); 
			sensor_write( 0xc6, 0x95 ); 
			sensor_write( 0xc7, 0xAB ); 
			sensor_write( 0xc8, 0xBF );
			sensor_write( 0xc9, 0xCE ); 
			sensor_write( 0xcA, 0xD9 ); 
			sensor_write( 0xcB, 0xE4 );  
			sensor_write( 0xcC, 0xEC ); 
			sensor_write( 0xcD, 0xF7 ); 
			sensor_write( 0xcE, 0xFD ); 
			sensor_write( 0xcF, 0xFF ); 
		break;
		case 3:
			sensor_write( 0xbF, 0x10 ); 
			sensor_write( 0xc0, 0x20 ); 
			sensor_write( 0xc1, 0x38 ); 
			sensor_write( 0xc2, 0x4E ); 
			sensor_write( 0xc3, 0x63 ); 
			sensor_write( 0xc4, 0x76 ); 
			sensor_write( 0xc5, 0x87 ); 
			sensor_write( 0xc6, 0xA2 ); 
			sensor_write( 0xc7, 0xB8 ); 
			sensor_write( 0xc8, 0xCA ); 
			sensor_write( 0xc9, 0xD8 ); 
			sensor_write( 0xcA, 0xE3 ); 
			sensor_write( 0xcB, 0xEB ); 
			sensor_write( 0xcC, 0xF0 ); 
			sensor_write( 0xcD, 0xF8 ); 
			sensor_write( 0xcE, 0xFD ); 
			sensor_write( 0xcF, 0xFF ); 

			break;
		case 4:
			sensor_write( 0xbF, 0x14 ); 
			sensor_write( 0xc0, 0x28 ); 
			sensor_write( 0xc1, 0x44 ); 
			sensor_write( 0xc2, 0x5D ); 
			sensor_write( 0xc3, 0x72 ); 
			sensor_write( 0xc4, 0x86 ); 
			sensor_write( 0xc5, 0x95 ); 
			sensor_write( 0xc6, 0xB1 ); 
			sensor_write( 0xc7, 0xC6 ); 
			sensor_write( 0xc8, 0xD5 ); 
			sensor_write( 0xc9, 0xE1 ); 
			sensor_write( 0xcA, 0xEA ); 
			sensor_write( 0xcB, 0xF1 ); 
			sensor_write( 0xcC, 0xF5 ); 
			sensor_write( 0xcD, 0xFB ); 
			sensor_write( 0xcE, 0xFE ); 
			sensor_write( 0xcF, 0xFF );
		break;
		case 5:								// largest gamma curve
			sensor_write( 0xbF, 0x15 ); 
			sensor_write( 0xc0, 0x2A ); 
			sensor_write( 0xc1, 0x4A ); 
			sensor_write( 0xc2, 0x67 ); 
			sensor_write( 0xc3, 0x79 ); 
			sensor_write( 0xc4, 0x8C ); 
			sensor_write( 0xc5, 0x9A ); 
			sensor_write( 0xc6, 0xB3 ); 
			sensor_write( 0xc7, 0xC5 ); 
			sensor_write( 0xc8, 0xD5 ); 
			sensor_write( 0xc9, 0xDF ); 
			sensor_write( 0xcA, 0xE8 ); 
			sensor_write( 0xcB, 0xEE ); 
			sensor_write( 0xcC, 0xF3 ); 
			sensor_write( 0xcD, 0xFA ); 
			sensor_write( 0xcE, 0xFD ); 
			sensor_write( 0xcF, 0xFF );
			break;
		default:
		break;
	}
}

static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
/*
	char data = 0;
	sensor_read(0x00,&data);
		printk("read -> data = 0x%x\n",data);
*/	

	sensor_write(0xfe , 0x80);  //soft reset
	sensor_write(0xfe , 0x80);  //soft reset
	sensor_write(0xfe , 0x80);  //soft reset
	
	sensor_write(0xfe , 0x00);  //page0
	sensor_write(0x45 , 0x00);  //output_enable
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////preview capture switch /////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	//preview
	sensor_write(0x02 , 0x01);  //preview mode
	sensor_write(0x2a , 0xca);  //[7]col_binning , 0x[6]even skip
	sensor_write(0x48 , 0x40);  //manual_gain

	sensor_write(0xfe , 0x01);  //page1
	////////////////////////////////////////////////////////////////////////
	////////////////////////// preview LSC /////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	
	sensor_write(0xb0 , 0x13);  //[4]Y_LSC_en [3]lsc_compensate [2]signed_b4 [1:0]pixel array select
	sensor_write(0xb1 , 0x20);  //P_LSC_red_b2
	sensor_write(0xb2 , 0x20);  //P_LSC_green_b2
	sensor_write(0xb3 , 0x20);  //P_LSC_blue_b2
	sensor_write(0xb4 , 0x20);  //P_LSC_red_b4
	sensor_write(0xb5 , 0x20);  //P_LSC_green_b4
	sensor_write(0xb6 , 0x20);  //P_LSC_blue_b4
	sensor_write(0xb7 , 0x00);  //P_LSC_compensate_b2
	sensor_write(0xb8 , 0x80);  //P_LSC_row_center , 0x344 , 0x (0x600/2-100)/2=100
	sensor_write(0xb9 , 0x80);  //P_LSC_col_center , 0x544 , 0x (0x800/2-200)/2=100

	////////////////////////////////////////////////////////////////////////
	////////////////////////// capture LSC ///////////////////////////
	////////////////////////////////////////////////////////////////////////
	sensor_write(0xba , 0x13);  //[4]Y_LSC_en [3]lsc_compensate [2]signed_b4 [1:0]pixel array select
	sensor_write(0xbb , 0x20);  //C_LSC_red_b2
	sensor_write(0xbc , 0x20);  //C_LSC_green_b2
	sensor_write(0xbd , 0x20);  //C_LSC_blue_b2
	sensor_write(0xbe , 0x20);  //C_LSC_red_b4
	sensor_write(0xbf , 0x20);  //C_LSC_green_b4
	sensor_write(0xc0 , 0x20);  //C_LSC_blue_b4
	sensor_write(0xc1 , 0x00);  //C_Lsc_compensate_b2
	sensor_write(0xc2 , 0x80);  //C_LSC_row_center , 0x344 , 0x (0x1200/2-344)/2=128
	sensor_write(0xc3 , 0x80);  //C_LSC_col_center , 0x544 , 0x (0x1600/2-544)/2=128

	sensor_write(0xfe , 0x00);  //page0

	////////////////////////////////////////////////////////////////////////
	////////////////////////// analog configure ///////////////////////////
	////////////////////////////////////////////////////////////////////////
	sensor_write(0x29 , 0x00);  //cisctl mode 1
	sensor_write(0x2b , 0x06);  //cisctl mode 3	
	sensor_write(0x32 , 0x0c);  //analog mode 1
	sensor_write(0x33 , 0x0f);  //analog mode 2
	sensor_write(0x34 , 0x00);  //[6:4]da_rsg
	
	sensor_write(0x35 , 0x88);  //Vref_A25
	sensor_write(0x37 , 0x30);  //Drive Current0x16

	/////////////////////////////////////////////////////////////////////
	///////////////////////////ISP Related//////////////////////////////
	/////////////////////////////////////////////////////////////////////
	sensor_write(0x40 , 0xff);  
	sensor_write(0x41 , 0x24);  //[5]skin_detectionenable[2]auto_gray , 0x[1]y_gamma
	sensor_write(0x42 , 0x76);  //[7]auto_sa[6]auto_ee[5]auto_dndd[4]auto_lsc[3]na[2]abs , 0x[1]awb
	sensor_write(0x4b , 0xea);  //[1]AWB_gain_mode , 0x1:atpregain0:atpostgain
	sensor_write(0x4d , 0x03);  //[1]inbf_en
	sensor_write(0x4f , 0x01);  //AEC enable

	////////////////////////////////////////////////////////////////////
	/////////////////////////// BLK  ///////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x63 , 0x77);  //BLK mode 1
	sensor_write(0x66 , 0x00);  //BLK global offset
	sensor_write(0x6d , 0x04);  
	sensor_write(0x6e , 0x18);  //BLK offset submode);offset R
	sensor_write(0x6f , 0x10); 
	sensor_write(0x70 , 0x18); 
	sensor_write(0x71 , 0x10); 
	sensor_write(0x73 , 0x03);  


	////////////////////////////////////////////////////////////////////
	/////////////////////////// DNDD ////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x80 , 0x07);  //[7]dn_inc_or_dec [4]zero_weight_mode[3]share [2]c_weight_adap [1]dn_lsc_mode [0]dn_b
	sensor_write(0x82 , 0x08);  //DN lilat b base

	////////////////////////////////////////////////////////////////////
	/////////////////////////// EEINTP ////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x8a , 0x7c); 
	sensor_write(0x8c , 0x02); 
	sensor_write(0x8e , 0x02); 
	sensor_write(0x8f , 0x48); 

	/////////////////////////////////////////////////////////////////////
	/////////////////////////// CC_t ///////////////////////////////
	/////////////////////////////////////////////////////////////////////
	sensor_write(0xb0 , 0x44); 
	sensor_write(0xb1 , 0xfe); 
	sensor_write(0xb2 , 0x00); 
	sensor_write(0xb3 , 0xf8); 
	sensor_write(0xb4 , 0x48); 
	sensor_write(0xb5 , 0xf8); 
	sensor_write(0xb6 , 0x00); 
	sensor_write(0xb7 , 0x04); 
	sensor_write(0xb8 , 0x00); 

	/////////////////////////////////////////////////////////////////////
	/////////////////////////// GAMMA ///////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	//RGB_GAMMA
	sensor_write(0xbf , 0x0e); 
	sensor_write(0xc0 , 0x1c); 
	sensor_write(0xc1 , 0x34); 
	sensor_write(0xc2 , 0x48); 
	sensor_write(0xc3 , 0x5a); 
	sensor_write(0xc4 , 0x6b); 
	sensor_write(0xc5 , 0x7b); 
	sensor_write(0xc6 , 0x95); 
	sensor_write(0xc7 , 0xab); 
	sensor_write(0xc8 , 0xbf); 
	sensor_write(0xc9 , 0xce); 
	sensor_write(0xca , 0xd9); 
	sensor_write(0xcb , 0xe4); 
	sensor_write(0xcc , 0xec); 
	sensor_write(0xcd , 0xf7); 
	sensor_write(0xce , 0xfd); 
	sensor_write(0xcf , 0xff); 

	/////////////////////////////////////////////////////////////////////
	/////////////////////////// YCP_t  ///////////////////////////////
	/////////////////////////////////////////////////////////////////////
	sensor_write(0xd1 , 0x38);  //saturation
	sensor_write(0xd2 , 0x38);  //saturation
	sensor_write(0xde , 0x21);  //auto_gray

	////////////////////////////////////////////////////////////////////
	/////////////////////////// ASDE ////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x98 , 0x30); 
	sensor_write(0x99 , 0xf0); 
	sensor_write(0x9b , 0x00); 

	sensor_write(0xfe , 0x01);  //page1
	////////////////////////////////////////////////////////////////////
	/////////////////////////// AEC  ////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x10 , 0x45);  //AEC mode 1
	sensor_write(0x11 , 0x32);  //[7]fix target
	sensor_write(0x13 , 0x60); 
	sensor_write(0x17 , 0x00); 
	sensor_write(0x1c , 0x96); 
	sensor_write(0x1e , 0x11); 
	sensor_write(0x21 , 0xc0);  //max_post_gain
	sensor_write(0x22 , 0x40);  //max_pre_gain
	sensor_write(0x2d , 0x06);  //P_N_AEC_exp_level_1[12:8]
	sensor_write(0x2e , 0x00);  //P_N_AEC_exp_level_1[7:0]
	sensor_write(0x1e , 0x32); 
	sensor_write(0x33 , 0x00);  //[6:5]max_exp_level [4:0]min_exp_level

	////////////////////////////////////////////////////////////////////
	///////////////////////////  AWB  ////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x57 , 0x40);  //number limit
	sensor_write(0x5d , 0x44);  //
	sensor_write(0x5c , 0x35);  //show mode);close dark_mode
	sensor_write(0x5e , 0x29);  //close color temp
	sensor_write(0x5f , 0x50); 
	sensor_write(0x60 , 0x50);  
	sensor_write(0x65 , 0xc0); 

	////////////////////////////////////////////////////////////////////
	///////////////////////////  ABS  ////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x80 , 0x82); 
	sensor_write(0x81 , 0x00); 
	sensor_write(0x83 , 0x00);  //ABS Y stretch limit

	sensor_write(0xfe , 0x00); 

	//////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Crop //////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	sensor_write(0x50 , 0x01); //out window
	sensor_write(0x51 , 0x00); 
	sensor_write(0x52 , 0x00); 
	sensor_write(0x53 , 0x00); 
	sensor_write(0x54 , 0x00); 
	sensor_write(0x55 , 0x01); 
	sensor_write(0x56 , 0xe0); 
	sensor_write(0x57 , 0x02); 
	sensor_write(0x58 , 0x80);
	////////////////////////////////////////////////////////////////////
	///////////////////////////  OUT  ////////////////////////////////
	////////////////////////////////////////////////////////////////////
	sensor_write(0x44 , 0xa0);  //YUV sequence
	sensor_write(0x45 , 0x0f);  //output enable
	sensor_write(0x46 , 0x03);  //sync mode

	//-----------Update the registers end---------//
    	sensor_write(0xfe , 0x01); 	//page1
    	sensor_write(0x34 , 0x02);  

    	sensor_write(0xfe , 0x00); 	//page0
    	/*Customer can adjust GAMMA, MIRROR & UPSIDEDOWN here!*/	
    	Gp_GC2015_Gamma_Select(1);

	return 0;	
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);
	
	sensor_write(0x48 , 0x40);
	
	sensor_write(0x02 , 0x01);
	sensor_write(0x2a , 0xca);

	sensor_write(0x55 , 0x01); 
	sensor_write(0x56 , 0xe0); 
	sensor_write(0x57 , 0x02); 
	sensor_write(0x58 , 0x80);
	
	return 0;
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);
	
	sensor_write(0x48 , 0x68);
	
	sensor_write(0x02 , 0x00);	
	sensor_write(0x2a , 0x0a);

	sensor_write(0x55 , 0x03); 
	sensor_write(0x56 , 0xc0); 
	sensor_write(0x57 , 0x06); 
	sensor_write(0x58 , 0x40);
	
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
			nRet = sensor_read(0x42, (unsigned char *)&data);			
	//		nRet = sensor_write(0x7a , 0x5f);
    	//		nRet = sensor_write(0x7b , 0x40);
	//		nRet = sensor_write(0x7c , 0x47);
			nRet = sensor_write(0x42 , data|0x02);
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		}
		else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(0x05 , 0x01);//HB
			nRet = sensor_write(0x06 , 0xc1);
			nRet = sensor_write(0x07 , 0x00);//VB
			nRet = sensor_write(0x08 , 0x40);

			nRet = sensor_write(0xfe , 0x01);
			nRet = sensor_write(0x29 , 0x00);//Anti Step
			nRet = sensor_write(0x2a , 0x80);

			nRet = sensor_write(0x2b , 0x05);//Level_0  10.00fps
			nRet = sensor_write(0x2c , 0x00);
			nRet = sensor_write(0x2d , 0x06);//Level_1   8.33fps
			nRet = sensor_write(0x2e , 0x00);
			nRet = sensor_write(0x2f , 0x08);//Level_2   6.25fps
			nRet = sensor_write(0x30 , 0x00);
			nRet = sensor_write(0x31 , 0x09);//Level_3   5.55fps
			nRet = sensor_write(0x32 , 0x00);
			nRet = sensor_write(0xfe , 0x00);
		
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {

			nRet = sensor_write(0x05 , 0x01);//HB
			nRet = sensor_write(0x06 , 0xd9);
			nRet = sensor_write(0x07 , 0x00);//VB
			nRet = sensor_write(0x08 , 0x20);

			nRet = sensor_write(0xfe , 0x01);
			nRet = sensor_write(0x29 , 0x00);//Anti Step
			nRet = sensor_write(0x2a , 0x68);

			nRet = sensor_write(0x2b , 0x04);//Level_0  10.00fps
			nRet = sensor_write(0x2c , 0xe0);
			nRet = sensor_write(0x2d , 0x05);//Level_1   8.57fps
			nRet = sensor_write(0x2e , 0xb0);
			nRet = sensor_write(0x2f , 0x06);//Level_2   7.05fps
			nRet = sensor_write(0x30 , 0xe8);
			nRet = sensor_write(0x31 , 0x08);//Level_3   6.00fps
			nRet = sensor_write(0x32 , 0x20);
			nRet = sensor_write(0xfe , 0x00); 		
		
		}
		  else
		{
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_read(0x42, (unsigned char *)&data);			
			nRet = sensor_write(0x42 , data&~0x02);
			nRet = sensor_write(0x7a , 0x74);
    			nRet = sensor_write(0x7b , 0x52);
			nRet = sensor_write(0x7c , 0x40);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_read(0x42, (unsigned char *)&data);			
			nRet = sensor_write(0x42 , data&~0x02);
			nRet = sensor_write(0x7a , 0x8c);
    			nRet = sensor_write(0x7b , 0x50);
			nRet = sensor_write(0x7c , 0x40);
		}else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_read(0x42, (unsigned char *)&data);			
			nRet = sensor_write(0x42 , data&~0x02);
			nRet = sensor_write(0x7a , 0x40);
    			nRet = sensor_write(0x7b , 0x42);
			nRet = sensor_write(0x7c , 0x50);
		}else if(ctrl->value == 3) {	// TUNGSTEN
			nRet = sensor_read(0x42, (unsigned char *)&data);			
			nRet = sensor_write(0x42 , data&~0x02);
			nRet = sensor_write(0x7a , 0x40);
    			nRet = sensor_write(0x7b , 0x54);
			nRet = sensor_write(0x7c , 0x70);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		//nRet = sensor_read(0x20, (unsigned char *)&data);	
		if(ctrl->value) {	// NIGH MODE ON
				nRet = sensor_write(0x05 , 0x03);//HB
				nRet = sensor_write(0x06 , 0xe1);
				nRet = sensor_write(0x07 , 0x01);//VB
				nRet = sensor_write(0x08 , 0x94);

				nRet = sensor_write(0xfe , 0x01);
				nRet = sensor_write(0x29 , 0x00);//Anti Step
				nRet = sensor_write(0x2a , 0x51);

				nRet = sensor_write(0x2b , 0x06);//Level_0 4.99fps
				nRet = sensor_write(0x2c , 0x54);
				nRet = sensor_write(0xfe , 0x00);
		}
		else{	// NIGH MODE OFF
				nRet = sensor_write(0x05 , 0x01);//HB
				nRet = sensor_write(0x06 , 0xc1);
				nRet = sensor_write(0x07 , 0x00);//VB
				nRet = sensor_write(0x08 , 0x40);

				nRet = sensor_write(0xfe , 0x01);
				nRet = sensor_write(0x29 , 0x00);//Anti Step
				nRet = sensor_write(0x2a , 0x80);

				nRet = sensor_write(0x2b , 0x05);//Level_0 10fps
				nRet = sensor_write(0x2c , 0x00);
				nRet = sensor_write(0xfe , 0x00);
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
	if(sensor_i2c_open(GC2015_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: gc2015 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_gc2015");
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
MODULE_DESCRIPTION("Generalplus gc2015 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

