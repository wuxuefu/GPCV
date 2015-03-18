#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/soundcard.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include "mach/typedef.h"
#include "disp.h"
#include "playback_pipe.h"
#include "public_pipe.h"
#include "ap_state_config.h"
#include <errno.h>


#define VIDEO_PLAY_SPEED_STEP (4)
#define VIDEO_PLAY_VOLUME_STEP (1)

//static char *filefilter = "MOV|avi|mp4|jpg";
static char *filefilter = "jpg|JPG|MOV|mov";
static char filepath[128];
static char filename[128];

static float play_speed[4] = {1.0, 1.4, 1.8, 2.0};
static int g_width, g_height;

static cdvr_thmb_t *g_cdvr = NULL;
static int g_VideoEnd = 0;
static int timeOut_count = 0;
static char g_sys_vol = 0;
static char g_vid_vol = 0;
static char g_SetSpeed = -1;
static int g_cmd_ret = -1;


extern disp1manage_t dispmanager;
extern UINT8 Memory_path[64];
extern int playback_menu_mode;


int dvr_thmb_mutex_lock(void)
{
	if(g_cdvr == NULL) {
		return -1;
	}
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	return 0;
}

int dvr_thmb_mutex_unlock(void)
{
	if(g_cdvr == NULL) {
		return -1;
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
	return 0;
}
char *FilelistGetFilePath(int index)
{
	if(g_cdvr->pPath == NULL) {
		playback_Set_PIPE_With_CMD(CMD_GET_FILE_PATH, 1);
	}
	g_cdvr->pPath = filepath;
	return g_cdvr->pPath;
}
char *FilelistGetFileName(int index)
{
	if(g_cdvr->pUrl == NULL) {
		playback_Set_PIPE_With_CMD(CMD_GET_FILE_NAME, 1);
	}
	g_cdvr->pUrl = filename;
	//printf("%s:%d filename: %s\n", __FUNCTION__, __LINE__, filename);
	return g_cdvr->pUrl;
}

int FilelistRebuild(void)
{
	return playback_Set_PIPE_With_CMD(CMD_SET_REBUILD_FILELIST, 1);
}

int FilelistGetLock(int index)
{
	if(index >= g_cdvr->cur_pageNum) {
		return -1;
	}
	return g_cdvr->disp_thmb[index].lock;
}

int FilelistGetPageLock(void)
{
	playback_Set_PIPE_With_CMD(CMD_GET_PAGE_LOCK, 1);
	return g_cdvr->disp_thmb[g_cdvr->cur_idx].lock;
}
int FilelistSetLock(int index)
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_LOCK_FILE_PB, 1);
	if(ret == 0) {
		g_cdvr->disp_thmb[g_cdvr->cur_idx].lock = GP_FILE_LOCK;
	}
	return ret;
}
int FilelistSetLockAll(void)
{
	int ret, i;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_LOCK_ALL_FILE, 1);
	if(ret == 0) {
		for(i=0; i<g_cdvr->cur_pageNum; i++)
			g_cdvr->disp_thmb[i].lock = GP_FILE_LOCK;
	}
	return ret;
}
int FilelistSetUnLock(int index)
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_UNLOCK_FILE, 1);
	if(ret == 0) {
		g_cdvr->disp_thmb[g_cdvr->cur_idx].lock = GP_FILE_UNLOCK;
	}
	return ret;
}
int FilelistSetUnLockAll(void)
{
	int ret, i;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_UNLOCK_ALL_FILE, 1);
	if(ret == 0) {
		for(i=0; i<g_cdvr->cur_pageNum; i++)
			g_cdvr->disp_thmb[i].lock = GP_FILE_UNLOCK;
	}
	return ret;
}
int FilelistSetDelete()
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_DELETE_FILE, 0);
	/*ret = dvr_thmb_get_info();
	g_cdvr->pUrl = NULL;
	g_cdvr->pPath= NULL;
	FilelistGetPageLock();
	FilelistGetFilePath(0);
	FilelistGetFileName(0);
	playback_Set_PIPE_With_CMD(CMD_GET_FILE_ALL_TYPE, 1);
	dvr_thmb_dispFullScreen(g_cdvr);*/
	return ret;
}
int FilelistSetDeleteAll()
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_DELETE_ALL_FILE, 0);
	return ret;
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

	if(strncmp(pType, "avi", 3) == 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else if(strncmp(pType, "MOV", 3) == 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else if(strncmp(pType, "mov", 3) == 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else if(strncmp(pType, "jpg", 3) == 0) {
		return GP_FILE_TYPE_JPEG;
	}
	else if(strncmp(pType, "JPG", 3) == 0) {
		return GP_FILE_TYPE_JPEG;
	}
	else if(strncmp(pType, "mp4", 3) == 0) {
		return GP_FILE_TYPE_VIDEO;
	}
	else {
		return GP_FILE_TYPE_UNKNOWN;
	}
}

cdvr_thmb_t *dvr_thmb_init(int screen_w, int screen_h)
{
	int ret;
	cdvr_thmb_t *dvr_thmb = NULL;
	disp_thmb_t *pthmb = NULL;
	int i;
	float scl_edg_x = 1.0, scl_edg_y = 1.0;

	if(!screen_w || !screen_h) {
		printf("screen width/height or file number error!");
		return NULL;
	}
	
	scl_edg_x = (float)screen_w/320.0;
	scl_edg_y = (float)screen_h/240.0;
	
	dvr_thmb = (cdvr_thmb_t *) malloc(sizeof(cdvr_thmb_t));
	memset(dvr_thmb, 0, sizeof(cdvr_thmb_t));
	dvr_thmb->pVideoInfo = malloc(sizeof(video_Info_t));
	memset(dvr_thmb->pVideoInfo, 0, sizeof(video_Info_t));
	dvr_thmb->file_number = -1;

	dvr_thmb->disp_number = DEF_THMB_ROW * DEF_THMB_COLUMN;
	
	dvr_thmb->pUrl = NULL;
	dvr_thmb->pPath = NULL;
	dvr_thmb->dispMode = -1;
	dvr_thmb->disp_page = -1;
	dvr_thmb->cur_idx = -1;
	dvr_thmb->cur_page = -1;
	dvr_thmb->cur_pageNum = -1;
	dvr_thmb->pVideoInfo->speed = 0;
	dvr_thmb->pVideoInfo->frameRate = 0;
	dvr_thmb->sclIdx = 0;

	pthread_mutex_init(&dvr_thmb->pb_mutex, NULL);

	if(g_vid_vol) {
		dvr_thmb->pVideoInfo->volume = g_vid_vol;
	}
	else {
		dvr_thmb->pVideoInfo->volume = ExtGpVideoEngineGetVolume();
	}
	if(dvr_thmb->pVideoInfo->volume > 8) {
		dvr_thmb->pVideoInfo->volume = 8;
	}
	dvr_thmb->pVideoInfo->videoMode = VIDEO_OPEN;

	for(i=0; i<dvr_thmb->disp_number; i++) {
		pthmb = &dvr_thmb->disp_thmb[i];
		pthmb->index = i;
		//pthmb->row = i/DEF_THMB_ROW;
		//pthmb->column = i%DEF_THMB_COLUMN;
		pthmb->bitmap.width = (float)((float)screen_w - (float)DEF_THMB_EDGE_W*scl_edg_x*4)/DEF_THMB_COLUMN;
		//pthmb->bitmap.height = (float)((float)screen_h - (float)DEF_THMB_EDGE_H*scl_edg_y*4)/DEF_THMB_ROW;
		pthmb->bitmap.height = (pthmb->bitmap.width*screen_h)/screen_w;
		int thmb_edge_h = (screen_h - pthmb->bitmap.height*DEF_THMB_ROW) / (DEF_THMB_ROW+1);
		pthmb->x = (DEF_THMB_EDGE_W*scl_edg_x + (i%DEF_THMB_COLUMN)* pthmb->bitmap.width + (i%DEF_THMB_COLUMN)*DEF_THMB_EDGE_W*scl_edg_x);
		//pthmb->y = (DEF_THMB_EDGE_H*scl_edg_y + (i/DEF_THMB_ROW)* pthmb->bitmap.height + (i/DEF_THMB_ROW)*DEF_THMB_EDGE_H*scl_edg_y);
		pthmb->y = (thmb_edge_h + (i/DEF_THMB_ROW)* pthmb->bitmap.height + (i/DEF_THMB_ROW)*thmb_edge_h);
		if(screen_w == 640 ) {
			if(i<6) {
				pthmb->y += 2;
			}
			if(i%DEF_THMB_COLUMN == 2) {
				pthmb->x -= 4;
			}
			if(i%DEF_THMB_COLUMN == 1) {
				pthmb->x -= 4;
			}
		pthmb->x += 2;
		pthmb->y += 2; 
		pthmb->bitmap.width -= 4;
		pthmb->bitmap.height -= 4;			
		}
		pthmb->x &= (~0x01);
		pthmb->y &= (~0x01); 
		
		//pthmb->bitmap.width -= (float)DEF_THMB_EDGE_W*scl_edg_x*2;
		//pthmb->bitmap.height -= (float)DEF_THMB_EDGE_H*scl_edg_y*2;
		pthmb->bitmap.width &= (~0x01);
		pthmb->bitmap.height &= (~0x01);
		pthmb->bitmap.validRect.width = pthmb->bitmap.width;
		pthmb->bitmap.validRect.height = pthmb->bitmap.height;
		pthmb->bitmap.validRect.x = 0;
		pthmb->bitmap.validRect.y = 0;
		pthmb->bitmap.bpl = pthmb->bitmap.width*2;

		pthmb->bitmap.type = SP_BITMAP_RGB565;
		
		//pthmb->bitmap.pData = gpChunkMemAlloc(pthmb->bitmap.width*pthmb->bitmap.height*2);
		printf("osd x %d y %d w %d h %d\n", pthmb->x, pthmb->y, pthmb->bitmap.width, pthmb->bitmap.height);
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

	pthread_mutex_destroy(&pdvr->pb_mutex);

	for(i=0; i<pdvr->disp_number; i++) {
		pthmb = &pdvr->disp_thmb[i];
		//gpChunkMemFree(pthmb->bitmap.pData);
	}

	free(pdvr);
	return 0;
}

int dvr_thmb_get_info(void)
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_GET_CVR_INFO, 1);
	dvr_thmb_printfInfo(g_cdvr);
	return ret;
}
int dvr_thmb_set_info(cdvr_thmb_t * pdvr)
{
	int ret;
	cdvr_info_t cvrInfo;
	cvrInfo.disp_number = pdvr->disp_number;
	cvrInfo.disp_page = pdvr->disp_page;
	cvrInfo.file_number = pdvr->file_number;
	cvrInfo.cur_idx = pdvr->cur_idx;
	cvrInfo.cur_page = pdvr->cur_page;
	cvrInfo.cur_pageNum = pdvr->cur_pageNum;
	cvrInfo.dispMode = pdvr->dispMode;
	cvrInfo.fileType = pdvr->fileType;
	ret = playback_Set_PIPE(CMD_SET_CVR_INFO, (char *)&cvrInfo, sizeof(cvrInfo));
	dvr_thmb_printfInfo(g_cdvr);
	return ret;
}

int dvr_thmb_UpdateAllInfo(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		pdvr = g_cdvr;
		if(!pdvr) {
			return -1;
		}
	}
	dvr_thmb_get_info();
	pdvr->pUrl = NULL;
	pdvr->pPath= NULL;
	FilelistGetPageLock();
	FilelistGetFilePath(0);
	FilelistGetFileName(0);
	playback_Set_PIPE_With_CMD(CMD_GET_FILE_ALL_TYPE, 1);
	dvr_thmb_dispFullScreen(pdvr);
	return 0;

}
int dvr_thmb_setFileNumber(cdvr_thmb_t *pdvr, int file_num)
{
	
	if(!pdvr) {
		return -1;
	}

	pdvr->file_number = file_num;

	pdvr->disp_page = file_num/(pdvr->disp_number);
	if(file_num%(pdvr->disp_number)) {
		pdvr->disp_page++;
	}

	if(pdvr->cur_idx == -1) {
		pdvr->cur_idx = 0;
	}
	else if(pdvr->cur_idx >= pdvr->file_number) {
		pdvr->cur_idx = 0;
		pdvr->cur_page = 0;
	}
	if(pdvr->cur_page == -1) {
		pdvr->cur_page = 0;
	}
	if(pdvr->cur_page == pdvr->disp_page-1) {
		pdvr->cur_pageNum = pdvr->file_number % pdvr->disp_number;
		pdvr->cur_pageNum = pdvr->cur_pageNum?pdvr->cur_pageNum:pdvr->disp_number;
	}
	else {
		pdvr->cur_pageNum = pdvr->disp_number;
	}
	pdvr->dispMode = DISP_FULL_SCREEN;
	return 0;
}

