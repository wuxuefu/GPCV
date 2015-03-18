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

/*******************************************************************************
*                         H E A D E R   F I L E S
*******************************************************************************/
#include <linux/io.h>
#include <mach/kernel.h>
#include <mach/hal/regmap/reg_cdsp.h>
#include <mach/hal/regmap/reg_front.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/hal/regmap/reg_vic.h>
#include <mach/hal/hal_cdsp.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define DEBUG_SRAM_OUTPUT		0
#define ONLY_EOF_IRQ_EN			0

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define R_SCUB_CLK7_CFG3	(*(volatile unsigned *)(LOGI_ADDR_SCU_B_REG + 0x47C))
#define R_SCUB_CLK			(*(volatile unsigned *)(LOGI_ADDR_SCU_B_REG + 0x520))

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
static cdspReg0_t *pCdspReg0 = (cdspReg0_t *)(LOGI_ADDR_CDSP_REG + 0x000);
static cdspReg1_t *pCdspReg1 = (cdspReg1_t *)(LOGI_ADDR_CDSP_REG + 0x100);
static cdspReg2_t *pCdspReg2 = (cdspReg2_t *)(LOGI_ADDR_CDSP_REG + 0x200);
static cdspReg7_t *pCdspReg7 = (cdspReg7_t *)(LOGI_ADDR_CDSP_REG + 0x700);
static cdspLensCmpLut_t *pLensCmpLutData = (cdspLensCmpLut_t *)(LOGI_ADDR_CDSP_REG + 0x800);
static cdspGammaLut_t *pGammaLutData = (cdspGammaLut_t *)(LOGI_ADDR_CDSP_REG + 0x800);
static cdspEdgeLut_t *pEdgeLutData = (cdspEdgeLut_t *)(LOGI_ADDR_CDSP_REG + 0x800);

/**
 * @brief   cdsp module reset function
 * @param   enable[in]: disable(0)/enable(1)
 * @return  None
 * @see
 */
void
gpHalCdspModuleRest(
	UINT32 enable
)
{
	scubReg_t *scubReg = (scubReg_t *)LOGI_ADDR_SCU_B_REG;

	if(enable) {
		scubReg->scubPZoneERst |= (1<<6);
		msleep(1);
		scubReg->scubPZoneERst &= ~(1<<6);
	}
}

/**
 * @brief	cdsp module vic interrupt enable/disable
 * @param 	enable[in]: enable
 * @return 	none
*/
void 
gpHalCdspSetVicIntEn(
	UINT32 enable
) 
{
	vicReg_t *pVic = (vicReg_t *)LOGI_ADDR_VIC_REG;	

	if(enable) {
		pVic->vicIntEnable |= (1 << IRQ_CDSP);
	} else {
		pVic->vicIntEnable &= ~(1 << IRQ_CDSP);	
	}
}

/**
 * @brief	cdsp module clock enable
 * @param 	mclk_en[in]: enable
 * @param 	mclk_div[in]: mclk divider, (clko_div+1)
 * @param 	pclk_dly[in]: pclk delay
 * @param 	pclk_revb[in]: pclk reverse 
 * @return 	none
*/
void
gpHalCdspSetMclk(
	UINT32 clk_sel, 
	UINT32 clko_div,
	UINT32 pclk_dly,
	UINT32 pclk_revb
)
{
	/*Setting CSI_CLK_O Clock,[8]:Enable CSI_CLK_O,[7:0]: Clock_Ratio */
	scuaReg_t *pScuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG;
	UINT32 reg = 0;

	//CSI1 Clock Source : SPLL
	if(clko_div) {
		reg = (clko_div & 0xFF) | (1 << 8);
		reg |= (pclk_dly & 0xF) << 24 ;
		reg |= (pclk_revb & 1) << 28;
	} else {
		reg = 0;
	}
	
	pScuaReg->scuaCsiClkCfg = 0;
	pScuaReg->scuaCsiClkCfg = reg;
}

void
gpHalCdspGetMclk(
	UINT8 *clk_sel, 
	UINT8 *clko_div,
	UINT8 *pclk_dly,
	UINT8 *pclk_revb
)
{
	scuaReg_t *pScuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG;
	UINT32 reg = pScuaReg->scuaCsiClkCfg;

	if(reg & (1<<8)) {
		*clko_div = reg & 0xFF;
	} else {
		*clko_div = 0;
	}
	
	*clk_sel = reg & (1<<16) >> 16;
	*pclk_dly = (reg & (0xF << 24)) >> 24;
	*pclk_revb = (reg & (1<<28)) >> 28;
}

/**
* @brief	cdsp clock set
* @param 	mode[in]: 0: cdsp system, 1: front sensor, 2: mipi if
* @param 	type[in]: 1: yuv 0: raw
* @return 	none
*/
void 
gpHalCdspSetClk(
	UINT8 mode, 
	UINT8 type
)
{
	//scuaReg_t *pScuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG;
	//scubReg_t *pScubReg = (scubReg_t *)LOGI_ADDR_SCU_B_REG;
	
	switch(mode)
	{
	case C_CDSP_CLK_ENABLE:
		/* cdsp_core, cdsp_clk, cdsp_clk_d2 */
		gp_enable_clock( (int*)"CDSP_PCLK", 1 );
		gp_enable_clock( (int*)"CDSP_PCLK_D2", 1 );
			
		/* Enable CSI_MCLK */
		//pScuaReg->scuaSysSel |= (1 << 6);	

		/* Enable MCLK out */
		gp_enable_clock( (int*)"clk_csi", 1 );
		gp_enable_clock( (int*)"CMOS_CTRL", 1 );
		//pScuaReg->scuaCsiClkCfg |= 0x10;			//divider ??
		break;
		
	case C_CDSP_CLK_DISABLE:
		/* cdsp_core, cdsp_clk, cdsp_clk_d2 */

		gp_enable_clock( (int*)"CDSP_PCLK", 0 );
		gp_enable_clock( (int*)"CDSP_PCLK_D2", 0 );

		/* Disable CSI_MCLK */
		//pScuaReg->scuaSysSel &= ~(1 << 6);	

		/* Disable MCLK out */
		gp_enable_clock( (int*)"clk_csi", 0 );
		gp_enable_clock( (int*)"CMOS_CTRL", 0 );
		//pScuaReg->scuaCsiClkCfg &= ~0xff;			//divider ??

		R_SCUB_CLK7_CFG3 = 0x0000;
		break;
		
	case C_CDSP_CLK_FB:
		/* from internal clk07 divider output */
		R_SCUB_CLK7_CFG3 = 0x0000;
		
		if(type) {
			/* cdsp clock sel mipi, cdsp_clk_d2_i = cdsp_clk_i / 2, yuyv */
			R_SCUB_CLK7_CFG3 |= (1 << 4);
		} else {
			/* cdsp clock sel mipi, cdsp_clk_d2_i = cdsp_clk_i, raw */
			R_SCUB_CLK7_CFG3 &= ~(1 << 4);
		}
			
		/* cdsp_clk, cdsp_clk_d2 */

		gp_enable_clock( (int*)"CDSP_PCLK_D2", 1 );	
		break;
		
	case C_CDSP_CLK_FRONT:
		/* from external PCLK*/
		R_SCUB_CLK7_CFG3 = 0x0004;
		
		if(type) {
			/* cdsp clock sel mipi, cdsp_clk_d2_i = cdsp_clk_i / 2, yuyv */
			R_SCUB_CLK7_CFG3 |= (1 << 4);
		} else {
			/* cdsp clock sel mipi, cdsp_clk_d2_i = cdsp_clk_i, raw */
			R_SCUB_CLK7_CFG3 &= ~(1 << 4);
		}
		break;
		
	case C_CDSP_CLK_MIPI:
		/* clock setting */
		R_SCUB_CLK = 0x0300;

		/* from MIPI PHY */
		R_SCUB_CLK7_CFG3 = 0x000C;
		
		if(type) {
			/* cdsp clock sel mipi, cdsp_clk_d2_i = cdsp_clk_i / 2, yuyv */
			R_SCUB_CLK7_CFG3 |= (1 << 4);
		} else {
			/* cdsp clock sel mipi, cdsp_clk_d2_i = cdsp_clk_i, raw */
			R_SCUB_CLK7_CFG3 &= ~(1 << 4);
		}
		break;
	}
}

/**
* @brief	cdsp reset
* @param 	none
* @return 	none
*/
void 
gpHalCdspReset(
	void
)
{
	pCdspReg0->cdspReset = 0x01;	
	pCdspReg0->cdspReset = 0x00;
}

/**
* @brief	cdsp raw data format
* @param	format [in]: format
* @return	none
*/
void 
gpHalCdspSetRawDataFormat(
	UINT8 format
)
{
	pCdspReg0->cdspImgType = format & 0x03;
}

UINT8
gpHalCdspGetRawDataFormat(
	void 
)
{
	return (pCdspReg0->cdspImgType & 0x03);
}

/**
* @brief	cdsp yuv range
* @param	signed_yuv_en [in]: yuv signed/unsigned set
* @return	none
*/
void 
gpHalCdspSetYuvRange(
	UINT8 signed_yuv_en
)
{
	pCdspReg0->cdspYuvRange = signed_yuv_en & 0x07;
}

UINT8 
gpHalCdspGetYuvRange(
	void 
)
{
	return (pCdspReg0->cdspYuvRange & 0x07);
}

/**
* @brief	cdsp image source
* @param	image_source [in]: image input source, 0:front, 1:sdram, 2:mipi 3: skip write dram
* @return	none
*/
void 
gpHalCdspDataSource(
	UINT8 image_source
)
{
#if 1
	if(image_source >= 3) {
		pCdspReg1->cdspDo |= (1 << 4);
	} else {
		pCdspReg1->cdspDo &= ~(1 << 4);
		pCdspReg1->cdspImgSrc = image_source & 0x03;
	}
#else
	if(image_source >= 3) {
		image_source = 1;
	}

	pCdspReg1->cdspImgSrc = image_source & 0x03;
#endif
}

UINT8 
gpHalCdspGetDataSource(
	void
)
{
	return (pCdspReg1->cdspImgSrc & 0x03);
}

/**
* @brief	cdsp post-process triger
* @param	docdsp [in]: enable
* @return	none
*/
void 
gpHalCdspRedoTriger(
	UINT8 docdsp
)
{
	if(docdsp) {
		pCdspReg1->cdspDo |= 0x01;
	} else {
		pCdspReg1->cdspDo &= ~0x01;
	}
}

/**
* @brief	cdsp interrupt enable
* @param	enable [in]: enable / disable
* @param	bit [in]: interrupt bit
* @return	none
*/
void
gpHalCdspSetIntEn(
	UINT8 enable, 
	UINT8 bit
)
{
#if ONLY_EOF_IRQ_EN == 0
	if(enable) {
		pCdspReg0->cdspIntEn |= bit;
	} else {
		pCdspReg0->cdspIntEn &= ~bit;
	}
	
#else
	if(bit & (CDSP_EOF|CDSP_OVERFOLW)) {
		SINT32 temp = bit & (CDSP_EOF|CDSP_OVERFOLW);

		bit &= ~temp;
		if(enable) {
			pCdspReg0->cdspIntEn |= temp;
		} else {
			pCdspReg0->cdspIntEn &= ~temp;
		}
	} 

	if(enable) {
		pCdspReg0->cdspDummy0 |= bit;
	} else {
		pCdspReg0->cdspDummy0 &= ~bit;
	}
	
	//printk("cdsp_irq_en:0x%x\n", pCdspReg0->cdspIntEn);
	//printk("cdsp_irq_dummy:0x%x\n", pCdspReg0->cdspDummy0);
#endif
}

/**
* @brief	cdsp get interrupt bit
* @param	none
* @return	bit
*/
UINT32
gpHalCdspGetIntEn(
	void
)
{
	return pCdspReg0->cdspIntEn;
}

/**
* @brief	cdsp interrupt clear status
* @param	bit [in]: clear interrupt bit
* @return	none
*/
void
gpHalCdspClrIntStatus(
	UINT8 bit
)
{
	pCdspReg0->cdspInt = bit;
}

/**
* @brief	cdsp get int status
* @param	none
* @return	int status
*/
UINT32 
gpHalCdspGetIntStatus(
	void
)
{
	UINT32 status = pCdspReg0->cdspInt;
	UINT32 i, irq;

	//clear interrupt
	pCdspReg0->cdspInt = status; //CDSP_INT_ALL
	irq = pCdspReg0->cdspInt;

	//check status clear
	if(irq & status) {
		for(i=0; i<10; i++) {
			pCdspReg0->cdspInt = status;
			irq = pCdspReg0->cdspInt;
			if((irq & status) == 0) {
				break;
			}
		} 
		
		if(i == 10) {
			printk("cdsp_irq:0x%02x\n", irq);
		}
	}
	
	status &= pCdspReg0->cdspIntEn;
#if ONLY_EOF_IRQ_EN == 1
	status |= pCdspReg0->cdspDummy0;
#endif
	
	//printk("cdsp_irq:0x%02x\n", status);
	return status;
}

/**
* @brief	cdsp get front vd int status
* @param	none
* @return	int status
*/
UINT32 
gpHalCdspGetFrontVdIntStatus(
	void
)
{
	UINT32 status = pCdspReg1->cdspGInt;

	status &= (CDSP_INT_BIT | FRONT_VD_INT_BIT | FRONT_INT_BIT);
	return status;	
}

/**
* @brief	cdsp get front int status
* @param	none
* @return	int status
*/
UINT32 
gpHalCdspGetFrontIntStatus(
	void
)
{
#if 0	
	UINT32 status = CdspRegFront->cdspInt;

	//clear interrupt
	pCdspRegFront->cdspInt = status;
	status &= pCdspRegFront->cdspIntEn;
	
	//printk("cdsp_front_irq:0x%x\n", status);
	return status;
#else
	return 0;
#endif
}

/**
* @brief	cdsp get global int status
* @param	none
* @return	int status
*/
UINT32
gpHalCdspGetGlbIntStatus(
	void
)
{
	UINT32 status = pCdspReg1->cdspGInt;

	status &= (CDSP_INT_BIT | FRONT_VD_INT_BIT | FRONT_INT_BIT);	
	return status;
}

/**
* @brief	cdsp bad pixel enable
* @param	badpixen [in]: bad pixel enable
* @param	badpixen [in]: bad pixel mirror enable, bit1: right, bit0: left, 
* @return	none
*/
void 
gpHalCdspSetBadPixelEn(
	UINT8 badpixen, 
	UINT8 badpixmiren
)
{
	pCdspReg0->cdspBadPixEn = badpixen & 0x01;
	pCdspReg0->cdspBadPixMir = badpixmiren & 0x03;
}

void 
gpHalCdspGetBadPixelEn(
	UINT8 *badpixen, 
	UINT8 *badpixmiren
)
{
	*badpixen = pCdspReg0->cdspBadPixEn & 0x01;
	*badpixmiren = pCdspReg0->cdspBadPixMir & 0x03;
}

/**
* @brief	cdsp bad pixel threshold set
* @param	bprthr [in]: R threshold
* @param	bpgthr [in]: G threshold 
* @param	bpbthr [in]: B threshold 
* @return	none
*/
void 
gpHalCdspSetBadPixel(
	UINT16 bprthr, 
	UINT16 bpgthr, 
	UINT16 bpbthr
)
{
	bprthr = (bprthr & 0x3FF) >> 2;
	bpgthr = (bpgthr & 0x3FF) >> 2;
	bpbthr = (bpbthr & 0x3FF) >> 2;
	pCdspReg0->cdspBadPixRThr = (UINT8)bprthr;
	pCdspReg0->cdspBadPixGThr = (UINT8)bpgthr;
	pCdspReg0->cdspBadPixBThr = (UINT8)bpbthr;
}

void 
gpHalCdspGetBadPixel(
	UINT16 *bprthr, 
	UINT16 *bpgthr, 
	UINT16 *bpbthr
)
{
	*bprthr = pCdspReg0->cdspBadPixRThr << 2;
	*bpgthr = pCdspReg0->cdspBadPixGThr << 2;
	*bpbthr = pCdspReg0->cdspBadPixBThr << 2;
}

/**
* @brief	cdsp manual optical black enable
* @param	manuoben [in]: manual optical black enable
* @param	manuob [in]: manual optical subtracted value 
* @return	none
*/
void 
gpHalCdspSetManuOB(
	UINT8 manuoben, 
	UINT16 manuob
)
{
	pCdspReg0->cdspManuObEn = manuoben & 0x01;
	pCdspReg0->cdspManuObVal[0] = (UINT8)(manuob & 0x00FF);
	pCdspReg0->cdspManuObVal[1] = (UINT8)((manuob & 0x0700) >> 8);
}

void 
gpHalCdspGetManuOB(
	UINT8 *manuoben, 
	UINT16 *manuob
)
{
	*manuoben = pCdspReg0->cdspManuObEn & 0x01;
	*manuob = ((UINT16)pCdspReg0->cdspManuObVal[1] << 8) | pCdspReg0->cdspManuObVal[0];
}
/**
* @brief	cdsp auto optical black enable
* @param	autooben [in]: auto optical black enable
* @param	obtype [in]: auto optical accumulation block type
* @param	obHOffset [in]: auto optical accumulation block horizontal offset
* @param	obVOffset [in]: auto optical accumulation block vertical offset
* @return	none
*/
void 
gpHalCdspSetAutoOB(
	UINT8 autooben, 
	UINT8 obtype, 
	UINT16 obHOffset, 
	UINT16 obVOffset
)
{
	pCdspReg0->cdspAutoObCtrl = ((autooben & 0x01) << 4) | (obtype & 0x07); 
	pCdspReg0->cdspAutoObHOff[0] = (UINT8)(obHOffset & 0x00FF);
	pCdspReg0->cdspAutoObHOff[1] = (UINT8)((obHOffset & 0x0F00) >> 8);
	pCdspReg0->cdspAutoObVOff[0] = (UINT8)(obVOffset & 0x00FF);
	pCdspReg0->cdspAutoObVOff[1] = (UINT8)((obVOffset & 0x0F00) >> 8);
}

