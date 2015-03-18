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
 * @file file_listi.c
 * @brief file list
 * @author zaimingmeng
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include<pthread.h>
#include "mach/gp_chunkmem.h"
#include "file_list.h"

/*
static struct gp_fileList_s gFileList;
static struct gp_filePool_s *gFilePool = NULL;
static struct gp_filter_s gFilter;
static char gFilerBuffer[128] = {0};
static char gFilePath[512] = {0}; */

pthread_t pDeletHandle;
int gDeleteTotalNum;
int gDeleteCurNum;
int gDeleteStatus = 0;

extern char fileMode;


void 
failInfo(
	int errorId,
	char *msg
)
{
	printf("<rsp state=\"fail\"><return errorId=\"%d\" msg=\"%s\" /></rsp>", errorId, msg);
}

void 
reportTotalNum(
	int num
)
{
	printf("<rsp state=\"ok\"><return totalnum=\"%d\" /></rsp>\n", num);
}

void 
reportDir(
	char *path,
	char *name
)
{
	printf("<file filepath=\"%s\" filename=\"%s\" attr=\"D\" />\n", path, name);
}

void 
reportFile(
	char *path,
	char *name
)
{
	printf("<file filepath=\"%s\" filename=\"%s\" attr=\"F\" />\n", path, name);
}

/*
 * for mach file name.
 * pa: a file,
 * pb: b file.
 * return: <0: not match >0:a file has M2 , 0: b file has M2
 */
int FileNameMatch(char *pa, char *pb)
{
	char *pa2, *pb2;
	int isPa = 0;

	if(!pa||!pb) {
		return -1;
	}

	pa2 = strstr(pa, "MB");
	if(pa2) {
		isPa = 1;
		pb2 = strstr(pb, "MA");
		if(pb2) {
			pa2 += 2;
			pb2 += 2;
		}
		else {
			return -1;
		}
	}
	else {
		pb2 = strstr(pb, "MB");
		pa2 = strstr(pa, "MA");
		if(pa2&&pb2) {
			isPa = 0;
			pa2+=2;
			pb2+=2;
		}
		else {
			return -2;
		}
	}
	//printf("pa:%s\n", pa2);
	//printf("pb:%s\n", pb2);

	if(strcmp(pa2, pb2) == 0) {
		return isPa;
	}
	else {
		return -1;
	}

}

/**
 *@brief parse filter
 *@param filter [in] file filter
 */
static void 
parse_filter(
	gp_file_t *pFile,
	char *filter
)
{
	int i,j,k = 0;
	struct gp_filter_s *pFilter = &pFile->gFilter;
	struct gp_fileList_s *pFileList = &pFile->gFileList;

	if(filter == NULL){
		return;
	}

	if(strcmp(filter, "*") == 0){
		pFileList->fileAll = 1;
		return;
	}
	
	memset(pFilter, 0, sizeof(gp_filter_t));

	for(i=0; i < FILTER_NUM; i++){
		for(j = 0; j < 5; j++){
			if(filter[k] == '|'){
				k++;
				break;
			}
			else if(filter[k] == '\0'){
				return;
			}
			else{
				pFilter->filter[i][j] += filter[k];
				k++;
				pFilter->count++;
			}						
		}
	}
}

/**
 *@brief compare file filter
 *@param ext_name [in] file extern name
 *@return 0: false 1: ture
 */
static int 
cmp_filter(
	gp_file_t *pFile,
	char *ext_name
)
{
	int i;
	struct gp_filter_s *pFilter = &pFile->gFilter;
	
	if(ext_name == NULL){
		return 0;
	}

	if(!pFilter->count){
		return 1;
	}

	for(i = 0; i < pFilter->count; i++){
		if(strcasecmp(pFilter->filter[i], ext_name) == 0){
			return 1;
		}
	}
	return 0;
}

/**
 *@brief get file filter
 *@param filename [in] file name
 *@return file type
 */
static char *
GetFileType(
	char *filename
)
{
	char *pType = NULL;
	
	while(*filename)
	{
		if(*filename++ == '.')
		{
			pType = filename;
		}
	}
	return pType;
}

/**
 *@brief get file type
 *@param filename [in] file name
 *@return 0: false 1: true
 */
static int 
ConverFileToType(
	gp_file_t *pFile,
	char *filename
)
{
	char *filetype;
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	
	if(filename == NULL){
		return 0;
	}

	if(pFileList->fileAll){
		return 1;
	}
	
		
	filetype = GetFileType(filename);

	if(cmp_filter(pFile, filetype)){
		return 1;
	}

	return 0;
}

unsigned int calibrateHash(char *string)
{
	unsigned int temp = 0;
	char *pString = string;

	while(*pString){
		temp += *pString;
		pString++;
	}
	
	return temp;
}

unsigned int getHash(gp_file_t *pFile, int index)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;

	if(pFileList->fileMap[index] < 0){
		return 0;
	}

	return pFile->gFilePool[pFileList->fileMap[index]].hash;
}

/**
 *@brief get file path from the file pool
 *@param index [in] file index
 *@return file path
 */
char *getPath(gp_file_t *pFile, int index)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;

	if(pFileList->fileMap[index] < 0){
		return NULL;
	}

	return pFile->gFilePool[pFileList->fileMap[index]].fileName;
}
/**
 *@brief get file name from the file pool
 *@param index [in] file index
 *@return file name
 */
