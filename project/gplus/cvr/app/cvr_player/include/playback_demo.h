#ifndef __PLAYBACK_DEMO_H__
#define __PLAYBACK_DEMO_H__
#include "disp.h"
#include "libMcpThread/mcp_thread.h"
#include "libMcpThread/mcp_queue.h"

#include "file_list.h"



#define SCALE_TAB 16
#define DISP_BUFFRER_NUM 3

#define GP_FILE_TYPE_UNKNOWN (-1)
#define GP_FILE_TYPE_VIDEO	(0)
#define GP_FILE_TYPE_JPEG	(1)

#define DEF_THMB_EDGE_W (10)
#define DEF_THMB_EDGE_H (10)

#define DEF_THMB_WIDTH (30)
#define DEF_THMB_HEIGHT (30)
#define DEF_THMB_ROW (3)
#define DEF_THMB_COLUMN (3)

//#define DISP_FULL_SCREEN 0
//#define DISP_THMB_SCREEN 1
enum display_mode{
	DISP_FULL_SCREEN = 0,
	DISP_THMB_SCREEN,
	DISP_SCALE_SCREEN,
	DISP_VIDEO_SCREEN,
	DISP_NO_FILE
};

enum {
	PLAYBACK_MENU_UNKNOWN = -1,
	PLAYBACK_MENU_DELETE_NONE = 1,
	PLAYBACK_MENU_DELETE_CUR_S,
	PLAYBACK_MENU_DELETE_ALL_S,
	PLAYBACK_MENU_DELETE_CUR,
	PLAYBACK_MENU_DELETE_ALL,
	PLAYBACK_MENU_LOCK_CUR,
	PLAYBACK_MENU_LOCK_ALL,
	PLAYBACK_MENU_UNLOCK_CUR,
	PLAYBACK_MENU_UNLOCK_ALL,
	PLAYBACK_MENU_LOCK_CUR_S,
	PLAYBACK_MENU_LOCK_ALL_S,
	PLAYBACK_MENU_UNLOCK_CUR_S,
	PLAYBACK_MENU_UNLOCK_ALL_S,	
	PLAYBACK_SETTING_FORMAT = 0x10
};

typedef struct disp_thmb_s {
	char *url;		/*file name*/
	int type; 		/*movie or jpeg*/
	int index;		/*screen disp index: 0~row*column*/
	int row;		/*row in the screen*/
	int column;		/*column int the screen*/
	int x;			/*show x*/
	int y;			/*show y*/
	gp_bitmap_t bitmap; /*bitmap data of the file*/
	int lock;		/*this file is lock/unlock*/
}disp_thmb_t;

enum video_play_mode {
	VIDEO_OPEN = 0,
	VIDEO_PLAY,
	VIDEO_PAUSE,
	VIDEO_SPEED,
	VIDEO_REVERSE
};

typedef struct video_Info_s {
	int videoMode; /*video mode: open_mode, play_mode, pause_mode, slow_mode, fast_mode*/
	UINT64 total_time;
	SINT32 play_time; /*video play time*/
	SINT32 volume;
	int speed; // 1x 2x 4x 8x 
	gp_size_t srcSize; /*picture size*/
	int frameRate;
}video_Info_t;

typedef struct photo_Info_s {
	gp_size_t srcSize; /*picture size*/
	UINT8* pSrcData; /*picture decode data*/
	UINT8 sclIdx; /*photo scale index. for scaleTable[]*/
    gp_bitmap_t *bitmap; /*photo display bitmap*/
}photo_Info_t;

typedef struct cdvr_info_s {
	int disp_number; /*how many thmb show in screen*/
	int disp_page; /*page number of all file*/
	int file_number; /*all number of file*/
	int cur_idx; /*cur select index*/
	int cur_page; /*current page: cur_page*disp_number+cur_idx*/ 
	int cur_pageNum; /*all number of file in this page*/
	int dispMode; /*playback show in full mode or thmb mode*/
    int fileType; /*file type: video file or photo*/
}cdvr_info_t;
/**
 * car dvr struct.
 **/
