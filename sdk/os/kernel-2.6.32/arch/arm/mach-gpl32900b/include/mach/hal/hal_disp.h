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
#ifndef _HAL_DISP_H_
#define _HAL_DISP_H_

/**************************************************************************
 *                         H E A D E R   F I L E S                        *
 **************************************************************************/
#include <mach/typedef.h>
#include <mach/gp_panel.h>

/**************************************************************************
*                           C O N S T A N T S                             *
 **************************************************************************/
#define HAL_DISP_CTRLER_NUM 1	/* Display hardware controller number */
enum {
	HAL_DISP_DEV_LCD = 0,
	HAL_DISP_DEV_LCM,
	HAL_DISP_DEV_TV,
	HAL_DISP_DEV_HDMI,
	HAL_DISP_DEV_MAX,
};

enum {
	HAL_DISP_CLK_TYPE_RGB888 = 0,
	HAL_DISP_CLK_TYPE_RGB565 = 0,
	HAL_DISP_CLK_TYPE_RGB666 = 1,
	HAL_DISP_CLK_TYPE_LCM = 2,
	HAL_DISP_CLK_TYPE_YPBPR = 3,
};

enum {
	HAL_DISP_INT_DISPLAY_OFF = 0x01,
	HAL_DISP_INT_FRAME_END = 0x04,
	HAL_DISP_INT_FIELD_END		= 0x08,
	HAL_DISP_INT_UPDATE_FAIL = 0x10,

	HAL_DISP_INT_PKT0_SEND		= 0x100,
	HAL_DISP_INT_PKT1_SEND		= 0x200,
	HAL_DISP_INT_PKT2_SEND		= 0x400,
	HAL_DISP_INT_PKT3_SEND		= 0x800,
	
	HAL_DISP_INT_ACR_SEND		= 0x1000,
	HAL_DISP_INT_GC_SEND		= 0x2000,
};

enum {
	HAL_DISP_INPUT_FMT_RGB = 0,
	HAL_DISP_INPUT_FMT_YCbCr = 1,
    HAL_DISP_INPUT_FMT_SEMI  = 3,
};

enum {
	HAL_DISP_INPUT_TYPE_RGB565 = 0,
	HAL_DISP_INPUT_TYPE_RGB555 = 1,
	HAL_DISP_INPUT_TYPE_RGB888 = 2,
	HAL_DISP_INPUT_TYPE_ARGB8888 = 3,

	HAL_DISP_INPUT_TYPE_YCbYCr = 0,
	HAL_DISP_INPUT_TYPE_4Y4Cb4Y4Cr = 1,
	HAL_DISP_INPUT_TYPE_YCbCr = 2,

	HAL_DISP_INPUT_TYPE_YCbYCr422 = 0,
    HAL_DISP_INPUT_TYPE_YCbYCr444 = 2,

	HAL_DISP_INPUT_TYPE_SEMI400 = 0,
    HAL_DISP_INPUT_TYPE_SEMI420 = 1,
    HAL_DISP_INPUT_TYPE_SEMI422 = 2,
    HAL_DISP_INPUT_TYPE_SEMI444 = 3,
};

enum {
	/* LCD Output Format */
	HAL_DISP_OUTPUT_FMT_RGB = 0,
	HAL_DISP_OUTPUT_FMT_YCbCr = 1,
	HAL_DISP_OUTPUT_FMT_YUV = 2,
};

enum {
	/* LCD Output Type */
	HAL_DISP_OUTPUT_TYPE_PRGB888 = 0,
	HAL_DISP_OUTPUT_TYPE_PRGB565 = 1,
	HAL_DISP_OUTPUT_TYPE_SRGB888 = 2,
	HAL_DISP_OUTPUT_TYPE_SRGBM888 = 3,

	HAL_DISP_OUTPUT_TYPE_YCbCr24 = 0,
	HAL_DISP_OUTPUT_TYPE_YCbCr16 = 1,
	HAL_DISP_OUTPUT_TYPE_YCbCr8 = 2,

	HAL_DISP_OUTPUT_TYPE_YUV24 = 0,
	HAL_DISP_OUTPUT_TYPE_YUV16 = 1,
	HAL_DISP_OUTPUT_TYPE_YUV8 = 2,

	/* LCM Output Type */
	HAL_DISP_OUTPUT_TYPE_RGB666 = 0,
	HAL_DISP_OUTPUT_TYPE_RGB565 = 1,
	HAL_DISP_OUTPUT_TYPE_RGB444 = 2,
	HAL_DISP_OUTPUT_TYPE_RGB332 = 3,
};

