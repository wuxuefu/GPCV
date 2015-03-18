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
 
/**
 * @file hal_rtc.c
 * @brief RTC HAL interface 
 * @author chao.chen
 */

#include <mach/kernel.h>
#include <mach/diag.h>
//#include <mach/hal/regmap/reg_scu.h>
#include <mach/hal/regmap/reg_rtc.h>
#include <mach/hal/hal_rtc.h>
#include <mach/hal/hal_common.h>

/**************************************************************************
*                               R E G I S T E R 					*
**************************************************************************/


/**************************************************************************
 *                              C O N S T A N T S                               *
 **************************************************************************/
#define HAL_RTC_MACRO_RW_TIMEOUT (100)		/*In ms*/
#define HAL_RTC_SUCCESS 0
#define HAL_RTC_FAIL 1
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define HAL_RTC_PWR_PROTECTION \
	({ \
		prtcReg->rtcCtrl = 0; \
		prtcReg->rtcCtrl = RTC_MACRO_CLK_ENABLE; \	
	})


static UINT32 
rtcMacroRead(
	UINT32 addr
)
{
	rtcReg_t *prtcReg = (rtcReg_t *)(LOGI_ADDR_RTC_REG);
	prtcReg->rtcAddr = addr;
	prtcReg->rtcRWReq = RTC_CTLREAD;

	/*check rtc ctller ready*/
	#if 0
	while (!(prtcReg->rtcRdy & RTC_MARCO_READY)){
		;
	}
	#else
	HAL_BUSY_WAITING((prtcReg->rtcRdy & RTC_MARCO_READY), HAL_RTC_MACRO_RW_TIMEOUT);
	#endif

	return prtcReg->rtcRData;
}

static void 
rtcMacroWrite(
	UINT32 addr, 
	UINT8 wdata
)
{
	rtcReg_t *prtcReg = (rtcReg_t *)(LOGI_ADDR_RTC_REG);
	prtcReg->rtcAddr = addr;
	prtcReg->rtcWData = wdata;
	prtcReg->rtcRWReq = RTC_CTLWRITE;
	
	/*check rtc ctller ready*/
	#if 0
	while (!(prtcReg->rtcRdy & RTC_MARCO_READY)){
		;
	}
	#else
	HAL_BUSY_WAITING((prtcReg->rtcRdy & RTC_MARCO_READY), HAL_RTC_MACRO_RW_TIMEOUT);
	#endif
}

/**
* @brief RTC Interrupt source read and clear.
* @return : Last Interrupt source.
*/
UINT32 
gpHalRtcIntSrcGet(
	void
)
{
	/* clear interrupt sources */
	UINT32 rtsr, intrEn;
	rtsr = rtcMacroRead(RTC_INTR_STATUS_MARCO);
	intrEn = rtcMacroRead(RTC_INTR_ENABLE_MARCO);
	
	rtcMacroWrite(RTC_INTR_STATUS_MARCO, ~(SEC_INTR_STATUS | ALARM_INTR_STATUS)); 

	//printk(" ==> (rtsr & intrEn) = 0x%x \n",(rtsr & intrEn));
	return (rtsr & intrEn);
}
EXPORT_SYMBOL(gpHalRtcIntSrcGet);

/**
* @brief Enable Control for RTC Interrupt(Sec / Alarm).
* @param secIntEn[in] : Sec Interrupt enable (0 disable/1 enable/invalid operation).
* @param secIntEn[in] : Alarm Interrupt enable (0 disable/1 enable/invalid operation).
*/
void 
gpHalRtcIntEnable(
	UINT8 secIntEn, 
	UINT8 alarmIntEn
)
{
	UINT8 val = (UINT8)rtcMacroRead(RTC_INTR_ENABLE_MARCO);
	if (secIntEn == 1){
		val |= RTC_SEC_INT_ENABLE;
	}
	else if (secIntEn == 0){
		val &= ~RTC_SEC_INT_ENABLE;
	}

	if (alarmIntEn == 1){
		val |= RTC_ALARM_INT_ENABLE;
	}
	else if (alarmIntEn == 0){
		val &= ~RTC_ALARM_INT_ENABLE;
	}

	rtcMacroWrite(RTC_INTR_ENABLE_MARCO, val);	
	
}
EXPORT_SYMBOL(gpHalRtcIntEnable);

/**
* @brief Get current RTC time value.
* @return : Time value. 
*/
UINT32 
gpHalRtcGetTime(
	UINT32 *pvalue/*void*/
)
{
	UINT32 valLow, valHigh;

	valLow = rtcMacroRead(RTC_TIMERCNT_7_0_MARCO);
	valLow |= (rtcMacroRead(RTC_TIMERCNT_15_8_MARCO) << 8);
	valLow |= (rtcMacroRead(RTC_TIMERCNT_23_16_MARCO) << 16);
	valLow |= (rtcMacroRead(RTC_TIMERCNT_31_24_MARCO) << 24);	
	valHigh =  rtcMacroRead(RTC_TIMERCNT_39_32_MARCO);
	valHigh |= (rtcMacroRead(RTC_TIMERCNT_47_40_MARCO) << 8);

	*pvalue = (valHigh << 16) | (valLow >> 16);
	/*rtc clk =32k , sec = 32k count*/
	return 0;   
}
EXPORT_SYMBOL(gpHalRtcGetTime);

