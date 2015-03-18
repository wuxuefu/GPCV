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
#ifndef _HAL_MIPI_H_
#define _HAL_MIPI_H_

#include <mach/common.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#ifndef ENABLE
#define ENABLE 		1
#endif
#ifndef DISABLE
#define DISABLE		0
#endif

/* IO Setting */
#define C_IO_IS_CMOS_PIN 		0
#define C_IO_IS_MIPI_PIN 		1

/* MIPI PHY to PPU / CDSP */
#define C_MIPI_RX_PPU			0
#define C_MIPI_RX_CDSP			1

/* mipi clock sample edge*/
#define D_PHY_SAMPLE_POS		0x0
#define D_PHY_SAMPLE_NEG		0x1

/* mipi lane */
#define MIPI_1_LANE				0x0
#define MIPI_2_LANE				0x1

/* mipi ecc order*/
#define MIPI_ECC_ORDER0			0x0
#define MIPI_ECC_ORDER1			0x1
#define MIPI_ECC_ORDER2			0x2
#define MIPI_ECC_ORDER3			0x3

/* mipi check method*/
#define MIPI_CHECK_LP_00		0x0
#define MIPI_CHECK_HS_SEQ		0x1

/* mipi format */
#define MIPI_YUV422				0x0
#define MIPI_RGB888				0x1
#define MIPI_YUV565				0x2
#define MIPI_RAW8				0x3
#define MIPI_RAW10				0x4
#define MIPI_RAW12				0x5
#define MIPI_GENERIC_8_BIT		0x6
#define MIPI_USER_DEFINE_BYTE	0x7

/* irq flag */
#define MIPI_LANE0_SOT_SYNC_ERR	0x01
#define MIPI_HD_1BIT_ERR 		0x02
#define MIPI_HD_NBIT_ERR		0x04
#define MIPI_DATA_CRC_ERR		0x08
#define MIPI_LANE1_SOT_SYNC_ERR	0x10
#define MIPI_CCIR_SOF 			0x20


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

/**
 * @brief   mipi phy input io select
 * @param   mipi_csi_io[in]: 0: MIPI_CLKNP, DATA0NP, 1: CSI, B_Data[8:5]
 * @return  None
 */
void gpHalMipiPhyInSelect(UINT8 mipi_csi_io);

/**
 * @brief   mipi phy output mux select
 * @param   idx[in]: 0: Data to PPU, 1: Data to CDSP
 * @return  None
 */
void gpHalMipiPhyOutSelect(UINT8 idx);

/**
 * @brief   mipi+cdsp module clock set enable
 * @param   idx: select mipi interface
 * @param   enable[in]: enable/disable
 * @return  None
 * @see
 */
void gpHalMipiSetModuleClk(UINT8 idx, UINT8 enable);


/**
 * @brief   mipi irq enable set
 * @param   idx: select mipi interface
 * @return  None
 * @see
 */
void gpHalMipiSetIrq(UINT8 idx,	UINT32 enable, UINT32 bits);

/**
 * @brief   mipi irq status get
 * @param   idx: select mipi interface
 * @return  None
 * @see
 */
UINT32 gpHalMipiGetIrqStatus(UINT8 idx);

/**
 * @brief   mipi set enable
 * @param   idx: select mipi interface
 * @param   mipi_en[in]: mipi enable
 * @return  None
 * @see
 */
void gpHalMipiSetEnable(UINT8 idx, UINT8 mipi_en);

/**
 * @brief   mipi set global configure
 * @param   idx: select mipi interface
 * @param   low_power_en[in]:  low power enable
 * @param   byte_clk_edge[in]: data clock edge
 * @param   lane_num[in]: 1: mipi 2 lane, 0: mipi 1lane 
 * @return  None
 * @see
 */
void gpHalMipiSetGloblaCfg(UINT8 idx, UINT8 low_power_en, UINT8 byte_clk_edge, UINT8 lane_num_sys);

/**
 * @brief   mipi set reset
 * @param   idx: select mipi interface
 * @param   enable[in]: enable mipi reset
 */
void gpHalMipiReset(UINT8 idx, UINT8 enable);

/**
 * @brief   mipi set ecc order
 * @param   idx: select mipi interface
 * @param   ecc_order[in]: ecc order set
 * @param   ecc_check_en[in]: ecc check enable
 */
void gpHalMipiSetEcc(UINT8 idx, UINT8 ecc_order, UINT8 ecc_check_en);

/**
 * @brief   mipi set LP to HS mask count
 * @param   idx: select mipi interface
 * @param   da_mask_cnt[in]: 
 * @param   check_hs_seq[in]: 
 */
void gpHalMipiSetMaskCnt(UINT8 idx, UINT8 da_mask_cnt, UINT8 check_hs_seq);

/**
 * @brief   mipi set ccir601 interface
 * @param   idx: select mipi interface
 * @param   h_back_proch[in]: horizontal back proch
 * @param   h_front_proch[in]: horizontal front proch
 * @param   blanking_line_en[in]: blanking line enable, 0 mask hsync when vsync
 */
void gpHalMipiSetCCIR601IF(UINT8 idx, UINT8 h_back_porch, UINT8 h_front_porch, UINT8 blanking_line_en);

/**
 * @brief   mipi set image size
 * @param   idx: select mipi interface 
 * @param   type[in]: 0: raw, 1: yuv
 * @param   h_size[in]: horizontal size set
 * @param   v_size[in]: vertical size set
 */
void gpHalMipiSetImageSize(UINT8 idx, UINT8 type, UINT16 h_size, UINT16 v_size);

/**
 * @brief   mipi set data format
 * @param   idx: select mipi interface 
 * @param   data_from_mmr[in]: mmr decide data type enable
 * @param   data_type_mmr[in]: data format
 * @param   data_type_cdsp_sys[in]: 1: output to cdsp,
 */
void gpHalMipiSetDataFmt(UINT8 idx, UINT8 data_from_mmr, UINT8 data_type_mmr);


#endif /* _HAL_MIPI_H_ */
