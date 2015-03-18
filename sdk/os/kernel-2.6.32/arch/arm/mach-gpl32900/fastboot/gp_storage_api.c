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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30077, Taiwan, R.O.C                                     *
 *                                                                        *
 **************************************************************************/
/**
 * @file    gp_storage_api.h
 * @brief   GPlus Speedy Boot header definition
 * @author  Roger hsu
 * @date    2012-10-24
 */
 
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/diag.h>
#include <mach/cdev.h>
#include <mach/gp_storage_api.h>
#include <mach/gp_board.h>
/**************************************************************************
 *                          C O N S T A N T S                          *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

 typedef struct gp_storage_api_s {
	struct miscdevice dev;     				
	spinlock_t lock;
	struct semaphore sem;
	int open_count;
} gp_storage_api_t;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
#define SD_MAIN_STORAGE	2
static gp_storage_api_t *gp_storage_api_info = NULL;
#if RELOAD_BOOTLOADER
static unsigned char *btl_reserved = NULL;
#endif /* RELOAD_BOOTLOADER */
static unsigned int bootDevId = 0;
gpGenStorageApiAddr_t gpGenStorageApi={0};
static char *gp_static_pBuf = NULL;
static int gp_sd_num = 0;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
 #if 0
//extern int gp_sdcard_app_read_sector(unsigned int sd_num, unsigned int lba, unsigned short sector_num, unsigned int buf_addr);
//extern int gp_sdcard_app_write_sector(unsigned int sd_num, unsigned int lba, unsigned short sector_num, unsigned int buf_addr);
extern SINT32 nand_read_user_partition(UINT8 partition_kind, UINT16 which_partition, UINT32 start_sect, UINT32 nr_sector, void *data_buf, UINT32 *p_ret_sector);
extern SINT32 nand_write_user_partition(UINT8 partition_kind, UINT16 which_partition, UINT32 start_sect, UINT32 nr_sector, void *data_buf, UINT32 *p_ret_sector);
extern SINT32 nand_search_user_partition_by_userbyte( uint16_t user_byte, uint16_t* which_kind, uint16_t* which_partition, uint32_t* sector_size);
extern SINT32 hal_nand_user_partition_get_nr_sector(UINT16 partition_kind, UINT16 which_partition, UINT32 *p_ret_nr_sector);
#endif
extern SINT32 nand_flush_user_partition(UINT8 partition_kind, UINT16 which_partition);
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int gpScanWarpBin(UINT8 partitionType, int *boot_offset, int *image_offset, int *boot_size, int *image_size, 
	UINT16 *partition_kind, UINT16 *which_partition);
#if 1 /* memory debug function for debug */
#define DUMP_PRINT    printk
#define DUMPLIST    16
void dumpbuftest(unsigned char *buf,int size)
{
    int i,j;
    int round;
    unsigned char *ptr = buf;

    DUMP_PRINT(" buf=%x,size=%d\n",(unsigned int)buf,size);
    if ((size&(DUMPLIST-1)) != 0)
        round = size/DUMPLIST+1;
    else
        round = size/DUMPLIST;

    /* DUMP_PRINT("ptr(%x)=%2x\n",ptr,*ptr++); */
    DUMP_PRINT("EXEC round=%d\n",round);
#if 1
    for (j=0;j < round;j++)
    {
        DUMP_PRINT(" %3x | ",j);
        for(i=0; i< DUMPLIST;i++)
            DUMP_PRINT("%02x;",*ptr++);
        DUMP_PRINT("\n");
    }
#endif
}
#endif
EXPORT_SYMBOL(dumpbuftest);

int getBootDevID (void)
{
	BootInfo_st *gpbootInfo_s;
	unsigned int  bootdevid;

	gpbootInfo_s =  (BootInfo_st *) ioremap(0xA0007FE0, 0x20);
	bootdevid = (gpbootInfo_s->iotraps>>1) & 0x07;

	iounmap(gpbootInfo_s);
	return bootdevid;
}
EXPORT_SYMBOL(getBootDevID);

//Todo: Fixme get define from header
typedef struct nand_user_partition_s {
	UINT32 data_size; // sector unit
	UINT32 partition_size; // sector unit
	UINT8	user_byte;
	UINT8 reserve; 
	UINT16 attri;	// refer user_partition_attr_enum
} nand_user_partition_t;


