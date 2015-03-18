#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include "mach/typedef.h"
#include "disp.h"
#include "file_list.h"
#include "media_shell.h"
#include "photo_manager.h"
#include "playback_demo.h"
#include "gp_VideoEngineApi.h"
#include "chunkmem.h"


#define VIDEO_PLAY_SPEED_STEP (4)
#define VIDEO_PLAY_VOLUME_STEP (1)

//static char*filepath = "/media/sdcardc1/";
//static char *filefilter = "MOV|avi|mp4|jpg";
static char filepath[32];
static char *filefilter = "jpg|JPG|MOV|mov";
static char filestring[512];
static char filestring2[512];

extern display_buffer_t DispBufManage;

float play_speed[4] = {1.0, 2.0, 3.0, 4.0};

static cdvr_thmb_t *g_cdvr = NULL;
int g_VideoEnd = 0;
gp_disp_pixelsize_t pixelSize;
char fileMode = 0; //for file list search file mode.0:normal, 1:for cvr_dual


UINT32 ExtGpVideoEngineSetSpeed(float spd);

#define GP_EPOCH 1156000000
static unsigned long sysGetCurTime(void)
{	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((tv.tv_sec- GP_EPOCH)*1000+tv.tv_usec/1000);
}


int mcpGetThumbnail(gp_bitmap_t *bitmap, char *media_url, int thmb_time)
{
	int ret = -1;
	ret = ExtGpVideoEngineSetUrl(media_url);
	if(ret<0) {
		return ret;
	}
	ExtGpVideoEngineGetInfo(&g_cdvr->pVideoInfo->srcSize);
	g_cdvr->pVideoInfo->frameRate = ExtGpVideoEngineGetFrameRate();
	ret = ExtGpVideoEngineGetThumbnail(bitmap);

	ExtGpVideoEngineStop();

	return ret;
}
int mcpQtffGetThumbnail(gp_bitmap_t *bitmap, char *media_url)
{
	int ret;
	ret = ExtGpVideoEngineQtffGetThumbnail(media_url, bitmap);
	return ret;
}

int FilelistSetFilePath(char *path)
{
	if(path && strlen(path)>5 && access(path, F_OK) == 0) {
		strcpy(filepath, path);
	}
	else {
		strcpy(filepath, "/media/");
	}

	return 0;
}