static int dvr_thmb_setDispFullMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_FULL_SCREEN;
	int ret = playback_Set_PIPE(CMD_SET_DISP_MODE, &pdvr->dispMode, sizeof(pdvr->dispMode));

	return 0;
}
static int dvr_thmb_setDispThmbMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_THMB_SCREEN;
	int ret = playback_Set_PIPE(CMD_SET_DISP_MODE, &pdvr->dispMode, sizeof(pdvr->dispMode));

	return 0;
}
static int dvr_thmb_setDispVideoMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_VIDEO_SCREEN;
	int ret = playback_Set_PIPE(CMD_SET_DISP_MODE, &pdvr->dispMode, sizeof(pdvr->dispMode));
	return 0;
}
static int dvr_thmb_setDispScaleMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	pdvr->dispMode = DISP_SCALE_SCREEN;
	int ret = playback_Set_PIPE(CMD_SET_DISP_MODE, &pdvr->dispMode, sizeof(pdvr->dispMode));

	return 0;
}
int dvr_thmb_getDispMode(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		pdvr = g_cdvr;
		if(!pdvr) {
			return -1;
		}
	}

	return pdvr->dispMode;
}

int dvr_thmb_getCurIdxOfFile(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}

	return pdvr->cur_idx+pdvr->cur_page*pdvr->disp_number;
}

