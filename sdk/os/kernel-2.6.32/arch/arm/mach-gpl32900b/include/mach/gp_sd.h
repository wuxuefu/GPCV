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
 * @file    gp_sd.h
 * @brief   Declaration of SD base driver.
 * @author  Dunker Chen
 */
 
#ifndef _GP_SD_H_
#define _GP_SD_H_

/**************************************************************************
 *                         H E A D E R   F I L E S                        *
 **************************************************************************/

#include <mach/gp_apbdma0.h>
#include <mach/gp_board.h>

/**************************************************************************
*                           C O N S T A N T S                             *
 **************************************************************************/

#define SD_NUM 			3
#define MAX_SD_PART		8

typedef enum gpSDType_e
{
	UNKNOWCARD	= 0,							/*!< @brief SD card type: unknown. */
	SDCARD 		= 1,							/*!< @brief SD card type: SD card. */
	MMCCARD 	= 2,							/*!< @brief SD card type: MMC card. */
	SDIO		= 3								/*!< @brief SD card type: SDIO card. */
}gpSDType_t;

typedef struct partition_s
{
 	unsigned short		partition_num;				/*!< @brief Partion_num */
 	unsigned short 		activity;        			/*!< @brief Activity partion */
 	unsigned int 		sn;							/*!< @brief Serial number, for usb usage */
 	unsigned long   	offset[MAX_SD_PART+1];		/*!< @brief Partition offsize, last partition is used for app area. */
 	unsigned int   		capacity[MAX_SD_PART+1];	/*!< @brief Partion capacity , last partition is used for app area. */
}
partition_t;

typedef struct gpSDInfo_s{
	unsigned char			device_id;			/*!< @brief Index of SD controller. */
	unsigned char			card_type;			/*!< @brief SD, MMC or SDIO. */
	unsigned char			cap_type;			/*!< @brief Standard or high capacity. */
	unsigned char			vsd;				/*!< @brief Reserved. */
	unsigned int			speed;				/*!< @brief SD speed (unit:100KHz). */
	unsigned int			RCA;				/*!< @brief SD relative card address. */
	unsigned int			capacity;			/*!< @brief Capacity (uint: sector ). */
	unsigned int			CID[4];				/*!< @brief SD card ID. */
	unsigned int			present;			/*!< @brief Card present (initial or not).*/
	spinlock_t				lock;               /*!< @brief For mutual exclusion. */
	spinlock_t				hal_lock;			/*!< @brief For hal dirver lock. */
	struct request_queue 	*queue;    			/*!< @brief The device request queue. */
	struct task_struct		*thread;			/*!< @brief Process thread. */
	struct semaphore		thread_sem;			/*!< @brief Thread semaphore. */
	struct scatterlist		*sg;				/*!< @brief SD scatter list. */
	struct request			*req;				/*!< @brief SD request. */
	struct gendisk 			*gd;				/*!< @brief The gendisk structure. */
	struct timer_list 		timer;        		/*!< @brief For simulated media changes. */
	struct work_struct		init;				/*!< @brief SD initial work queue. */
	struct work_struct		uninit;				/*!< @brief SD un-initial work queue. */
	gp_board_sd_t			*sd_func;			/*!< @brief Card detect, write protect, power function. */
	int 					pin_handle;			/*!< @brief Pin request handle */
	int 					handle_dma;			/*!< @brief Dma handle. */
	gpApbdma0Param_t		dma_param;			/*!< @brief Dma parameter. */
	short 					users;				/*!< @brief How many users. */
	unsigned short 			cnt;				/*!< @brief For detect io debounce. */
	unsigned int			status;				/*!< @brief For module status. Only use for block remove */
	unsigned int			fremove;			/*!< @brief For force remove. */		
	partition_t				partition;          /*!< @brief For partion information */
} gpSDInfo_t;

/*
 * ioctl calls that are permitted to the /dev/sdcarda or /dev/sdcardb interface, if
 * any of the SD drivers are enabled.
 */
#define SD_CLK_SET	_IOW('D', 0x01, unsigned int)		/*!< @brief Set SD clock. */
#define SD_CLK_BUS	_IOW('D', 0x02, unsigned int)		/*!< @brief Set SD bus width. */
#define SD_APP_SET	_IOW('D', 0x03, unsigned int)		/*!< @brief Set APP area. */

/*******************************************************************************
*                          D A T A    T Y P E S
*******************************************************************************/

/*******************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
*******************************************************************************/

/**
* @brief 	SD read sector in app area. (only used for fast boot)
* @param 	sd_num[in]: SD device number.
* @param 	lba[in]: Sector address.
* @param 	sector_num[in]: Sector number.
* @param 	buf_addr[in]: Buffer address.
* @return 	SUCCESS/ERROR_ID.
*/
extern int gp_sdcard_app_read_sector(unsigned int sd_num, unsigned int lba, unsigned short sector_num, unsigned int buf_addr);

/**
* @brief 	SD get serial number in app header. (only used for USB serial number)
* @param 	None.
* @return 	Serial number.
*/
extern unsigned int  gp_sdcard_get_sn(void);

#endif /* _GP_SD_H_ */