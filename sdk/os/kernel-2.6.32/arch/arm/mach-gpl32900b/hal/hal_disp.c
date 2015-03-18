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
 * @file hal_disp.c
 * @brief Display HAL interface 
 * @author Anson Chuang
 */
/*******************************************************************************
*                         H E A D E R   F I L E S
*******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <asm/io.h>

#include <mach/hal/sysregs.h>
#include <mach/hal/hal_disp.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/


/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if 0
#define DEBUG	printk
#else
#define DEBUG(...)
#endif

#define DERROR printk

#define OSDREG(RegBase)	(((int)RegBase) == LOGI_ADDR_DISP_REG ? osdRegArray : osdRegArray + 1)

#define OSDREGBACKUP(RegBase) ((RegBase == disp_RegBase[0]) ? osdRegBackupArray : osdRegBackupArray + 1)
#define DISPNUM(RegBase) ((RegBase == disp_RegBase[0]) ? 0 : 1)

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
#define DISP0REG	((dispReg_t*)(LOGI_ADDR_DISP_REG))

static const dispOsdRegArray_t osdRegArray[HAL_DISP_CTRLER_NUM] = 
{
	{
		{&DISP0REG->dispOsd0Addr,		&DISP0REG->dispOsd1Addr,},
		{&DISP0REG->dispOsd0Pitch,		&DISP0REG->dispOsd1Pitch,},
		{&DISP0REG->dispOsd0Res,		&DISP0REG->dispOsd1Res,},
		{&DISP0REG->dispOsd0XY,			&DISP0REG->dispOsd1XY,},
		{&DISP0REG->dispOsd0Fmt,		&DISP0REG->dispOsd1Fmt,},
		{&DISP0REG->dispOsd0Ctrl,		&DISP0REG->dispOsd1Ctrl,},
		{&DISP0REG->dispOsd0SclHParam,	&DISP0REG->dispOsd1SclHParam,},
		{&DISP0REG->dispOsd0SclVParam0,	&DISP0REG->dispOsd1SclVParam0,},
		{&DISP0REG->dispOsd0SclVParam1,	&DISP0REG->dispOsd1SclVParam1,},
		{&DISP0REG->dispOsd0SclRes,		&DISP0REG->dispOsd1SclRes,},
	},
};
typedef void *PVOID;
static const PVOID disp_RegBase[HAL_DISP_CTRLER_NUM] = {(void*)DISP0REG};

static dispReg_t dispRegBackup[HAL_DISP_CTRLER_NUM];
static const dispOsdRegArray_t osdRegBackupArray[HAL_DISP_CTRLER_NUM] = 
{
	{
		{&dispRegBackup[0].dispOsd0Addr,		&dispRegBackup[0].dispOsd1Addr,},
		{&dispRegBackup[0].dispOsd0Pitch,		&dispRegBackup[0].dispOsd1Pitch,},
		{&dispRegBackup[0].dispOsd0Res,			&dispRegBackup[0].dispOsd1Res,},
		{&dispRegBackup[0].dispOsd0XY,			&dispRegBackup[0].dispOsd1XY,},
		{&dispRegBackup[0].dispOsd0Fmt,			&dispRegBackup[0].dispOsd1Fmt,},
		{&dispRegBackup[0].dispOsd0Ctrl,		&dispRegBackup[0].dispOsd1Ctrl,},
		{&dispRegBackup[0].dispOsd0SclHParam,	&dispRegBackup[0].dispOsd1SclHParam,},
		{&dispRegBackup[0].dispOsd0SclVParam0,	&dispRegBackup[0].dispOsd1SclVParam0,},
		{&dispRegBackup[0].dispOsd0SclVParam1,	&dispRegBackup[0].dispOsd1SclVParam1,},
		{&dispRegBackup[0].dispOsd0SclRes,		&dispRegBackup[0].dispOsd1SclRes,},
	},
};

#include "hal_hdmi.c"

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void
gpHalDispInit(
	void *RegBase
)
{
	memcpy(&dispRegBackup[DISPNUM(RegBase)], RegBase, sizeof(dispReg_t));
}
EXPORT_SYMBOL(gpHalDispInit);

void *
gpHalDispGetRegBase(
	int idx
)
{
	return disp_RegBase[idx];
}
EXPORT_SYMBOL(gpHalDispGetRegBase);
	
UINT32 gpHalDispGetIntFlag(void *RegBase)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	return pdispReg->dispIrqSrc;
}
EXPORT_SYMBOL(gpHalDispGetIntFlag);
	
#if 0
void gpHalDispDumpRegister(void *RegBase)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 *pAddr = (UINT32 *)pdispReg;
	UINT32 i;
	
	for (i=0; i<256; i++) {
		if (i%8 == 0) {
			printk("\n");
			if ((int)pdispReg == LOGI_ADDR_DISP_REG)
			{
				printk("addr = 0x%08x\t", 0x92010000+i*4);
			}
			else
			{
				printk("addr = 0x%08x\t", 0x92011000+i*4);
			}
	}
		printk("0x%08x,  ", (UINT32) pAddr[i]);
	}
	printk("\n");

	
}
EXPORT_SYMBOL(gpHalDispDumpRegister);
#endif

//(0x000) Display format ------------------------------------------------------------------------
void gpHalDispSetOFunc(void *RegBase, UINT32 type)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);
	
	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~(0x1 << 31);
	regVal |= (type << 31);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOFunc);

void gpHalDispSetPanelFormat(void *RegBase, UINT32 format, UINT32 type, UINT32 seq0, UINT32 seq1)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, format, type, seq0, seq1);
	
	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~((0x3 << 28) | (0x3 << 24) | (0x7 << 20) | (0x7 << 16));
	regVal |= ((format << 28) | (type << 24) | (seq0 << 20) | (seq1 << 16));
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetPanelFormat);

void gpHalDispSetHueAdj(void *RegBase, UINT32 enable)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, enable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	if (enable)
		regVal |= (0x1 << 19);
	else
		regVal &= ~(0x1 << 19);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetHueAdj);

void gpHalDispSetEdgeType(void *RegBase, UINT32 type)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);

	if (type > 2) {
		type = 1;
	}

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~(0x3 << 14);
	regVal |= (type << 14);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetEdgeType);

/* Dither */
void gpHalDispSetDitherType(void *RegBase, UINT32 type)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~(0x3 << 12);
	regVal |= (type << 12);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetDitherType);

UINT32 gpHalDispGetDitherType(void *RegBase)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);	

	return (dispRegBackup[DISPNUM(RegBase)].dispFmt >> 12) & 0x03;
}
EXPORT_SYMBOL(gpHalDispGetDitherType);

void gpHalDispSetDitherEnable(void *RegBase, UINT32 enable)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, enable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	if (enable)
		regVal |= (0x1 << 11);
	else
		regVal &= ~(0x1 << 11);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetDitherEnable);

UINT32
gpHalDispGetDitherEnable(
	void *RegBase
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	return ((dispRegBackup[DISPNUM(RegBase)].dispFmt >> 11) & 0x1);
}
EXPORT_SYMBOL(gpHalDispGetDitherEnable);

void
gpHalDispSetGammaEnable(
	void *RegBase,
	UINT32 enable
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, enable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	if (enable)
		regVal |= (0x1 << 10);
	else
		regVal &= ~(0x1 << 10);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetGammaEnable);

UINT32
gpHalDispGetGammaEnable(
	void *RegBase
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	return ((dispRegBackup[DISPNUM(RegBase)].dispFmt >> 10) & 0x1);
}
EXPORT_SYMBOL(gpHalDispGetGammaEnable);

void
gpHalDispSetTvYupType(
	void *RegBase,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~(0x3 << 8);
	regVal |= (type << 8);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvYupType);

void
gpHalDispSetTvCupType(
	void *RegBase,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~(0x3 << 6);
	regVal |= (type << 6);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvCupType);

void
gpHalDispSetTvCftType(
	void *RegBase,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~(0x1 << 5);
	regVal |= (type << 5);
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvCftType);