int dvr_thmb_preIdx(cdvr_thmb_t *pdvr)
{
	int ret;
	if(!pdvr) {
		return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);

	pdvr->pUrl = NULL;
	pdvr->pPath = NULL;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_PRE_INDEX, 1);
	dvr_thmb_get_info();
	FilelistGetPageLock();
	pdvr->pUrl = FilelistGetFileName(0);

	return ret;

}

int dvr_thmb_nextIdx(cdvr_thmb_t *pdvr)
{
	int ret;
	if(!pdvr) {
		pdvr = g_cdvr;
		if(!pdvr)
			return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);

	pdvr->pUrl = NULL;
	pdvr->pPath = NULL;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_NEXT_INDEX, 1);
	dvr_thmb_get_info();
	FilelistGetPageLock();
	pdvr->pUrl = FilelistGetFileName(0);

	return ret;
}

int dvr_thmb_prePage(cdvr_thmb_t *pdvr)
{
	int ret;
	if(!pdvr) {
		return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);

	pdvr->pUrl = NULL;
	pdvr->pPath = NULL;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_PRE_PAGE, 1);
	dvr_thmb_get_info();
	FilelistGetPageLock();
	pdvr->pUrl = FilelistGetFileName(0);

	return ret;
}

int dvr_thmb_nextPage(cdvr_thmb_t *pdvr)
{
	int ret;
	if(!pdvr) {
		return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);

	pdvr->pUrl = NULL;
	pdvr->pPath = NULL;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_NEXT_PAGE, 1);
	dvr_thmb_get_info();
	FilelistGetPageLock();
	pdvr->pUrl = FilelistGetFileName(0);

	return ret;
}


int dvr_thmb_dispPage(cdvr_thmb_t *pdvr)
{
	int ret;
	if(!pdvr) {
		return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);
	if(pdvr->dispMode == DISP_NO_FILE) {
		return -1;
	}

	dvr_thmb_setDispThmbMode(pdvr);
	ret = playback_Set_PIPE_With_CMD(CMD_SET_THMBSCREEN, 1);
	dvr_thmb_get_info();
	pdvr->pUrl = FilelistGetFileName(0);
	ret = playback_Set_PIPE_With_CMD(CMD_GET_FILE_ALL_TYPE, 1);
	FilelistGetPageLock();

	return ret;
}

/**
 **/
int dvr_thmb_dispFullScreen(cdvr_thmb_t *pdvr)
{
	int ret;
	if(!pdvr) {
		pdvr = g_cdvr;
		if(!pdvr)
			return -1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);

	if(pdvr->dispMode == DISP_NO_FILE) {
		return -1;
	}

	dvr_thmb_setDispFullMode(pdvr);
	ret = playback_Set_PIPE_With_CMD(CMD_SET_FULLSCREEN, 1);
	if(g_cmd_ret == -1) {
		g_cmd_ret = dvr_thmb_setMovRecover(pdvr);
		ret = playback_Set_PIPE_With_CMD(CMD_SET_FULLSCREEN, 1);
	}
	dvr_thmb_get_info();
	pdvr->pUrl = FilelistGetFileName(0);

	return ret;
}

/**
 **/
int dvr_thmb_dispFullScreen_Replace(cdvr_thmb_t *pdvr)
{
	int ret;
	if(!pdvr) {
		pdvr = g_cdvr;
		if(!pdvr)
			return -1;
	}

	if(pdvr->dispMode == DISP_NO_FILE) {
		return -1;
	}

	dvr_thmb_setDispFullMode(pdvr);
	ret = playback_Set_PIPE_With_CMD(CMD_SET_FULLSCREEN_REPLACE, 1);
	dvr_thmb_get_info();
	pdvr->pUrl = FilelistGetFileName(0);

	return ret;
}


int dvr_thmb_getCurIdx(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}

	return pdvr->cur_idx;
}

/**
 * for photo to get size to show in screen.
 **/
int dvr_thmb_getFileResourse(cdvr_thmb_t *pdvr, gp_size_t *size)
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_GET_JPEG_RESOURSE, 1);
	size->width = pdvr->srcSize.width;
	size->height = pdvr->srcSize.height;

	return ret;
}
/**
 * for photo to get scale index to show in screen.
 **/
int dvr_thmb_getScaleIndex(cdvr_thmb_t *pdvr)
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_GET_JPEG_SCLIDX, 1);

	return pdvr->sclIdx;
}
/**
 * for vidoe to get frame rate.
 **/
int dvr_thmb_getFrameRate(cdvr_thmb_t *pdvr)
{
	int ret;
	if(pdvr->pVideoInfo->frameRate == 0) {
		ret = playback_Set_PIPE_With_CMD(CMD_GET_FRAME_RATE, 1);
	}
	return pdvr->pVideoInfo->frameRate;
}
/**
 * for video to recover.
 **/
int dvr_thmb_setMovRecover(cdvr_thmb_t *pdvr)
{
	int ret;
	ret = playback_Set_PIPE_With_CMD(CMD_SET_RECOVER_FILE, 1);
	return ret;
}
/**
 * to get file info to show in screen.
 **/
int dvr_thmb_getFileInfo(cdvr_thmb_t *pdvr, char *buf)
{
	struct stat info;
	struct tm *p;
	char file[512];
	sprintf(file, "%s", FilelistGetFilePath(0));
	strcat(file, FilelistGetFileName(0));
	if(access(file, F_OK) == 0) {
		stat((char *)file, &info);
		localtime_r(&(info.st_mtime), &pdvr->FileTime);
	}

	p = &pdvr->FileTime;

	sprintf(buf, "%d/%d%d/%d%d %d%d:%d%d", p->tm_year+1900, (p->tm_mon+1)/10, (p->tm_mon+1)%10, p->tm_mday/10, p->tm_mday%10, p->tm_hour/10, p->tm_hour%10, p->tm_min/10, p->tm_min%10);
	return 0;
}