/**
 * @brief 	Write buffer address to storage for bootloader
 * @param 	address[in]: buffer address
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_buffer_address_write(unsigned int address, unsigned int mem_size)
{
	
	int ret = 0;
	int boot_offset, image_offset, boot_size, image_size;
	char *pBuf = kzalloc( 512, GFP_KERNEL);
	unsigned int *buf_address = (unsigned int *)pBuf;
	UINT16 partition_kind; // application kind
	UINT16 which_partition; // partition 0
	//unsigned int ret_nr_sector = 0;
	
	ret = gpScanWarpBin( APP_PART_TYPE_QUICK_IMAGE, &boot_offset, &image_offset, &boot_size, &image_size, &partition_kind, &which_partition);
	if( ret != 0) {
		printk("[%s]Scan Boot Header Fail[%x]\n", __FUNCTION__, ret);
		ret = 1;
		goto __SIGNATURE_ERASE_FAIL;
	}
	else{
		//assign addres to first int of pBuf
		*buf_address = address;
		*(buf_address+1) = mem_size;
		#if 1
		if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
			ret = gpGenStorageApi.gpStorageWriteStart( gp_sd_num, boot_offset + SD_MBR_OFFSET_SECTOR + 1, 1, (unsigned char*)pBuf);
		}
		else{
			ret = gpGenStorageApi.gpStorageWriteStart( APP_PART_TYPE_QUICK_IMAGE, 1, 1, (unsigned char*)pBuf);
		}
		#else
		if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
			//ret = gp_sdcard_app_write_sector(0, boot_offset + SD_MBR_OFFSET_SECTOR + 1, 1, (unsigned int)pBuf);
		}
		else{
			ret = nand_write_user_partition(partition_kind, which_partition, 1, 1, (void *)pBuf, &ret_nr_sector);
			memset(pBuf, 0, 4);
			printk("[%s][%d] NAND. buf_address=[0x%x]\n", __FUNCTION__, __LINE__, *(unsigned int *)buf_address);
			ret = nand_read_user_partition(partition_kind, which_partition, 1, 1, (void *)pBuf, &ret_nr_sector);
		}
		#endif
		
		//Test
		//memset(pBuf, 0, 4);
		//ret = gp_sdcard_app_read_sector(0, boot_offset + SD_MBR_OFFSET_SECTOR + 1, 1, (unsigned int)pBuf);
		//printk("[%s][%d] buf_address=[0x%x]\n", __FUNCTION__, __LINE__, *(unsigned int *)buf_address);
	}
	ret = 0;
__SIGNATURE_ERASE_FAIL:
	kfree(pBuf);
	return ret;
}
EXPORT_SYMBOL(gp_fastbot_buffer_address_write);

#if 0
#define NAND_USER_PARTITION_KIND_APP							1
#define NAND_USER_PARTITION_KIND_DATA							2
extern SINT32 hal_nand_user_partition_get_info(UINT16 partition_kind, UINT16 which_partition, nand_user_partition_t *p_user_info);
extern SINT32 hal_nand_user_partition_get_nr(UINT16 partition_kind, UINT16 *p_nr);

static int nand_search_user_partition_by_userbyte( uint16_t user_byte, uint16_t* which_kind, uint16_t* which_partition, uint32_t* sector_size) {
	SINT32 ret = -1;
	UINT16 data_total;
	UINT16 app_total;
	UINT16 i;
	UINT32 nr_sector = 0;
	UINT32 partition_size = 0;
	nand_user_partition_t user_partition;

	//ret = nand_isp_get_user_partition_total(NAND_USER_PARTITION_KIND_APP, &app_total);
	ret = hal_nand_user_partition_get_nr(NAND_USER_PARTITION_KIND_APP, &app_total);
	printk("get app partition total[%d]ret[%d]\n", app_total, ret);
	if (ret != 0)
		return ret;
		
	//ret = nand_isp_get_user_partition_total(NAND_USER_PARTITION_KIND_DATA, &data_total);
	ret = hal_nand_user_partition_get_nr(NAND_USER_PARTITION_KIND_DATA, &data_total);
	printk("get data partition total[%d]ret[%d]\n", data_total, ret);
	if (ret != 0)
		return ret;	

	if (app_total == 0 && data_total == 0) {
		printk("NO PARTRITION FOUND !!!!\n");
		return ret;	
	}
	
	for( i=0; i<app_total; i++ ){
		//ret = nand_isp_get_user_partition_info(NAND_USER_PARTITION_KIND_APP, i, &user_partition);
		ret = hal_nand_user_partition_get_info(NAND_USER_PARTITION_KIND_APP, i, &user_partition);
		if( ret == 0 ){
			if( user_partition.user_byte == user_byte ){
				*which_kind = NAND_USER_PARTITION_KIND_APP;
				*which_partition = i;
				partition_size = user_partition.partition_size;
				printk("APP partition - part[%d]kind[%d]data_size[%d]part_size[%d]user[%d]reserve[%d]attri[%d]\n", *which_partition, *which_kind, user_partition.data_size, \
				user_partition.partition_size, user_partition.user_byte, user_partition.reserve, user_partition.attri);	
				//return 0;
				goto __GET_INFOR_SUCCESS;
			}
		}
	}
	for( i=0; i<data_total; i++ ){
		//ret = nand_isp_get_user_partition_info(NAND_USER_PARTITION_KIND_DATA, i, &user_partition);
		ret = hal_nand_user_partition_get_info(NAND_USER_PARTITION_KIND_DATA, i, &user_partition);
		if( ret == 0 ){
			if( user_partition.user_byte == user_byte ){
				*which_kind = NAND_USER_PARTITION_KIND_DATA;
				*which_partition = i;
				partition_size = user_partition.partition_size;
				printk("DATA partition - part[%d]kind[%d]data_size[%d]part_size[%d]user[%d]reserve[%d]attri[%d]\n", *which_partition, *which_kind, user_partition.data_size, \
				user_partition.partition_size, user_partition.user_byte, user_partition.reserve, user_partition.attri);	
				//return 0;
				goto __GET_INFOR_SUCCESS;
			}
		}
	}
	return ret;	
	
__GET_INFOR_SUCCESS:
	ret = hal_nand_user_partition_get_nr_sector(*partition_kind, *which_partition, &nr_sector);
	printk("This kind[%d] of partition[%d] has %d sector. data has %d sector ret[%x]\n", *partition_kind, *which_partition, partition_size, nr_sector, ret);
	*sector_size = ( nr_sector == 0 )? partition_size:nr_sector;
	if (nr_sector == 0 && partition_size == 0 ){
		printk ("[%d]NO OIMAGE FOUND!!!!\r\n", __LINE__);
		return -1;
	}
	return 0;
}
#endif
/**
 * @brief 	read binary interface
 * @param 	buf[in]: data buffer
 * @param 	size_sector[in]: binary size copy to buf, unit of sector
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_binary_read(int *buf, int size_sector)
{
	int ret = 0;
	int boot_offset, image_offset, boot_size, image_size;
	UINT16 partition_kind; // application kind
	UINT16 which_partition; // partition 0
	//unsigned int ret_nr_sector = 0;
	
	ret = gpScanWarpBin( APP_PART_TYPE_WARP_BIN, &boot_offset, &image_offset, &boot_size, &image_size, &partition_kind, &which_partition);
	if( ret == 0 ) {
		#if 1
		if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
			ret = gpGenStorageApi.gpStorageReadStart( gp_sd_num, boot_offset + SD_MBR_OFFSET_SECTOR, size_sector, (unsigned char*)buf);
		}
		else{
			ret = gpGenStorageApi.gpStorageReadStart( APP_PART_TYPE_WARP_BIN, 0, size_sector, (unsigned char*)buf);
		}
		#else
		if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
			ret = gp_sdcard_app_read_sector(0, boot_offset + SD_MBR_OFFSET_SECTOR, size_sector, (unsigned int)buf);
		}
		else{
			ret = nand_read_user_partition(partition_kind, which_partition, 0, size_sector, (void *)buf, &ret_nr_sector);	
		}
		#endif
		printk("[%s]ret[%x]bootoffset[%d][%d]buf0_32t[%x]\n", __FUNCTION__, ret, boot_offset, image_offset, buf[0]);
	}
	else{
		printk("[%s]Scan Boot Header Fail[%x]\n", __FUNCTION__, ret);
	}
	return ret;
}
EXPORT_SYMBOL(gp_fastbot_binary_read);

/**
 * @brief 	get app area parameter
 * @param 	index[in]: index of snapshot image ==> 0:quick snapshot, 1:hiberation image1,  2:hiberation image2, ...
 * @param 	boot_offset[out]: offset sector of boot flasg store.
 * @param 	image_offset[out]: offset sector of image store. 
 * @param 	boot_size[out]: boot sector size
 * @param 	image_size[out]: image sector size  
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_parameter_get(int index, int *boot_offset, int *image_offset, int *boot_size, int *image_size)
{
	char partitionType = 0;
	int ret = 0;
	UINT16 partition_kind; // application kind
	UINT16 which_partition; // partition 0
	
	if( index == 0 ) {
		partitionType = APP_PART_TYPE_QUICK_IMAGE;
	}
	else{
		partitionType = APP_PART_TYPE_HIBERATE_IMAGE;
	}

	ret = gpScanWarpBin( partitionType, boot_offset, image_offset, boot_size, image_size, &partition_kind, &which_partition);
	if( ret != 0) {
		printk("[%s]Scan Boot Header Fail[%x], index[%x]\n", __FUNCTION__, ret, index);
	}
	else{
		printk("[%s]Scan Boot Header OK. index[%x]Boot_offset[%x]image_offset[%x]boot_size[%x]image_size[%x]\n", 
			__FUNCTION__, index, *boot_offset, *image_offset, *boot_size, *image_size);
	}
	return ret;
}
EXPORT_SYMBOL(gp_fastbot_parameter_get);


/**
 * @brief 	Erase signature of fastboot image. The bootloader 
 *  	   reads it for certification.
 * @param 	index[in]: index of snapshot image ==> 0:quick snapshot, 1:hiberation image1,  2:hiberation image2, ...
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_signature_erase(int index)
{
	
	char partitionType = 0;
	int ret = 0;
	int boot_offset, image_offset, boot_size, image_size;
	char *pBuf = kmalloc( 512, GFP_KERNEL);
	unsigned int sdDevId = 0;
	UINT16 partition_kind; // application kind
	UINT16 which_partition; // partition 0
	unsigned int ret_nr_sector = 0;
	
	if( index == 0 ) {
		partitionType = APP_PART_TYPE_QUICK_IMAGE;
	}
	else{
		partitionType = APP_PART_TYPE_HIBERATE_IMAGE;
	}

	ret = gpScanWarpBin( partitionType, &boot_offset, &image_offset, &boot_size, &image_size, &partition_kind, &which_partition);
	if( ret != 0) {
		printk("[%s]Scan Partition Fail.PartitionType[%d]ret[%x]\n", __FUNCTION__, partitionType, ret);
		ret = 1;
		goto __SIGNATURE_ERASE_FAIL;
	}
	else{
		if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
			if (gpGenStorageApi.gpStorageReadStart)
				ret = gpGenStorageApi.gpStorageReadStart( gp_sd_num, boot_offset + SD_MBR_OFFSET_SECTOR, 1, (unsigned char*)pBuf);
			memset(pBuf, 0, 4);
			//ret = gp_sdcard_app_write_sector(0, boot_offset + SD_MBR_OFFSET_SECTOR, 1, (unsigned int)pBuf);
			if (gpGenStorageApi.gpStorageWriteStart)
				ret = gpGenStorageApi.gpStorageWriteStart( gp_sd_num, boot_offset + SD_MBR_OFFSET_SECTOR, 1, (unsigned char*)pBuf);
		}
		else{
			if (gpGenStorageApi.gpStorageReadStart)
				ret = gpGenStorageApi.gpStorageReadStart( APP_PART_TYPE_QUICK_IMAGE, 0, 1, (unsigned char*)pBuf);
			memset(pBuf, 0, 4);
			if (gpGenStorageApi.gpStorageWriteStart) {
				ret = gpGenStorageApi.gpStorageWriteStart( APP_PART_TYPE_QUICK_IMAGE, 0, 1, (unsigned char*)pBuf);
				gpGenStorageApi.gpStorageFlush(0, 0);
			}
		}
	}
__SIGNATURE_ERASE_FAIL:
	kfree(pBuf);
	return ret;
}
EXPORT_SYMBOL(gp_fastbot_signature_erase);

/**
* @brief 	Scan app header to determine warp bin location 
* @param 	partitionType[in]: which partiion to scan
* @param 	boot_offset[out]: boot start sector. Not include MBR
*  					 offset
* @param 	image_offset[out]: image start sector 
* @param 	boot_size[out]: boot sector size
* @param 	image_size[out]: image sector size 
* @return 	SUCCESS(0)/FAIL(1)
*/
//Todo: Fixme from refering header file
#define HAL_NAND_HEADER_FULL					0
// hal_nand_header_size_enum
#define HAL_NAND_HEADER_MAIN_SIZE			1024
#define HAL_NAND_HEADER_AUX_SIZE			768
#define HAL_NAND_HEADER_FULL_SIZE			(HAL_NAND_HEADER_MAIN_SIZE + HAL_NAND_HEADER_AUX_SIZE)