void 
gpHalCdspGetAutoOB(
	UINT8 *autooben, 
	UINT8 *obtype, 
	UINT16 *obHOffset, 
	UINT16 *obVOffset
)
{
	*autooben = (pCdspReg0->cdspAutoObCtrl >> 4) & 0x01;
	*obtype = pCdspReg0->cdspAutoObCtrl & 0x07;
	*obHOffset = ((UINT16)pCdspReg0->cdspAutoObHOff[1] << 8) | pCdspReg0->cdspAutoObHOff[0];
	*obVOffset = ((UINT16)pCdspReg0->cdspAutoObVOff[1] << 8) | pCdspReg0->cdspAutoObVOff[0];
}

/**
* @brief	cdsp get auto optical black average
* @param	Ravg [out]: r average
* @param	GRavg [out]: gr average
* @param	Bavg [out]: b average
* @param	GBavg [out]: gb average
* @return	none
*/
void 
gpHalCdspGetAutoOBAvg(
	UINT16 *Ravg, 
	UINT16 *GRavg, 
	UINT16 *Bavg, 
	UINT16 *GBavg
)
{
	*Ravg = ((UINT16)pCdspReg0->cdspAutoObRAvg[1] << 8) | pCdspReg0->cdspAutoObRAvg[0];
	*GRavg = ((UINT16)pCdspReg0->cdspAutoObGrAvg[1] << 8) | pCdspReg0->cdspAutoObGrAvg[0];;
	*Bavg = ((UINT16)pCdspReg0->cdspAutoObBAvg[1] << 8) | pCdspReg0->cdspAutoObBAvg[0];;
	*GBavg = ((UINT16)pCdspReg0->cdspAutoObGbAvg[1] << 8) | pCdspReg0->cdspAutoObGbAvg[0];;
}

/**
* @brief	cdsp lens compensation enable
* @param	plensdata [in]: lens compensation table pointer
* @return	none
*/
void 
gpHalCdspInitLensCmp(
	UINT16 *plensdata
)
{
	UINT32 i;

	pCdspReg0->cdspSwitchClk = 0x01;
	pCdspReg0->cdspMacroSel = 0x01;
	pCdspReg0->cdspCPUSramEn = 0x01;
	
	for(i=0; i<256; i++)
	{
		pLensCmpLutData->LensCmpTable[i*2+0] =  (UINT8)(plensdata[i] & 0x00FF);
		pLensCmpLutData->LensCmpTable[i*2+1] =  (UINT8)((plensdata[i] & 0xFF00) >> 8);
	}
	
	pCdspReg0->cdspCPUSramEn = 0x00;
	pCdspReg0->cdspMacroSel = 0x00;
	pCdspReg0->cdspSwitchClk = 0x00;

	if(DEBUG_SRAM_OUTPUT)
	{
		printk("Input LensCmp table:\n");
		for(i=0; i<256; i++)
		{
			printk("0x%02x, 0x%02x, ", (UINT8)(plensdata[i] & 0x00FF), (UINT8)((plensdata[i] & 0xFF00) >> 8));
			if((i%8) == 7)
				printk("\n");
		}

		pCdspReg0->cdspSwitchClk = 0x01;
		pCdspReg0->cdspMacroSel = 0x01;
		pCdspReg0->cdspCPUSramEn = 0x01;

		printk("SRAM LensCmp table:\n");
		for(i=0; i<512; i++)
		{
			printk("0x%02x, ", pLensCmpLutData->LensCmpTable[i]);
			if((i%16) == 15)
				printk("\n");
		}

		pCdspReg0->cdspCPUSramEn = 0x00;
		pCdspReg0->cdspMacroSel = 0x00;
		pCdspReg0->cdspSwitchClk = 0x00;
	}
}

/**
* @brief	cdsp lens compensation enable
* @param	lcen [in]: lens compensation enable
* @return	none
*/
void 
gpHalCdspSetLensCmpEn(
	UINT8 lcen
)
{
	if(lcen) {
		pCdspReg0->cdspLensCmpCtrl |= (1 << 4);
	} else {
		pCdspReg0->cdspLensCmpCtrl &= ~(1 << 4);
	}
}

UINT32 
gpHalCdspGetLensCmpEn(
	void 
)
{
	return (pCdspReg0->cdspLensCmpCtrl >> 4) & 0x01; 	
}

/**
* @brief	cdsp lens compensation postion
* @param	centx [in]: X center 
* @param	centy [in]: Y center
* @param	xmoffset [in]: X offset
* @param	ymoffset [in]: Y offset
* @return	none
*/
void 
gpHalCdspSetLensCmpPos(
	UINT16 centx, 
	UINT16 centy, 
	UINT16 xmoffset, 
	UINT16 ymoffset 
)
{
	pCdspReg0->cdspLensCmpXCent[0] = (UINT8)(centx & 0x00FF);
	pCdspReg0->cdspLensCmpXCent[1] = (UINT8)((centx & 0x1F00) >> 8);
	pCdspReg0->cdspLensCmpYCent[0] = (UINT8)(centy & 0x00FF);
	pCdspReg0->cdspLensCmpYCent[1] = (UINT8)((centy & 0x1F00) >> 8);
	pCdspReg0->cdspLensCmpXmOff[0] = (UINT8)(xmoffset & 0x00FF);
	pCdspReg0->cdspLensCmpXmOff[1] = (UINT8)((xmoffset & 0x0F00) >> 8);
	pCdspReg0->cdspLensCmpYmOff[0] = (UINT8)(ymoffset & 0x00FF);
	pCdspReg0->cdspLensCmpYmOff[1] = (UINT8)((ymoffset & 0x0F00) >> 8);
}

void 
gpHalCdspGetLensCmpPos(
	UINT16 *centx, 
	UINT16 *centy, 
	UINT16 *xmoffset, 
	UINT16 *ymoffset 
)
{
	*centx = ((UINT16)pCdspReg0->cdspLensCmpXCent[1] << 8) | pCdspReg0->cdspLensCmpXCent[0];
	*centy = ((UINT16)pCdspReg0->cdspLensCmpYCent[1] << 8) | pCdspReg0->cdspLensCmpYCent[0];
	*xmoffset = ((UINT16)pCdspReg0->cdspLensCmpXmOff[1] << 8) | pCdspReg0->cdspLensCmpXmOff[0];
	*ymoffset = ((UINT16)pCdspReg0->cdspLensCmpYmOff[1] << 8) | pCdspReg0->cdspLensCmpYmOff[0];
}

/**
* @brief	cdsp lens compensation enable
* @param	stepfactor [in]: step unit between entries of len shading compensation LUT
* @param	xminc [in]: X increase step
* @param	ymoinc [in]: Y increase step odd line
* @param	ymeinc [in]: Y increase step even lin
* @return	none
*/
void 
gpHalCdspSetLensCmp(
	UINT8 stepfactor,
	UINT8 xminc, 
	UINT8 ymoinc, 
	UINT8 ymeinc
)
{	
	pCdspReg0->cdspLensCmpCtrl &= ~(0x7);
	pCdspReg0->cdspLensCmpCtrl |= stepfactor & 0x07;
	pCdspReg0->cdspLensCmpXmInc = (xminc & 0x0F) << 4;
	pCdspReg0->cdspLensCmpYmInc = ((ymeinc & 0x0F) << 4) | (ymoinc & 0x0F);
	
}

void 
gpHalCdspGetLensCmp(
	UINT8 *stepfactor,
	UINT8 *xminc, 
	UINT8 *ymoinc, 
	UINT8 *ymeinc
)
{	
	*stepfactor = pCdspReg0->cdspLensCmpCtrl & 0x07;
	*xminc = (pCdspReg0->cdspLensCmpXmInc >> 4) & 0x0F;
	*ymeinc = (pCdspReg0->cdspLensCmpYmInc >> 4) & 0xF;
	*ymoinc = pCdspReg0->cdspLensCmpYmInc  & 0xF;
}

/**
* @brief	cdsp yuv lens compensation path set
* @param	yuvlens [in]: 0:yuv path2, 1:yuv path5
* @return	none
*/
void
gpHalCdspSetLensCmpPath(
	UINT8 yuvlens
)
{
	if(yuvlens) {
		pCdspReg0->cdspLensCmpCtrl |= 0x20;
	} else {
		pCdspReg0->cdspLensCmpCtrl &= ~0x20;
	}
}

UINT8
gpHalCdspGetLensCmpPath(
	void
)
{
	return (pCdspReg0->cdspLensCmpCtrl >> 5) & 0x01; 
}

/**
* @brief	cdsp crop function enable
* @param	hv_crop_en [in]: h/v crop enable
* @return	none
*/
void 
gpHalCdspSetCropEn(
	UINT8 hv_crop_en
)
{
	UINT32 temp;
	
	if(hv_crop_en) {
		hv_crop_en = 0x3;
	}

	temp = ((UINT16)pCdspReg0->cdspImgCropHOff[1] << 8) | pCdspReg0->cdspImgCropHOff[0];
	if(temp == 0) {
		hv_crop_en &= ~0x01;
	}

	temp = ((UINT16)pCdspReg0->cdspImgCropVOff[1] << 8) | pCdspReg0->cdspImgCropVOff[0];
	if(temp == 0) {
		hv_crop_en &= ~0x02;
	}
	
	pCdspReg0->cdspImgCropCtrl = hv_crop_en; 
	/* reflected at vd update */
	pCdspReg0->cdspImgCropCtrl |= 0x10;
}

UINT8 
gpHalCdspGetCropEn(
	void
)
{
	return (pCdspReg0->cdspImgCropCtrl & 0x03);
}

/**
* @brief	cdsp crop function
* @param	crop_hoffset [in]: h offset set
* @param	crop_voffset [in]: v offset set
* @param	crop_hsize [in]: h crop size
* @param 	crop_vsize [in]: v crop size
* @return	none
*/
void 
gpHalCdspSetCrop(
	UINT16 crop_hoffset, 
	UINT16 crop_voffset, 
	UINT16 crop_hsize, 
	UINT16 crop_vsize
)
{
	pCdspReg0->cdspImgCropHOff[0] = (UINT8)(crop_hoffset & 0x00FF);
	pCdspReg0->cdspImgCropHOff[1] = (UINT8)((crop_hoffset & 0x0F00) >> 8);
	pCdspReg0->cdspImgCropVOff[0] = (UINT8)(crop_voffset & 0x00FF);
	pCdspReg0->cdspImgCropVOff[1] = (UINT8)((crop_voffset & 0x0F00) >> 8);
	pCdspReg0->cdspImgCropHSize[0]= (UINT8)(crop_hsize & 0x00FF);
	pCdspReg0->cdspImgCropHSize[1] = (UINT8)((crop_hsize & 0x0F00) >> 8);
	pCdspReg0->cdspImgCropVSize[0] = (UINT8)(crop_vsize & 0x00FF);
	pCdspReg0->cdspImgCropVSize[1] = (UINT8)((crop_vsize & 0x0F00) >> 8);
}


void 
gpHalCdspSetVCrop(
	UINT16 crop_voffset,
	UINT16 crop_vsize,
	UINT8 v_crop_en
)
{
	
	pCdspReg0->cdspImgCropVOff[0] = (UINT8)(crop_voffset & 0x00FF);
	pCdspReg0->cdspImgCropVOff[1] = (UINT8)((crop_voffset & 0x0F00) >> 8);	
	pCdspReg0->cdspImgCropVSize[0] = (UINT8)(crop_vsize & 0x00FF);
	pCdspReg0->cdspImgCropVSize[1] = (UINT8)((crop_vsize & 0x0F00) >> 8);

	if(v_crop_en)
	{
		pCdspReg0->cdspImgCropCtrl = 0x02; 
		
		/* reflected at vd update */
		pCdspReg0->cdspImgCropCtrl |= 0x10;
	}
}


void 
gpHalCdspGetCrop(
	UINT16 *crop_hoffset, 
	UINT16 *crop_voffset, 
	UINT16 *crop_hsize, 
	UINT16 *crop_vsize
)
{
	*crop_hoffset = ((UINT16)pCdspReg0->cdspImgCropHOff[1] << 8) | pCdspReg0->cdspImgCropHOff[0];
	*crop_voffset = ((UINT16)pCdspReg0->cdspImgCropVOff[1] << 8) | pCdspReg0->cdspImgCropVOff[0];
	*crop_hsize = ((UINT16)pCdspReg0->cdspImgCropHSize[1] << 8) | pCdspReg0->cdspImgCropHSize[0];
	*crop_vsize = ((UINT16)pCdspReg0->cdspImgCropVSize[1] << 8) | pCdspReg0->cdspImgCropVSize[0];
}

/**
* @brief	cdsp raw horizontal scale down enable
* @param	hscale_en [in]: horizontal scale down enable 
* @param	hscale_mode [in]: scale down mode, 0:drop, 1:filter
* @return	none
*/
void 
gpHalCdspSetRawHScaleEn(
	UINT8 hscale_en, 
	UINT8 hscale_mode
)
{
	pCdspReg0->cdspRawHSclCtrl &= ~(1 << 4 | 0x1);
	pCdspReg0->cdspRawHSclCtrl |= ((hscale_en & 0x01) << 4) | (hscale_mode & 0x01);
}

void 
gpHalCdspGetRawHScaleEn(
	UINT8 *hscale_en, 
	UINT8 *hscale_mode
)
{
	*hscale_en = (pCdspReg0->cdspRawHSclCtrl >> 4) & 0x01;
	*hscale_mode = pCdspReg0->cdspRawHSclCtrl & 0x01;
}

/**
* @brief	cdsp raw horizontal scale down set
* @param	src_hsize [in]: source width 
* @param	dst_hsize [in]: densting width
* @return	none
*/
void 
gpHalCdspSetRawHScale(
	UINT8 src_hsize, 
	UINT8 dst_hsize
)
{
	UINT32 factor;
	
	if (dst_hsize >= src_hsize) 
	{
		pCdspReg0->cdspRawHSclFactor[0] = 0x00;
		pCdspReg0->cdspRawHSclFactor[1] = 0x00;
	}
	else 
	{
		factor = ((UINT32)dst_hsize << 16) / ((UINT32)src_hsize) + 1;
		pCdspReg0->cdspRawHSclIniEve[0] = (UINT8)(((factor/2)+0x8000) & 0x00FF); // by GPL32900
		pCdspReg0->cdspRawHSclIniEve[1] = (UINT8)((((factor/2)+0x8000) & 0xFF00) >> 8); // by GPL32900
		pCdspReg0->cdspRawHSclIniOdd[0] = (UINT8)(factor & 0x00FF); // by GPL32900
		pCdspReg0->cdspRawHSclIniOdd[1] = (UINT8)((factor & 0xFF00) >> 8); // by GPL32900
		pCdspReg0->cdspRawHSclFactor[0] = (UINT8)(factor & 0x00FF);
		pCdspReg0->cdspRawHSclFactor[1] = (UINT8)((factor & 0x00FF) >> 8);
	}

#if 1
	/* immediate update */
	pCdspReg0->cdspSclUpdateMode = 0x00;
#else	
	/* reflected at next vaild vd edge */
	pCdspReg0->cdspSclUpdateMode = 0x01;
#endif
}

/**
* @brief	cdsp whitle balance offset set enable
* @param	wboffseten [in]: white balance enable
* @return	none
*/
void 
gpHalCdspSetWbOffsetEn(
	UINT8 wboffseten
)
{
	if(wboffseten) {
		pCdspReg0->cdspWbCtrl |= (1 << 4);	
	} else {
		pCdspReg0->cdspWbCtrl &= ~(1 << 4);
	}
}

/**
* @brief	cdsp whitle balance offset set
* @param	roffset [in]: R offset 
* @param	groffset [in]: Gr offset
* @param	boffset [in]: B offset
* @param	gboffset [in]: Gb offset
* @return	none
*/
void 
gpHalCdspSetWbOffset(
	UINT8 roffset, 
	UINT8 groffset, 
	UINT8 boffset, 
	UINT8 gboffset
)
{
	pCdspReg0->cdspWbROff = roffset;
	pCdspReg0->cdspWbGrOff = groffset;
	pCdspReg0->cdspWbBOff = boffset;
	pCdspReg0->cdspWbGbOff = gboffset;
}

/**
* @brief	cdsp whitle balance offset get
* @param	roffset [out]: R offset 
* @param	groffset [out]: Gr offset
* @param	boffset [out]: B offset
* @param	gboffset [out]: Gb offset
* @return	wboffset enable bit
*/
UINT32 
gpHalCdspGetWbOffset(
	UINT8 *roffset, 
	UINT8 *groffset, 
	UINT8 *boffset, 
	UINT8 *gboffset
)
{
	*roffset = pCdspReg0->cdspWbROff;
	*groffset = pCdspReg0->cdspWbGrOff;
	*boffset = pCdspReg0->cdspWbBOff;
	*gboffset = pCdspReg0->cdspWbGbOff;	
	
	return ((pCdspReg0->cdspWbCtrl >> 4) & 0x01);
}

/**
* @brief	cdsp whitle balance gain set enable
* @param	wboffseten [in]: white balance enable
* @return	none
*/
void 
gpHalCdspSetWbGainEn(
	UINT8 wbgainen
)
{
	if(wbgainen) {
		pCdspReg0->cdspWbCtrl |= (1 << 5);	
	} else {
		pCdspReg0->cdspWbCtrl &= ~(1 << 5);
	}

#if 1
	/* immediate update */
	pCdspReg0->cdspSclUpdateMode = 0x00;
#else
	/* reflected at next vaild vd edge */
	pCdspReg0->cdspSclUpdateMode = 0x01; 
#endif	
}

