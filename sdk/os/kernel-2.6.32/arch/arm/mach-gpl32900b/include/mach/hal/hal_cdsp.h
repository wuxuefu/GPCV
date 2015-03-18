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

#ifndef HAL_CDSP_H
#define HAL_CDSP_H

#include <mach/common.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
/* cdsp int type */ 
#define CDSP_INT_BIT		0x1
#define FRONT_VD_INT_BIT	0x2
#define FRONT_INT_BIT		0x4

/* cdsp interrupt bit */
#define CDSP_AFWIN_UPDATE	(1<<0)
#define CDSP_AWBWIN_UPDATE	(1<<1)
#define CDSP_AEWIN_SEND		(1<<2)
#define CDSP_OVERFOLW		(1<<3)
#define CDSP_EOF			(1<<4)
#define CDSP_FACWR			(1<<5)
#define CDSP_INT_ALL		0x3F

/* front vd interrupt bit */
#define FRONT_VD_RISE		0x1
#define FRONT_VD_FALL		0x2

/* front interrupt bit */
#define VDR_EQU_VDRINT_NO	(1<<0)
#define VDF_EQU_VDFINT_NO	(1<<1)
#define SERIAL_DONE			(1<<2)
#define SNAP_DONE			(1<<3)
#define CLR_DO_CDSP			(1<<4)
#define EHDI_FS_EQU_INTH_NO	(1<<5)
#define FRONT_VVALID_RISE	(1<<6)
#define FRONT_VVALID_FALL	(1<<7)
#define OVERFLOW_RISE		(1<<8)
#define OVERFLOW_FALL		(1<<9)
#define FRONT_INT_ALL		0x3FF

/* cdsp input source */
#define C_CDSP_CLK_ENABLE	0x0
#define C_CDSP_CLK_DISABLE	0x1
#define C_CDSP_CLK_FB		0x2
#define C_CDSP_CLK_FRONT	0x3
#define C_CDSP_CLK_MIPI		0x4

/* image input source */
#define C_SDRAM_FMT_RAW8	0x10
#define C_SDRAM_FMT_RAW10	0x11
#define C_FRONT_FMT_RAW8	0x12
#define C_FRONT_FMT_RAW10	0x13
#define C_MIPI_FMT_RAW8		0x14
#define C_MIPI_FMT_RAW10	0x15

#define C_SDRAM_FMT_8Y4U4V	0x1F
#define C_SDRAM_FMT_VY1UY0	0x20
#define C_FRONT_FMT_UY1VY0	0x21
#define C_FRONT_FMT_Y1VY0U	0x22
#define C_FRONT_FMT_VY1UY0	0x23
#define C_FRONT_FMT_Y1UY0V	0x24
#define C_MIPI_FMT_Y1VY0U	0x25

/* dma set */
#define RD_A_WR_A			0x0
#define RD_A_WR_B			0x1
#define RD_B_WR_B			0x2
#define RD_B_WR_A			0x3
#define AUTO_SWITCH			0x4	

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct edge_filter_s
{
	UINT8 LPF00;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 LPF01;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */	
	UINT8 LPF02;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 RSD0;
	
	UINT8 LPF10;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 LPF11;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 LPF12;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 RSD1;

	UINT8 LPF20;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 LPF21;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 LPF22;		/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	UINT8 RSD2;
} edge_filter_t;

typedef struct color_matrix_s
{
	UINT16 CcCof00;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	UINT16 CcCof01;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	UINT16 CcCof02;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	
	UINT16 CcCof10;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	UINT16 CcCof11;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	UINT16 CcCof12;		/* 1 sign + 3.6 bit, -512/64~511/64 */

	UINT16 CcCof20;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	UINT16 CcCof21;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	UINT16 CcCof22;		/* 1 sign + 3.6 bit, -512/64~511/64 */
	UINT16 RSD0;
} color_matrix_t;

typedef struct uv_divide_s
{
	UINT8 YT1;			/* 0~0xFF, when Y=T1, UV divide 1/8 */	
	UINT8 YT2;			/* 0~0xFF, when Y=T2, UV divide 2/8 */
	UINT8 YT3;			/* 0~0xFF, when Y=T3, UV divide 3/8 */
	UINT8 YT4;			/* 0~0xFF, when Y=T4, UV divide 4/8 */
	UINT8 YT5;			/* 0~0xFF, when Y=T5, UV divide 5/8 */
	UINT8 YT6;			/* 0~0xFF, when Y=T6, UV divide 6/8 */
	UINT8 RSD0;
	UINT8 RSD1;
} uv_divide_t;

typedef struct awb_uv_thr_s
{
	SINT16 UL1N1; 
	SINT16 UL1P1;
	SINT16 VL1N1;
	SINT16 VL1P1;

	SINT16 UL1N2; 
	SINT16 UL1P2;
	SINT16 VL1N2;
	SINT16 VL1P2;

	SINT16 UL1N3; 
	SINT16 UL1P3;
	SINT16 VL1N3;
	SINT16 VL1P3;
} awb_uv_thr_t;

typedef struct af_windows_value_s
{
	UINT32 h_value_l;
	UINT32 h_value_h;
	UINT32 v_value_l;
	UINT32 v_value_h;
} af_windows_value_t;

typedef struct front_roi_s 
{	
	UINT16 hSize;	
	UINT16 hOffset;	
	UINT16 vSize;	
	UINT16 vOffset;
} front_roi_t;

typedef struct front_reshape_s {	
	UINT32 mode; // 0: RAW ; 1: YUV ; 2:MIPI	
	UINT8 hReshapeEn;	
	UINT8 vReshapeEn;	
	UINT8 vReshapeClkType; // 0: H-sync ; 1: pclk	
	UINT8 vBackOffEn;	
	UINT16 hRise;	
	UINT16 hFall;	
	UINT16 vRise;	
	UINT16 vFall;
} front_reshape_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
 /**
 * @brief   cdsp module reset function
 * @param   enable[in]: disable(0)/enable(1)
 * @return  None
 * @see
 */
void gpHalCdspModuleRest(UINT32 enable);

/**
 * @brief	cdsp module vic interrupt enable/disable
 * @param 	enable[in]: enable
 * @return 	none
*/
void gpHalCdspSetVicIntEn(UINT32 enable); 

