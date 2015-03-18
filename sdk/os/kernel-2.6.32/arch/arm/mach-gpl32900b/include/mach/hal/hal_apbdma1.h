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
#ifndef _HAL_APBDMA1_H_
#define _HAL_APBDMA1_H_
 
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

void gpHalApbdma1ClkEn(int enable);
void gpHalApbdma1Rst(int ch);
void gpHalApbdma1ClearIRQFlag(int ch);
void gpHalApbdma1SetBuf(int ch, int buf_num, char* addr, int ln);
int gpHalApbdma1BufStatus(int ch);
void gpHalApbdma1En(int ch);
void gpHalApbdma1Disable(int ch);

#endif