int dvr_thmb_getPlayingTime(void)
{
	int ret = -1;
	if(g_cdvr == NULL) {
		return ret;
	}
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(g_cdvr->dispMode == DISP_VIDEO_SCREEN) {
		if(g_cdvr->pVideoInfo->videoMode != VIDEO_OPEN && g_cdvr->pVideoInfo->videoMode != VIDEO_PAUSE) {
			SINT32 time = ExtGpVideoEngineGetCurTime();
			printf("%s:%d time %ld gtime: %ld\n", __FUNCTION__, __LINE__, time, g_cdvr->pVideoInfo->play_time);
			if(g_cdvr->pVideoInfo->show_time != g_cdvr->pVideoInfo->play_time) {
				g_cdvr->pVideoInfo->show_time = g_cdvr->pVideoInfo->play_time;
				ret = 0;
			}
			else {
				ret = -1;
			}
		}

		if(g_VideoEnd) {
			g_VideoEnd = 0;
			ExtGpVideoEngineStop();
			g_cdvr->pVideoInfo->videoMode = VIDEO_OPEN;
			g_cdvr->pVideoInfo->speed = 0;
			ap_ppu_text_to_rgb();
			dvr_thmb_dispFullScreen(g_cdvr);
		//	dvr_thmb_dispFullScreen_Replace(g_cdvr);
			printf("%s:%d\n", __FUNCTION__, __LINE__);
			//g_vid_vol = ExtGpVideoEngineGetVolume();
			Dsp_Open();
			ExtGpVideoEngineSetVolume(g_sys_vol);
			ret = 0;
		}
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
	return ret;
}

int dvr_check_video_playing(void)
{
	if(!g_cdvr) {
		return 1;
	}
	if(g_cdvr->pVideoInfo->videoMode == VIDEO_OPEN) {
		return 1;
	}
	else {
		return 0;
	}
}



UINT32 ExtGpVideoEnginePlay(void)
{
	int ret = playback_Set_PIPE_With_CMD(CMD_SET_PLAY, 1);
	return ret;
}
SINT32 ExtGpVideoEngineStop(void)
{
	int ret = playback_Set_PIPE_With_CMD(CMD_SET_STOP, 1);
	return ret;
}
SINT32 ExtGpVideoEnginePause()
{
	int ret = playback_Set_PIPE_With_CMD(CMD_SET_PAUSE, 1);
	return ret;
}
SINT32 ExtGpVideoEngineResume()
{
	int ret = playback_Set_PIPE_With_CMD(CMD_SET_RESUME, 1);
	return ret;
}
UINT32 ExtGpVideoEngineSetSpeed(int speed)
{
	printf("%s:%d set speed: %d\n", __FUNCTION__ , __LINE__, speed);
	int ret = playback_Set_PIPE(CMD_SET_SPEED, (char *)&speed, sizeof(int));
	return ret;
}
int ExtGpVideoEngineReversePlay(int revers)
{
	int ret;
	if(revers) {
		ret = playback_Set_PIPE_With_CMD(CMD_SET_REVERSE_ENABLE, 1);
	}
	else {
		ret = playback_Set_PIPE_With_CMD(CMD_SET_REVERSE_DISABLE, 1);
	}
	return ret;
}
SINT32 ExtGpVideoEngineGetCurTime()
{
	int ret = playback_Set_PIPE_With_CMD(CMD_GET_PLAYINT_TIME, 1);
	return g_cdvr->pVideoInfo->play_time;
}

void ExtGpVideoEngineSetVolume(int nGotoVol)
{
	//using audio_mixer
	SINT32 volume = (nGotoVol * 100) / 9;

	volume |= volume<<8;
    SINT32 MixHandle = open("/dev/mixer", O_RDONLY);

    ioctl(MixHandle, SOUND_MIXER_WRITE_VOLUME, &volume);
    close(MixHandle);
}
SINT32 ExtGpVideoEngineGetVolume()
{
    SINT32 MixHandle;
    SINT32 volume = 0;
    MixHandle = open("/dev/mixer", O_RDONLY);

    ioctl(MixHandle, SOUND_MIXER_READ_VOLUME, &volume);
    close(MixHandle);
	printf("%s:%d get: %d\n", __FUNCTION__, __LINE__, volume&0xff);

    volume = ((volume&0xff)*9)/100;
	
	printf("%s:%d get: %d\n", __FUNCTION__, __LINE__, volume);
    return volume;
}


/**
 * param: CMD: send command
 * param: wait: 1: wait ack, 0: not wait.
 **/
int playback_Set_PIPE_With_CMD(UINT32 CMD, int wait)
{
	UINT32 datasize;
	publicPlayerCmdPacket_t *packet;

	if(timeOut_count>2) {
		printf("[%s:%d]PIPE Error more than 2 packet\n", __FUNCTION__, __LINE__);
		return -1;
	}
	//printf("Play_Set_PIPE_With_CMD  CMD=[%d]\n", CMD);
	datasize = sizeof(publicPlayerCmdPacket_t);
	packet =(publicPlayerCmdPacket_t *) malloc(datasize*sizeof(char));
	packet->infoID = CMD;
	packet->dataSize =0;
	int ret = sendPacket(packet,wait);
	free(packet);
	packet = NULL;
	if(ret == -1) {
		printf("[%s:%d]playback send packet error!\n", __FUNCTION__, __LINE__);
		printf("[%s:%d]ID:%d datasize:%d\n", __FUNCTION__, __LINE__, CMD, 0);
		timeOut_count++;
		ret = kill(g_cdvr->pid, 0);
		if(ret == -1 && errno == ESRCH) {
			printf("========child thread crash=========\n");
			g_cdvr->pid = -1;
			ret = -1;
			//playback_reboot(g_width, g_height);
		}
	}
	else {
		timeOut_count = 0;
	}
	return ret;
}

int playback_Set_PIPE(UINT32 CMD,char *data, UINT32 size)
{
	UINT32 datasize;
	publicPlayerCmdPacket_t *packet;
	if(timeOut_count>2) {
		printf("[%s:%d]PIPE Error more than 2 packet\n", __FUNCTION__, __LINE__);
		return -1;
	}

	//printf("Play_Set_PIPE CMD=[%d],size = [%d]\n", CMD,size);
	datasize = sizeof(publicPlayerCmdPacket_t);
	packet =(publicPlayerCmdPacket_t *) malloc(datasize*sizeof(char)+size);
	packet->infoID = CMD;
	packet->dataSize = size;
	//*(UINT32 *)((char *)packet + sizeof(publicPlayerCmdPacket_t) + 0) = mode;
	memcpy((char *)packet+sizeof(publicPlayerCmdPacket_t), data, size);
	int ret = sendPacket(packet, 1);
	free(packet);
	packet = NULL;
	if(ret == -1) {
		printf("[%s:%d]playback send packet error!\n", __FUNCTION__, __LINE__);
		printf("[%s:%d]ID:%d datasize:%d\n", __FUNCTION__, __LINE__, CMD, size);
		timeOut_count++;
		ret = kill(g_cdvr->pid, 0);
		if(ret == -1 && errno == ESRCH) {
			printf("========child thread crash=========\n");
			g_cdvr->pid = -1;
			//ret = 0;
			//playback_reboot(g_width, g_height);
		}
	}
	else {
		timeOut_count = 0;
	}
	return ret;
}


void playback_receive_cmd_callback(publicPlayerCmdPacket_t *pCmdPacket, UINT8 *pData, pthread_mutex_t *mutex_resp)
{
	publicResponse_t *w ;
	cdvr_info_t *pInfo = NULL;
	gp_size_t *size;
	int i, param;
	//printf("@@  pCmdPacket->infoID %d\n",pCmdPacket->infoID);
	if((PLAYBACK_CMD_MIN < pCmdPacket->infoID) && (pCmdPacket->infoID < PLAYBACK_CMD_MAX)){
		switch(pCmdPacket->infoID) {
			case CMD_GET_FILE_NAME:
				memset(filename, '\0', (128));
				memcpy(filename, (char *)pData, pCmdPacket->dataSize);
				break;
			case CMD_GET_FILE_PATH:
				memset(filepath, '\0', (128));
				memcpy(filepath, (char *)pData, pCmdPacket->dataSize);
				break;
			case CMD_GET_FILE_ALL_TYPE:
				param = *(UINT32*)pData;
				printf("get file type:0x%x\n", param);
				for(i=g_cdvr->cur_pageNum-1; i>=0; i--) {
					param = param >> 1;
					g_cdvr->disp_thmb[i].type = (param&0x01);
				}
				g_cdvr->fileType = g_cdvr->disp_thmb[g_cdvr->cur_idx].type;
				break;
			case CMD_GET_PAGE_LOCK:
				param = *(UINT32*)pData;
				for(i=g_cdvr->cur_pageNum-1; i>=0; i--) {
					param = param >> 1;
					g_cdvr->disp_thmb[i].lock= (param&0x01);
				}
				//g_cdvr->disp_thmb[g_cdvr->cur_idx].lock = *(UINT32 *)pData;
				break;
			case CMD_SET_LOCK_FILE_PB:
			case CMD_SET_UNLOCK_FILE:
			case CMD_SET_LOCK_ALL_FILE:
			case CMD_SET_UNLOCK_ALL_FILE:
			case CMD_SET_DELETE_FILE:
				break;
			case CMD_SET_DELETE_ALL_FILE:
				playback_set_menumode( PLAYBACK_MENU_WAIT_END );
				break;
			case CMD_GET_CVR_INFO:
				pInfo = (cdvr_info_t *)pData;
				dvr_sdvr_printfInfo(pInfo);
				g_cdvr->disp_number = pInfo->disp_number;
				g_cdvr->disp_page = pInfo->disp_page;
				g_cdvr->file_number = pInfo->file_number;
				g_cdvr->cur_idx = pInfo->cur_idx;
				g_cdvr->cur_page = pInfo->cur_page;
				g_cdvr->cur_pageNum = pInfo->cur_pageNum;
				g_cdvr->dispMode = pInfo->dispMode;
				g_cdvr->fileType = pInfo->fileType;
				break;
			case CMD_GET_JPEG_RESOURSE:
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				size = (gp_size_t*)pData;
				g_cdvr->srcSize.width = size->width;
				g_cdvr->srcSize.height = size->height;
				break;
			case CMD_GET_JPEG_SCLIDX:
				param = *(int *)pData;
				g_cdvr->sclIdx = param;
				break;
			case CMD_GET_PLAYINT_TIME:
				g_VideoEnd = *(UINT32*)pData;
				pData += 4;
				g_cdvr->pVideoInfo->play_time = *(SINT32*)pData;
				//printf("playback get: end: %d time: %ld\n", g_VideoEnd, g_cdvr->pVideoInfo->play_time);
				break;
			case CMD_SET_PLAY:
				param = *(int *)pData;
				//printf("cmd set play %d\n", param);
				break;
			case CMD_SET_PAUSE:
				param = *(int *)pData;
				if(param == 0 /*&& g_cdvr->pVideoInfo->videoMode == VIDEO_PLAY*/) {
					//g_cdvr->pVideoInfo->videoMode = VIDEO_PAUSE;
				}
			case CMD_SET_RESUME:
				param = *(int *)pData;
				if(param == 0 /*&& g_cdvr->pVideoInfo->videoMode == VIDEO_PAUSE*/) {
					//g_cdvr->pVideoInfo->videoMode = VIDEO_PLAY;
				}
				break;
			case CMD_SET_REVERSE_DISABLE:
				param = *(int *)pData;
				if(param == 0 /*&& g_cdvr->pVideoInfo->videoMode == VIDEO_PLAY*/) {
					//g_cdvr->pVideoInfo->videoMode = VIDEO_PAUSE;
					g_cdvr->pVideoInfo->videoMode = VIDEO_PLAY;
				}
				break;
			case CMD_SET_REVERSE_ENABLE:
				param = *(int *)pData;
				if(param == 0 /*&& g_cdvr->pVideoInfo->videoMode == VIDEO_PAUSE*/) {
					//g_cdvr->pVideoInfo->videoMode = VIDEO_PLAY;
					g_cdvr->pVideoInfo->videoMode = VIDEO_REVERSE;
				}
				break;
			case CMD_SET_SPEED:
				param = *(int *)pData;
				g_SetSpeed = param;
				break;
			case CMD_GET_FRAME_RATE:
				param = *(int *)pData;
				g_cdvr->pVideoInfo->frameRate = param;
				printf("get frame rate: %d\n", param);
				break;
			case CMD_SET_FULLSCREEN:
				g_cmd_ret = *(int *)pData;
				break;
			default:
				break;
		}
		w = public_isInlist(pCmdPacket->infoID);
		if(w != NULL){

			pthread_mutex_lock(mutex_resp);
			w->result = RESPONSE_OK;
			pthread_mutex_unlock(mutex_resp);
		}
		else {
			printf("can not find [%d] in list\n", pCmdPacket->infoID);
		}
	}

		
}

void playback_reboot(int w, int h)
{

	int i, idx=0;
	int disp = 0;
	pid_t cam_pid;
	char *argv[32];
	cdvr_thmb_t *pdvr = NULL;

	argv[idx++] = strdup("/system/app/cvrplayer");
	//argv[idx++] = strdup("/media/sdcardc1/cvrplayer");
	argv[idx++] = strdup("cvrplayer");
	
	argv[idx++] = strdup("-p");
	argv[idx++] = strdup(Memory_path);
	argv[idx++] = strdup("-d");
	argv[idx++] = (char *)malloc(32);
	/*if(HDMI_Check() == 1) { // for hdmi and tv check.
		disp = 6;
	}
	else if(TV_Check() == 1) {
		disp = 4;
	}
	else */{
		disp = 0;
	}
	sprintf(argv[idx-1], "%d", disp);
	
	argv[idx++] = (char *)0;
	pipeDelete();
	public_pipeinit(playback_receive_cmd_callback);

	signal(SIGCHLD, SIG_IGN);
	if((cam_pid = fork()) <0)
	{
		perror("creat pid fail\n");
		return ;
	}
	else if(cam_pid==0)
	{
		for(i=0; i<idx-1; i++) {
			printf("%s\n", argv[i]);
		}
		execv(argv[0], &argv[1]);
	}
	else {
		for(i=0; i<idx-1; i++) {
			free(argv[i]);
		}

		pdvr = g_cdvr;
		if (pdvr->dispMode == DISP_NO_FILE) {
			pdvr->pid = cam_pid;
    		printf("%s:%d no file mode\n", __FUNCTION__, __LINE__);
		}
		usleep(500*1000);

		dvr_thmb_set_info(pdvr);
		pdvr->pUrl = NULL;
		pdvr->pPath = NULL;
		pdvr->pVideoInfo->speed = 0;
		pdvr->sclIdx = 0;
		pdvr->pVideoInfo->volume = ExtGpVideoEngineGetVolume();
		if(pdvr->pVideoInfo->volume > 8) {
			pdvr->pVideoInfo->volume = 8;
		}
		pdvr->pVideoInfo->videoMode = VIDEO_OPEN;

		pdvr->pUrl = FilelistGetFileName(0);
		pdvr->pPath = FilelistGetFilePath(0);
		FilelistGetPageLock();

		if(pdvr->dispMode == DISP_THMB_SCREEN) {
			dvr_thmb_dispPage(g_cdvr);
		}
		else {
    		printf("%s:%d init cvrplayer to FullScreen mode\n", __FUNCTION__, __LINE__);
			dvr_thmb_dispFullScreen(g_cdvr);
		}

		pdvr->pid = cam_pid;
	}
	return ;
}

cdvr_thmb_t *playback_init(int disp, int width, int height)
{
	int i, idx=0;
	pid_t cam_pid;
	char *argv[32];
	cdvr_thmb_t *pdvr = NULL;
	g_width = width;
	g_height = height;
	g_SetSpeed = -1;
	timeOut_count = 0;

	argv[idx++] = strdup("/system/app/cvrplayer");
	//argv[idx++] = strdup("/media/sdcardc1/cvrplayer");
	argv[idx++] = strdup("cvrplayer");
	
	argv[idx++] = strdup("-p");
	argv[idx++] = strdup(Memory_path);

	if(disp != 7) {
	argv[idx++] = strdup("-d");
	argv[idx++] = (char *)malloc(32);
		sprintf(argv[idx-1], "%d", disp);

	}
	else { //for display 1 using PPU
		argv[idx++] = strdup("-d2");
		argv[idx] = strdup("0x00000000");
		sprintf(argv[idx++], "0x%08x", gpChunkMemVA2PA(dispmanager.addr)); //physical address
		argv[idx] = strdup("320"); //width
		sprintf(argv[idx++], "%d", dispmanager.buffer_width);
		argv[idx] = strdup("240"); //height
		sprintf(argv[idx++], "%d", dispmanager.buffer_height);
		argv[idx] = strdup("3"); //buffer number	
		sprintf(argv[idx++], "%d", dispmanager.buffer_num);

	}


	argv[idx++] = (char *)0;
	public_pipeinit(playback_receive_cmd_callback);
	signal(SIGCHLD, SIG_IGN);
	if((cam_pid = fork()) <0)
	{
		perror("creat pid fail\n");
		return NULL;
	}
	else if(cam_pid==0)
	{
		for(i=0; i<idx-1; i++) {
			printf("%s\n", argv[i]);
		}
		execv(argv[0], &argv[1]);
	}
	else {
		for(i=0; i<idx-1; i++) {
			free(argv[i]);
		}
		g_sys_vol = ExtGpVideoEngineGetVolume();
		g_cdvr = pdvr = dvr_thmb_init(width, height);
		if(!pdvr) {
    		printf("%s:%d\n", __FUNCTION__, __LINE__);
			return NULL;
		}
		usleep(200*1000);
    	printf("%s:%d\n", __FUNCTION__, __LINE__);

		do{
			dvr_thmb_get_info();
			if(pdvr->dispMode != -1) {
				printf("cvrplayer init ok!\n");
				break;
			}
			else {
				usleep(200 *1000);
			}
		}while(pdvr->dispMode == -1);

		if(pdvr->dispMode == DISP_FULL_SCREEN) {
    		//printf("%s:%d init cvrplayer to FullScreen mode\n", __FUNCTION__, __LINE__);
			//dvr_thmb_dispFullScreen(g_cdvr);
			playback_Set_PIPE_With_CMD(CMD_GET_FILE_ALL_TYPE, 1);
			FilelistGetPageLock();
		}
		else if (pdvr->dispMode == DISP_NO_FILE) {
    		printf("%s:%d no file mode\n", __FUNCTION__, __LINE__);
		}

		pdvr->pid = cam_pid;
	}
	return pdvr;
}


int playback_uninit(cdvr_thmb_t *pdvr)
{
	if(!pdvr) {
		return -1;
	}
	
	playback_Set_PIPE_With_CMD(CMD_SET_PLAYBACK_EXIT, 1);
	//photo_uninit(pdvr->pPhotoInfo); //for jpeg->video->jpeg
	g_vid_vol = pdvr->pVideoInfo->volume;;
	pdvr->pPhotoInfo = NULL;
	int status=0;
	if(pdvr->pid > 0)
		waitpid(pdvr->pid, &status, 0);
	Dsp_Open();
	ExtGpVideoEngineSetVolume(g_sys_vol);
	dvr_thmb_uninit(pdvr);
	pdvr = NULL;
	g_cdvr = NULL;
	pipeDelete();
	return 0;
}


int playback_up_key_active(cdvr_thmb_t *pdvr)
{
    printf("%s:%d dispmode %d\n", __FUNCTION__, __LINE__, pdvr->dispMode);
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) { // jpeg for scale up function
			dvr_thmb_setDispScaleMode(pdvr);
			//image_scale_up(pdvr->pPhotoInfo);
			playback_Set_PIPE_With_CMD(CMD_SET_SCALE_UP, 1);
			dvr_thmb_getScaleIndex(pdvr);
		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {
		}

	}
	else if(pdvr->dispMode == DISP_THMB_SCREEN){
		int curPage = pdvr->cur_page;
		//dvr_thmb_nextIdx(pdvr);
		dvr_thmb_preIdx(pdvr);
		if(curPage != pdvr->cur_page) {
			dvr_thmb_dispPage(pdvr);
		}
	}
	else if(pdvr->dispMode == DISP_VIDEO_SCREEN) {

		pdvr->pVideoInfo->volume += VIDEO_PLAY_VOLUME_STEP;
		if(pdvr->pVideoInfo->volume >= 8) {
			pdvr->pVideoInfo->volume = 8;
		}
		ExtGpVideoEngineSetVolume(pdvr->pVideoInfo->volume);
	}
	else if(pdvr->dispMode == DISP_SCALE_SCREEN) { //now only in photo
		playback_Set_PIPE_With_CMD(CMD_SET_SCALE_UP, 1);
		dvr_thmb_getScaleIndex(pdvr);
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
}


int playback_down_key_active(cdvr_thmb_t *pdvr)
{
    printf("%s:%d dispmode %d\n", __FUNCTION__, __LINE__, pdvr->dispMode);
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) { // jpeg for scale up function
			playback_Set_PIPE_With_CMD(CMD_GET_JPEG_SCLIDX, 1);
			if(pdvr->sclIdx == 0) { //will be in thumbnail mode
				//dvr_thmb_setDispThmbMode(pdvr); //enter thmb mode
				dvr_thmb_dispPage(pdvr);
			}
			else { //if not scale down
				playback_Set_PIPE_With_CMD(CMD_SET_SCALE_DOWN, 1);
				playback_Set_PIPE_With_CMD(CMD_GET_JPEG_SCLIDX, 1);
			}
		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {
			//dvr_thmb_setDispThmbMode(pdvr); //enter thmb mode
			dvr_thmb_dispPage(pdvr);
		}

	}
	else if(pdvr->dispMode == DISP_THMB_SCREEN){
		int curPage = pdvr->cur_page;
		//dvr_thmb_preIdx(pdvr);
		dvr_thmb_nextIdx(pdvr);
		if(curPage != pdvr->cur_page) {
			dvr_thmb_dispPage(pdvr);
		}

	}
	else if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
		pdvr->pVideoInfo->volume -= VIDEO_PLAY_VOLUME_STEP;
		if(pdvr->pVideoInfo->volume <= 0) {
			pdvr->pVideoInfo->volume = 0;
		}
		ExtGpVideoEngineSetVolume(pdvr->pVideoInfo->volume);
	}
	else if(pdvr->dispMode == DISP_SCALE_SCREEN) {
		playback_Set_PIPE_With_CMD(CMD_SET_SCALE_DOWN, 1);
		playback_Set_PIPE_With_CMD(CMD_GET_JPEG_SCLIDX, 1);
		if(pdvr->sclIdx == 0) {
			dvr_thmb_setDispFullMode(pdvr); //enter full mode
		}
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
}

