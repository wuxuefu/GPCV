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
 * @file    hal_i2s_hdmi.c
 * @brief   Implement of I2S HAL API.
 * @author  Simon Hsu
 */

#include <linux/kernel.h>       /* printk() */
#include <linux/device.h>
#include <mach/audio/soundcard.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/hal/hal_clock.h>
#include <mach/hal/hal_i2s_hdmi.h>
#include <mach/hal/regmap/reg_i2s_hdmi.h>
#include <mach/hal/regmap/reg_scu.h>

static i2shdmiReg_t *pi2shdmiReg = (i2shdmiReg_t *)LOGI_ADDR_I2S_HDMI_REG;


#define P_SCUB_CLK6_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x468))
#define P_SCUB_CLK6_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x46C))


/**
 * @brief I2STX Mode Channel Get
 *
 * @param i2s_base I2S Registers base address 
 * @return channel number symbol
 */
static int I2STX_ChannelGet()
{
	register unsigned int val;
	int ch_sym = CH_2;
   
	val = pi2shdmiReg->ocp_cfg;
	val >>= 16;
	val &= 0xF;
	switch(val)
	{
		case 1:	/*0x0000*/
			ch_sym = CH_2;
		break;
		case 2:	/*0x0010*/
			ch_sym = CH_2_1;
		break;
		case 4:	/*0x0100*/
			ch_sym = CH_5_1;
		break;
		case 6:	/*0x0110*/
			ch_sym = CH_7_1;
		break;
		default:
			ch_sym = CH_2;
		break;
	}
	return ch_sym;
}

/**
 * @brief I2STX Mode Bit Get
 *
 * @param i2s_base I2S Registers base address 
 * @return bit number symbol
 */
static int I2STX_BitGet()
{
	register unsigned int val;
	int bit_sym = BIT_16;
   
	val = pi2shdmiReg->ctl;
	val >>= 6;
	val &= 7;
	switch(val)
	{
		case 0:
			bit_sym = BIT_16;
		break;
		case 4:
			bit_sym = BIT_24;
		break;
		case 5:
		case 6:
		case 7:
			bit_sym = BIT_32;
		break;
		default:
			bit_sym = BIT_16;
		break;
	}
	return bit_sym;
}

/**
 * @brief I2STX Mode Channel Set
 *
 * @param i2s_base I2S Registers base address
 * @param ch_mode CH_2/CH_2_1/CH_5_1/CH_7_1
 * @param bit_num BIT_16/BIT_24/BIT_32	 
 * @retval 0 PASS
 * @retval 1 FAIL
 */
static int I2STX_ChannelMode(int ch_mode, int bit_num)
{
	register unsigned int val;
	register unsigned int order;
   
	val = pi2shdmiReg->ocp_cfg;
	val &= (~I2STX_OCP_MASK_cahnnel_info);
	switch(ch_mode)
	{
		case CH_7_1:
			val |= I2STX_OCP_cahnnel_info_7_1;
			order = 0x67014523;
		break;
		case CH_5_1:
			val |= I2STX_OCP_cahnnel_info_5_1;
			order = 0xFF014523;
		break;
		case CH_2_1:
			val |= I2STX_OCP_cahnnel_info_2_1;
			order = 0xFF01FF2F;
		break;
		case CH_2:
		default:
			val |= I2STX_OCP_cahnnel_info_2;
			order = 0xFF01FFFF;
		break; 
	}
	pi2shdmiReg->pos_set0 = order;
	pi2shdmiReg->ocp_cfg = val;
   
	val = pi2shdmiReg->ocp_cfg;
	val &= (~I2STX_OCP_MASK_sdatabyteen);
	switch(bit_num)
	{
		case BIT_32:
			val |= I2STX_OCP_sdatabyteen_32;
		break;
		case BIT_24:
			val |= I2STX_OCP_sdatabyteen_24;
		break;
		case BIT_16:
		default:
			val |= I2STX_OCP_sdatabyteen_16;
		break;
	}
	pi2shdmiReg->ocp_cfg = val;

	val = pi2shdmiReg->ctl;
	val &= (~(I2STX_MASK_ValidDataMode|I2STX_MASK_MERGE));	// clear WordLength, MergeMode
	switch(bit_num)
	{
		case BIT_32:
			val |= (I2STX_WordLength_32BIT);
		break;
		case BIT_24:
			val |= (I2STX_WordLength_24BIT);
		break;
		case BIT_16:
		default:
			val |= (I2STX_WordLength_16BIT);
		break;
	}
	pi2shdmiReg->ctl = val;

	return 0;
}