/**
 * @brief	cdsp module clock enable
 * @param 	clko_en[in]: enable
 * @param 	clko_div[in]: mclk divider, (clko_div+1)
 * @return 	none
*/
void gpHalCdspSetMclk(UINT32 clk_sel, UINT32 clko_div, UINT32 pclk_dly, UINT32 pclk_revb);
void gpHalCdspGetMclk(UINT8 *clk_sel, UINT8 *clko_div, UINT8 *pclk_dly, UINT8 *pclk_revb);

/**
* @brief	cdsp clock set
* @param 	mode[in]: 0: cdsp enable, 1: cdsp disable, 2: sdram path, 3:front sensor, 4: mipi if
* @param 	type[in]: 0: raw, 1:yuv
* @return 	none
*/
void gpHalCdspSetClk(UINT8 mode, UINT8 type);

/**
* @brief	cdsp reset
* @param 	none
* @return 	none
*/
void gpHalCdspReset(void);

/**
* @brief	cdsp raw data format
* @param	format [in]: format
* @return	none
*/
void gpHalCdspSetRawDataFormat(UINT8 format);
UINT8 gpHalCdspGetRawDataFormat(void);

/**
* @brief	cdsp yuv range
* @param	signed_yuv_en [in]: yuv signed/unsigned set
* @return	none
*/
void gpHalCdspSetYuvRange(UINT8 signed_yuv_en);
UINT8 gpHalCdspGetYuvRange(void);

/**
* @brief	cdsp image source
* @param	image_source [in]: image input source, 0:front, 1:sdram, 2:mipi
* @return	none
*/
void gpHalCdspDataSource(UINT8 image_source);
UINT8 gpHalCdspGetDataSource(void);

/**
* @brief	cdsp post-process triger
* @param	docdsp [in]: enable
* @return	none
*/
void gpHalCdspRedoTriger(UINT8 docdsp);

/**
* @brief	cdsp interrupt enable
* @param	enable [in]: enable / disable
* @param	bit [in]: interrupt bit
* @return	none
*/
void gpHalCdspSetIntEn(UINT8 enable, UINT8 bit);

/**
* @brief	cdsp get interrupt bit
* @param	none
* @return	bit
*/
UINT32 gpHalCdspGetIntEn(void);

/**
* @brief	cdsp interrupt clear status
* @param	bit [in]: clear interrupt bit
* @return	none
*/
void gpHalCdspClrIntStatus(UINT8 bit);

/**
* @brief	cdsp get int status
* @param	none
* @return	int status
*/
UINT32 gpHalCdspGetIntStatus(void);

/**
* @brief	cdsp get front vd int status
* @param	none
* @return	int status
*/
UINT32 gpHalCdspGetFrontVdIntStatus(void);

/**
* @brief	cdsp get front int status
* @param	none
* @return	int status
*/
UINT32 gpHalCdspGetFrontIntStatus(void);

/**
* @brief	cdsp get global int status
* @param	none
* @return	int status
*/
UINT32 gpHalCdspGetGlbIntStatus(void);

/**
* @brief	cdsp bad pixel enable
* @param	badpixen [in]: bad pixel enable
* @param	badpixen [in]: bad pixel mirror enable, bit1: right, bit0: left, 
* @return	none
*/
void gpHalCdspSetBadPixelEn(UINT8 badpixen, UINT8 badpixmiren);
void gpHalCdspGetBadPixelEn(UINT8 *badpixen, UINT8 *badpixmiren);

/**
* @brief	cdsp bad pixel threshold set
* @param	bprthr [in]: R threshold
* @param	bpgthr [in]: G threshold 
* @param	bpbthr [in]: B threshold 
* @return	none
*/
void gpHalCdspSetBadPixel(UINT16 bprthr, UINT16 bpgthr, UINT16 bpbthr);
void gpHalCdspGetBadPixel(UINT16 *bprthr, UINT16 *bpgthr, UINT16 *bpbthr);

/**
* @brief	cdsp manual optical black enable
* @param	manuoben [in]: manual optical black enable
* @param	manuob [in]: manual optical subtracted value 
* @return	none
*/
void gpHalCdspSetManuOB(UINT8 manuoben, UINT16 manuob);
void gpHalCdspGetManuOB(UINT8 *manuoben, UINT16 *manuob);

/**
* @brief	cdsp auto optical black enable
* @param	autooben [in]: auto optical black enable
* @param	obtype [in]: auto optical accumulation block type
* @param	obHOffset [in]: auto optical accumulation block horizontal offset
* @param	obVOffset [in]: auto optical accumulation block vertical offset
* @return	none
*/
void gpHalCdspSetAutoOB(UINT8 autooben,UINT8 obtype,UINT16 obHOffset,UINT16 obVOffset);
void gpHalCdspGetAutoOB(UINT8 *autooben, UINT8 *obtype, UINT16 *obHOffset, UINT16 *obVOffset);

/**
* @brief	cdsp get auto optical black average
* @param	Ravg [out]: r average
* @param	GRavg [out]: gr average
* @param	Bavg [out]: b average
* @param	GBavg [out]: gb average
* @return	none
*/
void gpHalCdspGetAutoOBAvg(UINT16 *Ravg, UINT16 *GRavg, UINT16 *Bavg, UINT16 *GBavg);

/**
* @brief	cdsp lens compensation enable
* @param	plensdata [in]: lens compensation table pointer
* @return	none
*/
void gpHalCdspInitLensCmp(UINT16 *plensdata);

/**
* @brief	cdsp lens compensation enable
* @param	lcen [in]: lens compensation enable
* @return	none
*/
void gpHalCdspSetLensCmpEn(UINT8 lcen);
UINT32 gpHalCdspGetLensCmpEn(void);

/**
* @brief	cdsp lens compensation postion
* @param	centx [in]: X center 
* @param	centy [in]: Y center
* @param	xmoffset [in]: X offset
* @param	ymoffset [in]: Y offset
* @return	none
*/
void gpHalCdspSetLensCmpPos(UINT16 centx, UINT16 centy, UINT16 xmoffset, UINT16 ymoffset);
void gpHalCdspGetLensCmpPos(UINT16 *centx, UINT16 *centy, UINT16 *xmoffset, UINT16 *ymoffset);

