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
 * @file    hal_dac.c
 * @brief   Implement of AUDIO DAC HAL API.
 * @author  Dunker Chen
 */

#include <linux/kernel.h>       /* printk() */
#include <linux/device.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/hal/regmap/reg_dac.h>

//DEFINE_SPINLOCK(dac_lock);
static dacReg_t *pdacReg = (dacReg_t *)LOGI_ADDR_DAC_REG;

void gpHalDacClkEn(int enable)
{
	gp_enable_clock( (int*)"SAACC", enable );
}
EXPORT_SYMBOL(gpHalDacClkEn);

void gpHalAuddacEn(int enable)
{
	if(enable) {
		pdacReg->dacctrl &= ~SAR_HPINS_MASK;
		pdacReg->dacctrl |= SAR_HPINS_DAC | SAR_ENDAL | SAR_ENDAR | SAR_DAC_POWER;
	} else {
		pdacReg->dacctrl &= ~(SAR_ENDAL | SAR_ENDAR | SAR_DAC_POWER);
	}
}
EXPORT_SYMBOL(gpHalAuddacEn);

void gpHalAudadcEn(int enable)
{
	if(enable) {
		pdacReg->pwctrl |= SAR_ENMIC | SAR_ENMICBIAS | SAR_PGAG_MASK;
		pdacReg->adcctrl |= SAR_ENADL | SAR_VOLCTL;
	} else {
	}
}
EXPORT_SYMBOL(gpHalAudadcEn);

void gpHalDacFeqSet(int freq)
{
	switch(freq)
	{
		case 96000:
		case 88200:
		case 48000:
		case 44100:
		case 32000:
			pdacReg->dacctrl &= ~SAR_SEL_FS_MASK;
			pdacReg->dacctrl |= SAR_SEL_FS_32_48;
			break;
		case 24000:
		case 22050:
		case 16000:
			pdacReg->dacctrl &= ~SAR_SEL_FS_MASK;
			pdacReg->dacctrl |= SAR_SEL_FS_16_24;
			break;

		case 12000:
		case 11025:
		case 8000:
			pdacReg->dacctrl &= ~SAR_SEL_FS_MASK;
			pdacReg->dacctrl |= SAR_SEL_FS_08_12;
			break;
		default:
			pdacReg->dacctrl &= ~SAR_SEL_FS_MASK;
			pdacReg->dacctrl |= SAR_SEL_FS_32_48;
			break;
	}
}
EXPORT_SYMBOL(gpHalDacFeqSet);

void gpHalVrefCtrl(int enable, int fsmen)
{
	if(fsmen) {
		pdacReg->pwctrl |= SAR_VREFSM;
	} else {
		pdacReg->pwctrl &= ~SAR_VREFSM;
	}
	
	if(enable) {
		pdacReg->pwctrl |= SAR_ENVREF | SAR_ENZCD;
	} else {
		pdacReg->pwctrl &= ~SAR_ENVREF;
	}
}
EXPORT_SYMBOL(gpHalVrefCtrl);

void gpHalMicEn(int enable)
{
	if(enable) {
		pdacReg->pwctrl |= SAR_ENMIC | SAR_ENMICBIAS | SAR_BOOST | SAR_PGAG_MASK;
		pdacReg->linin &= ~(SAR_ENLNIN | SAR_ADINS);
		pdacReg->adcctrl &= ~(SAR_ENADR | SAR_ADHP);
		pdacReg->adcctrl |= SAR_ENADL | SAR_VOLCTL;
	} else {
		pdacReg->pwctrl &= ~(SAR_ENMIC | SAR_ENMICBIAS | SAR_BOOST);
		pdacReg->linin |= SAR_ADINS;
		pdacReg->adcctrl &= ~(SAR_ADHP | SAR_ENADR | SAR_ENADL);
	}
}
EXPORT_SYMBOL(gpHalMicEn);

void gpHalMicVolGet(long *vol)
{
	unsigned int val;
	
	val = pdacReg->pwctrl;
	*vol = ( val & SAR_PGAG_MASK ) >> 6;
}
EXPORT_SYMBOL(gpHalMicVolGet);

void gpHalMicVolSet(long vol)
{
	unsigned int val;
	val = pdacReg->pwctrl;
	val &= ~(SAR_PGAG_MASK);
	val |= (vol&0x1f)<<6;
	pdacReg->pwctrl = val;
}
EXPORT_SYMBOL(gpHalMicVolSet);


void gpHalLininEn(int enable)
{
	if(enable) {
		pdacReg->linin |= SAR_ENLNIN | SAR_ADINS;
		pdacReg->adcctrl |= SAR_ADHP | SAR_ENADR | SAR_ENADL;
		pdacReg->pwctrl &= ~(SAR_ENMICBIAS | SAR_ENMIC);
	} else {
		pdacReg->linin &= ~(SAR_ENLNIN | SAR_ADINS);
		pdacReg->adcctrl &= ~(SAR_ADHP | SAR_ENADR | SAR_ENADL);
	}
}
EXPORT_SYMBOL(gpHalLininEn);

