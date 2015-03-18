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
 * @file    hal_sdma.c
 * @brief   Implement of SDMA hal driver.
 * @author  
 */
 
#include <mach/hal/hal_sdma.h>
#include <mach/hal/regmap/reg_sdma.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/kernel.h>
#include <mach/hal/hal_clock.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

#define SDMA_CLK_EN				1
#define SDMA_CLK_DIS			0

static const SINT32 gSdmaRegBase[] = {GSDMA_BASE_CH0, GSDMA_BASE_CH1};

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

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

/**
* @brief 		SDMA clock enable.
* @param 		en[in]: Clock enable or disable.
* @return		None.
*/
static void 
gpHalSdmaClk(
	UINT32 en
)
{	
	gp_enable_clock( (int*)"SDMA", 1 );
}

/**
* @brief 		SDMA hardware module reset.
* @return		None.
*/
void
gpHalSdmaHWReset(void) 
{
	scucReg_t *pScucReg = (scucReg_t *)LOGI_ADDR_SCU_C_REG;
	
	pScucReg->scucPeriRst |= SCU_C_PERI_SDMA;
	pScucReg->scucPeriRst &= ~SCU_C_PERI_SDMA;
}

/**
* @brief 		SDMA channel reset (software reset).
* @param 		indexChan[in]: Channel Number.
* @return		None.
*/
void
gpHalSdmaReset(
	UINT32 indexChan
) 
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];
	
	sdma_reg->sdma_reg_status = 0;
	sdma_reg->sdma_reg_status = SDMA_STATUS_PAR;
	
	while(!gpHalCheckStatus(indexChan, SDMA_STATUS_PAU));
	sdma_reg->sdma_reg_status |= SDMA_STATUS_STOP;
}

/**
* @brief 		SDMA channel enable.
* @param 		indexChan[in]: Channel Number.
* @return		None.
*/
void 
gpHalEnble(
	UINT32 indexChan)
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];
	
	gpHalSdmaReset(indexChan);
	sdma_reg->sdma_reg_status = SDMA_STATUS_CHEN;
}

/**
* @brief 		SDMA channel status.
* @param 		indexChan[in]: Channel Number.
* @param 		statusBit[in]: Check Status.
* @return		Channel status.
*/
UINT32
gpHalCheckStatus(
	UINT32 indexChan,
	UINT32 statusBit
) 
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];

	return (sdma_reg->sdma_reg_status & statusBit);
}

/**
* @brief 		SDMA channel irq status.
* @param 		indexChan[in]: Channel Number.
* @return		Irq status.
*/
UINT32
gpHalGetIrq(
	UINT32 indexChan
) 
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];

	return sdma_reg->sdma_reg_irr;
}

/**
* @brief 		SDMA channel irq status.
* @param 		indexChan[in]: Channel Number.
* @return		Status.
*/
void
gpHalClearIrq(
	UINT32 indexChan
) 
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];

	sdma_reg->sdma_reg_icr = SDMA_ICR_CFIN | SDMA_ICR_CSERR | SDMA_ICR_CDERR
		| SDMA_ICR_CDIDX_END_BLOCK | SDMA_ICR_CDIDX_END_FRAME;
}

/**
* @brief 		Dump SDMA channel register. (only for debug)
* @param 		indexChan[in]: Channel Number.
* @return		None.
*/
void gpHalDump(
	UINT32 indexChan
) 
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];
	SINT32 i = 0;
	
	for (; i < sizeof(sdma_reg_t) / 4; i++) 
	{
		printk("register_%d:%08x\n", i, *(((UINT32*)sdma_reg)+i) );
	}
}

/**
* @brief 		SDMA initial function.  
* @return		None.
*/
void 
gpHalSdmaInit(void)
{
	/* ----- Enable clock ----- */
	gpHalSdmaClk(SDMA_CLK_EN);
	/* ----- Hardware reset ----- */
	gpHalSdmaHWReset();
}

/**
* @brief 		SDMA un-initial function.  
* @return		None.
*/
void 
gpHalSdmaUninit(void)
{	
	/* ----- Disable clock ----- */
	gpHalSdmaClk(SDMA_CLK_DIS);	
}