static int gpScanWarpBin(UINT8 partitionType, int *boot_offset, int *image_offset, int *boot_size, int *image_size, 
	UINT16 *partition_kind, UINT16 *which_partition)
{	
	pnandprofileheader_t *bootHeader = (pnandprofileheader_t *)gp_static_pBuf;
	uint16_t *pSectorOffset = NULL, *pSectorSize= NULL;

	unsigned char *ptr = &(bootHeader->APP_FS_Part0_Attrib);
	int i = 0, ret = 0;
	int sectorIndex = 0;
	UINT32 nr_sector = 0;
	
	*boot_offset = 0;
	*image_offset = 0;
	if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
		ret = gpGenStorageApi.gpStorageReadStart( gp_sd_num, SD_MBR_OFFSET_SECTOR, 2, (unsigned char*)bootHeader);
		//printk("Read Header, ret[%d]\n", ret);
		
		if( ret != 0 )
			goto __SCAN_FAIL;
		for (i = 0 ; i < 4 ; i++) {
			//printk ("[%s][%d] *ptr=0x%x,0x%x\n",__FUNCTION__, __LINE__, (int)ptr, *ptr);
			if ( *ptr++ == partitionType) {
				sectorIndex = i;
				pSectorOffset = &(bootHeader->APP_Part0_StartBlock);
				pSectorSize = &(bootHeader->APP_Part0_nrBlocks);
				goto __SCAN_SUCCESS;
			}
		}
		ptr = &(bootHeader->APP_FS_Part11_Attrib);
		for (i = 11 ; i > 3 ; i--) {
			//printk ("[%s][%d] partType=%x\n",__FUNCTION__, __LINE__,bootHeader->APP_FS_Part0_Attrib);
			if ( *ptr++ == partitionType) {
				//*index = i;
				sectorIndex = 11 - i;
				pSectorOffset = &(bootHeader->APP_Part11_StartBlock);
				pSectorSize = &(bootHeader->APP_Part11_nrBlocks);
				if (i < 10) {
					//For new boot header. 11, 10, [4], 9, 8, 7...
					pSectorOffset += 4;
					pSectorSize += 4;
				}
				goto __SCAN_SUCCESS;
			}
		}  
		goto __SCAN_FAIL;
	}
	else{
		if( gpGenStorageApi.gpStorageInforGet != NULL) {
			ret = gpGenStorageApi.gpStorageInforGet( partitionType, partition_kind, which_partition, &nr_sector ); 
		}
		else{
			printk ("[%d]FAIL storage infor is null\n", __LINE__);
			goto __SCAN_FAIL;
		}
		//ret = nandPartitionInforGet( partitionType, partition_kind, which_partition, &nr_sector );			
		if (ret != 0){
			printk ("[%d]FAIL READING OIMAGE!!!! ret[%d]\r\n", __LINE__, ret);
			goto __SCAN_FAIL;
		}
		else{
			printk("Partition[%d] has %d sectors. ret[%d]\n", *which_partition, nr_sector, ret);
			if( partitionType == APP_PART_TYPE_WARP_BIN ){
				gpGenStorageApi.gpStorageWarpKindNum = *partition_kind;
				gpGenStorageApi.gpStorageWarpPartitionNum = *which_partition;
			}
			else if( partitionType == APP_PART_TYPE_QUICK_IMAGE){
				gpGenStorageApi.gpStorageQuickKindNum = *partition_kind;
				gpGenStorageApi.gpStorageQuickPartitionNum = *which_partition;	
			}
			else if( partitionType == NAND_HIBERATE_IMAGE_BOOT_SECTOR){
				gpGenStorageApi.gpStorageHiberKindNum = *partition_kind;
				gpGenStorageApi.gpStorageHiberPartitionNum = *which_partition;	
			}
		}
	}

