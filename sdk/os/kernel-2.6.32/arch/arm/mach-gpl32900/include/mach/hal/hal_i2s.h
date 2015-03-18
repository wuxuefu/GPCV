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
 * @file    hal_i2s.h
 * @brief   Implement of I2S HAL API header file.
 * @author  Simon Hsu
 */

#ifndef _HAL_I2S_H_
#define _HAL_I2S_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

void gpHalI2sClkEn(int enable);
void gpHalI2sCtlSet(int sel, int ctl);
void gpHalI2sFmtSet( int sel, int order );
void gpHalI2sEn( int sel );
void gpHalI2sDisable( int sel );
void gpHalI2sIntEn( int sel );
void gpHalI2sIntDisable( int sel );
void gpHalI2sFifoClr( int sel );
void gpHalI2sChlSet( int sel, int ch );
unsigned int gpHalI2sCtlGet(int sel);

#endif
