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
#include <mqueue.h>
#include <linux/rtc.h>
#include <linux/input.h>
#include <time.h>
#include "public_pipe.h"
#include "mach/gp_display.h"
#include <config/sysconfig.h>
#define CRC32_POLY     0xEDB88320

#define STATUS_FAIL -1
#define STATUS_OK 0

#define NORMAL_LED		 1 	
#define IR_LED			 0
#define POWER_OFF        	2
#define MUTE_ON       		3

#define GP_WDT_MAGIC		'W'
#define WDIOC_CTRL		_IO(GP_WDT_MAGIC,0x01)		/*!< @brief watchdog enable/disable control */	
#define WDIOC_KEEPALIVE		_IO(GP_WDT_MAGIC,0x02)		/*!< @brief watchdog feed */	
#define WDIOC_FORCERESET	_IO(GP_WDT_MAGIC,0x03)		/*!< @brief watchdog force reset system */	
#define WDIOC_SETTIMEROUT	_IOW(GP_WDT_MAGIC,0x04,unsigned int)	/*!< @brief watchdog set timerout */
#define WDIOC_GETTIMEROUT	_IOR(GP_WDT_MAGIC,0x05,unsigned int)	/*!< @brief watchdog get timerout */


#define DISP_LCD_X_OFFSET	0
#define DISP_LCD_Y_OFFSET	0

#define DISP_TV_X_OFFSET	160
#define DISP_TV_Y_OFFSET	93

#define DISP_HDMI_X_OFFSET		320
#define DISP_HDMI_Y_OFFSET		60	

#define DISP_HDMI_X_OFFSET1		0
#define DISP_HDMI_Y_OFFSET1		0	

typedef struct {
 UINT8	LDW_EN;
 UINT8 	LDW_Sensitivity;
 UINT8	LDW_Area_Choice;
 UINT8 	LDW_Speed;
 UINT8	LDW_SFCW;
 UINT8 	LDW_StopAndGo;
 
} LDW_ITEMS;
typedef struct {
//0
	UINT8	language;
	UINT8	play_mode;
	UINT8	video_resolution;
	UINT8	loop_recording;
	UINT8	wdr;
	UINT8	exposure;
	UINT8	motiondetect;
	UINT8	record_audio;
//8
	UINT8	video_date_stamp;
	UINT8	gsensor;
	UINT8	capture_mode;
	UINT8	capture_resolution;
	UINT8	sequence;
	UINT8	quality;
	UINT8	sharpness;
	UINT8	white_balance;
//16
	UINT8	color;
	UINT8	iso;
	UINT8	anti_shaking;
	UINT8	capture_date_stamp;
	UINT8	slide_show;
	UINT8	beep_sound;
	UINT8	tv_mode;
	UINT8	frequency;
//24
	UINT8	factory_date[6];
	UINT8	screen_saver;
	UINT8	ir_led;
//32   
	UINT8 	date_format;
	UINT8	gsensor1;
	UINT8	LDW;
	UINT8	LDW_Sensitivity;
	UINT8	LDW_Area_Choice;
	UINT8	LDW_Car_type;
	UINT8	LDW_speed;
	UINT8   LDW_sfcw;
	UINT8   LDW_stopandgo;
	UINT8  	ifdirty;
//42
	UINT8	version[32];
//74    
	UINT8 TimeLapse;//0:OFF  1:100ms  2:200ms  3:500ms
	UINT8 LDW_sound;
} USER_ITEMS;

typedef struct {
    USER_ITEMS item ;
	UINT8  DUMMY_BYTE[256-sizeof(USER_ITEMS)-4];
	UINT8 crc[4];
} SYSTEM_USER_OPTION, *pSYSTEM_USER_OPTION;

typedef struct dv_set_s{
	UINT8 dv_UI_flag;
	UINT8 menu_select_flag;
	SINT8 menu_select_count;															
	SINT8 item_select_count;
	SINT8 item_select_count1;
	UINT8 sd_check;
	UINT8 battery_state;
	UINT8 dv_ctrl;
	UINT8 dc_ctrl;
	UINT8 usb_detect;
	UINT8 ui_show;
	UINT8 zoom_num;
/*volatile*/ UINT8 zoom_flag;
	UINT8 lock_flag;
	UINT8 power_off;
	UINT8 Power_off_num;
	UINT8 backlight_flag;
	UINT8 sdc_full;
	UINT8 display_mode;
	UINT16 display_x;
	UINT16 display_y;
	UINT8 ldws_on;
	UINT8 default_setting_flag;
	UINT8 change_disp;
	UINT8 tv_check;
	UINT8 hdmi_check;
	UINT8 pipe_lock;
	UINT8 date_type_flag;
	UINT8 motiondetect_start;
	UINT8 motiondetect_end;
	UINT8 no_power_flag;
	SINT32 usb_fd;
	UINT8 no_charge_flag;
	UINT8 upgrade_flag;
	UINT8 sdc_error;
	UINT8 parking_mode_flag;
	UINT8 LDW_INT_flag;
	UINT8 LDW_speed_flag;
	UINT8 draw_start;
	UINT8 LDW_v_offset_flag;
	UINT8 LDW_FCW_flag;
	UINT8 LDW_StopAndGo_flag;
    UINT8 extended_app;//by wuxuefu add
	
}dv_set_t;
typedef enum 
{   
    EXTAPP_NULL=0,
    EXTAPP_CLOUD_DOG,
    EXTAPP_GPS,
    EXTAPP_TOUCH,    
}EXTENDED_APP_ENUM;//by wuxuefu add

enum media_e {
	media_SD,
	media_NAND,
	media_total,
};

struct media_info_s {
	enum media_e media;
	char media_path[20];
	char media_device[20];
};

typedef struct media_on_pc_s {
	struct media_info_s list[8];
	int NAND_Support;
	int total_num;
}media_on_pc_t;

extern void test_config_set(void);
extern UINT8 DV_config_set(UINT8 index,UINT8 data);
extern UINT8 DV_config_get(SINT8 index);
extern UINT8 DC_config_set(UINT8 index,UINT8 data);
extern UINT8 DC_config_get(SINT8 index);
extern UINT8 setting_config_set(UINT8 index,UINT8 data);
extern UINT8 setting_config_get(SINT8 index);
extern UINT8 Playback_config_set(UINT8 index,UINT8 data);
extern UINT8 Playback_config_get(SINT8 index);
extern SINT32 ap_state_config_load(void);
extern void ap_state_config_default_set(void);
extern void ap_state_config_initial(SINT32 status);
extern void ap_state_config_store(void);
extern UINT8 Get_play_mode(void);
extern SINT8 SDC_Format(void);
extern UINT8 RTC_get(void);
extern void RTC_set(void);
extern UINT32 SDC_FreeSize(void);
extern SINT32 CheckMemoryPath(void);
extern void board_config_set_init(UINT8 type);
extern void statusScanThread( void);
void PowerOff_entry(void);
extern void usb_plugin();
extern void unmount_dev(void);
extern UINT8 LCD_Backlight_Set(UINT8 value);
extern UINT8 Power_Off_Set(void);
extern UINT8 LED_Set(UINT8 type,UINT8 value); //type 0:IR 1:LED 2:power off