/**
* @brief	cdsp whitle balance gain set
* @param	r_gain [in]: R gain 
* @param	gr_gain [in]: Gr gain
* @param	b_gain [in]: B gain
* @param	gb_gain [in]: Gb gain
* @return 	none
*/
void
gpHalCdspSetWbGain(
	UINT16 rgain, 
	UINT16 grgain, 
	UINT16 bgain, 
	UINT16 gbgain
)
{
	pCdspReg0->cdspWbRGain[0] = (UINT8)(rgain & 0x00FF);
	pCdspReg0->cdspWbRGain[1] = (UINT8)((rgain & 0x0100) >> 8);
	pCdspReg0->cdspWbGrGain[0] = (UINT8)(grgain & 0x00FF);
	pCdspReg0->cdspWbGrGain[1] = (UINT8)((grgain & 0x0100) >> 8);
	pCdspReg0->cdspWbBGain[0] = (UINT8)(bgain & 0x00FF);
	pCdspReg0->cdspWbBGain[1] = (UINT8)((bgain & 0x0100) >> 8);
	pCdspReg0->cdspWbGbGain[0] = (UINT8)(gbgain & 0x00FF);
	pCdspReg0->cdspWbGbGain[1] = (UINT8)((gbgain & 0x0100) >> 8);
}

void
gpHalCdspSetWb_r_b_Gain(
	UINT16 rgain, 
	UINT16 bgain
)
{
	pCdspReg0->cdspWbRGain[0] = (UINT8)(rgain & 0x00FF);
	pCdspReg0->cdspWbRGain[1] = (UINT8)((rgain & 0x0100) >> 8);
	
	pCdspReg0->cdspWbBGain[0] = (UINT8)(bgain & 0x00FF);
	pCdspReg0->cdspWbBGain[1] = (UINT8)((bgain & 0x0100) >> 8);	
}

/**
* @brief	cdsp whitle balance gain get
* @param	r_gain [out]: R gain 
* @param	gr_gain [out]: Gr gain
* @param	b_gain [out]: B gain
* @param	gb_gain [out]: Gb gain
* @return 	wb gain enable bit
*/
UINT32
gpHalCdspGetWbGain(
	UINT16 *rgain, 
	UINT16 *grgain, 
	UINT16 *bgain, 
	UINT16 *gbgain
)
{
	*rgain = ((UINT16)pCdspReg0->cdspWbRGain[1] << 8) | pCdspReg0->cdspWbRGain[0];
	*grgain = ((UINT16)pCdspReg0->cdspWbGrGain[1] << 8) | pCdspReg0->cdspWbGrGain[0];
	*bgain = ((UINT16)pCdspReg0->cdspWbBGain[1] << 8) | pCdspReg0->cdspWbBGain[0];
	*gbgain = ((UINT16)pCdspReg0->cdspWbGbGain[1] << 8) | pCdspReg0->cdspWbGbGain[0];

	return ((pCdspReg0->cdspWbCtrl >> 5) & 0x01);
}

/**
* @brief	cdsp whitle balance global gain set
* @param	global_gain [in]: wb global gain set
* @return	none
*/
void 
gpHalCdspSetGlobalGain(
	UINT8 global_gain
)
{
	pCdspReg0->cdspGlobalGain = global_gain;
}

UINT8 
gpHalCdspGetGlobalGain(
	void
)
{
	return pCdspReg0->cdspGlobalGain;
}

/**
* @brief	cdsp lut gamma table enable
* @param	pGammaTable [in]: lut gamma table pointer 
* @return	none
*/
void 
gpHalCdspInitGamma(
	UINT32 *pGammaTable
)
{
	UINT32 i;

	pCdspReg0->cdspSwitchClk = 0x02;
	pCdspReg0->cdspMacroSel = 0x02;
	pCdspReg0->cdspCPUSramEn = 0x01;
	
	for(i=0; i<128; i++)
	{
		pGammaLutData->GammaTable[i*4+0] =  (UINT8)(pGammaTable[i] & 0x000000FF);
		pGammaLutData->GammaTable[i*4+1] =  (UINT8)((pGammaTable[i] & 0x0000FF00) >> 8);
		pGammaLutData->GammaTable[i*4+2] =  (UINT8)((pGammaTable[i] & 0x00FF0000) >> 16);
		pGammaLutData->GammaTable[i*4+3] =  (UINT8)((pGammaTable[i] & 0xFF000000) >> 24);
	}
	
	pCdspReg0->cdspCPUSramEn = 0x00;
	pCdspReg0->cdspMacroSel = 0x00;
	pCdspReg0->cdspSwitchClk = 0x00;

	if(DEBUG_SRAM_OUTPUT)
	{
		printk("Input Gamma table:\n");
		for(i=0; i<128; i++)
		{
			printk("0x%02x, 0x%02x, 0x%02x, 0x%02x, \n", (UINT8)(pGammaTable[i] & 0x000000FF), (UINT8)((pGammaTable[i] & 0x0000FF00) >> 8), (UINT8)((pGammaTable[i] & 0x00FF0000) >> 16), (UINT8)((pGammaTable[i] & 0xFF000000) >> 24));
		}

		pCdspReg0->cdspSwitchClk = 0x02;
		pCdspReg0->cdspMacroSel = 0x02;
		pCdspReg0->cdspCPUSramEn = 0x01;

		printk("SRAM Gamma table:\n");
		for(i=0; i<(128*4); i++)
		{
			printk("0x%02x, ", pGammaLutData->GammaTable[i]);
			if((i%4) == 3)
				printk("\n");
		}

		pCdspReg0->cdspCPUSramEn = 0x00;
		pCdspReg0->cdspMacroSel = 0x00;
		pCdspReg0->cdspSwitchClk = 0x00;
	}
}

/**
* @brief	cdsp lut gamma table enable
* @param	lut_gamma_en [in]: lut gamma table enable 
* @return	none
*/
void 
gpHalCdspSetLutGammaEn(
	UINT8 lut_gamma_en
)
{
	if(lut_gamma_en) {
		pCdspReg0->cdspGammaEn |= 0x01;
	} else {
		pCdspReg0->cdspGammaEn &= ~0x01;
	}
}

UINT8 
gpHalCdspGetLutGammaEn(
	void
)
{
	return (pCdspReg0->cdspGammaEn & 0x01);
}

/**
* @brief	cdsp set line buffer
* @param	path [in]: 0: raw data, 1: YUV data	
* @return	none
*/
void
gpHalCdspSetLineCtrl(
	UINT8 ybufen	
)
{
	if(ybufen) {
		pCdspReg0->cdspSuppEn |= 0x10;	
	} else {
		pCdspReg0->cdspSuppEn &= ~0x10;
	}
}

/**
* @brief	cdsp set external line and bank
* @param	linesize [in]: external line size
* @param	lineblank [in]: external bank size
* @return	none
*/
void
gpHalCdspSetExtLine(
	UINT16 linesize,
	UINT16 lineblank
)
{	
	pCdspReg0->cdspExtLineSize[0] = (UINT8)(linesize & 0x00FF);
	pCdspReg0->cdspExtLineSize[1] = (UINT8)((linesize & 0x0F00) >> 8);
	pCdspReg0->cdspExtBlankSize[0] = (UINT8)(lineblank & 0x00FF);
	pCdspReg0->cdspExtBlankSize[1] = (UINT8)((lineblank & 0x0F00) >> 8);
}
	
/**
* @brief	cdsp external line path set
* @param	extinen [in]: external line enable
* @param	path [in]: 0:interpolution, 1:uvsuppression
* @return	none
*/
void
gpHalCdspSetExtLinePath(
	UINT8 extinen,
	UINT8 path
)
{
	if(extinen) {
		pCdspReg0->cdspInpMirCtrl |= 0x20;
	} else {
		pCdspReg0->cdspInpMirCtrl &= ~0x20;
	}
	
	if(path) {
		pCdspReg0->cdspInpMirCtrl |= 0x40;
	} else {
		pCdspReg0->cdspInpMirCtrl &= ~0x40;
	}
}

/**
* @brief	cdsp interpolation mirror enable
* @param	intplmiren [in]: mirror enable, bit0:left, bit1:right, bit2:top, bit3:down
* @param	intplmirvsel [in]: vertical down mirror postion select
* @param	intplcnt2sel [in]: initial count select 0:0x0, 1:0x7
* @return	none
*/
void 
gpHalCdspSetIntplMirEn(
	UINT8 intplmiren, 
	UINT8 intplmirvsel, 
	UINT8 intplcnt2sel
)
{
	pCdspReg0->cdspInpMirCtrl &= ~((1 << 7) | (1 << 4) | 0x0F);
	pCdspReg0->cdspInpMirCtrl |= ((intplcnt2sel & 0x01) << 7) | 
								((intplmirvsel & 0x01) << 4) | 
								(intplmiren & 0x0F);
}

void 
gpHalCdspGetIntplMirEn(
	UINT8 *intplmiren, 
	UINT8 *intplmirvsel, 
	UINT8 *intplcnt2sel
)
{
	*intplcnt2sel = (pCdspReg0->cdspInpMirCtrl & (1 << 7)) >> 7;
	*intplmirvsel = (pCdspReg0->cdspInpMirCtrl & (1 << 4)) >> 4;
	*intplmiren = pCdspReg0->cdspInpMirCtrl & 0x0F;
}

/**
* @brief	cdsp interpolation threshold set
* @param	int_low_thr [in]: low threshold
* @param	int_hi_thr [in]: heig threshold
* @return	none
*/
void 
gpHalCdspSetIntplThr(
	UINT8 int_low_thr, 
	UINT8 int_hi_thr
)
{
	pCdspReg0->cdspInpDnLowThr = int_low_thr;
	pCdspReg0->cdspInpDnHighThr = int_hi_thr;
}

void 
gpHalCdspGetIntplThr(
	UINT8 *int_low_thr, 
	UINT8 *int_hi_thr
)
{
	*int_low_thr = pCdspReg0->cdspInpDnLowThr;
	*int_hi_thr = pCdspReg0->cdspInpDnHighThr;
}

/**
* @brief	cdsp raw special mode set
* @param	rawspecmode [in]: raw special mode
* @return	none
*/
void 
gpHalCdspSetRawSpecMode(
	UINT8 rawspecmode
)
{
	if(rawspecmode > 6) {
		rawspecmode = 6;
	}
	
	pCdspReg0->cdspRgbSpeEfMode &= ~0x07;
	pCdspReg0->cdspRgbSpeEfMode |= rawspecmode;	
	
	/* reflected at vd ypdate */ 
	//pCdspReg0->cdspRgbSpeEfMode |= 1 << 3;	// Comi: it must set 0 for YUV special mode
}

UINT8 
gpHalCdspGetRawSpecMode(
	void
)
{
	return (pCdspReg0->cdspRgbSpeEfMode & 0x07);
}

/**
* @brief	cdsp edge source set
* @param	posyedgeen [in]: 0:raw, 1: YUV
* @return	none
*/
void
gpHalCdspSetEdgeSrc(
	UINT8 posyedgeen
)
{
	if(posyedgeen) {
		pCdspReg0->cdspInpEdgeCtrl |= 0x02;
	} else {
		pCdspReg0->cdspInpEdgeCtrl &= ~0x02;
	}
}

UINT8
gpHalCdspGetEdgeSrc(
	void
)
{
	return ((pCdspReg0->cdspInpEdgeCtrl & (1 << 1)) << 1);
}

/**
* @brief	cdsp edge enable
* @param	edgeen [in]: edge enable, effective when raw data input
* @return	none
*/
void 
gpHalCdspSetEdgeEn(
	UINT8 edgeen
)
{
	if(edgeen) {
		pCdspReg0->cdspInpEdgeCtrl |= 0x01;
	} else {
		pCdspReg0->cdspInpEdgeCtrl &= ~0x01;
	}
}

UINT8
gpHalCdspGetEdgeEn(
	void
)
{
	return (pCdspReg0->cdspInpEdgeCtrl & 0x01);
}

/**
* @brief	cdsp edge HPF matrix set
* @param	pLPF [in]: low pass filter parameter
* @return	none
*/
void 
gpHalCdspSetEdgeFilter(
	edge_filter_t *pLPF
)
{
	pLPF->LPF00 &= 0x0F;
	pLPF->LPF01 &= 0x0F;
	pLPF->LPF02 &= 0x0F;
	
	pLPF->LPF10 &= 0x0F;
	pLPF->LPF11 &= 0x0F;
	pLPF->LPF12 &= 0x0F;

	pLPF->LPF20 &= 0x0F;
	pLPF->LPF21 &= 0x0F;
	pLPF->LPF22 &= 0x0F;
	
	pCdspReg0->cdspInpHpfL00L01 = (pLPF->LPF01 << 4) | pLPF->LPF00;
	pCdspReg0->cdspInpHpfL02L10 = (pLPF->LPF10 << 4) | pLPF->LPF02;
	pCdspReg0->cdspInpHpfL11L12 = (pLPF->LPF12 << 4) | pLPF->LPF11;
	pCdspReg0->cdspInpHpfL20L21 = (pLPF->LPF21 << 4) | pLPF->LPF20;
	pCdspReg0->cdspInpHpfL22 = pLPF->LPF22;
}

/**
* @brief	cdsp edge HPF matrix get
* @param	pLPF [out]: low pass filter parameter
* @return	none
*/
void 
gpHalCdspGetEdgeFilter(
	edge_filter_t *pLPF
)
{
	pLPF->LPF00 = pCdspReg0->cdspInpHpfL00L01 & 0x0F;
	pLPF->LPF01 = (pCdspReg0->cdspInpHpfL00L01 >> 4) & 0x0F;
	pLPF->LPF02 = pCdspReg0->cdspInpHpfL02L10 & 0x0F;

	pLPF->LPF10 = (pCdspReg0->cdspInpHpfL02L10 >> 4) & 0x0F;
	pLPF->LPF11 = pCdspReg0->cdspInpHpfL11L12 & 0x0F;
	pLPF->LPF12 = (pCdspReg0->cdspInpHpfL11L12 >> 4) & 0x0F;
	
	pLPF->LPF20 = pCdspReg0->cdspInpHpfL20L21 & 0x0F;
	pLPF->LPF21 = (pCdspReg0->cdspInpHpfL20L21 >> 4) & 0x0F;
	pLPF->LPF22 = pCdspReg0->cdspInpHpfL22;
}

/**
* @brief	cdsp edge scale set
* @param	lhdiv [in]: L edge enhancement edge vale scale
* @param	lhtdiv [in]: L edge enhancement edge vale scale
* @param	lhcoring [in]: L core ring threshold
* @param	lhmode [in]: 1: default matrix, 0: enable paroramming matrix
* @return	none
*/
void 
gpHalCdspSetEdgeLCoring(
	UINT8 lhdiv, 
	UINT8 lhtdiv, 
	UINT8 lhcoring, 
	UINT8 lhmode
)
{
	UINT8 lh, lht;

	lh = lht = 0;
	while(1) // lhdiv=2^lh
	{
		lhdiv >>= 1;
		if(lhdiv) {
			lh++;
		} else {
			break;
		}

		if(lh >= 7) {
			break;
		}
	}
	
	while(1) // lhtdiv=2^lht
	{
		lhtdiv >>= 1;
		if(lhtdiv) { 
			lht++;
		} else { 
			break;
		}
		
		if(lht >= 7) {
			break;
		}
	}
	
	if(lh > 7) {
		lh = 7;
	}

	if(lht> 7) {
		lht = 7;
	}
	pCdspReg0->cdspInpLHDiv = (lht << 4)|lh;
	pCdspReg0->cdspInpLHCor = lhcoring;
	pCdspReg0->cdspInpLHMode = lhmode & 0x1; 
}

void 
gpHalCdspGetEdgeLCoring(
	UINT8 *lhdiv, 
	UINT8 *lhtdiv, 
	UINT8 *lhcoring, 
	UINT8 *lhmode
)
{
	*lhdiv = pCdspReg0->cdspInpLHDiv & 0x0F;
	*lhtdiv = (pCdspReg0->cdspInpLHDiv >> 4) & 0x0F; 
	*lhcoring = pCdspReg0->cdspInpLHCor;
	*lhmode = pCdspReg0->cdspInpLHMode & 0x01;
}

/**
* @brief	cdsp edge amp set
* @param	ampga [in]: 0:1, 1:2, 2:3, 3:4
* @param	edgedomain [in]: 0:add edge on y value, 1:add edge on rgb value
* @return	none
*/
void 
gpHalCdspSetEdgeAmpga(
	UINT8 ampga
)
{
	ampga &= 0x03;
/*	switch(ampga)
	{
	case 0:
	case 1:
		ampga = 0;
		break;
	case 2:
		ampga = 1;
		break;
	case 3:
		ampga = 2;
		break;
	case 4:
		ampga = 3;
		break;
	default:
		ampga = 3;
		break;		
	}*/

	pCdspReg0->cdspInpAmpga = ampga;
}

UINT8 
gpHalCdspGetEdgeAmpga(
	void 
)
{
	UINT8 ampga = pCdspReg0->cdspInpAmpga;
		
/*	switch(ampga)
	{
	case 0:
	case 1:
		ampga = 0;
		break;
	case 2:
		ampga = 1;
		break;
	case 3:
		ampga = 2;
		break;
	case 4:
		ampga = 3;
		break;
	default:
		ampga = 3;
		break;		
	}*/
	
	return ampga;
}

/**
* @brief	cdsp edge domain set
* @param	edgedomain [in]: 0:add edge on y value, 1:add edge on rgb value
* @return	none
*/
void
gpHalCdspSetEdgeDomain(
	UINT8 edgedomain
)
{
	if(edgedomain) {
		pCdspReg0->cdspInpEdgeCtrl |= (1 << 2);
	} else {
		pCdspReg0->cdspInpEdgeCtrl &= ~(1 << 2);
	}
}

UINT8
gpHalCdspGetEdgeDomain(
	void
)
{
	return ((pCdspReg0->cdspInpEdgeCtrl & (1 << 2)) >> 2);
}

/**
* @brief	cdsp edge Q threshold set
* @param	Qthr [in]: edge threshold 
* @return	none
*/
void
gpHalCdspSetEdgeQthr(
	UINT8 Qthr
)
{
	pCdspReg0->cdspInpQThr = Qthr;
}