void
gpHalDispSetPriInputInfo(
	void *RegBase,
	UINT32 format,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	int cbarColor = 0;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, format, type);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispFmt;
	regVal &= ~((0x3 << 2) | (0x3));
	regVal |= (format << 2);
	regVal |= type;
	dispRegBackup[DISPNUM(RegBase)].dispFmt = regVal;
	pdispReg->dispFmt = regVal;

    // Color bar black must following the color format
    switch((format << 2) | type) {
		case 0x0: //RGB565 :
		case 0x1: //RGB555 :
		case 0x2: //RGB888 :
			cbarColor = 0;
			break;
		case 0x4: //YCbCr422 :
		case 0x6: //YCbCr444 :
		case 0xC: //YCbCr400:
		case 0xD: //YCbCr420:
		case 0xE: //YCbCr422:
		case 0xF: //YCbCr444:
			cbarColor = 0 | (128 << 8) | (128 << 16); /* Y Cb Cr */
			break;
	}
	dispRegBackup[DISPNUM(RegBase)].dispCBar = cbarColor;
	pdispReg->dispCBar = cbarColor;
}
EXPORT_SYMBOL(gpHalDispSetPriInputInfo);

unsigned long gpHalDispGetPriInputInfo(void *RegBase)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);	
	return (dispRegBackup[DISPNUM(RegBase)].dispFmt & 0xF);
}
EXPORT_SYMBOL(gpHalDispGetPriInputInfo);

//(0x004) Display control ------------------------------------------------------------------------
void gpHalDispSetEnable(void *RegBase, int devType, int enable)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, enable);

	//Disable U & V Edgetype in Display Controller Register2 
	dispRegBackup[DISPNUM(RegBase)].dispCtrl2 = 0;
	pdispReg->dispCtrl2 = 0;
		
	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0xF << 28);
	if (enable) {
		if (devType == HAL_DISP_DEV_LCM)			regVal |= (0x4 << 28);
		else if (devType == HAL_DISP_DEV_LCD)		regVal |= (0x8 << 28);
		else if (devType == HAL_DISP_DEV_HDMI)		regVal |= (0x9 << 28);
		else if (devType == HAL_DISP_DEV_TV)		regVal |= (0xC << 28);
		else
			printk(KERN_ERR "[%s:%d] invalid devType\n", __FUNCTION__, __LINE__);
	}
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetEnable);

int gpHalDispGetDevType(void *RegBase)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return (dispRegBackup[DISPNUM(RegBase)].dispCtrl >> 28) & 0xF;
}
EXPORT_SYMBOL(gpHalDispGetDevType);

void
gpHalDispSetPriDmaType(
	void *RegBase,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 27);
	regVal |= (type << 27);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetPriDmaType);

UINT32
gpHalDispGetPriDmaType(
	void *RegBase
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return (dispRegBackup[DISPNUM(RegBase)].dispCtrl >> 27) & 0x1;
}

void
gpHalDispSetRepNum(
	void *RegBase,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, value);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 24);
	regVal |= (value << 24);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetRepNum);

void
gpHalDispSetClkPolarity(
	void *RegBase,
	UINT32 polarity
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, polarity);
	
	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 23);
	regVal |= (polarity << 23);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetClkPolarity);

void
gpHalDispSetOCLKSel(
	void *RegBase,
	UINT32 sel
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, sel);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x3 << 20);
	regVal |= (sel << 20);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOCLKSel);

void
gpHalDispSetOBT656l(
	void *RegBase,
	UINT32 enable
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, enable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 18);
	regVal |= (enable << 18);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOBT656l);

void
gpHalDispSetBlankingIntervalTo0(
	void *RegBase,
	UINT32 enable
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, enable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 16);
	regVal |= (enable << 16);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetBlankingIntervalTo0);

void
gpHalDispSetLcmInterface(
	void *RegBase,
	UINT32 interface
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, interface);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 14);
	regVal |= (interface << 14);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetLcmInterface);

void
gpHalDispSetLcmMode(
	void *RegBase,
	UINT32 mode
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, mode);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 13);
	regVal |= (mode << 13);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetLcmMode);

void
gpHalDispSetLcmDataSelect(
	void *RegBase,
	UINT32 sel
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, sel);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 12);
	regVal |= (sel << 12);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetLcmDataSelect);

/* Tv */
void
gpHalDispSetTvType(
	void *RegBase,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, type);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 11);
	regVal |= (type << 11);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvType);

void
gpHalDispSetTvPulse(
	void *RegBase,
	UINT32 pulse
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, pulse);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 10);
	regVal |= (pulse << 10);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvPulse);

void
gpHalDispSetTvScan(
	void *RegBase,
	UINT32 scan
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, scan);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x3 << 8);
	regVal |= (scan << 8);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvScan);

void
gpHalDispSetTvFscType(
	void *RegBase,
	UINT32 fsc
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, fsc);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x7 << 5);
	regVal |= (fsc << 5);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvFscType);

void
gpHalDispSetTvFix625(
	void *RegBase,
	UINT32 fix
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, fix);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 4);
	regVal |= (fix << 4);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvFix625);

void
gpHalDispSetTvLine(
	void *RegBase,
	UINT32 line
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, line);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 3);
	regVal |= (line << 3);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvLine);

void
gpHalDispSetTvColorBurstWidth(
	void *RegBase,
	UINT32 width
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, width);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x1 << 2);
	regVal |= (width << 2);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvColorBurstWidth);

void
gpHalDispSetTvColorBurstSel(
	void *RegBase,
	UINT32 sel
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, sel);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCtrl;
	regVal &= ~(0x3 << 0);
	regVal |= (sel << 0);
	dispRegBackup[DISPNUM(RegBase)].dispCtrl = regVal;
	pdispReg->dispCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetTvColorBurstSel);


//(0x008) Display burst ------------------------------------------------------------------------
void
gpHalDispSetPriFlip(
	void *RegBase,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, value);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispBurst;
	regVal &= ~(0x3 << 28);
	regVal |= (value << 28);
	dispRegBackup[DISPNUM(RegBase)].dispBurst = regVal;
	pdispReg->dispBurst = regVal;
}
EXPORT_SYMBOL(gpHalDispSetPriFlip);

void
gpHalDispSetHUE(
	void *RegBase,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, value);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispBurst;
	regVal &= ~(0x7ff << 16);
	regVal |= (value << 16);
	dispRegBackup[DISPNUM(RegBase)].dispBurst = regVal;
	pdispReg->dispBurst = regVal;
}
EXPORT_SYMBOL(gpHalDispSetHUE);

	void
gpHalDispSetEdgeGain(
	void *RegBase,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, value);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispBurst;
	regVal &= ~(0xf << 12);
	regVal |= (value << 12);
	dispRegBackup[DISPNUM(RegBase)].dispBurst = regVal;
	pdispReg->dispBurst = regVal;
}
EXPORT_SYMBOL(gpHalDispSetEdgeGain);

void
gpHalDispSetCortype(
	void *RegBase,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, value);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispBurst;
	regVal &= ~(0x3 << 10);
	regVal |= (value << 10);
	dispRegBackup[DISPNUM(RegBase)].dispBurst = regVal;
	pdispReg->dispBurst = regVal;
}
EXPORT_SYMBOL(gpHalDispSetCortype);

void
gpHalDispSetCliptype(
	void *RegBase,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, value);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispBurst;
	regVal &= ~(0x3 << 8);
	regVal |= (value << 8);
	dispRegBackup[DISPNUM(RegBase)].dispBurst = regVal;
	pdispReg->dispBurst = regVal;
}
EXPORT_SYMBOL(gpHalDispSetCliptype);

void
gpHalDispSetPriBurstNum(
	void *RegBase,
	UINT32 burstNum
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, burstNum);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispBurst;
	regVal &= ~0xff;
	regVal |= burstNum;
	dispRegBackup[DISPNUM(RegBase)].dispBurst = regVal;
	pdispReg->dispBurst = regVal;
}
EXPORT_SYMBOL(gpHalDispSetPriBurstNum);

//(0x00c) UV Channgel Address ------------------------------------------------------------------------
void gpHalDispSetPriUVAddr(void *RegBase, const void *addr)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][0x%08x]\n", __FUNCTION__, __LINE__, (UINT32)addr);
	dispRegBackup[DISPNUM(RegBase)].dispUVAddr = (UINT32)addr;
	pdispReg->dispUVAddr = (UINT32)addr;
}
EXPORT_SYMBOL(gpHalDispSetPriUVAddr);

