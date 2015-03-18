#ifndef _DISP_H
#define _DISP_H

#include "mach/typedef.h"
#include <config/sysconfig.h>
#define RGB565_DRAW		0x2
#define COLOR256_DRAW		0x3
#define COLOR16_DRAW		0x4
#define BUFFER_COLOR_TYPE	RGB565_DRAW	

#define TIME_LAPSE_ENABLE//


#define PAGE_ITEM_NUM	4
#define TRANSPARENT_COLOR 0x8c71
#define FONT_COLOR			0xffff
#define FONT_SELECT_COLOR	0x001f
#ifdef TIME_LAPSE_ENABLE			
#define DV_MENU_NUM		10//add Time_Lapse
#else
#define DV_MENU_NUM		9
#endif
#ifdef SYSCONFIG_DISP0_TVOUT
#define DV_SETTING_NUM		11
#else
#define DV_SETTING_NUM		10
#endif
#define DC_MENU_NUM		11
#define PLAY_MENU_NUM		3
#define MAX_ITEM 			21

//#define Icon_path "/media/sdcardc/resource/icon/"
#define Icon_path "/system/resource/icon/"

#define ICON_LEFT_X_POS		8
#define ICON_LEFT_Y_POS		8
#define UI_MENU_X_POS			0//80//36
#define UI_MENU_Y_POS			0//16//20
#define X_STEP					16
#define Y_STEP					40
#define ICON_WIDTH				32
#define ICON_HEIGHT				32
#define ICON_RIGHT_X_POS		ICON_LEFT_X_POS +  	ICON_WIDTH
#define ICON_RIGHT_Y_POS		ICON_LEFT_Y_POS +	 ICON_HEIGHT

typedef struct iconManager_s {
	char icon_name[20];
	UINT16 icon_Index;
	UINT16 icon_width;
	UINT16 icon_height;
	UINT16 x_pos;
	UINT16 y_pos;	
	UINT16 tran_color; //transparent color
	UINT32 *addr;
	UINT16 isexist;
} iconManager_t;

typedef enum {
	ICON_VIDEO	= 0					,
	ICON_VIDEO_M          ,
	ICON_LOOPRECORDING    ,
	ICON_LOOPRECORDING2   ,
	ICON_LOOPRECORDING3   ,
	ICON_LOOPRECORDING5   ,
	ICON_MOTIONDETECT_F   ,
	ICON_MOTIONDETECT_M   ,
	ICON_VIDEO_EXPOSURE         ,
	ICON_CAP_EXPOSURE	,
	ICON_EXPOSURE_P2_0    ,
	ICON_EXPOSURE_P5_3    ,
	ICON_EXPOSURE_P4_3    ,
	ICON_EXPOSURE_P1_0    ,
	ICON_EXPOSURE_P2_3    ,
	ICON_EXPOSURE_P1_3    ,
	ICON_EXPOSURE_P0_0    ,
	ICON_EXPOSURE_N1_3    ,
	ICON_EXPOSURE_N2_3    ,
	ICON_EXPOSURE_N1_0    ,
	ICON_EXPOSURE_N4_3    ,
	ICON_EXPOSURE_N5_3    ,
	ICON_EXPOSURE_N2_0    ,
	ICON_WDR_F            ,
	ICON_WDR_M            ,
	ICON_SDC              ,
	ICON_INT              ,
	ICON_CHARGE           ,
	ICON_BATTERY0         ,
	ICON_BATTERY1         ,
	ICON_BATTERY2         ,
	ICON_BATTERY3         ,
	ICON_BATTERYX         ,
	ICON_REDLIGHT         ,
	ICON_VIDEO_RESOLUTION ,
	ICON_CAP_RESOLUTION   ,
	ICON_RECORD_AUDIO     ,
	ICON_VIDEO_DATE_STAMP ,
	ICON_GSENSOR          ,
	ICON_MENU             ,
	ICON_UP               ,
	ICON_DOWN             ,
	ICON_LEFT             ,
	ICON_RIGHT            ,
	ICON_DATE_TIME_SET    ,
	ICON_BEEP_SOUND       ,
	ICON_LANGUAGE         ,
	ICON_TV_MODE          ,
	ICON_FREQUENCY        ,
	ICON_SCREEN_SAVER     ,
	ICON_IR_LED           ,
	ICON_FORMAT           ,
	ICON_DEFAULT_SET      ,
	ICON_VERSION          ,
	ICON_CAPTURE          ,
	ICON_CAPTURE_M        ,
	ICON_CAPTURE_M1,
	ICON_AWB_M            ,
	ICON_AWB_F            ,
	ICON_DAYLIGHT         ,
	ICON_CLOUDY           ,
	ICON_TUNGSTEN         ,
	ICON_FLUORESCENT      ,
	ICON_CAP_2S_TIMER     ,
	ICON_CAP_5S_TIMER     ,
	ICON_CAP_10S_TIMER    ,
	ICON_SEQUENCE         ,
	ICON_SEQUENCE_F         ,	
	ICON_QUALITY          ,
	ICON_STAR_1           ,
	ICON_STAR_2           ,
	ICON_STAR_3           ,
	ICON_SHARPNESS        ,
	ICON_COLOR            ,
	ICON_ISO              ,
	ICON_ISO_AUTO         ,
	ICON_ISO_100          ,
	ICON_ISO_200          ,
	ICON_ISO_400          ,
	ICON_ANTI_SHAKING_M,
	ICON_ANTI_SHAKING     ,
	ICON_NO_ANTI_SHAKING  ,
	ICON_CAP_DATE_STAMP   ,
	ICON_SETUP            ,
	ICON_PLAYBACK_F       ,
	ICON_PLAYBACK_M       ,
	ICON_DELETE           ,
	ICON_PROTECT          ,
	ICON_PROTECT_F        ,
	ICON_SLIDE_SHOW       ,
	ICON_PLAY_START       ,
	ICON_PLAY_PAUSE       ,
	ICON_VOLUME           ,
	ICON_VOLUME1          ,
	ICON_VOLUME2          ,
	ICON_SELECT           ,
	ICON_DATE_D,
	ICON_DATE_Y,
	ICON_ITEM_B,
	ICON_ITEM_S,
	ICON_MENU_B,
	ICON_MENU_S,
	ICON_NOFILE,
	ICON_PLAY_S,
	ICON_TOP_S,
	ICON_PLAYBACK_P,
	ICON_MIC_ON,
	ICON_MIC_OFF,
	ICON_GSENSOR1          ,
	ICON_LDW,
	ICON_LDW_1,
	ICON_TIMELAPSE,
	ICON_MAX	
} UI_ICON_NUM;