int playback_up_L_key_active(cdvr_thmb_t *pdvr)
{
	int speed;
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) { // jpeg for scale up function
			//dvr_thmb_nextIdx(pdvr);
			dvr_thmb_preIdx(pdvr);
			dvr_thmb_dispFullScreen(pdvr);
		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {
			//dvr_thmb_nextIdx(pdvr);
			pdvr->pVideoInfo->frameRate = 0;
			dvr_thmb_preIdx(pdvr);
			dvr_thmb_dispFullScreen(pdvr);
		}

	}
	else if(pdvr->dispMode == DISP_THMB_SCREEN){
		dvr_thmb_prePage(pdvr);
		//dvr_thmb_nextPage(pdvr);
		dvr_thmb_dispPage(pdvr);
	}
	else if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
		if(pdvr->pVideoInfo->videoMode == VIDEO_PLAY ||
				pdvr->pVideoInfo->videoMode == VIDEO_SPEED) {
			speed = pdvr->pVideoInfo->speed;
			speed++;
			if(speed == VIDEO_PLAY_SPEED_STEP) {
				speed = VIDEO_PLAY_SPEED_STEP-1;
			}
			else {
				g_SetSpeed = -1;
				ExtGpVideoEngineSetSpeed(speed);
				if(g_SetSpeed != 0) {
					if(g_SetSpeed == -1) { //-2 is not set
						pdvr->pVideoInfo->speed = speed;
					}
					printf("%s:%d change speed to %d error ret: %d!!!\n", __FUNCTION__, __LINE__, speed, g_SetSpeed);
				}
				else {
					pdvr->pVideoInfo->videoMode = VIDEO_SPEED;
					pdvr->pVideoInfo->speed = speed;
				}
			}
		}
		else if(pdvr->pVideoInfo->videoMode == VIDEO_REVERSE){
			speed = pdvr->pVideoInfo->speed;
			speed--;
			if(speed == 0) {
				ExtGpVideoEngineReversePlay(0); //disable reversed play
				// disable reversed mode, if at the last sec, it can not 
				// change video mode.
				if(pdvr->pVideoInfo->videoMode == VIDEO_PLAY) {
					g_SetSpeed = -1;
					ExtGpVideoEngineSetSpeed(speed);
					if(g_SetSpeed != 0) {
						if(g_SetSpeed == -1) { //-2 is not set
							pdvr->pVideoInfo->speed = speed;
						}
						printf("%s:%d change speed to %d error ret: %d!!!\n", __FUNCTION__, __LINE__, speed, g_SetSpeed);
					}
					else {
						pdvr->pVideoInfo->speed = speed;
					}
				}
			}
			else {
				pdvr->pVideoInfo->videoMode = VIDEO_REVERSE;
				g_SetSpeed = -1;
				ExtGpVideoEngineSetSpeed(speed);
				if(g_SetSpeed != 0) {
					if(g_SetSpeed == -1) { //-2 is not set
						pdvr->pVideoInfo->speed = speed;
					}
					printf("%s:%d change speed to %d error ret: %d!!!\n", __FUNCTION__, __LINE__, speed, g_SetSpeed);
				}
				else {
					pdvr->pVideoInfo->speed = speed;
				}
			}
		}
	}
	else if(pdvr->dispMode == DISP_SCALE_SCREEN) {
		
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
	
}
int playback_down_L_key_active(cdvr_thmb_t *pdvr)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);

	int speed = 0;	
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) { // jpeg for scale up function
			//dvr_thmb_preIdx(pdvr);
			dvr_thmb_nextIdx(pdvr);
			dvr_thmb_dispFullScreen(pdvr);
		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {
			//dvr_thmb_preIdx(pdvr);
			pdvr->pVideoInfo->frameRate = 0;
			dvr_thmb_nextIdx(pdvr);
			dvr_thmb_dispFullScreen(pdvr);
		}

	}
	else if(pdvr->dispMode == DISP_THMB_SCREEN){
		//dvr_thmb_prePage(pdvr);
		dvr_thmb_nextPage(pdvr);
		dvr_thmb_dispPage(pdvr);
	}
	else if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
		printf("set speed: %d\n", pdvr->pVideoInfo->speed);
		if(pdvr->pVideoInfo->videoMode == VIDEO_PLAY ||
				pdvr->pVideoInfo->videoMode == VIDEO_REVERSE) {
			if(pdvr->pVideoInfo->videoMode == VIDEO_PLAY) {
				//call pipe to enter reversed mode in pipe revice cmd callback function, 
				//if set error, it will be in video play
				ExtGpVideoEngineReversePlay(1); //enter reversed play
			}
			if(pdvr->pVideoInfo->videoMode == VIDEO_REVERSE) {
				speed = pdvr->pVideoInfo->speed;
				speed++;
				if(speed == VIDEO_PLAY_SPEED_STEP) {
					speed = VIDEO_PLAY_SPEED_STEP-1;
				}
				else {
					g_SetSpeed = -1;
					ExtGpVideoEngineSetSpeed(speed);
					if(g_SetSpeed != 0) {
						if(g_SetSpeed == -1) { //-2 is not set
							pdvr->pVideoInfo->speed = speed;
						}
						printf("%s:%d change speed to %d error ret: %d!!!\n", __FUNCTION__, __LINE__, speed, g_SetSpeed);
					}
					else {
						pdvr->pVideoInfo->speed = speed;
					}
				}
			}
		}
		else if(pdvr->pVideoInfo->videoMode == VIDEO_SPEED){
			speed = pdvr->pVideoInfo->speed;
			speed--;
			g_SetSpeed = -1;
			ExtGpVideoEngineSetSpeed(speed);
			if(g_SetSpeed == 0) {
				pdvr->pVideoInfo->speed = speed;
				if(speed == 0) {
					pdvr->pVideoInfo->videoMode = VIDEO_PLAY;
				}
				else  {
					pdvr->pVideoInfo->videoMode = VIDEO_SPEED;
				}
			}
			else {
				if(g_SetSpeed == -1) { //-2 is not set
					pdvr->pVideoInfo->speed = speed;
				}
				printf("%s:%d change speed to %d error ret: %d!!!\n", __FUNCTION__, __LINE__, speed, g_SetSpeed);
			}
		}
	}
	else if(pdvr->dispMode == DISP_SCALE_SCREEN) {
		
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
}

