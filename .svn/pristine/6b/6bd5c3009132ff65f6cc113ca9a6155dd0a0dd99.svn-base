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
#define OV9712_ID		0x60

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


#define SENSOR_EXTCLK	24000000

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
		.desc		= "preview=1280*800",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 27000000,
		.hpixel = 1280,
		.hoffset = 0x196,
		.vline = 800,
		.voffset = 0x18,
	},
	/* capature mode */
	{
		.desc		= "capture=1280*800",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 27000000,
		.hpixel = 1280,
		.hoffset = 0x196,
		.vline = 800,
		.voffset = 0x18,
	},
	/* record mode */
	{
		.desc		= "record=1280*800",
		.pixelformat = V4L2_PIX_FMT_SBGGR8,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 27000000,
		.hpixel = 1280,
		.hoffset = 0x196,
		.vline = 800,
		.voffset = 0x18,
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
	g_ti2c_handle.pDeviceString = "OV9712";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("OV9712 ti2c request failed\n");
		return -1;
	}
	printk("OV9712 ti2c request ok\n");
	
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

static int ov9712_getPixClk(void)
{
    unsigned int reg, pre_div, pll_m, pll_div, sys_div, pixclk;
    unsigned char val;

    u32 pll_bypass;

    sensor_read(0x5D, &val);
    pll_bypass = val & (1 << 6);

    if (pll_bypass)
    {
    	 sensor_read(0x5C, &val);
        reg = (val >> 5) & 0x03;
        if (reg <= 1)
            pre_div = 1;
        else if (reg == 2)
            pre_div = 2;
        else
            pre_div = 4;

        pixclk = SENSOR_EXTCLK / pre_div;
        printk("OV9712 Pixel Clock %d \n", pixclk);
    }
    else
    {
        sensor_read(0x5C, &val);	 	 
	 printk("[0x5c] = 0x%x\r\n", val);
	 
        //pre-divider
        pre_div = (val >> 5) & 0x03;
        if (pre_div <= 1)
            pre_div = 1;
        else if (pre_div == 2)
            pre_div = 2;
        else
            pre_div = 4;
		
        //pll multiplier
        pll_m = 32 - (val & 0x1F);
		
        //pll divider
        sensor_read(0x5D, &val);
        pll_div = ((val >> 2) & 0x03) + 1;
		
        //system divider
        sensor_read(0x11, &val);
        sys_div = ((val & 0x3F) + 1) * 2;

        pixclk = SENSOR_EXTCLK / pre_div * pll_m / pll_div / sys_div;
        printk("OV9712 Pixel Clock %d , %d, %d, %d, %d\n", pixclk, pre_div, pll_m, pll_div, sys_div);
    }

    return pixclk;
}