UINT32
gpHalCdspGetEdgeQCnt(
	void
)
{
	return ((pCdspReg0->cdspInpQCnt[2] << 16) | (pCdspReg0->cdspInpQCnt[1] << 8) | pCdspReg0->cdspInpQCnt[0]);
}

/**
* @brief	cdsp edge lut table enable
* @param	eluten [in]: edge lut table enable
* @return	none
*/
void 
gpHalCdspSetEdgeLutTableEn(
	UINT8 eluten
)
{
	if(eluten) {
		pCdspReg0->cdspInpEdgeCtrl |= 0x10;
	} else {
		pCdspReg0->cdspInpEdgeCtrl &= ~0x10;
	}
}

UINT8
gpHalCdspGetEdgeLutTableEn(
	void
)
{
	return (pCdspReg0->cdspInpEdgeCtrl >> 4) & 0x01;
}

/**
* @brief	cdsp edge lut table init
* @param	pLutEdgeTable [in]: table pointer
* @return	none
*/
void
gpHalCdspInitEdgeLut(
	UINT8 *pLutEdgeTable
)
{
	UINT32 i;

	pCdspReg0->cdspSwitchClk = 0x04;
	pCdspReg0->cdspMacroSel = 0x04;
	pCdspReg0->cdspCPUSramEn = 0x01;
	
	for(i=0; i<256; i++)
	{
		pEdgeLutData->EdgeTable[i] = pLutEdgeTable[i];
	}
	
	pCdspReg0->cdspCPUSramEn = 0x00;
	pCdspReg0->cdspMacroSel = 0x00;
	pCdspReg0->cdspSwitchClk = 0x00;

	if(DEBUG_SRAM_OUTPUT)
	{
		printk("Input Edge table:\n");
		for(i=0; i<256; i++)
		{
			printk("0x%02x, ", pLutEdgeTable[i]);
			if((i%16) == 15)
				printk("\n");
		}

		pCdspReg0->cdspSwitchClk = 0x04;
		pCdspReg0->cdspMacroSel = 0x04;
		pCdspReg0->cdspCPUSramEn = 0x01;

		printk("SRAM Edge table:\n");
		for(i=0; i<256; i++)
		{
			printk("0x%02x, ", pEdgeLutData->EdgeTable[i]);
			if((i%16) == 15)
				printk("\n");
		}

		pCdspReg0->cdspCPUSramEn = 0x00;
		pCdspReg0->cdspMacroSel = 0x00;
		pCdspReg0->cdspSwitchClk = 0x00;
	}
}

/**
* @brief	cdsp set pre r and b clamp set
* @param	pre_rb_clamp [in]: clamp value
* @return	none
*/
void
gpHalCdspSetPreRBClamp(
	UINT8 pre_rb_clamp
)
{
	pCdspReg0->cdspInpPreRbClamp = pre_rb_clamp;
}

UINT8
gpHalCdspGetPreRBClamp(
	void
)
{
	return pCdspReg0->cdspInpPreRbClamp;
}

/**
* @brief	cdsp color matrix enable
* @param	colcorren [in]: color matrix enable
* @return	none
*/
void 
gpHalCdspSetColorMatrixEn(
	UINT8 colcorren
)
{
	if(colcorren) {		
		pCdspReg0->cdspCcEn |= 0x01;	
	} else {		
		pCdspReg0->cdspCcEn &= ~0x01;	
	}
}

UINT8 
gpHalCdspGetColorMatrixEn(
	void
)
{
	return (pCdspReg0->cdspCcEn & 0x1);
}

/**
* @brief	cdsp color matrix set parameter
* @param	CMatrix [in]: color matrix parameter
* @return	none
*/
void 
gpHalCdspSetColorMatrix(
	color_matrix_t *CMatrix
)
{	
	pCdspReg0->cdspCcCof00[0] = (UINT8)(CMatrix->CcCof00 & 0x00FF);
	pCdspReg0->cdspCcCof00[1] = (UINT8)((CMatrix->CcCof00 & 0x0300) >> 8);
	pCdspReg0->cdspCcCof01[0] = (UINT8)(CMatrix->CcCof01 & 0x00FF);
	pCdspReg0->cdspCcCof01[1] = (UINT8)((CMatrix->CcCof01 & 0x0300) >> 8);
	pCdspReg0->cdspCcCof02[0] = (UINT8)(CMatrix->CcCof02 & 0x00FF);
	pCdspReg0->cdspCcCof02[1] = (UINT8)((CMatrix->CcCof02 & 0x0300) >> 8);
	
	pCdspReg0->cdspCcCof10[0] = (UINT8)(CMatrix->CcCof10 & 0x00FF);
	pCdspReg0->cdspCcCof10[1] = (UINT8)((CMatrix->CcCof10 & 0x0300) >> 8);
	pCdspReg0->cdspCcCof11[0] = (UINT8)(CMatrix->CcCof11 & 0x00FF);
	pCdspReg0->cdspCcCof11[1] = (UINT8)((CMatrix->CcCof11 & 0x0300) >> 8);
	pCdspReg0->cdspCcCof12[0] = (UINT8)(CMatrix->CcCof12 & 0x00FF);
	pCdspReg0->cdspCcCof12[1] = (UINT8)((CMatrix->CcCof12 & 0x0300) >> 8);
		
	pCdspReg0->cdspCcCof20[0] = (UINT8)(CMatrix->CcCof20 & 0x00FF);
	pCdspReg0->cdspCcCof20[1] = (UINT8)((CMatrix->CcCof20 & 0x0300) >> 8);
	pCdspReg0->cdspCcCof21[0] = (UINT8)(CMatrix->CcCof21 & 0x00FF);
	pCdspReg0->cdspCcCof21[1] = (UINT8)((CMatrix->CcCof21 & 0x0300) >> 8);
	pCdspReg0->cdspCcCof22[0] = (UINT8)(CMatrix->CcCof22 & 0x00FF);
	pCdspReg0->cdspCcCof22[1] = (UINT8)((CMatrix->CcCof22 & 0x0300) >> 8);
}

/**
* @brief	cdsp color matrix get parameter
* @param	CMatrix [out]: color matrix parameter
* @return	none
*/
void 
gpHalCdspGetColorMatrix(
	color_matrix_t *CMatrix
)
{	
	CMatrix->CcCof00 = (pCdspReg0->cdspCcCof00[1] << 8) | pCdspReg0->cdspCcCof00[0];
	CMatrix->CcCof01 = (pCdspReg0->cdspCcCof01[1] << 8) | pCdspReg0->cdspCcCof01[0];
	CMatrix->CcCof02 = (pCdspReg0->cdspCcCof02[1] << 8) | pCdspReg0->cdspCcCof02[0];

	CMatrix->CcCof10 = (pCdspReg0->cdspCcCof10[1] << 8) | pCdspReg0->cdspCcCof10[0];
	CMatrix->CcCof11 = (pCdspReg0->cdspCcCof11[1] << 8) | pCdspReg0->cdspCcCof11[0];
	CMatrix->CcCof12 = (pCdspReg0->cdspCcCof12[1] << 8) | pCdspReg0->cdspCcCof12[0];
	
	CMatrix->CcCof20 = (pCdspReg0->cdspCcCof20[1] << 8) | pCdspReg0->cdspCcCof20[0];
	CMatrix->CcCof21 = (pCdspReg0->cdspCcCof21[1] << 8) | pCdspReg0->cdspCcCof21[0];
	CMatrix->CcCof22 = (pCdspReg0->cdspCcCof22[1] << 8) | pCdspReg0->cdspCcCof22[0];
}

/**
* @brief	cdsp a and b clamp set
* @param	rbclampen [in]: clamp enable
* @param	rbclamp [in]: clamp size set
* @return	none
*/
void 
gpHalCdspSetRBClamp(
	UINT8 rbclampen,
	UINT8 rbclamp
)
{
	pCdspReg0->cdspRbClampEn = rbclampen & 0x01;
	pCdspReg0->cdspRbClampVal = rbclamp;
}

void 
gpHalCdspGetRBClamp(
	UINT8 *rbclampen,
	UINT8 *rbclamp
)
{
	*rbclampen = pCdspReg0->cdspRbClampEn & 0x01;
	 *rbclamp = pCdspReg0->cdspRbClampVal;
}

/**
* @brief	cdsp uv division set
* @param	uvDiven [in]: un div function enable
* @return	none
*/
void
gpHalCdspSetUvDivideEn(
	UINT8 uvDiven
)
{
	if(uvDiven) {
		pCdspReg0->cdspUvSclYuvInsertEn |= 0x01;
	} else {
		pCdspReg0->cdspUvSclYuvInsertEn &= ~0x01;
	}
}

UINT8
gpHalCdspGetUvDivideEn(
	void
)
{
	return (pCdspReg0->cdspUvSclYuvInsertEn & 0x01);
}

/**
* @brief	cdsp uv division set
* @param	UVDivide [in]: y value Tn
* @return	none
*/
void
gpHalCdspSetUvDivide(
	uv_divide_t *UVDivide
)
{
	pCdspReg0->cdspUvSclDiv18 = UVDivide->YT1;
	pCdspReg0->cdspUvSclDiv28 = UVDivide->YT2;
	pCdspReg0->cdspUvSclDiv38 = UVDivide->YT3;
	pCdspReg0->cdspUvSclDiv48 = UVDivide->YT4;
	pCdspReg0->cdspUvSclDiv58 = UVDivide->YT5;
	pCdspReg0->cdspUvSclDiv68 = UVDivide->YT6;
}

/**
* @brief	cdsp uv division get
* @param	UVDivide [out]: y value Tn
* @return	none
*/
void
gpHalCdspGetUvDivide(
	uv_divide_t *UVDivide
)
{
	UVDivide->YT1 = pCdspReg0->cdspUvSclDiv18;
	UVDivide->YT2 = pCdspReg0->cdspUvSclDiv28;
	UVDivide->YT3 = pCdspReg0->cdspUvSclDiv38;
	UVDivide->YT4 = pCdspReg0->cdspUvSclDiv48;
	UVDivide->YT5 = pCdspReg0->cdspUvSclDiv58;
	UVDivide->YT6 = pCdspReg0->cdspUvSclDiv68;
}

/**
* @brief	cdsp yuv mux path set
* @param	redoedge [in]: Set mux, 0:yuv path, 1:yuv path6
* @return	none
*/
void 
gpHalCdspSetMuxPath(
	UINT8 redoedge
)
{
	if(redoedge) {
		pCdspReg1->cdspDo |= 0x02;		
	} else {
		pCdspReg1->cdspDo &= ~0x02;
	}
}

/**
* @brief	cdsp yuv 444 insert enable
* @param	yuvinserten [in]: yuv 444 insert enable
* @return	none
*/
void 
gpHalCdspSetYuv444InsertEn(
	UINT8 yuvinserten
)
{
	if(yuvinserten) {
		pCdspReg0->cdspUvSclYuvInsertEn |= 0x10;
	} else {
		pCdspReg0->cdspUvSclYuvInsertEn &= ~0x10;
	}
}

UINT8 
gpHalCdspGetYuv444InsertEn(
	void
)
{
	return (pCdspReg0->cdspUvSclYuvInsertEn >> 4) & 0x01;
}

/**
* @brief	cdsp yuv coring threshold value set
* @param	y_corval_coring [in]: y coring threshold value
* @param	u_corval_coring [in]: y coring threshold value
* @param	v_corval_coring [in]: y coring threshold value
* @return	none
*/
void 
gpHalCdspSetYuvCoring(
	UINT8 y_corval_coring, 
	UINT8 u_corval_coring, 
	UINT8 v_corval_coring
)
{
	pCdspReg0->cdspYCoring = y_corval_coring;
	pCdspReg0->cdspUCoring = u_corval_coring;
	pCdspReg0->cdspVCoring = v_corval_coring;
}

void 
gpHalCdspGetYuvCoring(
	UINT8 *y_corval_coring, 
	UINT8 *u_corval_coring, 
	UINT8 *v_corval_coring
)
{
	*y_corval_coring = pCdspReg0->cdspYCoring;
	*u_corval_coring = pCdspReg0->cdspUCoring;
	*v_corval_coring = pCdspReg0->cdspVCoring;	
}

/**
* @brief	cdsp h average function
* @param	yuvhavgmiren [in]: mirror enable, bit0:left, bit1: right
* @param	ytype [in]: Y horizontal LPF type, 0:disable, 1:3tap, 2:5tap
* @param	utype [in]: U horizontal LPF type, 0:disable, 1:3tap, 2:5tap
* @param	vtype [in]: V horizontal LPF type, 0:disable, 1:3tap, 2:5tap
* @return	none
*/
void 
gpHalCdspSetYuvHAvg(
	UINT8 yuvhavgmiren, 
	UINT8 ytype, 
	UINT8 utype, 
	UINT8 vtype
)
{
	pCdspReg0->cdspYuvHAvgMirEn = yuvhavgmiren & 0x03;
	pCdspReg0->cdspYuvHAvgLpfType = ((vtype & 0x03) << 4) | 
									((utype & 0x03) << 2) |
									(ytype & 0x03);
}

void 
gpHalCdspGetYuvHAvg(
	UINT8 *yuvhavgmiren, 
	UINT8 *ytype, 
	UINT8 *utype, 
	UINT8 *vtype
)
{
	*yuvhavgmiren = pCdspReg0->cdspYuvHAvgMirEn;
	*ytype = pCdspReg0->cdspYuvHAvgLpfType & 0x03;
	*utype = (pCdspReg0->cdspYuvHAvgLpfType >> 2) & 0x03;
	*vtype = (pCdspReg0->cdspYuvHAvgLpfType >> 4) & 0x03;
}

/**
* @brief	cdsp yuv special mode set
* @param	yuvspecmode [in]: yuv special mode
* @return	none
*/
void 
gpHalCdspSetYuvSpecMode(
	UINT8 yuvspecmode
)
{
	if(yuvspecmode > 7) {
		yuvspecmode = 0;
	}
	
	pCdspReg0->cdspYuvSpeEfMode = yuvspecmode;
	/* reflected at vd update */
	pCdspReg0->cdspYuvSpeEfMode |= (1 << 3);	
}

UINT8
gpHalCdspGetYuvSpecMode(
	void
)
{
	return (pCdspReg0->cdspYuvSpeEfMode & 0x07);
}

/**
* @brief	cdsp yuv special mode Binary threshold set
* @param	binarthr [in]: Binary threshold set
* @return	none
*/
void 
gpHalCdspSetYuvSpecModeBinThr(
	UINT8 binarthr
)
{
	/* vaild when special mode = 2, (binarize) */
	pCdspReg0->cdspYuvSpEfBiThr = binarthr;	
}

UINT8 
gpHalCdspGetYuvSpecModeBinThr(
	void
)
{
	return pCdspReg0->cdspYuvSpEfBiThr;	
}

/**
* @brief	cdsp yuv special mode brightness and contrast adjust enable
* @param	YbYcEn [in]: enable y brightness and contrast adjust 
* @return	none
*/
void 
gpHalCdspSetBriContEn(
	UINT8 YbYcEn
)
{
	/* vaild when yuv special mode = 0x3 */
	if(YbYcEn) { 
		pCdspReg0->cdspYbYcEn |= 0x01; 
	} else {
		pCdspReg0->cdspYbYcEn &= ~0x01;	
	}	
}

UINT8
gpHalCdspGetBriContEn(
	void
)
{
	return (pCdspReg0->cdspYbYcEn & 0x01);
}

/**
* @brief	cdsp yuv special mode offset set
* @param	y_offset [in]: Y offset set 
* @param	u_offset [in]: U offset set 
* @param	v_offset [in]: V offset set 
* @return	none
*/
void 
gpHalCdspSetYuvSPEffOffset(
	UINT8 y_offset, 
	UINT8 u_offset, 
	UINT8 v_offset
)
{
	pCdspReg0->cdspYuvSpEfYOff = y_offset;
	pCdspReg0->cdspYuvSpEfUOff = u_offset;
	pCdspReg0->cdspYuvSpEfVOff = v_offset;
}

void 
gpHalCdspGetYuvSPEffOffset(
	UINT8 *y_offset, 
	UINT8 *u_offset, 
	UINT8 *v_offset
)
{
	*y_offset = pCdspReg0->cdspYuvSpEfYOff;
	*u_offset = pCdspReg0->cdspYuvSpEfUOff;
	*v_offset = pCdspReg0->cdspYuvSpEfVOff;
}

/**
* @brief	cdsp yuv special mode offset set
* @param	y_scale [in]: Y scale set 
* @param	u_scale [in]: U scale set 
* @param	v_scale [in]: V scale set 
* @return	none
*/
void 
gpHalCdspSetYuvSPEffScale(
	UINT8 y_scale, 
	UINT8 u_scale, 
	UINT8 v_scale
)
{
	pCdspReg0->cdspYuvSpEfYScl = y_scale;
	pCdspReg0->cdspYuvSpEfUScl = u_scale;
	pCdspReg0->cdspYuvSpEfVScl = v_scale;
}

void 
gpHalCdspGetYuvSPEffScale(
	UINT8 *y_scale, 
	UINT8 *u_scale, 
	UINT8 *v_scale
)
{
	*y_scale = pCdspReg0->cdspYuvSpEfYScl;
	*u_scale = pCdspReg0->cdspYuvSpEfUScl;
	*v_scale = pCdspReg0->cdspYuvSpEfVScl;
}

/**
* @brief	cdsp yuv special mode hue set
* @param	u_huesindata [in]: sin data for hue rotate for u
* @param	u_huecosdata [in]: cos data for hue rotate for u
* @param	v_huesindata [in]: sin data for hue rotate for v
* @param	v_huecosdata [in]: cos data for hue rotate for v
* @return 	none
*/
void gpHalCdspSetYuvSPHue(
	UINT8 u_huesindata, 
	UINT8 u_huecosdata,	
	UINT8 v_huesindata, 
	UINT8 v_huecosdata
)
{
	pCdspReg0->cdspHueSinData1 = u_huesindata;
	pCdspReg0->cdspHueCosData1 = u_huecosdata;
	pCdspReg0->cdspHueSinData2 = v_huesindata;
	pCdspReg0->cdspHueCosData2 = v_huecosdata;
}