//(0x010) Framebuffer Address ------------------------------------------------------------------------
void gpHalDispSetPriFrameAddr(void *RegBase, const void *addr)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][0x%08x]\n", __FUNCTION__, __LINE__, (UINT32)addr);
	dispRegBackup[DISPNUM(RegBase)].dispPriAddr = (UINT32)addr;
	pdispReg->dispPriAddr = (UINT32)addr;
}
EXPORT_SYMBOL(gpHalDispSetPriFrameAddr);

//(0x014) Framebuffer picth ------------------------------------------------------------------------
void
gpHalDispSetPriPitch(
	void *RegBase,
	UINT16 src,
	UINT16 act
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, src, act);
	dispRegBackup[DISPNUM(RegBase)].dispPriPitch = (src << 16) | act;
	pdispReg->dispPriPitch = (src << 16) | act;
}
EXPORT_SYMBOL(gpHalDispSetPriPitch);

//(0x018) Framebuffer resolution ------------------------------------------------------------------------
void
gpHalDispSetPriRes(
	void *RegBase,
	UINT16 width,
	UINT16 height
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, width, height);
	dispRegBackup[DISPNUM(RegBase)].dispPriRes = (width << 16) | height;
	pdispReg->dispPriRes = (width << 16) | height;
}
EXPORT_SYMBOL(gpHalDispSetPriRes);

//(0x01c-0x028) Scaling setting ------------------------------------------------------------------------
void
gpHalDispSetPriSclInfo(
	void *RegBase,
	gpHalDispSclInfo_t scale
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	UINT16 factor;

	DEBUG("[%s:%d][%d][%d][%d][%d][%d][%d][%d]\n", __FUNCTION__, __LINE__,
		scale.srcWidth, scale.srcHeight, scale.dstWidth, scale.dstHeight, 
		scale.hInit, scale.vInit0, scale.vInit1);

	/* Clear scale control register (0x028) */
	regVal = dispRegBackup[DISPNUM(RegBase)].dispPriSclCtrl;
	regVal &= ~((0xFFFF << 16) | (0x1 << 2) | (0x1)); /* Clear vinit1[31:16], v select[2], h select[0] */

	/* Set scale resolution */
	dispRegBackup[DISPNUM(RegBase)].dispPriSclRes = (scale.dstWidth << 16) | scale.dstHeight;
	pdispReg->dispPriSclRes = (scale.dstWidth << 16) | scale.dstHeight;

	/* Calculate & set h factor, h scale up/down flag(0x028) bit[0] */
	factor = 0;
	if (scale.dstWidth != scale.srcWidth) {
		if (scale.dstWidth > scale.srcWidth) {
			factor = (scale.srcWidth << 16) / scale.dstWidth;
			/* Update regVal (0x028) bit[0], scaling up */
			regVal &= ~0x01;
		}
		else {
			factor = ((scale.dstWidth << 16) + (scale.srcWidth - 1)) / scale.srcWidth;
			/* Update regVal (0x028) bit[0], scaling down */
			regVal |= 0x01;
		}
	}
	DEBUG("[%s:%d], hfactor=%d, scl=%d\n", __FUNCTION__, __LINE__, factor, regVal & 0x01);
	dispRegBackup[DISPNUM(RegBase)].dispPriSclHParam = (scale.hInit << 16) | factor;
	pdispReg->dispPriSclHParam = (scale.hInit << 16) | factor;


	/* Calculate & set v factor, v scale up/down flag(0x028) bit[2] */
	factor = 0;
	if (scale.dstHeight != scale.srcHeight) {
		if (scale.dstHeight > scale.srcHeight) {
			factor = (scale.srcHeight << 16) / scale.dstHeight;
			/* Update regVal (0x028) bit[2], scaling up */
			regVal &= ~0x04;
		}
		else {
			factor = ((scale.dstHeight << 16) + (scale.srcHeight - 1)) / scale.srcHeight;
			/* Update regVal (0x028) bit[2], scaling down */
			regVal |= 0x04;
		}
	}
	DEBUG("[%s:%d], vfactor=%d, scl=%d\n", __FUNCTION__, __LINE__, factor, regVal & 0x04);
	dispRegBackup[DISPNUM(RegBase)].dispPriSclVParam = (scale.vInit0 << 16) | factor;
	pdispReg->dispPriSclVParam = (scale.vInit0 << 16) | factor;

	/* Set v init1 */
	regVal |= scale.vInit1;
	dispRegBackup[DISPNUM(RegBase)].dispPriSclCtrl = regVal;
	pdispReg->dispPriSclCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetPriSclInfo);

void
gpHalDispSetPriSclEnable(
	void *RegBase,
	UINT32 hEnable,
	UINT32 vEnable
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d], h=%d, v=%d\n", __FUNCTION__, __LINE__, hEnable, vEnable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispPriSclCtrl;
	if (hEnable)	regVal |= 0x2;
	else			regVal &= ~0x2;
	if (vEnable)	regVal |= 0x8;
	else			regVal &= ~0x8;
	dispRegBackup[DISPNUM(RegBase)].dispPriSclCtrl = regVal;
	pdispReg->dispPriSclCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetPriSclEnable);

void
gpHalDispSetDeflickerInfo(
	void *RegBase,
	UINT32 vEnable,
	UINT32 hEnable,
	UINT32 DefInterlaced,
	UINT32 FSTFieldOutSE
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d], h=%d, v=%d\n", __FUNCTION__, __LINE__, hEnable, vEnable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispPriSclCtrl;
	if(vEnable)			regVal |= 0x200;
	else				regVal &= ~0x200;
	if(hEnable)			regVal |= 0x100;
	else				regVal &= ~0x100;
	if(DefInterlaced)	regVal |= 0x80;
	else				regVal &= ~0x80;
	if(FSTFieldOutSE)	regVal |= 0x40;
	else				regVal &= ~0x40;
	dispRegBackup[DISPNUM(RegBase)].dispPriSclCtrl = regVal;
	pdispReg->dispPriSclCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetDeflickerInfo);

/* Osd layer */
//(0x030/0x050) OSDx Base address ------------------------------------------------------------------------
void
gpHalDispSetOsdFrameAddr(
	void *RegBase,
	UINT32 layerNum,
	const void *addr
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][0x%x]\n", __FUNCTION__, __LINE__, layerNum, (UINT32) addr);

	*OSDREGBACKUP(pdispReg)->DISP_OSD_ADDR[layerNum] = (UINT32)addr;
	*OSDREG(pdispReg)->DISP_OSD_ADDR[layerNum] = (UINT32)addr;
}
EXPORT_SYMBOL(gpHalDispSetOsdFrameAddr);

//(0x034/0x054) OSDx pitch ------------------------------------------------------------------------
void
gpHalDispSetOsdPitch(
	void *RegBase,
	UINT32 layerNum,
	UINT16 src,
	UINT16 act
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, src, act);

	*OSDREGBACKUP(pdispReg)->DISP_OSD_PITCH[layerNum] = (src << 16) | act;
	*OSDREG(pdispReg)->DISP_OSD_PITCH[layerNum] = (src << 16) | act;
}
EXPORT_SYMBOL(gpHalDispSetOsdPitch);

//(0x038/0x058) OSDx resolution ------------------------------------------------------------------------
void
gpHalDispSetOsdRes(
	void *RegBase,
	UINT32 layerNum,
	UINT16 width,
	UINT16 height
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, width, height);

	*OSDREGBACKUP(pdispReg)->DISP_OSD_RES[layerNum] = (width << 16) | height;
	*OSDREG(pdispReg)->DISP_OSD_RES[layerNum] = (width << 16) | height;
}
EXPORT_SYMBOL(gpHalDispSetOsdRes);

//(0x03c/0x05c) OSDx staring coordinate ------------------------------------------------------------------------
void
gpHalDispSetOsdFlip(
	void *RegBase,
	UINT32 layerNum,
	UINT32 value
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, value);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_XY[layerNum];
	regVal &= ~(0x3 << 30);
	regVal |= (value << 30);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_XY[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_XY[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdFlip);

void
gpHalDispSetOsdXY(
	void *RegBase,
	UINT32 layerNum,
	UINT16 x,
	UINT16 y
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, x, y);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_XY[layerNum];
	regVal &= ~((0x7ff << 16) | (0x7ff));
	regVal |= ((x << 16) | (y));
	*OSDREGBACKUP(pdispReg)->DISP_OSD_XY[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_XY[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdXY);

//(0x040/0x048) OSDx format ------------------------------------------------------------------------
void
gpHalDispSetOsdInputFmt(
	void *RegBase,
	UINT32 layerNum,
	UINT32 format
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, format);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum];
	regVal &= ~(0x3 << 30);
	regVal |= ((format&0x3) << 30);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;

	// Bit[2] referred to OSDx control register
	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	regVal &= ~(0x1 << 20);
	regVal |= (((format&0x4) >> 2) << 20);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdInputFmt);

