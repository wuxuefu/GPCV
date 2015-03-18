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
 * @file    hal_dac.h
 * @brief   Implement of DAC HAL API header file.
 * @author  Simon Hsu
 */

#ifndef _HAL_DAC_H_
#define _HAL_DAC_H_

/*******************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
*******************************************************************************/
void gpHalDacClkEn(int enable);
void gpHalVrefCtrl(int enable, int fsmen);
void gpHalAuddacEn(int enable);
void gpHalAudadcEn(int enable);
void gpHalDacFeqSet(int freq);

void gpHalMicEn(int enable);
void gpHalMicVolGet(long *vol);
void gpHalMicVolSet(long vol);

void gpHalLininEn(int enable);
void gpHalLininVolSet(long l_vol, long r_vol);
void gpHalLininVolGet(long *l_vol, long *r_vol);

void gpHalHdphnEn(int enable);
void gpHalHdphnVolSet(long l_vol, long r_vol);
void gpHalHdphnVolGet(long *l_vol, long *r_vol);
void gpHalHdphnMuteGet(long *l_mute, long *r_mute);
void gpHalHdphnMuteSet(long l_mute, long r_mute);
unsigned int gpHalHpinsGet(void);
void gpHalHpinsSet(unsigned int hpins);

unsigned int gpHalAdinsGet(void);
void gpHalHdphnDischarge(void);

#endif