/**
 * @brief I2STX Merge Mode Channel Set
 *
 * @param i2s_base I2S Registers base address
 * @param ch_mode CH_2/CH_2_1/CH_5_1
 * @retval 0 PASS
 * @retval 1 FAIL
 */
static int I2STXMERGE_ChannelMode(int ch_mode)
{
	register unsigned int val;
	register unsigned int order;
   
	val = pi2shdmiReg->ocp_cfg;
	val &= (~(I2STX_OCP_MASK_cahnnel_info | I2STX_OCP_MASK_sdatabyteen));
	val |= I2STX_OCP_sdatabyteen_merge;
	switch(ch_mode)
	{
		case CH_7_1:
			val |= I2STX_OCP_cahnnel_info_7_1;
			order = 0x33002211;
		break;
		case CH_5_1:
			val |= I2STX_OCP_cahnnel_info_5_1;
			order = 0xFF110022;
		break;
		case CH_2_1:
			val |= I2STX_OCP_cahnnel_info_2_1;
			order = 0xFF00FF1F;
		break;
		case CH_2:
		default:
			val |= I2STX_OCP_cahnnel_info_2;
			order = 0xFF00FFFF;
		break; 
	}
	pi2shdmiReg->pos_set0 = order;
	pi2shdmiReg->ocp_cfg = val;

	val = pi2shdmiReg->ctl;
	val &= (~(I2STX_MASK_ValidDataMode));
	val |= (I2STX_WordLength_16BIT|I2STX_MERGE);
	pi2shdmiReg->ctl = val;   

	return 0;
}

void gpHalI2sHdmiClkdiv(int sample)
{
	/* i2s tx1 clock divider setting */
	if(sample==48000 || sample==44100)
		P_SCUB_CLK6_CFG2 = 0x18;
	else if(sample==22050)
		P_SCUB_CLK6_CFG2 = 0x30;
	P_SCUB_CLK6_CFG3 = 0x1;
}
EXPORT_SYMBOL(gpHalI2sHdmiClkdiv);

void gpHalI2sHdmiDisable(void)
{
	register unsigned int val;
	
	val = I2STX_FirstFrame_Left|I2STX_FramePolarity_Left|I2STX_EdgeMode_falling|I2STX_SendMode_MSB|I2STX_NormalModeAlign_Left|I2STX_Mode_I2Smode|I2STX_SLVMode_Tx_Master|I2STX_IntPolarity_High|I2STX_FrameSize_32BIT;
	pi2shdmiReg->ctl = val;

	/** config OCP_CONFIG */
	val = (I2STX_OCP_cahnnel_info_2 | I2STX_OCP_sdatabyteen_16);		/* channel_info = 2CH,  sdatabyteen = 16 bit */
	val |= (I2STX_OCP_double_buf | I2STX_OCP_sburstlength_32);
	pi2shdmiReg->ocp_cfg = val;
}
EXPORT_SYMBOL(gpHalI2sHdmiDisable);