enum {
	HAL_DISP_PRGB888_RGB = 0,
	HAL_DISP_PRGB888_BGR = 1,

	HAL_DISP_PRGB565_RGB = 0,
	HAL_DISP_PRGB565_BGR = 1,

	HAL_DISP_SRGB888_RGB = 0,
	HAL_DISP_SRGB888_GBR = 1,
	HAL_DISP_SRGB888_BRG = 2,
	HAL_DISP_SRGB888_RBG = 3,
	HAL_DISP_SRGB888_BGR = 4,
	HAL_DISP_SRGB888_GRB = 5,

	HAL_DISP_SRGBM888_RGBM = 0,
	HAL_DISP_SRGBM888_GBRM = 1,
	HAL_DISP_SRGBM888_BRGM = 2,
	HAL_DISP_SRGBM888_RBGM = 3,
	HAL_DISP_SRGBM888_BGRM = 4,
	HAL_DISP_SRGBM888_GRBM = 5,

	HAL_DISP_YCBCR24_YCbCr = 0,
	HAL_DISP_YCBCR24_YCrCb = 1,
	HAL_DISP_YCBCR24_CbYCr = 2,
	HAL_DISP_YCBCR24_CrYCb = 3,
	HAL_DISP_YCBCR24_CbCrY = 4,
	HAL_DISP_YCBCR24_CrCbY = 5,

	HAL_DISP_YCBCR16_YCbYCr = 0,
	HAL_DISP_YCBCR16_YCrYCb = 1,
	HAL_DISP_YCBCR16_CbYCrY = 2,
	HAL_DISP_YCBCR16_CrYCbY = 3,

	HAL_DISP_YCBCR8_YCbYCr = 0,
	HAL_DISP_YCBCR8_YCrYCb = 1,
	HAL_DISP_YCBCR8_CbYCrY = 2,
	HAL_DISP_YCBCR8_CrYCbY = 3,

	HAL_DISP_YUV24_YUV = 0,
	HAL_DISP_YUV24_YVU = 1,
	HAL_DISP_YUV24_UYV = 2,
	HAL_DISP_YUV24_VYV = 3,
	HAL_DISP_YUV24_UVY = 4,
	HAL_DISP_YUV24_VUY = 5,

	HAL_DISP_YUV16_YUYV = 0,
	HAL_DISP_YUV16_YVYU = 1,
	HAL_DISP_YUV16_UYVY = 2,
	HAL_DISP_YUV16_VYUY = 3,

	HAL_DISP_YUV8_YUYV = 0,
	HAL_DISP_YUV8_YVYU = 1,
	HAL_DISP_YUV8_UYVY = 2,
	HAL_DISP_YUV8_VYUY = 3,
};

enum {
	HAL_DISP_LCM_16BIT = 0,
	HAL_DISP_LCM_8BIT = 1,
};

enum {
	HAL_DISP_LCM_8080 = 0,
	HAL_DISP_LCM_6800 = 1,
};

enum {
	HAL_DISP_TV_TYPE_NTSC = 0,
	HAL_DISP_TV_TYPE_PAL = 1,
};

enum {
	HAL_DISP_TV_PULSE6_5PULSE = 0,
	HAL_DISP_TV_PULSE6_6PULSE = 1,
};

enum {
	HAL_DISP_TV_SCANSEL_NONINTERLACED = 0,
	HAL_DISP_TV_SCANSEL_INTERLACED = 1,
	HAL_DISP_TV_SCANSEL_PROGRESSIVE = 2,
};

enum {
	HAL_DISP_TV_FSCTYPE_NTSCMJ = 0,
	HAL_DISP_TV_FSCTYPE_PALBDGHIN = 1,
	HAL_DISP_TV_FSCTYPE_NTSC443_PAL60 = 2,
	HAL_DISP_TV_FSCTYPE_PALM = 3,
	HAL_DISP_TV_FSCTYPE_PALNC = 4,
};

enum {
	HAL_DISP_TV_LINESEL_262_525 = 0,
	HAL_DISP_TV_LINESEL_312_625 = 1,
};

enum {
	HAL_DISP_TV_CBWIDTH_252 = 0,
	HAL_DISP_TV_CBWIDTH_225 = 1,
};

enum {
	HAL_DISP_TV_CBSEL_NTSCMJ = 0,
	HAL_DISP_TV_CBSEL_PALM = 1,
	HAL_DISP_TV_CBSEL_PALBDGHINNC = 2,
	HAL_DISP_TV_CBSEL_DISABLE = 3,
};

