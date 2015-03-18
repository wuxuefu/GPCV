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
/**
 * @file gp_panel.h
 * @brief Panel header file
 * @author Anson Chuang
 */

#ifndef _GP_PANEL_H_
#define _GP_PANEL_H_

#include <mach/typedef.h>
#include <mach/gp_display.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
enum {
	/* Panel Output Format */
	PANEL_FMT_RGB = 0,
	PANEL_FMT_YCBCR = 1,
	PANEL_FMT_YUV = 2,
};

enum {
	/* Panel Output Type */
	PANEL_TYPE_PRGB888 = 0,
	PANEL_TYPE_PRGB565 = 1,
	PANEL_TYPE_SRGB888 = 2,
	PANEL_TYPE_SRGBM888 = 3,

	PANEL_TYPE_YCBCR24 = 0,
	PANEL_TYPE_YCBCR16 = 1,
	PANEL_TYPE_YCBCR8 = 2,

	PANEL_TYPE_YUV24 = 0,
	PANEL_TYPE_YUV16 = 1,
	PANEL_TYPE_YUV8 = 2,

	/* LCM Output Type */
	PANEL_TYPE_RGB666 = 0,
	PANEL_TYPE_RGB565 = 1,
	PANEL_TYPE_RGB444 = 2,
	PANEL_TYPE_RGB332 = 3,
};

enum {
	/* data sequence */
	PANEL_SEQUENCE_PRGB888_RGB = 0,
	PANEL_SEQUENCE_PRGB888_BGR = 1,

	PANEL_SEQUENCE_PRGB565_RGB = 0,
	PANEL_SEQUENCE_PRGB565_BGR = 1,
	PANEL_SEQUENCE_PRGB565_G3R5B5G3 = 2,

	PANEL_SEQUENCE_SRGB888_RGB = 0,
	PANEL_SEQUENCE_SRGB888_GBR = 1,
	PANEL_SEQUENCE_SRGB888_BRG = 2,
	PANEL_SEQUENCE_SRGB888_RBG = 3,
	PANEL_SEQUENCE_SRGB888_BGR = 4,
	PANEL_SEQUENCE_SRGB888_GRB = 5,

	PANEL_SEQUENCE_SRGBM888_RGBM = 0,
	PANEL_SEQUENCE_SRGBM888_GBRM = 1,
	PANEL_SEQUENCE_SRGBM888_BRGM = 2,
	PANEL_SEQUENCE_SRGBM888_RBGM = 3,
	PANEL_SEQUENCE_SRGBM888_BGRM = 4,
	PANEL_SEQUENCE_SRGBM888_GRBM = 5,

	PANEL_SEQUENCE_YCBCR24_YCbCr = 0,
	PANEL_SEQUENCE_YCBCR24_YCrCb = 1,
	PANEL_SEQUENCE_YCBCR24_CbYCr = 2,
	PANEL_SEQUENCE_YCBCR24_CrYCb = 3,
	PANEL_SEQUENCE_YCBCR24_CbCrY = 4,
	PANEL_SEQUENCE_YCBCR24_CrCbY = 5,

	PANEL_SEQUENCE_YCBCR16_YCbYCr = 0,
	PANEL_SEQUENCE_YCBCR16_YCrYCb = 1,
	PANEL_SEQUENCE_YCBCR16_CbYCrY = 2,
	PANEL_SEQUENCE_YCBCR16_CrYCbY = 3,

	PANEL_SEQUENCE_YCBCR8_YCbYCr = 0,
	PANEL_SEQUENCE_YCBCR8_YCrYCb = 1,
	PANEL_SEQUENCE_YCBCR8_CbYCrY = 2,
	PANEL_SEQUENCE_YCBCR8_CrYCbY = 3,

	PANEL_SEQUENCE_YUV24_YUV = 0,
	PANEL_SEQUENCE_YUV24_YVU = 1,
	PANEL_SEQUENCE_YUV24_UYV = 2,
	PANEL_SEQUENCE_YUV24_VYV = 3,
	PANEL_SEQUENCE_YUV24_UVY = 4,
	PANEL_SEQUENCE_YUV24_VUY = 5,

