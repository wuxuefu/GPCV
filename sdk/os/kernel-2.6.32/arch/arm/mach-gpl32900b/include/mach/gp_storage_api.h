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
 * @file    gp_storage_api.h
 * @brief   Declaration of Clock management driver
 * @author  Roger hsu
 * @date    2010-9-27
 */
#ifndef _GP_STORAGE_API_MGR_H_
#define _GP_STORAGE_API_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define RELOAD_BOOTLOADER	0

#define GPIO_IRQ_ACTIVE_BOTH    2

#define LOGO_MEMORY_SIZE 2097152 //2MB
#define GAMMA_MEMORY_SIZE 4*1024 //4KB
#define BOOTLOADER_RESERVED_SIZE	(LOGO_MEMORY_SIZE + GAMMA_MEMORY_SIZE)

#define SECTOR_SIZE				(512)
#define SECTOR_SHIFT_BIT		9

#define SD_MBR_OFFSET_SECTOR	2048
#define SD_PAGE_BYTE			2048
#define SD_BLOCK_BYTE			65536
#define SD_PAGE_SECTOR		(SD_PAGE_BYTE/SECTOR_SIZE)
#define SD_BLOCK_SECTOR	(SD_BLOCK_BYTE/SECTOR_SIZE)
#define NAND_WARP_IMAGE_BOOT_SECTOR		256
#define NAND_QUICK_IMAGE_BOOT_SECTOR	512
#define NAND_HIBERATE_IMAGE_BOOT_SECTOR	0x40000000
#define NAND_BOOT_OFFSET_SECTOR		128

//Boot header app attribute definition
#define APP_PART_TYPE_RESERVED			0
#define APP_PART_TYPE_BOOT_RESOURCE_BIN	1
#define APP_PART_TYPE_WARP_BIN			2
#define APP_PART_TYPE_QUICK_IMAGE		3
#define APP_PART_TYPE_UPDATER_BIN		4
#define APP_PART_TYPE_NORMAL_FW_BIN		5
#define APP_PART_TYPE_HIBERATE_IMAGE	6
#define APP_PART_TYPE_DATA_HEADER		7

// defgine for SRAM reserved area
#define SRAM_RESERVED_START		0xB0003000
#define SRAM_RESERVED_END		0xB000BFFF
#define SRAM_RESERVED_SIZE		(SRAM_RESERVED_END - SRAM_RESERVED_START + 1)
#define SRAM_RESERVED_ALIGN_MASK		0xFFF
#define SRAM_BOOT_HEADER_START	0xB000B000

// defgine for Bootlaoder area
#define DRAM_FASTAREA_START			0xC00F0000
#define DRAM_FASTAREA_END			0xC00FFFFF
#define DRAM_BOOTLOADER_START		0xC0100000
#define DRAM_BOOTLOADER_END			0xC01FFFFF
#define DRAM_BOOTLOADER_SIZE		(DRAM_BOOTLOADER_END - DRAM_FASTBIN_START + 1)
#define DRAM_FASTBOOT_API_SIZE		0x80
enum Device_table {
	DEVICE_NAND =0,
	DEVICE_NAND_PBA, //no bch
	DEVICE_NAND_SBLK, //Small block
	DEVICE_NAND_OTP, //OTP Rom (Nand interface)
	DEVICE_SD0,
	DEVICE_SPI, //serial flash
	DEVICE_SD1,
	DEVICE_AUTOSCAN,
	DEVICE_MAX
};
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
/* Ioctl for device node definition */
#define GP_STORAGE_API_MAGIC	'F'
#define IOCTL_GP_STORAGE_API_SET			_IOW(GP_STORAGE_API_MAGIC,1,unsigned int)
#define IOCTL_GP_STORAGE_API_ARM			_IOW(GP_STORAGE_API_MAGIC,2,unsigned int)
#define IOCTL_GP_STORAGE_API_CEVA			_IOW(GP_STORAGE_API_MAGIC,3,unsigned int)
#define IOCTL_GP_STORAGE_API_SYS			_IOW(GP_STORAGE_API_MAGIC,4,unsigned int)
#define IOCTL_GP_STORAGE_API_ENABLE		_IOW(GP_STORAGE_API_MAGIC,5,unsigned int)
#define IOCTL_GP_STORAGE_API_DISABLE		_IOW(GP_STORAGE_API_MAGIC,6,unsigned int)
#define IOCTL_GP_STORAGE_API_USAGE_DUMP	_IOW(GP_STORAGE_API_MAGIC,7,unsigned int)
#define IOCTL_GP_STORAGE_API_DUMP_ALL		_IOW(GP_STORAGE_API_MAGIC,8,unsigned int)
#define IOCTL_GP_STORAGE_API_SPLL_SEL		_IOW(GP_STORAGE_API_MAGIC,9,unsigned int)
#define IOCTL_GP_STORAGE_API_TEST		_IOW(GP_STORAGE_API_MAGIC,10,unsigned int)
#define IOCTL_GP_STORAGE_API_SIGNATURE_ERASE _IOW(GP_STORAGE_API_MAGIC,11,unsigned int)
#define IOCTL_GP_STORAGE_API_SRAM_TEST   _IOW(GP_STORAGE_API_MAGIC,12,unsigned int)
#define IOCTL_GP_STORAGE_API_UPDATE_BOOTHEADER   _IOW(GP_STORAGE_API_MAGIC,13,unsigned int)
#define IOCTL_GP_STORAGE_API_GET_SRAM_BOOTHEADER   _IOR(GP_STORAGE_API_MAGIC,14,uint32_t*)
#define IOCTL_GP_STORAGE_API_GET_USB_SERIALNUMBER   _IOR(GP_STORAGE_API_MAGIC,15,uint32_t*)


