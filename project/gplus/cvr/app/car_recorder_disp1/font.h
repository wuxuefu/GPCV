#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mach/typedef.h"
#include "disp.h"
#include <sys/stat.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#define INT32U UINT32
#define INT16U UINT16
#define INT8U UINT8
#define CHAR SINT8

#define INT32S SINT32
#define INT16S SINT16
#define INT8S SINT8

#define gp_malloc malloc
#define gp_free free
#define gp_memcpy memcpy
#define DBG_PRINT printf
#define STATUS_FAIL -1
#define STATUS_OK 0

#define YUYV_DRAW		0x0
#define YUV420_DRAW		0x1
#define RGB565_DRAW		0x2
#define COLOR256_DRAW		0x3
#define COLOR16_DRAW		0x4
//#define FONT_PATH "/media/sdcardc/resource/font/"
#define FONT_PATH "/system/resource/font/"

#define _DPF_SUPPORT_ENGLISH    1
#define _DPF_SUPPORT_SCHINESE   1
#define _DPF_SUPPORT_TCHINESE   1
#define _DPF_SUPPORT_FRENCH     0
#define _DPF_SUPPORT_ITALIAN    0
#define _DPF_SUPPORT_SPANISH    0
#define _DPF_SUPPORT_PORTUGUESE 0
#define _DPF_SUPPORT_GERMAN     0
#define _DPF_SUPPORT_RUSSIAN    0
#define _DPF_SUPPORT_SPANISH    0
#define _DPF_SUPPORT_TURKISH    0
#define _DPF_SUPPORT_WWEDISH    0
typedef enum {
	#if _DPF_SUPPORT_ENGLISH == 1
	LCD_EN = 0,  //English
	#endif
	#if _DPF_SUPPORT_TCHINESE == 1
	LCD_TCH,	//Traditional Chinese
 	#endif
	#if _DPF_SUPPORT_SCHINESE == 1
	LCD_SCH,    //Simple Chinese
	#endif
 	#if _DPF_SUPPORT_FRENCH == 1
    LCD_FR,     //French
 	#endif
	#if _DPF_SUPPORT_ITALIAN == 1
    LCD_IT, 	//Italian
 	#endif
	#if _DPF_SUPPORT_PORTUGUESE == 1
    LCD_ES,     // Spanish
 	#endif
 	#if _DPF_SUPPORT_GERMAN == 1
	LCD_PT,     // Portuguese
 	#endif
	#if _DPF_SUPPORT_RUSSIAN == 1
	LCD_DE,     // German
 	#endif
	#if _DPF_SUPPORT_SPANISH == 1
	LCD_RU,     // Russian
 	#endif
	#if _DPF_SUPPORT_TURKISH == 1
	LCD_TR,     // Turkish
 	#endif
	#if _DPF_SUPPORT_WWEDISH == 1
	LCD_SV,     //  Swedish
 	#endif
 	LCD_MAX
}_DPF_LCD;

typedef struct{
    INT16U font_color;
    INT16U font_type;
    INT16S pos_x;
    INT16S pos_y;
    INT16U buff_w;
    INT16U buff_h;
    INT16U language;
    INT16U str_idx;
}STRING_INFO;

typedef struct {
	INT16U	string_width;
	INT16U	string_height;
} t_STRING_TABLE_STRUCT;

typedef struct{
    INT16U font_color;
    INT16U font_type;
    INT16S pos_x;
    INT16S pos_y;
    INT16U buff_w;
    INT16U buff_h;
    char *str_ptr;
}STRING_ASCII_INFO;


typedef struct {
	CHAR  id[4];
	INT32U  version;
	INT32U  reserve0;
	INT16U	irkey_num;
	INT16U	rbkey_state_num;
	INT16U	rbkey_num;
	INT16U	osd_menu_num;
	INT16U	osd_menu_item_num;
	INT16U	main_menu_num;
	INT16U	main_menu_item_num;
	INT16U	palette_item;
	INT16U	image_item;
	INT16U	audio_item;
	INT16U	audio_open_item;
	INT16U	language_num;
	INT16U	string_item;
	INT8U	str_ch_width[20];	//1 byte or 2 byte
	INT16U	font_item;
	INT16U	auto_demo_num;
	INT16U	others_bin_num;
	INT32U	offset_startup_ctrl;
	INT32U	offset_ir_key;
	INT32U	offset_rubber_key;
	INT32U	offset_osd_menu;
	INT32U	offset_main_menu;
	INT32U	offset_popmessage;
	INT32U	offset_audio;
	INT32U	offset_palette;
	INT32U	offset_image;
	INT32U	offset_video;
	INT32U	offset_string[20];
	INT32U	offset_font[20];
	INT32U	offset_sys_image_idx;
	INT32U	offset_factor_default_option;
	INT32U	offset_auto_demo;
	INT32U	offset_others_bin;
	INT32U	offset_multi_language_str;
	INT32U	offset_multi_language_font;
	INT32U  reserve1[8];
} t_GP_RESOURCE_HEADER;

