/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2014 by Generalplus Inc.                         *
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
#ifndef _HAL_LBP_H_
#define _HAL_LBP_H_

#include <mach/hal/hal_common.h>
#include <mach/gp_lbp.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

SINT32 gpHalLBPGenMode(UINT32 inImg, UINT32 outImg, SINT16 width, SINT16 height, 
						SINT16 threshold, SINT32 format_mode);
SINT32 gpHalLBPAutoMode(UINT32 inImg, UINT32 outImg, UINT32 HammImg, 
						SINT16 width, SINT16 height, 
						SINT16 threshold, SINT32 format_mode);
SINT32 gpHalLBPCmpMode(UINT32 srcImg, UINT32 dstImg, SINT32 width, SINT32 height, 
						UINT32 stride, SINT32 format_mode, 
						gp_rect_t srcRoi, gp_rect_t dstRoi);
SINT32 gpHalLBPGetTotalSum(void);
SINT32 gpHalLBPGetIRQStatus(void);

#endif /* _HAL_SCALE_H_ */