typedef enum {
	DV_RESOLUTION=0,
	DV_LOOPRECORDING,
	DV_WDR,
	DV_EXPOSURE,
	DV_MOTIONDETECTION,
	DV_AUDIO,
	DV_DATESTAMP,
	DV_GSENSOR,
	DV_LDW,
	DV_TIMELAPSE
} DV_MENU_ENUM;
typedef enum {
	DC_CAPTURE_MODE=0,
	DC_RESOLUTION,
	DC_SEQUENCE,
	DC_QUALITY,
	DC_SHARPNESS,
	DC_WHITE_BALANCE,
	DC_COLOR,
	DC_ISO,
	DC_EXPOSURE,
	DC_ANTI_SHAKING,
	DC_DATE_STAMP
	
} DC_MENU_ENUM;

typedef enum {
	SET_GSENSOR1=0,
	SET_DATA_FORMAT,
	SET_SOUND,
	SET_LANGUAGE,
	#ifdef SYSCONFIG_DISP0_TVOUT
	SET_TV_MODE,
	#endif
	SET_FREQUENCY,
	SET_SCREEN_SAVER,
	SET_IR_LED,
	SET_FORMAT,
	SET_DEFAULT_SETTING,
	SET_VERSION
} SETTING_MENU_ENUM;

enum {
//=== Multi-language string item must be the first one, above this line. ===
	STR_MULTI_LANGUAGE=0		,	
	STR_VIDEO             ,
	STR_CAPTURE           ,
	STR_PLAYBACK          ,
	STR_SETUP             ,
	STR_RESOLUTION        ,
	STR_LOOPRECORDING     ,
	STR_WDR               ,
	STR_EXPOSURE          ,
	STR_MOTIONDETECT      ,
	STR_RECORD_AUDIO      ,
	STR_DATE_STAMP        ,
	STR_GSENSOR           ,
	STR_1080FHD           ,
	STR_720P60FPS         ,
	STR_720P30FPS         ,
	STR_WVGA              ,
	STR_VGA               ,
	STR_2_MIN             ,
	STR_3_MIN             ,
	STR_5_MIN             ,
	STR_ON_ALLTIME        ,
	STR_P2_0              ,
	STR_P5_3              ,
	STR_P4_3              ,
	STR_P1_0              ,
	STR_P2_3              ,
	STR_P1_3              ,
	STR_P0_0              ,
	STR_N1_3              ,
	STR_N2_3              ,
	STR_N1_0              ,
	STR_N4_3              ,
	STR_N5_3              ,
	STR_N2_0              ,
	STR_OFF               ,
	STR_ON                ,
	STR_HIGH              ,
	STR_MIDDLE            ,
	STR_LOW               ,
	STR_CAPRUTE_MODE      ,
	STR_SEQUENCE          ,
	STR_QUALITY           ,
	STR_SHARPNESS         ,
	STR_WRITE_BALANCE     ,
	STR_COLOR             ,
	STR_ISO               ,
	STR_ANTI_SHAKING      ,
	STR_SINGLE            ,
	STR_2S_TIMER          ,
	STR_5S_TIMER          ,
	STR_10S_TIMER         ,
	STR_12M               ,
	STR_10M               ,
	STR_8M                ,
	STR_5M                ,
	STR_3M                ,
	STR_2MHD              ,
	STR_1_3M              ,
	STR_FINE              ,
	STR_NORMAL            ,
	STR_ECONOMY           ,
	STR_STRONG            ,
	STR_SOFT              ,
	STR_AUTO              ,
	STR_DAYLIGHT          ,
	STR_CLOUDY            ,
	STR_TUNGSTEN          ,
	STR_FLUORESCENT       ,
	STR_BLACK_WHITE       ,
	STR_SEPIA             ,
	STR_ISO100            ,
	STR_ISO200            ,
	STR_ISO400            ,
	STR_DATE_ONLY         ,
	STR_DATE_TIME         ,
	STR_DELETE            ,
	STR_PROTECT           ,
	STR_SLIDE_SHOW        ,
	STR_DELETE_CURRENT    ,
	STR_DELETE_ALL        ,
	STR_ERASE,
	STR_ERASE1,
	STR_CANCEL            ,
	STR_OK                ,
	STR_LOCK_CURRENT      ,
	STR_UNLOCK_CURRENT    ,
	STR_LOCK_ALL          ,
	STR_UNLOCK_ALL        ,
	STR_2_SECOND          ,
	STR_5_SECOND          ,
	STR_8_SECOND          ,
	STR_BEEP_SOUND        ,
	STR_LANGUAGE          ,
	STR_TV_MODE           ,
	STR_FREQUENCY         ,
	STR_SCREEN_SAVER      ,
	STR_IR_LED            ,
	STR_FORMAT            ,
	STR_DEFAULT_SETTING   ,
	STR_VERSION           ,
	STR_NTSC              ,
	STR_PAL               ,
	STR_50HZ              ,
	STR_60HZ              ,
	STR_10_MIN            ,
	STR_SD_CARD           ,
	STR_FORMAT_SHOW1      ,
	STR_FORMAT_SHOW2      ,
	STR_INSERT_SDC        ,
	STR_NO_CARD           ,
	STR_MASS_STORAGE      ,
	STR_PC_CAMERA         ,
	STR_NO_FILE           ,
	STR_NO_POWER             ,
	STR_PARKING_MODE              ,
	STR_SDC_ERROR              ,
	STR_SEDAN_CAR              ,
	STR_SUV              ,//commercial vehicles
	STR_TRUCK              ,
	STR_LDW             ,
	STR_FILE_PROTECT              ,
	STR_NUM8              ,
	STR_NUM9              ,
	STR_SD_FULL           ,
	STR_PLEASE_WAIT       ,
	STR_USB               ,
	STR_ON_OFF,
	STR_CAR_TYPE,
	STR_SENSITIVITY,
	STR_AREA_CHOICE,
	STR_CHINA,
	STR_TAIWAN,
	STR_SRART_SPEED,
	STR_HIGH_SPEED,
	STR_LOW_SPEED,
	STR_TIME_LAPSE,
	STR_TIME_LAPSE_100MS,
	STR_TIME_LAPSE_200MS,
	STR_TIME_LAPSE_500MS,	
	STR_LOWLOW_SPEED,	
	STR_VERYHIGH,
	STR_LDW_SOUND,
	STR_MAX
};