void
gpHalDispSetOsdInputType(
	void *RegBase,
	UINT32 layerNum,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, type);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum];
	regVal &= ~(0x3 << 28);
	regVal |= (type << 28);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdInputType);

//(0x040/0x048 + 0x098)  color key setting ------------------------------------------------------------------------
void
gpHalDispSetOsdColorKey(
	void *RegBase,
	UINT32 layerNum,
	UINT32 color
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal, ColorKey8888;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, color);

	if (layerNum == 1) {
		regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum];
		regVal &= ~(0xffff);
		regVal |= (color & 0xffff);
		*OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
		*OSDREG(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
		return;
	}
	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	if (regVal & (0x1 << 20)) {
		regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum];
		regVal &= ~(0xffff);
		regVal |= (color & 0xffff);
		*OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
		*OSDREG(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;

		ColorKey8888 = pdispReg->dispOsdColorKey;
		ColorKey8888 &= ~(0xff << (layerNum * 8));
		ColorKey8888 |= (color & 0x00ff0000) >> (16 - (layerNum*8));
		dispRegBackup[DISPNUM(RegBase)].dispOsdColorKey = ColorKey8888;
		pdispReg->dispOsdColorKey = ColorKey8888;
	}
	else {
		regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum];
		regVal &= ~(0xffff);
		regVal |= (((color & 0x07e0) << 5)|((color & 0x001f) << 3));
		*OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
		*OSDREG(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;

		ColorKey8888 = pdispReg->dispOsdColorKey;
		ColorKey8888 &= ~(0xff << (layerNum * 8));
		ColorKey8888 |= (color & 0xf800) >> (8 - (layerNum*8));
		dispRegBackup[DISPNUM(RegBase)].dispOsdColorKey = ColorKey8888;
		pdispReg->dispOsdColorKey = ColorKey8888;
	}
}
EXPORT_SYMBOL(gpHalDispSetOsdColorKey);

//(0x044/0x04c) OSDx control ------------------------------------------------------------------------
void
gpHalDispSetOsdEnable(
	void *RegBase,
	UINT32 layerNum,
	UINT32 enable
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, enable);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	if (enable) {
		regVal |= (0x1 << 31);
	}
	else {
		regVal &= ~(0x1 << 31);
	}
	*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdEnable);

UINT32
gpHalDispGetOsdEnable(
	void *RegBase,
	UINT32 layerNum
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	return (*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] >> 31) & 0x1;
}
EXPORT_SYMBOL(gpHalDispGetOsdEnable);

void
gpHalDispSetOsdDmaType(
	void *RegBase,
	UINT32 layerNum,
	UINT32 type
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, type);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	regVal &= ~(0x1 << 30);
	regVal |= (type << 30);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdDmaType);

void
gpHalDispSetOsdAlpha(
	void *RegBase,
	UINT32 layerNum,
	UINT32 consta,
	UINT32 ppamd,
	UINT32 alpha,
	UINT16 alphasel
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	UINT32 newAlpha;

	DEBUG("[%s:%d][%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, consta, ppamd, alpha);

	if (alpha > 100)
		alpha = 100;
	newAlpha = (0xff * alpha) / 100;

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum];
	regVal &= ~((0x1 << 27) | (0x3 << 24) | (0xff << 16));
	regVal |= (((consta & 0xf) << 27) | (ppamd << 24) | (newAlpha << 16));
	*OSDREGBACKUP(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_FMT[layerNum] = regVal;
	
	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	regVal &= ~(0x1 << 21);
	regVal |= ((alphasel>0) ? 1:0) << 21;
	*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;	
}
EXPORT_SYMBOL(gpHalDispSetOsdAlpha);

void
gpHalDispSetOsdSclEnable(
	void *RegBase,
	UINT32 layerNum,
	UINT32 hEnable,
	UINT32 vEnable
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	DEBUG("[%s:%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, hEnable, vEnable);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	if (hEnable)
		regVal |= (0x1 << 17);
	else
		regVal &= ~(0x1 << 17);

	if (vEnable)
		regVal |= (0x1 << 16);
	else
		regVal &= ~(0x1 << 16);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdSclEnable);

void
gpHalDispSetOsdBurstNum(
	void *RegBase,
	UINT32 layerNum,
	UINT32 burstnum
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, burstnum);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	regVal &= ~(0xff << 8);
	regVal |= (burstnum << 8);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdBurstNum);

void
gpHalDispSetOsdPaletteOffset(
	void *RegBase,
	UINT32 layerNum,
	UINT32 offset
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum, offset);

	regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
	regVal &= ~(0xff);
	regVal |= offset;
	*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
	*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
}
EXPORT_SYMBOL(gpHalDispSetOsdPaletteOffset);

UINT32
gpHalDispGetOsdPaletteOffset(
	void *RegBase,
	UINT32 layerNum
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	return (*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum]) & 0xff;
}
EXPORT_SYMBOL(gpHalDispGetOsdPaletteOffset);

//(0x060-07c) OSDx scaling setting ------------------------------------------------------------------------
void
gpHalDispSetOsdSclInfo(
	void *RegBase,
	UINT32 layerNum,
	gpHalDispSclInfo_t scale
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	UINT16 factor;
    UINT32 InterlaceMode;

	DEBUG("[%s:%d][%d][%d][%d][%d][%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, layerNum,
		scale.srcWidth, scale.srcHeight, scale.dstWidth, scale.dstHeight, 
		scale.hInit, scale.vInit0, scale.vInit1);

	/* Set scale resolution */
	*OSDREGBACKUP(pdispReg)->DISP_OSD_SCLRES[layerNum] = (scale.dstWidth << 16) | scale.dstHeight;
	*OSDREG(pdispReg)->DISP_OSD_SCLRES[layerNum] = (scale.dstWidth << 16) | scale.dstHeight;

	/* Calculate & set h factor / initial value */
	factor = 0;
	if (scale.dstWidth != scale.srcWidth) {
		if (layerNum == HAL_DISP_OSD_0) {
			regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
			if (scale.dstWidth > scale.srcWidth) {
				factor = (scale.srcWidth << 16) / scale.dstWidth;
				/* Update regVal (0x044) bit[19], scaling up */
				regVal &= ~(0x01 << 19);
			}
			else {
				factor = ((scale.dstWidth << 16) + (scale.srcWidth - 1)) / scale.srcWidth;
				/* Update regVal (0x044) bit[19], scaling down */
				regVal |= (0x01 << 19);
			}
			*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
			*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
		}
		else {
			factor = ((scale.srcWidth - 1) << 11) / (scale.dstWidth - 1);
		}
	}

	DEBUG("[%s:%d], hfactor=%d\n", __FUNCTION__, __LINE__, factor);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_HPARAM[layerNum] = (scale.hInit << 16) | factor;
	*OSDREG(pdispReg)->DISP_OSD_HPARAM[layerNum] = (scale.hInit << 16) | factor;

	/* Calculate & set v factor / initial value */
	InterlaceMode = dispRegBackup[DISPNUM(RegBase)].dispCtrl & (0x1 << 27);
	factor = 0;
	if (scale.dstHeight != scale.srcHeight) {
		if (layerNum == HAL_DISP_OSD_0) {
			regVal = *OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum];
			if (scale.dstHeight > scale.srcHeight) {
				factor = (scale.srcHeight << 16) / scale.dstHeight;
				/* Update regVal (0x044) bit[18], scaling up */
				regVal &= ~(0x01 << 18);
			}
			else {
				factor = ((scale.dstHeight << 16) + (scale.srcHeight - 1)) / scale.srcHeight;
				/* Update regVal (0x044) bit[18], scaling down */
				regVal |= (0x01 << 18);
			}
			*OSDREGBACKUP(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
			*OSDREG(pdispReg)->DISP_OSD_CTRL[layerNum] = regVal;
		}
		else {
			if (InterlaceMode)
				factor = ((scale.srcHeight - 1) << 11) / (scale.dstHeight - 1) * 2;
			else
				factor = ((scale.srcHeight - 1) << 11) / (scale.dstHeight - 1);
		}
	}

	DEBUG("[%s:%d], vfactor=%d\n", __FUNCTION__, __LINE__, factor);
	*OSDREGBACKUP(pdispReg)->DISP_OSD_VPARAM0[layerNum] = (scale.vInit0 << 16) | factor;
	*OSDREG(pdispReg)->DISP_OSD_VPARAM0[layerNum] = (scale.vInit0 << 16) | factor;

	*OSDREGBACKUP(pdispReg)->DISP_OSD_VPARAM1[layerNum] = scale.vInit1 << 16;
	*OSDREG(pdispReg)->DISP_OSD_VPARAM1[layerNum] = scale.vInit1 << 16;

}
EXPORT_SYMBOL(gpHalDispSetOsdSclInfo);