gp_file_t *FilelistGetFileNum(cdvr_thmb_t *pdvr, gp_file_t *pFile, int reFlash)
{
	int ret,number;
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	gp_file_t *pF = fileListReflash(pFile, filepath, filefilter, SEARCH_ALL_FILE, 0, reFlash);
	if(!pF){
		printf("%s:%d error\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	pdvr->file_number = getFileListNum(pF, 1);		
	
	printf("=====file number is %d======\n",pdvr->file_number);
	return pF;
}
char *FilelistGetFileName(gp_file_t *pFile, int index)
{
	return getFile(pFile, index);
}
char *FilelistGetFilePath(gp_file_t *pFile, int index)
{
	char *str;
	char *strret;
	
	str = (char *)&filestring;
	strret = getPath(pFile, index);
	strcpy(str,strret);
	strret = getFile(pFile, index);
	strcat(str,strret);

	if(access(str, F_OK) == 0 ) { 
		return str;
	}
	else {
		return NULL;
	}
}
char *FilelistGetFilePath2(gp_file_t *pFile, int index)
{
	char *str;
	char *strret;
	
	str = (char *)&filestring2;
	strret = getPath(pFile, index);
	strcpy(str,strret);
	strret = getFile(pFile, index);
	strcat(str,strret);

	if(access(str, F_OK) == 0 ) { 
		return str;
	}
	else {
		return NULL;
	}
}
int FilelistGetLock(gp_file_t *pFile, int index)
{
	return getFileLock(pFile, index);
}
int FilelistSetLock(gp_file_t *pFile, int index)
{
	return setFileLock(pFile, index, GP_FILE_LOCK);
}
int FilelistSetLockAll(gp_file_t *pFile)
{
	return setFileAllLock(pFile, GP_FILE_LOCK);
}
int FilelistSetUnLock(gp_file_t *pFile, int index)
{
	return setFileLock(pFile, index, GP_FILE_UNLOCK);
}
int FilelistSetUnLockAll(gp_file_t *pFile)
{
	return setFileAllLock(pFile, GP_FILE_UNLOCK);
}
/**
 *@brief get file filter
 *@param filename [in] file name
 *@return file type
 */
static char * GetFileType( char *filename)
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
 * check file type by filename.
 * filename: check file name.
 * return : video or jpeg or error
 **/
static int checkFileType(char *filename)
{
	char *pType = GetFileType(filename);

	/*if(strncmp(pType, "avi", 3) == 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else if(strncmp(pType, "MOV", 3) == 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else if(strncmp(pType, "mov", 3) == 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else */if(strncmp(pType, "jpg", 3) == 0) {
		return GP_FILE_TYPE_JPEG;
	}
	else if(strncmp(pType, "JPG", 3) == 0) {
		return GP_FILE_TYPE_JPEG;
	}
	else if(strstr(filefilter, pType) != 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else {
		return GP_FILE_TYPE_UNKNOWN;
	}
}

cdvr_thmb_t *dvr_thmb_init(int screen_w, int screen_h)
{
	cdvr_thmb_t *dvr_thmb = NULL;
	disp_thmb_t *pthmb = NULL;
	int i;
	int s_w, s_h;
	int off_x=0, off_y=0;
	float scl_edg_x = 1.0, scl_edg_y = 1.0;

	if(!screen_w || !screen_h) {
		printf("screen width/height or file number error!");
		return NULL;
	}

	if(screen_h == 480) {
		s_w = 640;
		s_h = 426;
		off_x = 40;
		off_y = 27;
	}
	else {
		s_w = screen_w;
		s_h = screen_h;
	}

	scl_edg_x = (float)screen_w/320.0;
	scl_edg_y = (float)screen_h/240.0;

	dvr_thmb = (cdvr_thmb_t *) malloc(sizeof(cdvr_thmb_t));
	memset(dvr_thmb, 0, sizeof(cdvr_thmb_t));
	dvr_thmb->pVideoInfo = malloc(sizeof(video_Info_t));
	memset(dvr_thmb->pVideoInfo, 0, sizeof(video_Info_t));
	dvr_thmb->file_number = -1;

	dvr_thmb->disp_number = DEF_THMB_ROW * DEF_THMB_COLUMN;
	
	dvr_thmb->disp_page = -1;
	dvr_thmb->cur_idx = -1;
	dvr_thmb->cur_page = -1;
	dvr_thmb->cur_pageNum = -1;
	dvr_thmb->dispMode = -1;
	dvr_thmb->pVideoInfo->speed = 0;
	dvr_thmb->pVideoInfo->volume = ExtGpVideoEngineGetVolume();
	if(dvr_thmb->pVideoInfo->volume > 8) {
		dvr_thmb->pVideoInfo->volume = 8;
	}
	dvr_thmb->pVideoInfo->videoMode = VIDEO_OPEN;

	dvr_thmb->disp_bitmap.width = screen_w;
	dvr_thmb->disp_bitmap.height = screen_h;
	dvr_thmb->disp_bitmap.type = SP_BITMAP_RGB565;
	dvr_thmb->disp_bitmap.bpl = screen_w*2;
	dvr_thmb->disp_bitmap.validRect.x = 0;
	dvr_thmb->disp_bitmap.validRect.y = 0;
	dvr_thmb->disp_bitmap.validRect.width = screen_w;
	dvr_thmb->disp_bitmap.validRect.height = screen_h;
	//dvr_thmb->disp_bitmap.pData = gpChunkMemAlloc(dvr_thmb->disp_bitmap.width*dvr_thmb->disp_bitmap.height*2);

	for(i=0; i<dvr_thmb->disp_number; i++) {
		pthmb = &dvr_thmb->disp_thmb[i];
		pthmb->index = i;

		pthmb->bitmap.width = (float)((float)s_w - (float)DEF_THMB_EDGE_W*scl_edg_x*4)/DEF_THMB_COLUMN;
		//pthmb->bitmap.height = (float)((float)s_h - (float)DEF_THMB_EDGE_H*scl_edg_y*4)/DEF_THMB_ROW;
		pthmb->bitmap.height = (pthmb->bitmap.width*s_h)/s_w;
		int thmb_edge_h = (s_h - pthmb->bitmap.height*DEF_THMB_ROW) / (DEF_THMB_ROW+1);
		pthmb->x = off_x + (DEF_THMB_EDGE_W*scl_edg_x+ (i%DEF_THMB_COLUMN)* pthmb->bitmap.width + (i%DEF_THMB_COLUMN)*DEF_THMB_EDGE_W*scl_edg_x);
		//pthmb->y = off_y + (DEF_THMB_EDGE_H*scl_edg_y+ (i/DEF_THMB_ROW)* pthmb->bitmap.height + (i/DEF_THMB_ROW)*DEF_THMB_EDGE_H*scl_edg_y);
		pthmb->y = off_y + (thmb_edge_h + (i/DEF_THMB_ROW)* pthmb->bitmap.height + (i/DEF_THMB_ROW)*thmb_edge_h);

		pthmb->x &= (~0x01);
		pthmb->y &= (~0x01); 
		//pthmb->bitmap.width -= (float)DEF_THMB_EDGE_W*scl_edg_x*2;
		//pthmb->bitmap.height -= (float)DEF_THMB_EDGE_H*scl_edg_y*2;
		pthmb->bitmap.width &= (~0x01);
		pthmb->bitmap.height &= (~0x01);
		pthmb->bitmap.validRect.width = pthmb->bitmap.width;
		pthmb->bitmap.validRect.height = pthmb->bitmap.height;
		pthmb->bitmap.validRect.x = pthmb->x;
		pthmb->bitmap.validRect.y = pthmb->y;
		pthmb->bitmap.bpl = pthmb->bitmap.width*2;

		pthmb->bitmap.width = screen_w;
		pthmb->bitmap.height = screen_h;

		pthmb->bitmap.type = SP_BITMAP_RGB565;
		
		pthmb->bitmap.pData = NULL;
		printf("pri x %d y %d w %d h %d\n", pthmb->bitmap.validRect.x, pthmb->bitmap.validRect.y, pthmb->bitmap.validRect.width, pthmb->bitmap.validRect.height);
	}

	return dvr_thmb;;

}

int dvr_thmb_uninit(cdvr_thmb_t *pdvr)
{
	disp_thmb_t *pthmb = NULL;
	int i;
	if(!pdvr) {
		return -1;
	}

	free(pdvr->pVideoInfo);
	free(pdvr);
	return 0;
}

int dvr_thmb_setFileNumber(cdvr_thmb_t *pdvr, int file_num, int reset)
{
	
	if(!pdvr) {
		return -1;
	}

	//int lastFileNum = pdvr->file_number;
	int idx = dvr_thmb_getCurIdxOfFile(pdvr);
	printf("f:%d idx %d\n",pdvr->file_number, idx);
	if(pdvr->file_number == idx) { //delete last file
		reset = 1;
	}
	pdvr->file_number = file_num;

	pdvr->disp_page = file_num/(pdvr->disp_number);
	if(file_num%(pdvr->disp_number)) {
		pdvr->disp_page++;
	}


#if 0
	if(pdvr->cur_idx == -1) {
		pdvr->cur_idx = 0;
	}
	else if(pdvr->cur_idx >= pdvr->file_number) {
		pdvr->cur_idx = 0;
		pdvr->cur_page = 0;
	}
	if(pdvr->cur_page == -1) {
		pdvr->cur_page = pdvr->disp_page-1;
	}
	if(pdvr->cur_page == pdvr->disp_page-1) {
		pdvr->cur_pageNum = pdvr->file_number % pdvr->disp_number;
		pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
	}
	else {
		pdvr->cur_pageNum = pdvr->disp_number;
	}
#else
	if(pdvr->disp_page) {

		if(reset) {
			pdvr->cur_page = pdvr->disp_page-1;
			pdvr->cur_pageNum = pdvr->file_number%pdvr->disp_number;
			pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
			pdvr->cur_idx = pdvr->cur_pageNum-1;
		}
		else { //not reset and not delete the last file
 			//if(pdvr->cur_idx-1)
			//pdvr->cur_page = pdvr->disp_page-1;
			//pdvr->cur_pageNum = pdvr->file_number%pdvr->disp_number;
			//pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
		}

		//pdvr->cur_idx = pdvr->cur_pageNum-1;

		if(fileMode && pdvr->file_number > 1) {
			idx = dvr_thmb_getCurIdxOfFile(pdvr);
			if(FileNameMatch(FilelistGetFilePath(pdvr->pFile, idx), FilelistGetFilePath2(pdvr->pFile, idx-1)) >= 0) {
				dvr_thmb_preIdx(pdvr);
			}
		}
	}
#endif
	printf("[%s:%d]curIdx %d curPage %d pageNum %d \n", __FUNCTION__, __LINE__, pdvr->cur_idx, pdvr->cur_page, pdvr->cur_pageNum);




	pdvr->dispMode = DISP_FULL_SCREEN;
	if(!pdvr->disp_page) {
		pdvr->dispMode = DISP_NO_FILE;
		if(DispBufManage.DispDev == C_DISP_BUFFER) {
			pdvr->upFrame =  (unsigned char *)get_idle_buffer(0);
			clean_buffer(pdvr->upFrame, 0);
		}
		else {
			pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
			dispCleanFramebuffer(pdvr->hDisp);
		}
		pdvr->disp_page = -1;
		pdvr->cur_idx = -1;
		pdvr->cur_page = -1;
		pdvr->cur_pageNum = -1;
		dvr_thmb_dispFlip(pdvr);
	}
	else {
		pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
	}
	return 0;
}

static int dvr_thmb_setCurIdx(cdvr_thmb_t *pdvr, int curIdx)
{
	if(!pdvr) {
		return -1;
	}

	pdvr->cur_idx = curIdx;
	return 0;
}
static int dvr_thmb_setDispFullMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_FULL_SCREEN;

	return 0;
}
static int dvr_thmb_setDispThmbMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_THMB_SCREEN;

	return 0;
}
static int dvr_thmb_setDispVideoMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_VIDEO_SCREEN;

	return 0;
}
static int dvr_thmb_setDispScaleMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_SCALE_SCREEN;

	return 0;
}
int dvr_thmb_getDispMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}

	return pdvr->dispMode;
}