char *getFile(gp_file_t *pFile, int index)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;

	if(pFileList->fileMap[index] < 0){
		return NULL;
	}
	
	return (char *)(pFile->gFilePool[pFileList->fileMap[index]].fileName+ strlen(pFile->gFilePool[pFileList->fileMap[index]].fileName) + 1);
}

//just for compare
static gp_file_t *gpFile = NULL; 
int setCompareFile(gp_file_t *pFile)
{
	gpFile = pFile;
}

/**
 *@brief get file info
 *@param path [in] file path
 */
int FileNameCompare(const void *a, const void *b)
{
	int ret;
	char a_file[128];
	char b_file[128];
	char *pa = NULL;
	char *pb = NULL;
	char *p = NULL;
	int len = 0;

	a_file[0] = '\0';
	b_file[0] = '\0';

	if((*(int *)a) < 0){
		printf("%s:%d %s %s\n", __FUNCTION__, __LINE__, a, b);
		return 1;
	}
	else if((*(int *)b) < 0){
		printf("%s:%d %s %s\n", __FUNCTION__, __LINE__, a, b);
		return -1;
	}
	p = getPath(gpFile, *(int*)a);
	sprintf(a_file, "%s", p);
	len = strlen(a_file);
	if(len > 9) {
		pa = &a_file[len-9];
		strncpy(pa, &p[len-6], 5);
		strncpy(&pa[5], &p[len-9], 3);
	}
	else {
		pa = a_file;
	}
	strcat(a_file, getFile(gpFile, *(int*)a));

	p = getPath(gpFile, *(int*)b);
	sprintf(b_file, "%s", p);
	len = strlen(b_file);
	if(len > 9) {
		pb = &b_file[len-9];
		strncpy(pb, &p[len-6], 5);
		strncpy(&pb[5], &p[len-9], 3);
	}
	else {
		pb = b_file;
	}
	strcat(b_file, getFile(gpFile, *(int*)b));
	printf("a: %s\n", pa);
	printf("b: %s\n", pb);

	ret = strcasecmp((char*)pa, (char *)pb);
	printf("ret=%d\n", ret);
	return ret;

	//return strcasecmp((char *)(gFilePool[*(int *)a].fileName+ strlen(gFilePool[*(int *)a].fileName) + 1), (char *)(gFilePool[*(int *)b].fileName+ strlen(gFilePool[*(int *)b].fileName) + 1));
}

/**
 *@brief get file info
 *@param path [in] file path
 */
static int FileInfoCompare(const void *a, const void *b)
{
	int ret;
	struct stat a_info;
	struct stat b_info;
	struct tm ap;
   	struct tm bp;
	struct tm *a_p;
	struct tm *b_p;
	char a_file[512];
	char b_file[512];


	a_file[0] = '\0';
	b_file[0] = '\0';

	if((*(int *)a) < 0){
		//printf("%s:%d %s %s\n", __FUNCTION__, __LINE__, a, b);
		return 1;
	}
	else if((*(int *)b) < 0){
		//printf("%s:%d %s %s\n", __FUNCTION__, __LINE__, a, b);
		return -1;
	}
	//sprintf(a_file, "%s", getPath(gpFile, *(int*)a));
	//strcat(a_file, getFile(gpFile, *(int*)a));
	//sprintf(b_file, "%s", getPath(gpFile, *(int*)b));
	//strcat(b_file, getFile(gpFile, *(int*)b));
	sprintf(a_file,"%s%s", (char*)(gpFile->gFilePool[*(int*)a].fileName),(char *)(gpFile->gFilePool[*(int *)a].fileName+strlen(gpFile->gFilePool[*(int *)a].fileName) + 1));
	sprintf(b_file,"%s%s", (char*)(gpFile->gFilePool[*(int*)b].fileName),(char *)(gpFile->gFilePool[*(int *)b].fileName+strlen(gpFile->gFilePool[*(int *)b].fileName) + 1));
	//printf("a: %s\n", a_file);
	//printf("b: %s\n", b_file);
	ret = stat((char *)a_file, &a_info);
	if(ret == -1){
		printf("%s:%d %s %d %d\n", __FUNCTION__, __LINE__, a, b);
		return 1;
	}
	ret = stat((char *)b_file, &b_info);
	if(ret == -1){
		printf("%s:%d %s %d %d\n", __FUNCTION__, __LINE__, a, b);
		return -1;
	}

	localtime_r(&(a_info.st_mtime), &ap);
	localtime_r(&(b_info.st_mtime), &bp);
	a_p = &ap;
	b_p = &bp;

	ret = 0;

	int y = a_p->tm_year - b_p->tm_year;
	int m = a_p->tm_mon - b_p->tm_mon;
	int d = a_p->tm_mday - b_p->tm_mday;
	int h = a_p->tm_hour - b_p->tm_hour;
	int mi = a_p->tm_min - b_p->tm_min;
	int s = a_p->tm_sec - b_p->tm_sec;
	/*printf("%s->", a_file);
	printf("mode=\"%d\" ", S_ISDIR(a_info.st_mode));
	printf("year=\"%d\" ", a_p->tm_year+1900);
	printf("mon=\"%d\" ", a_p->tm_mon+1);
	printf("mday=\"%d\" ", a_p->tm_mday);
	printf("hour=\"%d\" ", a_p->tm_hour);
	printf("min=\"%d\" ", a_p->tm_min);
	printf("sec=\"%d\" \n", a_p->tm_sec);
	printf("%s->", b_file);
	printf("mode=\"%d\" ", S_ISDIR(b_info.st_mode));
	printf("year=\"%d\" ", b_p->tm_year+1900);
	printf("mon=\"%d\" ", b_p->tm_mon+1);
	printf("mday=\"%d\" ", b_p->tm_mday);
	printf("hour=\"%d\" ", b_p->tm_hour);
	printf("min=\"%d\" ", b_p->tm_min);
	printf("sec=\"%d\" ---->end\n", b_p->tm_sec);*/

	if(y) {
		//printf("return:y %d\n", y);
		return y;
	}
	else if(m) {
		//printf("return:m %d\n", m);
		return m;
	}
	else if(d) {
		//printf("return:d %d\n", d);
		return d;
	}
	else if(h) {
		//printf("return:h %d\n", h);
		return h;
	}
	else if(mi) {
		//printf("return:mi %d\n", mi);
		return mi;
	}
	else if(s) {
		if(!fileMode) {
			//printf("return:s %d\n", s);
			return s;
		}
#if 1
		ret = FileNameMatch(a_file, b_file);
		if(ret < 0) {
			return s;
		}
		else if(ret == 0) { //b_file is DCIM2
			return -1;
		}
		else { //a_file is DCIM2
			return 1;
		}
#endif
	}
	else if(fileMode){ //the same sec
#if 1
		ret = FileNameMatch(a_file, b_file);
		if(ret < 0) {
			return s;
		}
		else if(ret == 0) { //b_file is DCIM2
			return -1;
		}
		else { //a_file is DCIM2
			return 1;
		}
#endif
	}

	return -1;
		/*printf("<rsp state=\"ok\"><return ");
		printf("filesize=\"%d\" ", (int)info.st_size);
		printf("mode=\"%d\" ", S_ISDIR(info.st_mode));
		printf("year=\"%d\" ", p->tm_year+1900);
		printf("mon=\"%d\" ", p->tm_mon+1);
		printf("mday=\"%d\" ", p->tm_mday);
		printf("/></rsp>");*/

}