void gpHalCdspGetYuvSPHue(
	UINT8 *u_huesindata, 
	UINT8 *u_huecosdata,	
	UINT8 *v_huesindata, 
	UINT8 *v_huecosdata
)
{
	*u_huesindata = pCdspReg0->cdspHueSinData1;
	*u_huecosdata = pCdspReg0->cdspHueCosData1;
	*v_huesindata = pCdspReg0->cdspHueSinData2;
	*v_huecosdata = pCdspReg0->cdspHueCosData2;
}

/**
* @brief	cdsp yuv h scale down enable
* @param	yuvhscale_en [in]: yuv h scale enable
* @param	yuvhscale_mode [in]: yuv h scale skip pixel mode 0: drop, 1:filter
* @return	none
*/
void 
gpHalCdspSetYuvHScaleEn(
	UINT8 yuvhscale_en, 
	UINT8 yuvhscale_mode
)
{
	pCdspReg0->cdspYuvHSclCtrl &= ~(1 << 4 | 0x01);
	pCdspReg0->cdspYuvHSclCtrl |= ((yuvhscale_en & 0x01) << 4) | (yuvhscale_mode & 0x01);

#if 1
	/* immediate update */
	pCdspReg0->cdspSclUpdateMode = 0x00;
#else	
	/* reflected at next vaild vd edge */
	pCdspReg0->cdspSclUpdateMode = 0x01;
#endif
}

void 
gpHalCdspGetYuvHScaleEn(
	UINT8 *yuvhscale_en, 
	UINT8 *yuvhscale_mode
)
{
	*yuvhscale_en = (pCdspReg0->cdspYuvHSclCtrl >> 4) & 0x01;
	*yuvhscale_mode = pCdspReg0->cdspYuvHSclCtrl & 0x01;
}

/**
* @brief	cdsp yuv v scale down enable
* @param	vscale_en [in]: yuv v scale enable
* @param	vscale_mode [in]: yuv v scale skip pixel mode 0: drop, 1:filter
* @return	none
*/
void 
gpHalCdspSetYuvVScaleEn(
	UINT8 vscale_en, 
	UINT8 vscale_mode
)
{
	pCdspReg0->cdspYuvVSclCtrl &= ~(1 << 4 | 0x01);
	pCdspReg0->cdspYuvVSclCtrl |= ((vscale_en & 0x01) << 4) | (vscale_mode & 0x01);			

#if 1
	/* immediate update */
	pCdspReg0->cdspSclUpdateMode = 0x00;
#else	
	/* reflected at next vaild vd edge */
	pCdspReg0->cdspSclUpdateMode = 0x01;
#endif
}

void 
gpHalCdspGetYuvVScaleEn(
	UINT8 *vscale_en, 
	UINT8 *vscale_mode
)
{
	*vscale_en = (pCdspReg0->cdspYuvVSclCtrl >> 4) & 0x01;
	*vscale_mode = pCdspReg0->cdspYuvVSclCtrl & 0x01;
}

/**
* @brief	cdsp yuv h scale down set
* @param	hscaleaccinit [in]: yuv h scale accumation init vale set
* @param	yuvhscalefactor [in]: yuv h scale factor set
* @return	none
*/
void 
gpHalCdspSetYuvHScale(
	UINT16 hscaleaccinit, 
	UINT16 yuvhscalefactor
)
{
	pCdspReg0->cdspYuvHSclIni[0] = (UINT8)(hscaleaccinit & 0x00FF);
	pCdspReg0->cdspYuvHSclIni[1] = (UINT8)((hscaleaccinit & 0xFF00) >> 8);
	pCdspReg0->cdspYuvHSclFactor[0] = (UINT8)(yuvhscalefactor & 0x00FF);
	pCdspReg0->cdspYuvHSclFactor[1] = (UINT8)((yuvhscalefactor & 0xFF00) >> 8);
}

/**
* @brief	cdsp yuv v scale down set
* @param	vscaleaccinit [in]: yuv v scale accumation init vale set
* @param	yuvvscalefactor [in]: yuv v scale factor set
* @return	none
*/
void 
gpHalCdspSetYuvVScale(
	UINT16 vscaleaccinit, 
	UINT16 yuvvscalefactor
)
{
	pCdspReg0->cdspYuvVSclIni[0] = (UINT8)(vscaleaccinit & 0x00FF);
	pCdspReg0->cdspYuvVSclIni[1] = (UINT8)((vscaleaccinit & 0xFF00) >> 8);
	pCdspReg0->cdspYuvVSclFactor[0] = (UINT8)(yuvvscalefactor & 0x00FF);
	pCdspReg0->cdspYuvVSclFactor[1] = (UINT8)((yuvvscalefactor & 0xFF00) >> 8);
}

/**
* @brief	cdsp uv suppression enable
* @param	suppressen [in]: uv suppression enable, effective when yuv data input.
* @return	none
*/
void 
gpHalCdspSetUvSupprEn(
	UINT8 suppressen
)
{
	if(suppressen) {
		pCdspReg0->cdspSuppEn |= 0x01;
	} else {
		pCdspReg0->cdspSuppEn &= ~0x01;
	}
}

UINT8 
gpHalCdspGetUvSupprEn(
	void
)
{
	return (pCdspReg0->cdspSuppEn & 0x01);
}

/**
* @brief	cdsp uv suppression set
* @param	yuvsupmirvsel [in]: 1:cnt3eq2, 0:cnt3eq1
* @param	fstextsolen [in]: enable first sol when extened 2 line
* @param	yuvsupmiren [in]: suppression enable, bit0:top, bit1:down, bit2:left, bit3:right
* @return	none
*/
void 
gpHalCdspSetUvSuppr(
	UINT8 yuvsupmirvsel, 
	UINT8 fstextsolen, 
	UINT8 yuvsupmiren
)
{
	pCdspReg0->cdspSuppMirCtrl = ((yuvsupmiren & 0x0F) << 4) | 
								((fstextsolen & 0x01) << 1) | 
								(yuvsupmirvsel & 0x01);
}

void 
gpHalCdspGetUvSuppr(
	UINT8 *yuvsupmirvsel, 
	UINT8 *fstextsolen, 
	UINT8 *yuvsupmiren
)
{
	*yuvsupmirvsel = pCdspReg0->cdspSuppMirCtrl & 0x01;
	*fstextsolen = (pCdspReg0->cdspSuppMirCtrl & (0x01 << 4)) >> 1;
	*yuvsupmiren = (pCdspReg0->cdspSuppMirCtrl & (0x0F << 4)) >> 4;
}

/**
* @brief	cdsp y denoise enable
* @param	denoisen [in]: y denoise enable
* @return	none
*/
void 
gpHalCdspSetYDenoiseEn(
	UINT8 denoisen
)
{
	if(denoisen) {
		pCdspReg0->cdspSuppEn |= 0x02;
	} else {
		pCdspReg0->cdspSuppEn &= ~0x02;
	}
}

UINT8
gpHalCdspGetYDenoiseEn(
	void
)
{
	return (pCdspReg0->cdspSuppEn >> 1) & 0x01;
}

/**
* @brief	cdsp y denoise set
* @param	denoisethrl [in]: y denoise low threshold
* @param	denoisethrwth [in]: y denoise bandwidth set
* @param	yhtdiv [in]: y denoise divider
* @return	none
*/
void 
gpHalCdspSetYDenoise(
	UINT8 denoisethrl, 
	UINT8 denoisethrwth, 
	UINT8 yhtdiv
)
{
	UINT32 yth, thrwth;

	yth = thrwth = 0;
	while(1) // yhtdiv=2^yht
	{
		yhtdiv >>= 1;
		if(yhtdiv) {
			yth++;
		} else {
			break;
		}

		if(yth >= 7) {
			break;
		}
	}

	while(1) // denoisethrwth=2^thrwth
	{
		denoisethrwth >>= 1;
		if(denoisethrwth) {
			thrwth++;
		} else {
			break;
		}
		
		if(thrwth >= 7) {
			break;
		}
	}
	
	if(yth > 7) {
		yth = 7;
	}

	if(thrwth > 7) {
		thrwth = 7;
	}
	pCdspReg0->cdspDnLowThr = denoisethrl;
	pCdspReg0->cdspDnThrWth = thrwth;
	pCdspReg0->cdspDnYhtDiv = yth;
}

void 
gpHalCdspGetYDenoise(
	UINT8 *denoisethrl, 
	UINT8 *denoisethrwth, 
	UINT8 *yhtdiv
)
{
	*denoisethrl = pCdspReg0->cdspDnLowThr;
	*denoisethrwth = pCdspReg0->cdspDnThrWth;
	*yhtdiv = pCdspReg0->cdspDnYhtDiv;
}

/**
* @brief	cdsp y LPF enable
* @param	lowyen [in]: y LPF enable
* @return	none
*/
void 
gpHalCdspSetYLPFEn(
	UINT8 lowyen
)
{
	if(lowyen) {
		pCdspReg0->cdspSuppEn |= 0x04;
	} else {
		pCdspReg0->cdspSuppEn &= ~0x04;
	}
}

UINT8
gpHalCdspGetYLPFEn(
	void
)
{
	return (pCdspReg0->cdspSuppEn >> 2) & 0x01;
}

/**
* @brief	cdsp new denoise enable
* @param	newdenoiseen [in]: new denoise enable, effective when raw data input.
* @return	none
*/
void 
gpHalCdspSetNewDenoiseEn(
	UINT8 newdenoiseen
)
{
	if(newdenoiseen) {
		pCdspReg0->cdspNdnMirCtrl |= 0x01;
	} else {
		pCdspReg0->cdspNdnMirCtrl &= ~0x01;
	}
}

/**
* @brief	get cdsp new denoise
* @param	
* @return	status
*/
UINT8 
gpHalCdspGetNewDenoiseEn(
	void
)
{
	return (pCdspReg0->cdspNdnMirCtrl & 0x01);
}

/**
* @brief	cdsp new denoise set
* @param	ndmirvsel [in]: 1:cnt3eq2, 0:cnt3eq1
* @param	ndmiren [in]: new denoise enable, bit0:top, bit1:down, bit2:left, bit3:right
* @return	none
*/
void 
gpHalCdspSetNewDenoise(
	UINT8 ndmirvsel, 
	UINT8 ndmiren
)
{
	pCdspReg0->cdspNdnMirCtrl &= ~((0x0F << 4)|(0x1 << 1));
	pCdspReg0->cdspNdnMirCtrl |= ((ndmiren & 0x0F) << 4) | ((ndmirvsel & 0x01) << 1);
}

/**
* @brief	get cdsp new denoise
* @param	ndmirvsel [out]: 1:cnt3eq2, 0:cnt3eq1
* @param	ndmiren [out]: new denoise enable, bit0:top, bit1:down, bit2:left, bit3:right
* @return	none
*/
void 
gpHalCdspGetNewDenoise(
	UINT8 *ndmirvsel, 
	UINT8 *ndmiren
)
{
	*ndmirvsel = (pCdspReg0->cdspNdnMirCtrl >> 1) & 0x01;
	*ndmiren = (pCdspReg0->cdspNdnMirCtrl >> 4) & 0x0F;
}

/**
* @brief	cdsp new denoise edge enable
* @param	ndedgeen [in]: new denoise edge enable
* @param	ndeluten [in]: new denoise edge lut enable
* @return	none
*/
void 
gpHalCdspSetNdEdgeEn(
	UINT8 ndedgeen,
	UINT8 ndeluten
)
{
	if(ndedgeen) {
		pCdspReg0->cdspNdnEdgeCtrl |= 0x01;
	} else {
		pCdspReg0->cdspNdnEdgeCtrl &= ~0x01;
	}
	
	if(ndeluten) {
		pCdspReg0->cdspNdnEdgeCtrl |= 0x10;
	} else {
		pCdspReg0->cdspNdnEdgeCtrl &= ~0x10;
	}

	//printk("%s: ndedgeen = %d, ndeluten = %d\n", __FUNCTION__, ndedgeen, ndeluten);
}

/**
* @brief	get cdsp new denoise edge enable
* @param	ndedgeen [out]: new denoise edge enable
* @param	ndeluten [out]: new denoise edge lut enable
* @return	none
*/
void 
gpHalCdspGetNdEdgeEn(
	UINT8 *ndedgeen,
	UINT8 *ndeluten
)
{
	*ndedgeen = pCdspReg0->cdspNdnEdgeCtrl & 0x01;
	*ndeluten = (pCdspReg0->cdspNdnEdgeCtrl >> 4) & 0x01;

	//printk("%s: ndedgeen = %d, ndeluten = %d\n", __FUNCTION__, *ndedgeen, *ndeluten);
}

/**
* @brief	cdsp new denoise edge HPF matrix set
* @param	NDEdgeFilter [in]: 
* @return	none
*/
void 
gpHalCdspSetNdEdgeFilter(
	edge_filter_t *NDEdgeFilter
)
{
	NDEdgeFilter->LPF00 &= 0x0F;
	NDEdgeFilter->LPF01 &= 0x0F;
	NDEdgeFilter->LPF02 &= 0x0F;
	
	NDEdgeFilter->LPF10 &= 0x0F;
	NDEdgeFilter->LPF11 &= 0x0F;
	NDEdgeFilter->LPF12 &= 0x0F;

	NDEdgeFilter->LPF20 &= 0x0F;
	NDEdgeFilter->LPF21 &= 0x0F;
	NDEdgeFilter->LPF22 &= 0x0F;

	pCdspReg0->cdspNdnHpfL00L01 = (NDEdgeFilter->LPF01 << 4) | NDEdgeFilter->LPF00;
	pCdspReg0->cdspNdnHpfL02L10 = (NDEdgeFilter->LPF10 << 4) | NDEdgeFilter->LPF02;
	pCdspReg0->cdspNdnHpfL11L12 = (NDEdgeFilter->LPF12 << 4) | NDEdgeFilter->LPF11;
	pCdspReg0->cdspNdnHpfL20L21 = (NDEdgeFilter->LPF21 << 4) | NDEdgeFilter->LPF20;
	pCdspReg0->cdspNdnHpfL22 = NDEdgeFilter->LPF22;
}

/**
* @brief	cdsp new denoise edge HPF matrix get
* @param	NDEdgeFilter [out]: 
* @return	none
*/
void 
gpHalCdspGetNdEdgeFilter(
	edge_filter_t *NDEdgeFilter
)
{
	NDEdgeFilter->LPF00 = pCdspReg0->cdspNdnHpfL00L01 & 0x0F;
	NDEdgeFilter->LPF01 = (pCdspReg0->cdspNdnHpfL00L01 >> 4) & 0x0F;
	NDEdgeFilter->LPF02 = pCdspReg0->cdspNdnHpfL02L10 & 0x0F;

	NDEdgeFilter->LPF10 = (pCdspReg0->cdspNdnHpfL02L10 >> 4) & 0x0F;
	NDEdgeFilter->LPF11 = pCdspReg0->cdspNdnHpfL11L12 & 0x0F;
	NDEdgeFilter->LPF12 = (pCdspReg0->cdspNdnHpfL11L12 >> 4) & 0x0F;

	NDEdgeFilter->LPF20 = pCdspReg0->cdspNdnHpfL20L21 & 0x0F;
	NDEdgeFilter->LPF21 = (pCdspReg0->cdspNdnHpfL20L21 >> 4) & 0x0F;
	NDEdgeFilter->LPF22 = pCdspReg0->cdspNdnHpfL22;
}

/**
* @brief	cdsp new denoise edge scale set
* @param	ndlhdiv [in]: L edge enhancement edge vale scale
* @param	ndlhtdiv [in]: L edge enhancement edge vale scale
* @param	ndlhcoring [in]: L core ring threshold
* @param	ndlhmode [in]: 1: default matrix, 0: enable paroramming matrix
* @return	none
*/
void 
gpHalCdspSetNdEdgeLCoring(
	UINT8 ndlhdiv, 
	UINT8 ndlhtdiv, 
	UINT8 ndlhcoring, 
	UINT8 ndlhmode
)
{
	UINT8 lh, lht;
#if 0
	lh = lht = 0;
	while(1) // lhdiv=2^lh
	{
		ndlhdiv >>= 1;
		if(ndlhdiv) {
			lh++;
		} else { 
			break;
		}
		
		if(lh >= 7) {
			break;
		}
	}
	
	while(1) // lhtdiv=2^lht
	{
		ndlhtdiv >>= 1;
		if(ndlhtdiv) {
			lht++;
		} else { 
			break;
		}

		if(lht >= 7) {
			break;
		}
	}
	
	ndlhmode &= 0x1;
	if(lh > 7) {
		lh = 7;
	}
	
	if(lht> 7) {
		lht = 7;
	}
#else
	lh = ndlhdiv;
	lht = ndlhtdiv;
	ndlhmode &= 0x1;
#endif
	pCdspReg0->cdspNdnLHDiv = (lht << 4)|lh;
	pCdspReg0->cdspNdnLHCor = ndlhcoring;
	pCdspReg0->cdspNdnLHMode = ndlhmode;
}