/**
* @brief	cdsp lens compensation enable
* @param	stepfactor [in]: step unit between entries of len shading compensation LUT
* @param	xminc [in]: X increase step
* @param	ymoinc [in]: Y increase step odd line
* @param	ymeinc [in]: Y increase step even line
* @return	none
*/
void gpHalCdspSetLensCmp(UINT8 stepfactor, UINT8 xminc, UINT8 ymoinc, UINT8 ymeinc);
void gpHalCdspGetLensCmp(UINT8 *stepfactor, UINT8 *xminc, UINT8 *ymoinc, UINT8 *ymeinc);

/**
* @brief	cdsp yuv lens compensation path set
* @param	yuvlens [in]: 0:yuv path2, 1:yuv path5
* @return	none
*/
void gpHalCdspSetLensCmpPath(UINT8 yuvlens);
UINT8 gpHalCdspGetLensCmpPath(void);

/**
* @brief	cdsp crop function enable
* @param	hv_crop_en [in]: h/v crop enable
* @return	none
*/
void gpHalCdspSetCropEn(UINT8 hv_crop_en);
UINT8 gpHalCdspGetCropEn(void);

/**
* @brief	cdsp crop function
* @param	hoffset [in]: h offset set
* @param	voffset [in]: v offset set
* @param	hsize [in]: h crop size
* @param 	vsize [in]: v crop size
* @return	none
*/
void gpHalCdspSetCrop(UINT16 crop_hoffset, UINT16 crop_voffset, UINT16 crop_hsize, UINT16 crop_vsize);
void gpHalCdspGetCrop(UINT16 *crop_hoffset, UINT16 *crop_voffset, UINT16 *crop_hsize, UINT16 *crop_vsize);


/**
* @brief	cdsp vcrop function
* @param	voffset [in]: v offset set
* @param 	vsize [in]: v crop size
* @param	v_crop_en [in]: v crop enable
* @return	none
*/
void gpHalCdspSetVCrop(UINT16 crop_voffset, UINT16 crop_vsize, UINT8 v_crop_en);

/**
* @brief	cdsp raw horizontal scale down enable
* @param	hscale_en [in]: horizontal scale down enable 
* @param	hscale_mode [in]: scale down mode, 0:drop, 1:filter
* @return	none
*/
void gpHalCdspSetRawHScaleEn(UINT8 hscale_en, UINT8 hscale_mode);
void gpHalCdspGetRawHScaleEn(UINT8 *hscale_en, UINT8 *hscale_mode);

/**
* @brief	cdsp raw horizontal scale down set
* @param	src_hsize [in]: source width 
* @param	dst_hsize [in]: densting width
* @return	none
*/
void gpHalCdspSetRawHScale(UINT8 src_hsize, UINT8 dst_hsize);


/**
* @brief	cdsp whitle balance offset set enable
* @param	wboffseten [in]: white balance enable
* @return	none
*/
void gpHalCdspSetWbOffsetEn(UINT8 wboffseten);

/**
* @brief	cdsp whitle balance offset set
* @param	roffset [in]: R offset 
* @param	groffset [in]: Gr offset
* @param	boffset [in]: B offset
* @param	gboffset [in]: Gb offset
* @return	none
*/
void gpHalCdspSetWbOffset(UINT8 roffset, UINT8 groffset, UINT8 boffset, UINT8 gboffset);

/**
* @brief	cdsp whitle balance offset get
* @param	roffset [out]: R offset 
* @param	groffset [out]: Gr offset
* @param	boffset [out]: B offset
* @param	gboffset [out]: Gb offset
* @return	wboffset enable
*/
UINT32 gpHalCdspGetWbOffset(UINT8 *roffset, UINT8 *groffset, UINT8 *boffset, UINT8 *gboffset);

/**
* @brief	cdsp whitle balance gain set enable
* @param	wboffseten [in]: white balance enable
* @return	none
*/
void gpHalCdspSetWbGainEn(UINT8 wbgainen);

/**
* @brief	cdsp whitle balance gain set
* @param	r_gain [in]: R gain 
* @param	gr_gain [in]: Gr gain
* @param	b_gain [in]: B gain
* @param	gb_gain [in]: Gb gain
* @return 	none
*/
void gpHalCdspSetWbGain(UINT16 rgain, UINT16 grgain, UINT16 bgain, UINT16 gbgain);


/**
* @brief	cdsp whitle balance r & b gain set
* @param	r_gain [in]: R gain 
* @param	b_gain [in]: B gain
* @return 	none
*/
void gpHalCdspSetWb_r_b_Gain(UINT16 rgain, UINT16 bgain);

/**
* @brief	cdsp whitle balance gain get
* @param	r_gain [out]: R gain 
* @param	gr_gain [out]: Gr gain
* @param	b_gain [out]: B gain
* @param	gb_gain [out]: Gb gain
* @return 	wb gain enable 
*/
UINT32 gpHalCdspGetWbGain(UINT16 *rgain, UINT16 *grgain, UINT16 *bgain, UINT16 *gbgain);

/**
* @brief	cdsp whitle balance global gain set
* @param	global_gain [in]: wb global gain set
* @return	none
*/
void gpHalCdspSetGlobalGain(UINT8 global_gain);
UINT8 gpHalCdspGetGlobalGain(void);

/**
* @brief	cdsp lut gamma table enable
* @param	pGammaTable [in]: lut gamma table pointer 
* @return	none
*/
void gpHalCdspInitGamma(UINT32 *pGammaTable);

/**
* @brief	cdsp lut gamma table enable
* @param	lut_gamma_en [in]: lut gamma table enable 
* @return	none
*/
void gpHalCdspSetLutGammaEn(UINT8 lut_gamma_en);
UINT8 gpHalCdspGetLutGammaEn(void);

/**
* @brief	cdsp set line buffer
* @param	path [in]: 0: raw data, 1: YUV data	
* @return	none
*/
void gpHalCdspSetLineCtrl(UINT8 ybufen);

/**
* @brief	cdsp set external line and bank
* @param	linesize [in]: external line size
* @param	lineblank [in]: external bank size
* @return	none
*/
void gpHalCdspSetExtLine(UINT16 linesize, UINT16 lineblank);