typedef struct {
	INT16U	length;
	INT32U	raw_data_offset;
} t_STRING_STRUCT;

typedef struct {
	INT32U	length;
	INT32U	compress_length;	// font compress 1021 neal
	INT32U	raw_data_offset;
} t_FONT_STRUCT;

typedef struct {
	INT8U	font_width;
	INT8U	font_height;
	INT8U	bytes_per_line;
	INT32U	font_content;
} t_FONT_TABLE_STRUCT;

#define EVENT_KEY_BASE    		0x00001000
typedef enum {
	// Hot keys
    EVENT_KEY_POWER=EVENT_KEY_BASE,
    EVENT_KEY_MENU        ,
    EVENT_KEY_SLIDE_SHOW  ,
    EVENT_KEY_ICON_SHOW   ,
    EVENT_KEY_PHOTO_VIEW,
    EVENT_KEY_CALENDAR    ,
    EVENT_KEY_SETUP       ,
    EVENT_KEY_MUSIC       ,
    EVENT_KEY_PETS        ,
    EVENT_KEY_GAMES       ,
    EVENT_KEY_STORAGE	  ,
	// Direction keys
    EVENT_KEY_OK,
	EVENT_KEY_IR_OK		,
    EVENT_KEY_BACK      , 
    EVENT_KEY_LEFT      , 
    EVENT_KEY_IR_LEFT	,
    EVENT_KEY_RIGHT     , 
	EVENT_KEY_IR_RIGHT	,
    EVENT_KEY_UP        , 
    EVENT_KEY_DOWN      , 
    EVENT_KEY_PAGEUP    , 
    EVENT_KEY_PAGEDOWN  , 
    EVENT_KEY_ZOOM_IN   , 
    EVENT_KEY_ZOOM_OUT  , 
    EVENT_KEY_ZOOM_CYCLE, 
    EVENT_KEY_ROTATE	,
    EVENT_KEY_MODE		,
    EVENT_KEY_INTERVAL	,
    // Media keys
    EVENT_KEY_MEDIA_SOUND_ON_OFF,
    EVENT_KEY_MEDIA_VOL_UP   ,
    EVENT_KEY_MEDIA_VOL_DOWN ,
    EVENT_KEY_MEDIA_STOP     ,
    EVENT_KEY_MEDIA_PLAY     	,
    EVENT_KEY_MEDIA_PAUSE    ,
    EVENT_KEY_MEDIA_PLAY_PAUSE	,
    EVENT_KEY_MEDIA_PREV     ,
    EVENT_KEY_MEDIA_NEXT     ,
    EVENT_KEY_MEDIA_FF       ,
    EVENT_KEY_MEDIA_BF       ,
    // Auto demo
    EVENT_KEY_AUTO_DEMO,
    // AP should not response to this event
    EVENT_KEY_NO_ACTION,
    // PHOTO Frame display mode
	EVENT_PORTRAIT_DISPLAY,
    EVENT_LANDSCAPE_DISPLAY,
    // Pop-up style OSD menu
    EVENT_KEY_OSD_MENU,
    // End of key define
    EVENT_KEY_MAX
} EVENT_MEDIA_ENUM;
extern INT32S ap_state_resource_init(void);
extern void ap_state_resource_exit(void);
extern INT32S ap_state_resource_string_resolution_get(STRING_INFO *str_info, t_STRING_TABLE_STRUCT *str_res);
extern INT32S ap_state_resource_string_draw(INT16U *frame_buff, STRING_INFO *str_info);
extern INT32S ap_state_resource_string_ascii_draw(INT16U *frame_buff, STRING_ASCII_INFO *str_ascii_info);
extern INT16U ap_state_resource_language_num_get(void);
extern INT16S ap_state_resource_time_stamp_position_x_get(void);
extern INT16S ap_state_resource_time_stamp_position_y_get(void);
INT32S ap_state_resource_string_ascii_resolution_get(void *str_name, t_STRING_TABLE_STRUCT *str_res);