#define PWR_DOWN(RST,PZ,val) \
		{\
		 int tmp = 0;\
		 tmp = RST;\
         tmp |= val;\
		 RST = tmp;\
		 PZ = 0x2;\
		 PZ = 0x3;\
		}

#define PWR_UP(RST,PZ,msk) \
		{\
		 int tmp = 0;\
		 tmp = RST;\
		 PZ = 0x2;\
		 mdelay(2);\
		 PZ = 0x0;\
		 tmp &= (msk);\
		 RST = tmp;\
		}

#define PWR_DOWN_EN(x, RST, PZ, val) \
		if(x){\
			PWR_DOWN(RST,PZ,val);\
		}\
		else{\
			PWR_UP(RST,PZ,~val);\
		}

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
/** 
 * @brief storage_api handle sructure
 * @param : handle_state : 0 : release state, 1 : request state
 * @param : clodk_id : storage_api id information
 */
typedef struct gp_storage_api_handle_s {
	int 	handle_state;
	int 	storage_api_id;
} gp_storage_api_handle_t;

typedef struct _BootInfo_t
{
	unsigned long pwrc_cfg;		//PWRON1(USB), PWRON0(button)
	unsigned long vic1_rawSts;		//RTC status
	unsigned long keyscan4;
	unsigned long iotraps;
	unsigned long version;
	unsigned long UsbNo;		//for redboot only
	unsigned long PLL2_D1_Clock;	
}BootInfo_st;

