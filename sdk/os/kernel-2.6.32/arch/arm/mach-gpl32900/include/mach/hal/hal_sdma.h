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
 * @file    hal_sdma.h
 * @brief   Declaration of SDMA hal driver.
 * @author
 */

#ifndef _HAL_SDMA_H_
#define _HAL_SDMA_H_

/**************************************************************************
 *                         H E A D E R   F I L E S                        *
**************************************************************************/

#include <mach/typedef.h>
#include <mach/kernel.h>

/**************************************************************************
*                           C O N S T A N T S                             *
**************************************************************************/

/**************************************************************************
*                          D A T A    T Y P E S                           *
**************************************************************************/

typedef struct gpSdma_s {
	void* srcAddr;				/*!< @brief Source address */
	void* dstAddr;				/*!< @brief Destination address */
	UINT32 blockSize;			/*!< @brief Block size */
	UINT32 bStepSize;			/*!< @brief Source block step size */
	UINT32 frameSize;			/*!< @brief Frame size */
	UINT32 fStepSize;			/*!< @brief Source frame step size */
	UINT32 packetSize;			/*!< @brief Packet size */
	UINT32 pStepSize;			/*!< @brief Packet step size */
	UINT32 dbStepSize;			/*!< @brief Destination block step size */
	UINT32 dfStepSize;			/*!< @brief Destination frame step size */
	UINT8 useFlag;
} gpSdma_t;

/**************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
**************************************************************************/

/**
* @brief 		SDMA initial function.  
* @return		None.
*/
void gpHalSdmaInit(void);

/**
* @brief 		SDMA un-initial function.  
* @return		None.
*/
void gpHalSdmaUninit(void);

/**
* @brief 		SDMA hardware module reset.
* @return		None.
*/
extern void gpHalSdmaHWReset(void);

/**
* @brief 		SDMA channel reset (software reset).
* @param 		indexChan[in]: Channel Number.
* @return		None.
*/
extern void gpHalSdmaReset(UINT32 indexChan); 

/**
* @brief 		Dump SDMA channel register. (only for debug)
* @param 		indexChan[in]: Channel Number.
* @return		None.
*/
extern void gpHalDump(UINT32 indexChan); 

/**
* @brief 		SDMA channel status.
* @param 		indexChan[in]: Channel Number.
* @param 		statusBit[in]: Check Status.
* @return		Channel status.
*/
extern UINT32 gpHalCheckStatus(UINT32 indexChan, UINT32 statusBit); 

/**
* @brief 		SDMA channel irq status.
* @param 		indexChan[in]: Channel Number.
* @return		Irq status.
*/
extern UINT32 gpHalGetIrq(UINT32 indexChan);
 
/**
* @brief 		SDMA channel irq status.
* @param 		indexChan[in]: Channel Number.
* @return		Status.
*/
extern void gpHalClearIrq(UINT32 indexChan); 

/**
* @brief 		SDMA IRQ mask function.  
* @param 		indexChan[in]: Channel Number.
* @return		None.
*/
extern void gpHalMaskIrq(UINT32 indexChan); 

/**
* @brief 		SDMA channel start transfer extented function. (for one~three dimension transfer)
* @param 		indexChan[in]: Channel Number.
* @param 		pSdma[in]: Channel parameter.
* @return		None.
*/
extern void gpHalSdmaTrriger(UINT32 indexChan, gpSdma_t *pSdma); 

#endif
