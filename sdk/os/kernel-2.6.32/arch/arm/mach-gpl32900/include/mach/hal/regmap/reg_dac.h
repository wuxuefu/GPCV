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
 
#ifndef _REG_DAC_H_
#define _REG_DAC_H_

#include <mach/hardware.h>
#include <mach/typedef.h>

#define	LOGI_ADDR_DAC_REG		(IO3_BASE + 0x1F020)
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct dacReg_s {
	/* volatile UINT8 regOffset[0xB03000]; */ /* 0x92B03000 */
	volatile UINT32 pwctrl;			/* 0x0000 ~ 0x0003 */
	volatile UINT32 linin;			/* 0x0004 ~ 0x0007 */
	volatile UINT32 adcctrl;		/* 0x0008 ~ 0x000B */
	volatile UINT32 dacctrl;		/* 0x000C ~ 0x000F */
	volatile UINT32 autoslp;		/* 0x0010 ~ 0x0013 */
	volatile UINT32 linout;			/* 0x0014 ~ 0x0017 */
	volatile UINT32 hdphone;		/* 0x0018 ~ 0x001B */
	volatile UINT32 tadda;			/* 0x001C ~ 0x001F */
	volatile UINT32 rsv020[8];		/* 0x0020 ~ 0x004F */
	volatile UINT32 ovrlv;			/* 0x0050 ~ 0x0053 */
	volatile UINT32 ovrinten;		/* 0x0054 ~ 0x0057 */
	volatile UINT32 ovrintclr;		/* 0x0058 ~ 0x005B */
	volatile UINT32 ovrdeb;			/* 0x005C ~ 0x005F */
} dacReg_t;

//------PWCTRL register----------------------------
#define SAR_ENZCD					(1<<0)     	//gain is updated while signal crossing zero
#define SAR_ENVREF					(1<<1)     	//VREF power control
#define SAR_VREFSM					(1<<2)     	//VREF fast setup mode
#define SAR_ENMICBIAS				(1<<3)     	//Microphone bias-voltage output power control
#define SAR_ENMIC					(1<<4)     	//MIC power control
#define SAR_BOOST					(1<<5)     	//boost amplifier gain control
#define SAR_PGAG_MASK				(0x1F<<6)  	//mute mask

//------HDPHN register------------------------
#define SAR_ENHPL					(1<<0)		//left channel headphone amplifier power control
#define SAR_ENHPR					(1<<1)		//right channel headphone amplifier power control
#define SAR_HLG_MASK				(0x1F<<2)	//left channel headphone volume control
#define SAR_HRG_MASK				(0x1F<<7)	//right channel headphone volume control

//------ADCCTRL register read only---------------------------
#define SAR_ENADL               	(1<<0)     	//Left channel ADC power control
#define SAR_ENADR               	(1<<1)     	//Right channel ADC power control
#define SAR_ADHP  	             	(1<<2)     	//Audio ADC high pass filter control
#define SAR_ADOVRS_LMT84            (0x0<<3)   	//ADC input limit range 0.84*fullrange
#define SAR_ADOVRS_LMT71            (0x1<<3)   	//ADC input limit range 0.71*fullrange
#define SAR_ADOVRS_LMT60            (0x2<<3)   	//ADC input limit range 0.60*fullrange
#define SAR_ADOVRS_LMT50            (0x3<<3)   	//ADC input limit range 0.50*fullrange
#define SAR_ADOVRS_MASK            	(0x3<<3)    //ADC input limit range 0.50*fullrange
#define SAR_ADLOVP               	(1<<5)  	//when high,it means left ADC input is over ADC'positive input ranges
#define SAR_ADLOVN               	(1<<6)  	//when high,it means left ADC input is over ADC's negative input range
#define SAR_ADROVP               	(1<<7)   	//when high,it means right ADC input is over ADC's positive input range
#define SAR_ADROVN               	(1<<8)		//when high,it means right ADC input is over ADC's negative input range
#define	SAR_VOLCTL					(1<<9)		//Add by Simon

//------DACCTRL register------------------------
#define SAR_ENDAL					(1<<0)		//audio left channel DAC power control
#define SAR_ENDAR					(1<<1)		//audio right channel DAC power control
#define SAR_FORMAT_IIS				(1<<2)		//format selection 0:normal format,1:IIS format
#define SAR_SEL_FS_32_48			(0x0<<3)	//DAC sample rate select:32~48K
#define SAR_SEL_FS_16_24			(0x1<<3)	//DAC sample rate select:16~24K
#define SAR_SEL_FS_08_12			(0x2<<3)	//DAC sample rate select:08~12K
#define SAR_SEL_FS_MASK				(0x3<<3)	//DAC sample rate select:mask
#define SAR_BPFIR					(1<<5)		//bypass DAC digital filter
#define SAR_HPINS_DAC				(0x0<<6)	//audio driver input 00:DAC
#define SAR_HPINS_MIC				(0x1<<6)	//audio driver input 01:MIC
#define SAR_HPINS_LININ				(0x2<<6)	//audio driver input 10:line-in
#define SAR_HPINS_MASK				(0x3<<6)	//audio driver input mask
#define	SAR_DITHER					(0x1<<8)
#define	SAR_EQ_GND					(0x2<<9)
#define	SAR_EQ_VREF					(0x1<<9)
#define SAR_DAC_POWER				(1<<11)		//

//------LININ register-------------------------------
#define SAR_ENLNIN					(1<<0)		//Line in power control
#define SAR_ADINS					(1<<1)		//Left channel ADC input select(0:mic,1:lineLeft)
#define SAR_LNINLG_MASK				(0x1F<<2)	//Left channel line-in gain
#define SAR_LNINRG_MASK				(0x1F<<7)   //Right channel line-in gain

#endif /* _REG_DAC_H_ */