/**
* @brief Set RTC time value.
* @param time[in] : Time value to set.
* @return : Real time value after set.
*/
UINT32 
gpHalRtcSetTime(
	UINT32 time
)
{
	UINT8 val;
	UINT32 valLow, valHigh;

	#if 1	
	val = (UINT8) rtcMacroRead(RTC_CTL_MARCO);
	val = val | CTL_LOADCNT_MUX/*CTL_WRITE_LOAD*/;
	rtcMacroWrite(RTC_CTL_MARCO, val);	
	#endif
	
	valLow = (time & 0xFFFF) << 16;
	rtcMacroWrite(RTC_LOADCNTBIT_7_0_MARCO, valLow & 0xFF);
	rtcMacroWrite(RTC_LOADCNTBIT_15_8_MARCO, (valLow >> 8) & 0xFF);
	rtcMacroWrite(RTC_LOADCNTBIT_23_16_MARCO, (valLow >> 16) & 0xFF);
	rtcMacroWrite(RTC_LOADCNTBIT_31_24_MARCO, (valLow >> 24) & 0xFF);
	
	valHigh = time >> 16;
	rtcMacroWrite(RTC_LOADCNTBIT_39_32_MARCO, valHigh & 0xFF);
	rtcMacroWrite(RTC_LOADCNTBIT_47_40_MARCO, (valHigh >> 8) & 0xFF);

	/*loading*/
	rtcMacroWrite(RTC_LOAD_START_VALUE_MARCO, 1);

	return 0;//gpHalRtcGetTime();
}
EXPORT_SYMBOL(gpHalRtcSetTime);

/**
* @brief Get RTC Alarm Interrupt settings.
* @param enable[out] : Alarm Interrupt enable (0 disable/1 enable).
* @param pending[out] : Alarm Interrupt pending (0 not pending/1 pending).
* @param time[out] : Alarm time value.
*/
UINT32 
gpHalRtcGetAlarmStatus(
	UINT8 *enable, 
	UINT8 *pending, 
	UINT32 *time
)
{
	UINT32 valLow, valHigh;
#if 0
	valLow  = (UINT8) rtcMacroRead(RTC_ALARM_7_0_MARCO);
	valLow  |= ((UINT8) rtcMacroRead(RTC_ALARM_15_8_MARCO)) << 8;
	valLow  |= ((UINT8) rtcMacroRead(RTC_ALARM_23_16_MARCO)) << 16;
	valLow  |= ((UINT8) rtcMacroRead(RTC_ALARM_31_24_MARCO)) << 24;

	valHigh = (UINT8) rtcMacroRead(RTC_ALARM_39_32_MARCO);
	valHigh |= ((UINT8) rtcMacroRead(RTC_ALARM_47_40_MARCO) & 0x7F) << 8;
#else
	valLow  =  rtcMacroRead(RTC_ALARM_7_0_MARCO);
	valLow  |= rtcMacroRead(RTC_ALARM_15_8_MARCO) << 8;
	valLow  |= rtcMacroRead(RTC_ALARM_23_16_MARCO) << 16;
	valLow  |= rtcMacroRead(RTC_ALARM_31_24_MARCO) << 24;
	
	valHigh =  rtcMacroRead(RTC_ALARM_39_32_MARCO);
	valHigh |= (rtcMacroRead(RTC_ALARM_47_40_MARCO)) << 8;
#endif
	/*rtc clk =32k , sec = 32k count*/
	*time = (valHigh << 16) | (valLow >> 16);   
     	
	*enable = !!(rtcMacroRead(RTC_INTR_ENABLE_MARCO) & RTC_ALARM_INT_ENABLE);		

	return 0;
}
EXPORT_SYMBOL(gpHalRtcGetAlarmStatus);

/**
* @brief Set RTC Alarm Interrupt settings.
* @param enable[in] : Alarm Interrupt enable (0 disable/1 enable).
* @param pending[in] : Alarm Interrupt pending (0 not pending/1 pending).
* @param time[in] : Alarm time value.
*/
UINT32 
gpHalRtcSetAlarmStatus(
	UINT8 enable, 
	UINT8 pending, 
	UINT32 time
)
{
	UINT32 val;
	UINT32 valLow, valHigh;

	valLow = (time & 0xFFFF) << 16;
	rtcMacroWrite(RTC_ALARM_7_0_MARCO, valLow & 0xFF);
	rtcMacroWrite(RTC_ALARM_15_8_MARCO, (valLow >> 8) & 0xFF);
	rtcMacroWrite(RTC_ALARM_23_16_MARCO, (valLow >> 16) & 0xFF);
	rtcMacroWrite(RTC_ALARM_31_24_MARCO, (valLow >> 24) & 0xFF); 	
	
	valHigh = time >> 16;
	rtcMacroWrite(RTC_ALARM_39_32_MARCO, valHigh & 0xFF);
	rtcMacroWrite(RTC_ALARM_47_40_MARCO, (valHigh >> 8));		
	val = rtcMacroRead(RTC_INTR_ENABLE_MARCO);
	
	if (enable){
		val |= RTC_ALARM_INT_ENABLE;
	}
	else{
		val &= ~RTC_ALARM_INT_ENABLE;
	}
	
	rtcMacroWrite(RTC_INTR_ENABLE_MARCO, val);

	return 0;
}
EXPORT_SYMBOL(gpHalRtcSetAlarmStatus);