typedef struct cdvr_thmb_s {
	disp_thmb_t disp_thmb[DEF_THMB_ROW * DEF_THMB_COLUMN];
	gp_bitmap_t disp_bitmap; //for show in full screen
	int disp_number; /*how many thmb show in screen*/
	int disp_page; /*page number of all file*/
	int file_number; /*all number of file*/
	int cur_idx; /*cur select index*/
	int cur_page; /*current page: cur_page*disp_number+cur_idx*/ 
	int cur_pageNum; /*all number of file in this page*/
	char* upFrame; /*1: user need to up data frame, 0: do not*/
	int dispMode; /*playback show in full mode or thmb mode*/
    int fileType; /*file type: video file or photo*/
    char *pUrl; /*curren display file*/
    photo_Info_t *pPhotoInfo;
	video_Info_t *pVideoInfo;
	gp_file_t *pFile;
	char fileStat; //for  cvr_dual,: 0: pre sensor file, 1: back sensor file
	HANDLE hDisp;
}cdvr_thmb_t;


typedef struct display_buffer_s{
	int address; //buffer address
	int DispDev;
	int DispNum;
	int Width;
	int Height;
	int DispBuffer[DISP_BUFFRER_NUM];
	mcp_mbox_t BufferMBox;
}display_buffer_t;

//filelist

int FilelistSetFilePath(char *path);
gp_file_t *FilelistGetFileNum(cdvr_thmb_t *pdvr, gp_file_t *pFile, int reFlash);
char *FilelistGetFileName(gp_file_t *pFile, int index);
char *FilelistGetFilePath(gp_file_t *pFile, int index);
int FilelistGetLock(gp_file_t *pFile, int index);
int FilelistSetLock(gp_file_t *pFile, int index);
int FilelistSetLockAll(gp_file_t *pFile);
int FilelistSetUnLock(gp_file_t *pFile, int index);
int FilelistSetUnLockAll(gp_file_t *pFile);

int dvr_thmb_setFileNumber(cdvr_thmb_t *pdvr, int file_num, int reset);
int dvr_thmb_getDispMode(cdvr_thmb_t *pdvr);
int dvr_thmb_getCurIdxOfFile(cdvr_thmb_t *pdvr);
int dvr_thmb_dispFullScreen(cdvr_thmb_t *pdvr, char *disp_pData);
int dvr_thmb_dispFlip(cdvr_thmb_t *pdvr);
int dvr_thmb_getFileInfo(cdvr_thmb_t *pdvr, char *buf);
int dvr_thmb_getFileResourse(cdvr_thmb_t *pdvr, gp_size_t *size);
int dvr_thmb_getPlayingTime(cdvr_thmb_t *pdvr);
int dvr_thmb_dispPage(cdvr_thmb_t *pdvr, int page, char *disp_pData);
int dvr_thmb_deleteFile(cdvr_thmb_t *pdvr, gp_file_t **ppFile);
int dvr_thmb_deleteAllFile(cdvr_thmb_t *pdvr, gp_file_t **ppFile);
int dvr_thmb_preIdx(cdvr_thmb_t *pdvr);
int dvr_thmb_nextIdx(cdvr_thmb_t *pdvr);
int dvr_thmb_prePage(cdvr_thmb_t *pdvr);
int dvr_thmb_nextPage(cdvr_thmb_t *pdvr);
int dvr_thmb_getCvrInfo(cdvr_thmb_t *pdvr, cdvr_info_t *pCvrInfo);
int dvr_thmb_setDispMode(cdvr_thmb_t *pdvr, int mode);
int dvr_thmb_getDispMode(cdvr_thmb_t *pdvr);
char *dvr_thmb_getFileName(cdvr_thmb_t *pdvr, gp_file_t *pFile);

// for playback mode
cdvr_thmb_t *playback_init(int disp);
int playback_uninit(cdvr_thmb_t *pdvr);
int playback_up_key_active(cdvr_thmb_t *pdvr);
int playback_down_key_active(cdvr_thmb_t *pdvr);
int playback_up_L_key_active(cdvr_thmb_t *pdvr);
int playback_up_L_key_active(cdvr_thmb_t *pdvr);
int playback_down_L_key_active(cdvr_thmb_t *pdvr);
int playback_mode_checkVideoStatus(cdvr_thmb_t *pdvr);
int playback_mode_key_active(cdvr_thmb_t *pdvr);
int playback_menu_key_active(cdvr_thmb_t *pdvr);
int playback_enter_key_active(cdvr_thmb_t *pdvr);

void dvr_thmb_printfInfo(cdvr_thmb_t *pdvr);
void dvr_sdvr_printfInfo(cdvr_info_t *pInfo);
int get_idle_buffer(int arg);

int send_ready_buffer(int arg);

#endif