/**
* @brief	cdsp external line path set
* @param	extinen [in]: external line enable
* @param	path [in]: 0:interpolution, 1:uvsuppression
* @return	none
*/
void gpHalCdspSetExtLinePath(UINT8 extinen, UINT8 path);

/**
* @brief	cdsp interpolation mirror enable
* @param	intplmiren [in]: mirror enable, bit0:left, bit1:right, bit2:top, bit3:down
* @param	intplmirvsel [in]: vertical down mirror postion select
* @param	intplcnt2sel [in]: initial count select 0:0x0, 1:0x7
* @return	none
*/
void gpHalCdspSetIntplMirEn(UINT8 intplmiren, UINT8 intplmirvsel, UINT8 intplcnt2sel);
void gpHalCdspGetIntplMirEn(UINT8 *intplmiren, UINT8 *intplmirvsel, UINT8 *intplcnt2sel);

/**
* @brief	cdsp interpolation threshold set
* @param	int_low_thr [in]: low threshold
* @param	int_hi_thr [in]: heig threshold
* @return	none
*/
void gpHalCdspSetIntplThr(UINT8 int_low_thr, UINT8 int_hi_thr);
void gpHalCdspGetIntplThr(UINT8 *int_low_thr, UINT8 *int_hi_thr);

/**
* @brief	cdsp raw special mode set
* @param	rawspecmode [in]: raw special mode
* @return	none
*/
void gpHalCdspSetRawSpecMode(UINT8 rawspecmode);
UINT8 gpHalCdspGetRawSpecMode(void);

/**
* @brief	cdsp edge source set
* @param	posyedgeen [in]: 0:raw, 1: YUV
* @return	none
*/
void gpHalCdspSetEdgeSrc(UINT8 posyedgeen);
UINT8 gpHalCdspGetEdgeSrc(void);

/**
* @brief	cdsp edge enable
* @param	edgeen [in]: edge enable, effective when raw data input
* @return	none
*/
void gpHalCdspSetEdgeEn(UINT8 edgeen);
UINT8 gpHalCdspGetEdgeEn(void);

/**
* @brief	cdsp edge HPF matrix set
* @param	pLPF [in]: low pass filter parameter
* @return	none
*/
void gpHalCdspSetEdgeFilter(edge_filter_t *pLPF);

/**
* @brief	cdsp edge HPF matrix get
* @param	pLPF [out]: low pass filter parameter
* @return	none
*/
void gpHalCdspGetEdgeFilter(edge_filter_t *pLPF);

/**
* @brief	cdsp edge scale set
* @param	lhdiv [in]: L edge enhancement edge vale scale
* @param	lhtdiv [in]: L edge enhancement edge vale scale
* @param	lhcoring [in]: L core ring threshold
* @param	lhmode [in]: 1: default matrix, 0: enable paroramming matrix
* @return	none
*/
void gpHalCdspSetEdgeLCoring(UINT8 lhdiv, UINT8 lhtdiv, UINT8 lhcoring, UINT8 lhmode);
void gpHalCdspGetEdgeLCoring(UINT8 *lhdiv, UINT8 *lhtdiv, UINT8 *lhcoring, UINT8 *lhmode);

/**
* @brief	cdsp edge amp set
* @param	ampga [in]: 0:1, 1:2, 2:3, 3:4
* @param	edgedomain [in]: 0:add edge on y value, 1:add edge on rgb value
* @return	none
*/
void gpHalCdspSetEdgeAmpga(UINT8 ampga);
UINT8 gpHalCdspGetEdgeAmpga(void);

/**
* @brief	cdsp edge domain set
* @param	edgedomain [in]: 0:add edge on y value, 1:add edge on rgb value
* @return	none
*/
void gpHalCdspSetEdgeDomain(UINT8 edgedomain);
UINT8 gpHalCdspGetEdgeDomain(void);

/**
* @brief	cdsp edge Q threshold set
* @param	Qthr [in]: edge threshold
* @return	none
*/
void gpHalCdspSetEdgeQthr(UINT8 Qthr);
UINT32 gpHalCdspGetEdgeQCnt(void);

/**
* @brief	cdsp edge lut table enable
* @param	eluten [in]: edge lut table enable
* @return	none
*/
void gpHalCdspSetEdgeLutTableEn(UINT8 eluten);
UINT8 gpHalCdspGetEdgeLutTableEn(void);

/**
* @brief	cdsp edge lut table init
* @param	pLutEdgeTable [in]: table pointer
* @return	none
*/
void gpHalCdspInitEdgeLut(UINT8 *pLutEdgeTable);

/**
* @brief	cdsp set pre r and b clamp set
* @param	pre_rb_clamp [in]: clamp value
* @return	none
*/
void gpHalCdspSetPreRBClamp(UINT8 pre_rb_clamp);
UINT8 gpHalCdspGetPreRBClamp(void);

/**
* @brief	cdsp color matrix enable
* @param	colcorren [in]: color matrix enable
* @return	none
*/
void gpHalCdspSetColorMatrixEn(UINT8 colcorren);
UINT8 gpHalCdspGetColorMatrixEn(void);

/**
* @brief	cdsp color matrix set parameter
* @param	CMatrix [in]: color matrix parameter
* @return	none
*/
void gpHalCdspSetColorMatrix(color_matrix_t *CMatrix);

/**
* @brief	cdsp color matrix get parameter
* @param	CMatrix [out]: color matrix parameter
* @return	none
*/
void gpHalCdspGetColorMatrix(color_matrix_t *CMatrix);

/**
* @brief	cdsp r and clamp set
* @param	rbclampen [in]: clamp enable
* @param	rbclamp [in]: clamp size set
* @return	none
*/
void gpHalCdspSetRBClamp(UINT8 rbclampen,UINT8 rbclamp);
void gpHalCdspGetRBClamp(UINT8 *rbclampen, UINT8 *rbclamp);

/**
* @brief	cdsp uv division set
* @param	uvDiven [in]: un div function enable
* @return	none
*/
void gpHalCdspSetUvDivideEn(UINT8 uvDiven);
UINT8 gpHalCdspGetUvDivideEn(void);

/**
* @brief	cdsp uv division set
* @param	UVDivide [in]: y value Tn
* @return	none
*/
void gpHalCdspSetUvDivide(uv_divide_t *UVDivide);

