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

#ifndef _REG_LBP_H_
#define _REG_LBP_H_

#include <mach/hardware.h>
#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define LOGI_ADDR_LBP_REG     IO3_ADDRESS(0x3000)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct LbpReg_s {
	volatile UINT32 LbpCtrl;			/* 0x0000 ~ 0x0003 LBP control register  */
	volatile UINT32 LbpStatus;			/* 0x0004 ~ 0x0007 LBP control status  */
	volatile UINT32 ImgStrAddr;			/* 0x0008 ~ 0x000B Source image start address  */
	volatile UINT32 ImgSize;			/* 0x000C ~ 0x000F Source image size  */
	volatile UINT32 LbpGenOBAddr;		/* 0x0010 ~ 0x0013 LBP generator output buffer address   */
	volatile UINT32 LbpCmpOBAddr;		/* 0x0014 ~ 0x0017 LBP compare output buffer address  */
	volatile UINT32 LbpOCodeAddr;		/* 0x0018 ~ 0x001B LBP old code start address  */
	volatile UINT32 LbpNCodeAddr;		/* 0x001C ~ 0x001F LBP new code start address  */
	volatile UINT32 LbpImgSize;			/* 0x0020 ~ 0x0023 LBP original image size  */
	volatile UINT32 LbpBlkSize;			/* 0x0024 ~ 0x0027 LBP comapre block size  */
	volatile UINT32 TotalSum;			/* 0x0028 ~ 0x002B total summation of LBP comapre block  */
} LbpReg_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif /* _REG_LBP_H_ */
