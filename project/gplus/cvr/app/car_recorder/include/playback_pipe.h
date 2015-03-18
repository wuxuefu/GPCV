#ifndef __PLAYBACK_DEMO_H__
#define __PLAYBACK_DEMO_H__
#include "disp.h"

#define CVR_DUAL 0

#define SCALE_TAB 16

#define GP_FILE_TYPE_UNKNOWN (-1)
#define GP_FILE_TYPE_VIDEO	(0)
#define GP_FILE_TYPE_JPEG	(1)

#define DEF_THMB_EDGE_W (10)
#define DEF_THMB_EDGE_H (10)

#define DEF_THMB_WIDTH (30)
#define DEF_THMB_HEIGHT (30)
#define DEF_THMB_ROW (3)
#define DEF_THMB_COLUMN (3)

#define GP_FILE_LOCK 1
#define GP_FILE_UNLOCK 0

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
	PLAYBACK_MENU_WAIT,
	PLAYBACK_MENU_WAIT_END,
	PLAYBACK_SETTING_FORMAT = 0x10
};

enum{
	PLAYBACK_CMD_MIN=0,
	CMD_SET_FULLSCREEN,
	CMD_SET_THMBSCREEN,
	CMD_SET_PRE_INDEX,
	CMD_SET_NEXT_INDEX,
	CMD_SET_PRE_PAGE,
	CMD_SET_NEXT_PAGE,
	CMD_SET_SCALE_UP,
	CMD_SET_SCALE_DOWN,
	CMD_SET_PLAY,
	CMD_SET_PAUSE, //10
	CMD_SET_RESUME,
	CMD_SET_STOP,
	CMD_SET_SPEED,
	CMD_SET_VOLUME,
	CMD_GET_VOLUME,
	CMD_SET_REVERSE_ENABLE,
	CMD_SET_REVERSE_DISABLE,
	CMD_GET_PLAYINT_TIME,
	CMD_GET_JPEG_RESOURSE,
	CMD_GET_JPEG_SCLIDX, //20

	CMD_GET_FILE_NAME,
	CMD_GET_FILE_PATH,
	CMD_GET_FILE_ALL_TYPE,
	CMD_GET_CVR_INFO,
	CMD_SET_CVR_INFO,
	CMD_SET_DELETE_FILE,
	CMD_SET_DELETE_ALL_FILE,
	CMD_SET_LOCK_FILE_PB,
	CMD_SET_LOCK_ALL_FILE,
	CMD_SET_UNLOCK_FILE, //30
	CMD_SET_UNLOCK_ALL_FILE,
	CMD_GET_PAGE_LOCK,
	CMD_SET_DISP_MODE,
	CMD_SET_PLAYBACK_EXIT,
	CMD_SET_REBUILD_FILELIST,
	CMD_SET_FULLSCREEN_REPLACE,
	CMD_GET_FRAME_RATE,
	CMD_SET_RECOVER_FILE,
	CMD_GET_FILE_DUAL_IDX,
	PLAYBACK_CMD_MAX
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
	VIDEO_ERROR = -1,
	VIDEO_OPEN = 0,
	VIDEO_PLAY,
	VIDEO_PAUSE,
	VIDEO_SPEED,
	VIDEO_REVERSE
};

typedef struct video_Info_s {
	int videoMode; /*video mode: open_mode, play_mode, pause_mode, slow_mode, fast_mode*/
	int lastMode;
	UINT64 total_time;
	SINT32 play_time; /*video play time*/
	SINT32 show_time;
	SINT32 volume;
	int speed; // 1x 2x 4x 8x 
	int frameRate;
}video_Info_t;

typedef struct photo_Info_s {
	gp_size_t srcSize; /*picture size*/
	UINT8* pSrcData; /*picture decode data*/
    UINT8* pSclData; /*scale buffer address*/
	gp_rect_t scl_rect; /*scale size, and begin display point*/
    gp_point_t pxy; /*scale point x y */
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
	int disp_number; /*how many thmb show in screen*/
	int disp_page; /*page number of all file*/
	int file_number; /*all number of file*/
	int cur_idx; /*cur select index*/
	int cur_page; /*current page: cur_page*disp_number+cur_idx*/ 
	int cur_pageNum; /*all number of file in this page*/
	char dispMode; /*playback show in full mode or thmb mode*/
    char fileType; /*file type: video file or photo*/
	UINT8 sclIdx; /*photo scale index. for scaleTable[]*/
	char fileStat;
    char *pUrl; /*curren display file*/
    char *pPath; /*curren display file*/
	gp_size_t srcSize; /*picture size*/
    photo_Info_t *pPhotoInfo;
	video_Info_t *pVideoInfo;
	HANDLE hDisp;
	pid_t pid;
	pthread_mutex_t pb_mutex;
	struct tm FileTime;
}cdvr_thmb_t;

//filelist

int dvr_thmb_mutex_lock(void);
int dvr_thmb_mutex_unlock(void);

int FilelistGetFileNum(int reFlash);
char *FilelistGetFileName(int index);
char *FilelistGetFilePath(int index);
int FilelistGetLock(int index);
int FilelistGetPageLock(void);
int FilelistSetLock(int index);
int FilelistSetLockAll(void);
int FilelistSetUnLock(int index);
int FilelistSetUnLockAll(void);
int FilelistRebuild(void);
int FilelistSetDelete();
int FilelistSetDeleteAll();

int dvr_thmb_setFileNumber(cdvr_thmb_t *pdvr, int file_num);
int dvr_thmb_getDispMode(cdvr_thmb_t *pdvr);
int dvr_thmb_getCurIdxOfFile(cdvr_thmb_t *pdvr);
int dvr_thmb_dispFullScreen(cdvr_thmb_t *pdvr);
int dvr_thmb_dispFlip(cdvr_thmb_t *pdvr);
int dvr_thmb_getFileInfo(cdvr_thmb_t *pdvr, char *buf);
int dvr_thmb_getFileResourse(cdvr_thmb_t *pdvr, gp_size_t *size);
int dvr_thmb_getPlayingTime(void);
int dvr_thmb_dispPage(cdvr_thmb_t *pdvr);
int dvr_thmb_get_info(void);
int dvr_check_video_playing(void);
int dvr_thmb_nextIdx(cdvr_thmb_t *pdvr);
int dvr_thmb_getFrameRate(cdvr_thmb_t *pdvr);

// for playback mode
cdvr_thmb_t *playback_init(int disp, int w, int h);
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

int playback_Set_PIPE_With_CMD(UINT32 CMD, int wait);
int playback_Set_PIPE(UINT32 CMD,char *data, UINT32 size);

SINT32 ExtGpVideoEngineGetCurTime();
SINT32 ExtGpVideoEngineGetVolume(void);
void ExtGpVideoEngineSetVolume(int nGotoVol);
SINT32 ExtGpVideoEngineStop(void);

#endif