typedef struct menu_item_s{
	UINT16 str_index;
	UINT16 xPos;															/** item x offset **/
	UINT16 yPos;															/** item y offset **/
}menu_item_t;
typedef struct menuManager_s {
	UINT16 icon_Index;
	UINT16 str_index;	//坐标为前面icon x坐标加上偏移量即可
	UINT16 color; //draw color
	UINT16 item_num;
	menu_item_t item[MAX_ITEM];
} menuManager_t;

typedef enum
{
    CMD_VIDEO_MODE = 0,
    CMD_STILL_MODE,	
    CMD_PLAYBACK_MODE,
    CMD_USB_MODE,
    CMD_POWEROFF_MODE,
    DV_FOREGROUND_MODE = 0x10,
    DV_MENU_MODE,	
    DV_SETTING_MODE,
	DV_MENU_KEY = 0x20,
	DV_MODE_KEY,
	DV_ENTER_KEY,
	DV_DOWN_KEY,
	DV_DOWN_L_KEY,
	DV_UP_KEY,
	DV_UP_L_KEY,
    DV_EXIT_MODE,
    DV_MAX_MODE
    
} PLAYMODE_ENUM;

typedef enum
{
	SOUND_KEY = 0,
	SOUND_CAMERA,
	SOUND_POWER_OFF,
	SOUND_LDW_ALRAM,
	SOUND_LDW_FCW,
	SOUND_LDW_STOPALRAM,
	SOUND_LDW_GOALRAM
	
} SOUND_TYPE_ENUM;


