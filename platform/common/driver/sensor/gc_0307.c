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
#define GC0307_ID		0x42

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
		.mclk = 12000000,
		.hpixel = 640,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
	/* capature mode */
	{
		.desc		= "capture=640*480",
		.pixelformat = V4L2_PIX_FMT_VYUY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 12000000,
		.hpixel = 640,
		.hoffset = 0,
		.vline = 480,
		.voffset = 0,
	},
	/* record mode */
	{
		.desc		= "record=640*480",
		.pixelformat = V4L2_PIX_FMT_VYUY,
		.bpp 		= 2,
		.mclk_src = MODE_MCLK_SRC_96M,
		.mclk = 12000000,
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
	g_ti2c_handle.pDeviceString = "GC0307";		
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;		
	g_ti2c_handle.slaveAddr = slave_id;		
	g_ti2c_handle.clockRate = scl_speed;
	if (gp_ti2c_bus_request(&g_ti2c_handle) != 0) {
		printk("GC0307 ti2c request failed\n");
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
	/*
		char data;
		sensor_read(0x00,&data);
			printk("read -> data = 0x%x\n",data);
	*/		
	sensor_write(0x43  ,0x00); 
	sensor_write(0x44  ,0xa0); 
	
	//========= close some functions
	// open them after configure their parmameters
	sensor_write(0x40  ,0x10);  
	sensor_write(0x41  ,0x00);  			
	sensor_write(0x42  ,0x10); 					  	
	sensor_write(0x47  ,0x00);  //mode1);				  	
	sensor_write(0x48  ,0xc3);  //mode2); 	
	sensor_write(0x49  ,0x00);  //dither_mode 		
	sensor_write(0x4a  ,0x00);  //clock_gating_en
	sensor_write(0x4b  ,0x00);  //mode_reg3
	sensor_write(0x4E  ,0x23);  //sync mode
	sensor_write(0x4F  ,0x01);  //AWB); AEC); every N frame	
		
	//========= frame timing
	sensor_write(0x01  ,0x6a); //HB    //12M  mclk
	sensor_write(0x02  ,0x25); //VB  0x70
	//sensor_write(0x03  ,0x01); 
	//sensor_write(0x04  ,0x50); 
	sensor_write(0x1C  ,0x00); //Vs_st
	sensor_write(0x1D  ,0x00); //Vs_et
	sensor_write(0x10  ,0x00); //high 4 bits of VB); HB
	sensor_write(0x11  ,0x05); //row_tail);  AD_pipe_number
			
	//========= windowing
	sensor_write(0x05  ,0x00); //row_start
	sensor_write(0x06  ,0x00);
	sensor_write(0x07  ,0x00); //col start
	sensor_write(0x08  ,0x00); 
	sensor_write(0x09  ,0x01); //win height
	sensor_write(0x0A  ,0xE8);
	sensor_write(0x0B  ,0x02); //win width); pixel array only 640
	sensor_write(0x0C  ,0x80);
		
	//========= analog
	sensor_write(0x0D  ,0x22); //  0522 rsh_width
						  
	sensor_write(0x0E  ,0x02); //CISCTL mode2);  
	sensor_write(0x0F  ,0xb2); //CISCTL mode1
				  
	sensor_write(0x12  ,0x70); //7 hrst); 6_4 darsg);
	sensor_write(0x13  ,0x00); //7 CISCTL_restart); 0 apwd
	sensor_write(0x14  ,0x00); //NA
	sensor_write(0x15  ,0xba); //7_4 vref
	sensor_write(0x16  ,0x13); //5to4 _coln_r);  __1to0__da18
	sensor_write(0x17  ,0x52); //opa_r); ref_r); sRef_r
	sensor_write(0x18  ,0xc0); //analog_mode); best case for left band.
		
	sensor_write(0x1E  ,0x0d); //tsp_width 		   
	sensor_write(0x1F  ,0x32); //sh_delay
		
	//========= offset
	sensor_write(0x47  ,0x00);  //7__test_image); __6__fixed_pga); __5__auto_DN); __4__CbCr_fix); 
					//__3to2__dark_sequence); __1__allow_pclk_vcync); __0__LSC_test_image
	sensor_write(0x19  ,0x06);  //pga_o			 
	sensor_write(0x1a  ,0x06);  //pga_e			 
		
	sensor_write(0x31  ,0x00);  //4	//pga_oFFset );	 high 8bits of 11bits
	sensor_write(0x3B  ,0x00);  //global_oFFset); low 8bits of 11bits
		
	sensor_write(0x59  ,0x0f);  //offset_mode 	
	sensor_write(0x58  ,0x88);  //DARK_VALUE_RATIO_G);  DARK_VALUE_RATIO_RB
	sensor_write(0x57  ,0x08);  //DARK_CURRENT_RATE
	sensor_write(0x56  ,0x77);  //PGA_OFFSET_EVEN_RATIO); PGA_OFFSET_ODD_RATIO
		
	//========= blk
	sensor_write(0x35  ,0xd8);  //blk_mode

	sensor_write(0x36  ,0x40);  
		
	sensor_write(0x3C  ,0x00); 
	sensor_write(0x3D  ,0x00); 
	sensor_write(0x3E  ,0x00); 
	sensor_write(0x3F  ,0x00); 
		
	sensor_write(0xb5  ,0x70); 
	sensor_write(0xb6  ,0x40); 
	sensor_write(0xb7  ,0x00); 
	sensor_write(0xb8  ,0x38); 
	sensor_write(0xb9  ,0xc3); 		  
	sensor_write(0xba  ,0x0f); 
	        
	sensor_write(0x7e  ,0x35); 
	sensor_write(0x7f  ,0x86); 
	    
	sensor_write(0x5c  ,0x68); //78
	sensor_write(0x5d  ,0x78); //88	    
	
	//========= manual_gain 
	sensor_write(0x61  ,0x80); //manual_gain_g1	
	sensor_write(0x63  ,0x80); //manual_gain_r
	sensor_write(0x65  ,0x98); //manual_gai_b); sensor_write(0xa0=1.25); sensor_write(0x98=1.1875
	sensor_write(0x67  ,0x80); //manual_gain_g2
	sensor_write(0x68  ,0x18); // global_manual_gain	 2.4bits
		
	//=========CC _R
	sensor_write(0x69  ,0x58);  //54
	sensor_write(0x6A  ,0xf6);  //ff
	sensor_write(0x6B  ,0xfb);  //fe
	sensor_write(0x6C  ,0xf4);  //ff
	sensor_write(0x6D  ,0x5a);  //5f
	sensor_write(0x6E  ,0xe6);  //e1

	sensor_write(0x6f  ,0x00); 	
		
	//=========lsc							  
	sensor_write(0x70  ,0x14); 
	sensor_write(0x71  ,0x1c); 
	sensor_write(0x72  ,0x20); 
		
	sensor_write(0x73  ,0x10); 	
	sensor_write(0x74  ,0x3c); 
	sensor_write(0x75  ,0x52); 
		
	//=========dn																			 
	sensor_write(0x7d  ,0x2f);  //dn_mode   	
	sensor_write(0x80  ,0x0c); //when auto_dn); check 7e);7f
	sensor_write(0x81  ,0x0c);
	sensor_write(0x82  ,0x44);
																							
	//dd																		   
	sensor_write(0x83  ,0x18);  //DD_TH1 					  
	sensor_write(0x84  ,0x18);  //DD_TH2 					  
	sensor_write(0x85  ,0x04);  //DD_TH3 																							  
	sensor_write(0x87  ,0x34);  //32 b DNDD_low_range X16);  DNDD_low_range_C_weight_center					
	           
        //=========intp-ee                                                                         
	sensor_write(0x88  ,0x04);                                                       
	sensor_write(0x89  ,0x01);                                          
	sensor_write(0x8a  ,0x50);//60                                           
	sensor_write(0x8b  ,0x50);//60                                           
	sensor_write(0x8c  ,0x07);                                                                  
	                                                                                      
	sensor_write(0x50  ,0x0c);                                
	sensor_write(0x5f  ,0x3c);                                                                                     
	                                                                                     
	sensor_write(0x8e  ,0x02);                                                              
	sensor_write(0x86  ,0x02);  																  
																						
	sensor_write(0x51  ,0x20);  																
	sensor_write(0x52  ,0x08);  
	sensor_write(0x53  ,0x00); 
		
		
	//========= YCP 
	//contrast_center																			  
	sensor_write(0x77  ,0x80); //contrast_center 																  
	sensor_write(0x78  ,0x00); //fixed_Cb																		  
	sensor_write(0x79  ,0x00); //fixed_Cr																		  
	sensor_write(0x7a  ,0x20); //luma_offset 																																							
	sensor_write(0x7b  ,0x40); //hue_cos 																		  
	sensor_write(0x7c  ,0x00); //hue_sin 																		  
																								 
	//saturation																				  
	sensor_write(0xa0  ,0x40); //40global_saturation
	sensor_write(0xa1  ,0x40); //luma_contrast																	  
	sensor_write(0xa2  ,0x34); //saturation_Cb																	  
	sensor_write(0xa3  ,0x34); //saturation_Cr
																					
	sensor_write(0xa4  ,0xc8); 																  
	sensor_write(0xa5  ,0x02); 
	sensor_write(0xa6  ,0x28); 																			  
	sensor_write(0xa7  ,0x02); 
		
	//skin																								  
	sensor_write(0xa8  ,0xee); 															  
	sensor_write(0xa9  ,0x12); 															  
	sensor_write(0xaa  ,0x01); 														  
	sensor_write(0xab  ,0x20); 													  
	sensor_write(0xac  ,0xf0); 														  
	sensor_write(0xad  ,0x10); 															  
			
	//========= ABS
	sensor_write(0xae  ,0x18); 
	sensor_write(0xaf  ,0x74); 
	sensor_write(0xb0  ,0xe0); 	  
	sensor_write(0xb1  ,0x20); 
	sensor_write(0xb2  ,0x6c); 
	sensor_write(0xb3  ,0x40); 
	sensor_write(0xb4  ,0x04); 
			
	//========= AWB 
	sensor_write(0xbb  ,0x42); 
	sensor_write(0xbc  ,0x60); 
	sensor_write(0xbd  ,0x50); 
	sensor_write(0xbe  ,0x50); 
		
	sensor_write(0xbf  ,0x0c); 
	sensor_write(0xc0  ,0x06); 
	sensor_write(0xc1  ,0x70); 
	sensor_write(0xc2  ,0xf1);  //f4
	sensor_write(0xc3  ,0x40); 
	sensor_write(0xc4  ,0x20); //18
	sensor_write(0xc5  ,0x33); //33
	sensor_write(0xc6  ,0x1d);   

	sensor_write(0xca  ,0x70);// 70  
	sensor_write(0xcb  ,0x70); // 70
	sensor_write(0xcc  ,0x78); // 78
		
	sensor_write(0xcd  ,0x80); //R_ratio 									 
	sensor_write(0xce  ,0x80); //G_ratio  ); cold_white white 								   
	sensor_write(0xcf  ,0x80); //B_ratio  	
		
	//=========  aecT  
	sensor_write(0x20  ,0x02); 
	sensor_write(0x21  ,0xc0); 
	sensor_write(0x22  ,0x60);    
	sensor_write(0x23  ,0x88); 
	sensor_write(0x24  ,0x96); 
	sensor_write(0x25  ,0x30); 
	sensor_write(0x26  ,0xd0); 
	sensor_write(0x27  ,0x00); 

	sensor_write(0x28  ,0x02); //AEC_exp_level_1bit11to8   
	sensor_write(0x29  ,0x0D); //AEC_exp_level_1bit7to0	  
	sensor_write(0x2a  ,0x02); //AEC_exp_level_2bit11to8   
	sensor_write(0x2b  ,0x0D); //AEC_exp_level_2bit7to0			 
	sensor_write(0x2c  ,0x02); //AEC_exp_level_3bit11to8   659 - 8FPS);  8ca - 6FPS  //	 
	sensor_write(0x2d  ,0x0D); //AEC_exp_level_3bit7to0			 
	sensor_write(0x2e  ,0x02); //AEC_exp_level_4bit11to8   4FPS 
	sensor_write(0x2f  ,0xEE); //AEC_exp_level_4bit7to0	  
		
	sensor_write(0x30  ,0x20); 						  
	sensor_write(0x31  ,0x00); 					   
	sensor_write(0x32  ,0x1c); 
	sensor_write(0x33  ,0x90); 			  
	sensor_write(0x34  ,0x10);	
		
	sensor_write(0xd0  ,0x34); 
				   
	sensor_write(0xd1  ,0x60); //AEC_target_Y						   
	sensor_write(0xd2  ,0xf2); 	  
	sensor_write(0xd4  ,0x96); 
	sensor_write(0xd5  ,0x10); 
	sensor_write(0xd6  ,0x4B); //antiflicker_step  0x96						   
	sensor_write(0xd7  ,0x10); //AEC_exp_time_min 			   
	sensor_write(0xd8  ,0x02); 
				   
	sensor_write(0xdd  ,0x12); 
		  															
	//========= measure window										
	sensor_write(0xe0  ,0x03); 						 
	sensor_write(0xe1  ,0x02); 							 
	sensor_write(0xe2  ,0x27); 								 
	sensor_write(0xe3  ,0x1e); 				 
	sensor_write(0xe8  ,0x3b); 					 
	sensor_write(0xe9  ,0x6e); 						 
	sensor_write(0xea  ,0x2c); 					 
	sensor_write(0xeb  ,0x50); 					 
	sensor_write(0xec  ,0x73); 		 
	
	//========= close_frame													
	sensor_write(0xed  ,0x00); //close_frame_num1 );can be use to reduce FPS				 
	sensor_write(0xee  ,0x00); //close_frame_num2  
	sensor_write(0xef  ,0x00); //close_frame_num
		
	// page1
	sensor_write(0xf0  ,0x01); //select page1 
		
	sensor_write(0x00  ,0x20); 							  
	sensor_write(0x01  ,0x20); 							  
	sensor_write(0x02  ,0x20); 									
	sensor_write(0x03  ,0x20); 							
	sensor_write(0x04  ,0x78); 
	sensor_write(0x05  ,0x78); 					 
	sensor_write(0x06  ,0x78); 								  
	sensor_write(0x07  ,0x78); 									 
				
	sensor_write(0x10  ,0x04); 						  
	sensor_write(0x11  ,0x04);							  
	sensor_write(0x12  ,0x04); 						  
	sensor_write(0x13  ,0x04); 							  
	sensor_write(0x14  ,0x01); 							  
	sensor_write(0x15  ,0x01); 							  
	sensor_write(0x16  ,0x01); 						 
	sensor_write(0x17  ,0x01); 						 
			  
														 
	sensor_write(0x20  ,0x00); 					  
	sensor_write(0x21  ,0x00); 					  
	sensor_write(0x22  ,0x00); 						  
	sensor_write(0x23  ,0x00); 						  
	sensor_write(0x24  ,0x00); 					  
	sensor_write(0x25  ,0x00); 						  
	sensor_write(0x26  ,0x00); 					  
	sensor_write(0x27  ,0x00);  						  
		
	sensor_write(0x40  ,0x11); 
		
	//=============================lscP 
	sensor_write(0x45  ,0x06); 	 
	sensor_write(0x46  ,0x06); 			 
	sensor_write(0x47  ,0x05); 
		
	sensor_write(0x48  ,0x04); 	
	sensor_write(0x49  ,0x03); 		 
	sensor_write(0x4a  ,0x03); 
		

	sensor_write(0x62  ,0xd8); 
	sensor_write(0x63  ,0x24); 
	sensor_write(0x64  ,0x24); 
	sensor_write(0x65  ,0x24); 
	sensor_write(0x66  ,0xd8); 
	sensor_write(0x67  ,0x24);
		
	sensor_write(0x5a  ,0x00); 
	sensor_write(0x5b  ,0x00); 
	sensor_write(0x5c  ,0x00); 
	sensor_write(0x5d  ,0x00); 
	sensor_write(0x5e  ,0x00); 
	sensor_write(0x5f  ,0x00); 
		
		
	//============================= ccP 
		
	sensor_write(0x69  ,0x03); //cc_mode
			  
	//CC_G
	sensor_write(0x70  ,0x5d); 
	sensor_write(0x71  ,0xed); 
	sensor_write(0x72  ,0xff); 
	sensor_write(0x73  ,0xe5); 
	sensor_write(0x74  ,0x5f); 
	sensor_write(0x75  ,0xe6); 
		
	//CC_B
	sensor_write(0x76  ,0x41); 
	sensor_write(0x77  ,0xef); 
	sensor_write(0x78  ,0xff); 
	sensor_write(0x79  ,0xff); 
	sensor_write(0x7a  ,0x5f); 
	sensor_write(0x7b  ,0xfa); 	 
				
	//============================= AGP
		
	sensor_write(0x7e  ,0x00);  
	sensor_write(0x7f  ,0x00);  
	sensor_write(0x80  ,0xc8);  
	sensor_write(0x81  ,0x06);  
	sensor_write(0x82  ,0x08);  
		
	sensor_write(0x83  ,0x23);  
	sensor_write(0x84  ,0x38);  
	sensor_write(0x85  ,0x4F);  
	sensor_write(0x86  ,0x61);  
	sensor_write(0x87  ,0x72);  
	sensor_write(0x88  ,0x80);  
	sensor_write(0x89  ,0x8D);  
	sensor_write(0x8a  ,0xA2);  
	sensor_write(0x8b  ,0xB2);  
	sensor_write(0x8c  ,0xC0);  
	sensor_write(0x8d  ,0xCA);  
	sensor_write(0x8e  ,0xD3);  
	sensor_write(0x8f  ,0xDB);  
	sensor_write(0x90  ,0xE2);  
	sensor_write(0x91  ,0xED);  
	sensor_write(0x92  ,0xF6);  
	sensor_write(0x93  ,0xFD);  
		
	//about gamma1 is hex r oct
	sensor_write(0x94  ,0x04);  
	sensor_write(0x95  ,0x0E);  
	sensor_write(0x96  ,0x1B);  
	sensor_write(0x97  ,0x28);  
	sensor_write(0x98  ,0x35);  
	sensor_write(0x99  ,0x41);  
	sensor_write(0x9a  ,0x4E);  
	sensor_write(0x9b  ,0x67);  
	sensor_write(0x9c  ,0x7E);  
	sensor_write(0x9d  ,0x94);  
	sensor_write(0x9e  ,0xA7);  
	sensor_write(0x9f  ,0xBA);  
	sensor_write(0xa0  ,0xC8);  
	sensor_write(0xa1  ,0xD4);  
	sensor_write(0xa2  ,0xE7);  
	sensor_write(0xa3  ,0xF4);  
	sensor_write(0xa4  ,0xFA); 
	        
        //========= open functions  
	sensor_write(0xf0  ,0x00); //set back to page0                                                                          
	sensor_write(0x0F  ,0xb2);
	sensor_write(0x45  ,0x27);
	sensor_write(0x40  ,0x7e); 
	sensor_write(0x41  ,0x2F);
	sensor_write(0x47  ,0x2c);    
	    
    //=========open output
	sensor_write(0x43  ,0x40);
	sensor_write(0x44  ,0xE0);
	
	return 0;
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
			nRet = sensor_read(0x41, (unsigned char *)&data);			
			nRet = sensor_write(0xc7,0x4c); //for AWB can adjust back
			nRet = sensor_write(0xc8,0x40);
			nRet = sensor_write(0xc9,0x4a);
			nRet = sensor_write(0x41,data|0x04);// Enable AWB
		}else{	// Disable Auto AWB
			
		}
		break;

	case V4L2_CID_POWER_LINE_FREQUENCY:
		printk("PL = %d\n", ctrl->value);
		if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_DISABLED) {
			
		}
		else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_50HZ) {
			nRet = sensor_write(  0x01  ,0x6a); 
			nRet = sensor_write(  0x02  ,0x25); 
			nRet = sensor_write(  0x10  ,0x00);
			nRet = sensor_write(  0xd6  ,0x4b); 
			nRet = sensor_write(  0x28  ,0x02); 
			nRet = sensor_write(  0x29  ,0x0d); 
			nRet = sensor_write(  0x2a  ,0x02); 
			nRet = sensor_write(  0x2b  ,0x0d); 
			nRet = sensor_write(  0x2c  ,0x02); 
			nRet = sensor_write(  0x2d  ,0x0d); 
			nRet = sensor_write(  0x2e  ,0x02); 
			nRet = sensor_write(  0x2f  ,0xee); 			
		
		}else if(ctrl->value == V4L2_CID_POWER_LINE_FREQUENCY_60HZ) {
			nRet = sensor_write(	0x01  ,0x32); 	
			nRet = sensor_write(	0x02  ,0x3e); 
			nRet = sensor_write(	0x10  ,0x01); 	
			nRet = sensor_write(	0xd6  ,0x32); 
			nRet = sensor_write(	0x28  ,0x02); 
			nRet = sensor_write(	0x29  ,0x26); 
			nRet = sensor_write(	0x2a  ,0x02); 
			nRet = sensor_write(	0x2b  ,0x26); 
			nRet = sensor_write(	0x2c  ,0x02); 
			nRet = sensor_write(	0x2d  ,0x26); 
			nRet = sensor_write(	0x2e  ,0x04); 
			nRet = sensor_write(	0x2f  ,0x4c);
		
		}
		  else
		{
			return -EINVAL;
		}
		break;
		
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		printk("WB = %d\n", ctrl->value);
		if(ctrl->value == 0) {	// SUNSHINE 
			nRet = sensor_read(0x41, (unsigned char *)&data);			
			nRet = sensor_write(0x41,data&~0x04);   // Enable AWB 
			nRet = sensor_write(0xc7,0x50);
			nRet = sensor_write(0xc8,0x45);
			nRet = sensor_write(0xc9,0x40);
		}else if(ctrl->value == 1) {	// CLOUDY
			nRet = sensor_read(0x41, (unsigned char *)&data);			
			nRet = sensor_write(0x41,data&~0x04);   // Enable AWB 
			nRet = sensor_write(0xc7,0x5a); //WB_manual_gain
			nRet = sensor_write(0xc8,0x42);
			nRet = sensor_write(0xc9,0x40);
		}else if(ctrl->value == 2) {	// ri guang deng
			nRet = sensor_read(0x41, (unsigned char *)&data);			
			nRet = sensor_write(0x41,data&~0x04);   // Enable AWB 
			nRet = sensor_write(0xc7,0x40);
			nRet = sensor_write(0xc8,0x42);
			nRet = sensor_write(0xc9,0x50);
		}else if(ctrl->value == 3) {	// wu si deng
			nRet = sensor_read(0x41, (unsigned char *)&data);			
			nRet = sensor_write(0x41,data&~0x04);   // Enable AWB 
			nRet = sensor_write(0xc7,0x40);
			nRet = sensor_write(0xc8,0x54);
			nRet = sensor_write(0xc9,0x70);			
		}
		break; 
 
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		printk("NightMode = %d\n", ctrl->value);
		nRet = sensor_read(0x41, (unsigned char *)&data);	
		if(ctrl->value) {	// NIGH MODE ON
			nRet = sensor_write(	0xdd  ,0x32);	
			nRet = sensor_write(	0x41  ,data & ~0x20);
			nRet = sensor_write(	0xb0  ,0x10);
			nRet = sensor_write(	0x21  ,0xf0);
		}
		else{	// NIGH MODE OFF
			nRet = sensor_write(	0xdd  ,0x22);		 
			nRet = sensor_write(	0x41  ,data | 0x20);
			nRet = sensor_write(	0x21  ,0xc0);
			nRet = sensor_write(	0xb0  ,0xe0);
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
	if(sensor_i2c_open(GC0307_ID, 50) < 0) {
		printk(KERN_WARNING "i2cReqFail\n");
		return -1;
	}

	printk(KERN_WARNING "ModuleInit: gc0307 \n");
	g_sensor_dev.fmt = &g_fmt_table[0];
	v4l2_subdev_init(&(g_sensor_dev.sd), &sensor_ops);
	strcpy(g_sensor_dev.sd.name, "sensor_gc0307");
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
MODULE_DESCRIPTION("Generalplus gc0307 Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

