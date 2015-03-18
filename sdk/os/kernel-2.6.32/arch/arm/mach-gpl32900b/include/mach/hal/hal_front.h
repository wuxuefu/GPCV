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

#ifndef _HAL_FRONT_H
#define _HAL_FRONT_H

#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gpHalFrontRoi_s
 {
	UINT16 hSize;
	UINT16 hOffset;
	UINT16 vSize;
	UINT16 vOffset;
} gpHalFrontRoi_t;

typedef struct gpHalFrontReshape_s
 {
	UINT32 mode; // 0: RAW ; 1: YUV ; 2:MIPI
	UINT8 hReshapeEn;
	UINT8 vReshapeEn;
	UINT8 vReshapeClkType; // 0: H-sync ; 1: pclk
	UINT8 vBackOffEn;
	UINT16 hRise;
	UINT16 hFall;
	UINT16 vRise;
	UINT16 vFall;
} gpHalFrontReshape_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
void gpHalFrontReset(void);
void gpHalFrontStart(void);
void gpHalFrontStop(void);

void gpHalFrontInputPathSet(UINT8 path);
void gpHalFrontInputModeSet(UINT8 mode);

void gpHalFrontYuvSubEn(UINT32 ySub, UINT32 uSub, UINT32 vSub);
void gpHalFrontDataTransMode(UINT8 type);
UINT8 gpHalFrontDataTransModeGet(void);

void gpHalFrontYuvOrderSet(UINT32 mode);
void gpHalFrontSyncPolaritySet(UINT8 hPol, UINT8 vPol, UINT8 in1den);
void gpHalFrontSyncPolarityGet(UINT8 *hPol, UINT8 *vPol, UINT8 *in1den);

void gpHalFrontInputGate(UINT32 tggz);
void gpHalFrontIntCfg(UINT32 interrupten, UINT32 vdrintnum, UINT32 vdfintnum, UINT32 hdfintnum);
void gpHalFrontIntEventClr(UINT32 clrintevt);
void gpHalFrontIntEventGet(UINT32 *pintevtvalue);

void gpHalFrontGpioInterruptEn(UINT32 intriseen, UINT32 intfallen);
void gpHalFrontGpioInterruptClr(UINT32 clrriseevt, UINT32 clrfallevt);
void gpHalFrontGpioInterruptGet(UINT32 *priseread, UINT32 *pfallread);

void gpHalFrontSnapTrigger(UINT32 number);
void gpHalFrontFlashSet(UINT32 width, UINT32 linenum, UINT32 mode);
void gpHalFrontFlashTrigger(void);

void gpHalFrontRoiSet(gpHalFrontRoi_t roi, UINT8 syncVdEn);
void gpHalFrontRoiGet(gpHalFrontRoi_t* roi);

void gpHalFrontVdUpdateEn(UINT32 enable);
void gpHalFrontReshapeSet(gpHalFrontReshape_t reshapeCtl);
void gpHalFrontSiggenSet(UINT32 enable, UINT32 type, UINT32 mode);
void gpHalFrontVdWait(UINT32 mode, UINT32 number);
void gpHalFrontProbeSet(UINT32 enable, UINT32 mode);

#if 0
/**
* @brief	cdsp sensor interface reset
* @return	none
*/
void gpHalCdspFrontReset(void);

/**
* @brief	cdsp set front sensor input format
* @param	format [in]: input format
* @return	0: success, other: fail
*/
SINT32 gpHalCdspFrontSetInputFormat(UINT32 format);

/**
* @brief	cdsp set front sensor input size
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size
* @param	vsize [in]: v size
* @return	none
*/
void gpHalCdspFrontSetFrameSize(UINT32 hoffset,UINT32 voffset,UINT32 hsize,UINT32 vsize);

/**
* @brief	cdsp set front harizontal reshape
* @param	HReshEn [in]: h reshape enable
* @param	Hrise [in]: h rise size
* @param	Hfall [in]: h fall size
* @return	none
*/
void gpHalCdspFrontSetHReshape(UINT32 HReshEn, UINT32 Hrise, UINT32 Hfall);
void gpHalCdspFrontSetVReshape(UINT32 VReshEn, UINT32 Vrise, UINT32 Vfall);

/**
* @brief	cdsp set front h/v polarity
* @param	CCIR656En [in]: 0: CCIR601, 1: CCIR656
* @param	hsync_act [in]: 0: HACT, 1: LACT
* @param	vsync_act [in]: 0: HACT, 1: LACT
* @param	sync_en [in]: 0: disable, 1: enable
* @return	none
*/
void gpHalCdspFrontSetInterface(UINT32 CCIR656En, UINT32 hsync_act, UINT32 vsync_act, UINT32 sync_en);
void gpHalCdspFrontGetInterface(UINT8 *CCIR656En, UINT8 *hsync_act, UINT8 *vsync_act, UINT8 *sync_en);

/**
* @brief	cdsp set interlace and field
* @param	field [in]: 0: field 0, 1: field1
* @param	interlace [in]: 0: non-interlace, 1: interlace
* @return	none
*/
void gpHalCdspFrontSetInterlace(UINT32 field, UINT32 interlace);

/**
* @brief	cdsp set front mipi sensor input size
* @param	hoffset [out]: h offset
* @param	voffset [out]: v offset
* @param	hsize [out]: h size
* @param	vsize [out]: v size
* @return	none
*/
void gpHalCdspFrontSetMipiFrameSize(UINT32 hoffset,UINT32 voffset,UINT32 hsize,UINT32 vsize);
#endif

#endif

