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
#include <linux/io.h>
#include <mach/hal/regmap/reg_mipi.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/hal/hal_mipi.h>
#include <mach/module.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

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
static mipiReg_t *pMipi0Reg = (mipiReg_t *)LOGI_ADDR_MIPI_PPU_REG;
static mipiReg_t *pMipi1Reg = (mipiReg_t *)LOGI_ADDR_MIPI_CDSP_REG;
static scuaReg_t *pScuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG; //0x93007000
static scubReg_t *pScubReg = (scubReg_t *)LOGI_ADDR_SCU_B_REG; //0x90005000

/**
 * @brief   mipi phy input io select
 * @param   mipi_csi_io[in]: 0: MIPI_CLKNP, DATA0NP, 1: CSI, B_Data[8:5]
 * @return  None
 */
void gpHalMipiPhyInSelect(
	UINT8 mipi_csi_io
)
{
	pScubReg->scubMipiPinSel &= ~(0x03 << 4);
	if(mipi_csi_io == C_IO_IS_MIPI_PIN){	
		// I_MIPI_CLKP, CLKN, DATA0P, DATA0N
		pScubReg->scubMipiPinSel |= (0x01 << 4);
	} else {	
		//B_CMOS[8:5]
		pScubReg->scubMipiPinSel |= (0x02 << 4);
	}
}

/**
 * @brief   mipi phy output mux select
 * @param   idx[in]: 0: Data to PPU, 1: Data to CDSP
 * @return  None
 */
void gpHalMipiPhyOutSelect(
	UINT8 idx
)
{
	if(idx == C_MIPI_RX_CDSP){	
		//cdsp mipi rx control
		pScubReg->scubMipiPinSel |= 0x01;
		//disable mipi to ppu csi
		pScuaReg->scuaSysSel &= ~(1<<13);
	} else {	
		//ppu mipi rx control
		pScubReg->scubMipiPinSel &= ~0x01;
		//enable mipi to ppu csi
		pScuaReg->scuaSysSel |= 1<<13;
	}
}

/**
 * @brief   mipi+cdsp module clock set enable
 * @param   idx: select mipi interface
 * @param   enable[in]: enable/disable
 * @return  None
 * @see
 */
void 
gpHalMipiSetModuleClk(
	UINT8 idx,
	UINT8 enable
)
{
	if(idx == C_MIPI_RX_PPU) {
		gp_enable_clock( (int*)"MIPI", enable );
	} else {
		gp_enable_clock( (int*)"MIPI_RX1", enable );
	}
	
	gp_enable_clock( (int*)"MIPI_PHY", enable );
}

/**
 * @brief   mipi irq enable set
 * @param   idx: select mipi interface
 * @return  None
 * @see
 */
void 
gpHalMipiSetIrq(
	UINT8 idx, 
	UINT32 enable,
	UINT32 bits
)
{
	mipiReg_t *pMipiReg;
	
	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}

	if(enable) {
		pMipiReg->mipiIntSource = bits & 0x3F;
		pMipiReg->mipiIntEnable |= bits & 0x3F; 
	} else {
		pMipiReg->mipiIntSource = bits & 0x3F;
		pMipiReg->mipiIntEnable &= ~bits & 0x3F; 
	}
}

/**
 * @brief   mipi irq status get
 * @param   idx: select mipi interface
 * @return  None
 * @see
 */
UINT32 
gpHalMipiGetIrqStatus(
	UINT8 idx
)
{
	UINT32 status;
	UINT32 enable;
	mipiReg_t *pMipiReg;
		
	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return 0;
	}
	
	status = pMipiReg->mipiIntFlag;
	enable = pMipiReg->mipiIntEnable;
	status &= enable;
	if(status) {
		pMipiReg->mipiIntSource = status;
	}
	
	return status;
}

/**
 * @brief   mipi set enable
 * @param   idx: select mipi interface
 * @param   mipi_en[in]: mipi enable
 * @return  None
 * @see
 */
void
gpHalMipiSetEnable(
	UINT8 idx,
	UINT8 mipi_en
)
{
	mipiReg_t *pMipiReg;

	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}
	
	if(mipi_en) {
		pMipiReg->mipiGlbCsr = 0x01;
		pMipiReg->mipiEccOrder = 0;
		pMipiReg->mipiCCIR601Timing = 0;
		pMipiReg->mipiImgSize = 0;
		pMipiReg->mipiDataFmt = 0;
		pMipiReg->mipiIntEnable = 0;
		pMipiReg->mipiIntSource = 0x3F;
	} else {
		pMipiReg->mipiGlbCsr = 0;
	}
}


/**
 * @brief   mipi set global configure
 * @param   idx: select mipi interface
 * @param   low_power_en[in]:  low power enable
 * @param   byte_clk_edge[in]: data clock edge
 * @param   lane_num[in]: 1: mipi 2 lane, 0: mipi 1lane 
 * @return  None
 * @see
 */