void gpHalLininVolSet(long l_vol, long r_vol)
{
	unsigned int val;
	
	val = pdacReg->linin;
	val = (val&(~SAR_LNINLG_MASK))|((l_vol&(0x1f))<<2);
	val = (val&(~SAR_LNINRG_MASK))|((r_vol&(0x1f))<<7);
	pdacReg->linin = val;
}
EXPORT_SYMBOL(gpHalLininVolSet);

void gpHalLininVolGet(long *l_vol, long *r_vol)
{
	unsigned int val;
	
	val = pdacReg->linin;
    *l_vol = ((val & SAR_LNINLG_MASK) >> 2);
	*r_vol = ((val & SAR_LNINRG_MASK) >> 7);
}
EXPORT_SYMBOL(gpHalLininVolGet);

void gpHalHdphnEn(int enable)
{
	if(enable) {
		pdacReg->dacctrl |= SAR_DAC_POWER;					//enable power control
		pdacReg->hdphone |= SAR_HLG_MASK | SAR_HRG_MASK;	//mute
		pdacReg->hdphone |= SAR_ENHPL | SAR_ENHPR;
	} else {
		pdacReg->hdphone |= SAR_HLG_MASK | SAR_HRG_MASK;
		pdacReg->hdphone &= ~(SAR_ENHPL | SAR_ENHPR);
	}
}
EXPORT_SYMBOL(gpHalHdphnEn);

void gpHalHdphnVolGet(long *l_vol, long *r_vol)
{
	unsigned int val;
	
	val = pdacReg->hdphone;
    *l_vol = ((val & SAR_HLG_MASK) >> 2);
	*r_vol = ((val & SAR_HRG_MASK) >> 7);
}
EXPORT_SYMBOL(gpHalHdphnVolGet);

void gpHalHdphnVolSet(long l_vol, long r_vol)
{
	unsigned int val;
	pdacReg->dacctrl |= SAR_DAC_POWER;
	
	val = pdacReg->hdphone;
	val = (val&(~SAR_HLG_MASK))|((l_vol)<<2);
	val = (val&(~SAR_HRG_MASK))|((r_vol)<<7);
//	val |= (SAR_ENHPL | SAR_ENHPR);
	pdacReg->hdphone = val;
}
EXPORT_SYMBOL(gpHalHdphnVolSet);

void gpHalHdphnMuteGet(long *l_mute, long *r_mute)
{
	unsigned int val;
	
	val = pdacReg->hdphone;
	*l_mute = !!(val & SAR_ENHPL);
	*r_mute = !!(val & SAR_ENHPR);
}
EXPORT_SYMBOL(gpHalHdphnMuteGet);

void gpHalHdphnMuteSet(long l_mute, long r_mute)
{
	unsigned int val;

	val = pdacReg->hdphone;
	val |= (SAR_ENHPL | SAR_ENHPR);
	if (!l_mute)
		val &= (~SAR_ENHPL);
	if (!r_mute)
		val &= (~SAR_ENHPR);
	pdacReg->hdphone = val;
}
EXPORT_SYMBOL(gpHalHdphnMuteSet);

unsigned int gpHalHpinsGet(void)
{
	unsigned int val;

	val = pdacReg->dacctrl;
	val = (val & SAR_HPINS_MASK) >> 6;

	return val;
}
EXPORT_SYMBOL(gpHalHpinsGet);

void gpHalHpinsSet(unsigned int hpins)
{
	unsigned int val;

	val = pdacReg->dacctrl;
	val &= ~SAR_HPINS_MASK;
	val |= hpins << 6;
	pdacReg->dacctrl = val;
}
EXPORT_SYMBOL(gpHalHpinsSet);

unsigned int gpHalAdinsGet(void)
{
	unsigned int val;

	val = pdacReg->linin;
	val = (val & SAR_ADINS) >> 1;

	return val;
}
EXPORT_SYMBOL(gpHalAdinsGet);

void gpHalHdphnDischarge(void)
{
	pdacReg->dacctrl |= SAR_EQ_GND | SAR_DITHER;
}
EXPORT_SYMBOL(gpHalHdphnDischarge);


/*void gpHalHdphnmute()
{
	int temp;
	pdacReg->dacctrl |= SAR_DAC_POWER;
	pdacReg->hdphone = SAR_HLG_MASK | SAR_HRG_MASK;
	
	temp = pdacReg->hdphone;
}
EXPORT_SYMBOL(gpHalHdphnmute);*/


/*void gpHalLinoutVolset()
{
	pdacReg->linout = 0x10b;
}
EXPORT_SYMBOL(gpHalLinoutVolset);

void gpHalAdcSetGND()
{
	pdacReg->dacctrl &= ~SAR_EQ_VREF;
	pdacReg->dacctrl |= SAR_EQ_GND;
}
EXPORT_SYMBOL(gpHalAdcSetGND);

void gpHalAdcSetVref()
{
	pdacReg->dacctrl &= ~SAR_EQ_GND;
	pdacReg->dacctrl |= SAR_EQ_VREF;
}
EXPORT_SYMBOL(gpHalAdcSetVref);*/