/**
* @brief	cdsp uv division get
* @param	UVDivide [out]: y value Tn
* @return	none
*/
void gpHalCdspGetUvDivide(uv_divide_t *UVDivide);

/**
* @brief	cdsp yuv mux path set
* @param	redoedge [in]: Set mux, 0:yuv path, 1:yuv path6
* @return	none
*/
void gpHalCdspSetMuxPath(UINT8 redoedge);

/**
* @brief	cdsp yuv 444 insert enable
* @param	yuvinserten [in]: yuv 444 insert enable
* @return	none
*/
void gpHalCdspSetYuv444InsertEn(UINT8 yuvinserten);
UINT8 gpHalCdspGetYuv444InsertEn(void);

/**
* @brief	cdsp yuv coring threshold value set
* @param	y_corval_coring [in]: y coring threshold value
* @param	u_corval_coring [in]: y coring threshold value
* @param	v_corval_coring [in]: y coring threshold value
* @return	none
*/
void gpHalCdspSetYuvCoring(UINT8 y_corval_coring, UINT8 u_corval_coring, UINT8 v_corval_coring);
void gpHalCdspGetYuvCoring(UINT8 *y_corval_coring, UINT8 *u_corval_coring, UINT8 *v_corval_coring);

/**
* @brief	cdsp h average function
* @param	yuvhavgmiren [in]: mirror enable, bit0:left, bit1: right
* @param	ytype [in]: Y horizontal LPF type, 0:disable, 1:3tap, 2:5tap
* @param	utype [in]: U horizontal LPF type, 0:disable, 1:3tap, 2:5tap
* @param	vtype [in]: V horizontal LPF type, 0:disable, 1:3tap, 2:5tap
* @return	none
*/
void gpHalCdspSetYuvHAvg(UINT8 yuvhavgmiren, UINT8 ytype, UINT8 utype, UINT8 vtype);
void gpHalCdspGetYuvHAvg(UINT8 *yuvhavgmiren, UINT8 *ytype, UINT8 *utype, UINT8 *vtype);

/**
* @brief	cdsp yuv special mode set
* @param	yuvspecmode [in]: yuv special mode
* @return	none
*/
void gpHalCdspSetYuvSpecMode(UINT8 yuvspecmode);
UINT8 gpHalCdspGetYuvSpecMode(void);

/**
* @brief	cdsp yuv special mode Binary threshold set
* @param	binarthr [in]: Binary threshold set
* @return	none
*/
void gpHalCdspSetYuvSpecModeBinThr(UINT8 binarthr);
UINT8 gpHalCdspGetYuvSpecModeBinThr(void);

/**
* @brief	cdsp yuv special mode brightness and contrast adjust enable
* @param	YbYcEn [in]: enable y brightness and contrast adjust 
* @return	none
*/
void gpHalCdspSetBriContEn(UINT8 YbYcEn);
UINT8 gpHalCdspGetBriContEn(void);

/**
* @brief	cdsp yuv special mode offset set
* @param	y_offset [in]: Y offset set 
* @param	u_offset [in]: U offset set 
* @param	v_offset [in]: V offset set 
* @return	none
*/
void gpHalCdspSetYuvSPEffOffset(UINT8 y_offset, UINT8 u_offset, UINT8 v_offset);
void gpHalCdspGetYuvSPEffOffset(UINT8 *y_offset, UINT8 *u_offset, UINT8 *v_offset);

/**
* @brief	cdsp yuv special mode offset set
* @param	y_scale [in]: Y scale set 
* @param	u_scale [in]: U scale set 
* @param	v_scale [in]: V scale set 
* @return	none
*/
void gpHalCdspSetYuvSPEffScale(UINT8 y_scale, UINT8 u_scale, UINT8 v_scale);
void gpHalCdspGetYuvSPEffScale(UINT8 *y_scale, UINT8 *u_scale, UINT8 *v_scale);

/**
* @brief	cdsp yuv special mode hue set
* @param	u_huesindata [in]: sin data for hue rotate for u
* @param	u_huecosdata [in]: cos data for hue rotate for u
* @param	v_huesindata [in]: sin data for hue rotate for v
* @param	v_huecosdata [in]: cos data for hue rotate for v
* @return 	none
*/
void gpHalCdspSetYuvSPHue(UINT8 u_huesindata, UINT8 u_huecosdata, UINT8 v_huesindata, UINT8 v_huecosdata);
void gpHalCdspGetYuvSPHue(UINT8 *u_huesindata, UINT8 *u_huecosdata,	UINT8 *v_huesindata, UINT8 *v_huecosdata);

/**
* @brief	cdsp yuv h scale down enable
* @param	yuvhscale_en [in]: yuv h scale enable
* @param	yuvhscale_mode [in]: yuv h scale skip pixel mode 0: drop, 1:filter
* @return	none
*/
void gpHalCdspSetYuvHScaleEn(UINT8 yuvhscale_en, UINT8 yuvhscale_mode);
void gpHalCdspGetYuvHScaleEn(UINT8 *yuvhscale_en, UINT8 *yuvhscale_mode);

/**
* @brief	cdsp yuv v scale down enable
* @param	vscale_en [in]: yuv v scale enable
* @param	vscale_mode [in]: yuv v scale skip pixel mode 0: drop, 1:filter
* @return	none
*/
void gpHalCdspSetYuvVScaleEn(UINT8 vscale_en, UINT8 vscale_mode);
void gpHalCdspGetYuvVScaleEn(UINT8 *vscale_en, UINT8 *vscale_mode);

/**
* @brief	cdsp yuv h scale down set
* @param	hscaleaccinit [in]: yuv h scale accumation init vale set
* @param	yuvhscalefactor [in]: yuv h scale factor set
* @return	none
*/
void gpHalCdspSetYuvHScale(UINT16 hscaleaccinit, UINT16 yuvhscalefactor);

/**
* @brief	cdsp yuv v scale down set
* @param	vscaleaccinit [in]: yuv v scale accumation init vale set
* @param	yuvvscalefactor [in]: yuv v scale factor set
* @return	none
*/
void gpHalCdspSetYuvVScale(UINT16 vscaleaccinit, UINT16 yuvvscalefactor);