enum {
	HAL_DISP_TV_DMA_PROGRESSIVE = 0,
	HAL_DISP_TV_DMA_INTERLACED = 1,
};

enum {
	HAL_DISP_DITHER_FIXED = 0,
	HAL_DISP_DITHER_WHEEL = 1,
	HAL_DISP_DITHER_HERRDIFFUSTION = 2,
};

enum {
	HAL_DISP_OSD_0 = 0,
	HAL_DISP_OSD_1 = 1,
	HAL_DISP_OSD_MAX = 2,
};

enum {
	HAL_DISP_OSD_FMT_RGB565 = 0,
	HAL_DISP_OSD_FMT_RGB5515 = 1,
	HAL_DISP_OSD_FMT_RGB1555 = 2,
    HAL_DISP_OSD_FMT_ALPHA    = 3,
    HAL_DISP_OSD_FMT_ARGB8888 = 4,
    HAL_DISP_OSD_FMT_ABGR8888 = 5,
    HAL_DISP_OSD_FMT_RGBA8888 = 6,
    HAL_DISP_OSD_FMT_BGRA8888 = 7,
};

enum {
	HAL_DISP_OSD_TYPE_16BPP = 0,
	HAL_DISP_OSD_TYPE_8BPP = 1,
	HAL_DISP_OSD_TYPE_4BPP = 2,
	HAL_DISP_OSD_TYPE_1BPP = 3,
};

enum {
	DISP_MODE_VDAC = 0,
	DISP_MODE_CVBS,
	DISP_MODE_LCD,
	DISP_MODE_LCM,
	DISP_MODE_TCON,
};

enum {
	DISP_PATH_PPU = 0,
	DISP_PATH_DISP0,
};

/*******************************************************************************
*                          D A T A    T Y P E S
*******************************************************************************/
typedef struct {
	UINT16 width;
	UINT16 height;
} gpHalDispRes_t;

typedef struct {
	UINT16 top;
	UINT16 bottom;
	UINT16 left;
	UINT16 right;
	UINT32 pattern;
} gpHalDispBlankInfo_t;

typedef struct {
	UINT16 srcWidth;
	UINT16 srcHeight;
	UINT16 dstWidth;
	UINT16 dstHeight;
	UINT16 hInit;
	UINT16 vInit0;
	UINT16 vInit1;
} gpHalDispSclInfo_t;

typedef struct {
	UINT32 addrSetup;
	UINT32 addrHold;
	UINT32 csSetup;
	UINT32 csHold;
	UINT32 cycLength;
} gpHalDispLcmTiming_t;

typedef struct {
	UINT32 luminance;
	UINT32 blank;
	UINT32 burst;
} gpHalDispTvAmpAdj_t;

typedef struct {
	UINT32 vAct0;
	UINT32 vAct1;
	UINT32 hAct;
} gpHalDispTvPosAdj_t;


/*******************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
*******************************************************************************/
/* hal_disp.c */
void gpHalDispInit(void *RegBase);
void gpHalDispUpdateParameter(void *RegBase);
void gpHalDispSetIntEnable(void *RegBase, UINT32 field);
void gpHalDispSetIntDisable(void *RegBase, UINT32 field);
void gpHalDispClearIntFlag(void *RegBase, UINT32 field);
int gpHalDispGetDevType(void *RegBase);
// void gpHalDispOsdRegInit(void *RegBase);
void gpHalDispSetClockEnable(void *RegBase, UINT32 enable);
void scu_change_pin_grp(void *RegBase, UINT32 aPinGrp, UINT32 aPinNum);

/* Primary layer */
void gpHalDispSetEnable(void *RegBase, int type, int enable);
UINT32 gpHalDispGetEnable(void *RegBase);
UINT32 gpHalDispGetOutputType(void *RegBase);
void gpHalDispSetPriBlank(void *RegBase, const gpHalDispBlankInfo_t *blankInfo);
void gpHalDispSetPriFrameAddr(void *RegBase, const void *addr);
void gpHalDispSetPriUVAddr(void *RegBase, const void *addr);
void gpHalDispSetPriPitch(void *RegBase, UINT16 src, UINT16 act);
void gpHalDispSetPriRes(void *RegBase, UINT16 width, UINT16 height);
void gpHalDispSetPriSclInfo(void *RegBase, gpHalDispSclInfo_t scale);
void gpHalDispSetPriSclEnable(void *RegBase, UINT32 hEnable, UINT32 vEnable);
void gpHalDispSetPriInputInfo(void *RegBase, UINT32 format, UINT32 type);
unsigned long gpHalDispGetPriInputInfo(void *RegBase);
//void gpHalDispSetPriBurst(UINT32 burst);		// Removed in GP8300. SJ 20110221
void gpHalDispSetPriBurstNum(void *RegBase, UINT32 burstNum);
void gpHalDispSetPriDmaType(void *RegBase, UINT32 type);
UINT32 gpHalDispGetPriDmaType(void *RegBase);
void gpHalDispSetHueAdj(void *RegBase, UINT32 enable);
void gpHalDispSetPriFlip(void *RegBase, UINT32 value);