/**
* @brief RTC enable/disable Control, if enable, check reliable code first; if fail, reset RTC registers.
* @param enable[in] :(0 disable/1 enable).
*/
UINT32 
gpHalRtcEnable(
	UINT8 enable
)
{
	UINT8 val = 0;
	rtcReg_t *prtcReg = (rtcReg_t *)(LOGI_ADDR_RTC_REG);

	if (!enable) {
		//val = (UINT8) rtcMacroRead(RTC_CTL_MARCO);
		//rtcMacroWrite(RTC_CTL_MARCO, (~CTL_RTC_CLKEN) & val);
	} 
	else {
		 /* --- rtc config --- */
		prtcReg->rtcDiv = 0x1F;// 27M/32 hz
		prtcReg->rtcCtrl = RTC_MACRO_CLK_ENABLE;
		#if 0
		/*check rtc ctller ready*/
		while (!(prtcReg->rtcRdy & RTC_MARCO_READY)){
			;
		}
		#endif

		/*--- init rtc , trig hardware pulse ---*/
		rtcMacroWrite(RTC_CTL_MARCO, CTL_RTC_CLKEN |CTL_COUNT_UP);
		//rtcMacroWrite(RTC_CTL_MARCO, CTL_RTC_CLKEN |CTL_RTCRST | CTL_COUNT_UP);
		rtcMacroWrite(RTC_CTL_MARCO, CTL_RTC_CLKEN |CTL_COUNT_UP | CTL_LOADCNT_MUX);

		val = (UINT8)rtcMacroRead(RTC_RELIABLECODE_MARCO);
		DIAG_ERROR("rtc reliable code: 0x%02x\n", val);
		if (RELIABLE_CODE_CHECK_NUMBER != val) {
			DIAG_ERROR("reliable code not match, reset rtc [%s:%d]\n", __FUNCTION__, __LINE__);
			rtcMacroWrite(RTC_CTL_MARCO, CTL_RTC_CLKEN | CTL_COUNT_UP | CTL_LOADCNT_MUX);	
			rtcMacroWrite(RTC_CTL_MARCO,CTL_RTC_CLKEN | CTL_COUNT_UP /*| CTL_LOADCNT_MUX*/);			
			rtcMacroWrite(RTC_FDEN_MARCO, FD_ENABLE);		/*enable FD*/
			rtcMacroWrite(RTC_INTR_ENABLE_MARCO, 0x00);
			rtcMacroWrite(RTC_INTR_STATUS_MARCO,  ~RTC_INT_ENABLE_MASK);		/*clear intr status*/
			rtcMacroWrite(RTC_RELIABLECODE_MARCO, RELIABLE_CODE_CHECK_NUMBER);	/*reset reliable code value*/
		}
		else {
			 /* --- rtc core --- */
			rtcMacroWrite(RTC_CTL_MARCO, CTL_RTC_CLKEN | CTL_COUNT_UP | CTL_LOADCNT_MUX);
			rtcMacroWrite(RTC_CTL_MARCO,CTL_RTC_CLKEN | CTL_COUNT_UP /*| CTL_LOADCNT_MUX*/);
		}

	}

	return 0;
}
EXPORT_SYMBOL(gpHalRtcEnable);

/**
* @brief RTC enable/disable system clk source.
* @param enable[in] :(0 disable/1 enable).
*/
void
gpHalRtcClkEnable(
	SP_BOOL enable
)
{
#if 0
	scubReg_t *pScubReg = (scubReg_t *)LOGI_ADDR_SCU_B_REG;

	if (enable){
		//pScubReg->scubPeriClkEn |= SCU_B_PERI_RTC;
	}
	else{
		//pScubReg->scubPeriClkEn &= ~SCU_B_PERI_RTC;
	}
#endif	
}
EXPORT_SYMBOL(gpHalRtcClkEnable);


#if 1	
/**
* @breif RTC Dummy field Write function
* @param value[in] : value to save
*/
void
gpHalRtcDummyWrite(
	UINT16 value
)
{
	rtcMacroWrite(RTC_DUMMY_0, (UINT8)(value & 0xFF));
	rtcMacroWrite(RTC_DUMMY_1, (UINT8)(value >> 8));
}

/**
* @breif RTC Dummy field Read function
*/
UINT16
gpHalRtcDummyRead(
	void
)	
{
	UINT32 lvalue, hvalue;

	lvalue = rtcMacroRead(RTC_DUMMY_0);
	hvalue = rtcMacroRead(RTC_DUMMY_1);

	return (UINT16)(lvalue | (hvalue << 8));
}

#endif

