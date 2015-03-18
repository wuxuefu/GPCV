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
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/
#ifndef _REG_DISP2_H_
#define _REG_DISP2_H_

#include <mach/hardware.h>
#include <mach/typedef.h>

#define	LOGI_ADDR_DISP2_REG		      (IO3_BASE + 0x20000)
#define	LOGI_ADDR_SYSTEM_BASE_REG		(IO3_BASE + 0x7000)
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct disp2Reg_s {
	volatile UINT32 rsv0000[60];	    /* offset00, 0x93020000 ~ 0x930200F0*/
	volatile UINT32 disp2Ctrl;		    /* P_TV1_CTRL, 0x930200F0 */
	volatile UINT32 disp2Ctrl2;		    /* P_TV1_CTRL2, 0x930200F4 */	
	volatile UINT32 rsv00f8[38];        /* offset01, 0x930200F8 ~ 0x93020188*/ 
	volatile UINT32 rsv0190[20];        /* offset02, 0x93020190 ~ 0x930201E0*/	
	volatile UINT32 disp2Addr;	        /* P_TV1_FBADDR, 0x930201E0 */
	volatile UINT32 rsv01e4[7];         /* offset03, 0x930201E4 ~ 0x930201F8*/  
	volatile UINT32 disp2Saturation;	/* P_TV1_SATURATION, 0x93020200 */
	volatile UINT32 disp2Hue;	        /* P_TV1_HUE, 0x93020204 */
	volatile UINT32 disp2Brightness;	/* P_TV1_BRIGHTNESS, 0x93020208 */
	volatile UINT32 disp2Sharpness;	    /* P_TV1_SHARPNESS, 0x93020020C */
	volatile UINT32 disp2YGain;		    /* P_TV1_Y_GAIN, 0x93020210 */
	volatile UINT32 disp2YDelay;	    /* P_TV1_Y_DELAY, 0x93020214 */
	volatile UINT32 disp2VPosition;		/* P_TV1_V_POSITION, 0x93020218 */
	volatile UINT32 disp2HPosition;		/* P_TV1_H_POSITION, 0x9302021C */
	volatile UINT32 disp2Videodac;	    /* P_TV1_VIDEODAC, 0x93020220 */
	volatile UINT32 rsv0224[1];         /* offset04, 0x93020224 ~ 0x93020228*/
	volatile UINT32 disp2Ccmoffse;    /* P_TV1_COM_C_OFFSET, 0x93020228*/
	volatile UINT32 rsv022C[80];        /* offset04, 0x9302022C ~ 0x9302036C*/
	volatile UINT32 disp2Cotrl3;		/* P_TV1_CTRL4, 0x9302036C */
} disp2Reg_t;

typedef struct gpSysReg_s {
	volatile  UINT8 offset00[0x0004];	         /* offset00, 0x93007000 ~ 0x93007004*/
	volatile UINT32 gpTvSysCotrl0;		         /* P_TV1_SYS_CTRL0, 0x93007004 */
	volatile  UINT8 offset01[0x0008];	         /* offset01, 0x93007008 ~ 0x93007010*/
	volatile UINT32 gpTvEnable2;		           /* P_TV1_ENABLE2, 0x93007010 */	
  volatile  UINT8 offset02[0x0004];          /* offset02, 0x93007014 ~ 0x93007018*/ 
  volatile UINT32 gpTvSysCotrl1;	           /* P_TV1_SYS_CTRL1, 0x93007018 */
  volatile  UINT8 offset03[0x0024];          /* offset03, 0x9300701C ~ 0x93007040*/ 
  volatile UINT32 gpTvEnable1;		           /* P_TV1_ENABLE1, 0x93007040 */
  volatile  UINT8 offset04[0x009C];          /* offset04, 0x93007044 ~ 0x930070E0*/ 
  volatile UINT32 gpTvEnable0;		           /* P_TV1_ENABLE0, 0x930070E0 */	  	
} gpSysReg_t;

#endif /* _REG_DISP2_H_ */

