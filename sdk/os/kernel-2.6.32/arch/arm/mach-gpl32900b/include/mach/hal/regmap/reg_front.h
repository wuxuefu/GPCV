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
 
#ifndef _REG_FRONT_H_
#define _REG_FRONT_H_

#include <mach/hardware.h>
#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define	LOGI_ADDR_FRONT_REG		IO2_ADDRESS(0x14a00)

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct frontReg_s 
{
	// REGISTER ADDRESS
	volatile UINT8 frontCapInterval;	//0xa00
	volatile UINT8 frontVdUpdate;		//0xa01
	volatile UINT8 rsvA02[2];			//0xa02
	volatile UINT8 frontFlashCtrl;		//0xa04
	volatile UINT8 rsvA05;				//0xa05
	volatile UINT8 frontFlashWidthL;	//0xa06
	volatile UINT8 frontFlashWidthH;	//0xa07
	volatile UINT8 frontFlashDelayL;	//0xa08
	volatile UINT8 frontFlashDelayH;	//0xa09
	volatile UINT8 rsvA0a[2];			//0xa0a
	volatile UINT8 frontFieldSel;		//0xa0c
	volatile UINT8 frontFieldNum;		//0xa0d
	volatile UINT8 rsvA0e[2];			//0xa0e
	volatile UINT8 frontSnapCtrl;		//0xa10
	volatile UINT8 rsvA11[3];			//0xa11
	volatile UINT8 frontFramePixelCntLL;//0xa14
	volatile UINT8 frontFramePixelCntLH;//0xa15
	volatile UINT8 frontFramePixelCntHL;//0xa16
	volatile UINT8 frontFramePixelCntHH;//0xa17
	volatile UINT8 rsvA18[3];			//0xa18
	volatile UINT8 frontValidCtrl;		//0xa1b
	volatile UINT8 frontSyncPolarity;	//0xa1c
	volatile UINT8 rsvA1d[3];			//0xa1d
	volatile UINT8 frontReshapeHRiseL;	//0xa20
	volatile UINT8 frontReshapeHRiseH;	//0xa21
	volatile UINT8 frontReshapeHFallL;	//0xa22
	volatile UINT8 frontReshapeHFallH;	//0xa23
	volatile UINT8 frontReshapeVRiseL;	//0xa24
	volatile UINT8 frontReshapeVRiseH;	//0xa25
	volatile UINT8 frontReshapeVFallL;	//0xa26
	volatile UINT8 frontReshapeVFallH;	//0xa27
	volatile UINT8 frontReshapeEnable;	//0xa28
	volatile UINT8 rsvA29[7];			//0xa29
	volatile UINT8 frontHSizeL;			//0xa30
	volatile UINT8 frontHSizeH;			//0xa31
	volatile UINT8 frontHOffsetL;		//0xa32
	volatile UINT8 frontHOffsetH;		//0xa33
	volatile UINT8 frontVSizeL;			//0xa34
	volatile UINT8 frontVSizeH;			//0xa35
	volatile UINT8 frontVOffsetL;		//0xa36
	volatile UINT8 frontVOffsetH;		//0xa37
	volatile UINT8 frontHVSyncVd;		//0xa38
	volatile UINT8 rsvA39[7];			//0xa39
	volatile UINT8 frontLineTotalL;		//0xa40
	volatile UINT8 frontLineTotalH;		//0xa41
	volatile UINT8 frontLineBlankL;		//0xa42
	volatile UINT8 frontLineBlankH;		//0xa43
	volatile UINT8 frontFrameTotalL;	//0xa44
	volatile UINT8 frontFrameTotalH;	//0xa45
	volatile UINT8 frontFrameBlankL;	//0xa46
	volatile UINT8 frontFrameBlankH;	//0xa47
	volatile UINT8 frontDhdvdoposL;		//0xa48
	volatile UINT8 frontDhdvdoposH;		//0xa49
	volatile UINT8 rsvA4a[2];			//0xa4a
	volatile UINT8 frontSyncGen;		//0xa4c
	volatile UINT8 rsvA4d;				//0xa4d
	volatile UINT8 frontDummyLineL;		//0xa4e
	volatile UINT8 frontDummyLineH;		//0xa4f
	volatile UINT8 frontYuvCtrl;		//0xa50
	volatile UINT8 frontYuvMode;		//0xa51
	volatile UINT8 rsvA52[10];			//0xa52
	volatile UINT8 frontTgSignalGating;	//0xa5c
	volatile UINT8 rsvA5d[51];			//0xa5d
	volatile UINT8 frontMipiEnable;		//0xa90
	volatile UINT8 frontMipiHOffsetL;	//0xa91
	volatile UINT8 frontMipiHOffsetH;	//0xa92
	volatile UINT8 frontMipiVOffsetL;	//0xa93
	volatile UINT8 frontMipiVOffsetH;	//0xa94
	volatile UINT8 frontMipiVSizeL;		//0xa95
	volatile UINT8 frontMipiVSizeH;		//0xa96
	volatile UINT8 rsvA97[9];			//0xa97
	volatile UINT8 frontMipiHSizeL;		//0xaa0
	volatile UINT8 frontMipiHSizeH;		//0xaa1
	volatile UINT8 rsvAa2[14];			//0xaa2
	volatile UINT8 frontReset;			//0xab0
	volatile UINT8 rsvAb1[3];			//0xab1
	volatile UINT8 frontClkGatingDisable;	//0xab4
	volatile UINT8 front2xckGatingDisable;	//0xab5
	volatile UINT8 rsvAb6[2];			//0xab6
	volatile UINT8 frontSyncSignalState;//0xab8
	volatile UINT8 rsvAb9[7];			//0xab9
	volatile UINT8 frontIntEvent;		//0xac0
	volatile UINT8 rsvAc1[3];			//0xac1
	volatile UINT8 frontVdIntNumSet;	//0xac4
	volatile UINT8 rsvAc5;				//0xac5
	volatile UINT8 frontHdIntNumSetL;	//0xac6
	volatile UINT8 frontHdIntNumSetH;	//0xac7
	volatile UINT8 rsvAc8[8];			//0xac8
	volatile UINT8 frontIntEnable;		//0xad0
	volatile UINT8 frontVdIntEvent;		//0xad1
	volatile UINT8 frontVdIntEnable;	//0xad2
	volatile UINT8 rsvAd3[13];			//0xad3
	volatile UINT8 frontSignalGen;		//0xae0
	volatile UINT8 rsvAe1[3];			//0xae1
	volatile UINT8 frontProbeMode;		//0xae4
	volatile UINT8 frontProbeSub;		//0xac5
}frontReg_t;



#endif /* _REG_FRONT_H_ */

