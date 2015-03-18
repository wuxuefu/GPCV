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
 * @file    hal_i2c_bus.c
 * @brief   Implement of GPL32900 I2C Bus HAL API.
 * @author  Simon.Hsu
 * @since   2012/03/26
 * @date    2012/03/26
 */
 
#include <mach/hal/regmap/reg_scu.h>
#include <mach/kernel.h>
#include <mach/hal/regmap/reg_ti2c_bus.h>
#include <mach/hal/hal_common.h>
#include <mach/hal/hal_ti2c_bus.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define STS_CBSY_MASK		0x40
#define STS_BBSY_MASK		0x80

#define RTDR_INIT			0x00
#define IER_INIT			0x00
#define IIR_INIT			0x00
#define OAR_INIT			0x00
#define SAR_INIT			0x00
#define XAR_INIT			0x00
#define FCR_INIT			0x00
#define CFG_INIT			0x01
#define CMD_INIT			0x00
#define STS_INIT			0x00
#define CNT_INIT			0x00
#define PSL_INIT			0x5F
#define PSM_INIT			0x00

#define	FIFO_CTL_MFCEN		0x01
#define	FIFO_RFRST			0x02
#define	FIFO_TFRST			0x04

#define BUSY_STS         	0xC0
#define STS_DR_MASK        	0x20
#define	STS_TR_MASK			0x10

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static ti2cBusReg_t *ti2cBusReg = (ti2cBusReg_t *)(LOGI_ADDR_TI2C_BUS_REG);

/**
 * @brief   set I2C Bus clock rate
 */
void
gpHalTi2cBusSetClkRate(
	UINT32 clkRate
)
{
	ti2cBusReg->TI2C_PSM = (clkRate & 0xFF00)>>8;
	ti2cBusReg->TI2C_PSL = clkRate & 0xFF;
}

/**
 * @brief   I2C Bus Send command
 */
int
gpHalTi2cSendCmd(
	int cmd
)
{
	int ret;
	
	ret = HAL_BUSY_WAITING(((ti2cBusReg->TI2C_STS & STS_CBSY_MASK)==0), 30);
	if(ret<0)
		DIAG_ERROR("TI2C Send Command Fail, cmd=0x%x\n", cmd);
	else
		ti2cBusReg->TI2C_CMD = cmd;
	return ret;
}

int
gpHalTi2cSendStop(
	void
)
{
	int temp, ret;
	
	ret = HAL_BUSY_WAITING(((ti2cBusReg->TI2C_STS & STS_CBSY_MASK)==0), 30);
	if(ret<0)
		DIAG_ERROR("TI2C Send Stop Command Fail\n");
	else {
		temp = ti2cBusReg->TI2C_CMD & 0x30;
		ti2cBusReg->TI2C_CMD = (temp | 0x2);
	//	DIAG_ERROR("TI2C mode is (R/T): %d\n", temp);
	}
	return ret;
}

void
gpHalTi2cTxData(
	UINT8 data
)
{
	ti2cBusReg->TI2C_RTDR = data;
}

UINT8
gpHalTi2cRxData(
	void
)
{
	return ti2cBusReg->TI2C_RTDR;
}

UINT8 gpHalTi2cIntFlagGet(
	void
)
{
	return ti2cBusReg->TI2C_IIR;
}

UINT8 gpHalTi2cGetStatus(
	void
)
{
	return ti2cBusReg->TI2C_STS;
}

void gpHalTi2cIntFlagClr(
	UINT8 intReg
)
{
	ti2cBusReg->TI2C_IIR = intReg;
}

void gpHalTi2cIntEnSet(
	UINT8 intReg, UINT8 en
)
{
	if(en)
		ti2cBusReg->TI2C_IER = ti2cBusReg->TI2C_IER | intReg;
	else
		ti2cBusReg->TI2C_IER = ti2cBusReg->TI2C_IER & (~intReg);
}

