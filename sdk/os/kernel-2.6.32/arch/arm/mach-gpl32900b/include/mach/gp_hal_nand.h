/*2012-06-29 add for update the use code*/
#ifndef _GP_HAL_NAND_H
#define _GP_HAL_NAND_H
#define HAL_UPDATE_IOCTL_ENABLE		1

#if (HAL_UPDATE_IOCTL_ENABLE == 1)

#define HAL_NAND_SET_NAND_INFO			0
#define HAL_NAND_GET_NAND_INFO			1
#define HAL_NAND_GOOD_BLK_CHECK			10//2
#define HAL_NAND_ERASE_PHY_BLK			3
#define HAL_NAND_WRITE_PHY_PAGE			4
#define HAL_NAND_READ_PHY_PAGE			5
#define HAL_NAND_INIT					6
#define HAL_NAND_UNINIT					7
#define HAL_NAND_SET_BOOT_HEADER_INFO	8
#define HAL_NAND_SET_USRE_BAD_BLOCK		9
#define HAL_NAND_SET_BCH                11

typedef struct 
{
	unsigned int update_flag;
	unsigned int page_num;
	unsigned int physical_block;
	void *buffer;
}hal_nand_access_info;
#endif

#endif