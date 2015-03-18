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
 * @file file_list.h
 * @brief fs service
 * @author zaimingmeng
 */

#ifndef _FILE_LIST_H
#define _FILE_LIST_H

#define FILE_NUM		2000
#define FILTER_NUM		20

#define GP_FILE_LOCK 1
#define GP_FILE_UNLOCK 0

typedef struct gp_filter_s{
	int 	count;
	char filter[FILTER_NUM][5];
}gp_filter_t;

typedef struct gp_fileList_s{
	int fileMap[FILE_NUM];
	unsigned int filePoolSize;
	unsigned int fileNum;
	unsigned int dirPoolSize;
	unsigned int dirNum;
	char dirEn;
	char fileAll;
	char numInit;
	char fileInit;
	char buildInit;
	char searchMode;
	char sortMode;
}gp_fileList_t;

typedef struct gp_filePool_s{
	char fileName[512];
	unsigned int hash;
	int lock; /*for lock/unlock file*/
}gp_filePool_t;

typedef struct gp_file_s {
	gp_fileList_t gFileList;
	gp_filePool_t *gFilePool;
	gp_filter_t gFilter;
	char gFilterBuffer[128];
	char gFilePath[512];
	char gDelFilePath[512];
}gp_file_t;

/*define sort mode*/
#define SORT_DEFAULT		0
#define SORT_FILE_TYPE		1

/*define search mode*/
#define SEARCH_DEFAULT		0
#define SEARCH_ALL_FILE		1
#define SEARCH_ALL_DIR		2


enum Delete_Status
{
	DELETE_INIT     = 0, 
	DELETE_ING,
	DELETE_OK,
	DELETE_ERROR,
};

/**
 *@brief reflash the file list
 *@param filePath [in] search filePath
 *@param filter [in] search file filter
 *@param searchMode [in] search mode
 *@param sort [in] sort mode
 *@param reflash [in] reflash or not
 */
gp_file_t *fileListReflash(gp_file_t *pFile, char *filePath, char *filter,  char searchMode,char sort,int reflash);


/**
 *@brief get file list
 *@param start [in] start index
 *@param end [in] end index
 *@param flag [in] 0:file and folder; 1:file; 2:folder
 */
void getFileList(gp_file_t *pFile, int start,int end,int flag);

/**
 *@brief get file list number
 *@param flag [in] 0:file and dir; 1:file; 2:dir
 */
int getFileListNum(gp_file_t *pFile, int flag);

/**
 *@brief get file list
 *@param path [in] search path
 *@param filter [in] search file filter
 *@param start [in] start index
 *@param end [in] end index
 *@param dirOrFile [in] 0:file; 1:dir
 */
void getFileDirect(gp_file_t *pFile, char *path, unsigned char *filter, int start, int end, int dirOrFile);

/**
 *@brief get file list
 *@param path [in] search path
 *@param filter [in] search file filter
 *@param flag [in] 0:file and dir; 1:file; 2:dir
 */
void getNumDirect(gp_file_t *pFile, char *path, unsigned char *filter, int flag);

/**
 *@brief get file info
 *@param path [in] file path
 */
void getFileInfo(char *path);

/**
 *@brief delete file
 *@param path [in] file path
 */
void fileDel(gp_file_t *pFile, char *path);

/**
 *@brief delete folder
 *@param path [in] folder path
 */
void dirDel(gp_file_t *pFile, char *path);

/**
 *@brief get delete dir status
 */
void gpGetDelStatus(void);

/**
 *@brief invalidate file list
 */
void fileInvalidate(gp_file_t *pFile);

char *getFile(gp_file_t *pFile, int index);
char *getPath(gp_file_t *pFile, int index);

int setFileLock(gp_file_t *pFile, int index, int lock);
int setFileAllLock(gp_file_t *pFile, int lock);
int getFileLock(gp_file_t *pFile, int index);
int gpFileListUnInit(gp_file_t *pFile);

#endif