void gpHalI2sHdmiInit(int reset)
{
	register unsigned int val;
	register unsigned int en_bit;
	
	if (reset)
	{
		val = I2STX_FirstFrame_Left|I2STX_FramePolarity_Left|I2STX_EdgeMode_falling|I2STX_SendMode_MSB|I2STX_NormalModeAlign_Left|I2STX_Mode_I2Smode|I2STX_SLVMode_Tx_Master|I2STX_IntPolarity_High|I2STX_FrameSize_32BIT;
		pi2shdmiReg->ctl = val;

		/** config OCP_CONFIG */
		val = (I2STX_OCP_cahnnel_info_2 | I2STX_OCP_sdatabyteen_16);		/* channel_info = 2CH,  sdatabyteen = 16 bit */
		val |= (I2STX_OCP_double_buf | I2STX_OCP_sburstlength_32);
		pi2shdmiReg->ocp_cfg = val;
	}
   
   /* different kind of output */
	val = pi2shdmiReg->ctl;
	val &= (~I2STX_I2S_OUT_ENABLE);
	val |= I2STX_HDMI_OUT_ENABLE | I2STX_EN;
	en_bit = 0x0;

	pi2shdmiReg->spdif_ctl = en_bit;
	pi2shdmiReg->ctl = val;
}
EXPORT_SYMBOL(gpHalI2sHdmiInit);

void gpHalI2sHdmiChlSet(int ch)
{
	register unsigned int bit_sym = I2STX_BitGet();
	int ch_sym = CH_2;

	switch(ch)
	{
		case 2:
			ch_sym = CH_2;
		break;
		case 3:
			ch_sym = CH_2_1;
		break;
		case 6:
			ch_sym = CH_5_1;
		break;
		case 8:
			ch_sym = CH_7_1;
		break;
		default:
			ch_sym = CH_2;
		break;
	}

	if (bit_sym == BIT_16)
		I2STXMERGE_ChannelMode(ch_sym);
	else
		I2STX_ChannelMode( ch_sym, bit_sym );
}
EXPORT_SYMBOL(gpHalI2sHdmiChlSet);

void gpHalI2sHdmiFrameOrderSet(int order)
{
	register unsigned int val = pi2shdmiReg->ctl;

	if (order == 0) 
		val = val | (I2STX_SendMode_LSB);
	else
		val = val & (~I2STX_SendMode_LSB);

	pi2shdmiReg->ctl = val;
}

void gpHalI2sHdmiBitSet(int bit)
{
	register unsigned int ch_sym = I2STX_ChannelGet();
	int bit_sym = BIT_16;

	switch(bit)
	{
		case AFMT_S32_LE:
		case AFMT_S32_BE:
			bit_sym = BIT_32;
		break;
		case AFMT_S24_LE:
		case AFMT_S24_BE:
			bit_sym = BIT_24;
		break;
		case AFMT_S16_LE:
		case AFMT_S16_BE:
		default:
			bit_sym = BIT_16;
		break;
	}

	if (bit_sym == BIT_16)
		I2STXMERGE_ChannelMode( ch_sym);
	else
		I2STX_ChannelMode( ch_sym, bit_sym );
}

void gpHalI2sHdmiSetfmt(int fmt)
{
	if(fmt==AFMT_S16_LE)
	{
		gpHalI2sHdmiFrameOrderSet(1);
		gpHalI2sHdmiBitSet(fmt);
	}
	else
	{
		gpHalI2sHdmiFrameOrderSet(0);
		gpHalI2sHdmiBitSet(fmt);		
	}
}
EXPORT_SYMBOL(gpHalI2sHdmiSetfmt);