static int filterCompare(const void *a,const void *b)
{
	int ret = 0 ;
	char *aa;
	char *bb;

	if((*(int *)a) < 0){
		return 1;
	}
	else if((*(int *)b) < 0){
		return -1;
	}

	aa = GetFileType((char *)(gpFile->gFilePool[*(int *)a].fileName+ strlen(gpFile->gFilePool[*(int *)a].fileName) + 1));
	bb = GetFileType((char *)(gpFile->gFilePool[*(int *)b].fileName+ strlen(gpFile->gFilePool[*(int *)b].fileName) + 1));

	if((aa == NULL)&&(bb == NULL)){
		ret = 0;
	}
	else if(aa == NULL){
		return -1;
	}
	else if(bb == NULL){
		return 1;
	}

	if(aa&&bb){
		ret = strcasecmp(aa,bb);
	}

	if(ret == 0){
		return strcasecmp((char *)(gpFile->gFilePool[*(int *)a].fileName+ strlen(gpFile->gFilePool[*(int *)a].fileName) + 1),
	(char *)(gpFile->gFilePool[*(int *)b].fileName+ strlen(gpFile->gFilePool[*(int *)b].fileName) + 1));
	}
	else{
		return ret;
	}
}

int strCompare(const void *a,const void *b)
{

	if((*(int *)a) < 0){
		return 1;
	}
	else if((*(int *)b) < 0){
		return -1;
	}
	printf("-----------%s:%d----------\n", __FUNCTION__, __LINE__);
	
	return strcasecmp((char *)(gpFile->gFilePool[*(int *)a].fileName+ strlen(gpFile->gFilePool[*(int *)a].fileName) + 1),
	(char *)(gpFile->gFilePool[*(int *)b].fileName+ strlen(gpFile->gFilePool[*(int *)b].fileName) + 1));
}

/**
 *@brief filelist init
 *@return -1: error; 0 :ok
 */
gp_file_t *gpFileListInit( gp_file_t *pF )
{
	int i;
	int handle;
	gp_file_t *pFile;

	printf("%s:%d 0x%x pF\n", __FUNCTION__, __LINE__, pF);
	if(pF == NULL) {
	printf("%s:%d\n", __FUNCTION__, __LINE__);
		pF = pFile = (gp_file_t *)malloc(sizeof(gp_file_t));
	}
	else {
	printf("%s:%d\n", __FUNCTION__, __LINE__);
		pFile = pF;
	}

	printf("%s:%d 0x%x\n", __FUNCTION__, __LINE__, pFile);
	memset(pFile, 0, sizeof(gp_file_t));

	if(pFile->gFilePool == NULL){
		handle = open("/dev/chunkmem", O_RDWR, O_SYNC);
	
		if (handle > 0){
			chunk_block_t priBlk;
			
			priBlk.size = sizeof(gp_filePool_t) * FILE_NUM;
			if(ioctl(handle, CHUNK_MEM_ALLOC, (unsigned long)&priBlk) >= 0)
			{
				pFile->gFilePool = (struct gp_filePool_s *)priBlk.addr;
			}		
		}
		close(handle);
	}

	if(pFile->gFilePool == NULL){
		failInfo(1, "alloc file pool error");
		fprintf(stderr, "alloc file pool error\n");
		free(pFile);
		pFile = NULL;
		return NULL;
	}

	memset(pFile->gFilePool, 0 ,sizeof(gp_filePool_t) * FILE_NUM);
	memset(&pFile->gFileList, 0, sizeof(gp_fileList_t));
	memset(&pFile->gFilter, 0, sizeof(gp_filter_t));
	memset(pFile->gFilterBuffer, 0, sizeof(pFile->gFilterBuffer));
	memset(pFile->gFilePath, 0, sizeof(pFile->gFilePath));
	memset(pFile->gDelFilePath, 0, sizeof(pFile->gDelFilePath));

	for(i = 0; i < FILE_NUM; i++){
		pFile->gFileList.fileMap[i] = i;
	}
	printf("%s:%d 0x%x\n", __FUNCTION__, __LINE__, pFile);
	return pFile;
}