void
gpHalMipiSetGloblaCfg(
	UINT8 idx,
	UINT8 low_power_en,
	UINT8 byte_clk_edge,
	UINT8 lane_num_sys
)
{
	mipiReg_t *pMipiReg;
	UINT32 reg;
	
	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}

	if(lane_num_sys) {
		lane_num_sys = 0;
	}

	reg = pMipiReg->mipiGlbCsr;
	reg &= ~(0x03 << 4);
	reg |= ((lane_num_sys & 0x01) << 8) | ((byte_clk_edge & 0x01) << 5) | ((low_power_en & 0x01) << 4);
	pMipiReg->mipiGlbCsr = reg; 
}

/**
 * @brief   mipi set reset
 * @param   idx: select mipi interface
 * @param   enable[in]: enable mipi reset
 */
void
gpHalMipiReset(
	UINT8 idx,
	UINT8 enable
)
{
	mipiReg_t *pMipiReg;

	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}
	
	pMipiReg->mipiPhyRst = (enable & 0x01) << 4;
}

/**
 * @brief   mipi set ecc order
 * @param   idx: select mipi interface
 * @param   ecc_order[in]: ecc order set
 * @param   ecc_check_en[in]: ecc check enable
 */
void
gpHalMipiSetEcc(
	UINT8 idx,
	UINT8 ecc_order,
	UINT8 ecc_check_en
)
{
	mipiReg_t *pMipiReg;
	UINT32 reg;
	
	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}
	
	reg = pMipiReg->mipiEccOrder;
	reg &= ~(0x07);
	reg |= ((ecc_check_en & 0x01) << 2) | (ecc_order & 0x03);
	pMipiReg->mipiEccOrder = reg;
}

/**
 * @brief   mipi set LP to HS mask count
 * @param   idx: select mipi interface
 * @param   da_mask_cnt[in]: LP to HS mask count
 * @param   check_hs_seq[in]: 1: Check HS sequence. 0: just check LP. when enter HS mode
 */
void
gpHalMipiSetMaskCnt(
	UINT8 idx,
	UINT8 da_mask_cnt,
	UINT8 check_hs_seq
)
{
	mipiReg_t *pMipiReg;
	UINT32 reg;
	
	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}

	reg = pMipiReg->mipiEccOrder;
	reg &= ~(0x1FF << 8);
	reg |= (((UINT32)check_hs_seq & 0x01) << 16) | (((UINT32)da_mask_cnt & 0xFF) << 8);
	pMipiReg->mipiEccOrder = reg;
}

/**
 * @brief   mipi set ccir601 interface
 * @param   idx: select mipi interface
 * @param   h_back_proch[in]: horizontal back proch
 * @param   h_front_proch[in]: horizontal front proch
 * @param   blanking_line_en[in]: blanking line enable, 0 mask hsync when vsync
 */
void
gpHalMipiSetCCIR601IF(
	UINT8 idx,
	UINT8 h_back_porch,
	UINT8 h_front_porch,
	UINT8 blanking_line_en
)
{
	mipiReg_t *pMipiReg;

	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}
	
	pMipiReg->mipiCCIR601Timing = (((UINT32)blanking_line_en & 0x01) << 16) |
								(((UINT32)h_front_porch & 0x0F) << 8) |
								(h_back_porch & 0x0F);
}

/**
 * @brief   mipi set image size
 * @param   idx: select mipi interface 
 * @param   type[in]: 0: raw, 1: yuv
 * @param   h_size[in]: horizontal size set
 * @param   v_size[in]: vertical size set
 */
void
gpHalMipiSetImageSize(
	UINT8 idx,
	UINT8 type,
	UINT16 h_size,
	UINT16 v_size
)
{
	mipiReg_t *pMipiReg;

	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}

	if(h_size == 0) {
		h_size = 1;
	}

	if(v_size == 0) {
		v_size = 1;
	}
	
	pMipiReg->mipiImgSize = (((UINT32)v_size & 0xFFFF) << 16) | (h_size & 0xFFFF);
}

/**
 * @brief   mipi set data format
 * @param   idx: select mipi interface 
 * @param   data_from_mmr[in]: mmr decide data type enable
 * @param   data_type_mmr[in]: data format
 * @param   data_type_cdsp_sys[in]: 1: output to cdsp,
 */
void
gpHalMipiSetDataFmt(
	UINT8 idx,
	UINT8 data_from_mmr,
	UINT8 data_type_mmr
)
{
	mipiReg_t *pMipiReg;

	if(idx == C_MIPI_RX_PPU) {
		pMipiReg = pMipi0Reg;
	} else if(idx == C_MIPI_RX_CDSP) {
		pMipiReg = pMipi1Reg;
	} else {
		return;
	}

	pMipiReg->mipiDataFmt = ((data_type_mmr & 0x07) << 4) | (data_from_mmr & 0x01);
}