/**
* @brief 		SDMA IRQ mask function.  
* @param 		indexChan[in]: Channel Number.
* @return		None.
*/
void gpHalMaskIrq(
	UINT32 indexChan
) 
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];
	
	sdma_reg->sdma_reg_imr = SDMA_IMR_MLL_ENDMODE
		| SDMA_IMR_MLL_TRIGGER
		| SDMA_IMR_MSIDX_END_HBLOCK
		| SDMA_IMR_MSIDX_END_BLOCK
		| SDMA_IMR_MSIDX_END_FRAME
		| SDMA_IMR_MSIDX_END_PACKET
		| SDMA_IMR_MSIDX_END_INDEXMODE
		//| SDMA_IMR_MSERR
		//| SDMA_IMR_MFIN
		| SDMA_IMR_MDIDX_END_HBLOCK
		| SDMA_IMR_MDIDX_END_BLOCK
		| SDMA_IMR_MDIDX_END_FRAME
		| SDMA_IMR_MDIDX_END_PACKET
		| SDMA_IMR_MDIDX_END_INDEXMODE;
		//| SDMA_IMR_MDERR;
}

/**
* @brief 		SDMA channel start transfer extented function. (for one~three dimension transfer)
* @param 		indexChan[in]: Channel Number.
* @param 		pSdma[in]: Channel parameter.
* @return		None.
*/
void
gpHalSdmaTrriger(
	UINT32 indexChan, 
	gpSdma_t *pSdma
) 
{
	sdma_reg_t *sdma_reg = (sdma_reg_t *)gSdmaRegBase[indexChan];
	UINT32 cfg = 0;
	UINT32 size;
	UINT32 align = ((UINT32)pSdma->dstAddr) | ((UINT32)pSdma->srcAddr) | ((UINT32)pSdma->blockSize) | ((UINT32)pSdma->bStepSize) \
					| ((UINT32)pSdma->fStepSize) | ((UINT32)pSdma->dbStepSize) | ((UINT32)pSdma->dfStepSize);
	/* ----- Check align ----- */
	if((align&0x03)==0)
	{
		size = pSdma->blockSize >>2;
		cfg |= SDMA_CFG_SSIZE_32BIT | SDMA_CFG_DSIZE_32BIT;
	}
	else if ((align&0x01)==0)
	{
		size = pSdma->blockSize >>1;
		cfg |= SDMA_CFG_SSIZE_16BIT | SDMA_CFG_DSIZE_16BIT;
	}
	else
	{
		size = pSdma->blockSize;
		cfg |= SDMA_CFG_SSIZE_8BIT | SDMA_CFG_DSIZE_8BIT;
	}	
	/* ----- Check burst ----- */
	if((size&0xf)==0)
		cfg |= SDMA_CFG_SBST_16 | SDMA_CFG_DBST_16 ;
	else if ((size&0x07)==0)
		cfg |= SDMA_CFG_SBST_8 | SDMA_CFG_DBST_8 ;
	else if ((size&0x03)==0)
		cfg |= SDMA_CFG_SBST_4 | SDMA_CFG_DBST_4 ;
	else
		cfg |= SDMA_CFG_SBST_1 | SDMA_CFG_DBST_1 ;
	
	sdma_reg->sdma_reg_cfg = cfg;
	
	if (pSdma->frameSize == 0) 
	{
		sdma_reg->sdma_reg_ctrl = SDMA_CTRL_SAM_INC | SDMA_CTRL_DAM_INC
			| SDMA_CTRL_DID_MEM | SDMA_CTRL_SID_MEM;
	}
	else 
	{
		sdma_reg->sdma_reg_ctrl = SDMA_CTRL_SAM_INDEX | SDMA_CTRL_DAM_INDEX
			| SDMA_CTRL_DID_MEM | SDMA_CTRL_SID_MEM;
	}

	sdma_reg->sdma_reg_dadr = (UINT32)pSdma->dstAddr;
	sdma_reg->sdma_reg_sadr = (UINT32)pSdma->srcAddr;
	sdma_reg->sdma_reg_dbsize = pSdma->blockSize;
	sdma_reg->sdma_reg_sbsize = pSdma->blockSize;
	sdma_reg->sdma_reg_ecfg = 0;
	sdma_reg->sdma_reg_sfsize = pSdma->frameSize;
	sdma_reg->sdma_reg_dfsize = pSdma->frameSize;
	sdma_reg->sdma_reg_sbstep = pSdma->bStepSize;
	sdma_reg->sdma_reg_dbstep = pSdma->dbStepSize;
	sdma_reg->sdma_reg_sfstep = pSdma->fStepSize;
	sdma_reg->sdma_reg_sfstep = pSdma->dfStepSize;
	sdma_reg->sdma_reg_spsize = pSdma->packetSize;
	sdma_reg->sdma_reg_dpsize = pSdma->packetSize;
	
	/* ----- set sdma start bit ----- */
	sdma_reg->sdma_reg_status |= SDMA_STATUS_CHEN;
}