typedef struct LDW_DISPLINE_S
{
	int LT_alarmP_X;
	int LT_alarmP_Y;
	int LB_alarmP_X;
	int LB_alarmP_Y;	

	int RT_alarmP_X;
	int RT_alarmP_Y;
	int RB_alarmP_X;
	int RB_alarmP_Y;

	int LTP_X;//green
	int LTP_Y;
	int LBP_X;
	int LBP_Y;

	int RTP_X;
	int RTP_Y;
	int RBP_X;
	int RBP_Y;

	int LLcheckFlg;
	int RLcheckFlg;
	int LLAlarmFlg;
	int RLAlarmFlg;


} LDW_DISPLINE_t;

enum{
	DISP_CMD_MIN=0,
	DISP_BUF_READY,
	DISP_BUF_FREE,
	DISP_BUF_READ_UI,
	DISP_CMD_MAX
};
	
#define LDW_LINEDISP	0//1//disp LDW CheckLine && Alarm Line


#define DISP_LAYER_PRIMARY 0
#define DISP_LAYER_OSD 1

UINT32 dispCreate(HANDLE *pHandle, UINT32 layer,UINT8 dis_type);
UINT32 dispDestroy(HANDLE handle);
void dispGetResolution(HANDLE handle, gp_size_t *resolution);
UINT8* dispGetFramebuffer(HANDLE handle);
void dispFlip(HANDLE handle);
void dispDisablePrimary(HANDLE handle);
void dispUpDataPrimary( HANDLE handle, gp_bitmap_t *fb);
UINT8* dispGetFramebuffer( HANDLE handle);
void dispCleanFramebuffer(HANDLE handle);
UINT32 dispLayerEnable(HANDLE handle);


int gp_block_cpy(void *destbuf, int x_dest, int y_dest,int widthdest, int heightdest, void *srcbuf,  int widthsrc, int heightsrc,UINT16 trancolor,UINT16 background_color);
SINT32 gp_Icon_draw(void *destbuf,int x_dest, int y_dest,int widthdest, int heightdest,UINT8 icon_index,UINT16 background_color);
int fill_rectangle(void *destbuf, int x_dest, int y_dest,int widthdest, int heightdest,int widthrec, int heightrec,UINT16 color);
void set_date_time(UINT8 index,UINT8 value);
UINT8 get_date_time(UINT8 index);
void ap_setting_date_time_string_process(UINT8 dt_tag);
void set_date_time_string_process(void*str, UINT8 index,UINT8 type);
int draw_rectangle(void *destbuf, int x_dest, int y_dest,int widthdest, int heightdest,int widthrec, int heightrec,UINT16 color,UINT16 background_color);
int draw_rect(void *destbuf, gp_size_t *size,gp_rect_t *prect, UINT8 lineWidth, UINT16 color,UINT16 background_color);
void Dsp_Open(void);
void Dsp_close(void);
SINT32 Gdjpegdecode(void *filepath, gp_bitmap_t *bitmap, int owidth, int oheight,gp_size_t* rect, int thumbnail);
int ap_ppu_text_to_vyuy(void);
int ap_ppu_text_to_rgb(void);
#endif