/**
* @brief	cdsp new denoise edge scale get
* @param	ndlhdiv [out]: L edge enhancement edge vale scale
* @param	ndlhtdiv [out]: L edge enhancement edge vale scale
* @param	ndlhcoring [out]: L core ring threshold
* @param	ndlhmode [out]: 1: default matrix, 0: enable paroramming matrix
* @return	none
*/
void 
gpHalCdspGetNdEdgeLCoring(
	UINT8 *ndlhdiv, 
	UINT8 *ndlhtdiv, 
	UINT8 *ndlhcoring, 
	UINT8 *ndlhmode
)
{
	//*ndlhdiv = 1 << (pCdspReg0->cdspNdnLHDiv & 0x0F);
	//*ndlhtdiv = 1 << ((pCdspReg0->cdspNdnLHDiv << 4) & 0x0F);
	
	*ndlhdiv = pCdspReg0->cdspNdnLHDiv & 0x0F;
	*ndlhtdiv = (pCdspReg0->cdspNdnLHDiv >> 4) & 0x0F;
	
	*ndlhcoring = pCdspReg0->cdspNdnLHCor;
	*ndlhmode = pCdspReg0->cdspNdnLHMode;
}

/**
* @brief	cdsp new denoise edge amp set
* @param	ndampga [in]: 0:1, 1:2, 2:3, 3:4
* @return	none
*/
void 
gpHalCdspSetNdEdgeAmpga(
	UINT8 ndampga
)
{
	ndampga &= 0x03;
/*	switch(ndampga)
	{
	case 0:
	case 1:
		ndampga = 0;
		break;
	case 2:
		ndampga = 1;
		break;
	case 3:
		ndampga = 2;
		break;
	case 4:
		ndampga = 3;
		break;
	default:
		ndampga = 3;
		break;		
	}*/

	pCdspReg0->cdspNdnAmpga = ndampga;
}

/**
* @brief	cdsp new denoise edge amp get
* @param	ndampga [out]: 0:1, 1:2, 2:3, 3:4
* @return	none
*/
UINT8
gpHalCdspGetNdEdgeAmpga(
	void 
)
{
	UINT8 ndampga = pCdspReg0->cdspNdnAmpga;
	
	/*switch(ndampga)
	{
	case 0:
	case 1:
		ndampga = 0;
		break;
	case 2:
		ndampga = 1;
		break;
	case 3:
		ndampga = 2;
		break;
	case 4:
		ndampga = 3;
		break;
	default:
		ndampga = 3;
		break;		
	}*/

	return ndampga;
}

/**
* @brief	cdsp wb gain2 enable
* @param	wbgain2en [in]: enable 
* @return	none
*/
void 
gpHalCdspSetWbGain2En(
	UINT8 wbgain2en
)
{
	if(wbgain2en) {
		pCdspReg2->cdspAwbWinGain2En |= 1 << 0;
	} else {
		pCdspReg2->cdspAwbWinGain2En &= ~(1 << 0);
	}
}

UINT8 
gpHalCdspGetWbGain2En(
	void
)
{
	return (pCdspReg2->cdspAwbWinGain2En & 0x01);
}

/**
* @brief	cdsp wb gain2 set
* @param	rgain2 [in]: R gain
* @param	ggain2 [in]: G gain
* @param	bgain2 [in]: B gain
* @return	none
*/
void
gpHalCdspSetWbGain2(
	UINT16 rgain2,
	UINT16 ggain2,
	UINT16 bgain2
)
{
	pCdspReg2->cdspAwbWinRGain2[0] = (UINT8)(rgain2 & 0x00FF);
	pCdspReg2->cdspAwbWinRGain2[1] = (UINT8)((rgain2 & 0x0100) >> 8);
	pCdspReg2->cdspAwbWinGGain2[0] = (UINT8)(ggain2 & 0x00FF);
	pCdspReg2->cdspAwbWinGGain2[1] = (UINT8)((ggain2 & 0x0100) >> 8);	
	pCdspReg2->cdspAwbWinBGain2[0] = (UINT8)(bgain2 & 0x00FF);
	pCdspReg2->cdspAwbWinBGain2[1] = (UINT8)((bgain2 & 0x0100) >> 8);	
}

void
gpHalCdspGetWbGain2(
	UINT16 *rgain2,
	UINT16 *ggain2,
	UINT16 *bgain2
)
{
	*rgain2 = (pCdspReg2->cdspAwbWinRGain2[1] << 9) | pCdspReg2->cdspAwbWinRGain2[0];
	*ggain2 = (pCdspReg2->cdspAwbWinGGain2[1] << 9) | pCdspReg2->cdspAwbWinGGain2[0];
	*bgain2 = (pCdspReg2->cdspAwbWinBGain2[1] << 9) | pCdspReg2->cdspAwbWinBGain2[0];
}

/**
* @brief	cdsp auto focus enable
* @param	af_en [in]: af enable
* @param	af_win_hold [in]: af hold
* @return	none
*/
void 
gpHalCdspSetAFEn(
	UINT8 af_en, 
	UINT8 af_win_hold
)
{
	pCdspReg2->cdspAfWinHoldEn = ((af_en & 0x01) << 4) | (af_win_hold & 0x01); 
}

void 
gpHalCdspGetAFEn(
	UINT8 *af_en, 
	UINT8 *af_win_hold
)
{
	*af_en = (pCdspReg2->cdspAfWinHoldEn >> 4) & 0x01;
	*af_win_hold = pCdspReg2->cdspAfWinHoldEn & 0x01;
}

/**
* @brief	cdsp auto focus window 1 set
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size
* @param	vsize [in]: v size
* @return	none
*/
void 
gpHalCdspSetAfWin1(
	UINT16 hoffset, 
	UINT16 voffset, 
	UINT16 hsize, 
	UINT16 vsize
)
{
	pCdspReg2->cdspAfWin1HOff[0] = (UINT8)(hoffset & 0x00FF);
	pCdspReg2->cdspAfWin1HOff[1] = (UINT8)((hoffset & 0x0F00) >> 8);
	pCdspReg2->cdspAfWin1VOff[0] = (UINT8)(voffset & 0x00FF);
	pCdspReg2->cdspAfWin1VOff[1] = (UINT8)((voffset & 0x0F00) >> 8);	
	pCdspReg2->cdspAfWin1HSize[0] = (UINT8)(hsize & 0x00FF);
	pCdspReg2->cdspAfWin1HSize[1] = (UINT8)((hsize & 0x0F00) >> 8);
	pCdspReg2->cdspAfWin1VSize[0] = (UINT8)(vsize & 0x00FF);
	pCdspReg2->cdspAfWin1VSize[1] = (UINT8)((vsize & 0x0F00) >> 8);
}

void 
gpHalCdspGetAfWin1(
	UINT16 *hoffset, 
	UINT16 *voffset, 
	UINT16 *hsize, 
	UINT16 *vsize
)
{
	*hoffset = (pCdspReg2->cdspAfWin1HOff[1] << 8) | pCdspReg2->cdspAfWin1HOff[0];
	*voffset = (pCdspReg2->cdspAfWin1VOff[1] << 8) | pCdspReg2->cdspAfWin1VOff[0];
	*hsize = (pCdspReg2->cdspAfWin1HSize[1] << 8) | pCdspReg2->cdspAfWin1HSize[0];
	*vsize = (pCdspReg2->cdspAfWin1VSize[1] << 8) | pCdspReg2->cdspAfWin1VSize[0];
}

/**
* @brief	cdsp auto focus window 2 set
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size, 256, 512, 1024, 64, 2048
* @param	vsize [in]: v size, 256, 512, 1024, 64, 2048
* @return	none
*/
void 
gpHalCdspSetAfWin2(
	UINT16 hoffset, 
	UINT16 voffset, 
	UINT16 hsize, 
	UINT16 vsize
)
{
	UINT8 h_size, v_size;
	
	hoffset = (hoffset >> 2) & 0x3FF;	/* offset unit is 4 pixel */
	voffset = (voffset >> 2) & 0x3FF;

	if(hsize <= 64) h_size = 3;
	else if(hsize <= 256) h_size = 0;
	else if(hsize <= 512) h_size = 1;
	else if(hsize <= 1024) h_size = 2;
	else if(hsize <= 2048) h_size = 4;
	else h_size = 4;

	if(vsize <= 64) v_size = 3;
	else if(vsize <= 256) v_size = 0;
	else if(vsize <= 512) v_size = 1;
	else if(vsize <= 1024) v_size = 2;
	else if(vsize <= 2048) v_size = 4;
	else v_size = 4;

	pCdspReg2->cdspAfWin2HOff[0] = (UINT8)(hoffset & 0x00FF);
	pCdspReg2->cdspAfWin2HOff[1] = (UINT8)((hoffset & 0x0300) >> 8);
	pCdspReg2->cdspAfWin2VOff[0] = (UINT8)(voffset & 0x00FF);
	pCdspReg2->cdspAfWin2VOff[1] = (UINT8)((voffset & 0x0300) >> 8);
	pCdspReg2->cdspAfWin23SizeL &= ~0x0F;
	pCdspReg2->cdspAfWin23SizeL |= ((v_size & 0x03) << 2)|(h_size & 0x03);
	pCdspReg2->cdspAfWin23SizeH &= ~0x03;
	pCdspReg2->cdspAfWin23SizeH |= ((v_size & 0x04) >> 1)|((h_size & 0x04) >> 2);
}

void 
gpHalCdspGetAfWin2(
	UINT16 *hoffset, 
	UINT16 *voffset, 
	UINT16 *hsize, 
	UINT16 *vsize
)
{
	UINT32 temp;
	
	*hoffset = ((pCdspReg2->cdspAfWin2HOff[1] << 8) | pCdspReg2->cdspAfWin2HOff[0]) << 2;
	*voffset = ((pCdspReg2->cdspAfWin2VOff[1] << 8) | pCdspReg2->cdspAfWin2VOff[0]) << 2;

	temp = (pCdspReg2->cdspAfWin23SizeL & 0x03) | ((pCdspReg2->cdspAfWin23SizeH & 0x01) << 2);
	if(temp == 3) *hsize = 64;
	else if(temp == 0) *hsize = 256;
	else if(temp == 1) *hsize = 512;
	else if(temp == 2) *hsize = 1024;
	else if(temp == 4) *hsize = 2048;
	else *hsize = 2048;

	temp = ((pCdspReg2->cdspAfWin23SizeL & 0x0C) >> 2) | ((pCdspReg2->cdspAfWin23SizeH & 0x02) << 2);
	if(temp == 3) *vsize = 64;
	else if(temp == 0) *vsize = 256;
	else if(temp == 1) *vsize = 512;
	else if(temp == 2) *vsize = 1024;
	else if(temp == 4) *vsize = 2048;
	else *vsize = 2048;
}

/**
* @brief	cdsp auto focus window 3 set
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size, 256, 512, 1024, 64, 2048
* @param	vsize [in]: v size, 256, 512, 1024, 64, 2048
* @return	none
*/
void 
gpHalCdspSetAfWin3(
	UINT16 hoffset, 
	UINT16 voffset, 
	UINT16 hsize, 
	UINT16 vsize
)
{
	UINT8 h_size, v_size;
	
	hoffset = (hoffset >> 2) & 0x3FF;	/* offset unit is 4 pixel */
	voffset = (voffset >> 2) & 0x3FF;

	if(hsize <= 64) h_size = 3;
	else if(hsize <= 256) h_size = 0;
	else if(hsize <= 512) h_size = 1;
	else if(hsize <= 1024) h_size = 2;
	else if(hsize <= 2048) h_size = 4;
	else h_size = 4;

	if(vsize <= 64) v_size = 3;
	else if(vsize <= 256) v_size = 0;
	else if(vsize <= 512) v_size = 1;
	else if(vsize <= 1024) v_size = 2;
	else if(vsize <= 2048) v_size = 4;
	else v_size = 4;

	pCdspReg2->cdspAfWin3HOff[0] = (UINT8)(hoffset & 0x00FF);
	pCdspReg2->cdspAfWin3HOff[1] = (UINT8)((hoffset & 0x0300) >> 8);
	pCdspReg2->cdspAfWin3VOff[0] = (UINT8)(voffset & 0x00FF);
	pCdspReg2->cdspAfWin3VOff[1] = (UINT8)((voffset & 0x0300) >> 8);
	pCdspReg2->cdspAfWin23SizeL &= ~0xF0;
	pCdspReg2->cdspAfWin23SizeL |= ((v_size & 0x03) << 6)|((h_size & 0x03) << 4);
	pCdspReg2->cdspAfWin23SizeH &= ~0x0C;
	pCdspReg2->cdspAfWin23SizeH |= ((v_size & 0x04) << 1)|(h_size & 0x04);
}	

void 
gpHalCdspGetAfWin3(
	UINT16 *hoffset, 
	UINT16 *voffset, 
	UINT16 *hsize, 
	UINT16 *vsize
)
{
	UINT32 temp;

	*hoffset = ((pCdspReg2->cdspAfWin3HOff[1] << 8) | pCdspReg2->cdspAfWin3HOff[0]) << 2;
	*voffset = ((pCdspReg2->cdspAfWin3VOff[1] << 8) | pCdspReg2->cdspAfWin3VOff[0]) << 2;

	temp = ((pCdspReg2->cdspAfWin23SizeL & 0x30) >> 4) | (pCdspReg2->cdspAfWin23SizeH & 0x04);
	if(temp == 3) *hsize = 64;
	else if(temp == 0) *hsize = 256;
	else if(temp == 1) *hsize = 512;
	else if(temp == 2) *hsize = 1024;
	else if(temp == 4) *hsize = 2048;
	else *hsize = 2048;

	temp = ((pCdspReg2->cdspAfWin23SizeL & 0xC0) >> 6) | ((pCdspReg2->cdspAfWin23SizeH & 0x08) >> 1);
	if(temp == 3) *vsize = 64;
	else if(temp == 0) *vsize = 256;
	else if(temp == 1) *vsize = 512;
	else if(temp == 2) *vsize = 1024;
	else if(temp == 4) *vsize = 2048;
	else *vsize = 2048;
}

/**
* @brief	cdsp auto white balance enable
* @param	awb_en [in]: awb enable
* @param	awb_win_hold [in]: awb hold
* @return	none
*/
void 
gpHalCdspSetAWBEn(
	UINT8 awb_en, 
	UINT8 awb_win_hold
)
{
	pCdspReg2->cdspAeHisAwbWinHold &= ~0x04;
	pCdspReg2->cdspAeHisAwbWinHold |= (awb_win_hold & 0x01) << 2;
	pCdspReg2->cdspAeAwbWinEn &= ~0x10;
	pCdspReg2->cdspAeAwbWinEn |= (awb_en & 0x01) << 4; 
}

void 
gpHalCdspGetAWBEn(
	UINT8 *awb_en, 
	UINT8 *awb_win_hold
)
{
	*awb_en = (pCdspReg2->cdspAeAwbWinEn >> 4) & 0x01;
	*awb_win_hold = (pCdspReg2->cdspAeHisAwbWinHold >> 2) & 0x01;
}

/**
* @brief	cdsp auto white balance set
* @param	awbclamp_en [in]: awb special window clamp enable.
* @param	sindata [in]: sin data for AWB
* @param	cosdata [in]: cos data for AWB
* @param	awbwinthr [in]: AWB winwow accumulation threshold
* @return	none
*/
void 
gpHalCdspSetAWB(
	UINT8 awbclamp_en, 
	SINT8 sindata, 
	SINT8 cosdata, 
	UINT8 awbwinthr
)
{
	pCdspReg2->cdspAwbSinData = sindata;
	pCdspReg2->cdspAwbCosData = cosdata;
	pCdspReg2->cdspAwbWinThr = awbwinthr;
	pCdspReg2->cdspAwbClampEn = awbclamp_en & 0x01;
}

void 
gpHalCdspGetAWB(
	UINT8 *awbclamp_en, 
	SINT8 *sindata, 
	SINT8 *cosdata, 
	UINT8 *awbwinthr
)
{
	*awbclamp_en = pCdspReg2->cdspAwbClampEn & 0x01;
	*sindata = pCdspReg2->cdspAwbSinData;
	*cosdata = pCdspReg2->cdspAwbCosData;
	*awbwinthr = pCdspReg2->cdspAwbWinThr;
}

/**
* @brief	cdsp awb special windows Y threshold set
* @param	Ythr0 [in]: AWB Y threshold0
* @param	Ythr1 [in]: AWB Y threshold1
* @param	Ythr2 [in]: AWB Y threshold2
* @param	Ythr3 [in]: AWB Y threshold3
* @return	none
*/
void 
gpHalCdspSetAwbYThr(
	UINT8 Ythr0,
	UINT8 Ythr1,
	UINT8 Ythr2,
	UINT8 Ythr3
)
{
	pCdspReg2->cdspAwbSpWinYThr0 = Ythr0;
	pCdspReg2->cdspAwbSpWinYThr1 = Ythr1;
	pCdspReg2->cdspAwbSpWinYThr2 = Ythr2;
	pCdspReg2->cdspAwbSpWinYThr3 = Ythr3;
}

void 
gpHalCdspGetAwbYThr(
	UINT8 *Ythr0,
	UINT8 *Ythr1,
	UINT8 *Ythr2,
	UINT8 *Ythr3
)
{
	*Ythr0 = pCdspReg2->cdspAwbSpWinYThr0;
	*Ythr1 = pCdspReg2->cdspAwbSpWinYThr1;
	*Ythr2 = pCdspReg2->cdspAwbSpWinYThr2;
	*Ythr3 = pCdspReg2->cdspAwbSpWinYThr3;
}

