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

#ifndef _REG_SCALE_H_
#define _REG_SCALE_H_

#include <mach/hardware.h>
#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define LOGI_ADDR_SCALE0_REG     IO3_ADDRESS(0x1000)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct Scale0Reg_s {
	volatile UINT32 Scale0SrcYAddr;     /* 0x0000 ~ 0x0003 Source Address Y  */
	volatile UINT32 Scale0SrcCbAddr;    /* 0x0004 ~ 0x0007 Source Address Cb */
	volatile UINT32 Scale0SrcCrAddr;    /* 0x0008 ~ 0x000B Source Address Cr */
	volatile UINT32 Scale0SrcXYWidth;   /* 0x000C ~ 0x000F Source Image width/height */

	volatile UINT32 Scale0ActOffset;    /* 0x0010 ~ 0x0013 active x/y coordinate */
	volatile UINT32 Scale0ActWidth;     /* 0x0014 ~ 0x0017 active width */
	volatile UINT32 Scale0BGAddr;  		/* 0x0018 ~ 0x001B output frame buffer*/
	volatile UINT32 Scale0DstXYWidth;   /* 0x001C ~ 0x001F dstination width/height */

	volatile UINT32 ScaleOHInitValue;   /* 0x0020 ~ 0x0023 Horizontal init value */
	volatile UINT32 Scale0HFactor;      /* 0x0024 ~ 0x0027 Horizontal scale factor */
	volatile UINT32 ScaleOVInitValue;   /* 0x0028 ~ 0x002B vertical init value */
	volatile UINT32 Scale0VFactor;      /* 0x002C ~ 0x002F vertical scale factor */

	volatile UINT32 Scale0CSM_10_00;    /* 0x0030 ~ 0x0033 color space matrix */
	volatile UINT32 Scale0CSM_30_20;    /* 0x0034 ~ 0x0037 color space matrix */
	volatile UINT32 Scale0CSM_11_01;    /* 0x0038 ~ 0x003B color space matrix */
	volatile UINT32 Scale0CSM_31_21;    /* 0x003C ~ 0x003F color space matrix */

	volatile UINT32 Scale0CSM_12_02;	/* 0x0040 ~ 0x0043 color space matrix */
	volatile UINT32 Scale0CSM_32_22;    /* 0x0044 ~ 0x0047 color space matrix */
	volatile UINT32 Scale0ChormaThr;    /* 0x0048 ~ 0x004B chorma threshold */
	volatile UINT32 Scale0Arbit;    	/* 0x004C ~ 0x004F scale arbit */

	volatile UINT32 Scale0Ctrl;         /* 0x0050 ~ 0x0053 scale ctrl */
	volatile UINT32 Scale0Status;       /* 0x0054 ~ 0x0057 scale status */
	volatile UINT32 Scale0ID0;       	/* 0x0058 ~ 0x005B scale ID */
	volatile UINT32 Scale0ID1;       	/* 0x005C ~ 0x005F scale ID */	

	volatile UINT32 reserved[8];       	/* 0x0060 ~ 0x0080 */

	volatile UINT32 Scale0BGXYWidth;    /* 0x0080 ~ 0x0083 background width/height */	
	volatile UINT32 Scale0DstXYOffset;  /* 0x0084 ~ 0x0087 dstination X/Y Offset */	
	volatile UINT32 Scale0BoundaryColor;/* 0x0088 ~ 0x008B boundary color */		
} Scale0Reg_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif /* _REG_SCALE_H_ */