//(0x080) Output resolution ------------------------------------------------------------------------
void
gpHalDispSetRes(
	void *RegBase,
	UINT16 width,
	UINT16 height
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, width, height);

	dispRegBackup[DISPNUM(RegBase)].dispRes = (width << 16) | height;
	pdispReg->dispRes = (width << 16) | height;
}
EXPORT_SYMBOL(gpHalDispSetRes);

void gpHalDispGetRes(void *RegBase, UINT16 *width, UINT16 *height)
{
	UINT32 regVal;

	regVal = dispRegBackup[DISPNUM(RegBase)].dispRes;
	*width = (regVal >> 16) & 0xfff;
	*height = regVal & 0xfff;
	DEBUG("[%s:%d][%d][%d]\n", __FUNCTION__, __LINE__, *width, *height);
}
EXPORT_SYMBOL(gpHalDispGetRes);

//(0x084-0x08c) Video blankingl ------------------------------------------------------------------------
void
gpHalDispSetPriBlank(
	void *RegBase,
	const gpHalDispBlankInfo_t *blankInfo
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d][%d][%d][0x%x]\n", __FUNCTION__, __LINE__,
		blankInfo->top, blankInfo->bottom, blankInfo->left, blankInfo->right, blankInfo->pattern);

	regVal = (blankInfo->top << 16) | blankInfo->bottom;
	dispRegBackup[DISPNUM(RegBase)].dispVBlank = regVal;
	pdispReg->dispVBlank = regVal;

	regVal = (blankInfo->left << 16) | (blankInfo->right);
	dispRegBackup[DISPNUM(RegBase)].dispHBlank = regVal;
	pdispReg->dispHBlank = regVal;

	dispRegBackup[DISPNUM(RegBase)].dispBlankData = blankInfo->pattern;
	pdispReg->dispBlankData = blankInfo->pattern;
}
EXPORT_SYMBOL(gpHalDispSetPriBlank);

//(0x090-0x94) colorbar  ------------------------------------------------------------------------
void
gpHalDispSetColorBarEnable(
	void *RegBase,
	UINT32 enable
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d]\n", __FUNCTION__, __LINE__, enable);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCBarCtrl;
	if (enable)
		regVal |= (0x1 << 31);
	else
		regVal &= ~(0x1 << 31);
	dispRegBackup[DISPNUM(RegBase)].dispCBarCtrl = regVal;
	pdispReg->dispCBarCtrl = regVal;
}
EXPORT_SYMBOL(gpHalDispSetColorBarEnable);

UINT32
gpHalDispGetColorBarEnable(
	void *RegBase
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return (dispRegBackup[DISPNUM(RegBase)].dispCBarCtrl >> 31) & 0x1;
}
EXPORT_SYMBOL(gpHalDispGetColorBarEnable);

void
gpHalDispSetColorBar(
	void *RegBase,
	UINT32 type,
	UINT32 size,
	UINT32 color
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, type, size, color);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispCBarCtrl;
	regVal &= ~((0xf << 8) | (0xFF));
	regVal |= ((type << 8) | (size));
	dispRegBackup[DISPNUM(RegBase)].dispCBarCtrl = regVal;
	pdispReg->dispCBarCtrl = regVal;

	dispRegBackup[DISPNUM(RegBase)].dispCBar = color;
	pdispReg->dispCBar = color;
}
EXPORT_SYMBOL(gpHalDispSetColorBar);

//(0x0a0-0xb4) color matrix ------------------------------------------------------------------------
void
gpHalDispSetColorMatrix(
	void *RegBase,
	const UINT16 *matrix
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	pdispReg->dispCMatrix0 = matrix[0] | (matrix[1] << 16);
	pdispReg->dispCMatrix1 = matrix[2] | (matrix[3] << 16);
	pdispReg->dispCMatrix2 = matrix[4] | (matrix[5] << 16);
	pdispReg->dispCMatrix3 = matrix[6] | (matrix[7] << 16);
	pdispReg->dispCMatrix4 = matrix[8] | (matrix[9] << 16);
	pdispReg->dispCMatrix5 = matrix[10] | (matrix[11] << 16);

}
EXPORT_SYMBOL(gpHalDispSetColorMatrix);

void
gpHalDispGetColorMatrix(
	void *RegBase,
	UINT16 *matrix
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	matrix[0] = (pdispReg->dispCMatrix0      ) & 0xff;
	matrix[1] = (pdispReg->dispCMatrix0 >> 16) & 0xff;
	matrix[2] = (pdispReg->dispCMatrix1      ) & 0xff;
	matrix[3] = (pdispReg->dispCMatrix1 >> 16) & 0xff;
	matrix[4] = (pdispReg->dispCMatrix2      ) & 0xff;
	matrix[5] = (pdispReg->dispCMatrix2 >> 16) & 0xff;
	matrix[6] = (pdispReg->dispCMatrix3      ) & 0xff;
	matrix[7] = (pdispReg->dispCMatrix3 >> 16) & 0xff;
	matrix[8] = (pdispReg->dispCMatrix4      ) & 0xff;
	matrix[9] = (pdispReg->dispCMatrix4 >> 16) & 0xff;
	matrix[10]= (pdispReg->dispCMatrix5      ) & 0xff;
	matrix[11]= (pdispReg->dispCMatrix5 >> 16) & 0xff;
}
EXPORT_SYMBOL(gpHalDispGetColorMatrix);

//(0x0b8-0x0bc) dither map ------------------------------------------------------------------------
void
gpHalDispSetDitherMap(
	void *RegBase,
	UINT32 map0,
	UINT32 map1
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;

	DEBUG("[%s:%d], %d, %d\n", __FUNCTION__, __LINE__, map0, map1);

	dispRegBackup[DISPNUM(RegBase)].dispDitherMap0 = map0;
	dispRegBackup[DISPNUM(RegBase)].dispDitherMap1 = map1;

	pdispReg->dispDitherMap0 = map0;
	pdispReg->dispDitherMap1 = map1;

}
EXPORT_SYMBOL(gpHalDispSetDitherMap);

void
gpHalDispGetDitherMap(
	void *RegBase,
	UINT32 *map0,
	UINT32 *map1
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	*map0 = dispRegBackup[DISPNUM(RegBase)].dispDitherMap0;
	*map1 = dispRegBackup[DISPNUM(RegBase)].dispDitherMap1;
}
EXPORT_SYMBOL(gpHalDispGetDitherMap);