/**
 * playback mode check video status.
 * return: 0: video not play, 1: vidoe playing.
 **/
int playback_mode_checkVideoStatus(cdvr_thmb_t *pdvr)
{
	if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
		return 1;  //video play mode
	}	
	else {
		return 0; //mode key
	}
}

/**
 * playback mode key active.
 * return: 0: do mode active, 1: change mode.
 **/
int playback_mode_key_active(cdvr_thmb_t *pdvr)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) { 
		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {

		}

	}
	else if(pdvr->dispMode == DISP_THMB_SCREEN) {
		
	}
	else if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
		if(pdvr->pVideoInfo->videoMode == VIDEO_PAUSE) {
			ExtGpVideoEngineResume();
		}
		ExtGpVideoEngineStop();
		g_VideoEnd = 0;
		pdvr->pVideoInfo->videoMode = VIDEO_OPEN;
		pdvr->pVideoInfo->speed = 0;
		ap_ppu_text_to_rgb();
		dvr_thmb_dispFullScreen(pdvr);
		//dvr_thmb_dispFullScreen_Replace(pdvr);
		printf("%s:%d\n", __FUNCTION__, __LINE__);
		Dsp_Open();
		ExtGpVideoEngineSetVolume(g_sys_vol);
		timeOut_count = 0;
		pthread_mutex_unlock(&g_cdvr->pb_mutex);
		return 1;
	}
	else if(pdvr->dispMode == DISP_SCALE_SCREEN) {
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
	return 0;
}
/**
 * menu key active.
 * return: 0: need do draw ui. -1:do not draw ui.
 * */