int dvr_thmb_setDispMode(cdvr_thmb_t *pdvr, int mode)
{
	if(!pdvr) {
		return -1;
	}

	pdvr->dispMode = mode;
	return 0;
}

int dvr_thmb_getCvrInfo(cdvr_thmb_t *pdvr, cdvr_info_t *pCvrInfo)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	if(!pdvr || !pCvrInfo) {
		return -1;
	}
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	pCvrInfo->disp_number = pdvr->disp_number;
	pCvrInfo->disp_page = pdvr->disp_page;
	pCvrInfo->file_number = pdvr->file_number;
	pCvrInfo->cur_idx = pdvr->cur_idx;
	pCvrInfo->cur_page = pdvr->cur_page;
	pCvrInfo->cur_pageNum = pdvr->cur_pageNum;
	pCvrInfo->dispMode = pdvr->dispMode;
	pCvrInfo->fileType = pdvr->fileType;

	return 0;
}

int dvr_thmb_getCurIdxOfFile(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}

	return pdvr->cur_idx+pdvr->cur_page*pdvr->disp_number;
}

int dvr_thmb_deleteFile(cdvr_thmb_t *pdvr, gp_file_t **ppFile)
{
	
	if(FilelistGetLock(*ppFile, dvr_thmb_getCurIdxOfFile(pdvr)) == GP_FILE_UNLOCK) {
		printf("======del file %s=======\n", pdvr->pUrl);
		fileDel(*ppFile, pdvr->pUrl);
		printf("delete ok!\n");
		sync();
		*ppFile = FilelistGetFileNum(pdvr, *ppFile, 1);
		printf("get file number:%d\n", pdvr->file_number);
		dvr_thmb_setFileNumber(pdvr, pdvr->file_number, 0);
		if(pdvr->dispMode == DISP_FULL_SCREEN) {
			dvr_thmb_dispFullScreen(pdvr, pdvr->upFrame);
		}
	}
	return 0;
}