/**
* @brief	cdsp awb special windows uv threshold set
* @param	UVthr [in]: AWB UV threshold
* @return	none
*/
void 
gpHalCdspSetAwbUVThr(
	awb_uv_thr_t *UVthr
)
{
	printk("Set awb u, v setting:\n");
	printk("UL1N1 = %d, UL1P1 = %d, VL1N1 = %d, VL1P1 = %d\r\n", UVthr->UL1N1, UVthr->UL1P1, UVthr->VL1N1, UVthr->VL1P1);
	printk("UL1N2 = %d, UL1P2 = %d, VL1N2 = %d, VL1P2 = %d\r\n", UVthr->UL1N2, UVthr->UL1P2, UVthr->VL1N2, UVthr->VL1P2);
	printk("UL1N3 = %d, UL1P3 = %d, VL1N3 = %d, VL1P3 = %d\r\n", UVthr->UL1N3, UVthr->UL1P3, UVthr->VL1N3, UVthr->VL1P3);
	
	pCdspReg2->cdspAwbSpWinULowThr1 = (SINT8)UVthr->UL1N1;
	pCdspReg2->cdspAwbSpWinUHiThr1 = (SINT8)UVthr->UL1P1;
	pCdspReg2->cdspAwbSpWinVLowThr1 = (SINT8)UVthr->VL1N1;
	pCdspReg2->cdspAwbSpWinVHiThr1 = (SINT8)UVthr->VL1P1;
	
	pCdspReg2->cdspAwbSpWinULowThr2 = (SINT8)UVthr->UL1N2;
	pCdspReg2->cdspAwbSpWinUHiThr2 = (SINT8)UVthr->UL1P2;
	pCdspReg2->cdspAwbSpWinVLowThr2 = (SINT8)UVthr->VL1N2;
	pCdspReg2->cdspAwbSpWinVHiThr2 = (SINT8)UVthr->VL1P2;

	pCdspReg2->cdspAwbSpWinULowThr3 = (SINT8)UVthr->UL1N3;
	pCdspReg2->cdspAwbSpWinUHiThr3 = (SINT8)UVthr->UL1P3;
	pCdspReg2->cdspAwbSpWinVLowThr3 = (SINT8)UVthr->VL1N3;
	pCdspReg2->cdspAwbSpWinVHiThr3 = (SINT8)UVthr->VL1P3;
}

/**
* @brief	cdsp awb special windows uv threshold get
* @param	UVthr [out]: AWB UV threshold
* @return	none
*/
void 
gpHalCdspGetAwbUVThr(
	awb_uv_thr_t *UVthr
)
{
	UVthr->UL1N1 = pCdspReg2->cdspAwbSpWinULowThr1;
	UVthr->UL1P1 = pCdspReg2->cdspAwbSpWinUHiThr1;
	UVthr->VL1N1 = pCdspReg2->cdspAwbSpWinVLowThr1;
	UVthr->VL1P1 = pCdspReg2->cdspAwbSpWinVHiThr1;

	UVthr->UL1N2 = pCdspReg2->cdspAwbSpWinULowThr2;
	UVthr->UL1P2 = pCdspReg2->cdspAwbSpWinUHiThr2;
	UVthr->VL1N2 = pCdspReg2->cdspAwbSpWinVLowThr2;
	UVthr->VL1P2 = pCdspReg2->cdspAwbSpWinVHiThr2;

	UVthr->UL1N3 = pCdspReg2->cdspAwbSpWinULowThr3;
	UVthr->UL1P3 = pCdspReg2->cdspAwbSpWinUHiThr3;
	UVthr->VL1N3 = pCdspReg2->cdspAwbSpWinVLowThr3;
	UVthr->VL1P3 = pCdspReg2->cdspAwbSpWinVHiThr3;
	
	printk("Get awb u, v setting:\n");
	printk("UL1N1 = %d, UL1P1 = %d, VL1N1 = %d, VL1P1 = %d\r\n", UVthr->UL1N1, UVthr->UL1P1, UVthr->VL1N1, UVthr->VL1P1);
	printk("UL1N2 = %d, UL1P2 = %d, VL1N2 = %d, VL1P2 = %d\r\n", UVthr->UL1N2, UVthr->UL1P2, UVthr->VL1N2, UVthr->VL1P2);
	printk("UL1N3 = %d, UL1P3 = %d, VL1N3 = %d, VL1P3 = %d\r\n", UVthr->UL1N3, UVthr->UL1P3, UVthr->VL1N3, UVthr->VL1P3);
}

/**
* @brief	cdsp ae/awb source set
* @param	raw_en [in]: ae/awb windows set, 0:from poswb, 1:form awb line ctrl
* @return	none
*/
void 
gpHalCdspSetAeAwbSrc(
	UINT8 raw_en 
)
{
	if(raw_en) {
		printk("%s: select line ctrl\r\n", __FUNCTION__);
		pCdspReg2->cdspRawAwbWinCtrl |= 1 << 0;
	} else {
		printk("%s: select poswb\r\n", __FUNCTION__);
		pCdspReg2->cdspRawAwbWinCtrl &= ~(1 << 0);
	}
}

UINT32 
gpHalCdspGetAeAwbSrc(
	void 
)
{
	return (pCdspReg2->cdspRawAwbWinCtrl & 0x01);
}

/**
* @brief	cdsp ae/awb subsample set
* @param	subample [in]: 0:disable, 2:1/2, 4:1/4 subsample
* @return	none
*/
void 
gpHalCdspSetAeAwbSubSample(
	UINT8 subsample
)
{
/*	printk("1: subsample = %d\r\n", subsample);
	if(subsample == 2)
		subsample = 1;
	else if(subsample == 4)
		subsample = 2;
	else if(subsample == 8)
		subsample = 3;
	else
		subsample = 0;
*/
	printk("%s: subsample = %d\r\n", __FUNCTION__, subsample);
	
	pCdspReg2->cdspRawAwbWinCtrl &= ~(3 << 1);
	pCdspReg2->cdspRawAwbWinCtrl |= (subsample<< 1);  
}

UINT8 
gpHalCdspGetAeAwbSubSample(
	void
)
{
	return (1 << (pCdspReg2->cdspRawAwbWinCtrl & 0x03));
}

/**
* @brief	cdsp auto expore enable
* @param	ae_en [in]: ae enable
* @param	ae_win_hold [in]: ae hold
* @return	none
*/
void 
gpHalCdspSetAEEn(
	UINT8 ae_en, 
	UINT8 ae_win_hold
)
{
	pCdspReg2->cdspAeHisAwbWinHold &= ~0x01;
	pCdspReg2->cdspAeHisAwbWinHold |= ae_win_hold & 0x01;
	pCdspReg2->cdspAeAwbWinEn &= ~0x01;
	pCdspReg2->cdspAeAwbWinEn |= ae_en & 0x01; 
}

void 
gpHalCdspGetAEEn(
	UINT8 *ae_en, 
	UINT8 *ae_win_hold
)
{
	*ae_en = pCdspReg2->cdspAeAwbWinEn & 0x01;
	*ae_win_hold = pCdspReg2->cdspAeHisAwbWinHold & 0x01;
}

/**
* @brief	cdsp auto expore set
* @param	phaccfactor [in]: pseudo h window size for ae windows
* @param	pvaccfactor [in]: pseudo v window size for ae windows
* @return	none
*/
void 
gpHalCdspSetAEWin(
	UINT8 phaccfactor, 
	UINT8 pvaccfactor
)
{
	UINT8 h_factor, v_factor;
	
	if(phaccfactor <= 4) h_factor = 0;
	else if(phaccfactor <= 8) h_factor = 1;
	else if(phaccfactor <= 16) h_factor = 2;
	else if(phaccfactor <= 32) h_factor = 3;
	else if(phaccfactor <= 64) h_factor = 4;
	else if(phaccfactor <= 128) h_factor = 5;
	else h_factor = 5;

	if(pvaccfactor <= 4) v_factor = 0;
	else if(pvaccfactor <= 8) v_factor = 1;
	else if(pvaccfactor <= 16) v_factor = 2;
	else if(pvaccfactor <= 32) v_factor = 3;
	else if(pvaccfactor <= 64) v_factor = 4;
	else if(pvaccfactor <= 128) v_factor = 5;
	else v_factor = 5;
	
	printk("AE Win factor: h = %d, v = %d\r\n", h_factor, v_factor);
		
	pCdspReg2->cdspAeAccFator = (v_factor << 4)|h_factor;
}

void 
gpHalCdspGetAEWin(
	UINT8 *phaccfactor, 
	UINT8 *pvaccfactor
)
{	
	*phaccfactor = 1 << (pCdspReg2->cdspAeAccFator & 0x0F);
	*pvaccfactor = 1 << ((pCdspReg2->cdspAeAccFator >> 4) & 0x0F);
}

/**
* @brief	cdsp ae buffer address set
* @param	winaddra [in]: AE a buffer address set 
* @param	winaddrb [in]: AE b buffer address set
* @return	none
*/
void 
gpHalCdspSetAEBuffAddr(
	UINT32 winaddra, 
	UINT32 winaddrb
)
{
	winaddra >>= 1;
	winaddrb >>= 1;
	pCdspReg2->cdspAeWinBufAddrA[0] = (UINT8)(winaddra & 0x000000FF);
	pCdspReg2->cdspAeWinBufAddrA[1] = (UINT8)((winaddra & 0x0000FF00) >> 8);
	pCdspReg2->cdspAeWinBufAddrA[2] = (UINT8)((winaddra & 0x00FF0000) >> 16);
	pCdspReg2->cdspAeWinBufAddrA[3] = (UINT8)((winaddra & 0xFF000000) >> 24);
	pCdspReg2->cdspAeWinBufAddrB[0] = (UINT8)(winaddrb & 0x000000FF);
	pCdspReg2->cdspAeWinBufAddrB[1] = (UINT8)((winaddrb & 0x0000FF00) >> 8);
	pCdspReg2->cdspAeWinBufAddrB[2] = (UINT8)((winaddrb & 0x00FF0000) >> 16);
	pCdspReg2->cdspAeWinBufAddrB[3] = (UINT8)((winaddrb & 0xFF000000) >> 24);
}

/**
* @brief	cdsp ae buffer address sett 
* @return	0: ae a buffer ready, 1: ae b buffer ready
*/
UINT32 
gpHalCdspGetAEActBuff(
	void
)
{
	if(pCdspReg2->cdspAeActiveWin & 0x01)
		return 1;	/* buffer b active */
	else
		return 0;	/* buffer a active */
}

/**
* @brief	cdsp rgb window set
* @param	hwdoffset [in]: rgb window h offset
* @param	vwdoffset [in]: rgb window v offset
* @param	hwdsize [in]: rgb window h size
* @param	vwdsize [in]: rgb window v size
* @return	none
*/
void 
gpHalCdspSetRGBWin(
	UINT16 hwdoffset, 
	UINT16 vwdoffset, 
	UINT16 hwdsize, 
	UINT16 vwdsize
)
{
	printk("RAW win: hoffset = %d, voffset = %d, w = %d, h = %d\r\n",  hwdoffset, vwdoffset, hwdsize, vwdsize);
		
	pCdspReg2->cdspRgbWinHOff[0] = (UINT8)(hwdoffset & 0x00FF);
	pCdspReg2->cdspRgbWinHOff[1] = (UINT8)((hwdoffset & 0x1F00) >> 8);	
	pCdspReg2->cdspRgbWinHSize[0] = (UINT8)(hwdsize & 0x00FF);
	pCdspReg2->cdspRgbWinHSize[1] = (UINT8)((hwdsize & 0x0300) >> 8);	
	pCdspReg2->cdspRgbWinVOff[0] = (UINT8)(vwdoffset & 0x00FF);
	pCdspReg2->cdspRgbWinVOff[1] = (UINT8)((vwdoffset & 0x1F00) >> 8);	
	pCdspReg2->cdspRgbWinVSize[0] = (UINT8)(vwdsize & 0x00FF);
	pCdspReg2->cdspRgbWinVSize[1] = (UINT8)((vwdsize & 0x0300) >> 8);
}

void 
gpHalCdspGetRGBWin(
	UINT16 *hwdoffset, 
	UINT16 *vwdoffset, 
	UINT16 *hwdsize, 
	UINT16 *vwdsize
)
{
	*hwdoffset = (pCdspReg2->cdspRgbWinHOff[1] << 8) | pCdspReg2->cdspRgbWinHOff[0];
	*vwdoffset = (pCdspReg2->cdspRgbWinVOff[1] << 8) | pCdspReg2->cdspRgbWinVOff[0];
	*hwdsize = (pCdspReg2->cdspRgbWinHSize[1] << 8) | pCdspReg2->cdspRgbWinHSize[0];
	*vwdsize = (pCdspReg2->cdspRgbWinVSize[1] << 8) | pCdspReg2->cdspRgbWinVSize[0];
}

/**
* @brief	cdsp ae/af test windows enable
* @param	AeWinTest [in]: ae test window enable
* @param	AfWinTest [in]: af test window enable
* @return	none
*/
void 
gpHalCdspSet3ATestWinEn(
	UINT8 AeWinTest, 
	UINT8 AfWinTest
)
{
	pCdspReg2->cdspAeAfWinTest = ((AfWinTest & 0x01) << 4) | (AeWinTest & 0x01);
}

void 
gpHalCdspGet3ATestWinEn(
	UINT8 *AeWinTest, 
	UINT8 *AfWinTest
)
{
	*AeWinTest = (pCdspReg2->cdspAeAfWinTest >> 4) & 0x01;
	*AfWinTest = pCdspReg2->cdspAeAfWinTest & 0x01;
}

/**
* @brief	cdsp histgm enable
* @param	his_en [in]: histgm enable
* @param	his_hold_en [in]: histgm hold
* @return	none
*/
void 
gpHalCdspSetHistgmEn(
	UINT8 his_en, 
	UINT8 his_hold_en
)
{
	pCdspReg2->cdspAeHisAwbWinHold &= ~0x02;
	pCdspReg2->cdspAeHisAwbWinHold |= (his_hold_en & 0x01) << 1;
	pCdspReg2->cdspHisEn = his_en & 0x01;
}

void 
gpHalCdspGetHistgmEn(
	UINT8 *his_en, 
	UINT8 *his_hold_en
)
{
	*his_en = pCdspReg2->cdspHisEn & 0x01;
	*his_hold_en = (pCdspReg2->cdspAeHisAwbWinHold >> 1) & 0x01;	
}

/**
* @brief	cdsp histgm statistics set
* @param	hislowthr [in]: histgm low threshold set
* @param	hishighthr [in]: histgm high threshold set
* @return	none
*/
void 
gpHalCdspSetHistgm(
	UINT8 hislowthr, 
	UINT8 hishighthr
)
{
	pCdspReg2->cdspHisLowThr = hislowthr;
	pCdspReg2->cdspHisHiThr = hishighthr;
}

void 
gpHalCdspGetHistgm(
	UINT8 *hislowthr, 
	UINT8 *hishighthr
)
{
	*hislowthr = pCdspReg2->cdspHisLowThr;
	*hishighthr = pCdspReg2->cdspHisHiThr;
}

void 
gpHalCdspGetHistgmCount(
	UINT32 *hislowcnt, 
	UINT32 *hishighcnt
)
{
	*hislowcnt = ((UINT32)(pCdspReg2->cdspHisLowCnt[2]&0x3F)<<16)| ((UINT32)pCdspReg2->cdspHisLowCnt[1]<<8)|(UINT32)pCdspReg2->cdspHisLowCnt[0];
	*hishighcnt = ((UINT32)(pCdspReg2->cdspHisHiCnt[2]&0x3F)<<16)| ((UINT32)pCdspReg2->cdspHisHiCnt[1]<<8)|(UINT32)pCdspReg2->cdspHisHiCnt[0];
}

/**
* @brief	cdsp get awb cnt
* @param	section [in]: index = 1, 2, 3
* @param	sumcnt [out]: count get
* @return	SUCCESS/ERROR
*/
SINT32
gpHalCdspGetAwbSumCnt(
	UINT8 section,
	UINT32 *sumcnt 
)
{
	volatile UINT32 *ptr;
	
	if(section == 1) {
		ptr = (UINT32 *)pCdspReg2->cdspAwbSumCnt1;
		*sumcnt = *ptr;
	} else if(section == 2) {
		ptr = (UINT32 *)pCdspReg2->cdspAwbSumCnt2;
		*sumcnt = *ptr;
	} else if(section == 3) {
		ptr = (UINT32 *)pCdspReg2->cdspAwbSumCnt3;
		*sumcnt = *ptr;
	} else {
		return -1;
	}
	return 0;
}