int playback_menu_key_active(cdvr_thmb_t *pdvr)
{
	int ret = 0;
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) { 
		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {

		}

		ret = 0;
	}
	else if(pdvr->dispMode == DISP_THMB_SCREEN) {
		ret = -1;
	}
	else if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
		ret = -1;
	}
	else if(pdvr->dispMode == DISP_SCALE_SCREEN) {
		dvr_thmb_dispFullScreen(pdvr);
		ret = -1;
	}

	pthread_mutex_unlock(&g_cdvr->pb_mutex);
	return ret;
}

int playback_enter_key_active(cdvr_thmb_t *pdvr)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&g_cdvr->pb_mutex);
	if(pdvr->dispMode == DISP_FULL_SCREEN) {
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) { 
		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {
			printf("%s:%d\n", __FUNCTION__, __LINE__);
			dvr_thmb_setDispVideoMode(pdvr);
			if(pdvr->pVideoInfo->videoMode == VIDEO_OPEN) {
				g_cdvr->pVideoInfo->play_time = 0;
				g_cdvr->pVideoInfo->show_time = 0;
				g_cdvr->pVideoInfo->speed = 0;
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				Dsp_close();
				ExtGpVideoEngineSetVolume(pdvr->pVideoInfo->volume);
				//ExtGpVideoEngineSetUrl(pdvr->pUrl);
				g_VideoEnd = 0;
				ap_ppu_text_to_vyuy();
				ExtGpVideoEnginePlay();
				pdvr->pVideoInfo->videoMode = VIDEO_PLAY;
			}
		}

	}
	else if(pdvr->dispMode == DISP_THMB_SCREEN){
			printf("%s:%d\n", __FUNCTION__, __LINE__);
		
		//dvr_thmb_setDispFullMode(pdvr);
		pdvr->pVideoInfo->videoMode = VIDEO_OPEN;
		pdvr->pVideoInfo->frameRate = 0;
		dvr_thmb_dispFullScreen(pdvr);
	}
	else if(pdvr->dispMode == DISP_VIDEO_SCREEN) {
			if(pdvr->pVideoInfo->videoMode == VIDEO_PAUSE) {
				//pdvr->pVideoInfo->videoMode = VIDEO_PLAY;
				pdvr->pVideoInfo->videoMode = pdvr->pVideoInfo->lastMode;
				pdvr->pVideoInfo->lastMode = -1;
				ExtGpVideoEngineResume();
			}
			else if(pdvr->pVideoInfo->videoMode == VIDEO_PLAY || pdvr->pVideoInfo->videoMode == VIDEO_SPEED||
					pdvr->pVideoInfo->videoMode == VIDEO_REVERSE) {
				pdvr->pVideoInfo->lastMode = pdvr->pVideoInfo->videoMode;
				pdvr->pVideoInfo->videoMode = VIDEO_PAUSE;
				ExtGpVideoEnginePause();
			}
	}
	else if(pdvr->dispMode == DISP_SCALE_SCREEN) { //need to change to dv mode
		
	}
	pthread_mutex_unlock(&g_cdvr->pb_mutex);
}

void dvr_thmb_printfInfo(cdvr_thmb_t *pdvr)
{
#if 0
	printf("-------cur info---------\n");
	printf("FileNum: %d curIdx %d curPage %d pageNum %d\n", pdvr->file_number, pdvr->cur_idx, pdvr->cur_page, pdvr->cur_pageNum);
	printf("file(%s):%s\n","unlock", pdvr->pUrl);
	printf("------------------------\n");
#endif
}

void dvr_sdvr_printfInfo(cdvr_info_t *pInfo)
{
#if 1
	printf("----------cdvr info-------------\n");
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