int dvr_thmb_deleteAllFile(cdvr_thmb_t *pdvr, gp_file_t **ppFile)
{
	int number = pdvr->file_number;
	int i;
	printf("get delete file number:%d\n", number);
	for(i=0; i<number; i++) {
		pdvr->pUrl = FilelistGetFilePath(*ppFile, i);
		if(FilelistGetLock(*ppFile, i) == GP_FILE_UNLOCK) {
			printf("======del file %d %s=======\n", i, pdvr->pUrl);
			fileDel(*ppFile, pdvr->pUrl);
			printf("delete ok!\n");
		}
		else {
			printf("%s is lock!!!!\n", pdvr->pUrl);
		}
	}
	sync();
	*ppFile = FilelistGetFileNum(pdvr, *ppFile, 1);
	printf("get file number:%d\n", pdvr->file_number);
	dvr_thmb_setFileNumber(pdvr, pdvr->file_number, 1);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		dvr_thmb_dispFullScreen(pdvr, pdvr->upFrame);
	}
	return 0;
}

int dvr_thmb_preIdx(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);

	int curIdx, curPage;
	curIdx = pdvr->cur_idx;
	curPage = pdvr->cur_page;
	curIdx--;
	if(curIdx<0) {
		curPage--;
		if(curPage < 0) {
			curPage = pdvr->disp_page-1;
		}
		pdvr->cur_page = curPage;

		if(pdvr->cur_page == pdvr->disp_page-1) {
			pdvr->cur_pageNum = pdvr->file_number % pdvr->disp_number;
			pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
		}
		else {
			pdvr->cur_pageNum = pdvr->disp_number;
		}

		pdvr->cur_idx = curIdx = pdvr->cur_pageNum-1;
		pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
	}
	else {
		pdvr->cur_idx = curIdx;
		pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
	}
	//dvr_thmb_dispFlip(pdvr);
	//printf("curIdx %d curPage %d pageNum %d\n", pdvr->cur_idx, pdvr->cur_page, pdvr->cur_pageNum);
	return 0;
}

