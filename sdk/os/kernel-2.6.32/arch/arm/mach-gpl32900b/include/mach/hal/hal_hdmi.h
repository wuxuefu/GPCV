/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2013 by Generalplus Inc.                         *
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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30077, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
#ifndef _HAL_HDMI_H_
#define _HAL_HDMI_H_

/**************************************************************************
 *                         H E A D E R   F I L E S                        *
 **************************************************************************/
#include <mach/typedef.h>



/**************************************************************************
*                           C O N S T A N T S                             *
 **************************************************************************/
enum {
	HAL_HDMI_PIXEL_REP_1TIMES = 0,
	HAL_HDMI_PIXEL_REP_2TIMES = 1,
	HAL_HDMI_PIXEL_REP_4TIMES = 2,
	HAL_HDMI_PIXEL_REP_8TIMES = 3,
};

#define HAL_HDMI_PKT_HVLD 0x10000000
#define HAL_HDMI_PKT_VVLD 0x20000000
#define HAL_HDMI_PKT_AUTO 0x40000000
#define HAL_HDMI_PKT_SEND 0x80000000


/*******************************************************************************
*                          D A T A    T Y P E S
*******************************************************************************/

/*******************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
*******************************************************************************/
void gpHalHDMISendPacket(void *RegBase, UINT32 ch, const void *data, UINT32 blank, UINT32 sendMode);
void gpHalHDMISetACRPacket(void *RegBase, UINT32 N, UINT32 CTS);
void gpHalHDMISetAudioSamplePacket(void *RegBase, UINT32 ch);
void gpHalHDMISetTimeCycle(void *RegBase, UINT32 VBack, UINT32 HBlank);
void gpHalHDMISetPixelRepetition(void *RegBase, UINT32 value);
void gpHalDispHDMIPHYConfig(void *RegBase, UINT32 phy1, UINT32 phy2);

#endif  /* __HAL_HDMI_H__ */

