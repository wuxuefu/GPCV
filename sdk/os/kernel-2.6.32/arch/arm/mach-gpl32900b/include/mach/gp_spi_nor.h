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
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
 
/**
 * @file gp_spi_nor.h
 * @brief spi nor flash header interface 
 * @author Daniel Huang
 */
#ifndef _SPMP_SPI_NOR_H_
#define _SPMP_SPI_NOR_H_

#include <mach/typedef.h>

typedef struct gpSpiNorBuf_s
{
	unsigned int spi_addr;	 
	unsigned int buf_addr;
}gpSpiNorBuf_t;

/*Ioctl for device node define */
#define SPI_NOR_IOCTL_ID           					'S'
#define SPI_NOR_IOCTL_READ_ID					_IOR(SPI_NOR_IOCTL_ID, 2, int)       			/*!< @brief Read ID command */
#define SPI_NOR_IOCTL_ERASE_SECTOR			_IOR(SPI_NOR_IOCTL_ID, 3, int)      			/*!< @brief Erase sector command */
#define SPI_NOR_IOCTL_ERASE_CHIP				_IOR(SPI_NOR_IOCTL_ID, 4, int)       			/*!< @brief Erase whole chip command */
#define SPI_NOR_IOCTL_READ_PAGE				_IOR(SPI_NOR_IOCTL_ID, 5, gpSpiNorBuf_t)       /*!< @brief read page command */
#define SPI_NOR_IOCTL_WRITE_PAGE				_IOR(SPI_NOR_IOCTL_ID, 6, gpSpiNorBuf_t)       /*!< @brief write page command */


#endif