int dvr_thmb_nextIdx(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}

    printf("%s:%d\n", __FUNCTION__, __LINE__);
	int curIdx, curPage;
	curIdx = pdvr->cur_idx;
	curPage = pdvr->cur_page;
	curIdx++;
	if(curIdx>=pdvr->cur_pageNum) {
		curPage++;
		if(curPage >= pdvr->disp_page) {
			curPage = 0;
		}
		pdvr->cur_page = curPage;
		if(pdvr->cur_page == pdvr->disp_page-1) {
			pdvr->cur_pageNum = pdvr->file_number % pdvr->disp_number;
			pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
		}
		else {
			pdvr->cur_pageNum = pdvr->disp_number;
		}
		curIdx = 0;
		pdvr->cur_idx = curIdx;
		pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
	}
	else {
		pdvr->cur_idx = curIdx;
		pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
	}
	//dvr_thmb_dispFlip(pdvr);
	printf("curIdx %d curPage %d pageNum %d\n", pdvr->cur_idx, pdvr->cur_page, pdvr->cur_pageNum);
	return 0;
}

int dvr_thmb_prePage(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);
	pdvr->cur_page--;
	if(pdvr->cur_page < 0) {
		pdvr->cur_page = pdvr->disp_page-1;
	}
	if(pdvr->cur_page == pdvr->disp_page-1) {
		pdvr->cur_pageNum = pdvr->file_number % pdvr->disp_number;
		pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
	}
	else {
		pdvr->cur_pageNum = pdvr->disp_number;
	}
	if(pdvr->cur_idx >= pdvr->cur_pageNum) {
		pdvr->cur_idx = pdvr->cur_pageNum-1;
	}
	return 0;
}

int dvr_thmb_nextPage(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);
	pdvr->cur_page++;
	if(pdvr->cur_page >= pdvr->disp_page) {
		pdvr->cur_page = 0;
	}
	if(pdvr->cur_page == pdvr->disp_page-1) {
		pdvr->cur_pageNum = pdvr->file_number % pdvr->disp_number;
		pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
	}
	else {
		pdvr->cur_pageNum = pdvr->disp_number;
	}
	if(pdvr->cur_idx >= pdvr->cur_pageNum) {
		pdvr->cur_idx = pdvr->cur_pageNum-1;
	}
	return 0;
}