int gpFileListUnInit(gp_file_t *pFile)
{
	int handle;
	if(pFile->gFilePool){
		handle = open("/dev/chunkmem", O_RDWR, O_SYNC);
	
		if (handle > 0){
			chunk_block_t priBlk;
			
			priBlk.addr = pFile->gFilePool;
			if(ioctl(handle, CHUNK_MEM_FREE, (unsigned long)&priBlk) >= 0)
			{
				printf("filelist free chunkmem ok!\n");
			}		
		}
		close(handle);
	}

	free(pFile);
	return 0;
}

/**
 *@brief file number init
 *@return -1: error; 0 :ok
 */
int gpGetFileNum(gp_file_t *pFile, char *filePath)
{
	DIR *fd;
	struct dirent *dir;
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	
	fd = opendir(filePath);
	if(fd == NULL){
		fprintf(stderr, "open dir error\n");
		failInfo(2, "open dir error");
		return -1;
	}

	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}
		
		if(dir->d_type == DT_DIR){
			pFileList->dirNum++;
		}
		else if(ConverFileToType(pFile, dir->d_name)){
			pFileList->fileNum++;
		}	
	}
	closedir(fd);

	pFileList->numInit = 1;
	if(pFileList->dirNum > FILE_NUM){
		pFileList->dirNum = FILE_NUM;
		pFileList->fileNum = 0;
	}
	else if(pFileList->dirNum +pFileList->fileNum > FILE_NUM){
		pFileList->fileNum = FILE_NUM - pFileList->dirNum;
	}	

	pFileList->dirPoolSize = pFileList->dirNum;
	pFileList->filePoolSize = pFileList->fileNum;

	return 0;
}

/**
 *@brief search file to file pool
 *@return -1: error; 0 :ok
 */
int searchFileList(gp_file_t *pFile, char *filePath)
{
	DIR *fd;
	struct dirent *dir;
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	int dirIndex = 0;
	int fileIndex = 0;

	fd = opendir(filePath);
	if(fd == NULL){
		fprintf(stderr, "open dir error\n");
		failInfo(2, "open dir error");
		return -1;
	}


	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}

		if(dir->d_type == DT_DIR){
			strcpy(pFile->gFilePool[dirIndex].fileName, filePath);
			strcpy((char *)(pFile->gFilePool[dirIndex].fileName + strlen(filePath) + 1), dir->d_name);
			pFile->gFilePool[dirIndex].hash = calibrateHash(filePath) + calibrateHash(dir->d_name) + calibrateHash("/");
			dirIndex++;
		}
		else{
			if((fileIndex < pFileList->fileNum)&&(ConverFileToType(pFile, dir->d_name))){
				strcpy(pFile->gFilePool[fileIndex + pFileList->dirPoolSize].fileName, filePath);
				strcpy((char *)(pFile->gFilePool[fileIndex + pFileList->dirPoolSize].fileName + strlen(filePath) + 1), dir->d_name);
				pFile->gFilePool[fileIndex + pFileList->dirPoolSize].hash = calibrateHash(filePath) + calibrateHash(dir->d_name);
				fileIndex++;
			}
		}
	}
	closedir(fd);

	pFileList->fileInit = 1;
	return 0;

}

/**
 *@brief search all path file to file pool
 *@param path [in] search path
 *@param dirEn [in] 0: search all path file; 1:search all folder name
 *@return -1: error; 0:ok
 */
int searchAllFile(gp_file_t *pFile, char *path,int dirEn)
{
	DIR *fd;
	struct dirent *dir;
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	char buffer[512] = {0};
	
	FILE* fp = NULL;
	int ret=0, isdir=0;
	//printf("%s:%d %s\n", __FUNCTION__, __LINE__, path);
	
	fd = opendir(path);
	if(fd == NULL){
		return -1;
	}

	//printf("%s:%d\n", __FUNCTION__, __LINE__);
	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "/etc/chkdir.sh %s%s", path, dir->d_name);
		fp = popen(buffer, "r");
		memset(buffer, 0, sizeof(buffer));
		ret = fread(buffer, sizeof(char), sizeof(buffer), fp);
		pclose(fp);
		isdir = atoi(buffer);
		
		if(dirEn){
			//-if((dir->d_type == DT_DIR) && (pFileList->dirNum < FILE_NUM)){
			if((isdir) && (pFileList->dirNum < FILE_NUM)){
				strcpy(pFile->gFilePool[pFileList->dirNum].fileName, path);
				strcpy((char *)(pFile->gFilePool[pFileList->dirNum].fileName + strlen(path) + 1), dir->d_name);
				pFileList->dirNum++;

				/*search the sub dir*/
				sprintf(buffer,"%s%s/",path,dir->d_name);
				pFile->gFilePool[pFileList->dirNum].hash = calibrateHash(buffer);
				
				searchAllFile(pFile, buffer,dirEn);
			}
		}
		else if(pFileList->fileNum < FILE_NUM){	
		
			//-if(dir->d_type == DT_DIR){
			if(isdir){
				/*search the sub dir*/
				sprintf(buffer,"%s%s/",path,dir->d_name);
				searchAllFile(pFile, buffer,dirEn);
			}
			else if(ConverFileToType(pFile, dir->d_name)){
				strcpy(pFile->gFilePool[pFileList->fileNum].fileName, path);
				strcpy((char *)(pFile->gFilePool[pFileList->fileNum].fileName + strlen(path) + 1), dir->d_name);
				pFile->gFilePool[pFileList->fileNum].hash = calibrateHash(path) + calibrateHash(dir->d_name);
				struct stat sta;
				char buf[512];
				sprintf(buf, "%s/%s", path, dir->d_name);
				stat(buf, &sta);
				if(sta.st_mode&S_IWUSR) {
					pFile->gFilePool[pFileList->fileNum].lock = GP_FILE_UNLOCK;
				}
				else {
					pFile->gFilePool[pFileList->fileNum].lock = GP_FILE_LOCK;
				}
				pFileList->fileNum++;
			}
		}
	}
	//printf("%s:%d\n", __FUNCTION__, __LINE__);

	closedir(fd);
	return 0;

}