/**
* @brief	cdsp uv suppression enable
* @param	suppressen [in]: uv suppression enable, effective when yuv data input.
* @return	none
*/
void gpHalCdspSetUvSupprEn(UINT8 suppressen);
UINT8 gpHalCdspGetUvSupprEn(void);

/**
* @brief	cdsp uv suppression set
* @param	yuvsupmirvsel [in]: 1:cnt3eq2, 0:cnt3eq1
* @param	fstextsolen [in]: enable first sol when extened 2 line
* @param	yuvsupmiren [in]: suppression enable, bit0:top, bit1:down, bit2:left, bit3:right
* @return	none
*/
void gpHalCdspSetUvSuppr(UINT8 yuvsupmirvsel, UINT8 fstextsolen, UINT8 yuvsupmiren);
void gpHalCdspGetUvSuppr(UINT8 *yuvsupmirvsel, UINT8 *fstextsolen, UINT8 *yuvsupmiren);

/**
* @brief	cdsp y denoise enable
* @param	denoisen [in]: y denoise enable
* @return	none
*/
void gpHalCdspSetYDenoiseEn(UINT8 denoisen);
UINT8 gpHalCdspGetYDenoiseEn(void);

/**
* @brief	cdsp y denoise set
* @param	denoisethrl [in]: y denoise low threshold
* @param	denoisethrwth [in]: y denoise bandwidth set
* @param	yhtdiv [in]: y denoise divider
* @return	none
*/
void gpHalCdspSetYDenoise(UINT8 denoisethrl, UINT8 denoisethrwth, UINT8 yhtdiv);
void gpHalCdspGetYDenoise(UINT8 *denoisethrl, UINT8 *denoisethrwth, UINT8 *yhtdiv);

/**
* @brief	cdsp y LPF enable
* @param	lowyen [in]: y LPF enable
* @return	none
*/
void gpHalCdspSetYLPFEn(UINT8 lowyen);
UINT8 gpHalCdspGetYLPFEn(void);

/**
* @brief	cdsp new denoise enable
* @param	newdenoiseen [in]: new denoise enable, effective when raw data input.
* @return	none
*/
void gpHalCdspSetNewDenoiseEn(UINT8 newdenoiseen);
UINT8 gpHalCdspGetNewDenoiseEn(void);

/**
* @brief	cdsp new denoise set
* @param	ndmirvsel [in]: 1:cnt3eq2, 0:cnt3eq1
* @param	ndmiren [in]: new denoise enable, bit0:top, bit1:down, bit2:left, bit3:right
* @return	none
*/
void gpHalCdspSetNewDenoise(UINT8 ndmirvsel, UINT8 ndmiren);
void gpHalCdspGetNewDenoise(UINT8 *ndmirvsel, UINT8 *ndmiren);

/**
* @brief	cdsp new denoise edge enable
* @param	ndedgeen [in]: new denoise edge enable
* @param	ndeluten [in]: new denoise edge lut enable
* @return	none
*/
void gpHalCdspSetNdEdgeEn(UINT8 ndedgeen, UINT8 ndeluten);
void gpHalCdspGetNdEdgeEn(UINT8 *ndedgeen, UINT8 *ndeluten);

/**
* @brief	cdsp new denoise edge HPF matrix set
* @param	NDEdgeFilter [in]: 
* @return	none
*/
void gpHalCdspSetNdEdgeFilter(edge_filter_t *NDEdgeFilter);

/**
* @brief	cdsp new denoise edge HPF matrix get
* @param	NDEdgeFilter [out]: 
* @return	none
*/
void gpHalCdspGetNdEdgeFilter(edge_filter_t *NDEdgeFilter);

/**
* @brief	cdsp new denoise edge scale set
* @param	ndlhdiv [in]: L edge enhancement edge vale scale
* @param	ndlhtdiv [in]: L edge enhancement edge vale scale
* @param	ndlhcoring [in]: L core ring threshold
* @param	ndlhmode [in]: 1: default matrix, 0: enable paroramming matrix
* @return	none
*/
void gpHalCdspSetNdEdgeLCoring(UINT8 ndlhdiv, UINT8 ndlhtdiv, UINT8 ndlhcoring, UINT8 ndlhmode);
void gpHalCdspGetNdEdgeLCoring(UINT8 *ndlhdiv, UINT8 *ndlhtdiv,	UINT8 *ndlhcoring, UINT8 *ndlhmode);

/**
* @brief	cdsp new denoise edge amp set
* @param	ndampga [in]: 0:1, 1:2, 2:3, 3:4
* @return	none
*/
void gpHalCdspSetNdEdgeAmpga(UINT8 ndampga);
UINT8 gpHalCdspGetNdEdgeAmpga(void);

/**
* @brief	cdsp wb gain2 enable
* @param	wbgain2en [in]: enable 
* @return	none
*/
void gpHalCdspSetWbGain2En(UINT8 wbgain2en);
UINT8 gpHalCdspGetWbGain2En(void);

/**
* @brief	cdsp wb gain2 set
* @param	rgain2 [in]: R gain
* @param	ggain2 [in]: G gain
* @param	bgain2 [in]: B gain
* @return	none
*/
void gpHalCdspSetWbGain2(UINT16 rgain2, UINT16 ggain2, UINT16 bgain2);
void gpHalCdspGetWbGain2(UINT16 *rgain2, UINT16 *ggain2, UINT16 *bgain2);

/**
* @brief	cdsp auto focus enable
* @param	af_en [in]: af enable
* @param	af_win_hold [in]: af hold
* @return	none
*/
void gpHalCdspSetAFEn(UINT8 af_en, UINT8 af_win_hold);
void gpHalCdspGetAFEn(UINT8 *af_en, UINT8 *af_win_hold);

/**
* @brief	cdsp auto focus window 1 set
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size
* @param	vsize [in]: v size
* @return	none
*/
void gpHalCdspSetAfWin1(UINT16 hoffset, UINT16 voffset, UINT16 hsize, UINT16 vsize);
void gpHalCdspGetAfWin1(UINT16 *hoffset, UINT16 *voffset, UINT16 *hsize, UINT16 *vsize);