int dvr_thmb_dispPage(cdvr_thmb_t *pdvr, int page, char *disp_pData)
{
	int s_idx = -1;
	int ret = -1;
	int i;
	SINT64 stime, etime;
	disp_thmb_t *pthmb = NULL;
	if(!pdvr) {
		return -1;
	}

	dvr_thmb_setDispThmbMode(pdvr);

	photo_uninit(pdvr->pPhotoInfo); //for jpeg->thmb->jpeg
	pdvr->pPhotoInfo = NULL;

	//pdvr->cur_page = page;
	s_idx = pdvr->disp_number*pdvr->cur_page;
	//pdvr->cur_idx = 0;

	for(i=0; i<pdvr->disp_number; i++) {
		pthmb = &pdvr->disp_thmb[i];
		pdvr->pUrl = pthmb->url = FilelistGetFilePath(pdvr->pFile, s_idx);
		//printf("get thmb:%s\n", pdvr->pUrl);
		if(pthmb->url) {
			pdvr->fileType = pthmb->type = checkFileType(pthmb->url);
			pthmb->lock = FilelistGetLock(pdvr->pFile, s_idx);
			//printf("get lock %d\n", pthmb->lock);
			//printf("get %d %s type: %d thumbnail:\n", s_idx, pthmb->url, pthmb->type);
			stime = sysGetCurTime(); 
			ret = -1;

			if(pthmb->type == GP_FILE_TYPE_VIDEO) {
				pthmb->bitmap.pData = disp_pData;
				ret = mcpQtffGetThumbnail(&pthmb->bitmap, pthmb->url);
				if(ret == -1) {
					printf("WARNNING: USING parse video thumbnail !\n");
					ret = mcpGetThumbnail(&pthmb->bitmap, pthmb->url, 0);
					if(ret == -1) {
						printf("WARNNING: get video thumbnail error!!!!!!!\n");
					}
				}
			}
			else if(pthmb->type == GP_FILE_TYPE_JPEG) {
				ret = load_imageThumbnail(pthmb->url, pdvr, pthmb, (UINT8 *)disp_pData);
			}
			etime = sysGetCurTime();
			printf("thumbnail used time %lld - %lld = %lldms ret %d\n", etime, stime, etime - stime, ret);
		}
		s_idx++;
		if(s_idx == pdvr->file_number) {
			break;
		}
	}

	return 0;
}

/**
 * jpeg file : need to updata pdvr->disp_bitmap to display;
 * video: need to call dispFlip();
 **/
int dvr_thmb_dispFullScreen(cdvr_thmb_t *pdvr, char *disp_pData)
{
	int ret = -1;
	SINT64 stime, etime;
	gp_bitmap_t *pbitmap= NULL;
	if(!pdvr) {
		return -1;
	}
	char *purl = NULL;
	int type = -1;
	dvr_thmb_setDispFullMode(pdvr);

	if(DispBufManage.DispDev == C_DISP_BUFFER) {
		pdvr->upFrame =  (unsigned char *)get_idle_buffer(0);
		clean_buffer(pdvr->upFrame, 0);
	}
	else {
		pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
	}
	
	pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
	pdvr->fileType = checkFileType(pdvr->pUrl);
	printf("[%s:%d]curIdx %d curPage %d pageNum %d type %d\n", __FUNCTION__, __LINE__, pdvr->cur_idx, pdvr->cur_page, pdvr->cur_pageNum, pdvr->fileType);
	purl = pdvr->pUrl;
	type = pdvr->fileType;
	photo_uninit(pdvr->pPhotoInfo); //for jpeg->video->jpeg
	pdvr->pPhotoInfo = NULL;
	if(purl) {
		stime = sysGetCurTime(); 
		ret = -1;

		if(type == GP_FILE_TYPE_VIDEO) {
			pbitmap= &pdvr->disp_bitmap;
			pbitmap->pData = (unsigned char*)pdvr->upFrame;
			ret = mcpGetThumbnail(pbitmap, purl, 0);
		}
		else if(type == GP_FILE_TYPE_JPEG) {
			pdvr->pPhotoInfo = photo_init(purl, &pdvr->disp_bitmap);
			if(pdvr->pPhotoInfo) {
				pdvr->pPhotoInfo->bitmap->pData = (unsigned char*)pdvr->upFrame;
				ret = image_scale(pdvr->pPhotoInfo);
				if(ret == -1) {
				}
			}
		}
		etime = sysGetCurTime();
		printf("thumbnail used time %lld - %lld = %lldms ret %d\n", etime, stime, etime - stime, ret);
		if(ret == -1) {
			//dispCleanFramebuffer(pdvr->hDisp);
			if(DispBufManage.DispDev == C_DISP_BUFFER) {
				pdvr->upFrame =  (unsigned char *)get_idle_buffer(0);
				clean_buffer(pdvr->upFrame, 0);
			}
			else {
				pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
				dispCleanFramebuffer(pdvr->hDisp);
			}
			printf("WARNNING: get fulscreen error!!!!!!!\n");
		}

	}
	dvr_thmb_dispFlip(pdvr);
	return ret;
}