//(0x0d8) Amplitude adjust ------------------------------------------------------------------------
void
gpHalDispSetTvAmpAdj(
	void *RegBase,
	gpHalDispTvAmpAdj_t amp
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;

	DEBUG("[%s:%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, amp.luminance, amp.blank, amp.burst);

	dispRegBackup[DISPNUM(RegBase)].dispTvAmp = (amp.luminance << 20) | (amp.blank << 10) | (amp.burst);
	pdispReg->dispTvAmp = (amp.luminance << 20) | (amp.blank << 10) | (amp.burst);
}
EXPORT_SYMBOL(gpHalDispSetTvAmpAdj);

//(0x0dc) Position adjust ------------------------------------------------------------------------
void
gpHalDispSetTvPosAdj(
	void *RegBase,
	gpHalDispTvPosAdj_t pos
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;

	DEBUG("[%s:%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, pos.vAct0, pos.vAct1, pos.hAct);

	dispRegBackup[DISPNUM(RegBase)].dispTvPos = (pos.vAct0 << 16) | (pos.vAct1 << 8) | (pos.hAct);
	pdispReg->dispTvPos = (pos.vAct0 << 16) | (pos.vAct1 << 8) | (pos.hAct);
}
EXPORT_SYMBOL(gpHalDispSetTvPosAdj);

//(0x0e0) LCM AC timing ------------------------------------------------------------------------
void
gpHalDispSetLcmAcTiming(
	void *RegBase,
	gpHalDispLcmTiming_t timing
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;

	DEBUG("[%s:%d][%d][%d][%d][%d][%d]\n", __FUNCTION__, __LINE__,
		timing.addrSetup, timing.addrHold, timing.csSetup, timing.csHold, timing.cycLength);

	dispRegBackup[DISPNUM(RegBase)].dispLcmAcTiming = (timing.addrSetup << 28) |
								(timing.addrHold << 16) |
								(timing.csSetup << 12) |
								(timing.csHold << 8) |
								(timing.cycLength << 0);

	pdispReg->dispLcmAcTiming = (timing.addrSetup << 28) |
								(timing.addrHold << 16) |
								(timing.csSetup << 12) |
								(timing.csHold << 8) |
								(timing.cycLength << 0);
}
EXPORT_SYMBOL(gpHalDispSetLcmAcTiming);

//(0x0e4) LCD Vsync  timing ------------------------------------------------------------------------
void
gpHalDispSetLcdVsync(
	void *RegBase,
	lcdTiming_t vsync
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][%d][%d][%d]\n", __FUNCTION__, __LINE__,
		vsync.polarity, vsync.fPorch, vsync.bPorch, vsync.width);

	vsync.polarity -= 1;
	vsync.fPorch -= 1;
	vsync.bPorch -= 1;
	vsync.width -= 1;
	dispRegBackup[DISPNUM(RegBase)].dispLcdVsync = (vsync.polarity << 28) | (vsync.fPorch << 18) | (vsync.bPorch << 8) | (vsync.width);
	pdispReg->dispLcdVsync = (vsync.polarity << 28) | (vsync.fPorch << 18) | (vsync.bPorch << 8) | (vsync.width);
}
EXPORT_SYMBOL(gpHalDispSetLcdVsync);

void
gpHalDispGetLcdVsync(
	void *RegBase,
	lcdTiming_t *vsync
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispLcdVsync;
	vsync->polarity = (regVal >> 28) & 0x01;
	vsync->fPorch = (regVal >> 18) & 0x3ff;
	vsync->bPorch = (regVal >> 8)  & 0x3ff;
	vsync->width = regVal & 0xff;
}
EXPORT_SYMBOL(gpHalDispGetLcdVsync);

//(0x0e8) LCD Hsync  timing ------------------------------------------------------------------------
void
gpHalDispSetLcdHsync(
	void *RegBase,
	lcdTiming_t hsync
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d][%d][%d][%d][%d]\n", __FUNCTION__, __LINE__,
		hsync.polarity, hsync.fPorch, hsync.bPorch, hsync.width);

	hsync.polarity -= 1;
	hsync.fPorch -= 1;
	hsync.bPorch -= 1;
	hsync.width -= 1;
	dispRegBackup[DISPNUM(RegBase)].dispLcdHsync = (hsync.polarity << 28) | (hsync.fPorch << 18) | (hsync.bPorch << 8) | (hsync.width);
	pdispReg->dispLcdHsync = (hsync.polarity << 28) | (hsync.fPorch << 18) | (hsync.bPorch << 8) | (hsync.width);

}
EXPORT_SYMBOL(gpHalDispSetLcdHsync);

void
gpHalDispGetLcdHsync(
	void *RegBase,
	lcdTiming_t *hsync
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispLcdHsync;
	hsync->polarity = (regVal >> 28) & 0x01;
	hsync->fPorch = (regVal >> 18) & 0x3ff;
	hsync->bPorch = (regVal >> 8)  & 0x3ff;
	hsync->width = regVal & 0xff;

}
EXPORT_SYMBOL(gpHalDispGetLcdHsync);

//(0x0ec) LCD  timing ------------------------------------------------------------------------
void
gpHalDispSetLcdTiming(
	void *RegBase,
    UINT32 interlaced,
    UINT32 Hoffset,
    UINT32 Vtotal
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	dispRegBackup[DISPNUM(RegBase)].dispLcdTiming = (interlaced << 31) | (Hoffset << 13) | Vtotal;
	pdispReg->dispLcdTiming = (interlaced << 31) | (Hoffset << 13) | Vtotal;
}
EXPORT_SYMBOL(gpHalDispSetLcdTiming);

//(0x300) LCD TCON timing - TCONSTHAREA: TCON[0] ------------------------------------------------------------------------
void
gpHalDispSetTconSTHarea(
	void *RegBase,
	lcdTconSTVH_t tcon_stvh
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d], sth_pulse=%d, sth_area_ed=%d, sth_area_st=%d\n", __FUNCTION__, __LINE__, tcon_stvh.sth_pulse, tcon_stvh.sth_area_ed, tcon_stvh.sth_area_st );

	dispRegBackup[DISPNUM(RegBase)].dispTCON[0] = ( (tcon_stvh.sth_pulse << 28) | (tcon_stvh.sth_area_ed << 16) | (tcon_stvh.sth_area_st) );
	pdispReg->dispTCON[0] = ( (tcon_stvh.sth_pulse << 28) | (tcon_stvh.sth_area_ed << 16) | (tcon_stvh.sth_area_st) );
}
EXPORT_SYMBOL(gpHalDispSetTconSTHarea);

void
gpHalDispGetTconSTHarea(
	void *RegBase,
	lcdTconSTVH_t *tcon_stvh
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispTCON[0];
	tcon_stvh->sth_pulse = ( regVal >> 28) & 0x0f;
	tcon_stvh->sth_area_ed = ( regVal >> 16) & 0x0fff;
	tcon_stvh->sth_area_st = regVal & 0x0fff;
}
EXPORT_SYMBOL(gpHalDispGetTconSTHarea);

//(0x310) LCD TCON timing - TCONSTVHPOS: TCON[4] ------------------------------------------------------------------------
void
gpHalDispSetTconSTVH(
	void *RegBase,
	lcdTconSTVH_t tcon_stvh
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d], stv_pstn=%d, sth_pstn=%d\n", __FUNCTION__, __LINE__, tcon_stvh.stv_pstn, tcon_stvh.sth_pstn );

	dispRegBackup[DISPNUM(RegBase)].dispTCON[4] = ( (tcon_stvh.stv_pstn << 16) | (tcon_stvh.sth_pstn) );
	pdispReg->dispTCON[4] = ( (tcon_stvh.stv_pstn << 16) | (tcon_stvh.sth_pstn) );
}
EXPORT_SYMBOL(gpHalDispSetTconSTVH);

void
gpHalDispGetTconSTVH(
	void *RegBase,
	lcdTconSTVH_t *tcon_stvh
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispTCON[4];
	tcon_stvh->stv_pstn = ( regVal >> 16) & 0x0fff;
	tcon_stvh->sth_pstn = regVal & 0x0fff;
}
EXPORT_SYMBOL(gpHalDispGetTconSTVH);

//(0x304) LCD TCON timing - TCONOEH: TCON[1] ------------------------------------------------------------------------
void
gpHalDispSetTconOEH(
	void *RegBase,
	lcdTconTiming_t oeh
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d], oeh_pos_ed=%d, oeh_pos_st=%d\n", __FUNCTION__, __LINE__, oeh.pos_ed, oeh.pos_st );
	
	dispRegBackup[DISPNUM(RegBase)].dispTCON[1] = ( (oeh.pos_ed << 16) | (oeh.pos_st) );
	pdispReg->dispTCON[1] = ( (oeh.pos_ed << 16) | (oeh.pos_st) );
}
EXPORT_SYMBOL(gpHalDispSetTconOEH);

void
gpHalDispGetTconOEH(
	void *RegBase,
	lcdTconTiming_t *oeh
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispTCON[1];
	oeh->pos_ed = ( regVal >> 16) & 0x0fff;
	oeh->pos_st = regVal & 0x0fff;
}
EXPORT_SYMBOL(gpHalDispGetTconOEH);

