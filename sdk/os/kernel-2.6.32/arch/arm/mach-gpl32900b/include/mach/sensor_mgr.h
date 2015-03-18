 /**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Technology Co., Ltd.         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Technology Co., Ltd. All rights are reserved by Generalplus Technology*
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Technology Co., Ltd. reserves the right to modify this    *
 *  software without notice.                                              *
 *                                                                        *
 *  Generalplus Technology Co., Ltd.                                      *
 *  3F, No.8, Dusing Rd., Science-Based Industrial Park,                  *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/
#ifndef _GP_SENSOR_MGR_H
#define _GP_SENSOR_MGR_H

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
//sensor_timing_mode 
#define MODE_CCIR_601				0
#define MODE_CCIR_656				1
#define MODE_CCIR_HREF				2

//sensor_data_mode
#define MODE_DATA_RGB				0
#define MODE_DATA_YUV				1

//sensor_interlace_mode
#define MODE_NONE_INTERLACE			0
#define MODE_INTERLACE				1

//sensor_pclk_mode
#define MODE_POSITIVE_EDGE			0
#define MODE_NEGATIVE_EDGE			1

//sensor_hsync/vsync_mode
#define MODE_ACTIVE_LOW				0
#define MODE_ACTIVE_HIGH			1

//mclk source, mclk_src
#define MODE_MCLK_SRC_320M			0	/* src is 320M, can not output 24M */
#define MODE_MCLK_SRC_96M			1	/* src is 96M, can output 24M */


#ifndef AEGAIN_0EV
#define AEGAIN_0EV						1024
#endif

#ifndef ISO_AUTO
#define ISO_AUTO						0x1881
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
/*typedef struct sensor_calibration_s
{
	const unsigned short *ob;
	const unsigned short *lenscmp;
	const unsigned short (*wb_gain)[2];
	const unsigned int *gamma1;
	const short *color_matrix1;
	const unsigned int *gamma2;
	const short *color_matrix2;

	const short *awb_thr;
	
} sensor_calibration_t;*/

typedef struct sensor_calibration_s
{
	const unsigned short *ob;
	int ob_size;	
	const unsigned short *lenscmp;
	int lenscmp_size;
	const unsigned short (*wb_gain)[2];
	int wbgain_size;
	const unsigned int *gamma1;
	int gamma1_size;
	const short *color_matrix1;
	int matrix1_size;
	const unsigned int *gamma2;
	int gamma2_size;
	const short *color_matrix2;
	int matrix2_size;
	const short *awb_thr;
	int awb_size;
} sensor_calibration_t;



typedef struct {
	// real setting for sensor
	int time, analog_gain, digital_gain;

	// set limitation
	int max_time, min_time;
	int max_analog_gain, min_analog_gain;
	int max_digital_gain, min_digital_gain;


	int daylight_ev_idx;	// daylight time
	int night_ev_idx;		// night time
	int max_ev_idx;

	// ev index
	int sensor_ev_idx;
	int ae_ev_idx;

	int userISO;
} sensor_exposure_t;




typedef struct callbackfunc_s {
//	int (*suspend)(struct v4l2_subdev *sd);
//	int (*resume)(struct v4l2_subdev *sd);
	int (*powerctl)(int ctl);
	int (*standby)(int ctl);
	int (*reset)(int ctl);
	int (*set_port)(char *port);
}callbackfunc_t;

/** @brief A structure of sensor config */
typedef struct sensor_timing_s 
{
	unsigned char *desc;
	unsigned int pixelformat;
	unsigned int bpp;
	unsigned int mclk_src;
	unsigned int mclk;
	unsigned int hpixel;
	unsigned int hoffset;
	unsigned int vline;
	unsigned int voffset;

	sensor_calibration_t *cdsp_calibration;
}sensor_fmt_t;

typedef struct mipi_config_s 
{
	/* mipi clock out set */
	unsigned int mipi_sep_clk_en;	/* 0:use mipi input clk, 1:use separate clk */ 
	unsigned int mipi_sep_clk;		/* separate clock speed */
	unsigned int mipi_sep_clk_src;	/* 0:CEVA pll, 1:USB pll*/
	unsigned int byte_clk_edge;		/* 0:posedge, 1:negedge */
	/* global configure */
	unsigned int low_power_en;		/* 0:disable, 1:enable */
	unsigned int lane_num;			/* 0:1 lane, 1:2 lane */
	/* ecc */
	unsigned int ecc_check_en;		/* 0~3 */
	unsigned int ecc_order;			/* 0:disable, 1:enable */
	unsigned int data_mask_time;	/* unit is ns */
	unsigned int check_hs_seq;		/* 0:disable, 1:enable */
}mipi_config_t;

typedef struct sensor_config_s 
{
	unsigned int sensor_timing_mode;
	unsigned int sensor_data_mode;
	unsigned int sensor_interlace_mode;
	unsigned int sensor_pclk_mode;
	unsigned int sensor_hsync_mode;
	unsigned int sensor_vsync_mode;
	unsigned int sensor_fmt_num;
	sensor_fmt_t *fmt;
	mipi_config_t *mipi_config;
}sensor_config_t;

#define VIDIOC_S_CALIBRATE 	_IOWR('V', BASE_VIDIOC_PRIVATE+7, sensor_calibration_t)

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
#ifdef __KERNEL__
int32_t gp_get_sensorinfo( int sel, int* sdaddr, int* cbaddr, int* port, int* sensor );
int32_t register_sensor(struct v4l2_subdev *sd, int *port, sensor_config_t *config);
int32_t unregister_sensor(struct v4l2_subdev *sd);
#endif

#endif