static int 
sensor_init(
	struct v4l2_subdev *sd,
	u32 val
)
{
	printk("%s\n", __FUNCTION__);
	
	sensor_write(0x12, 0x80);	//reset,[7]set to factory default values for all register	
	msleep(200);
	/*Core Setting*/
	sensor_write(0x1e, 0x07);   	//reserved                                             
	sensor_write(0x5f, 0x18);    //reserved                                            
	sensor_write(0x69, 0x04);    //reserved                                            
	sensor_write(0x65, 0x2a);    //reserved                                  
	sensor_write(0x68, 0x0a);    //reserved                                  
	sensor_write(0x39, 0x28);    //reserved                                  
	sensor_write(0x4d, 0x90);    //reserved                                  
	sensor_write(0xc1, 0x80);	//added 

	
	/*HYSNC Mode Enable*/
	sensor_write(0xcb, 0x32);    //Hsync mode enable
	sensor_write(0xc3, 0x21);    //0x29;Starts to calculate the parameters for HYSNC mode
	sensor_write(0xc2, 0x88);    //pclk gate disable
		
	#if 1
	sensor_write(0x15, 0x00);	//HREF swap to HYSNC
	sensor_write(0x30, 0x05);    //HYSNC Start Point
	sensor_write(0x31, 0x20);    //HYSNC Stop Point
	sensor_write(0x0c, 0x30);    //??[0]:single_frame_output                 
	sensor_write(0x6d, 0x02);    //reserved                                  
	#endif
	
	/*DSP*/
	sensor_write(0x96, 0x00); 	//DSP options enable_0xf1, disable 0  //en_black&white_pixel,isp 0xC1
	sensor_write(0xbc, 0x68);    //WB_CTRL                                   
	
	/*Resolution and format*/
	//sensor_write(SLAVE_ID,0x12, 0x80);
	sensor_write(0x3b, 0x00);    //reserved                                  
	sensor_write(0x97, 0x00);    //smph_mean, 0x88:test pattern                               
	
	/*HightxWeight*/
	// HStart = HSTART_MSB*8 + HStart_LSBs	
	sensor_write(0x17, 0x25);    //HSTART_MSB,start 0x25 valid frame

	// HSize = HSize_MSB*8 + HSize_LSBs
	sensor_write(0x18, 0xa2);    //HSize_MSB (0xa2)H1296 (H640 0x51)
	sensor_write(0x32, 0x01);    //[5:3]HSize_LSBs,[2:0]HSTART_LSBs (0x07)  	
	
	sensor_write(0x19, 0x02);    //0x0//VSTART_MSBs                               
	sensor_write(0x1a, 0xca);    //VSize_MSBs  V810                             
	sensor_write(0x03, 0x02);    //[3:2]VSize_LSBs,[1:0]VSTART_LSBs                   


	sensor_write(0x04, 0xc0);    //flip on, mirror on
	
	sensor_write(0x98, 0x00);   	//pre_out_hoff[7:0]		/*sensor_write-(SLAVE_ID,0x98, 0x00)*/         
	sensor_write(0x99, 0x00);   	//pre_out_voff[7:0]		/*sensor_write-(SLAVE_ID,0x99, 0x00)*/         
	sensor_write(0x9a, 0x00);   	//pre_out_voff[9:8],pre_out_hoff[10:8]/*sensor_write-(SLAVE_ID,0x9a, 0x00)*/   
	sensor_write(0x57, 0x03);    //DSP[4:2] output_Hszie_LSBs,[1:0]output_Vsize_LSBs           
	sensor_write(0x58, 0xc8);   	//DSP output_Vsize_MSBs	/*sensor_write-(SLAVE_ID,0x58, 0xc8)*/ 0xb40=0x2d0=720
	sensor_write(0x59, 0xa0);   	//DSP output_Hsize_MSBs	/*sensor_write-(SLAVE_ID,0x59, 0xa0)*/ 
	sensor_write(0x4c, 0x13);    //reserved                                            
	sensor_write(0x4b, 0x36);    //reserved                                            
	sensor_write(0x3d, 0x3c);    //Red_counter_End_Point_LSBs                          
	sensor_write(0x3e, 0x03);    //Red_counter_End_Point_MSBs                          
	
	/*YAVG_CTRL*/
	sensor_write(0xc1, 0x80);    //YAVG_CTRL:yacg_win_man en                                 
	sensor_write(0xbd, 0xa0);   	//yavg_winh	/*sensor_write-(SLAVE_ID,0xbd, 0xa0),*/
	sensor_write(0xbe, 0xc8);   	//yavg_winv	/*sensor_write-(SLAVE_ID,0xbe, 0xc8),*/
	/*16-Zone_Y_Avarage_Select*/
	sensor_write(0x4e, 0x55);    //Y Avg Select:zone1~4 = weight x 1                 
	sensor_write(0x4f, 0x55);	//Y Avg Select:zone5~8 = weight x 1            
	sensor_write(0x50, 0x55);	//Y Avg Select:zone9~12 = weight x 1                  
	sensor_write(0x51, 0x55);	//Y Avg Select:zone13~16 = weight x 1
	sensor_write(0x2c, 0x60);	//Reserved
	sensor_write(0x23, 0x10);    //Reserved                                                       
	
	#if 1		//Manual Exposure time & Gain
	/*AEC/AGC/Banding function*/
	sensor_write(0x13, 0x0);;	//OFF Fast AEC adj,Banding ,Auto AGC, Auto Exposure

	// Exposure_Time
	sensor_write(0x10, 0xb0);		//Exposure_Time_LSB
	//sensor_write(0x10, 0x40);		//Exposure_Time_LSB
	sensor_write(0x16, 0x03);		//Exposure_Time_MSB

	// AGC gain, Bit[6:0]: analog gain, Bit[7]: digital gain
	sensor_write(0x00, 0x00);		//AGC Gain CTRL   
	
	sensor_write(0x4a, 0x00);	//Banding step MSBs[1:0]
	sensor_write(0x49, 0xce);	//Banding step LSBs[1:0]
	sensor_write(0x22, 0x03);	//Max smooth banding steps
	//sensor_write(SLAVE_ID,0x09, 0x00);
	//sensor_write(0x0d, 0x01);	//AEC_ctrl,default:0x01,
	
	sensor_write(0x0d, 0x03);	//AEC_ctrl, Manual mode
	
	sensor_write(0x0e, 0x40);	//AEC/AGC_based mode,default:0x40,
	//sensor_write(0x14, 0x10);//40);	//AGC-Gainx8(0x40)
	
	sensor_write(0x14, 0x50); //AGC-Gainx8

	// Tp level Exposure Control
	sensor_write(0x1f, 0x0);		//LAEC[7:0]:less_lan_1_row_of_exposure_time
	sensor_write(0x3c, 0x0);		//LAEC[15:8]
	
	/*AEC/AGC operation*/				//Maximum_Exposue_Lines:[(0x1A+0x03[3:2])-2Lines]
	sensor_write(0x24, 0x70);    //Luminance Signal High Range for AEC/AGC                       
	sensor_write(0x25, 0x40);    //Luminance Signal Low  Range for AEC/AGC                       
	sensor_write(0x26, 0xa1);    //Fast mode Thresholds,([3:0]*16)>YAVG(0x2f)>([7:4]*16)
	#endif
	
	#if 0	//Auto Exporsure & Gain
	/*AEC/AGC/Banding function*/
	sensor_write(0x13, 0x0);//a5);	//Fast AEC adj,Banding ON,Auto AGC, Auto Exposure
	sensor_write(0x1f, 0x0);		//LAEC[7:0]:less_lan_1_row_of_exposure_time
	sensor_write(0x3c, 0x0);		//LAEC[15:8]
	sensor_write(0x00, 0x80);		//AGC Gain CTRL    	
	sensor_write(0x14, 0x10);//40);	//AGC-Gainx8(0x40)
	
	sensor_write(0x4a, 0x00);	//Banding step MSBs[1:0]
	sensor_write(0x49, 0xce);	//Banding step LSBs[1:0]
	sensor_write(0x22, 0x03);	//Max smooth banding steps
	//sensor_write(SLAVE_ID,0x09, 0x00);
	
	sensor_write(0x10, 0x60);		//Exposure_Time_LSB
	sensor_write(0x16, 0x02);		//Exposure_Time_MSB
	sensor_write(0x0d, 0x01);	//AEC_ctrl,default:0x01,
	sensor_write(0x0e, 0x40);	//AEC/AGC_based mode,default:0x40,
	
	/*AEC/AGC operation*/				//Maximum_Exposue_Lines:[(0x1A+0x03[3:2])-2Lines]
	sensor_write(0x24, 0x70);    //Luminance Signal High Range for AEC/AGC                       
	sensor_write(0x25, 0x40);    //Luminance Signal Low  Range for AEC/AGC                       
	sensor_write(0x26, 0xa1);    //Fast mode Thresholds,([3:0]*16)>YAVG(0x2f)>([7:4]*16)
	#endif
	
	sensor_write(0x2f, 0x00);//55);    //Luminance_Average_Value(YAVG)

	/*Histogram-based*/
	sensor_write(0x72, 0x60);    //
	sensor_write(0x73, 0x49);    //
	sensor_write(0x74, 0xe0);    //
	sensor_write(0x75, 0xe0);    //
	sensor_write(0x76, 0xc0);    //
	sensor_write(0x77, 0xc0);    //
	sensor_write(0x78, 0xff);    //
	sensor_write(0x79, 0xff);    //
	
	sensor_write(0x09, 0x08);   	//[4]:Chip sleep mode, [3]:Reset sensor timing when mode changes  
	sensor_write(0x55, 0xfc);    //disable Y0,Y1
	sensor_write(0x56, 0x1f);    //Enable HREF & enable HSync	


	// set PLL for PCLK by Comi	
/*	{
		int pclk;
		unsigned char val;
		
		sensor_write(0x5c, 0x59);
		sensor_read(0x5c, &val);
		printk("[0x5c] = 0x%x\r\n", val);
		
		sensor_write(0x5d, 0x00);
		sensor_write(0x11, 0x00);	
		//pclk = ov9712_getPixClk();
		//printk("pclk = %d\r\n", pclk);
	}*/
	
//	msleep(10);
	return 0;	// normal:10
}