__SCAN_SUCCESS:
	if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
		*boot_offset = ((*(pSectorOffset + (2 * sectorIndex)))) << 7 ;
		*image_offset = ((*(pSectorOffset + (2 * sectorIndex))) << 7) + SD_BLOCK_SECTOR;
		*boot_size = SD_BLOCK_SECTOR;
		*image_size = ((*(pSectorSize + (2 * sectorIndex))) << 7) - SD_BLOCK_SECTOR;
	}         
	else{
		pnandprofileheader_t *bootHeader;
		int nand_block_size = 0;
		bootHeader =  (pnandprofileheader_t *) ioremap(SRAM_BOOT_HEADER_START, sizeof(pnandprofileheader_t));
		nand_block_size = (bootHeader->PagePerBlock*bootHeader->PageSize) >> SECTOR_SHIFT_BIT;
		iounmap(bootHeader);

		*boot_offset = (partitionType == APP_PART_TYPE_WARP_BIN)?NAND_WARP_IMAGE_BOOT_SECTOR:((partitionType == APP_PART_TYPE_QUICK_IMAGE)? NAND_QUICK_IMAGE_BOOT_SECTOR:NAND_HIBERATE_IMAGE_BOOT_SECTOR);
		//Quick Image Offset = 256, Hibernate Image Offset = 512. Bootloader should know it.
		*image_offset = *boot_offset + nand_block_size;
		*boot_size = nand_block_size;
		*image_size = nr_sector;
	}
	printk ("Get fast boot binary dev[%d] type[%d] at partition [%d], "
			  "bootSector[%d], imageSector[%d], bootSize[%d], imageSize[%d]\n", bootDevId, partitionType, i, *boot_offset, *image_offset, *boot_size, *image_size);
	return 0;	
	