typedef struct NandProfileHeader_st
{
	unsigned int		Signature;	//G8K3
	unsigned short	SecondHdrStartBlock;
	unsigned char 	SecondHdrBlkCopies;
	unsigned char 	SecondHdrPageCopies;
	
	unsigned char		NAND_ID_Parsing_type;
	unsigned char	  MAX_CS;		
	unsigned short  BadBlockNum;
	unsigned int		USB_SerialNumber;
	
	unsigned char		BtDevice;
	unsigned char		NandType;	
	unsigned char		ScanDramDisable;
	unsigned char	  nrmapPageTableLoader;	
	unsigned char 	BchType;
	unsigned char		addrCycle;	
	unsigned short	BlockNum;
	
	unsigned char		DRAM_Type;
	unsigned char		DRAMParaCnt;
	unsigned char		DRAMClk0;
	unsigned char		DRAMClk1;
	unsigned char		DRAMClk2;
	unsigned char		DRAMClk3;
	unsigned char		CPUClk;
	unsigned char		PreRegisterCount;	
	
	unsigned short	PagePerBlock;
	unsigned short	PagePerBlockWithGap;
	unsigned short	PageSize;
	unsigned short 	ReduntSize;
	
	unsigned int		ACwriteTiming;	
	unsigned int		ACReadTiming;
	
	unsigned char		Platform_Version;
	unsigned char 	DebugOption1;
	unsigned short	ScanRamStartBlock;	
	unsigned short	ScanRamDataSector;
	unsigned char	  ScanRamBlkCopies;
	unsigned char   ScanRamBlkDataCopy;
	
	unsigned int		ScanRamLoadAddress;	
	unsigned int		ScanRamExecAddress;
	
	unsigned char 	Romcode_Version;
	unsigned char 	RFU5;				
	unsigned short 	BtLdrStartBlock;
	unsigned short	BtLdrDataSector;
	unsigned char		BtLdrBlkCopies;
	unsigned char		BtLdrBlkDataCopy;
	
	unsigned int		BtLdrLoadAddress;	
	unsigned int		BtLdrExecAddress;	

	unsigned short  SmallBLStartBlock;
	unsigned short  SmallBLDataSector;
	unsigned char   SmallBLBlkCopies;
	unsigned char   SmallBLBlkDataCopy;
	unsigned char		DRAMSize;
	unsigned char   DRAM_rd_speed;
	
	unsigned int		SCUA_DDR_PHY_CTRL_1;
	unsigned int		SCUA_DDR_PHY_CTRL_2;
			
	unsigned short 	MRS;
	unsigned short 	EMRS;
	unsigned short	EMRS2;
	unsigned short 	EMRS3;			
			
	unsigned int		DRAM_AC_Config0;		
	unsigned int		DRAM_AC_Config1;
	
	unsigned char		DRAM_pin_mux;
	unsigned char		RFU31;
	unsigned char		RFU32;
	unsigned char		RFU33;
	unsigned short	DRAM_ref_period;
	unsigned char		DRAM_cl;
	unsigned char		DRAM_dly_dqs;
			
	unsigned char		DRAMPara0[48];	
	unsigned char		DRAMPara1[48];
	unsigned char		DRAMPara2[48];		
	unsigned char		DRAMPara3[48];	

	unsigned char		DRAM_r_cap_rd_sel_start;
	unsigned char		DRAM_r_cap_rd_sel_end;
	unsigned char		USER_Part5_Swap_nrBlocks;
	unsigned char		USER_Part4_Swap_nrBlocks;
	unsigned char		APP_Part11_Swap_nrBlocks;
	unsigned char		APP_Part10_Swap_nrBlocks;
	unsigned char		USER_Part3_Swap_nrBlocks;
	unsigned char		USER_Part2_Swap_nrBlocks;	
	
	unsigned char		APP_Part9_Swap_nrBlocks;
	unsigned char		APP_Part8_Swap_nrBlocks;
	unsigned char		APP_Part7_Swap_nrBlocks;
	unsigned char		APP_Part6_Swap_nrBlocks;	
	unsigned char		APP_Part5_Swap_nrBlocks;
	unsigned char		APP_Part4_Swap_nrBlocks;		
	unsigned char		APP_Part3_Swap_nrBlocks;
	unsigned char		APP_Part2_Swap_nrBlocks;			

	unsigned char 	APP_Part_Nums;
	unsigned char		User_Part_Nums;
	unsigned char		APP_FS_Part0_Attrib;
	unsigned char 	APP_FS_Part1_Attrib;
	unsigned char		APP_FS_Part2_Attrib;
	unsigned char 	APP_FS_Part3_Attrib;	
	unsigned char		APP_Part1_Swap_nrBlocks;
	unsigned char		APP_Part0_Swap_nrBlocks;			
	
	unsigned short	APP_Part0_StartBlock;
	unsigned short	APP_Part0_nrBlocks;
	unsigned short	APP_Part1_StartBlock;
	unsigned short	APP_Part1_nrBlocks;	
	
	unsigned short	APP_Part2_StartBlock;
	unsigned short	APP_Part2_nrBlocks;
	unsigned short	APP_Part3_StartBlock;
	unsigned short	APP_Part3_nrBlocks;		

	unsigned char 	User_FS_Part0_Attrib;
	unsigned char 	User_FS_Part1_Attrib;
	unsigned char 	User_FS_Part2_Attrib;
	unsigned char 	User_FS_Part3_Attrib;
	unsigned char 	User_FS_Part4_Attrib;
	unsigned char 	User_FS_Part5_Attrib;
	unsigned char		USER_Part1_Swap_nrBlocks;
	unsigned char  	USER_Part0_Swap_nrBlocks;

	unsigned short	User_Part0_StartBlock;
	unsigned short 	User_Part0_nrBlocks;
	unsigned short	User_Part1_StartBlock;
	unsigned short 	User_Part1_nrBlocks;	
	
	unsigned int		PreRegisters[64];

	unsigned char		RFU_9[40];	
	
	unsigned short	User_Part5_StartBlock;
	unsigned short 	User_Part5_nrBlocks;		
	unsigned short	User_Part4_StartBlock;
	unsigned short 	User_Part4_nrBlocks;	
	
	unsigned short	APP_Part11_StartBlock;
	unsigned short	APP_Part11_nrBlocks;
	unsigned short	APP_Part10_StartBlock;
	unsigned short	APP_Part10_nrBlocks;	
 
	unsigned short	User_Part3_StartBlock;
	unsigned short 	User_Part3_nrBlocks;	
	unsigned short	User_Part2_StartBlock;
	unsigned short 	User_Part2_nrBlocks;
		
	unsigned short	APP_Part9_StartBlock;
	unsigned short	APP_Part9_nrBlocks;
	unsigned short	APP_Part8_StartBlock;
	unsigned short	APP_Part8_nrBlocks;		
	
	unsigned short	APP_Part7_StartBlock;
	unsigned short	APP_Part7_nrBlocks;
	unsigned short	APP_Part6_StartBlock;
	unsigned short	APP_Part6_nrBlocks;		
	
	unsigned short	APP_Part5_StartBlock;
	unsigned short	APP_Part5_nrBlocks;
	unsigned short	APP_Part4_StartBlock;
	unsigned short	APP_Part4_nrBlocks;		
	
	unsigned char 	APP_FS_Part11_Attrib;
	unsigned char 	APP_FS_Part10_Attrib;
	unsigned char 	APP_FS_Part9_Attrib;
	unsigned char 	APP_FS_Part8_Attrib;
	unsigned char 	APP_FS_Part7_Attrib;
	unsigned char 	APP_FS_Part6_Attrib;
	unsigned char		APP_FS_Part5_Attrib;
	unsigned char  	APP_FS_Part4_Attrib;	
	
	unsigned int		DRAM_DQ_DELAY_1;
	unsigned int		DRAM_DQ_DELAY_2;
	
	unsigned int		DRAM_DM_DQS_DELAY;
	unsigned int		RFU10;
		
	unsigned char		CS0_BlockMappingTable_StartBlock;
	unsigned char		CS1_BlockMappingTable_StartBlock;
	unsigned char		CS2_BlockMappingTable_StartBlock;
	unsigned char		CS3_BlockMappingTable_StartBlock;
	unsigned char		NAND_RANDOM_EN;
	unsigned char		NAND_SEEDGEN_CTRL;
	unsigned short	NAND_PRBS_COE;
	
	unsigned short	NAND_PreNum0;
	unsigned short	NAND_PreNum1;
	unsigned short	NAND_PreNum2;
	unsigned short	NAND_PreNum3;
	
	unsigned short	NAND_PreNum4;
	unsigned short	NAND_PreNum5;
	unsigned short	NAND_PreNum6;
	unsigned short	NAND_PreNum7;	

	unsigned short	mapPageTableLoader[128];	
	
	unsigned int		EndSignature;
	unsigned int		CheckSum;
	
}pnandprofileheader_t, *ppnandprofileheader_t;