/**
* @brief	cdsp auto focus window 2 set
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size, 256, 512, 1024, 64, 2048
* @param	vsize [in]: v size, 256, 512, 1024, 64, 2048
* @return	none
*/
void gpHalCdspSetAfWin2(UINT16 hoffset,UINT16 voffset,UINT16 hsize,UINT16 vsize);
void gpHalCdspGetAfWin2(UINT16 *hoffset, UINT16 *voffset, UINT16 *hsize, UINT16 *vsize);

/**
* @brief	cdsp auto focus window 3 set
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size, 256, 512, 1024, 64, 2048
* @param	vsize [in]: v size, 256, 512, 1024, 64, 2048
* @return	none
*/
void gpHalCdspSetAfWin3(UINT16 hoffset,UINT16 voffset,UINT16 hsize,UINT16 vsize);
void gpHalCdspGetAfWin3(UINT16 *hoffset, UINT16 *voffset, UINT16 *hsize, UINT16 *vsize);

/**
* @brief	cdsp auto white balance enable
* @param	awb_en [in]: awb enable
* @param	awb_win_hold [in]: awb hold
* @return	none
*/
void gpHalCdspSetAWBEn(UINT8 awb_en, UINT8 awb_win_hold);
void gpHalCdspGetAWBEn(UINT8 *awb_en, UINT8 *awb_win_hold);

/**
* @brief	cdsp auto white balance set
* @param	awbclamp_en [in]: awb special window clamp enable.
* @param	sindata [in]: sin data for AWB
* @param	cosdata [in]: cos data for AWB
* @param	awbwinthr [in]: AWB winwow accumulation threshold
* @return	none
*/
void gpHalCdspSetAWB(UINT8 awbclamp_en, SINT8 sindata, SINT8 cosdata, UINT8 awbwinthr);
void gpHalCdspGetAWB(UINT8 *awbclamp_en, SINT8 *sindata, SINT8 *cosdata, UINT8 *awbwinthr);

/**
* @brief	cdsp awb special windows Y threshold set
* @param	Ythr0 [in]: AWB Y threshold0
* @param	Ythr1 [in]: AWB Y threshold1
* @param	Ythr2 [in]: AWB Y threshold2
* @param	Ythr3 [in]: AWB Y threshold3
* @return	none
*/
void gpHalCdspSetAwbYThr(UINT8 Ythr0, UINT8 Ythr1, UINT8 Ythr2, UINT8 Ythr3);
void gpHalCdspGetAwbYThr(UINT8 *Ythr0, UINT8 *Ythr1, UINT8 *Ythr2, UINT8 *Ythr3);

/**
* @brief	cdsp awb special windows uv threshold set
* @param	UVthr [in]: AWB UV threshold
* @return	none
*/
void gpHalCdspSetAwbUVThr(awb_uv_thr_t *UVthr);

/**
* @brief	cdsp awb special windows uv threshold get
* @param	UVthr [out]: AWB UV threshold
* @return	none
*/
void gpHalCdspGetAwbUVThr(awb_uv_thr_t *UVthr);

/**
* @brief	cdsp ae/awb source set
* @param	raw_en [in]: ae/awb windows set, 0:from poswb, 1:form awb line ctrl
* @return	none
*/
void gpHalCdspSetAeAwbSrc(UINT8 raw_en);
UINT32 gpHalCdspGetAeAwbSrc(void);

/**
* @brief	cdsp ae/awb subsample set
* @param	subample [in]: 0:disable, 2:1/2, 4:1/4 subsample
* @return	none
*/
void gpHalCdspSetAeAwbSubSample(UINT8 subample);
UINT8 gpHalCdspGetAeAwbSubSample(void);

/**
* @brief	cdsp auto expore enable
* @param	ae_en [in]: ae enable
* @param	ae_win_hold [in]: ae hold
* @return	none
*/
void gpHalCdspSetAEEn(UINT8 ae_en, UINT8 ae_win_hold);
void gpHalCdspGetAEEn(UINT8 *ae_en, UINT8 *ae_win_hold);

/**
* @brief	cdsp auto expore set
* @param	phaccfactor [in]: pseudo h window size for ae windows
* @param	pvaccfactor [in]: pseudo v window size for ae windows
* @return	none
*/
void gpHalCdspSetAEWin(UINT8 phaccfactor, UINT8 pvaccfactor);
void gpHalCdspGetAEWin(UINT8 *phaccfactor, UINT8 *pvaccfactor);

/**
* @brief	cdsp ae buffer address set
* @param	winaddra [in]: AE a buffer address set 
* @param	winaddrb [in]: AE b buffer address set
* @return	none
*/
void gpHalCdspSetAEBuffAddr(UINT32 winaddra, UINT32 winaddrb);

/**
* @brief	cdsp get ae active buffer
* @return	0: buffer a, 1: buffer b
*/
UINT32 gpHalCdspGetAEActBuff(void);

/**
* @brief	cdsp rgb window set
* @param	hwdoffset [in]: rgb window h offset
* @param	vwdoffset [in]: rgb window v offset
* @param	hwdsize [in]: rgb window h size
* @param	vwdsize [in]: rgb window v size
* @return	none
*/
void gpHalCdspSetRGBWin(UINT16 hwdoffset,UINT16 vwdoffset,UINT16 hwdsize,UINT16 vwdsize);
void gpHalCdspGetRGBWin(UINT16 *hwdoffset, UINT16 *vwdoffset, UINT16 *hwdsize, UINT16 *vwdsize);

/**
* @brief	cdsp ae/af test windows enable
* @param	AeWinTest [in]: ae test window enable
* @param	AfWinTest [in]: af test window enable
* @return	none
*/
void gpHalCdspSet3ATestWinEn(UINT8 AeWinTest, UINT8 AfWinTest);
void gpHalCdspGet3ATestWinEn(UINT8 *AeWinTest, UINT8 *AfWinTest);

/**
* @brief	cdsp histgm enable
* @param	his_en [in]: histgm enable
* @param	his_hold_en [in]: histgm hold
* @return	none
*/
void gpHalCdspSetHistgmEn(UINT8 his_en, UINT8 his_hold_en);
void gpHalCdspGetHistgmEn(UINT8 *his_en, UINT8 *his_hold_en);