__SCAN_FAIL:	
	return 1;

}


static int gp_fastbot_bootheader_write(char *pBuf)
{
	int ret = 0;
	
	if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
		ret = gpGenStorageApi.gpStorageWriteStart( gp_sd_num, SD_MBR_OFFSET_SECTOR, 2, (unsigned char*)pBuf);
	}
	else{
		printk("[%s][%d]NOT YET\n", __FUNCTION__, __LINE__);
		//ret = gpGenStorageApi.gpStorageReadStart( APP_PART_TYPE_WARP_BIN, 0, 2, (unsigned char*)pBuf);
	}
	//ret = gp_sdcard_app_write_sector(sdDevId, SD_MBR_OFFSET_SECTOR, 2, (unsigned int)pBuf);
	return ret;
}
EXPORT_SYMBOL(gp_fastbot_bootheader_write);

#if RELOAD_BOOTLOADER
/**
* @brief 	Scan app header to determine warp bin location 
* @param 	partitionType[in]: which partiion to scan
* @param 	boot_offset[out]: boot start sector. Not include MBR
*  					 offset
* @param 	image_offset[out]: image start sector 
* @param 	boot_size[out]: boot sector size
* @param 	image_size[out]: image sector size 
* @return 	SUCCESS(0)/FAIL(1)
*/
static int gp_get_bootloader(unsigned char *btldf_buf)
{
	char *pBuf = kmalloc( 1024, GFP_KERNEL);
	pnandprofileheader_t *bootHeader = (pnandprofileheader_t *)pBuf;
	int ret = 0;
	int btldr_offset_sector = 0;
	int btldr_size_page = 0;
	int i;
	int read_size_sector;

	if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
		ret = gpGenStorageApi.gpStorageReadStart( gp_sd_num, SD_MBR_OFFSET_SECTOR, 2, (unsigned char*)pBuf);
	}
	else{
		printk("[%s][%d]NOT YET\n", __FUNCTION__, __LINE__);
		//ret = gpGenStorageApi.gpStorageReadStart( APP_PART_TYPE_WARP_BIN, 0, 2, (unsigned char*)pBuf);
	}
	if (ret != 0 ) {
		printk("[%s][%d]storage read fail:[%x]\n", __FUNCTION__, __LINE__, ret);
		goto __BTL_END;
	}

	btldr_offset_sector = (bootHeader->BtLdrStartBlock << 7);
	btldr_size_page = bootHeader->BtLdrDataSector;
	//read size is half of page
	read_size_sector = bootHeader->PageSize >> 10;
	//printk("[%s][%d]1 addr=0x%x, size=0x%x\n", __FUNCTION__, __LINE__, bootHeader->BtLdrStartBlock, bootHeader->BtLdrDataSector);
	
	for ( i = 0 ; i < btldr_size_page ; i ++ ) {
		if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
			ret = gpGenStorageApi.gpStorageReadStart( gp_sd_num, btldr_offset_sector + SD_MBR_OFFSET_SECTOR + i*read_size_sector*2, read_size_sector, (unsigned char*)(btldf_buf + i*(read_size_sector<<9)));
		}
		else{
			printk("[%s][%d]NOT YET\n", __FUNCTION__, __LINE__);
			//ret = gpGenStorageApi.gpStorageReadStart( APP_PART_TYPE_WARP_BIN, 0, 2, (unsigned char*)pBuf);
		}
		if (ret != 0 ) {
			printk("[%s][%d]storage read fail:[%x]\n", __FUNCTION__, __LINE__, ret);
			goto __BTL_END;
		}
	}