//(0x308) LCD TCON timing - TCONOEV: TCON[2] ------------------------------------------------------------------------
void
gpHalDispSetTconOEV(
	void *RegBase,
	lcdTconTiming_t oev
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d], oev_pos_ed=%d, oev_pos_st=%d\n", __FUNCTION__, __LINE__, oev.pos_ed, oev.pos_st );

	dispRegBackup[DISPNUM(RegBase)].dispTCON[2] = ( (oev.pos_ed << 16) | (oev.pos_st) );
	pdispReg->dispTCON[2] = ( (oev.pos_ed << 16) | (oev.pos_st) );
}
EXPORT_SYMBOL(gpHalDispSetTconOEV);

void
gpHalDispGetTconOEV(
	void *RegBase,
	lcdTconTiming_t *oev
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispTCON[2];
	oev->pos_ed = ( regVal >> 16) & 0x0fff;
	oev->pos_st = regVal & 0x0fff;
}
EXPORT_SYMBOL(gpHalDispGetTconOEV);

//(0x30C) LCD TCON timing - TCONCKV: TCON[3] ------------------------------------------------------------------------
void
gpHalDispSetTconCKV(
	void *RegBase,
	lcdTconTiming_t ckv
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d], ckv_pos_ed=%d, ckv_pos_st=%d\n", __FUNCTION__, __LINE__, ckv.pos_ed, ckv.pos_st );

	dispRegBackup[DISPNUM(RegBase)].dispTCON[3] = ( (ckv.pos_ed << 16) | (ckv.pos_st) );
	pdispReg->dispTCON[3] = ( (ckv.pos_ed << 16) | (ckv.pos_st) );
}
EXPORT_SYMBOL(gpHalDispSetTconCKV);

void
gpHalDispGetTconCKV(
	void *RegBase,
	lcdTconTiming_t *ckv
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispTCON[3];
	ckv->pos_ed = ( regVal >> 16) & 0x0fff;
	ckv->pos_st = regVal & 0x0fff;
}
EXPORT_SYMBOL(gpHalDispGetTconCKV);

//(0x314) LCD TCON timing - TCONPOLVCOM: TCON[5] ------------------------------------------------------------------------
void
gpHalDispSetTconPOL(
	void *RegBase,
	lcdTconInfo_t tcon_info
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d], VCOM_ST=%d, POL_ST=%d\n", __FUNCTION__, __LINE__, tcon_info.vcom_st, tcon_info.pol_st );

	dispRegBackup[DISPNUM(RegBase)].dispTCON[5] = ( (tcon_info.vcom_st << 16) | (tcon_info.pol_st) );
	pdispReg->dispTCON[5] = ( (tcon_info.vcom_st << 16) | (tcon_info.pol_st) );
}
EXPORT_SYMBOL(gpHalDispSetTconPOL);

void
gpHalDispGetTconPOL(
	void *RegBase,
	lcdTconInfo_t *tcon_info
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispTCON[5];
	tcon_info->vcom_st = ( regVal >> 16) & 0x0fff;
	tcon_info->pol_st = regVal & 0x0fff;
}
EXPORT_SYMBOL(gpHalDispGetTconPOL);

//(0x318) LCD TCON timing - TCONCONFIG: TCON[6] ------------------------------------------------------------------------
void
gpHalDispSetTconConfig(
	void *RegBase,
	lcdTconInfo_t tcon_info
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d], tcon_en=%d, inv_en=%d, edgsl_en=%d,scan_ctl=%d\n", __FUNCTION__, __LINE__, tcon_info.tcon_en, tcon_info.inv_en, tcon_info.edgsl_en, tcon_info.scan_ctl );

	dispRegBackup[DISPNUM(RegBase)].dispTCON[6] = ( (tcon_info.tcon_en << 4) | (tcon_info.inv_en << 3) | (tcon_info.edgsl_en << 2) | (tcon_info.scan_ctl) );
	pdispReg->dispTCON[6] = ( (tcon_info.tcon_en << 4) | (tcon_info.inv_en << 3) | (tcon_info.edgsl_en << 2) | (tcon_info.scan_ctl) );
}
EXPORT_SYMBOL(gpHalDispSetTconConfig);

void
gpHalDispGetTconConfig(
	void *RegBase,
	lcdTconInfo_t *tcon_info
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispTCON[6];
	tcon_info->tcon_en = (regVal >> 4) & 0x01;
	tcon_info->inv_en = (regVal >> 3) & 0x01;
	tcon_info->edgsl_en = (regVal >> 2)  & 0x01;
	tcon_info->scan_ctl = regVal & 0x03;
}
EXPORT_SYMBOL(gpHalDispGetTconConfig);

//(0x19c) LCD LVDS timing ------------------------------------------------------------------------
void
gpHalDispSetLVDSConfig(
	void *RegBase,
	LVDSconfig_t	LVDSconfig
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	dispRegBackup[DISPNUM(RegBase)].dispLVDSCtrl = ( (LVDSconfig.HFME << 7) | (LVDSconfig.CV_R_LVDS << 4) | (LVDSconfig.CPI_LVDS << 2) | (LVDSconfig.PD_CH3_LVDS << 1) | (LVDSconfig.PD_LVDS) );
	pdispReg->dispLVDSCtrl = ( (LVDSconfig.HFME << 7) | (LVDSconfig.CV_R_LVDS << 4) | (LVDSconfig.CPI_LVDS << 2) | (LVDSconfig.PD_CH3_LVDS << 1) | (LVDSconfig.PD_LVDS) );
}
EXPORT_SYMBOL(gpHalDispSetLVDSConfig);

void
gpHalDispGetLVDSConfig(
	void *RegBase,
	LVDSconfig_t *LVDSconfig
)
{
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispLVDSCtrl;
	LVDSconfig->HFME = (regVal >> 7) & 0x01;
	LVDSconfig->CV_R_LVDS = (regVal >> 4) & 0x07;
	LVDSconfig->CPI_LVDS = (regVal >> 2) & 0x03;
	LVDSconfig->PD_CH3_LVDS = (regVal >> 1)  & 0x01;
	LVDSconfig->PD_LVDS = regVal & 0x01;
}
EXPORT_SYMBOL(gpHalDispGetLVDSConfig);

//(0x0f0) Interrupt enable  ------------------------------------------------------------------------
void
gpHalDispSetIntEnable(
	void *RegBase,
	UINT32 field
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	regVal = dispRegBackup[DISPNUM(RegBase)].dispIrqEn;
	regVal |= field;
	dispRegBackup[DISPNUM(RegBase)].dispIrqEn = regVal;
	pdispReg->dispIrqEn = regVal;
}
EXPORT_SYMBOL(gpHalDispSetIntEnable);

void
gpHalDispSetIntDisable(
	void *RegBase,
	UINT32 field
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	regVal = dispRegBackup[DISPNUM(RegBase)].dispIrqEn;
	regVal &= ~field;
	dispRegBackup[DISPNUM(RegBase)].dispIrqEn = regVal;
	pdispReg->dispIrqEn = regVal;
}
EXPORT_SYMBOL(gpHalDispSetIntDisable);

//(0x0f4) Interrupt status  ------------------------------------------------------------------------
void
gpHalDispClearIntFlag(
	void *RegBase,
	UINT32 field
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	pdispReg->dispIrqSrc = field;
}
EXPORT_SYMBOL(gpHalDispClearIntFlag);

void
gpHalDispUpdateParameter(
	void *RegBase
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	pdispReg->dispIrqSrc = 0x02;
}
EXPORT_SYMBOL(gpHalDispUpdateParameter);

//(0x0f8) Global Config ------------------------------------------------------------------------
void gpHalDispSetOsdPalette(void *RegBase, int layerNum, int startIndex, int count, const void *ColorTable)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	int maxIndex;
	int i;
	const unsigned long *src = ColorTable;
	unsigned long *dst = (unsigned long*)pdispReg->dispPalettePtr + startIndex;
	UINT32 regVal;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	maxIndex = startIndex + count;

	regVal = dispRegBackup[DISPNUM(RegBase)].dispGlobalConfig;
	if(layerNum == HAL_DISP_OSD_0) {
		regVal &= ~(0x1 << 20);
		/* The max palette index of osd0 = 256 */
		if (maxIndex > 256) maxIndex = 256;
	}
	else if (layerNum == HAL_DISP_OSD_1) {
		regVal |= (0x1 << 20);
		/* The max palette index of osd1 = 16 */
		if (maxIndex > 16) maxIndex = 16;
	}
	else {
		DERROR("[%s:%d] Error osd number %d\n", __FUNCTION__, __LINE__, layerNum);
		return;
	}

	dispRegBackup[DISPNUM(RegBase)].dispGlobalConfig = regVal;
	pdispReg->dispGlobalConfig = regVal;

	DEBUG("start index=%d, max index=%d\n", startIndex, maxIndex);
	for (i=startIndex; i<maxIndex; i++) *dst++ = *src++;
		
}
EXPORT_SYMBOL(gpHalDispSetOsdPalette);