typedef int (*gpFastbootInit_t		)(void);
typedef int (*gpStorageReadStart_t	)(UINT8 partitionType, unsigned int indexSector, unsigned int wLen, unsigned char *buf); //sector operation
typedef int (*gpStorageReadEnd_t	)(void);
typedef int (*gpStorageWriteStart_t	)(UINT8 partitionType, unsigned int indexSector, unsigned int wLen, unsigned char *buf); //sector operation
typedef int (*gpStorageWriteEnd_t	)(void); //always return true now
typedef int (*gpStorageFlush_t		)(UINT8 partition_kind, UINT16 which_partition);
typedef int (*gpGetCancelKey_t		)(void);
typedef int (*gpFastbootEnd_t		)(void);
typedef int (*gpSetFastBootProgress_t	)(unsigned int percentage);
typedef int (*gpSpeedyEntry_t		)(void *param); //GP Speedy boot entry point
typedef int (*gpStorageInforGet_t	)(UINT8 partitionType, UINT16 *which_kind, UINT16 *which_partition,  unsigned int *nrSector); //sector operation

typedef struct gpStorageApiAddr_s {
	gpFastbootInit_t		 gpFastbootInit;			// init function before fastboot
	gpStorageReadStart_t	 gpStorageReadStart;
	gpStorageReadEnd_t	     gpStorageReadEnd;
	gpStorageWriteStart_t	 gpStorageWriteStart;
	gpStorageWriteEnd_t	     gpStorageWriteEnd;
	gpStorageFlush_t		 gpStorageFlush;
	gpGetCancelKey_t		 gpGetCancelKey;
	gpFastbootEnd_t		     gpFastbootEnd;
	unsigned int gpStorageReadBurstSector;		// unit of sector, for fast boot burst size 
	unsigned int gpStorageWriteBurstSector;		// unit of sector, for fast boot burst size 
	unsigned int bufferAddr;					// Buffer Address for snapshot write
	unsigned int bufferSize;					// size is max(readSize, writeSize) * 512 * 2.
	unsigned int gpStorageReadSizeSector;		// unit of sector
	unsigned int gpStorageWriteSizeSector;		// unit of sector
	unsigned int gpBootloaderBssStart;			// unit of byte
	unsigned int gpBootloaderBssSize;		// unit of byte
	gpSetFastBootProgress_t gpSetFastBootProgress; 
	gpSpeedyEntry_t			gpSpeedyEntry;
}gpStorageApiAddress_t;