__BTL_END:

	kfree(pBuf);
	return 0;
}

/**
 * @brief 	Loader boot loader from storage to dram, prevent 0xf0000-0x200000 been used in kernel
 * @param 	none
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_load_bootlaoder(void)
{
	int ret = 0;
	gpStorageApiAddress_t *storage_api;
	unsigned char *btl_bss_start;
	int btl_bss_size;

	printk("[%s][%d] bootDevId=0x%x\n", __FUNCTION__, __LINE__, bootDevId);
	if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1){
		//restore fastboot storage interface
		memcpy((void *)DRAM_FASTAREA_START, btl_reserved, DRAM_FASTBOOT_API_SIZE);
		storage_api = (gpStorageApiAddress_t *)(DRAM_FASTAREA_START);
	
		btl_bss_start = __va(storage_api->gpBootloaderBssStart);
		btl_bss_size = storage_api->gpBootloaderBssSize;
	
		//re-load bootlaoder
		ret = gp_get_bootloader((unsigned char *)DRAM_BOOTLOADER_START);
		if( ret != 0 ) {
			printk("[%s] load bootlaoder fail[%x]\n", __FUNCTION__, ret);
		}
	
		//restore bss
		memcpy(btl_bss_start, btl_reserved + DRAM_FASTBOOT_API_SIZE, btl_bss_size);
	}

	return ret;
}
EXPORT_SYMBOL(gp_fastbot_load_bootlaoder);

/**
 * @brief 	reserved bootloader bss and storage spi
 * @param 	none
 * @return 	SUCCESS/FAIL.
 */
static int gp_reserved_bootloader_bss(void)
{
	gpStorageApiAddress_t *storage_api = (gpStorageApiAddress_t *)(DRAM_FASTAREA_START);
	unsigned char *btl_bss_start = __va(storage_api->gpBootloaderBssStart);
	int btl_bss_size = storage_api->gpBootloaderBssSize;

	btl_reserved = kmalloc( btl_bss_size + DRAM_FASTBOOT_API_SIZE, GFP_KERNEL);
	if (btl_reserved == NULL) {
		printk("[%s]kmalloc fail : out of memory! (size:%08X)\n", __FUNCTION__, btl_bss_size+DRAM_FASTBOOT_API_SIZE);
		return -ENOMEM;
	}

	printk("[%s][%d]BSS addr=0x%x, size=0x%x\n", __FUNCTION__, __LINE__, storage_api->gpBootloaderBssStart, storage_api->gpBootloaderBssSize);

	memcpy(btl_reserved, (void *)DRAM_FASTAREA_START, DRAM_FASTBOOT_API_SIZE);
	memcpy(btl_reserved + DRAM_FASTBOOT_API_SIZE, btl_bss_start, btl_bss_size);
	
	//test
	//memset(DRAM_FASTAREA_START, 0xaa, DRAM_FASTBOOT_API_SIZE);
	//memset(btl_bss_start, 0xaa, btl_bss_size);
	//memset(DRAM_BOOTLOADER_START, 0xaa, storage_api->gpBootloaderBssStart);


	return 0;
}
#endif /* RELOAD_BOOTLOADER */

static void gp_gen_storage_api_init(gpGenStorageApiAddr_t *gpGenDeviceApi){
	printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	if(gpGenDeviceApi == NULL){
		printk("[%s][%d]point is null\n", __FUNCTION__, __LINE__);
		return;
	}
	//gpGenStorageApi.gpFastbootInit		= gpGenDeviceApi->gpFastbootInit;
	gpGenStorageApi.gpStorageReadStart	= gpGenDeviceApi->gpStorageReadStart;
	gpGenStorageApi.gpStorageWriteStart = gpGenDeviceApi->gpStorageWriteStart;
	gpGenStorageApi.gpStorageFlush	= gpGenDeviceApi->gpStorageFlush;
	gpGenStorageApi.gpStorageInforGet		= gpGenDeviceApi->gpStorageInforGet;
	gpGenStorageApi.gpStorageHiberPartitionNum = gpGenStorageApi.gpStorageHiberPartitionNum;
	printk("[%s][%d]\n", __FUNCTION__, __LINE__);  
}

unsigned char* gp_fastboot_storage_api_get(void){
	return  (unsigned char*)&gpGenStorageApi;
}
EXPORT_SYMBOL(gp_fastboot_storage_api_get);


static int gp_storage_api_fops_open(struct inode *inode, struct file *file)
{
	if (!gp_storage_api_info) {
		DIAG_ERROR("Driver not initial\n");
		return -ENXIO;
	}

	gp_storage_api_info->open_count++;

	return 0;
}

static int gp_storage_api_fops_release(struct inode *inode, struct file *file)
{
	if (gp_storage_api_info->open_count <= 0) {
		DIAG_ERROR("storage_api device already close\n");
		gp_storage_api_info->open_count = 0;
		return -ENXIO;
	}
	else {
		gp_storage_api_info->open_count -- ;
		return 0;
	}
}