/**
 *@brief sort the file pool
 *@param path [in] search path
 *@param dirEn [in] 0: search all path file; 1:search all folder name
 *@return -1: error; 0:ok
 */
int sortFileList(gp_file_t *pFile, int sort)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;

	if(!pFileList->fileInit){
		fprintf(stderr, "pFileList not init\n");
		return -1;
	}

	gpFile = pFile;

	switch(sort){
		case SORT_FILE_TYPE:
			if(pFileList->dirNum > 1){
				qsort(pFileList->fileMap, pFileList->dirNum, sizeof(pFileList->fileMap[0]), strCompare);
			}
			if(pFileList->fileNum > 1){
				qsort(&pFileList->fileMap[pFileList->dirNum], pFileList->fileNum, sizeof(pFileList->fileMap[0]), filterCompare);
			}
			break;
			
		default:
			/*if(pFileList->dirNum > 1){
				qsort(pFileList->fileMap, pFileList->dirNum, sizeof(pFileList->fileMap[0]), strCompare);
			}
			if(pFileList->fileNum > 1){
				qsort(&pFileList->fileMap[pFileList->dirNum], pFileList->fileNum, sizeof(pFileList->fileMap[0]), strCompare);
			}*/
			if(pFileList->dirNum > 1){
				qsort(pFileList->fileMap, pFileList->dirNum, sizeof(pFileList->fileMap[0]), strCompare);
			}
			if(pFileList->fileNum > 1){
				qsort(&pFileList->fileMap[pFileList->dirNum], pFileList->fileNum, sizeof(pFileList->fileMap[0]), FileInfoCompare);
			}
			/*if(pFileList->dirNum > 1){
				qsort(pFileList->fileMap, pFileList->dirNum, sizeof(pFileList->fileMap[0]), FileNameCompare);
			}
			if(pFileList->fileNum > 1){
				qsort(&pFileList->fileMap[pFileList->dirNum], pFileList->fileNum, sizeof(pFileList->fileMap[0]), FileNameCompare);
			}*/
			break;
	}
	pFileList->buildInit = 1;
	gpFile = NULL;

	return 0;
}

/**
 *@brief get file list
 *@param start [in] start index
 *@param end [in] end index
 *@param flag [in] 0:file and folder; 1:file; 2:folder
 */
void getFileList(gp_file_t *pFile, int start,int end,int flag)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	int i = 0;
	int num = 0;

	if(!pFileList->buildInit){
		fprintf(stderr, "not build file list\n");
		failInfo(1, "error");
		return;
	}

	printf("<rsp state=\"ok\">");

	if(flag == 2){
		while(i < pFileList->dirPoolSize){
			if(pFileList->fileMap[i] >= 0){
				if(num > end){
					break;
				}
				else if(num >= start){
					reportDir(getPath(pFile, i), getFile(pFile, i));
				}
				num++;
			}
			i++;
		}
	}
	else if(flag == 1){
		i = pFileList->dirPoolSize;
		while(i < pFileList->dirPoolSize + pFileList->filePoolSize){
			if(pFileList->fileMap[i] >= 0){
				if(num > end){
					break;
				}
				else if(num >= start){
					//reportDir(getPath(pFile, i + pFileList->dirPoolSize), getFile(pFile, i + pFileList->dirPoolSize));
					reportFile(getPath(pFile, i), getFile(pFile, i));
				}
				num++;
			}
			i++;
		}
	}
	else{
		while(i < pFileList->dirPoolSize + pFileList->filePoolSize){
			if(pFileList->fileMap[i] >= 0){
				if(num > end){
					break;
				}
				else if(num >= start){
					if(i < pFileList->dirPoolSize){
						reportDir(getPath(pFile, i), getFile(pFile, i));
					}
					else{
						reportFile(getPath(pFile, i), getFile(pFile, i));
					}
				}
				num++;
			}
			i++;
		}
	}

	printf("</rsp>\n");	
	return;

}

/**
 *@brief get file list number
 *@param flag [in] 0:file and dir; 1:file; 2:dir
 */
