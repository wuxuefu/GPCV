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
 * @file    hal_i2s.c
 * @brief   Implement of I2S HAL API.
 * @author  Simon Hsu
 */

#include <linux/kernel.h>       /* printk() */
#include <linux/device.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/hal/hal_clock.h>
#include <mach/hal/regmap/reg_i2s.h>

static i2stxReg_t *pi2stxReg = (i2stxReg_t *)LOGI_ADDR_I2STX_REG;
static i2srxReg_t *pi2srxReg = (i2srxReg_t *)LOGI_ADDR_I2SRX_REG;

static int i2stx_en_cnt=0;
static int i2srx_en_cnt=0;

DEFINE_SPINLOCK(i2s_lock);

void gpHalI2sClkEn(int enable)
{
	spin_lock(&i2s_lock);
	gpHalScuClkEnable(SCU_A_PERI_I2S | SCU_A_PERI_I2SRX, SCU_A, enable);
	spin_unlock(&i2s_lock);
}
EXPORT_SYMBOL(gpHalI2sClkEn);

void gpHalI2sCtlSet(int sel, int ctl)
{
	unsigned int val;
	
	val = pi2stxReg->ctl;
	val &= IISTX_ENABLE;
	val |= ctl;
	
	if(sel==IISTX) {
		pi2stxReg->ctl = val;
	} else {
		pi2srxReg->ctl = val;
	}
}
EXPORT_SYMBOL(gpHalI2sCtlSet);

unsigned int gpHalI2sCtlGet(int sel)
{
	if(sel==IISTX) {
		return pi2stxReg->ctl;
	} else {
		return pi2srxReg->ctl;
	}
}
EXPORT_SYMBOL(gpHalI2sCtlGet);

void gpHalI2sFifoClr( int sel )
{
//	int val;
	if( sel==IISTX ) {
		pi2stxReg->ctl |= IISTX_CLRFIFO;
	//	do {
	//		val = pi2stxReg->status;
	//	} while(val & IISTX_EMPTY);
	} else {
		pi2srxReg->ctl |= IISRX_CLRFIFO;
		pi2srxReg->ctl &= ~IISRX_CLRFIFO;
//		do {
//			val = pi2srxReg->status;
//		} while(val & IISRX_EMPTY);
	}		
}
EXPORT_SYMBOL(gpHalI2sFifoClr);

void gpHalI2sIntEn( int sel )
{	
	if(sel==IISTX) {
		pi2stxReg->ctl |= IISTX_EN_IRT;
	} else {
		pi2srxReg->ctl |= IISRX_EN_IRT;
	}
}
EXPORT_SYMBOL(gpHalI2sIntEn);

void gpHalI2sIntDisable( int sel )
{
	if(sel==IISTX) {
		pi2stxReg->ctl |= IISTX_IRT_FLAG;
		pi2stxReg->ctl &= ~IISTX_EN_IRT;
	} else {
		pi2srxReg->ctl |= IISRX_IRT_PEND;
		pi2srxReg->ctl &= ~IISRX_EN_IRT;
	}
}
EXPORT_SYMBOL(gpHalI2sIntDisable);

void gpHalI2sEn( int sel )
{
	unsigned int val;

	spin_lock(&i2s_lock);
	if(sel==IISTX) {
		i2stx_en_cnt++;
		pi2stxReg->ctl &= ~IISTX_SLAVE_MODE;
		pi2stxReg->ctl |= IISTX_ENABLE;
	} else {
//		pi2srxReg->ctl &= ~IISRX_SLAVE_MODE;
//		pi2srxReg->ctl |= IISRX_ENABLE;
		i2srx_en_cnt++;
		do {
			pi2srxReg->ctl |= IISRX_ENABLE;
			val = pi2srxReg->ctl;
		} while(!(val & IISRX_ENABLE));
	}
	spin_unlock(&i2s_lock);
}
EXPORT_SYMBOL(gpHalI2sEn);

void gpHalI2sDisable( int sel )
{
	spin_lock(&i2s_lock);
	if(sel==IISTX) {
		i2stx_en_cnt--;
		if(i2stx_en_cnt<=0) {
			pi2stxReg->ctl &= ~IISTX_ENABLE;
			i2stx_en_cnt=0;
		}
	} else {
		i2srx_en_cnt--;
		if(i2srx_en_cnt<=0) {
			pi2srxReg->ctl &= ~IISRX_ENABLE;
			i2srx_en_cnt=0;
		}
	}
	spin_unlock(&i2s_lock);
}
EXPORT_SYMBOL(gpHalI2sDisable);

void gpHalI2sFmtSet( int sel, int order )
{
	if(sel==IISTX) {
		if(order==0) {
			pi2stxReg->ctl |= IISTX_SENDMODE_LSB;
		} else {
			pi2stxReg->ctl &= ~IISTX_SENDMODE_LSB;
		}
	} else {
		if(order==0) {
			pi2srxReg->ctl |= IISTX_SENDMODE_LSB;
		} else {
			pi2srxReg->ctl &= ~IISTX_SENDMODE_LSB;
		}
	}
}
EXPORT_SYMBOL(gpHalI2sFmtSet);

void gpHalI2sChlSet( int sel, int ch )
{
	if(sel==IISTX) {
		if(ch==1) {
			pi2stxReg->ctl |= IISTX_MONO;
		} else {
			pi2stxReg->ctl &= ~IISTX_MONO;
		}
	} else {
		if(ch==1) {
			pi2srxReg->ctl |= IISRX_MONO;
		} else {
			pi2srxReg->ctl &= ~IISRX_MONO;
		}
	}
}
EXPORT_SYMBOL(gpHalI2sChlSet);
