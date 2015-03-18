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
#ifndef _REG_APBDMA1_H_
#define _REG_APBDMA1_H_

#include <mach/hardware.h>                          
#include <mach/typedef.h>

#define	LOGI_ADDR_APBDMA1_REG		IO3_ADDRESS(0x010000)
#define	LOGI_ADDR_APBDMA1_CH0_REG	IO3_ADDRESS(0x010008)
#define	LOGI_ADDR_APBDMA1_CH1_REG	IO3_ADDRESS(0x01000C)
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
 
typedef struct apbdma1Reg_s {
	volatile UINT32 apbStatus;		/* P_APBDMA0_STATUS: 0x92B00000 */
	volatile UINT32 apbINT;			/* P_APBDMA0_INT: 0x92B09004 */
	volatile UINT32 rsv008[0x1D];	/* Reserved */ 
	volatile UINT32 apbRST;			/* P_APBDMA0_RESET: 0x92B0007C */
} apbdma1Reg_t;

typedef struct apbdma1chReg_s {
	volatile UINT32 apbchSAA;		/* P_APBDMA0_SA0A: 0x92B00008, P_APBDMA0_SA1A: 0x92B0000C, P_APBDMA0_SA2A: 0x92B00010, P_APBDMA0_SA3A: 0x92B00014 */
	volatile UINT32 rsv004[0x03];
	volatile UINT32 apbchEAA;		/* P_APBDMA0_EA0A: 0x92B00018, P_APBDMA0_EA1A: 0x92B0001C, P_APBDMA0_EA2A: 0x92B00020, P_APBDMA0_EA3A: 0x92B00024 */
	volatile UINT32 rsv014[0x03];
	volatile UINT32 apbchSA;		/* P_APBDMA0_SA0: 0x92B00028, P_APBDMA0_SA1: 0x92B0002C, P_APBDMA0_SA2: 0x92B00030, P_APBDMA0_SA3: 0x92B00034 */
	volatile UINT32 rsv024[0x08];
	volatile UINT32 apbchSAB;		/* P_APBDMA0_SA0B: 0x92B0004C, P_APBDMA0_SA1B: 0x92B00050, P_APBDMA0_SA2B: 0x92B00054, P_APBDMA0_SA3B: 0x92B00058 */
	volatile UINT32 rsv048[0x03];
	volatile UINT32 apbchEAB;		/* P_APBDMA0_EA0B: 0x92B0005C, P_APBDMA0_EA1B: 0x92B00060, P_APBDMA0_EA2B: 0x92B00064, P_APBDMA0_EA3B: 0x92B00068 */
	volatile UINT32 rsv058[0x03];
	volatile UINT32 apbchCtrl;		/* P_APBDMA0_CTRL0: 0x92B0006C, P_APBDMA0_CTRL1: 0x92B00070, P_APBDMA0_CTRL2: 0x92B00074, P_APBDMA0_CTRL3: 0x92B00078 */
}apbdma1chReg_t;

#define DMAX_BUSY_STATUS 0x00
#define DMAX_IRQ_STATUS  0x04
#define DMAX_AHB_SAXA    0x08  //0x08~0x14
#define DMAX_AHB_EAXA    0x18  //0x18~0x24
#define DMAX_APB_SAX     0x28  //0x28~0x34
#define DMAX_AHB_SAXB    0x4C  //0x4C~0x58
#define DMAX_AHB_EAXB    0x5C  //0x5C~0x68
#define DMAX_CRX         0x6C  //0x6C~0x78
#define DMAX_RST         0x7C


// APB DMA
#define DMA_M2P			0x00000000   //MIU to APB
#define DMA_P2M			0x00000001   //APB to MIU

#define DMA_AUTO		0x00000000   //AUTO ENABLE
#define DMA_REQ			0x00000002   //REQ ENABLE 

#define DMA_CON			0x00000000   //Address mode = CONTINUE
#define DMA_FIX			0x00000004   //Address mode = FIX

#define DMA_SINGLE_BUF	0x00000000   //Single Buffer
#define DMA_DOUBLE_BUF	0x00000008   //Nulti Buffer

#define DMA_8BIT		0x00000000   //8-bits single transfer
#define DMA_16BIT		0x00000010   //16-bits single transfer
#define DMA_32BIT		0x00000020   //32-bits single transfer
#define DMA_32BIT_BURST	0x00000030   //32-bits burst transfer

#define DMA_IRQOFF		0x00000000
#define DMA_IRQON		0x00000040

#define DMA_OFF			0x00000000
#define DMA_ON			0x00000080

#endif /* _REG_APBDMA0_H_ */