/* Dither */
void gpHalDispSetDitherEnable(void *RegBase, UINT32 enable);
UINT32 gpHalDispGetDitherEnable(void *RegBase);
void gpHalDispSetDitherType(void *RegBase, UINT32 type);
UINT32 gpHalDispGetDitherType(void *RegBase);
void gpHalDispSetDitherMap(void *RegBase, UINT32 map0, UINT32 map1);
void gpHalDispGetDitherMap(void *RegBase, UINT32 *map0, UINT32 *map1);

/* Color matrix */
void gpHalDispSetColorMatrix(void *RegBase, const UINT16 *matrix);
void gpHalDispGetColorMatrix(void *RegBase, UINT16 *matrix);
void gpHalDispSetDynCMtxInfo(void *RegBase, unsigned long dispCMatxLimit);
void gpHalDispSetDynCMtxIndex(void *RegBase, UINT32 *threshold);
void gpHalDispSetDynCMtxTable(void *RegBase, const gp_disp_dyncmtxpara_t *cMatrix);

/* Gamma */
void gpHalDispSetGammaEnable(void *RegBase, UINT32 enable);
UINT32 gpHalDispGetGammaEnable(void *RegBase);
void gpHalDispSetGammaTable(void *RegBase, int id, const void *table);
void gpHalDispSetMaxGamma(void *RegBase);

/* Color bar */
void gpHalDispSetColorBarEnable(void *RegBase, UINT32 enable);
UINT32 gpHalDispGetColorBarEnable(void *RegBase);
void gpHalDispSetColorBar(void *RegBase, UINT32 type, UINT32 size, UINT32 color);

/* Osd layer */
void gpHalDispSetOsdEnable(void *RegBase, UINT32 layerNum, UINT32 enable);
UINT32 gpHalDispGetOsdEnable(void *RegBase, UINT32 layerNum);
void gpHalDispSetOsdXY(void *RegBase, UINT32 layerNum, UINT16 x, UINT16 y);
void gpHalDispSetOsdFrameAddr(void *RegBase, UINT32 layerNum, const void *addr);
void gpHalDispSetOsdPitch(void *RegBase, UINT32 layerNum, UINT16 src, UINT16 act);
void gpHalDispSetOsdRes(void *RegBase, UINT32 layerNum, UINT16 width, UINT16 height);
void gpHalDispSetOsdSclInfo(void *RegBase, UINT32 layerNum, gpHalDispSclInfo_t scale);
void gpHalDispSetOsdSclEnable(void *RegBase, UINT32 layerNum, UINT32 hEnable,UINT32 vEnable);
void gpHalDispSetOsdInputFmt(void *RegBase, UINT32 layerNum, UINT32 format);
void gpHalDispSetOsdInputType(void *RegBase, UINT32 layerNum, UINT32 type);
//void gpHalDispSetOsdBurst(void *RegBase, UINT32 layerNum, UINT32 burst);		//Removed in GP8300. SJ 20110221
void gpHalDispSetOsdBurstNum(void *RegBase, UINT32 layerNum, UINT32 burstnum);
void gpHalDispSetOsdAlpha(void *RegBase, UINT32 layerNum, UINT32 consta, UINT32 ppamd, UINT32 alpha, UINT16 alphasel);
void gpHalDispSetOsdColorKey(void *RegBase, UINT32 layerNum, UINT32 color);
void gpHalDispSetOsdPalette(void *RegBase, int layerNum, int startIndex, int count, const void *pColorTable);
void gpHalDispSetOsdPaletteOffset(void *RegBase, UINT32 layerNum, UINT32 offset);
UINT32 gpHalDispGetOsdPaletteOffset(void *RegBase, UINT32 layerNum);
void gpHalDispSetOsdDmaType(void *RegBase, UINT32 layerNum, UINT32 type);
void gpHalDispSetOsdFlip(void *RegBase, UINT32 layerNum, UINT32 value);