int getFileListNum(gp_file_t *pFile, int flag)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	int number;
	
	switch(flag){
		case 1:
			reportTotalNum(pFileList->fileNum);
			number = pFileList->fileNum;
			break;

		case 2:
			reportTotalNum(pFileList->dirNum);
			number = pFileList->fileNum;
			break;

		default:
			reportTotalNum(pFileList->fileNum + pFileList->dirNum);
			number = pFileList->fileNum + pFileList->dirNum;
	}
	
	return number;
}


/**
 *@brief get file list
 *@param path [in] search path
 *@param filter [in] search file filter
 *@param start [in] start index
 *@param end [in] end index
 *@param dirOrFile [in] 0:file; 1:dir
 */
void getFileDirect(gp_file_t *pFile, char *path, unsigned char *filter, int start, int end, int dirOrFile)
{
	DIR *fd;
	struct dirent *dir;
	int file_count = 0;
	int file_all = 0;

	if((path == NULL)||(filter == NULL)){
		failInfo(1, "path or filter NULL");
		return;
	}

	if(strcmp((char *)filter,"*") == 0){
		file_all = 1;
	}
	else if(!dirOrFile){
		parse_filter(pFile, (char *)filter);
	}

	fd = opendir(path);
	if(fd == NULL){
		fprintf(stderr, "open dir error\n");
		failInfo(2, "open dir error");
		return;
	}

	printf("<rsp state=\"ok\">");
	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}

		if(file_count >= end){
				break;
		}

		if(dirOrFile){
			if(dir->d_type == DT_DIR){
				if(file_count >= start){
					reportDir(path, dir->d_name);
				}
				file_count++;
			}
		}
		else if(file_all){		/*sear all file*/
			if(dir->d_type != DT_DIR){
				if(file_count >= start){
					reportFile(path, dir->d_name);
				}
				file_count++;
			}
		}
		else{		/*sear filter file*/
			if((dir->d_type != DT_DIR)&&ConverFileToType(pFile, dir->d_name)){
				if(file_count >= start){
					reportFile(path, dir->d_name);
				}
				file_count++;
			}
		}
	}

	printf("</rsp>\n");

	closedir(fd);
	return;
}

/**
 *@brief get file list
 *@param path [in] search path
 *@param filter [in] search file filter
 *@param flag [in] 0:file and dir; 1:file; 2:dir
 */
void getNumDirect(gp_file_t *pFile, char *path, unsigned char *filter, int flag)
{
	DIR *fd;
	struct dirent *dir;
	int file_count = 0;

	if((path == NULL)||(filter == NULL)){
		failInfo(1, "path or filter NULL");
		return;
	}

	parse_filter(pFile, (char *)filter);
	
	fd = opendir(path);
	if(fd == NULL){
		fprintf(stderr, "open dir error\n");
		failInfo(2, "open dir error");
		return;
	}

	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}

		if(flag == 2){
			if(dir->d_type == DT_DIR){
				file_count++;
			}
		}
		else if(flag == 1){
			if((dir->d_type != DT_DIR)&&ConverFileToType(pFile, dir->d_name)){
				file_count++;
			}
		}
		else {
			if((dir->d_type == DT_DIR)||ConverFileToType(pFile, dir->d_name)){
				file_count++;
			}
		}
	}
	reportTotalNum(file_count);;

	closedir(fd);
	return;
}


/**
 *@brief reflash the file list
 *@param filePath [in] search filePath
 *@param filter [in] search file filter
 *@param searchMode [in] search mode
 *@param sort [in] sort mode
 *@param reflash [in] reflash or not
 */
gp_file_t *fileListReflash(gp_file_t *pFile, char *filePath, char *filter,  char searchMode,char sort,int reflash)
{
	char needReflash = 0;
	struct gp_fileList_s *pFileList = &pFile->gFileList;;
	int ret;
	char buffer[512] = {0};

	/*init file list now*/
	pFile = gpFileListInit(pFile);
	if(!pFile){
		return NULL;
	}


	pFileList = &pFile->gFileList;;

	if((strlen(pFile->gFilePath) == 0)||(strlen(pFile->gFilterBuffer) == 0)){
		needReflash = 1;
	}
	else if(strcmp(filePath, pFile->gFilePath)||strcmp(filter, pFile->gFilterBuffer)||(pFileList->searchMode != searchMode)||(!pFileList->buildInit)){
		needReflash = 1;
	}

	if(reflash != 0){
		needReflash = 1;
	}

	if(!needReflash){
		if(pFileList->sortMode != sort){
			pFileList->sortMode = sort;
			sortFileList(pFile, pFileList->sortMode);
		}
		return pFile;
	}

	strcpy(pFile->gFilePath, filePath);
	if(filter){
		strcpy(pFile->gFilterBuffer, filter);
	}
	pFileList->sortMode = sort;

	parse_filter(pFile, filter);

	switch(searchMode){
		case SEARCH_ALL_FILE:
			if(filePath[strlen(filePath) - 1] != '/'){
				sprintf(buffer, "%s/", filePath);
			}
			else{
				strcpy(buffer, filePath);
			}
			ret = searchAllFile(pFile, buffer, 0);
			
			if(ret < 0){
				fprintf(stderr, "open dir error\n");
				failInfo(2, "open dir error");
				gpFileListUnInit(pFile);
				return NULL;
			}
			pFileList->filePoolSize = pFileList->fileNum;
			pFileList->fileInit = 1;
			pFileList->searchMode = SEARCH_ALL_FILE;
			break;
			
		case SEARCH_ALL_DIR:
			if(filePath[strlen(filePath) - 1] != '/'){
				sprintf(buffer, "%s/", filePath);
			}
			else{
				strcpy(buffer, filePath);
			}
			ret = searchAllFile(pFile, buffer, 1);
			if(ret < 0){
				fprintf(stderr, "open dir error\n");
				failInfo(2, "open dir error");
				gpFileListUnInit(pFile);
				return NULL;
			}
			pFileList->dirPoolSize = pFileList->dirNum;
			pFileList->fileInit = 1;
			pFileList->searchMode = SEARCH_ALL_DIR;
			break;

		default:
			if(filePath[strlen(filePath) - 1] != '/'){
				sprintf(buffer, "%s/", filePath);
			}
			else{
				strcpy(buffer, filePath);
			}
			
			if(gpGetFileNum(pFile, buffer) < 0){
				gpFileListUnInit(pFile);
				return NULL;
			}
			if(searchFileList(pFile, buffer) < 0){
				gpFileListUnInit(pFile);
				return NULL;
			}
			pFileList->searchMode = SEARCH_DEFAULT;
			break;
			
	}

	sortFileList(pFile, pFileList->sortMode);
	return pFile;
}