static long gp_storage_api_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = -ENOTTY;
	uint32_t getTestId = 0;
	uint32_t partitionIndex = 0;
	uint32_t bootSector, imageSector, bootSize, imageSize;
	uint32_t bootHeaderCheckSum = 0, i = 0;
	int* pBuf;
	char* pMem;
	BootInfo_st bootInfor;
	pnandprofileheader_t *bootHeader = NULL;
	printk("[%s][%d]\n", __FUNCTION__, __LINE__);

	switch (cmd) {
	case IOCTL_GP_STORAGE_API_TEST:
		copy_from_user(&getTestId, (void __user*)arg, sizeof(uint32_t));
		if( getTestId == 0 ) {
			pBuf = kmalloc(64*1024, GFP_KERNEL);
			ret = gp_fastbot_binary_read(pBuf, 128); 
			//dumpbuftest(pBuf, 512);
			kfree(pBuf);
		}
		else{
			ret = gp_fastbot_parameter_get(0, &bootSector, &imageSector, &bootSize, &imageSize); 
		}
	break;
	case IOCTL_GP_STORAGE_API_SIGNATURE_ERASE:
		copy_from_user(&partitionIndex, (void __user*)arg, sizeof(uint32_t));
		//ret = gp_fastbot_signature_erase(partitionIndex);
		printk("[%s][%d], ret[%d]\n", __FUNCTION__, __LINE__, ret);
	break;
	case IOCTL_GP_STORAGE_API_SRAM_TEST:
		//copy_from_user(&partitionIndex, (void __user*)arg, sizeof(uint32_t));
		printk("[%s][%d]\n", __FUNCTION__, __LINE__);
		pMem = ioremap(SRAM_BOOT_HEADER_START, 0x800 + 384);
		printk("[%s][%d], [%x][%x][%x][%x]\n", __FUNCTION__, __LINE__, pMem[0], pMem[1], pMem[2], pMem[3]);
		memcpy(&bootInfor, &pMem[1024], sizeof(BootInfo_st));
		dumpbuftest(&pMem[1024], sizeof(BootInfo_st));
		printk("[%s][%d], [%x][%x][%x][%x], [%x][%x][%x][%x], [%x]\n", __FUNCTION__, __LINE__, 
			   pMem[0], pMem[1], pMem[2], pMem[3], pMem[12], pMem[13], pMem[14], pMem[15], (uint32_t)bootInfor.UsbNo);
		iounmap(pMem);
		ret = 0;
	break;
	case IOCTL_GP_STORAGE_API_UPDATE_BOOTHEADER:
		{
		printk("[%s][%d]\n", __FUNCTION__, __LINE__);
		pMem = ioremap(SRAM_BOOT_HEADER_START, 0x800 + 384);

		bootHeader = kmalloc( sizeof(pnandprofileheader_t), GFP_KERNEL);
		if (bootHeader == NULL) {
			printk("[IOCTL_GP_STORAGE_API_UPDATE_BOOTHEADER]kmalloc fail : out of memory! (size:%08X)\n", sizeof(pnandprofileheader_t));
			ret = -ENOMEM;
			break;
		}

		memcpy((char*)(&bootHeader), pMem, 1024);
		memcpy((char*)(&bootInfor), &pMem[1024], sizeof(BootInfo_st));
		dumpbuftest(&pMem[0], 1024);
		dumpbuftest(&pMem[1024], 32);
		//Update serial number
		bootHeader->USB_SerialNumber = bootInfor.UsbNo;
		printk("[%s][%d], [%x][%x][%x][%x], [%x][%x][%x][%x], [%x]\n", __FUNCTION__, __LINE__, 
			   pMem[0], pMem[1], pMem[2], pMem[3], pMem[12], pMem[13], pMem[14], pMem[15], (uint32_t)bootHeader->USB_SerialNumber);
		//Update boot header
		if( pMem[0] == 0x47 && pMem[1] == 0x38 && pMem[2] == 0x4b && pMem[3] == 0x33) {	   
			printk("\n\n[%s][%d], Write Boot Header\n\n\n", __FUNCTION__, __LINE__);
			for(i=0; i<1021; i++) {
				bootHeaderCheckSum += pMem[i];
			}
			printk("[%s][%d], [%x]\n", __FUNCTION__, __LINE__, bootHeaderCheckSum);
			bootHeader->CheckSum = bootHeaderCheckSum;
			//gp_fastbot_bootheader_write( bootHeader );
		}
		ret = 0;
		iounmap(pMem);
		}
	break;
	case IOCTL_GP_STORAGE_API_GET_USB_SERIALNUMBER:
		//printk("USB_SERIALNUMBER [%s][%d]\n", __FUNCTION__, __LINE__);
		pMem = ioremap(SRAM_BOOT_HEADER_START, 0x800 + 384);
		memcpy((char*)(&bootInfor), &pMem[1024], sizeof(BootInfo_st));
		copy_to_user ((void __user *) arg, (const void *) &bootInfor.UsbNo, sizeof(uint32_t));
		ret = 0;
		iounmap(pMem);
		//printk("[%s][%d], [[%x]\n", __FUNCTION__, __LINE__, 
		//	   (uint32_t)bootInfor.UsbNo);
	break;
	}
	return ret;
}

static void gp_storage_api_device_release(struct device *dev)
{
	return ;
}

#ifdef CONFIG_PM
static int gp_storage_api_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int gp_storage_api_resume(struct platform_device *pdev)
{
	return 0;
}

#else
#define gp_storage_api_suspend NULL
#define gp_storage_api_resume NULL
#endif

static struct file_operations gp_storage_api_fops = {
	.owner		= THIS_MODULE,
	.open		= gp_storage_api_fops_open,
	.release	= gp_storage_api_fops_release,
	.unlocked_ioctl = gp_storage_api_fops_ioctl,
};