static void videoProcessCallbackMessge(unsigned int id, unsigned int data)
{
	printf("videoProcessCallbackMessge....\n");
	switch(id)
	{
		case 0:
			printf("[mw video]PLAYER_MSG_PLAY_QUITOK ->\n");
			break;
		case 1:
			printf("[mw video]PLAYER_MSG_DISP_TO_END ->\n");
			break;
		case 3:
			printf("[mw video]PLAYER_MSG_PLAY_TO_END ->\n");
			g_VideoEnd = 1;
			break;					
		default:
			printf("[mw video] other play call back Message %d\n", id);
	}
}

int dvr_thmb_dispFlip(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	if(DispBufManage.DispDev == C_DISP_BUFFER) {
		send_ready_buffer((int)pdvr->upFrame);
	}
	else {
		dispFlip(pdvr->hDisp);
	}
	return 0;

}

int dvr_thmb_getCurIdx(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}

	return pdvr->cur_idx;
}
/**
 * return cur index file name. using for show name in screen.
 **/
char *dvr_thmb_getFileName(cdvr_thmb_t *pdvr, gp_file_t *pFile)
{
	return FilelistGetFileName(pFile, dvr_thmb_getCurIdxOfFile(pdvr));
}

/**
 * for photo to get size to show in screen.
 **/
int dvr_thmb_getFileResourse(cdvr_thmb_t *pdvr, gp_size_t *size)
{
	if(pdvr->fileType == GP_FILE_TYPE_JPEG) {
		if(!pdvr->pPhotoInfo) {
			size->width = 0;
			size->height = 0;
			return -1;
		} 
		size->width = pdvr->pPhotoInfo->srcSize.width;
		size->height = pdvr->pPhotoInfo->srcSize.height;
	}
	else if(pdvr->fileType == GP_FILE_TYPE_VIDEO){
		size->width = pdvr->pVideoInfo->srcSize.width;
		size->height = pdvr->pVideoInfo->srcSize.height;
	}
	else {
		size->width = 0;
		size->height = 0;
		return -1;
	}

	return 0;
}
/**
 * to get file info to show in screen.
 **/
int dvr_thmb_getFileInfo(cdvr_thmb_t *pdvr, char *buf)
{
	struct stat info;
	struct tm *p;
	stat((char *)pdvr->pUrl, &info);
	p = localtime(&(info.st_mtime));

	sprintf(buf, "%d/%d%d/%d%d %d%d:%d%d:%d%d", p->tm_year+1900, (p->tm_mon+1)/10, (p->tm_mon+1)%10, p->tm_mday/10, p->tm_mday%10, p->tm_hour/10, p->tm_hour%10, p->tm_min/10, p->tm_min%10, p->tm_sec/10,p->tm_sec%10);
	return 0;
}

int dvr_thmb_getPlayingTime(cdvr_thmb_t *pdvr)
{
	int ret = -1;
	if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
		SINT32 time = ExtGpVideoEngineGetCurTime();
		printf("get cur time: %d\n", time);
		pdvr->pVideoInfo->play_time = time;

		//ret = ExtGpVideoEngineGetPlayingStatus();
		//printf("get video decode status: %d\n", ret);

		if(g_VideoEnd) {
			printf("%s:%d, play video end %d\n", __FUNCTION__, __LINE__, g_VideoEnd);
			//pdvr->pVideoInfo->play_time = -1;
		}
		ret = 0;
	}
	return ret;
}