/**
* @brief	cdsp get awb g
* @param	section [in]: index = 1, 2, 3
* @param	sumgl [out]: sum g1 low 
* @param	sumgl [out]: sum g1 high 
* @return	SUCCESS/ERROR
*/
SINT32
gpHalCdspGetAwbSumG(
	UINT8 section,
	UINT32 *sumgl,
	UINT32 *sumgh
)
{
	if(section == 1) {
		*sumgl = ((UINT32)pCdspReg2->cdspAwbSumG1L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumG1L[0];
		*sumgh = ((UINT32)(pCdspReg2->cdspAwbSumG1H&0x03)<<16)|((UINT32)pCdspReg2->cdspAwbSumG1L[3]<<8)|((UINT32)pCdspReg2->cdspAwbSumG1L[2]);
	} else if(section == 2) {
		*sumgl = ((UINT32)pCdspReg2->cdspAwbSumG2L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumG2L[0];
		*sumgh = ((UINT32)(pCdspReg2->cdspAwbSumG2H&0x03)<<16)|((UINT32)pCdspReg2->cdspAwbSumG2L[3]<<8)|((UINT32)pCdspReg2->cdspAwbSumG2L[2]);
	} else if(section == 3) {
		*sumgl = ((UINT32)pCdspReg2->cdspAwbSumG3L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumG3L[0];
		*sumgh = ((UINT32)(pCdspReg2->cdspAwbSumG3H&0x03)<<16)|((UINT32)pCdspReg2->cdspAwbSumG3L[3]<<8)|((UINT32)pCdspReg2->cdspAwbSumG3L[2]);
	} else {
		return -1;
	}
	return 0;
}

/**
* @brief	cdsp get awb rg
* @param	section [in]: section = 1, 2, 3
* @param	sumrgl [out]: sum rg low 
* @param	sumrgl [out]: sum rg high 
* @return	SUCCESS/ERROR
*/
SINT32
gpHalCdspGetAwbSumRG(
	UINT8 section,
	UINT32 *sumrgl,
	SINT32 *sumrgh
)
{
	volatile UINT32 *ptr;
	
	if(section == 1) {		
		ptr = (UINT32 *)&pCdspReg2->cdspAwbSumRg1L[0];
		*sumrgh = pCdspReg2->cdspAwbSumRg1H;
	} else if(section == 2) {
		ptr = (UINT32 *)&pCdspReg2->cdspAwbSumRg2L[0];
		*sumrgh = pCdspReg2->cdspAwbSumRg2H;
	} else if(section == 3) {
		ptr = (UINT32 *)&pCdspReg2->cdspAwbSumRg3L[0];
		*sumrgh = pCdspReg2->cdspAwbSumRg3H;
	} else {
		return -1;
	}

	*sumrgl = *ptr;
	
	return 0;
}

/**
* @brief	cdsp get awb bg
* @param	section [in]: section = 1, 2, 3
* @param	sumbgl [out]: sum bg low 
* @param	sumbgl [out]: sum bg high 
* @return	SUCCESS/ERROR
*/
SINT32
gpHalCdspGetAwbSumBG(
	UINT8 section,
	UINT32 *sumbgl,
	SINT32 *sumbgh
)
{
	volatile UINT32 *ptr;
	
	if(section == 1) {
		ptr = (UINT32 *)&pCdspReg2->cdspAwbSumBg1L[0];
		*sumbgh = pCdspReg2->cdspAwbSumBg1H;
	} else if(section == 2) {
		ptr = (UINT32 *)&pCdspReg2->cdspAwbSumBg2L[0];
		*sumbgh = pCdspReg2->cdspAwbSumBg2H;
	} else if(section == 3) {
		ptr = (UINT32 *)&pCdspReg2->cdspAwbSumBg3L[0];
		*sumbgh = pCdspReg2->cdspAwbSumBg3H;
	} else {
		return -1;
	}

	*sumbgl = *ptr;
	
	return 0;
}

/**
* @brief	cdsp get af window1 statistics
* @param	windows_no[in]: window index number
* @param	af_value[out]: 
* @return	SUCCESS/ERROR
*/
SINT32
gpHalCdspGetAFWinVlaue(
	UINT8 windows_no,
	af_windows_value_t *af_value
)
{
	if(windows_no == 1) {
		af_value->h_value_l = ((UINT32)pCdspReg2->cdspAfWin1HVal[1]<<8)|(UINT32)pCdspReg2->cdspAfWin1HVal[0];
		af_value->h_value_h = ((UINT32)(pCdspReg2->cdspAfWin1HVal[4]&0x07)<<16)|((UINT32)pCdspReg2->cdspAfWin1HVal[3]<<8)|((UINT32)pCdspReg2->cdspAfWin1HVal[2]);
		af_value->v_value_l = ((UINT32)pCdspReg2->cdspAfWin1VVal[1]<<8)|(UINT32)pCdspReg2->cdspAfWin1VVal[0];
		af_value->v_value_h = ((UINT32)(pCdspReg2->cdspAfWin1VVal[4]&0x07)<<16)|((UINT32)pCdspReg2->cdspAfWin1VVal[3]<<8)|((UINT32)pCdspReg2->cdspAfWin1VVal[2]);
	} else if(windows_no == 2) {
		af_value->h_value_l = ((UINT32)pCdspReg2->cdspAfWin2HValL[1]<<8)|(UINT32)pCdspReg2->cdspAfWin2HValL[0];
		af_value->h_value_h = ((UINT32)(pCdspReg2->cdspAfWin2HValH&0x07)<<16)|((UINT32)pCdspReg2->cdspAfWin2HValL[3]<<8)|((UINT32)pCdspReg2->cdspAfWin2HValL[2]);
		af_value->v_value_l = ((UINT32)pCdspReg2->cdspAfWin2VValL[1]<<8)|(UINT32)pCdspReg2->cdspAfWin2VValL[0];
		af_value->v_value_h = ((UINT32)(pCdspReg2->cdspAfWin2VValH&0x07)<<16)|((UINT32)pCdspReg2->cdspAfWin2VValL[3]<<8)|((UINT32)pCdspReg2->cdspAfWin2VValL[2]);		
	} else if(windows_no == 3) {
		af_value->h_value_l = ((UINT32)pCdspReg2->cdspAfWin3HValL[1]<<8)|(UINT32)pCdspReg2->cdspAfWin3HValL[0];
		af_value->h_value_h = ((UINT32)(pCdspReg2->cdspAfWin3HValH&0x07)<<16)|((UINT32)pCdspReg2->cdspAfWin3HValL[3]<<8)|((UINT32)pCdspReg2->cdspAfWin3HValL[2]);
		af_value->v_value_l = ((UINT32)pCdspReg2->cdspAfWin3VValL[1]<<8)|(UINT32)pCdspReg2->cdspAfWin3VValL[0];
		af_value->v_value_h = ((UINT32)(pCdspReg2->cdspAfWin3VValH&0x07)<<16)|((UINT32)pCdspReg2->cdspAfWin3VValL[3]<<8)|((UINT32)pCdspReg2->cdspAfWin3VValL[2]);
	} else {
		return -1;
	}
	return 0;
}

/**
* @brief	cdsp raw data path set
* @param	raw_mode [in]: raw data path set, 0:disable, 1:RGB_Path1, 3:RGB_Path2, 5:RGB_Path3 
* @param	cap_mode [in]: set 0:raw10, 1:raw8 
* @param	yuv_mode [in]: set 0:8y4u4v, 1:yuyv
* @param	yuv_fmt  [in]: set out sequence, 0:YUYV, 1:UYVY, 2:YVYU, 3:VYUY
* @return	none
*/
void 
gpHalCdspSetRawPath(
	UINT8 raw_mode, 
	UINT8 cap_mode, 
	UINT8 yuv_mode,
	UINT8 yuv_fmt
)
{
	raw_mode &= 0x07;
	cap_mode &= 0x01;
	yuv_mode &= 0x01;
	yuv_fmt &= 0x03;
	pCdspReg1->cdspRawCapYuvMode &= ~(0x3 << 6 | 1 << 5 | 1 << 4 | 0x7);
	pCdspReg1->cdspRawCapYuvMode |= (yuv_fmt << 6)|(yuv_mode << 5)|(cap_mode << 4)|raw_mode;
	printk("set raw path [0x%x] = 0x%x\r\n", (UINT32)&pCdspReg1->cdspRawCapYuvMode, pCdspReg1->cdspRawCapYuvMode);
}

/**
* @brief	cdsp sram fifo threshold
* @param	overflowen [in]: overflow enable
* @param	sramthd [in]: sram threshold
* @return	none
*/
void
gpHalCdspSetSRAM(
	UINT8 overflowen,
	UINT16 sramthd
)
{
	if(sramthd > 0x1FF) {
		sramthd = 0x100;
	}

	overflowen &= 0x1;
	sramthd &= ~0x01; 	//align 2
	pCdspReg1->cdspSramThr[0] = (UINT8)(sramthd & 0x00FF);
	pCdspReg1->cdspSramThr[1] = (overflowen << 4)|(UINT8)((sramthd & 0x0100) >> 8);
}

/**
* @brief	cdsp dma clamp size set
* @param	clamphsizeen [in]: clamp enable
* @param	Clamphsize [in]: clamp size set
* @return	none
*/
void 
gpHalCdspSetClampEn(
	UINT8 clamphsizeen,
	UINT16 Clamphsize
)
{
	pCdspReg1->cdspClampHSize[0] = (UINT8)(Clamphsize & 0x00FF);
	pCdspReg1->cdspClampHSize[1] = (UINT8)((Clamphsize & 0x0F00) >> 8);
	pCdspReg1->cdspClampHSizeEn = clamphsizeen & 0x01;
}

/**
* @brief	cdsp line interval set
* @param	line_interval [in]: line number
* @return	none
*/
void
gpHalCdspSetLineInterval(
	UINT16 line_interval
)
{
	pCdspReg1->cdspLineInterval[0] = (UINT8)(line_interval & 0x00FF);
	pCdspReg1->cdspLineInterval[1] = (UINT8)((line_interval & 0x0100) >> 8);
}

/**
* @brief	cdsp dma yuv buffer a set
* @param	width [in]: dma buffer width
* @param	height [in]: dma buffer height
* @param	buffe_addr [in]: dma buffer address
* @return	none
*/
void 
gpHalCdspSetYuvBuffA(
	UINT16 width,
	UINT16 height,
	UINT32 buffer_addr
)
{
	UINT8 temp[4];

	if(buffer_addr) {
		temp[3] = (UINT8)((buffer_addr & 0xFF000000) >> 24);
		temp[2] = (UINT8)((buffer_addr & 0x00FF0000) >> 16);
		temp[1] = (UINT8)((buffer_addr & 0x0000FF00) >> 8);
		temp[0] = (UINT8)(buffer_addr & 0x000000FF);
		pCdspReg7->cdspYuvAfbSAddr[3] = temp[3];
		pCdspReg7->cdspYuvAfbSAddr[2] = temp[2];
		pCdspReg7->cdspYuvAfbSAddr[1] = temp[1];
		pCdspReg7->cdspYuvAfbSAddr[0] = temp[0];
	}
	
	if(width && height) {
		temp[1] = (UINT8)((width & 0x0F00) >> 8);
		temp[0] = (UINT8)(width & 0x00FF);
		temp[3] = (UINT8)((height & 0x0F00) >> 8);
		temp[2] = (UINT8)(height & 0x00FF);
		pCdspReg7->cdspYuvAfbHSize[1] = temp[1];
		pCdspReg7->cdspYuvAfbHSize[0] = temp[0];
		pCdspReg7->cdspYuvAfbVSize[1] = temp[3];
		pCdspReg7->cdspYuvAfbVSize[0] = temp[2];
	}
}

/**
* @brief	cdsp get dma yuv buffer size
* @param	width [out]: dma buffer width
* @param	height [out]: dma buffer height
* @return	none
*/
void 
gpHalCdspGetYuvBuffASize(
	UINT16 *width,
	UINT16 *height
)
{
	*width = (pCdspReg7->cdspYuvAfbHSize[1] << 8) | pCdspReg7->cdspYuvAfbHSize[0];
	*height = (pCdspReg7->cdspYuvAfbVSize[1] << 8) | pCdspReg7->cdspYuvAfbVSize[0];
}

/**
* @brief	cdsp dma yuv buffer b set
* @param	width [in]: dma buffer width
* @param	height [in]: dma buffer height
* @param	buffe_addr [in]: dma buffer address
* @return	none
*/
void 
gpHalCdspSetYuvBuffB(
	UINT16 width,
	UINT16 height,
	UINT32 buffer_addr
)
{
	UINT8 temp[4];

	if(buffer_addr) {
		temp[3] = (UINT8)((buffer_addr & 0xFF000000) >> 24);
		temp[2] = (UINT8)((buffer_addr & 0x00FF0000) >> 16);
		temp[1] = (UINT8)((buffer_addr & 0x0000FF00) >> 8);
		temp[0] = (UINT8)(buffer_addr & 0x000000FF);
		pCdspReg7->cdspYuvBfbSAddr[3] = temp[3];
		pCdspReg7->cdspYuvBfbSAddr[2] = temp[2];
		pCdspReg7->cdspYuvBfbSAddr[1] = temp[1];
		pCdspReg7->cdspYuvBfbSAddr[0] = temp[0];
	}
	
	if(width && height) {
		temp[1] = (UINT8)((width & 0x0F00) >> 8);
		temp[0] = (UINT8)(width & 0x00FF);
		temp[3] = (UINT8)((height & 0x0F00) >> 8);
		temp[2] = (UINT8)(height & 0x00FF);
		pCdspReg7->cdspYuvBfbHSize[1] = temp[1];	
		pCdspReg7->cdspYuvBfbHSize[0] = temp[0];
		pCdspReg7->cdspYuvBfbVSize[1] = temp[3];
		pCdspReg7->cdspYuvBfbVSize[0] = temp[2];
	}
}

/**
* @brief	cdsp get dma yuv buffer size
* @param	width [out]: dma buffer width
* @param	height [out]: dma buffer height
* @return	none
*/
void 
gpHalCdspGetYuvBuffBSize(
	UINT16 *width,
	UINT16 *height
)
{
	*width = (pCdspReg7->cdspYuvBfbHSize[1] << 8) | pCdspReg7->cdspYuvBfbHSize[0];
	*height = (pCdspReg7->cdspYuvBfbVSize[1] << 8) | pCdspReg7->cdspYuvBfbVSize[0];
}

/**
* @brief	cdsp dma raw buffer size set
* @param	width [in]: dma buffer width
* @param	height [in]: dma buffer height
* @param	hoffset [in]: dma buffer h offset
* @param	raw_bit [in]: raw data bits
* @return	none
*/
void 
gpHalCdspSetRawBuffSize(
	UINT16 width,
	UINT16 height,
	UINT32 hoffset,
	UINT32 raw_bit
)
{
	
	//width = (width >> 1) & 0xFFF;
	width = (width * raw_bit / 16) & 0xfff;
	height &= 0xFFF; 
	//hoffset = (hoffset >> 1) & 0xFFF; 
	//hoffset = (hoffset  * raw_bit / 16) & 0xFFF; 
	//printk("%s: rawbit = %d, width = %d, hoffset = %d\r\n", __FUNCTION__, raw_bit, width, hoffset);
	
	pCdspReg7->cdspRawfbHSize[0] = (UINT8)(width & 0x00FF);
	pCdspReg7->cdspRawfbHSize[1] = (UINT8)((width & 0x0F00) >> 8);
	pCdspReg7->cdspRawfbVSize[0] = (UINT8)(height & 0x00FF);
	pCdspReg7->cdspRawfbVSize[1] = (UINT8)((height & 0x0F00) >> 8);
	pCdspReg7->cdspRawfbHOff[0] = (UINT8)(hoffset & 0x00FF);
	pCdspReg7->cdspRawfbHOff[1] = (UINT8)((hoffset & 0x0F00) >> 8);
}

/**
* @brief	cdsp dma raw buffer set 
* @param	buffe_addr [in]: dma buffer address
* @return	none
*/
void 
gpHalCdspSetRawBuff(
	UINT32 buffer_addr
)
{
	pCdspReg7->cdspRawfbSAddr[0] = (UINT8)(buffer_addr & 0x000000FF);
	pCdspReg7->cdspRawfbSAddr[1] = (UINT8)((buffer_addr & 0x0000FF00) >> 8);
	pCdspReg7->cdspRawfbSAddr[2] = (UINT8)((buffer_addr & 0x00FF0000) >> 16);
	pCdspReg7->cdspRawfbSAddr[3] = (UINT8)((buffer_addr & 0xFF000000) >> 24);
}

/**
* @brief	cdsp dma yuv buffer mode set
* @param	buffer_mode [in]: dma buffer mode
* @return	none
*/
void 
gpHalCdspSetDmaBuff(
	UINT8 buffer_mode
)
{
	UINT32 reg = pCdspReg7->cdspYuvABfbWrIdx;
	
	if(buffer_mode == RD_A_WR_A) {
		reg = 0x00;
	} else if(buffer_mode == RD_A_WR_B) {
		reg = 0x01;
	} else if(buffer_mode == RD_B_WR_B) {
		reg = 0x03;
	} else if(buffer_mode == RD_B_WR_A) {
		reg = 0x02;
	} else {
		reg = 0x80; // by GPL32900
	}
	pCdspReg7->cdspYuvABfbWrIdx = reg;
}

/**
* @brief	cdsp read back size set
* @param	hoffset [in]: read back h offset
* @param	voffset [in]: read back v offset
* @param	hsize [in]: read back h size
* @param	vsize [in]: read back v size
* @return	none
*/
void 
gpHalCdspSetReadBackSize(
	UINT16 hoffset,
	UINT16 voffset,
	UINT16 hsize,
	UINT16 vsize
)
{
	//printk("%s: hsize = %d, vsize = %d\r\n", __FUNCTION__, hsize, vsize);
	pCdspReg1->cdspWdramHOffset[0] = (UINT8)(hoffset & 0x00FF); // by GPL32900
	pCdspReg1->cdspWdramHOffset[1] = (UINT8)((hoffset & 0x0F00) >> 8); // by GPL32900
	pCdspReg1->cdspWdramVOffset[0] = (UINT8)(voffset & 0x00FF); // by GPL32900
	pCdspReg1->cdspWdramVOffset[1] = (UINT8)((voffset & 0x0F00) >> 8); // by GPL32900
	pCdspReg1->cdspRbHOffset[0] = (UINT8)(hoffset & 0x00FF);
	pCdspReg1->cdspRbHOffset[1] = (UINT8)((hoffset & 0x0F00) >> 8);
	pCdspReg1->cdspRbVOffset[0] = (UINT8)(voffset & 0x00FF);
	pCdspReg1->cdspRbVOffset[1] = (UINT8)((voffset & 0x0F00) >> 8);
	pCdspReg1->cdspRbHSize[0] = (UINT8)(hsize & 0x00FF);
	pCdspReg1->cdspRbHSize[1] = (UINT8)((hsize & 0x0F00) >> 8);
	pCdspReg1->cdspRbVSize[0] = (UINT8)(vsize & 0x00FF);
	pCdspReg1->cdspRbVSize[1] = (UINT8)((vsize & 0x0F00) >> 8);
}

/**
* @brief	cdsp write register
* @param 	reg[in]: register address
* @param 	value[in]: register value
* @return 	SUCCESS/ERROR
*/
UINT8
gpHalCdspWriteReg(
	UINT32 reg,
	UINT8 value
)
{
	reg -= IO3_START + 0x1000;
	if(reg > 0x700)
		return -1;

	(*(volatile unsigned char *)(LOGI_ADDR_CDSP_REG + reg)) = value;
	return 0;
}

/**
* @brief	cdsp read register
* @param 	reg[in]: register address
* @param 	value[in]: register value
* @return 	SUCCESS/ERROR
*/
UINT8
gpHalCdspReadReg(
	UINT32 reg,
	UINT8 *value
)
{
	reg -= IO3_START + 0x1000;
	if(reg > 0x700)
		return -1;
	
	*value = (*(volatile unsigned char*)(LOGI_ADDR_CDSP_REG + reg));
	return 0;
}

