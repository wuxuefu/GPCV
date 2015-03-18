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

/**
 * @file hal_hdmi.c
 * @brief HDMI HAL interface 
 * @author SJ Lin
 */
/*******************************************************************************
*                         H E A D E R   F I L E S
*******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <asm/io.h>

#include <mach/hal/sysregs.h>
#include <mach/hal/hal_hdmi.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/


/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#ifndef DEBUG
#if 0
#define DEBUG	printk
#else
#define DEBUG(...)
#endif
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/


/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/


/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void
gpHalHDMISendPacket(
	void *RegBase,
	UINT32 ch,
	const void *data,
	UINT32 blank,
	UINT32 sendMode
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	const unsigned long *tmp = (const unsigned long *)data;

	pdispReg->HdmiPkt[ch].dispHDMIPacketBD[0] = tmp[1];
	pdispReg->HdmiPkt[ch].dispHDMIPacketBD[1] = tmp[2];
	pdispReg->HdmiPkt[ch].dispHDMIPacketBD[2] = tmp[3];
	pdispReg->HdmiPkt[ch].dispHDMIPacketBD[3] = tmp[4];
	pdispReg->HdmiPkt[ch].dispHDMIPacketBD[4] = tmp[5];
	pdispReg->HdmiPkt[ch].dispHDMIPacketBD[5] = tmp[6];
	pdispReg->HdmiPkt[ch].dispHDMIPacketBD[6] = tmp[7];
	pdispReg->HdmiPkt[ch].dispHDMIPacketHD = (tmp[0] & 0xFFFFFF) | blank | sendMode;
}
EXPORT_SYMBOL(gpHalHDMISendPacket);

void
gpHalHDMISetACRPacket(
	void *RegBase,
	UINT32 N,
	UINT32 CTS
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	int ACR_STEP = 128, ACR_EN = 1;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, N, CTS);

	pdispReg->dispHDMIAudioN = HAL_HDMI_PKT_AUTO | HAL_HDMI_PKT_VVLD | (ACR_STEP << 20) | N;
	pdispReg->dispHDMIAudioCTS = (ACR_EN << 31) | CTS;
}
EXPORT_SYMBOL(gpHalHDMISetACRPacket);

void
gpHalHDMISetAudioSamplePacket(
	void *RegBase,
	UINT32 ch
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, ch);

	switch(ch)
	{
	case 2:	pdispReg->dispHDMIAudioSample = 0x001;	break;
	case 4: pdispReg->dispHDMIAudioSample = 0x103;	break;
	case 6:	pdispReg->dispHDMIAudioSample = 0x107;	break;
	case 8: pdispReg->dispHDMIAudioSample = 0x10F;	break;
	}
}
EXPORT_SYMBOL(gpHalHDMISetAudioSamplePacket);

/**
 * \brief Set the HDMI time cycle register
 */ 
void
gpHalHDMISetTimeCycle(
	void *RegBase,
	UINT32 VBack,
	UINT32 HBlank
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	pdispReg->dispHDMITimeCycle = (VBack << 12) | HBlank;
}
EXPORT_SYMBOL(gpHalHDMISetTimeCycle);

void
gpHalHDMISetPixelRepetition(
	void *RegBase,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, value);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x7 << 24);
	regVal |= (value << 24);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalHDMISetPixelRepetition);

void
gpHalDispHDMIPHYConfig(
	void *RegBase,
	UINT32 phy1,
	UINT32 phy2
)
{	
	dispReg_t *pdispReg = (dispReg_t *)RegBase;

	DEBUG("[%s:%d][0x%x][0x%x]\n", __FUNCTION__, __LINE__, phy1, phy2);

	pdispReg->hdmiTXPHYConfig1 = phy1;
	pdispReg->hdmiTXPHYConfig2 = phy2;
}
EXPORT_SYMBOL(gpHalDispHDMIPHYConfig);