cdvr_thmb_t *playback_init(int disp)
{
	int file_num;
	HANDLE hDisp;
    gp_size_t resolution;

	if(DispBufManage.DispDev == C_DISP_BUFFER) {
		resolution.width = DispBufManage.Width;
		resolution.height = DispBufManage.Height;
		pixelSize.width = 16;
		pixelSize.height = 9;
		hDisp = NULL;
	}
	else {
		if (dispCreate(&hDisp, DISP_LAYER_PRIMARY) != SP_OK) {
			printf("dispCreate error\n");
			return NULL;
		}
    	dispGetResolution(hDisp, &resolution);
    	dispGetPixelSize(hDisp, disp, &pixelSize);
	}

	//mcplayer open
	ExtGpVideoEngineOpen(disp, resolution.width, resolution.height, videoProcessCallbackMessge, get_idle_buffer, send_ready_buffer);


	cdvr_thmb_t *pdvr = dvr_thmb_init(resolution.width, resolution.height);
	if(!pdvr) {
    	printf("%s:%d\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	//init filelist
	pdvr->pFile = FilelistGetFileNum(pdvr, pdvr->pFile, 1);
 
	pdvr->hDisp = hDisp;
	dvr_thmb_setFileNumber(pdvr, pdvr->file_number, 1);
    printf("%s:%d, %d\n", __FUNCTION__, __LINE__, pdvr->disp_number);
	int s_idx = pdvr->disp_number*pdvr->cur_page;
	int i;
	disp_thmb_t *pthmb = NULL;
	for(i=0; i<pdvr->disp_number&&i<=pdvr->cur_pageNum; i++) {
		pthmb = &pdvr->disp_thmb[i];
		pdvr->pUrl = pthmb->url = FilelistGetFilePath(pdvr->pFile, s_idx);
		printf("get thmb:%s\n", pdvr->pUrl);
		if(pthmb->url) {
			pdvr->fileType = pthmb->type = checkFileType(pthmb->url);
			pthmb->lock = FilelistGetLock(pdvr->pFile, s_idx);
			printf("get lock %d\n", pthmb->lock);
			//printf("get %d %s type: %d thumbnail:\n", s_idx, pthmb->url, pthmb->type);
		}
		s_idx++;
		if(s_idx == pdvr->file_number) {
			break;
		}
	}
	g_cdvr = pdvr;
	char buf[256];
	for(i=0; i<pdvr->file_number; i++) {
		printf("%s\n", pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, i));
		memset(buf, 0, 256);
		dvr_thmb_getFileInfo(pdvr, buf);
		printf("%s\n", buf);
	}
    	printf("%s:%d\n", __FUNCTION__, __LINE__);
	return pdvr;
}

int playback_uninit(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	
    printf("%s:%d\n", __FUNCTION__, __LINE__);
	dispDisablePrimary(pdvr->hDisp);
	dispDestroy(pdvr->hDisp);
    printf("%s:%d\n", __FUNCTION__, __LINE__);
	photo_uninit(pdvr->pPhotoInfo); //for jpeg->video->jpeg
	pdvr->pPhotoInfo = NULL;
	gpFileListUnInit(pdvr->pFile);
	pdvr->pFile = NULL;
	dvr_thmb_uninit(pdvr);
	pdvr = NULL;
	g_cdvr = NULL;
    printf("%s:%d\n", __FUNCTION__, __LINE__);
	
	ExtGpVideoEngineExit();
    printf("%s:%d player uninit\n", __FUNCTION__, __LINE__);
	return 0;
}


void dvr_thmb_printfInfo(cdvr_thmb_t *pdvr)
{
#if 0
	printf("-------cur player info---------\n");
	printf("FileNum: %d curIdx %d curPage %d pageNum %d\n", pdvr->file_number, pdvr->cur_idx, pdvr->cur_page, pdvr->cur_pageNum);
	printf("file(%s):%s\n","unlock", pdvr->pUrl);
	printf("------------------------\n");
#endif
}
void dvr_sdvr_printfInfo(cdvr_info_t *pInfo)
{
#if 0
	printf("----------cdvr player info-------------\n");
	printf("disp_number: %d\n", pInfo->disp_number);
	printf("disp_page: %d\n", pInfo->disp_page);
	printf("file_number: %d\n", pInfo->file_number);
	printf("cur_idx: %d\n", pInfo->cur_idx);
	printf("cur_page: %d\n", pInfo->cur_page);
	printf("cur_pageNum: %d\n", pInfo->cur_pageNum);
	printf("dispMode: %d\n", pInfo->dispMode);
	printf("fileType: %d\n", pInfo->fileType);
	printf("-----------------------\n");
#endif
}