//(0x01b0) Dynamic color matrix limit setting ------------------------------------------------------------------------
void gpHalDispSetDynCMtxInfo(void *RegBase, unsigned long dispCMatxLimit)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	pdispReg->dispCMatxLimit = dispCMatxLimit;
}
EXPORT_SYMBOL(gpHalDispSetDynCMtxInfo);

//(0x01a0-1ac) Dynamic color matrix threshold setting ------------------------------------------------------------------------
void
gpHalDispSetDynCMtxIndex(
	void *RegBase,
	UINT32 *threshold
)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
    UINT32 i;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

    for (i=0; i<4; i++)
	    pdispReg->dispCMatxThre[i] = threshold[i];
}
EXPORT_SYMBOL(gpHalDispSetDynCMtxIndex);

//(0x01e0-2fc) Dynamic color matrix table ------------------------------------------------------------------------
void gpHalDispSetDynCMtxTable(void *RegBase, const gp_disp_dyncmtxpara_t *cMatrix)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
    int i;
	volatile unsigned long *dst = (unsigned long *)pdispReg->dispCMatxPara;
	const unsigned long *src = (const unsigned long *)cMatrix;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
    for(i=0; i<sizeof(pdispReg->dispCMatxPara)/sizeof(long); i++)
		*dst++ = *src++;
}
EXPORT_SYMBOL(gpHalDispSetDynCMtxTable);

//(0x0400) Gamma Table setting ------------------------------------------------------------------------
void gpHalDispSetGammaTable(void *RegBase, int ch, const void *table)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;
	int i;
	const unsigned char *src = table;
	unsigned long *dst = (unsigned long *)pdispReg->dispGammaPtr;
	unsigned temp;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	/* Set gamma bank */
	regVal = dispRegBackup[DISPNUM(RegBase)].dispGlobalConfig;
	regVal &= ~(0x3 << 16);
	regVal |= (ch << 16);
	dispRegBackup[DISPNUM(RegBase)].dispGlobalConfig = regVal;
	pdispReg->dispGlobalConfig = regVal;

	for(i=0; i<256; i++)
	{ 
		temp = 0;	
		temp = *src++|*src++<<8|*src++<<16|*src++<<24;
		*dst++ = temp;
		//*src++;
	}
}
EXPORT_SYMBOL(gpHalDispSetGammaTable);

void gpHalDispSetMaxGamma(void *RegBase)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal, ch, i;
	volatile unsigned long *dst;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	regVal = dispRegBackup[DISPNUM(RegBase)].dispGlobalConfig;
	for (ch=0; ch<3; ch++) {
		dst = (volatile unsigned long*)pdispReg->dispGammaPtr;
		regVal &= ~(0x3 << 16);
		regVal |= (ch << 16);
		dispRegBackup[DISPNUM(RegBase)].dispGlobalConfig = regVal;
		pdispReg->dispGlobalConfig = regVal;
		for(i=0;i<256;i++) *dst++ = 0xFFFFFFFF;
	}
}
EXPORT_SYMBOL(gpHalDispSetMaxGamma);

void gpHalDispSetTVClock(void *RegBase)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
    struct clk *p_tv_clk;
	unsigned int tv_clk_rate = 0;

	if ((int)pdispReg == LOGI_ADDR_DISP_REG) {
		p_tv_clk = clk_get(NULL, "disp1");
	}
	else {
		p_tv_clk = clk_get(NULL, "disp2");
	}
	clk_enable(p_tv_clk);
	clk_set_rate(p_tv_clk, 27000000UL);
	tv_clk_rate = clk_get_rate(p_tv_clk);
}
EXPORT_SYMBOL(gpHalDispSetTVClock);
	
void gpHalDispSetClockEnable(void *RegBase, UINT32 enable)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
    struct clk *p_disp_clk;

	if ((int)pdispReg == LOGI_ADDR_DISP_REG) {
		p_disp_clk = clk_get(NULL, "disp1");
        DEBUG("[%s:%d] disp1 clock, enable = %d\n", __FUNCTION__, __LINE__, enable);
	}
	else {
		p_disp_clk = clk_get(NULL, "disp2");
        DEBUG("[%s:%d] disp2 clock, enable = %d\n", __FUNCTION__, __LINE__, enable);
	}
}
EXPORT_SYMBOL(gpHalDispSetClockEnable);

void
gpHalDispSetVDACPowerDown(
	void *RegBase,
	UINT32 enable
)
{
	#define P_SCUA_VDAC_CFG	((volatile uint32_t *)(0xfc807040))

	DEBUG("[%s:%d], %d\n", __FUNCTION__, __LINE__, enable);

	if (enable)
		*P_SCUA_VDAC_CFG = 0x1;
	else
		*P_SCUA_VDAC_CFG = 0x8;
}
EXPORT_SYMBOL(gpHalDispSetVDACPowerDown);

void gpHalDispHDMISetAudioTimingSlot(void *RegBase, int val)
{
	dispReg_t *pdispReg = (dispReg_t*)RegBase;
	UINT32 regVal;

	regVal = dispRegBackup[DISPNUM(RegBase)].dispHDMIConfig;
	if (val)	regVal |= 1 << 10;
	else		regVal &= ~(1 << 10);
	dispRegBackup[DISPNUM(RegBase)].dispHDMIConfig = regVal;
	pdispReg->dispHDMIConfig = regVal;
}
EXPORT_SYMBOL(gpHalDispHDMISetAudioTimingSlot);

void gpHalDispHDMISetHPolarity(void *RegBase, int polarity)
{
	dispReg_t *pdispReg = (dispReg_t *)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d] H polarity = %d\n", __FUNCTION__, __LINE__, polarity);
	regVal = dispRegBackup[DISPNUM(RegBase)].dispHDMIConfig;
	if (polarity)	regVal |= 1 << 12;
	else			regVal &= ~(1 << 12);
	dispRegBackup[DISPNUM(RegBase)].dispHDMIConfig = regVal;
	pdispReg->dispHDMIConfig = regVal;
}
EXPORT_SYMBOL(gpHalDispHDMISetHPolarity);

void gpHalDispHDMISetVPolarity(void *RegBase, int polarity)
{
	dispReg_t *pdispReg = (dispReg_t *)RegBase;
	UINT32 regVal;

	DEBUG("[%s:%d] V polarity = %d\n", __FUNCTION__, __LINE__, polarity);
	regVal = dispRegBackup[DISPNUM(RegBase)].dispHDMIConfig;
	if (polarity)	regVal |= 1 << 13;
	else			regVal &= ~(1 << 13);
	dispRegBackup[DISPNUM(RegBase)].dispHDMIConfig = regVal;
	pdispReg->dispHDMIConfig = regVal;
}
EXPORT_SYMBOL(gpHalDispHDMISetVPolarity);

void gpHalDispPathSelect(int mode, int path)
{
	uint32_t regVal;
	#define P_SCUB_DISP_VDAC_SEL	((volatile uint32_t *)(0xfc00518c))

	regVal = *P_SCUB_DISP_VDAC_SEL;
	regVal &= ~(1 << mode);
	regVal |= (path << mode);
	*P_SCUB_DISP_VDAC_SEL = regVal;
}
EXPORT_SYMBOL(gpHalDispPathSelect);

void gpHalDispSetClock(unsigned int freq, unsigned int pllsel)
{
	if(freq == 27000000) {
		gp_clk_set_parent((int*)"clk_ref_disp", (int*)"xtal");
		gp_clk_set_rate( (int*)"clk_ref_disp", freq );
	}
	else {
		if(pllsel==0)
			gp_clk_set_parent((int*)"clk_ref_disp", (int*)"pll0");
		else if(pllsel==2)
			gp_clk_set_parent((int*)"clk_ref_disp", (int*)"pll2");
		gp_clk_set_rate( (int*)"clk_ref_disp", freq );
	}
}
EXPORT_SYMBOL(gpHalDispSetClock);
