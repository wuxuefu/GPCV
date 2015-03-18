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

#ifndef _HAL_I2S_HDMI_H_
#define _HAL_I2S_HDMI_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

enum {
	CH_2,
	CH_2_1,
	CH_5_1,
	CH_7_1
};

enum {
	BIT_16,
	BIT_24,
	BIT_32
};
 
/* for HDMI */
enum {
	a = 0x0,
	b = 0x0,
	c = 0x4,
	d = 0x10,
	Mode = 0x00
};

enum Source_number {
	Source_number_00 = 0x00000,
	Source_number_01 = 0x10000,
	Source_number_02 = 0x20000,
	Source_number_03 = 0x30000,
	Source_number_04 = 0x40000,
	Source_number_05 = 0x50000,
	Source_number_06 = 0x60000,
	Source_number_07 = 0x70000,
	Source_number_08 = 0x80000,
	Source_number_09 = 0x90000,
	Source_number_10 = 0xA0000,
	Source_number_11 = 0xB0000,
	Source_number_12 = 0xC0000,
	Source_number_13 = 0xD0000,
	Source_number_14 = 0xE0000,
	Source_number_15 = 0xF0000
};

enum Channel_number {
	Channel_number_none = 0x000000,
	Channel_number_stereo_left = 0x100000,
	Channel_number_stereo_right = 0x200000,
	Channel_number_center = 0x300000,
	Channel_number_sub = 0x400000,
	Channel_number_lfs = 0x500000,
	Channel_number_rfs = 0x600000,
	Channel_number_lbs = 0x700000,
	Channel_number_rbs = 0x800000
};

enum Sampling {
	Sampling_22K = 0x4000000,
	Sampling_44K = 0x0000000,
	Sampling_88K = 0x8000000,
	Sampling_176K = 0xC000000,
	Sampling_24K = 0x6000000,
	Sampling_48K = 0x2000000,
	Sampling_96K = 0xA000000,
	Sampling_192K = 0xE000000,
	Sampling_32K = 0x3000000,
	Sampling_none = 0x1000000,
	Sampling_768K = 0x9000000
};

enum Clock_accuracy {
	Clock_LvII = 0x00000000,
	Clock_LvI = 0x10000000,
	Clock_LvIII = 0x20000000,
	Clock_no_match = 0x30000000
};

enum Word_length {
	Word0_no_match = (0x0<<1)+0,
	Word0_16bit = (0x1<<1)+0,
	Word0_18bit = (0x2<<1)+0,
	Word0_19bit = (0x4<<1)+0,
	Word0_20bit = (0x5<<1)+0,
	Word0_17bit = (0x6<<1)+0,
	Word1_no_match = (0x0<<1)+1,
	Word1_20bit = (0x1<<1)+1,
	Word1_22bit = (0x2<<1)+1,
	Word1_23bit = (0x4<<1)+1,
	Word1_24bit = (0x5<<1)+1,
	Word1_21bit = (0x6<<1)+1
};

enum Original_sampling {
	Original_sampling_no_match = 0x00,
	Original_sampling_16K = 0x80,
	Original_sampling_Reserved1 = 0x40,
	Original_sampling_32K = 0xC0,
	Original_sampling_12K = 0x20,
	Original_sampling_11K = 0xA0,
	Original_sampling_8K = 0x60,
	Original_sampling_Reserved2 = 0xE0,
	Original_sampling_192K = 0x10,
	Original_sampling_24K = 0x90,
	Original_sampling_96K = 0x50,
	Original_sampling_48K = 0xD0,
	Original_sampling_176K = 0x30,
	Original_sampling_22K = 0xB0,
	Original_sampling_88K = 0x70,
	Original_sampling_44K = 0xF0
};

enum CGMS_A {
	CGMS_A_Copy = 0x000,
	CGMS_A_Condition = 0x100,
	CGMS_A_One = 0x200,
	CGMS_A_NoCopy = 0x300
};

enum {
	DMA_BUFFER_A = 1,
	DMA_BUFFER_B = 2
};
 
void gpHalI2sHdmiInit(int reset);
void gpHalI2sHdmiSetfmt(int fmt);
void gpHalI2sHdmiChlSet(int ch);
void gpHalI2sHdmiChanStatus(int ch_mode, int bit_num, int sampling_rate);
int gpHalOcpdmaClearIRQFlag(void);
void gpHalOcpdmaSetBuf(int buf_num, char* addr, int ln);
void gpHalOcpdmaIntEnable(void);
void gpHalI2sHdmiClkdiv(int sample);
void gpHalI2sHdmiDisable(void);
void gpHalHdmiI2sClkEn(int enable);

#endif
