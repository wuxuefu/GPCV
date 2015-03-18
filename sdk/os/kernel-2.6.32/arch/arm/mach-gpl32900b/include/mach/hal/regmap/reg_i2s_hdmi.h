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
#ifndef _REG_I2S_HDMI_H_
#define _REG_I2S_HDMI_H_

#include <mach/hardware.h>                          
#include <mach/typedef.h>

#define	LOGI_ADDR_I2S_HDMI_REG		IO2_ADDRESS(0x12000)

/* For Tx Control Register (ISRC) */
enum I2STX_ISCR_REG {
	/* Tx Control Register */
	I2STX_MASK_EN_I2S_TX = 0x1,
	I2STX_MASK_FirstFrameLR = 0x2,
	I2STX_MASK_FramePolarity = 0x4,
	I2STX_MASK_EdgeMode = 0x8,
	I2STX_MASK_SendMode = 0x10,
	I2STX_MASK_NormalModeAlign = 0x20,
	I2STX_MASK_ValidDataMode = 0x1C0,
	I2STX_MASK_FrameSizeMode = 0x600,
	I2STX_MASK_I2SMode = 0x1800,
	I2STX_MASK_SLVMode = 0x2000,
	I2STX_MASK_IRT_Polarity = 0x8000,
	I2STX_MASK_EN_IRT = 0x10000,
	I2STX_MASK_IRT_FLAG = 0x20000,
	I2STX_MASK_ClrFIFO = 0x40000,
	I2STX_MASK_MERGE = 0x100000,
	I2STX_MASK_R_LSB = 0x200000,
	I2STX_MASK_EQ = 0x800000,
	I2STX_MASK_I2S_OUT_ENABLE = 0x1000000,
	I2STX_MASK_HDMI_OUT_ENABLE = 0x2000000,

	I2STX_EN = 0x0001,
	I2STX_FirstFrame_Right = 0x0002,		/* Right frame first */
	I2STX_FirstFrame_Left =	0x0000,
	I2STX_FramePolarity_Left = 0x0004,	/* Left frame polarity */
	I2STX_FramePolarity_Right = 0x0000,
	I2STX_EdgeMode_Rising = 0x0008,		/* rising edge */
	I2STX_EdgeMode_falling = 0x0000,		
	I2STX_SendMode_LSB = 0x0010,		/* LSB first */
	I2STX_SendMode_MSB = 0x0000,		/* MSB first */
	I2STX_NormalModeAlign_Left = 0x0020,		/* Left align */
	I2STX_NormalModeAlign_Right = 0x0000,		/* Left align */
	I2STX_WordLength_16BIT = 0x0000,
	I2STX_WordLength_18BIT = 0x0040,
	I2STX_WordLength_20BIT = 0x0080,
	I2STX_WordLength_22BIT = 0x00C0,
	I2STX_WordLength_24BIT = 0x0100,
	I2STX_WordLength_32BIT = 0x0180,
	I2STX_FrameSize_16BIT = 0x0000,
	I2STX_FrameSize_24BIT = 0x0200,
	I2STX_FrameSize_32BIT = 0x0400,
	I2STX_FrameSize_Slave = 0x0600,
	I2STX_Mode_I2Smode = 0x0000,
	I2STX_Mode_Normal = 0x0800,
	I2STX_Mode_DSP = 0x1000,
	I2STX_Mode_DSP_2 = 0x1800,
	I2STX_SLVMode_Tx_Master = 0x0000,
	I2STX_SLVMode_Tx_Slave = 0x2000,
	I2STX_IntPolarity_High = 0x8000,		/* interrupt is high active */
	I2STX_IntPolarity_Low = 0x0000,		/* interrupt is Low active */
	I2STX_EN_IRT = 0x10000,		/* HIgh Active: Tx interrupt enable bit */
	I2STX_IRT_FLAG = 0x20000,	/* High Active: Tx interrupt flag bit, write 1 clear */
	I2STX_ClrFIFO = 0x40000,	/* High active Tx FIFO, automatically clear to 0 after FIFO cleared. */
	I2STX_MERGE = 0x100000,
	I2STX_R_LSB = 0x200000,
	I2STX_EQ = 0x800000,
	I2STX_I2S_OUT_ENABLE = 0x1000000,
	I2STX_HDMI_OUT_ENABLE = 0x2000000
};

/* For OCP DMA Register (OCP_CONFIG) */
enum I2STX_OCP_CONFIG_REG{
	/* Tx OCP_CONFIG Register */   
	I2STX_OCP_MASK_ocp_en = 0x1,
	I2STX_OCP_MASK_cmd_accept_en = 0x2,
	I2STX_OCP_MASK_double_buf = 0x4,
	I2STX_OCP_MASK_sdatabyteen = 0x78,
	I2STX_OCP_MASK_sburstlength = 0xFF80,
	I2STX_OCP_MASK_cahnnel_info = 0xF0000,
	I2STX_OCP_MASK_en_ocp_int = 0x200000,

	I2STX_OCP_ocp_en = 0x1,
	I2STX_OCP_double_buf = 0x4,
	I2STX_OCP_sdatabyteen_16 = 0x18,
	I2STX_OCP_sdatabyteen_24 = 0x38,
	I2STX_OCP_sdatabyteen_32 = 0x78,
	I2STX_OCP_sdatabyteen_merge = 0x78,
	I2STX_OCP_sburstlength_32 = 0x800,
	I2STX_OCP_cahnnel_info_2 = 0x10000,
	I2STX_OCP_cahnnel_info_2_1 = 0x20000,
	I2STX_OCP_cahnnel_info_5_1 = 0x40000,
	I2STX_OCP_cahnnel_info_7_1 = 0x60000,   
	I2STX_OCP_en_ocp_int = 0x200000
};

/* For SPDIF Config Register */
enum SPDIF_XX_REG{
	/* Tx SPDIF Register */      
	SPDIF_MASK_xxx_auxiliary_l = 0x1,
	SPDIF_MASK_xxx_auxiliary_data_l = 0x1E,
	SPDIF_MASK_xxx_valid_flag_l = 0x20,
	SPDIF_MASK_xxx_user_data_l = 0x40,
	SPDIF_MASK_xxx_channel_status_l = 0x80,
	SPDIF_MASK_xxx_auxiliary_r = 0x100,
	SPDIF_MASK_xxx_auxiliary_data_r = 0x1E00,
	SPDIF_MASK_xxx_valid_flag_r = 0x2000,
	SPDIF_MASK_xxx_user_data_r = 0x4000,
	SPDIF_MASK_xxx_channel_status_r = 0x8000
};

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct i2shdmiReg_s {
	volatile UINT32 ctl;
	volatile UINT32 reserve0;
	volatile UINT32 pos_set0;
	volatile UINT32 pos_set1;
	volatile UINT32 ocp_cfg;
	volatile UINT32 ch0_start_addr;
	volatile UINT32 ch0_end_addr;
	volatile UINT32 ch1_start_addr;
	volatile UINT32 ch1_end_addr;
	volatile UINT32 reserve1;
	volatile UINT32 ocp_int_read;
	volatile UINT32 reserve2[0x5];
	volatile UINT32 spdif_ctl;
	volatile UINT32 reserve3;
	volatile UINT32 spdif_lf_cfg;
	volatile UINT32 reserve4[0xD];
	volatile UINT32 spdif_lch0;
	volatile UINT32	spdif_lch1;
	volatile UINT32	spdif_rch0;
	volatile UINT32	spdif_rch1;
} i2shdmiReg_t;

#endif