/**
* @brief	cdsp histgm statistics set
* @param	hislowthr [in]: histgm low threshold set
* @param	hishighthr [in]: histgm high threshold set
* @return	none
*/
void gpHalCdspSetHistgm(UINT8 hislowthr, UINT8 hishighthr);
void gpHalCdspGetHistgm(UINT8 *hislowthr, UINT8 *hishighthr);
void gpHalCdspGetHistgmCount(UINT32 *hislowcnt, UINT32 *hishighcnt);

/**
* @brief	cdsp get awb cnt
* @param	section [in]: index = 1, 2, 3
* @param	sumcnt [out]: count get
* @return	SUCCESS/ERROR
*/
SINT32 gpHalCdspGetAwbSumCnt(UINT8 section, UINT32 *sumcnt);

/**
* @brief	cdsp get awb g
* @param	section [in]: section = 1, 2, 3
* @param	sumgl [out]: sum g1 low 
* @param	sumgl [out]: sum g1 high 
* @return	SUCCESS/ERROR
*/
SINT32 gpHalCdspGetAwbSumG(UINT8 section,UINT32 *sumgl,UINT32 *sumgh);

/**
* @brief	cdsp get awb rg
* @param	section [in]: section = 1, 2, 3
* @param	sumrgl [out]: sum rg low 
* @param	sumrgl [out]: sum rg high 
* @return	SUCCESS/ERROR
*/
SINT32 gpHalCdspGetAwbSumRG(UINT8 section,UINT32 *sumrgl,SINT32 *sumrgh);

/**
* @brief	cdsp get awb bg
* @param	section [in]: section = 1, 2, 3
* @param	sumbgl [out]: sum bg low 
* @param	sumbgl [out]: sum bg high 
* @return	SUCCESS/ERROR
*/
SINT32 gpHalCdspGetAwbSumBG(UINT8 section, UINT32 *sumbgl, SINT32 *sumbgh);

/**
* @brief	cdsp get af window1 statistics
* @param	windows_no[in]: window index number
* @param	af_value[out]: 
* @return	SUCCESS/ERROR
*/
SINT32 gpHalCdspGetAFWinVlaue(UINT8 windows_no, af_windows_value_t *af_value);

/**
* @brief	cdsp raw data path set
* @param	raw_mode [in]: raw data path set, 0:disable, 1:RGB_Path1, 3:RGB_Path2, 5:RGB_Path3 
* @param	cap_mode [in]: set 0:raw8, 1:raw10 
* @param	yuv_mode [in]: set 0:8y4u4v, 1:yuyv set
* @param	yuv_fmt  [in]: set out sequence, 0:YUYV, 1:UYVY, 2:YVYU, 3:VYUY
* @return	none
*/
void gpHalCdspSetRawPath(UINT8 raw_mode, UINT8 cap_mode, UINT8 yuv_mode, UINT8 yuv_fmt);

/**
* @brief	cdsp sram fifo threshold
* @param	overflowen [in]: overflow enable
* @param	sramthd [in]: sram threshold
* @return	none
*/
void gpHalCdspSetSRAM(UINT8 overflowen,UINT16 sramthd);

/**
* @brief	cdsp dma clamp size set
* @param	clamphsizeen [in]: clamp enable
* @param	Clamphsize [in]: clamp size set
* @return	none
*/
void gpHalCdspSetClampEn(UINT8 clamphsizeen,UINT16 Clamphsize);

/**
* @brief	cdsp line interval set
* @param	line_interval [in]: line number
* @return	none
*/
void gpHalCdspSetLineInterval(UINT16 line_interval);

/**
* @brief	cdsp dma yuv buffer a set
* @param	width [in]: dma buffer width
* @param	height [in]: dma buffer height
* @param	buffe_addr [in]: dma buffer address
* @return	none
*/
void gpHalCdspSetYuvBuffA(UINT16 width,UINT16 height,UINT32 buffer_addr);

/**
* @brief	cdsp get dma yuv buffer size
* @param	width [out]: dma buffer width
* @param	height [out]: dma buffer height
* @return	none
*/
void gpHalCdspGetYuvBuffASize(UINT16 *width,UINT16 *height);

/**
* @brief	cdsp dma yuv buffer b set
* @param	width [in]: dma buffer width
* @param	height [in]: dma buffer height
* @param	buffe_addr [in]: dma buffer address
* @return	none
*/
void gpHalCdspSetYuvBuffB(UINT16 width,UINT16 height,UINT32 buffer_addr);

/**
* @brief	cdsp get dma yuv buffer size
* @param	width [out]: dma buffer width
* @param	height [out]: dma buffer height
* @return	none
*/
void gpHalCdspGetYuvBuffBSize(UINT16 *width,UINT16 *height);

/**
* @brief	cdsp dma raw buffer size set
* @param	width [in]: dma buffer width
* @param	height [in]: dma buffer height
* @param	hoffset [in]: dma buffer h offset
* @param	raw_bit [in]: raw data bits
* @return	none
*/
void gpHalCdspSetRawBuffSize(UINT16 width, UINT16 height, UINT32 hoffset, UINT32 raw_bit);

/**
* @brief	cdsp dma raw buffer set
* @param	buffe_addr [in]: dma buffer address
* @return	none
*/
void gpHalCdspSetRawBuff(UINT32 buffer_addr);

/**
* @brief	cdsp dma yuv buffer mode set
* @param	buffer_mode [in]: dma buffer mode
* @return	none
*/
void gpHalCdspSetDmaBuff(UINT8 buffer_mode);

/**
* @brief	cdsp read back size set
* @param	hoffset [in]: read back h offset
* @param	voffset [in]: read back v offset
* @param	hsize [in]: read back h size
* @param	vsize [in]: read back v size
* @return	none
*/
void gpHalCdspSetReadBackSize(UINT16 hoffset,UINT16 voffset,UINT16 hsize,UINT16 vsize);

/**
* @brief	cdsp write register
* @param 	reg[in]: register address
* @param 	value[in]: register value
* @return 	SUCCESS/ERROR
*/
UINT8 gpHalCdspWriteReg(UINT32 reg, UINT8 value);

/**
* @brief	cdsp read register
* @param 	reg[in]: register address
* @param 	value[in]: register value
* @return 	SUCCESS/ERROR
*/
UINT8 gpHalCdspReadReg(UINT32 reg, UINT8 *value);
#endif


