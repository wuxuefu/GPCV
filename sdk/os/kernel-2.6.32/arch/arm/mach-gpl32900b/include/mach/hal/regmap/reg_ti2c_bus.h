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
 * @file    reg_i2c_bus.h
 * @brief   Regmap of SPMP8050 I2C Bus.
 * @author  junp.zhang
 * @since   2010/10/12
 * @date    2010/10/12
 */
#ifndef _REG_TI2C_BUS_H_
#define _REG_TI2C_BUS_H_

#include <mach/hardware.h>                          
#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define	LOGI_ADDR_TI2C_BUS_REG		IO2_ADDRESS(0xB01000)	/* Turbo i2c */

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct ti2cBusReg_s {
	volatile UINT32  TI2C_RTDR;
	volatile UINT32  TI2C_IER;
	volatile UINT32  TI2C_IIR;
	volatile UINT32  rsv0C;
	volatile UINT32  TI2C_OAR;
	volatile UINT32  TI2C_SAR;
	volatile UINT32  TI2C_XAR;
	volatile UINT32  TI2C_FCR;
	volatile UINT32  TI2C_CFG;
	volatile UINT32  TI2C_CMD;
	volatile UINT32  TI2C_STS;
	volatile UINT32  TI2C_CNT;
	volatile UINT32  TI2C_PSL;
	volatile UINT32  TI2C_PSM;
} ti2cBusReg_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif /* _REG_TI2C_BUS_H_ */