static int 
sensor_preview(void)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static int 
sensor_capture(void)
{
	printk("%s\n", __FUNCTION__);
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
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x5a, 0x56); //for AWB can adjust back
			nRet = sensor_write(0x5b, 0x40);
			nRet = sensor_write(0x5c, 0x4a);			
			nRet = sensor_write(0x22, data|0x02);	 // Enable AWB
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		}
		else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(0x01  ,0x32); 	
			nRet = sensor_write(0x02  ,0x70); 
			nRet = sensor_write(0x0f  ,0x01);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x78);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x02);   //exp level 0  12.5fps
			nRet = sensor_write(0xe5  ,0x58); 
		
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_write(0x01  ,0x6a); 	
			nRet = sensor_write(0x02  ,0x89); 
			nRet = sensor_write(0x0f  ,0x00);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x7d);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x02);   //exp level 0  12.00fps
			nRet = sensor_write(0xe5  ,0x71); 
		
		}
		  else
		{
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02);   
			nRet = sensor_write(0x5a, 0x74);	// 50 45 40
			nRet = sensor_write(0x5b, 0x52);
			nRet = sensor_write(0x5c, 0x40);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02);	// Disable AWB 
			nRet = sensor_write(0x5a, 0x8c);	// WB_manual_gain // 5a 42 40
			nRet = sensor_write(0x5b, 0x50);
			nRet = sensor_write(0x5c, 0x40);
		}else if(ctrl->value == 2) {	// FLUORESCENCE
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02);   
			nRet = sensor_write(0x5a, 0x40);
			nRet = sensor_write(0x5b, 0x42);
			nRet = sensor_write(0x5c, 0x50);
		}else if(ctrl->value == 3) {	// INCANDESCENCE
			nRet = sensor_read(0x22, (unsigned char *)&data);			
			nRet = sensor_write(0x22, data&~0x02); 
			nRet = sensor_write(0x5a, 0x48);
			nRet = sensor_write(0x5b, 0x40);
			nRet = sensor_write(0x5c, 0x5c);
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		nRet = sensor_read(0x20, (unsigned char *)&data);	
		if(ctrl->value) {	// NIGH MODE ON
			nRet = sensor_write(0x01  ,0x32); 	
			nRet = sensor_write(0x02  ,0xc8); 
			nRet = sensor_write(0x0f  ,0x21);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x78);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x04);   //exp level 0  10fps
			nRet = sensor_write(0xe5  ,0xb0); 	

			nRet = sensor_write(0xec, 0x00);
			nRet = sensor_write(0x20, data&0x5f);   // close cc
			nRet = sensor_write(0x3c, 0x08);
			nRet = sensor_write(0x3d, 0x08);
			nRet = sensor_write(0x3e, 0x08);
			nRet = sensor_write(0x3f, 0x08);
		}
		else{	// NIGH MODE OFF
			nRet = sensor_write(0x01  ,0x32); 	
			nRet = sensor_write(0x02  ,0x70); 
			nRet = sensor_write(0x0f  ,0x01);

			nRet = sensor_write(0xe2  ,0x00); 	//anti-flicker step [11:8]
			nRet = sensor_write(0xe3  ,0x78);   //anti-flicker step [7:0]
				
			nRet = sensor_write(0xe4  ,0x02);   //exp level 0  20fps
			nRet = sensor_write(0xe5  ,0x58); 

			nRet = sensor_write(0xec, 0x00);
			nRet = sensor_write(0x20, data|0x20);
			nRet = sensor_write(0x3c, 0x02);
			nRet = sensor_write(0x3d, 0x02);
			nRet = sensor_write(0x3e, 0x02);
			nRet = sensor_write(0x3f, 0x02);
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
	if(sensor_i2c_open(OV9712_ID, 100) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: ov9712 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_ov9712");
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
MODULE_DESCRIPTION("Generalplus ov9712 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