static struct platform_device gp_storage_api_device = {
	.name	= "gp-storage-api",
	.id	= -1,
    .dev	= {
		.release = gp_storage_api_device_release,
    }
};

static struct platform_driver gp_storage_api_driver = {
	.driver		= {
		.name	= "gp-storage-api",
		.owner	= THIS_MODULE,
	},
	.suspend	= gp_storage_api_suspend,
	.resume		= gp_storage_api_resume,
};


/**
 * @brief   RTC driver init
 */
static int __init gp_storage_api_init(void)
{
	int ret = 0;
	struct gp_board_s *pConfig = gp_board_get_config("board", gp_board_t);
	gp_board_sd_t *pSdConfig = NULL;
	
	printk("[%s][%d] run\n",__FUNCTION__, __LINE__);
	gp_storage_api_info = kzalloc(sizeof(gp_storage_api_t),GFP_KERNEL);
	if ( NULL == gp_storage_api_info ) {
		return -ENOMEM;
	}	

	platform_device_register(&gp_storage_api_device);
	ret = platform_driver_register(&gp_storage_api_driver);
	if (ret) {
		DIAG_ERROR("%s: failed to add fastboot driver\n", __func__);
		goto INIT_FAIL;
	}


	sema_init(&gp_storage_api_info->sem, 1);

	/* register misc device */
	gp_storage_api_info->dev.name = "storage-api";
	gp_storage_api_info->dev.minor = MISC_DYNAMIC_MINOR;
	gp_storage_api_info->dev.fops  = &gp_storage_api_fops;
	ret = misc_register(&gp_storage_api_info->dev);
	if ( ret != 0 ) {
		DIAG_ERROR(KERN_ALERT "misc register fail\n");
		goto INIT_FAIL;
	}

#if 0 // milton
	//reserved bootloader bss area	
	if(pConfig->isp_mode_get != NULL){ 
		//It's ISP mode.
		printk("[%s][%d] ISP mode[%d]\n", __FUNCTION__, __LINE__, pConfig->isp_mode_get());		   
		if( pConfig->isp_mode_get() == 1 ){
			goto __NOT_RESERVE_BSS_AND_HIBER_FUN;
		}
	}
#endif // milton

	//get main storage sd function 
	pSdConfig = gp_board_get_config("sd0", gp_board_sd_t);
	if ( (pSdConfig != NULL) && (pSdConfig->get_func != NULL) && (pSdConfig->get_func() == SD_MAIN_STORAGE) ) {
		gp_sd_num = 0;
	}
	else {
		pSdConfig = gp_board_get_config("sd1", gp_board_sd_t);
		if ( (pSdConfig != NULL) && (pSdConfig->get_func != NULL) && (pSdConfig->get_func() == SD_MAIN_STORAGE) ) {
			gp_sd_num = 1;
		}
		else {
			pSdConfig = gp_board_get_config("sd2", gp_board_sd_t);
			if ( (pSdConfig != NULL) && (pSdConfig->get_func != NULL) && (pSdConfig->get_func() == SD_MAIN_STORAGE) ) {
					gp_sd_num = 2;
			}
		}
	}

#if 0 // milton 
	if(pConfig->fastboot_callback_func_get != NULL){ 
		gp_gen_storage_api_init((gpGenStorageApiAddr_t *)pConfig->fastboot_callback_func_get());
	}
	else {
		printk("[%s][%d]Error! Get Null Main Storage Structure. Check board_config.c\n", __FUNCTION__, __LINE__);
		ret = -EIO;
		goto INIT_FAIL;
	}
#endif // milton 

#if RELOAD_BOOTLOADER
	//It's normal mode.
	ret = gp_reserved_bootloader_bss();
	if ( ret != 0 ) {
		DIAG_ERROR(KERN_ALERT "reserve bootloader bss\n");
		goto INIT_FAIL;
	}
#endif /* RELOAD_BOOTLOADER */

__NOT_RESERVE_BSS_AND_HIBER_FUN:	
	bootDevId = getBootDevID();                                                  
	printk("[%s][%d] bootid[%d]\n", __FUNCTION__, __LINE__, bootDevId);          
	if(bootDevId == DEVICE_SD0 || bootDevId == DEVICE_SD1) {
		gp_static_pBuf = kmalloc( 1024+768, GFP_KERNEL);
		if (gp_static_pBuf == NULL) {
			printk("kmalloc fail : out of memory! (size:%08X)\n", 1024+768);
			ret = -ENOMEM;
			goto INIT_FAIL;
		}
	}

	return ret;
INIT_FAIL:
	if (gp_storage_api_info) 
		kfree(gp_storage_api_info);
	gp_storage_api_info = NULL;
	return ret;
}

/**
 * @brief   RTC driver exit
 */
static void __exit gp_storage_api_exit(void)
{
	platform_device_unregister(&gp_storage_api_device);
	platform_driver_unregister(&gp_storage_api_driver);

	misc_deregister(&gp_storage_api_info->dev);

	if (gp_storage_api_info) 
		kfree(gp_storage_api_info);
	gp_storage_api_info = NULL;

	if (gp_static_pBuf)	kfree(gp_static_pBuf);		  

	return;
}

module_init(gp_storage_api_init);
module_exit(gp_storage_api_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP Storage Interface for AppArea");
MODULE_LICENSE_GP;

