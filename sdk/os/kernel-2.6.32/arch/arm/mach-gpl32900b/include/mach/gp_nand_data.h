#ifndef __GP_NAND_NAND__
#define __GP_NAND_NAND__

#define NAND_FLUASH_ALL_BLK	1

#define NAND_DIRECT_WRITE 	3
#define NAND_DIRECT_READ   	4
#define NAND_DATA_SYNC		5

typedef struct
{
	void *buffer;				// ����buffer
	unsigned int start_sector;	// ��ʼsector
	unsigned int sector_cnt;	// sector����	
}Xsfer_arg1;

#endif