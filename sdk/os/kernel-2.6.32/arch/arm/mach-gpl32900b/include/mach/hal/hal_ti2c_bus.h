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
 * @file    hal_i2c_bus.h
 * @brief   Declaration of GPL32900 I2C Bus HAL API.
 * @author  Simon.Hsu
 * @since   2010/10/12
 * @date    2010/10/12
 */

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define	CMD_GEN_STR		0x01
#define	CMD_GEN_STP		0x02
#define	CMD_GEN_ACK		0x04
#define	CMD_GEN_NACK	0x08
#define	CMD_XMIT		0x10
#define CMD_RCVR		0x20

enum {
	HAL_I2C_INT_ARBITRATION_LOST = 1,
	HAL_I2C_INT_NACK = 2,
	HAL_I2C_INT_TRANSMIT_READY = 4,
	HAL_I2C_INT_RECEIVE_READY = 8,
	HAL_I2C_INT_TRIGGER_LEVEL_REACH = 0x10,
	HAL_I2C_INT_RW = 0x20,
	HAL_I2C_INT_ADDRESS_AS_SLAVE = 0x40,
	HAL_I2C_INT_GENERAL_CALL = 0x80,
};

/*slaveAddrMode*/
enum {
	HAL_I2C_NORMAL_SLAVEADDR_8BITS = 0,
	HAL_I2C_NORMAL_SLAVEADDR_16BITS
};


void gpHalTi2cBusSetClkRate(UINT32 clkRate);
int gpHalTi2cSendCmd(int cmd);
void gpHalTi2cTxData(UINT8 data);
UINT8 gpHalTi2cRxData(void);
UINT8 gpHalTi2cIntFlagGet(void);
void gpHalTi2cIntFlagClr(UINT8 intReg);
void gpHalTi2cIntEnSet(UINT8 intReg, UINT8 en);
UINT8 gpHalTi2cGetStatus(void);
void gpHalTi2cClrFIFO(void);
void gpHalTi2cMasterFlowSetting(UINT8 slaveAddrMode, UINT16 slaveAddr, UINT32 dataCnt, UINT8 dir);
void gpHalTi2cClear(void);
int gpHalTi2cWaitBusIdle(void);
int gpHalTi2cWaitDR(void);
int gpHalTi2cSendStop(void);
void gpHalTi2cSetDataCnt(UINT32 dataCnt);
void gpHalTi2cReset(void);
int gpHalTi2cWaitTxRdy(int ms);

/**
* @brief	Enable Ti2c interrupt.
* @param	en[in]: enable
* @return: 	None.
*/
void gpHalTi2cIRQEn (UINT8 en);
void gpHalTi2cBusInit(void);
