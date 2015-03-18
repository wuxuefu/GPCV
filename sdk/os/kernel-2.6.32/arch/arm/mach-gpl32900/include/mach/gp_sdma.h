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
 * @file    gp_sdma.h
 * @brief   Declaration of SDMA base driver.
 * @author
 */

#ifndef _GP_SDMA_H_
#define _GP_SDMA_H_

/**************************************************************************
 *                         H E A D E R   F I L E S                        *
**************************************************************************/

/**************************************************************************
*                           C O N S T A N T S                             *
**************************************************************************/

/*
 * ioctl calls that are permitted to the /dev/sdma interface, if
 * any of the SD drivers are enabled.
 */
#define SDMA_IOCTL_ID           's'
#define SDMA_IOCTL_TRIGGER     _IOR(SDMA_IOCTL_ID, 0, gp_Sdma_t)
#define SDMA_IOCTL_MEMCPY_2D	_IOR(SDMA_IOCTL_ID, 1, gp_memcpy_2d_t)

/**************************************************************************
*                          D A T A    T Y P E S                           *
**************************************************************************/

typedef struct gpSdma_Chan_s {
	void* srcAdress;
	void* dstAdress;
	unsigned int size;
} gp_Sdma_t;

typedef struct gp_memcpy_2d_s {
	void* src;					/*!<Source address*/
	void* dst;					/*!<Destination address*/
	unsigned int width;			/*!<Blcok width, must small than 1MB*/
	unsigned int height;		/*!<Block height, must small than 1MB*/
	unsigned int src_width;		/*!<Source background image size*/
	unsigned int dst_width;		/*!<Destination background image size*/
} gp_memcpy_2d_t;

/**************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
**************************************************************************/

/**
* @brief 	SDMA memcpy function for kernel space.
* @param 	sdma [in]: SDMA parameter.
* @return: 	SUCCESS(0)/ERROR_ID
*/
extern int gp_sdma_memcpy_kernel(gp_Sdma_t* sdma );

#endif