typedef struct gpGenStorageApiAddr_s {
	//gpFastbootInit_t		 gpFastbootInit;			// init function before fastboot
	gpStorageReadStart_t	 gpStorageReadStart;
	gpStorageWriteStart_t	 gpStorageWriteStart;
	gpStorageFlush_t		 gpStorageFlush;
	gpStorageInforGet_t 	 gpStorageInforGet;
	UINT16 gpStorageWarpKindNum;
	UINT16 gpStorageWarpPartitionNum;
	UINT16 gpStorageQuickKindNum;
	UINT16 gpStorageQuickPartitionNum;
	UINT16 gpStorageHiberKindNum;
	UINT16 gpStorageHiberPartitionNum;
}gpGenStorageApiAddr_t;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
    
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
/**
 * @brief 	read binary interface
 * @param 	buf[in]: data buffer
 * @param 	size_sector[in]: binary size copy to buf, unit of sector
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_binary_read(int *buf, int size_sector);
/**
 * @brief 	get app area parameter
 * @param 	index[in]: index of snapshot image ==> 0:quick snapshot, 1:hiberation image1,  2:hiberation image2, ...
 * @param 	boot_offset[out]: offset sector of boot flasg store.
 * @param 	image_offset[out]: offset sector of image store. 
 * @param 	boot_size[out]: boot sector size
 * @param 	image_size[out]: image sector size  
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_parameter_get(int index, int *boot_offset, int *image_offset, int *boot_size, int *image_size);

/**
 * @brief 	Write buffer address to storage for bootloader
 * @param 	address[in]: buffer address
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_buffer_address_write(unsigned int address, unsigned int mem_size);

/**
 * @brief 	Loader boot loader from storage to dram, prevent 0xf0000-0x200000 been used in kernel
 * @param 	none
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_load_bootlaoder(void);

/**
 * @brief 	Erase signature of fastboot image. The bootloader 
 *  	   reads it for certification.
 * @param 	index[in]: index of snapshot image ==> 0:quick snapshot, 1:hiberation image1,  2:hiberation image2, ...
 * @return 	SUCCESS/FAIL.
 */
int gp_fastbot_signature_erase(int index);

unsigned char* gp_fastboot_storage_api_get(void);

#endif /* _GP_STORAGE_API_MGR_H_ */