	PANEL_SEQUENCE_YUV16_YUYV = 0,
	PANEL_SEQUENCE_YUV16_YVYU = 1,
	PANEL_SEQUENCE_YUV16_UYVY = 2,
	PANEL_SEQUENCE_YUV16_VYUY = 3,

	PANEL_SEQUENCE_YUV8_YUYV = 0,
	PANEL_SEQUENCE_YUV8_YVYU = 1,
	PANEL_SEQUENCE_YUV8_UYVY = 2,
	PANEL_SEQUENCE_YUV8_VYUY = 3,
};

enum {
	/* LCM interface */
	LCM_INTERFACE_16BIT = 0,
	LCM_INTERFACE_8BIT = 1,
};

enum {
	/* LCM mode */
	LCM_MODE_8080 = 0,
	LCM_MODE_6800 = 1,
};

enum {
	/* Panel Output GID MODE */
	GID_PRGB888 = 0,
	GID_PRGB777 = 1,
	GID_PRGB666 = 2,
	GID_PRGB565 = 3,
	GID_SRGB888 = 4,
	GID_SRGBM888 = 5,
	GID_TCON = 6,
	GID_LVDS = 7,
};

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct lcdTiming_s {
	uint16_t polarity;
	uint16_t fPorch;
	uint16_t bPorch;
	uint16_t width;
} lcdTiming_t;

typedef struct lcdTconTiming_s {
	uint16_t pos_ed;
	uint16_t pos_st;
} lcdTconTiming_t;

typedef struct lcdTconSTVH_s {
	uint16_t sth_pulse;
	uint16_t sth_area_ed;
	uint16_t sth_area_st;
	
	uint16_t stv_pstn;
	uint16_t sth_pstn;
} lcdTconSTVH_t;

typedef struct lcdTconInfo_s {
	uint16_t vcom_st;
	uint16_t pol_st;
	uint16_t tcon_en;		/* TCON Function enable */
	uint16_t inv_en;
	uint16_t edgsl_en;
	uint16_t scan_ctl;
} lcdTconInfo_t;

typedef struct LVDSconfig_s {
	uint16_t Enable;
	uint16_t HFME;
	uint16_t CV_R_LVDS;			/* 3-bit */
	uint16_t CPI_LVDS;			/* 2-bit */
	uint16_t PD_CH3_LVDS;		/* RGB888 only */
	uint16_t PD_LVDS;
} LVDSconfig_t;

typedef struct panel_lcdInfo_s {
	char *name;
	uint32_t workFreq;
	uint32_t clkPolatiry;
	gp_size_t resolution;
	gp_size_t pixelPitch;
	uint32_t format;
	uint32_t type;
	uint32_t dataSeqEven;
	uint32_t dataSeqOdd;
	lcdTiming_t vsync;
	lcdTiming_t hsync;
	gp_disp_colormatrix_t *pColorMatrix;
	uint8_t *pGammaTable[3];
	lcdTconSTVH_t	tcon_stvh;
	lcdTconTiming_t oeh;
	lcdTconTiming_t oev;
	lcdTconTiming_t ckv;
	lcdTconInfo_t	tcon_info;
	LVDSconfig_t	LVDSconfig;
} panel_lcdInfo_t;

typedef struct panel_lcmInfo_s {
	char *name;
	gp_size_t resolution;
	gp_size_t pixelPitch;
	uint32_t format;
	uint32_t type;
	uint32_t dataSeqEven;
	uint32_t dataSeqOdd;
	uint32_t interface;
	uint32_t mode;
	uint32_t dataSelect;
	gp_disp_colormatrix_t *pColorMatrix;
	uint8_t *pGammaTable[3];
} panel_lcmInfo_t;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/



#endif //endif _GP_PANEL_H_