/**
 *@brief invalidate file list
 */
void fileInvalidate(gp_file_t *pFile)
{
	memset(pFile->gFilePath, 0, sizeof(pFile->gFilePath));
	memset(&pFile->gFileList, 0, sizeof(gp_fileList_t));
	printf("<rsp state=\"ok\"><return msg=\"ok\" /></rsp>");
}


/**
 *@brief get file info
 *@param path [in] file path
 */
void getFileInfo(char *path)
{
	int ret;
	struct stat info;
	struct tm *p;

	ret = stat(path, &info);

	if(ret == -1){
		failInfo(1, "can't stat file\n");
	}
	else{
		p = localtime(&(info.st_mtime));

		printf("<rsp state=\"ok\"><return ");
		printf("filesize=\"%d\" ", (int)info.st_size);
		printf("mode=\"%d\" ", S_ISDIR(info.st_mode));
		printf("year=\"%d\" ", p->tm_year+1900);
		printf("mon=\"%d\" ", p->tm_mon+1);
		printf("mday=\"%d\" ", p->tm_mday);
		printf("/></rsp>");
	}

}

void moveItemFromFileList(gp_file_t *pFile, char *filePath, int dir)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	int i;
	unsigned int temp;
	char buffer[512];

	if(!pFileList->buildInit){
		return;
	}

	temp = calibrateHash(filePath);

	if(dir){
		if(pFileList->searchMode == SEARCH_DEFAULT){
			i = 0;
			while(i < pFileList->dirPoolSize){
				if(getHash(pFile, i) == temp){
					sprintf(buffer, "%s%s/", getPath(pFile, i), getFile(pFile, i));
					if(strcmp(filePath, buffer) == 0){
						pFileList->fileMap[i] = -1;
						pFileList->dirNum --;
						break;
					}
				}
				i++;
			}
		}
	}
	else{
		if(!ConverFileToType(pFile, filePath)){
			return;
		}
		i = pFileList->dirPoolSize;
		while(i < pFileList->dirPoolSize + pFileList->filePoolSize){
			if(getHash(pFile, i) == temp){
				sprintf(buffer, "%s%s", getPath(pFile, i), getFile(pFile, i));
				if(strcmp(filePath, buffer) == 0){
					pFileList->fileMap[i] = -1;
					//pFileList->fileNum --;
					break;
				}
			}
			i++;
		}
	}

}

/**
 *@brief delete file
 *@param path [in] file path
 */
void fileDel(gp_file_t *pFile, char *path)
{
	int ret;
	struct stat info;

	ret = stat(path, &info);

	if(ret < 0){
		failInfo(1, "stat file error\n");
		goto error;
	}

	if(S_ISDIR(info.st_mode)){
		failInfo(1, "it is dir\n");
		goto error;
	}

	ret = remove(path);
	
	if(ret){
		failInfo(1, "can't remove file\n");
		goto error;
	}
	else{
		moveItemFromFileList(pFile, path, 0);
		printf("<rsp state=\"ok\"><return status=\"ok\" /></rsp>");
	}
	//system("nandsync &");
	return;

error:
	failInfo(1, "can't delete file\n");
}

int removeDir(char *path){
	DIR *fd;
	struct dirent *dir;
	int ret;
	char file[512];
	
	fd = opendir(path);

	if(fd < 0){
		return -1;
	}

	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}
		if(dir->d_type == DT_DIR){
			sprintf(file,"%s%s/",path,dir->d_name);
			ret = removeDir(file);
			if(ret){
				closedir(fd);
				return -1;
			}

		}
		else{			
			sprintf(file,"%s%s",path,dir->d_name);
			ret = remove(file);
			if(ret){
				closedir(fd);
				return -1;
			}
			gDeleteCurNum++;
		}

		if((gDeleteCurNum%50) == 0){
			sync();
			system("nandsync &");
		}
	
	}
	closedir(fd);
	
	ret = remove(path);
	if(ret){
		return -1;
	}
	gDeleteCurNum++;
	return 0;
}

int getDeleteAllNum(char *path)
{
	DIR *fd;
	struct dirent *dir;
	int ret;
	char buffer[512];
	int num = 0;
	
	fd = opendir(path);

	if(fd < 0){
		return -1;
	}
	num ++;

	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}
		
		if(dir->d_type == DT_DIR){
			sprintf(buffer, "%s/%s", path, dir->d_name);
			ret = getDeleteAllNum(buffer);

			if(ret >= 0){
				num = num + ret;
			}
		}
		else{
			num++;
		}
	}
	closedir(fd);
	return num;

}