void gpHalTi2cClrFIFO(
	void
)
{
	ti2cBusReg->TI2C_FCR |= FIFO_RFRST | FIFO_TFRST;
}

/**
* @brief	Enable Ti2c interrupt.
* @param	en[in]: enable
* @return: 	None.
*/
void gpHalTi2cIRQEn (UINT8 en)
{
	UINT32 bit = (en == 1)? 0x80:~(0x80);
	UINT32 reg;

//	reg = APBDMA_INT_EN;
//	APBDMA_INT_EN = reg | bit;
}
EXPORT_SYMBOL(gpHalTi2cIRQEn);

/**
 * @brief   I2C Bus hardware initial
 * @return  None
 * @see
 */
void 
gpHalTi2cBusInit(
	void
)
{
/*	ti2cBusReg->TI2C_CMD = CMD_INIT;
	ti2cBusReg->TI2C_IER = IER_INIT;
	ti2cBusReg->TI2C_IIR = IIR_INIT;
	ti2cBusReg->TI2C_OAR = OAR_INIT;
	ti2cBusReg->TI2C_SAR = SAR_INIT;
	ti2cBusReg->TI2C_XAR = XAR_INIT;
	ti2cBusReg->TI2C_FCR = FCR_INIT;
	ti2cBusReg->TI2C_CFG = CFG_INIT;
	ti2cBusReg->TI2C_STS = STS_INIT;
	ti2cBusReg->TI2C_CNT = CNT_INIT;
	ti2cBusReg->TI2C_PSL = PSL_INIT;
	ti2cBusReg->TI2C_PSM = PSM_INIT;*/
	ti2cBusReg->TI2C_CFG = CFG_INIT;
	ti2cBusReg->TI2C_PSL = PSL_INIT;
}

void
gpHalTi2cMasterFlowSetting(
	UINT8 slaveAddrMode, UINT16 slaveAddr, UINT32 dataCnt, UINT8 dir
)
{
	ti2cBusReg->TI2C_FCR = 0x09;
	if(slaveAddrMode!=HAL_I2C_NORMAL_SLAVEADDR_16BITS)
		ti2cBusReg->TI2C_SAR = ((slaveAddr) >> 1) & 0x7f;

	ti2cBusReg->TI2C_CNT = dataCnt - 1;
	if(slaveAddrMode!=HAL_I2C_NORMAL_SLAVEADDR_16BITS)
		ti2cBusReg->TI2C_CMD = 0x1 | dir<<4;
	else
		ti2cBusReg->TI2C_CMD = 0x0 | dir<<4;
}

void
gpHalTi2cSetDataCnt(
	UINT32 dataCnt
)
{
	ti2cBusReg->TI2C_CNT = dataCnt - 1;
}

void gpHalTi2cClear(
	void
)
{
	ti2cBusReg->TI2C_CMD = 0;
	ti2cBusReg->TI2C_FCR = FIFO_RFRST | FIFO_TFRST;		//Clear fifo & diable master flow control
	ti2cBusReg->TI2C_SAR = 0;
}

int gpHalTi2cWaitTxRdy(
	int ms
)
{
	return HAL_BUSY_WAITING((ti2cBusReg->TI2C_STS & STS_TR_MASK), ms);
}

int gpHalTi2cWaitBusIdle(
	void
)
{
	return HAL_BUSY_WAITING(((ti2cBusReg->TI2C_STS & BUSY_STS) == 0), 30);
}

int gpHalTi2cWaitDR(
	void
)
{
	return HAL_BUSY_WAITING((ti2cBusReg->TI2C_STS & STS_DR_MASK), 30);
}

void gpHalTi2cReset(
	void
)
{
	int ret;
	
	ret = HAL_BUSY_WAITING(((ti2cBusReg->TI2C_STS & STS_CBSY_MASK)==0), 30);
	if(ret<0)
		DIAG_ERROR("TI2C Send Stop Command Fail\n");
	else {
		ti2cBusReg->TI2C_CMD |= 0x80;
	}
}