/* LCD panel */
void gpHalDispSetRes(void *RegBase, UINT16 width, UINT16 height);
void gpHalDispGetRes(void *RegBase, UINT16 *width, UINT16 *height);
void gpHalDispSetLcdVsync(void *RegBase, lcdTiming_t vsync);
void gpHalDispSetLcdHsync(void *RegBase, lcdTiming_t hsync);
void gpHalDispSetPanelFormat(void *RegBase, UINT32 format, UINT32 type, UINT32 seq0, UINT32 seq1);
void gpHalDispSetClkPolarity(void *RegBase, UINT32 polarity);

/* LCD TCON */
void gpHalDispSetTconSTHarea(void *RegBase, lcdTconSTVH_t tcon_stvh);
void gpHalDispSetTconSTVH(void *RegBase, lcdTconSTVH_t tcon_stvh);
void gpHalDispSetTconOEH(void *RegBase, lcdTconTiming_t oeh);
void gpHalDispSetTconOEV(void *RegBase, lcdTconTiming_t oev);
void gpHalDispSetTconCKV(void *RegBase, lcdTconTiming_t ckv);
void gpHalDispSetTconPOL(void *RegBase, lcdTconInfo_t tcon_info);
void gpHalDispSetTconConfig(void *RegBase, lcdTconInfo_t tcon_info);

/* LCD LVDS */
void gpHalDispSetLVDSConfig(void *RegBase, LVDSconfig_t	LVDSconfig);

/* LCM panel */
void gpHalDispSetLcmInterface(void *RegBase, UINT32 interface);
void gpHalDispSetLcmMode(void *RegBase, UINT32 mode);
void gpHalDispSetLcmDataSelect(void *RegBase, UINT32 sel);
void gpHalDispSetLcmAcTiming(void *RegBase, gpHalDispLcmTiming_t timing);

/* TV */
void gpHalDispSetTvType(void *RegBase, UINT32 type);
void gpHalDispSetTvPulse(void *RegBase, UINT32 pulse);
void gpHalDispSetTvScan(void *RegBase, UINT32 scan);
void gpHalDispSetTvFscType(void *RegBase, UINT32 fsc);
void gpHalDispSetTvFix625(void *RegBase, UINT32 fix);
void gpHalDispSetTvLine(void *RegBase, UINT32 line);
void gpHalDispSetTvColorBurstWidth(void *RegBase, UINT32 width);
void gpHalDispSetTvColorBurstSel(void *RegBase, UINT32 sel);
void gpHalDispSetTvCftType(void *RegBase, UINT32 type);
void gpHalDispSetTvCupType(void *RegBase, UINT32 type);
void gpHalDispSetTvYupType(void *RegBase, UINT32 type);
void gpHalDispSetTvAmpAdj(void *RegBase, gpHalDispTvAmpAdj_t amp);
void gpHalDispSetTvPosAdj(void *RegBase, gpHalDispTvPosAdj_t pos);
void gpHalDispSetBlankingIntervalTo0(void *RegBase, UINT32 enable);
void gpHalDispSetTVClock(void *RegBase);
void gpHalDispSetVDACPowerDown(void *RegBase, UINT32 enable);

//void gpHalDispDumpRegister(void *RegBase);

/* Misc */
void gpHalDispSetDeflickerInfo(void *RegBase, UINT32 vEnable, UINT32 hEnable, UINT32 DefInterlaced, UINT32 FSTFieldOutSE);
void gpHalDispSetLcdTiming(void *RegBase, UINT32 interlaced, UINT32 Hoffset, UINT32 Vtotal);
UINT32 gpHalDispGetIntFlag(void *RegBase);

//Edge gain, coring and clip settings
void gpHalDispSetEdgeType(void *RegBase, UINT32 type);
void gpHalDispSetEdgeGain(void *RegBase, UINT32 value);
void gpHalDispSetCortype(void *RegBase, UINT32 value);
void gpHalDispSetCliptype(void *RegBase, UINT32 value);

void gpHalDispSetOFunc(void *RegBase, UINT32 type);

void *gpHalDispGetRegBase(int idx);

void gpHalDispHDMISetAudioTimingSlot(void *RegBase, int val);
void gpHalDispHDMISetHPolarity(void *RegBase, int polarity);
void gpHalDispHDMISetVPolarity(void *RegBase, int polarity);

void gpHalDispPathSelect(int mode, int path);
void gpHalDispSetClock(unsigned int freq, unsigned int pllsel);

#endif  /* __HAL_DISP_H__ */