void *deleteThread(void *arg){
	int ret;

	gp_file_t *pFile = (gp_file_t *)arg;
	
	pthread_detach(pthread_self());
	ret = getDeleteAllNum(pFile->gDelFilePath);
	if(ret <= 0){
		fprintf(stderr, "delete error\n");
		gDeleteStatus = DELETE_ERROR;
		return NULL;

	}
	gDeleteTotalNum = ret;
	
	ret = removeDir(pFile->gDelFilePath);

	if(ret){
		fprintf(stderr, "delete error\n");
		gDeleteStatus = DELETE_ERROR;
		return NULL;
	}
	moveItemFromFileList(pFile, pFile->gDelFilePath, 1);

	sync();
	system("nandsync &");	

	gDeleteStatus = DELETE_OK;

	return (void *)0;
}

/**
 *@brief delete folder
 *@param path [in] folder path
 */
void dirDel(gp_file_t *pFile, char *path)
{
	int ret;
	struct stat info;
	char buffer[512];

	ret = stat(path, &info);
	if((ret < 0)||(!S_ISDIR(info.st_mode))){
		goto error;
	}

	if(path[strlen(path) - 1] != '/'){
		sprintf(buffer, "%s/", path);
	}
	else{
		strcpy(buffer, path);
	}

	gDeleteStatus = DELETE_ING;
	gDeleteTotalNum = 1;
	gDeleteCurNum = 0;
	sprintf(pFile->gDelFilePath, "%s", buffer);
	ret = pthread_create(&pDeletHandle,NULL,deleteThread,pFile);
	if(ret != 0){
		fprintf(stderr, "can't create thread\n");
		gDeleteStatus = DELETE_ERROR;
		goto error;
	}
	printf("<rsp state=\"ok\"><return msg=\"ok\" /></rsp>");
	return;

error:
	gDeleteStatus = DELETE_ERROR;
	failInfo(1, "can't delete dir\n");
}

void gpGetDelStatus(void){

	if(gDeleteStatus == DELETE_ING){
		printf("<rsp state=\"ok\"><return progress=\"%d\" /></rsp>", (gDeleteCurNum*100)/gDeleteTotalNum);
	}
	else if(gDeleteStatus == DELETE_ERROR){
		printf("<rsp state=\"fail\"><return msg=\"error\" /></rsp>");
	}
	else if(gDeleteStatus == DELETE_OK){
		printf("<rsp state=\"ok\"><return msg=\"ok\" /></rsp>");
	}
	else{
		printf("<rsp state=\"ok\"><return msg=\"ok\" /></rsp>");
	}
		
}
int setFileLock(gp_file_t *pFile, int index, int lock)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;

	if(index > pFileList->fileNum) {
		printf("Error: index %d fileNum:%d\n", index, pFileList->fileNum);
		return -1;
	}

	if(pFileList->fileMap[index] < 0){
		printf("Error: file map\n");
		return -1;
	}

	pFile->gFilePool[pFileList->fileMap[index]].lock = lock;
	char buf[512];
	struct stat statbuf;
	printf("%s:%d lock %d \n", __FUNCTION__, __LINE__, lock);
	if(lock) {
		sprintf(buf, "chmod a-w %s%s &", getPath(pFile, index), getFile(pFile, index));
		system(buf);
		printf("lock file:%s\n", buf);
		/*sprintf(buf, "%s%s", getPath(pFile, index), getFile(pFile, index));
		if(stat(buf, &statbuf) < 0) {
			perror("stat");
			return -1;
		}
		printf("lock file:%s\n", buf);
		if(chmod(buf, (statbuf.st_mode & (~S_IWUSR))) <0 ) {
			perror("chmod");
			return -1;
		}*/
	}
	else {
		sprintf(buf, "chmod a+w %s%s &", getPath(pFile, index), getFile(pFile, index));
		system(buf);
		printf("unlock file:%s\n", buf);
		/*sprintf(buf, "%s%s", getPath(pFile, index), getFile(pFile, index));
		printf("unlock file:%s\n", buf);
		if(chmod(buf, S_IWUSR) < 0) {
			perror("chmod");
			return -1;
		}*/
	}
	system("sync");
	return 0;
}
int setFileAllLock(gp_file_t *pFile, int lock)
{
	int idx = 0;
	struct gp_fileList_s *pFileList = &pFile->gFileList;
	char buf[512];

	for(idx=0; idx<pFileList->fileNum;idx++) {
		pFile->gFilePool[pFileList->fileMap[idx]].lock = lock;
		if(lock) {
			sprintf(buf, "chmod a-w %s%s", getPath(pFile, idx), getFile(pFile, idx));
			printf("%s\n", buf);
			system(buf);
		}
		else {
			sprintf(buf, "chmod a+w %s%s", getPath(pFile, idx), getFile(pFile, idx));
			system(buf);
		}
	}
	system("sync");
	return 0;
}
int getFileLock(gp_file_t *pFile, int index)
{
	struct gp_fileList_s *pFileList = &pFile->gFileList;

	if(index > pFileList->fileNum) {
		return -1;
	}

	if(pFileList->fileMap[index] < 0){
		return -1;
	}

	return pFile->gFilePool[pFileList->fileMap[index]].lock;
}