void gpHalI2sHdmiChanStatus(int ch_mode, int bit_num, int sampling_rate)
{
	int ret = 0;
	int sampling = 0;
	int sampling_org = 0;
   
	printk("[%s]: CHAN=%d, BIT=%d, RATE=%d\n",__FUNCTION__,ch_mode,bit_num,sampling_rate);
	switch (sampling_rate)
	{
		case 48000:
			sampling = Sampling_48K;
			sampling_org = Original_sampling_48K;
		break;
		case 44100:
			sampling = Sampling_44K;
			sampling_org = Original_sampling_44K;
		break;
		case 22050:
			sampling = Sampling_22K;
			sampling_org = Original_sampling_22K;
		break;
	}
   
	switch (ch_mode)
	{
		case CH_2:
		default:
			if (bit_num == BIT_16)
			{
				pi2shdmiReg->spdif_lf_cfg = 0x6060;
				pi2shdmiReg->spdif_lch0 = a|b|c|d|Mode|Source_number_02|Channel_number_stereo_left|sampling|Clock_LvII;
				pi2shdmiReg->spdif_lch1 = Word0_16bit|sampling_org|CGMS_A_Copy;
				pi2shdmiReg->spdif_rch0 = a|b|c|d|Mode|Source_number_02|Channel_number_stereo_right|sampling|Clock_LvII;
				pi2shdmiReg->spdif_rch1 = Word0_16bit|sampling_org|CGMS_A_Copy;
			}
			if (bit_num == BIT_24)
			{
				pi2shdmiReg->spdif_lf_cfg = 0x6060;
				pi2shdmiReg->spdif_lch0 = a|b|c|d|Mode|Source_number_02|Channel_number_stereo_left|sampling|Clock_LvII;
				pi2shdmiReg->spdif_lch1 = Word1_24bit|sampling_org|CGMS_A_Copy;
				pi2shdmiReg->spdif_rch0 = a|b|c|d|Mode|Source_number_02|Channel_number_stereo_right|sampling|Clock_LvII;
				pi2shdmiReg->spdif_rch1 = Word1_24bit|sampling_org|CGMS_A_Copy;
			}
			if (bit_num == BIT_32)
			{
				ret = -1;
				printk("HDMI ChanStatus Config not yet\n");
			}
		break;
	}
	return ret;
}
EXPORT_SYMBOL(gpHalI2sHdmiChanStatus);

int gpHalOcpdmaClearIRQFlag(void)
{
	register int val;
	register int chk;
	
	val = pi2shdmiReg->ocp_int_read;
	pi2shdmiReg->ocp_int_read = val;  // clear A/B buffer interrupt flag
   
   /* protect */
	chk = pi2shdmiReg->ocp_int_read;
	if (chk!=0)
	{
		/* clear interrupt */
		printk("I2S_CLR_Err\n");
		pi2shdmiReg->ocp_int_read = 0x3;
		val = 0x3;
	}
	return val;
}
EXPORT_SYMBOL(gpHalOcpdmaClearIRQFlag);

void gpHalOcpdmaSetBuf(int buf_num, char* addr, int ln)
{
	if (buf_num == DMA_BUFFER_A)
	{
		pi2shdmiReg->ch0_start_addr = (int)addr;
		pi2shdmiReg->ch0_end_addr = (int)(addr+ln);
	}
	if (buf_num == DMA_BUFFER_B)
	{
		pi2shdmiReg->ch1_start_addr = (int)addr;
		pi2shdmiReg->ch1_end_addr = (int)(addr+ln);
	}
}
EXPORT_SYMBOL(gpHalOcpdmaSetBuf);

void gpHalOcpdmaIntEnable(void)
{
	register int val;
   
	val = pi2shdmiReg->ocp_cfg;
	val |= I2STX_OCP_ocp_en;
	pi2shdmiReg->ocp_cfg = val;
      
	val |= I2STX_OCP_en_ocp_int;
	pi2shdmiReg->ocp_cfg = val;            
}
EXPORT_SYMBOL(gpHalOcpdmaIntEnable);

void gpHalHdmiI2sClkEn(int enable)
{
	gp_enable_clock( (int*)"I2S_TX1_CLK", enable );
	gp_enable_clock( (int*)"I2S_TX1", enable );
}
EXPORT_SYMBOL(gpHalHdmiI2sClkEn);

