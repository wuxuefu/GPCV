#include "ap_state_config.h"
#include <mach/gp_board.h>
#include "cvr_pipe.h"
#include <sys/vfs.h>
#include <dirent.h>
#include <mach/gp_usb.h>
#include "mach/gp_spi.h"
#include <dirent.h>
#include "version.h"

#define AUTO_UPDATE 1 //1: auto update spi flash, 0: update spi flash with power key

#define SPI_FLASH_ADDR_END 4*1024*1024
UINT32 CRC32_tbl[256];
SYSTEM_USER_OPTION Global_User_Optins;
UINT8 Memory_path[64];
media_on_pc_t g_device_list;
UINT8 HDMIxNum=1;
UINT16 HDMI_X_offset=0;
UINT16	 HDMI_Y_offset=0;
UINT8 LDW_Change = 0;
extern pthread_mutex_t key_mutex;
extern struct timeval starttime;
extern UINT32 free_size_flag;
extern dv_set_t dv_set;
extern HANDLE hDisp;
extern mqd_t menumodeMsgQ;
extern UINT8 foreground_draw_flag;
extern UINT8 cur_draw_flag;
extern UINT8 Playmode_flag;
UINT32 USB_time_num = 0;
extern UINT32 time_num;
UINT8 old_battery=0;
extern void USB_entry_init(void);
extern void Power_off(void);
extern HANDLE hDisp1;
/*========================================================
DV menu config set/get function
==========================================================*/
static UINT8 ap_state_config_video_resolution_get(void)
{
	return Global_User_Optins.item.video_resolution;
}
static void ap_state_config_video_resolution_set(UINT8 resolution)
{
	if (resolution != ap_state_config_video_resolution_get()) {
		CVR_Set_PIPE(CMD_SET_DV_RESOLUTION,resolution);
		Global_User_Optins.item.video_resolution = resolution;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_video_looprecording_get(void)
{
	return Global_User_Optins.item.loop_recording;
}
static void ap_state_config_video_looprecording_set(UINT8 type)
{
	if (type != ap_state_config_video_looprecording_get()) {
		CVR_Set_PIPE(CMD_SET_LOOPRECORDING,type);
		Global_User_Optins.item.loop_recording = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_exposure_get(void)
{
	return Global_User_Optins.item.exposure;
}

static void ap_state_config_exposure_set(UINT8 exposure)
{
	if (exposure != ap_state_config_exposure_get()) {
		CVR_Set_PIPE(CMD_SET_EXPOSURE,exposure);
		Global_User_Optins.item.exposure = exposure;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_WDR_get(void)
{
	return Global_User_Optins.item.wdr;
}

static void ap_state_config_WDR_set(UINT8 type)
{
	if (type != ap_state_config_WDR_get()) {
		CVR_Set_PIPE(CMD_SET_WDR,type);
		Global_User_Optins.item.wdr = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_motion_detect_get(void)
{
	return Global_User_Optins.item.motiondetect;
}

static void ap_state_config_motion_detect_set(UINT8 type)
{
	if (type != ap_state_config_motion_detect_get()) {
		if(dv_set.sd_check == 1)
		{
			CVR_Set_PIPE(CMD_SET_MD,type);
		}
		Global_User_Optins.item.motiondetect = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_audio_on_get(void)
{
	return Global_User_Optins.item.record_audio;
}

static void ap_state_config_audio_on_set(UINT8 type)
{
	if (type != ap_state_config_audio_on_get()) {
		CVR_Set_PIPE(CMD_SET_RECORD_AUDIO,type);
		Global_User_Optins.item.record_audio = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_datestamp_on_get(void)
{
	return Global_User_Optins.item.video_date_stamp;
}

static void ap_state_config_datestamp_on_set(UINT8 type)
{
	if (type != ap_state_config_datestamp_on_get()) {
		CVR_Set_PIPE(CMD_SET_DV_DATE_STAMP,type);
		Global_User_Optins.item.video_date_stamp = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_gsensor_get(void)
{
	return Global_User_Optins.item.gsensor;
}

static void ap_state_config_gsensor_set(UINT8 type)
{
	if (type != ap_state_config_gsensor_get()) {
		if(type !=0)
		Gsensor_write(type);
		Global_User_Optins.item.gsensor = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

UINT8 ap_state_config_TimeLapse_get(void)
{
	return Global_User_Optins.item.TimeLapse;
}

void ap_state_config_TimeLapse_set(UINT8 type)
{
	if (type != ap_state_config_TimeLapse_get()) {
		
		CVR_Set_PIPE(CMD_SET_TIME_LAPSE,type);
		Global_User_Optins.item.TimeLapse= type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_LDW_get(void)
{
	return Global_User_Optins.item.LDW;
}

static void ap_state_config_LDW_set(UINT8 type)
{
	if (type != ap_state_config_LDW_get()) {
		Global_User_Optins.item.LDW = type;
		LDW_Change = 1;
		Global_User_Optins.item.ifdirty = 1;
	}
}
UINT8 ap_state_config_LDW_Sensitivity_get(void)
{
	return Global_User_Optins.item.LDW_Sensitivity;
}

void ap_state_config_LDW_Sensitivity_set(UINT8 type)
{
	if (type != ap_state_config_LDW_Sensitivity_get()) {
		Global_User_Optins.item.LDW_Sensitivity = type;
		LDW_Change = 1;
		Global_User_Optins.item.ifdirty = 1;
	}
}
UINT8 ap_state_config_LDW_Area_Choice_get(void)
{
	return Global_User_Optins.item.LDW_Area_Choice;
}

void ap_state_config_LDW_Area_Choice_set(UINT8 type)
{
	if (type != ap_state_config_LDW_Area_Choice_get()) {
		Global_User_Optins.item.LDW_Area_Choice = type;
		LDW_Change = 1;
		Global_User_Optins.item.ifdirty = 1;
	}
}

UINT8 ap_state_config_LDW_SPEED_get(void)
{
	return Global_User_Optins.item.LDW_speed;
}

void ap_state_config_LDW_SPEED_set(UINT8 type)
{
	if (type != ap_state_config_LDW_SPEED_get()) {
		Global_User_Optins.item.LDW_speed = type;
		LDW_Change = 1;
		Global_User_Optins.item.ifdirty = 1;
	}
}

UINT8 ap_state_config_LDW_Car_type_get(void)
{
	return Global_User_Optins.item.LDW_Car_type;
}

void ap_state_config_LDW_Car_type_set(UINT8 type)
{
	if (type != ap_state_config_LDW_Car_type_get()) {
		Global_User_Optins.item.LDW_Car_type = type;
		LDW_Change = 1;
		Global_User_Optins.item.ifdirty = 1;
	}
}

UINT8 ap_state_config_LDW_Sound_get(void)
{
	return Global_User_Optins.item.LDW_sound;
}

void ap_state_config_LDW_Sound_set(UINT8 type)
{
	if (type != ap_state_config_LDW_Sound_get()) {
		Global_User_Optins.item.LDW_sound = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_gsensor1_get(void)
{
	return Global_User_Optins.item.gsensor1;
}

static void ap_state_config_gsensor1_set(UINT8 type)
{
	if (type != ap_state_config_gsensor1_get()) {
		Global_User_Optins.item.gsensor1 = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

UINT8 DV_config_get(SINT8 index)
{
	UINT8 ret;
	if(index>DV_TIMELAPSE)
		index = DV_RESOLUTION;
	if(index<0)
		index = DV_TIMELAPSE;
	
	switch(index)
	{	
		case DV_RESOLUTION:
			ret =  ap_state_config_video_resolution_get();	
		break;	
		case DV_LOOPRECORDING:
			ret =  ap_state_config_video_looprecording_get();	
		break;
		case DV_WDR:
			ret =  ap_state_config_WDR_get();
		break;
		case DV_EXPOSURE:
			ret =  ap_state_config_exposure_get();
		break;
		case DV_MOTIONDETECTION:
			ret = ap_state_config_motion_detect_get();
		break;
		case DV_AUDIO:
			ret = ap_state_config_audio_on_get();
		break;
		case DV_DATESTAMP:
			ret = ap_state_config_datestamp_on_get();
		break;
		case DV_GSENSOR:
			ret = ap_state_config_gsensor_get();
		break;
		case DV_LDW:
			ret = ap_state_config_LDW_get();
		break;		
		case DV_TIMELAPSE:
			ret = ap_state_config_TimeLapse_get();
		break;	

		default:
		break;
	}
	return ret;
}

UINT8 DV_config_set(UINT8 index,UINT8 data)
{
	switch(index)
	{	
		case DV_RESOLUTION:
			ap_state_config_video_resolution_set(data);	
		break;	
		case DV_LOOPRECORDING:
			ap_state_config_video_looprecording_set(data);	
		break;
		case DV_WDR:
			ap_state_config_WDR_set(data);
		break;
		case DV_EXPOSURE:
			ap_state_config_exposure_set(data);
		break;
		case DV_MOTIONDETECTION:
			ap_state_config_motion_detect_set(data);
		break;
		case DV_AUDIO:
			ap_state_config_audio_on_set(data);
		break;
		case DV_DATESTAMP:
			ap_state_config_datestamp_on_set(data);
		break;
		case DV_GSENSOR:
			ap_state_config_gsensor_set(data);
		break;
		case DV_LDW:
			ap_state_config_LDW_set(data);
		break;		
		case DV_TIMELAPSE:
			ap_state_config_TimeLapse_set(data);
		break;
		default:
		break;
	}
	ap_state_config_store();
}

/*==========================================================
setting config set and get function
===========================================================*/
static UINT8 ap_state_config_date_format_display_get(void)
{
	return Global_User_Optins.item.date_format;
}

static void ap_state_config_date_format_display_set(UINT8 type)
{
	if (type != ap_state_config_date_format_display_get()) {
		if(Playmode_flag <= CMD_STILL_MODE)
		{
			CVR_Set_PIPE(CMD_SET_DATE_TYPE,type);
		}
		else
		{
			dv_set.date_type_flag = 1;
		}
		Global_User_Optins.item.date_format = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_beep_sound_get(void)
{
	return Global_User_Optins.item.beep_sound;
}

static void ap_state_config_beep_sound_set(UINT8 type)
{
	if (type != ap_state_config_beep_sound_get()) {
		Global_User_Optins.item.beep_sound = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_language_get(void)
{
	return Global_User_Optins.item.language;
}
static void ap_state_config_language_set(UINT8 language)
{
	if (language != ap_state_config_language_get()) {
		Global_User_Optins.item.language = language;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_TV_mode_get(void)
{
	return Global_User_Optins.item.tv_mode;
}

static void ap_state_config_TV_mode_set(UINT8 type)
{
	if (type != ap_state_config_TV_mode_get()) {
		CVR_Set_PIPE(CMD_SET_TVMODE,type);
		Global_User_Optins.item.tv_mode = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_frequency_get(void)
{
	return Global_User_Optins.item.frequency;
}

static void ap_state_config_frequency_set(UINT8 type)
{
	if (type != ap_state_config_frequency_get()) {
		CVR_Set_PIPE(CMD_SET_FREQUENCY,type);
		Global_User_Optins.item.frequency = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_screen_saver_get(void)
{
	return Global_User_Optins.item.screen_saver;
}

static void ap_state_config_screen_saver_set(UINT8 type)
{
	if (type != ap_state_config_screen_saver_get()) {
		Global_User_Optins.item.screen_saver = type;
		Global_User_Optins.item.ifdirty = 1;
		if(type == 1)
		{
			gettimeofday(&starttime,NULL);
		}
	}
}
static UINT8 ap_state_config_IR_LED_get(void)
{
	return Global_User_Optins.item.ir_led;
}

static void ap_state_config_IR_LED_set(UINT8 type)
{
	if (type != ap_state_config_IR_LED_get()) {
		LED_Set(IR_LED,type);
		Global_User_Optins.item.ir_led = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

UINT8 setting_config_get(SINT8 index)
{
	UINT8 ret;
	switch(index)
	{	
		case SET_GSENSOR1:
			ret = ap_state_config_gsensor1_get();	
		break;
		case SET_DATA_FORMAT:
			ret =  ap_state_config_date_format_display_get();	
		break;	
		case SET_SOUND:
			ret =  ap_state_config_beep_sound_get();	
		break;
		case SET_LANGUAGE:
			ret =  ap_state_config_language_get();
		break;
		#ifdef SYSCONFIG_DISP0_TVOUT
		case SET_TV_MODE:
			ret =  ap_state_config_TV_mode_get();
		break;
		#endif
		case SET_FREQUENCY:
			ret = ap_state_config_frequency_get();
		break;
		case SET_SCREEN_SAVER:
			ret = ap_state_config_screen_saver_get();
		break;
		case SET_IR_LED:
			ret = ap_state_config_IR_LED_get();
		break;

		default:
		break;
	}
	return ret;
}

UINT8 setting_config_set(UINT8 index,UINT8 data)
{
	switch(index)
	{	
		case SET_GSENSOR1:
			ap_state_config_gsensor1_set(data);	
		break;
		case SET_DATA_FORMAT:
			ap_state_config_date_format_display_set(data);	
		break;	
		case SET_SOUND:
			ap_state_config_beep_sound_set(data);	
		break;
		case SET_LANGUAGE:
			ap_state_config_language_set(data);
		break;
		#ifdef SYSCONFIG_DISP0_TVOUT
		case SET_TV_MODE:
			ap_state_config_TV_mode_set(data);
		break;
		#endif
		case SET_FREQUENCY:
			ap_state_config_frequency_set(data);
		break;
		case SET_SCREEN_SAVER:
			ap_state_config_screen_saver_set(data);
		break;
		case SET_IR_LED:
			ap_state_config_IR_LED_set(data);
		break;

		default:
		break;
	}	
	ap_state_config_store();
}
/*========================================================
DC menu config set/get function
==========================================================*/
static UINT8 ap_state_config_capture_mode_get(void)
{
	return Global_User_Optins.item.capture_mode;
}
static  void ap_state_config_capture_mode_set(UINT8 type)
{
	if (type != ap_state_config_capture_mode_get()) {
		Global_User_Optins.item.capture_mode = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_capture_resolution_get(void)
{
	return Global_User_Optins.item.capture_resolution;
}
static void ap_state_config_capture_resolution_set(UINT8 type)
{
	if (type != ap_state_config_capture_resolution_get()) {
		CVR_Set_PIPE(CMD_SET_DC_RESOLUTION,type);
		free_size_flag = 1;
		Global_User_Optins.item.capture_resolution = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_sequence_get(void)
{
	return Global_User_Optins.item.sequence;
}
static void ap_state_config_sequence_set(UINT8 type)
{
	if (type != ap_state_config_sequence_get()) {
		CVR_Set_PIPE(CMD_SET_SEQUENCE,type);
		Global_User_Optins.item.sequence = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_quality_get(void)
{
	return Global_User_Optins.item.quality;
}

static void ap_state_config_quality_set(UINT8 type)
{
	if (type != ap_state_config_quality_get()) {
		CVR_Set_PIPE(CMD_SET_QUALITY,type);
		free_size_flag = 1;
		Global_User_Optins.item.quality = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_sharpness_get(void)
{
	return Global_User_Optins.item.sharpness;
}

static void ap_state_config_sharpness_set(UINT8 type)
{
	if (type != ap_state_config_sharpness_get()) {
		CVR_Set_PIPE(CMD_SET_SHARPNESS,type);
		Global_User_Optins.item.sharpness = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_white_balance_get(void)
{
	return Global_User_Optins.item.white_balance;
}

static void ap_state_config_white_balance_set(UINT8 type)
{
	if (type != ap_state_config_white_balance_get()) {
		CVR_Set_PIPE(CMD_SET_AWB,type);
		Global_User_Optins.item.white_balance = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}

static UINT8 ap_state_config_color_get(void)
{
	return Global_User_Optins.item.color;
}

static void ap_state_config_color_set(UINT8 type)
{
	if (type != ap_state_config_color_get()) {
		CVR_Set_PIPE(CMD_SET_COLOR,type);
		Global_User_Optins.item.color = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_ISO_get(void)
{
	return Global_User_Optins.item.iso;
}

static void ap_state_config_ISO_set(UINT8 type)
{
	if (type != ap_state_config_ISO_get()) {
		CVR_Set_PIPE(CMD_SET_ISO,type);
		Global_User_Optins.item.iso = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_anti_shaking_get(void)
{
	return Global_User_Optins.item.anti_shaking;
}

static void ap_state_config_anti_shaking_set(UINT8 type)
{
	if (type != ap_state_config_anti_shaking_get()) {
		CVR_Set_PIPE(CMD_SET_ANTI_SHAKING,type);
		Global_User_Optins.item.anti_shaking = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
static UINT8 ap_state_config_capture_date_stamp_get(void)
{
	return Global_User_Optins.item.capture_date_stamp;
}

static void ap_state_config_capture_date_stamp_set(UINT8 type)
{
	if (type != ap_state_config_capture_date_stamp_get()) {
		CVR_Set_PIPE(CMD_SET_DC_DATE_STAMP,type);
		Global_User_Optins.item.capture_date_stamp = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}


UINT8 DC_config_get(SINT8 index)
{
	UINT8 ret;
	if(index>DC_DATE_STAMP)
		index = DV_RESOLUTION;
	if(index<0)
		index = DC_DATE_STAMP;
	
	switch(index)
	{	
		case DC_CAPTURE_MODE:
			ret =  ap_state_config_capture_mode_get();	
		break;	
		case DC_RESOLUTION:
			ret =  ap_state_config_capture_resolution_get();	
		break;
		case DC_SEQUENCE:
			ret =  ap_state_config_sequence_get();
		break;
		case DC_QUALITY:
			ret =  ap_state_config_quality_get();
		break;
		case DC_SHARPNESS:
			ret = ap_state_config_sharpness_get();
		break;
		case DC_WHITE_BALANCE:
			ret = ap_state_config_white_balance_get();
		break;
		case DC_COLOR:
			ret = ap_state_config_color_get();
		break;
		case DC_ISO:
			ret = ap_state_config_ISO_get();
		break;
		case DC_EXPOSURE:
			ret = ap_state_config_exposure_get();
		break;
		case DC_ANTI_SHAKING:
			ret = ap_state_config_anti_shaking_get();
		break;
		case DC_DATE_STAMP:
			ret = ap_state_config_capture_date_stamp_get();
		break;

		
		default:
		break;
	}
	return ret;
}

UINT8 DC_config_set(UINT8 index,UINT8 data)
{
	switch(index)
	{	
		case DC_CAPTURE_MODE:
			ap_state_config_capture_mode_set(data);	
		break;	
		case DC_RESOLUTION:
			ap_state_config_capture_resolution_set(data);	
		break;
		case DC_SEQUENCE:
			ap_state_config_sequence_set(data);
		break;
		case DC_QUALITY:
			ap_state_config_quality_set(data);
		break;
		case DC_SHARPNESS:
			ap_state_config_sharpness_set(data);
		break;
		case DC_WHITE_BALANCE:
			ap_state_config_white_balance_set(data);
		break;
		case DC_COLOR:
			ap_state_config_color_set(data);
		break;
		case DC_ISO:
			ap_state_config_ISO_set(data);
		break;
		case DC_EXPOSURE:
			ap_state_config_exposure_set(data);
		break;
		case DC_ANTI_SHAKING:
			ap_state_config_anti_shaking_set(data);
		break;
		case DC_DATE_STAMP:
			ap_state_config_capture_date_stamp_set(data);
		break;

		default:
		break;
	}
	ap_state_config_store();
}
/*========================================================
PLAY back menu config set/get function
==========================================================*/
static UINT8 ap_state_config_slide_show_get(void)
{
	return Global_User_Optins.item.slide_show;
}
static  void ap_state_config_slide_show_set(UINT8 type)
{
	if (type != ap_state_config_slide_show_get()) {
		Global_User_Optins.item.slide_show = type;
		Global_User_Optins.item.ifdirty = 1;
	}
}
UINT8 Playback_config_get(SINT8 index)
{
	UINT8 ret;
	if(index>DV_WDR)
		index = DV_RESOLUTION;
	if(index<0)
		index = DV_WDR;
	
	switch(index)
	{	
		case DV_RESOLUTION:
			ret =  0;	
		break;	
		case DV_LOOPRECORDING:
			ret =  0;
		break;
		case DV_WDR:
			ret =  ap_state_config_slide_show_get();
		break;

		default:
		break;
	}
	return ret;
}

UINT8 Playback_config_set(UINT8 index,UINT8 data)
{
	switch(index)
	{	
		case DV_RESOLUTION:

		break;	
		case DV_LOOPRECORDING:
				
		break;
		case DV_WDR:
			ap_state_config_slide_show_set(data);
		break;
		default:
		break;
	}
}
extern UINT8 setup_date_time[7];
extern dv_set_t dv_set;
void test_config_set(void)
{
	ap_state_config_initial(ap_state_config_load());
	//dv_set.battery_state = 4;
	//dv_set.sd_check = 1;
	Global_User_Optins.item.color = 0;
	dv_set.dv_ctrl = 0;
	dv_set.dc_ctrl = 0;
	dv_set.ui_show = 0;
	dv_set.zoom_num = 0;
	dv_set.zoom_flag = 0;
	dv_set.lock_flag = 0;
	dv_set.power_off = 0;
	dv_set.Power_off_num = 0;
	dv_set.backlight_flag = 1;
	dv_set.ldws_on = 0;
	dv_set.default_setting_flag = 0;
	dv_set.change_disp = 0;
	dv_set.pipe_lock = 1;
	dv_set.date_type_flag = 0;
	dv_set.motiondetect_start = 0;
	dv_set.motiondetect_end = 0;
	dv_set.no_power_flag = 0;
	dv_set.usb_fd = -1;
	dv_set.no_charge_flag = 0;
	dv_set.upgrade_flag = 2;
	dv_set.sdc_error = 0;
	dv_set.parking_mode_flag = 0;
	dv_set.LDW_speed_flag = 0;
	dv_set.LDW_INT_flag = 0;
	dv_set.draw_start = 0;
}
void DC_Set_Color(void)
{
	Global_User_Optins.item.color = 0;
}
UINT8 Get_play_mode(void)
{
	return Global_User_Optins.item.play_mode;
}

void CRC32tbl_init(void)
{
    UINT32 i,j;
    UINT32 c;

    for (i=0 ; i<256 ; ++i) {
        c = i ;
        for (j=0 ; j<8 ; j++) { 
            c = (c & 1) ? (c >> 1) ^ CRC32_POLY : (c >> 1);
        }
        CRC32_tbl[i] = c;
    }
}

void CRC_cal(UINT8 *pucBuf, UINT32 len, UINT8 *CRC)
{
    UINT32 i_CRC, value;
    UINT32 i;

    if (!CRC32_tbl[1]) {
        CRC32tbl_init();
    }    
    i_CRC = 0xFFFFFFFF;
    for (i=0 ; i<len ; i++) {
        i_CRC = (i_CRC >> 8) ^ CRC32_tbl[(i_CRC ^ pucBuf[i]) & 0xFF];
    }
    value = ~i_CRC;
    CRC[0] = (value & 0x000000FF) >> 0;
    CRC[1] = (value & 0x0000FF00) >> 8;
    CRC[2] = (value & 0x00FF0000) >> 16;
    CRC[3] = (value & 0xFF000000) >> 24;
}

SINT32 ap_state_config_load(void)
{
	UINT8 expect_crc[4];
	int fd;
	UINT8 file_path[128];
	int ret;
	char spi_addr[4];
	int freq = 12000000;	
	UINT32 addr;
	int i;
	UINT8 *P;
	P = (UINT8 *)&Global_User_Optins;	
	fd = open("/dev/spi",O_RDWR);
	if(fd<0){		
		printf("Can't open [/dev/spi0]\n");		
		return STATUS_FAIL;	
	}
	ioctl(fd, SPI_IOCTL_SET_CHANNEL, 0);	//Set SPI CS0		
	ioctl(fd, SPI_IOCTL_SET_FREQ, freq);		
	ioctl(fd, SPI_IOCTL_SET_DMA, 0);	

	addr = SPI_FLASH_ADDR_END - 4*1024;	
	spi_addr[0] = 0x03;		
	spi_addr[1] = ((addr>>16) & 0xFF);		
	spi_addr[2] = ((addr>>8) & 0xFF);		
	spi_addr[3] = (addr & 0xFF);				
	ioctl(fd, SPI_IOCTL_CS_ENABLE, 0);		
	write(fd, spi_addr, 4);
	read(fd, (void*)&Global_User_Optins, 256);	
	ioctl(fd, SPI_IOCTL_CS_DISABLE, 0);	


	//for(i=0;i<256;i++)
	//	printf("read buffer[%d]=%d\n",i,*(P+i));		
	printf("load CRC = %02x %02x %02x %02x\n",Global_User_Optins.crc[0],Global_User_Optins.crc[1],Global_User_Optins.crc[2],Global_User_Optins.crc[3]);
	/* calculate expect CRC */
	CRC_cal((UINT8*)&Global_User_Optins.item, sizeof(USER_ITEMS),expect_crc);
	printf("expect CRC = %02x %02x %02x %02x\r\n",expect_crc[0],expect_crc[1],expect_crc[2],expect_crc[3]);
	/* compare CRC and expect CRC */
	if (memcmp((SINT8*)expect_crc, (SINT8*)Global_User_Optins.crc, 4) != 0) {
		printf("crc error\r\n");
		close(fd);
		return STATUS_FAIL; /* CRC error */
	}
	close(fd);
	return STATUS_OK;
}
void ap_state_config_default_set(void)
{
	SINT16 handle;
	UINT8 buffer[32],i;
	for(i=0;i<32;i++)
	{
		buffer[i] = Global_User_Optins.item.version[i];
	}
	memset((SINT8 *)&Global_User_Optins, 0x00, sizeof(SYSTEM_USER_OPTION));
	handle = open((UINT8 *) "/system/resource/Default_Setting.bin",O_RDONLY);	
	read(handle, (void *)&Global_User_Optins, 256);
	close(handle);
	for(i=0;i<32;i++)
	{
		Global_User_Optins.item.version[i] = buffer[i];
	}	
	/*setup_date_time[0] = Global_User_Optins.item.factory_date[0];
	setup_date_time[1] = Global_User_Optins.item.factory_date[1];
	setup_date_time[2] = Global_User_Optins.item.factory_date[2];
	setup_date_time[3] = Global_User_Optins.item.factory_date[3];
	setup_date_time[4] = Global_User_Optins.item.factory_date[4];
	setup_date_time[5] = Global_User_Optins.item.factory_date[5];	
	RTC_set();*/	
	Global_User_Optins.item.ifdirty = 1;
	printf("load default set!\n");
}
void ap_state_config_initial(SINT32 status)
{
	//default setting
	Global_User_Optins.item.ifdirty = 0;
	if (status == STATUS_FAIL) { 
		ap_state_config_default_set();
	}
}
#define SPI_FLASH_SR_WIP	0x01
void ap_state_config_store(void)
{
	int fd;
	UINT8 file_path[128];
	int block;
	char spi_addr[4];
	int freq = 12000000;	
	UINT32 addr;
	int i;
	UINT8 *P;
	char buffer[4];
	P = (UINT8 *)&Global_User_Optins;
	if (Global_User_Optins.item.ifdirty == 1) {
		
		CRC_cal((UINT8*)&Global_User_Optins.item, sizeof(USER_ITEMS),Global_User_Optins.crc);
		printf("store CRC = %02x %02x %02x %02x\r\n",Global_User_Optins.crc[0],Global_User_Optins.crc[1],Global_User_Optins.crc[2],Global_User_Optins.crc[3]);
		fd = open("/dev/spi",O_RDWR);
		if(fd<0){		
			printf("Can't open [/dev/spi0]\n");		
			return;	
		}
		addr = SPI_FLASH_ADDR_END - 4*1024;
		block = addr/512;
		ioctl(fd, SPI_IOCTL_SET_CHANNEL, 0);	//Set SPI CS0		
		ioctl(fd, SPI_IOCTL_SET_FREQ, freq);		
		ioctl(fd, SPI_IOCTL_SET_DMA, 0);				
		spi_addr[0] = 0x06;		
		ioctl(fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(fd, spi_addr, 1);		
		ioctl(fd, SPI_IOCTL_CS_DISABLE, 0);			
		spi_addr[0] = 0x20;		
		spi_addr[1] = (((block<<9)>>16) & 0xFF);		
		spi_addr[2] = (((block<<9)>>8) & 0xFF);		
		spi_addr[3] = ((block<<9) & 0xFF);  			
		ioctl(fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(fd, spi_addr, 4);		
		ioctl(fd, SPI_IOCTL_CS_DISABLE, 0);
    		i=0;
		do{
			spi_addr[0] = 0x05;		
			ioctl(fd, SPI_IOCTL_CS_ENABLE, 0);		
			write(fd, spi_addr, 1);
			read(fd, buffer, 1);	
			ioctl(fd, SPI_IOCTL_CS_DISABLE, 0);	
			usleep(100000); //100ms
			i++;
			if(i >= 30)
			{
				printf("SPI ERASE TIMEOUT!\n");
				break;
			}
		}while(buffer[0]& SPI_FLASH_SR_WIP != 0);		
		
		//write
		//ioctl(fd, SPI_IOCTL_SET_CHANNEL, 0);	//Set SPI CS0		
		//ioctl(fd, SPI_IOCTL_SET_FREQ, freq);		
		//ioctl(fd, SPI_IOCTL_SET_DMA, 0);		
		
		spi_addr[0] = 0x06;		
		ioctl(fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(fd, spi_addr, 1);		
		ioctl(fd, SPI_IOCTL_CS_DISABLE, 0);			
		spi_addr[0] = 0x02;		
		spi_addr[1] = ((addr>>16) & 0xFF);		
		spi_addr[2] = ((addr>>8) & 0xFF);		
		spi_addr[3] = (addr & 0xFF);		  			
		ioctl(fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(fd, spi_addr, 4);
		write(fd, (void*)&Global_User_Optins, 256);		
		ioctl(fd, SPI_IOCTL_CS_DISABLE, 0);	


		//for(i=0;i<256;i++)
		//	printf("buffer[%d]=%d\n",i,*(P+i));
		close(fd);
		Global_User_Optins.item.ifdirty = 0;
    }	
}



UINT8 RTC_get(void)
{
	time_t t;
	struct tm tm;	
	time(&t);
	tm = *localtime(&t);

	//printf("RTC GET\n");
	if(tm.tm_year>=112&&tm.tm_year<=137) //rtc power down
	{
		setup_date_time[0] = tm.tm_year + 1900 - 2000;
		setup_date_time[1] = tm.tm_mon + 1;
		setup_date_time[2] = tm.tm_mday;
		setup_date_time[3] =  tm.tm_hour;
		setup_date_time[4] = tm.tm_min;
		setup_date_time[5] = tm.tm_sec;			
	}
	else
	{
		setup_date_time[0] = Global_User_Optins.item.factory_date[0];
		setup_date_time[1] = Global_User_Optins.item.factory_date[1];
		setup_date_time[2] = Global_User_Optins.item.factory_date[2];
		setup_date_time[3] = Global_User_Optins.item.factory_date[3];
		setup_date_time[4] = Global_User_Optins.item.factory_date[4];
		setup_date_time[5] = Global_User_Optins.item.factory_date[5];	
		RTC_set();
	}
}
void RTC_set(void)
{
	int rtc = open("/dev/rtc0", O_RDWR);
	struct rtc_time input = {0};
	int ret;

	input.tm_year = setup_date_time[0] +2000 - 1900;
	input.tm_mon = setup_date_time[1] - 1;
	input.tm_mday= setup_date_time[2];
	 input.tm_hour =setup_date_time[3];
	input.tm_min =setup_date_time[4];
	input.tm_sec = setup_date_time[5];		
	if (rtc < 0){
		printf("Can't open [dev/rtc0]\n");
	}	
	ret = ioctl(rtc, RTC_SET_TIME, &input);
	close(rtc);	
	system("hwclock -s");
}


static int Format(char* mount_name )
{
	int ret = 0;
	int fat_type = -1;
	
	char buffer[256]={0};
	unsigned char dev_name[32];
	FILE*   fp=NULL;
	unsigned int size=0;
	char* ptr = NULL;
	/*got mount device name from mount_name*/
	sprintf(buffer,"mount | grep -w \"%s\"", mount_name);
	fp = popen(buffer, "r");
	if( NULL == fp ) {
		ret = -1;
		goto __fail;
	}
	ret = fread( buffer, sizeof(unsigned char), sizeof(buffer), fp);
	pclose(fp);
	ptr = strstr(buffer, " on ");
	if( NULL != ptr ) {
		memset(dev_name, 0, sizeof(dev_name));
		strncpy(dev_name, buffer, ptr-buffer);
		/* umount volume*/
		sprintf(buffer,"umount \"%s\"", mount_name);
		fp = popen(buffer, "r");
		if( NULL == fp ) {
			ret = -3;
			goto __fail;
		}
		ret = fread( buffer, sizeof(unsigned char), sizeof(buffer), fp);
		pclose(fp);
		ptr = strstr(buffer, " can't umount");
		if( NULL != ptr ) {
			ret = -3;
			goto __fail;
		}
	}  else {
		ret = -2;
		goto __fail;
	}

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "blockdev --getsz %s", dev_name);

	fp = popen(buffer, "r");
	if (NULL == fp) {
		printf("[FAIL] popen %s\r\n", buffer);
		goto __fail;
	}

	memset(buffer, 0, sizeof(buffer));
	ret = fread(buffer, sizeof(char), sizeof(buffer), fp);
	pclose(fp);

	size = atoi(buffer);

	if (size <  4194304) { // 2G = 4194304 sectors * 512 bytes
		fat_type = 1;	// FAT16
		printf("format type:FAT16\r\n");
	}
	else if (size < 67108864) { // 32G = 67108864 sectors  * 512 bytes
		fat_type = 2;	// FAT32
		printf("format type:FAT32\r\n");
	}
	else {
		fat_type = 3;	// exFAT
	//	printf("format type:exFAT\r\n");
		printf("format type:FAT32(64G up)\r\n");
	}
	
	/*format*/
	memset(buffer, 0, sizeof(buffer));
	if (fat_type == 1)
		sprintf(buffer,"mkfs.fat -F 16 -s 64 \"%s\"", dev_name);
	else if (fat_type == 2)
		sprintf(buffer,"mkfs.fat -F 32 -s 64 \"%s\"", dev_name);
	else if (fat_type == 3)
	//	sprintf(buffer,"mkfs.exfat \"%s\"", dev_name);
		sprintf(buffer,"mkfs.fat -F 32 -s 128 \"%s\"", dev_name);
	
	fp = popen(buffer, "r");
	if( NULL == fp ) {
		ret = -4;
		goto __fail;
	}
	ret = fread( buffer, sizeof(unsigned char), sizeof(buffer), fp);
	pclose(fp);

	/*re-mount*/
//	if (fat_type != 3)
		sprintf(buffer,"mount -t vfat -o utf8 \"%s\" \"%s\"", dev_name, mount_name);
//	else
//		sprintf(buffer,"mount.exfat \"%s\" \"%s\"", dev_name, mount_name);
	
	fp = popen(buffer, "r");
	if( NULL == fp ) {
		ret = -5;
		goto __fail;
	}
	ret = fread(buffer, sizeof(unsigned char), sizeof(buffer), fp);
	pclose(fp);
	printf("format OK\n");
	return 0;
__fail:
	printf("format FAIL,errorId=%d\n", ret);
	return ret; 
}

int Format2(void )
{
	int ret = 0;
	int fat_type = -1;
	
	char buffer[256]={0};
	unsigned char dev_name[32];
	FILE*   fp=NULL;
	unsigned int size=0;
	char* ptr = NULL;
	/*got mount device name from mount_name*/
	 if( 0==access("/dev/sdcarda", F_OK))
	{
		strncpy(dev_name, "/dev/sdcarda", sizeof(dev_name));
		strncpy(Memory_path, "/media/sdcarda", sizeof(Memory_path));
	}
	else if(0==access("/media/sdcardb", F_OK))
	{
		strncpy(dev_name, "/dev/sdcardb", sizeof(dev_name));
		strncpy(Memory_path, "/media/sdcarda", sizeof(Memory_path));
	}	
	else if( 0==access("/media/sdcardc", F_OK))
	{
		strncpy(dev_name, "/dev/sdcardc", sizeof(dev_name));
		strncpy(Memory_path, "/media/sdcarda", sizeof(Memory_path));
		
	}
	else
	{
		printf(" Get dev PATH fail\n");
		ret = -1;
		goto __fail;
	}

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "blockdev --getsz %s", dev_name);

	fp = popen(buffer, "r");
	if (NULL == fp) {
		printf("[FAIL] popen %s\r\n", buffer);
		goto __fail;
	}

	memset(buffer, 0, sizeof(buffer));
	ret = fread(buffer, sizeof(char), sizeof(buffer), fp);
	pclose(fp);

	size = atoi(buffer);

	if (size <  4194304) { // 2G = 4194304 sectors * 512 bytes
		fat_type = 1;	// FAT16
		printf("format type:FAT16\r\n");
	}
	else if (size < 67108864) { // 32G = 67108864 sectors  * 512 bytes
		fat_type = 2;	// FAT32
		printf("format type:FAT32\r\n");
	}
	else {
		fat_type = 3;	// exFAT
	//	printf("format type:exFAT\r\n");
		printf("format type:FAT32(64G up)\r\n");
	}
	
	/*format*/
	if (fat_type == 1)
		sprintf(buffer,"mkfs.fat -F 16 -s 64 \"%s\"", dev_name);
	else if (fat_type == 2)
		sprintf(buffer,"mkfs.fat -F 32 -s 64 \"%s\"", dev_name);
	else if (fat_type == 3)
	//	sprintf(buffer,"mkfs.exfat \"%s\"", dev_name);
		sprintf(buffer,"mkfs.fat -F 32 -s 128 \"%s\"", dev_name);
	
	fp = popen(buffer, "r");
	if( NULL == fp ) {
		ret = -4;
		goto __fail;
	}
	ret = fread( buffer, sizeof(unsigned char), sizeof(buffer), fp);
	pclose(fp);

	/*re-mount*/
	sprintf(buffer,"mkdir \"%s\"", Memory_path);
	fp = popen(buffer, "r");
	if( NULL == fp ) {
		ret = -6;
		goto __fail;
	}
	ret = fread( buffer, sizeof(unsigned char), sizeof(buffer), fp);
	pclose(fp);

//	if (fat_type != 3)
		sprintf(buffer,"mount -t vfat -o utf8 \"%s\" \"%s\"", dev_name, Memory_path);
//	else
//		sprintf(buffer,"mount.exfat \"%s\" \"%s\"", dev_name, Memory_path);

	fp = popen(buffer, "r");
	if( NULL == fp ) {
		ret = -5;
		goto __fail;
	}
	ret = fread( buffer, sizeof(unsigned char), sizeof(buffer), fp);
	pclose(fp);
	printf("format OK\n");
	return 0;
__fail:
	printf("format FAIL,errorId=%d\n", ret);
	return ret; 
}

int check_cluster_size() {
	int ret = 0;
	char buffer[256]={0};
	char dev_name[32];
	char* ptr = NULL;
	
	FILE* fp=NULL;

	int fat_type=-1;
	int cluster_size=0;
	
	/*got mount device name from mount_name*/
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer,"mount |grep -w \"%s\"", Memory_path);
	fp = popen(buffer, "r");
	if (NULL == fp) {
		printf("[FAIL] popen %s\r\n", buffer);
		goto error;
	}
	memset(buffer, 0, sizeof(buffer));
	ret = fread(buffer, sizeof(char), sizeof(buffer), fp);
	pclose(fp);
	fp = NULL;
	ptr = strstr(buffer, " on ");
	if (NULL != ptr) {
		memset(dev_name, 0, sizeof(dev_name));
		strncpy(dev_name, buffer, ptr-buffer);
	}  else {
		printf("[FAIL] can not find dev path\r\n");
		goto error;
	}

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "blkid %s", dev_name);

	fp = popen(buffer, "r");
	if (NULL == fp) {
		printf("[FAIL] popen %s\r\n", buffer);
		goto error;
	}

	memset(buffer, 0, sizeof(buffer));
	ret = fread(buffer, sizeof(char), sizeof(buffer), fp);
	pclose(fp);
	fp = NULL;
	if (NULL != strstr(buffer, "vfat")) {
		fat_type = 1;
	}
	else if (NULL != strstr(buffer, "exfat")) {
		fat_type = 2;
	}
	else {
		printf("[FAIL] unknown format\r\n");
		goto error;
	}

	memset(buffer, 0, sizeof(buffer));

	if (fat_type == 1)
		sprintf(buffer, "fsck.fat -v %s | grep -w \"bytes per cluster\" | awk '{print $1}'", dev_name);
	else if (fat_type == 2)
		sprintf(buffer, "fsck.exfat %s | grep -w \"Cluster size\" | awk '{print $3}'", dev_name);
	
	fp = popen(buffer, "r");
	if (NULL == fp) {
		printf("[FAIL] popen %s\r\n", buffer);
		goto error;
	}

	memset(buffer, 0, sizeof(buffer));
	ret = fread(buffer, sizeof(char), sizeof(buffer), fp);
	pclose(fp);
	fp = NULL;

	cluster_size = atoi(buffer);

	if (fat_type == 2) {
		cluster_size = 0;						// not support exfat
	//	cluster_size = cluster_size  * 1024;
	}

	
	printf("bytes per cluster =%d\r\n", cluster_size);


	if (cluster_size >= 32768)
	{
		dv_set.sdc_error = 0;
		printf("cluster size is larger than 32K!\r\n");

		if (fat_type == 1) // auto-repair fat table
		{
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer,"fsck.fat -a \"%s\"", dev_name);
			fp = popen(buffer, "r");
			if (NULL == fp) {
				printf("[FAIL] popen %s\r\n", buffer);
			}
			memset(buffer, 0, sizeof(buffer));
			ret = fread(buffer, sizeof(char), sizeof(buffer), fp);
			pclose(fp);
			fp = NULL;
		}	
	}
	else {
		dv_set.sdc_error= 1;
		time_num = 2;
		printf("cluster size is incorrect! please format!\r\n");
	}
			
	return 0;

error:
	if (fp)
		pclose(fp);
	fp = NULL;
	
	dv_set.sdc_error= 1;
	time_num = 2;
	return -1;
}

SINT8 SDC_Format(void)
{ 
	Format(Memory_path);
	return 0;	
}
static UINT32 DiskFreeSpace(char *pDisk)
{
	long long freespace = 0;     
	struct statfs disk_statfs; 
	if( statfs(pDisk, &disk_statfs) >= 0 ) 
	{ 
		freespace = (((long long)disk_statfs.f_bsize  * (long long)disk_statfs.f_bfree)/(long long)1024); 
		//totalspace = (((long long)disk_statfs.f_bsize * (long long)disk_statfs.f_blocks) /(long long)1024); 
		//printf("%s free size(KB)=%d\n", pDisk,freespace);
		return freespace;
	}
}

UINT32 SDC_FreeSize(void)
{ 
	return DiskFreeSpace(Memory_path);	
}

SINT32 CheckMemoryPath(void)
{
	SINT32 ret = 0;
	 if( 0==access("/media/sdcarda", F_OK))
	{
		strncpy(Memory_path, "/media/sdcarda", sizeof(Memory_path));
	}
	else if(0==access("/media/sdcarda1", F_OK))
	{
		strncpy(Memory_path, "/media/sdcarda1", sizeof(Memory_path));
	}
	else if(0==access("/media/sdcardb", F_OK))
	{
		strncpy(Memory_path, "/media/sdcardb", sizeof(Memory_path));
	}	
	else if(0==access("/media/sdcardb1", F_OK))
	{
		strncpy(Memory_path, "/media/sdcardb1", sizeof(Memory_path));
	}
	else if( 0==access("/media/sdcardc", F_OK))
	{
		strncpy(Memory_path, "/media/sdcardc", sizeof(Memory_path));
	}
	else if(0==access("/media/sdcardc1", F_OK))
	{
		strncpy(Memory_path, "/media/sdcardc1", sizeof(Memory_path));
	}
	else
	{
		printf(" Get Memory PATH fail\n");
		return -1;
	}
	printf("Memory PATH is [%s]\n",Memory_path);
	return 0;
}

UINT8 Memory_Check(void)
{
	gp_board_sd_t *pSDC0 = gp_board_get_config("sd0", gp_board_sd_t);
	gp_board_sd_t *pSDC2 = gp_board_get_config("sd2", gp_board_sd_t);

	if(pSDC0->detect()==1)
	return pSDC0->detect();
	else if(pSDC2->detect()==1)	
	return pSDC2->detect();
	else
	return 0;
}

void USB_power_or_pc_check(void)
{
	int tmp = 0;
	//while(1)
	{
		dv_set.usb_fd = open("/dev/usb_device", O_RDWR);
		if(dv_set.usb_fd >= 0)
		{
			system("modprobe g_spmp_storage file=/dev/ram0 stall=1 removable=1 mode=0 detect_mode=1");	
			usleep(500000);	
			ioctl(dv_set.usb_fd, USBDEVFS_GET_CONNECT_TYPE, &tmp);
			if(tmp == USB_DEVICE_CONNECT_TO_HOST)
			{
				dv_set.usb_detect = 1; //usb insert PC
				printf("USB_DEVICE_CONNECT_TO_HOST !\n");
				close(dv_set.usb_fd);
				system("rmmod g_spmp_storage");	
				system("rmmod spmp_udc_b");
				system("modprobe gp_usb");
				dv_set.usb_fd = -1;
			}
			else// if(tmp == USB_DEVICE_CONNECT_TO_CHARGER)
			{
				dv_set.battery_state = 5;
				printf("USB_DEVICE_CONNECT_TO_CHARGER !\n");
				USB_time_num = 5;
			}
		}
		//sleep(1);
	}
	
}

UINT8 USB_Check(void)
{
	gp_board_usb_t *pUsb = gp_board_get_config("usb", gp_board_usb_t);
	if(pUsb && pUsb->slave_detect)
	{
		return pUsb->slave_detect();
	}
	else
	{
		return 0;
	}
}
UINT8 TV_Check(void)
{
	gp_board_TV_t *pTV = gp_board_get_config("TV", gp_board_TV_t);
	if(pTV && pTV->detect_tv_in)	
	{
		//printf(" @@@@@@@@@@@@@CHECK TV=[%d]\n",pTV->detect_tv_in());
		return pTV->detect_tv_in();
	}
	else
	{
		return 0;
	}	
}
UINT8 HDMI_Check(void)
{
	gp_board_HDMI_t *pHDMI = gp_board_get_config("HDMI", gp_board_HDMI_t);
	if(pHDMI && pHDMI->detect_hdmi_in)	
	{
		return pHDMI->detect_hdmi_in();
	}
	else
	{
		return 0;
	}
	
}
UINT8 LCD_Backlight_Set(UINT8 value)
{
	gp_board_panel_t *pPanel = gp_board_get_config("panel", gp_board_panel_t);
	if(pPanel && pPanel->set_backlight)	
	{
		 pPanel->set_backlight(value);
		return 0;
	}
	else
	{
		return 0xff;
	}
}
UINT8 LED_Set(UINT8 type,UINT8 value) //type 0:IR 1:LED
{
	gp_board_led_t *pLed = gp_board_get_config("led", gp_board_led_t);
	if(pLed && pLed->set_led_light)	
	{
		 pLed->set_led_light(type,value);
		return 0;
	}
	else
	{
		return 0xff;
	}	
}

UINT8 Power_Off_Set(void)
{
/*	gp_board_t *pPower = gp_board_get_config("board", gp_board_t);
	if(pPower && pPower->power_off)	
	{
		 pPower->power_off();
		return 0;
	}
	else
	{
		return 0xff;
	}*/
	LED_Set(POWER_OFF,0);
}

UINT8 Battery_voltage_detect()
{
	gp_board_system_t *pSystem = gp_board_get_config("sys_pwr", gp_board_system_t);
	if(pSystem && pSystem->detect_battery_voltage)	
	{
		 return pSystem->detect_battery_voltage();
	}
	else
	{
		return 0;
	}	
	
}

UINT8 Power_Key_detect()
{
	gp_board_system_t *pSystem = gp_board_get_config("sys_pwr", gp_board_system_t);
	if(pSystem && pSystem->detect_dc_in)	
	{
		 return pSystem->detect_dc_in();
	}
	else
	{
		return 0;
	}	
}

UINT8 Speaker_set_power( int enable )
{
	gp_board_audio_t *pAudio = gp_board_get_config("audio_out", gp_board_audio_t);
	if(pAudio && pAudio->set_speaker_power)	
	{
		 return pAudio->set_speaker_power( enable );
	}
	else
	{
		return 0;
	}	
}
void board_config_set_init(UINT8 type)
{
	UINT8 ret;

	if((type &0x01) != 0)
	{
		dv_set.display_mode = SP_DISP_OUTPUT_LCD;
		#ifdef SYSCONFIG_DISP0_TVOUT
		ret = TV_Check();
		if(ret == 1)
		{
			dv_set.display_mode = SP_DISP_OUTPUT_TV;
			dv_set.tv_check = 1;
		}
		else
		{
			dv_set.tv_check = 0;
			dv_set.display_mode = SP_DISP_OUTPUT_LCD;
		}
		#endif
		ret = HDMI_Check();
		if(ret == 1)
		{
			dv_set.display_mode = SP_DISP_OUTPUT_HDMI;
			dv_set.hdmi_check = 1;
		}
		else
		{
			dv_set.hdmi_check = 0;
			//dv_set.display_mode = SP_DISP_OUTPUT_LCD;
		}
		printf("DISPLAY IS [%d]\n",dv_set.display_mode);
		if(dv_set.display_mode == SP_DISP_OUTPUT_HDMI)	
		{
			HDMIxNum = 4;
			HDMI_X_offset = DISP_HDMI_X_OFFSET1;
			HDMI_Y_offset = DISP_HDMI_Y_OFFSET1;	
		}
		else
		{
			HDMIxNum = 1;
			HDMI_X_offset = 0;
			HDMI_Y_offset = 0;		
		}
		ret = Memory_Check();
		if(ret != 0xff)
		{
			dv_set.sd_check = ret;
		}
	}
	if((type &0x02) != 0)
	{
		ret = USB_Check();
		if(ret != 0xff)
		{
			if(ret == 1) //vbus high
			{
				dv_set.usb_detect = 1;
				USB_power_or_pc_check();
				if((dv_set.usb_detect == 1)&&(dv_set.battery_state!=5))
				{
					USB_entry_init();
				}
			}
		}
	}
	
}

void statusScanThread( void)
{
	UINT8 ret;
	UINT8 msg_id;
	UINT8 old_disp;
	//check sdc insert
	old_disp = dv_set.display_mode;
	if(dv_set.no_power_flag != 0&&dv_set.upgrade_flag!=2)
	{
		printf("NO Power,enter start,Power_off_num = [%d]\n",dv_set.Power_off_num);	
		dv_set.Power_off_num ++;
		if(dv_set.Power_off_num >=15)
		{
			Power_off();			
		}
		return;
	}
	if(dv_set.no_charge_flag != 0&&dv_set.upgrade_flag != 2)
	{
		printf("Charge is plut out,5s power off num = [%d]\n",dv_set.Power_off_num);	
		dv_set.Power_off_num ++;
		if(dv_set.Power_off_num >=6)
		{
			Power_off();			
		}
	}	
	ret = Memory_Check();
	if((ret != dv_set.sd_check)&&(dv_set.sd_check <= 1))
	{
		printf(" insert or plugout sdc,Enter Power Off Flow\n");
		dv_set.sd_check = ret;
		Power_off();
	}
	
	ret = USB_Check();
	if((ret != dv_set.usb_detect))
	{
		dv_set.usb_detect = ret;
		if(ret == 0)
		{
			if(dv_set.battery_state!=5)
			{
				Power_off();
				printf("Plug out USB,Enter Power Off Flow\n");
			}
			else
			{
				dv_set.battery_state = 4;
				dv_set.battery_state = Battery_voltage_detect();
				dv_set.no_charge_flag = 1;
				dv_set.Power_off_num = 0;
			}	
		}
		else if(ret == 1)
		{
			USB_power_or_pc_check();
			if((dv_set.usb_detect == 1)&&(dv_set.battery_state!=5))
			{
				USB_entry_init();
			}
			dv_set.no_charge_flag = 0;
		}
	}
	ret = HDMI_Check();
	if((ret != dv_set.hdmi_check)&&(ret != 0xff)&&(dv_set.dv_UI_flag < DV_EXIT_MODE))
	{
		if(ret == 0)
		{
			printf("@@@detect HDMI plut out!\n");
			dv_set.hdmi_check = 0;
			if(dv_set.dv_ctrl == 1)
			{
				CVR_Set_PIPE_With_CMD(CMD_STOP_DV,1);
				dv_set.dv_ctrl =0;
				dv_set.lock_flag = 0;
			}
			else if(dv_set.dc_ctrl == 1)
			{
				dv_set.dc_ctrl = 0;
			}
			while(1)
			{
				if(dv_set.draw_start == 0)
				{
					dv_set.change_disp = 1;
					break;
				}
			}		
			if(dv_set.tv_check == 1)
			{
				dv_set.display_mode = SP_DISP_OUTPUT_TV;
			}
			else
			{
				dv_set.display_mode = SP_DISP_OUTPUT_LCD;
			}
			HDMIxNum = 1;
			HDMI_X_offset = 0;
			HDMI_Y_offset = 0;				
			dv_set.dv_UI_flag = DV_EXIT_MODE;
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);				

			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				LCD_Backlight_Set(0);
			}
			Dsp_close();
			usleep(10000);
			if(old_disp != SP_DISP_OUTPUT_LCD)
			dispDestroy(hDisp);

			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				if (dispCreate(&hDisp, DISP_LAYER_OSD,dv_set.display_mode) != SP_OK) {
						printf("dispCreate error\n");
				}
			}
			else
			{
				hDisp = hDisp1;	
			}			
			dv_set.change_disp = 0;
			cur_draw_flag = DV_EXIT_MODE;
			foreground_draw_flag=0;	
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);
			if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
			{
				dv_set.backlight_flag = 1;
				gettimeofday(&starttime,NULL);
				LCD_Backlight_Set(1);
				Speaker_set_power(1);
			}			
			else { // hdmi -> tv
				Speaker_set_power(0);
			}
			Dsp_Open();				
		}
		else if(ret == 1)
		{
			printf("@@@detect HDMI plut in!\n");
			dv_set.hdmi_check = 1;
			if(dv_set.dv_ctrl == 1)
			{
				CVR_Set_PIPE_With_CMD(CMD_STOP_DV,1);
				dv_set.dv_ctrl =0;
				dv_set.lock_flag = 0;
			}
			else if(dv_set.dc_ctrl == 1)
			{
				dv_set.dc_ctrl = 0;
			}
			while(1)
			{
				if(dv_set.draw_start == 0)
				{
					dv_set.change_disp = 1;
					break;
				}
			}	
			dv_set.display_mode = SP_DISP_OUTPUT_HDMI;		
			HDMIxNum = 4;
			HDMI_X_offset = DISP_HDMI_X_OFFSET1;
			HDMI_Y_offset = DISP_HDMI_Y_OFFSET1;				
			dv_set.dv_UI_flag = DV_EXIT_MODE;
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				LCD_Backlight_Set(0);
			}		
			Dsp_close();
			usleep(10000);
			if(old_disp != SP_DISP_OUTPUT_LCD)
			dispDestroy(hDisp);	

			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				if (dispCreate(&hDisp, DISP_LAYER_OSD,dv_set.display_mode) != SP_OK) {
						printf("dispCreate error\n");
				}
			}
			else
			{
				hDisp = hDisp1;
			}			
			dv_set.change_disp = 0;
			cur_draw_flag = DV_EXIT_MODE;
			foreground_draw_flag=0;	
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
			if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
			{
				dv_set.backlight_flag = 1;
				gettimeofday(&starttime,NULL);
				LCD_Backlight_Set(1);
			}	
			Dsp_Open();
			Speaker_set_power(0);
	
		}
	}	
	#ifdef SYSCONFIG_DISP0_TVOUT
	ret = TV_Check();
	if((ret != dv_set.tv_check)&&(ret != 0xff)&&(dv_set.dv_UI_flag < DV_EXIT_MODE))
	{
		if(ret == 0)
		{
			printf("@@@detect TV plut out!\n");
			dv_set.tv_check = 0;
			if(dv_set.dv_ctrl == 1)
			{
				CVR_Set_PIPE_With_CMD(CMD_STOP_DV,1);
				dv_set.dv_ctrl =0;	
				dv_set.lock_flag = 0;
			}
			else if(dv_set.dc_ctrl == 1)
			{
				dv_set.dc_ctrl = 0;
			}
			while(1)
			{
				if(dv_set.draw_start == 0)
				{
					dv_set.change_disp = 1;
					break;
				}
			}
			if(dv_set.hdmi_check == 1)
			{
				dv_set.display_mode = SP_DISP_OUTPUT_HDMI;
				HDMIxNum = 4;
				HDMI_X_offset = DISP_HDMI_X_OFFSET1;
				HDMI_Y_offset = DISP_HDMI_Y_OFFSET1;	
			}
			else
			{
				dv_set.display_mode = SP_DISP_OUTPUT_LCD;
				HDMIxNum = 1;
				HDMI_X_offset = 0;
				HDMI_Y_offset = 0;				
			}
			dv_set.dv_UI_flag = DV_EXIT_MODE;
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				LCD_Backlight_Set(0);
			}			
			Dsp_close();
			usleep(10000);
			if(old_disp != SP_DISP_OUTPUT_LCD)
			dispDestroy(hDisp);

			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				if (dispCreate(&hDisp, DISP_LAYER_OSD,dv_set.display_mode) != SP_OK) {
						printf("dispCreate error\n");
				}
			}
			else
			{
				hDisp = hDisp1;	
			}			
			dv_set.change_disp = 0;
			cur_draw_flag = DV_EXIT_MODE;
			foreground_draw_flag=0;	
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
			if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
			{
				dv_set.backlight_flag = 1;
				gettimeofday(&starttime,NULL);
				LCD_Backlight_Set(1);
			}				
			Dsp_Open();
			Speaker_set_power(1);
		}
		else if(ret == 1)
		{
			printf("@@@detect TV plut in!\n");
			dv_set.tv_check = 1;
			if(dv_set.dv_ctrl == 1)
			{
				CVR_Set_PIPE_With_CMD(CMD_STOP_DV,1);
				dv_set.dv_ctrl =0;	
				dv_set.lock_flag = 0;
			}
			else if(dv_set.dc_ctrl == 1)
			{
				dv_set.dc_ctrl = 0;
			}
			while(1)
			{
				if(dv_set.draw_start == 0)
				{
					dv_set.change_disp = 1;
					break;
				}
			}
			dv_set.display_mode = SP_DISP_OUTPUT_TV;	
			HDMIxNum = 1;
			HDMI_X_offset = 0;
			HDMI_Y_offset = 0;				
			dv_set.dv_UI_flag = DV_EXIT_MODE;
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				LCD_Backlight_Set(0);
			}			
			Dsp_close();
			usleep(10000);
			if(old_disp != SP_DISP_OUTPUT_LCD)
			dispDestroy(hDisp);

			if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
			{
				if (dispCreate(&hDisp, DISP_LAYER_OSD,dv_set.display_mode) != SP_OK) {
						printf("dispCreate error\n");
				}
			}
			else
			{
				hDisp = hDisp1;	
			}			
			dv_set.change_disp = 0;
			cur_draw_flag = DV_EXIT_MODE;
			foreground_draw_flag=0;	
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);		
			if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
			{
				dv_set.backlight_flag = 1;
				gettimeofday(&starttime,NULL);
				LCD_Backlight_Set(1);
			}				
			Dsp_Open();
			Speaker_set_power(0);		
		}
	}
	#endif
	if(dv_set.battery_state != 5)
	{
		if(dv_set.battery_state != 0)
		{
			if(old_battery != dv_set.battery_state && dv_set.dv_UI_flag == DV_FOREGROUND_MODE)
			{
				msg_id = dv_set.dv_UI_flag;
				ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);				
			}
			old_battery = dv_set.battery_state;
		}
		dv_set.battery_state = Battery_voltage_detect();
		if(dv_set.battery_state == 0)
		{
			printf("NO Power,enter start,Power_off_num = [%d]\n",dv_set.Power_off_num);	
			dv_set.Power_off_num ++;
			if(dv_set.Power_off_num <=5)
			{
				if(old_battery != 0)
				{
					dv_set.battery_state = old_battery;
				}
			}
			if((dv_set.Power_off_num >=10)&&(dv_set.no_power_flag == 0))
			{
				//show no power
				dv_set.no_power_flag = 1;
			}
		}
		else
		{
			if(dv_set.no_charge_flag == 0)
			dv_set.Power_off_num = 0;
			dv_set.no_power_flag = 0;			
		}
		
	}
	else
	{
		if(dv_set.no_charge_flag == 0)
		dv_set.Power_off_num = 0;
		dv_set.no_power_flag = 0;
		old_battery = 6;
	}

	
}

static int scan_list(media_on_pc_t *device_list)
{
	int index=0;
	DIR *fd;
	struct dirent *dir;
	int i;
	
	memset(device_list,0,sizeof(media_on_pc_t));
	
	fd = opendir("/media/");
	if(fd == NULL){
		printf("=========media path can not open ==========\n");
		return -1;
	}
	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;		
			}			
		if((dir->d_type == DT_DIR)&&strstr(dir->d_name, "sdcard")){
				sprintf(device_list->list[index].media_path,"/media/%s", dir->d_name);	
				sprintf(device_list->list[index].media_device,"/dev/%s", dir->d_name);
				device_list->list[index].media =media_SD;
				index++;
		}	
		if(index>7)
			break;
	}	
	device_list->total_num = index;
	printf("===============scan result ================\n");
	for(i=0;i<device_list->total_num;i++)
	{
		printf(" %d  media path [%s]",i,device_list->list[i].media_path);
		printf(" media device [%s]",device_list->list[i].media_device);
		printf("\n\r");
	}
	printf("===============scan result end================\n");
}
void unmount_dev(void)
{
	int i;
	char umount_info[256];
	scan_list(&g_device_list);
	for(int i=0;i<g_device_list.total_num;i++)
	{
		sprintf(umount_info,"umount %s",g_device_list.list[i].media_path);
		printf("system command %s\n",umount_info);
		system(umount_info);
	}
}


void usb_plugin()
{
	int i;
	char command[256],device_info[256];
	scan_list(&g_device_list);
	for(int i=0;i<g_device_list.total_num;i++)
	{	
		if(i==0)
			sprintf(device_info,"%s",g_device_list.list[i].media_device);
		else
			sprintf(device_info,"%s,%s",device_info,g_device_list.list[i].media_device);
	}
	sprintf(command,"modprobe g_spmp_storage file=%s  stall=1 removable=1,luns=%d, mode=0",device_info,g_device_list.total_num);
	printf("system command:%s",command);
	system(command);
}
void usb_plugout()
{
	int i=0;
	system("modprobe -r g_spmp_storage");
	/*for(i=0;i<g_device_list.total_num;i++)
	{
		char command[256];
		sprintf(command,"mount -o utf8 %s %s\n",g_device_list.list[i].media_device,g_device_list.list[i].media_path);
		printf("system command %s\n",command);
		system(command);
	}
	if(g_device_list.NAND_Support==1)
		system("nandsync");*/
}
void Enable_Sensor_Power(UINT8 value)
{
	gp_board_sensor_t *pSensor = gp_board_get_config("sensor2", gp_board_sensor_t);
	if(pSensor && pSensor->set_sensor_power)	
	{
		 pSensor->set_sensor_power(value);
	}	
}
void Enable_Sensor_Reset(UINT8 value)
{
	gp_board_sensor_t *pSensor = gp_board_get_config("sensor2", gp_board_sensor_t);
	if(pSensor && pSensor->set_sensor_reset)	
	{
		 pSensor->set_sensor_reset(value);
	}	
}

UINT8 Power_key_Check(void)
{
	gp_board_power_key_t *pPower= gp_board_get_config("Power_key", gp_board_power_key_t);
	if(pPower && pPower->power_key_value)	
	{
		printf(" @@@@@@@@@@@@@CHECK power_key=[%d]\n",pPower->power_key_value());
		return pPower->power_key_value();
	}
	else
	{
		return 0;
	}	
}
/**
 * check update package file.
 * param: file_path, to save package file path.
 * param: count, retry count. 100ms 
 * return 1: get update package file success and will be delete; 2: get update package file success with not delete. 0, get update package file error.
 */
int checkUpdateFile(char *file_path, char count)
{
	int ret = 0;

	do{
		if(CheckMemoryPath() == 0) //get memory path
		{
			sprintf(file_path, "%s/%s", Memory_path, "CVR_firmware.bin");
			if(access(file_path, F_OK) == 0) {
				ret = 2;
				break;
			}
			else {
				sprintf(file_path, "%s/%s", Memory_path, "CVR_firmware_mass.bin");
				if(access(file_path, F_OK) == 0) {
					ret = 1;
					break;
				}
			}
		}
		if(count == 0) {
			break;
		}
		count--;
		printf("%s:%d count:%d %s\n", __FUNCTION__, __LINE__, count, file_path);
		usleep(100*1000);	
	}while(1);
	printf("%s:%d ret:%d %s\n", __FUNCTION__, __LINE__, ret, file_path);
	return ret;
}


SINT32 SPI_Flash_update(void)
{
	int spi_fd,sdc_fd,read_sdc;
	UINT8 file_path[128];
	int ret;
	char spi_addr[4];
	int freq = 12000000;	
	UINT32 addr,block,file_size;
	char* pInput=NULL;
	int i,j;
	char led_flash = 1;
	char buffer[4];
	char buffer1[256];
	UINT32 loop_num;
	float  value,old_value;
	DIR *fd;
	struct dirent *dir;	
	int file_num = 0;
	int name_len=0;
	printf("SPI_Flash_update Enter\n");
	printf("============STEP 1==============\n");
	if(dv_set.sd_check == 0)
	{
		printf("NO SDC\n");
		return 0;
	}
	printf("============STEP 2==============\n");
#if 0
	strcpy(file_path, Memory_path);
	fd = opendir(file_path);
	if(fd == NULL){
		return 0;
	}
	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}
		printf("find name = %s\n",dir->d_name);
		if(dir->d_type != DT_DIR){
			if(strncmp(dir->d_name,"CVR_firmware_mass",17)==0)
			{
				file_num = 1;
				/*name_len = strlen(dir->d_name);
				printf("name_len = %d\n",name_len);
				if(name_len>17+5&&name_len<=33+5)
				{
					for(i=0;i<32;i++)
						Global_User_Optins.item.version[i]=0;
					strncpy(Global_User_Optins.item.version,(dir->d_name)+13+5,name_len-13-5-4);
					printf("version = %s\n",Global_User_Optins.item.version);
				}
				else
				{
					printf("version is too long\n");
				}*/
				break;
			}			
			else if(strncmp(dir->d_name,"CVR_firmware",12)==0)
			{
				file_num = 2;
				/*name_len = strlen(dir->d_name);
				printf("name_len = %d\n",name_len);
				if(name_len>17&&name_len<=33)
				{
					for(i=0;i<32;i++)
						Global_User_Optins.item.version[i]=0;
					strncpy(Global_User_Optins.item.version,(dir->d_name)+13,name_len-13-4);
					printf("version = %s\n",Global_User_Optins.item.version);
				}
				else
				{
					printf("version is too long\n");
				}*/
				break;
			}
				
		}
	}
	Get_cmputer_build_time();
	printf("version = %s\n",Global_User_Optins.item.version);
	if(file_num == 0)
	{
		closedir(fd);
		return 0;
	}
	strcat(file_path, "/");
	strcat(file_path, dir->d_name);
	closedir(fd);	
#else
#if (AUTO_UPDATE == 0)
	if(Power_key_Check() == 1) {
		file_num = checkUpdateFile(file_path, 20);
	}
	else {
		return 0;
	}
#else
	file_num = checkUpdateFile(file_path, 6);
#endif
	Get_cmputer_build_time();
	printf("version = %s\n",Global_User_Optins.item.version);
	if(file_num == 0)
	{
		return 0;
	}
#endif
	
	printf("Open file[%s] ",file_path);
	sdc_fd = open(file_path,O_RDWR);
	if(sdc_fd<0){	
		printf("Fail\n");
		return 0;
	}
	else
	{
		printf("OK\n");
	}
	file_size = lseek(sdc_fd,0,SEEK_END);
	lseek(sdc_fd,0,SEEK_SET);
	printf("file size = [%d]\n",file_size);
	if(file_size <=0)
	{
		close(sdc_fd);
		return 0;
	}
	pInput = gpChunkMemAlloc(file_size);
	if(pInput < 0)
	{
		close(sdc_fd);
		printf("ChunkMem Fail\n");
		return 0;
	}
	else
	{
		printf("pInput = [0x%x]\n",pInput);
	}
	ret = read(sdc_fd,pInput,file_size);
	if(ret != file_size)
	{
		close(sdc_fd);
		gpChunkMemFree(pInput);
		printf("read file Fail\n");		
	}
	close(sdc_fd);	
	printf("============STEP 3==============\n");
	spi_fd = open("/dev/spi",O_RDWR);
	if(spi_fd<0){		
		printf("Can't open [/dev/spi0]\n");		
		return 0;	
	}
	ioctl(spi_fd, SPI_IOCTL_SET_CHANNEL, 0);	//Set SPI CS0		
	ioctl(spi_fd, SPI_IOCTL_SET_FREQ, freq);		
	ioctl(spi_fd, SPI_IOCTL_SET_DMA, 0);	
	printf("============STEP 4==============\n");
	printf("Chip Erase\n");
	if(dv_set.display_mode == SP_DISP_OUTPUT_LCD) {
		LCD_Backlight_Set(1);
	}
	UPgrade_UI(0,0);
	UPgrade_UI(0,0);
	UPgrade_UI(1,0);
	UPgrade_UI(1,0);	
	spi_addr[0] = 0x06;		//erase commmand
	ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
	write(spi_fd, spi_addr, 1);		
	ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);			
					
	spi_addr[0] = 0x60;		//erase commmand
	ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
	write(spi_fd, spi_addr, 1);		
	ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);
	j=0;
	name_len = 0;
	do{
		LED_Set(NORMAL_LED,led_flash);
		led_flash = !led_flash;

		spi_addr[0] = 0x05;		
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 1);
		read(spi_fd, buffer, 1);	
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
		usleep(100000); //100ms
		j++;
		if(j%5 == 0)
		{
			printf("j=%d\n",j);
			name_len ++;
			if(name_len > 4)
				name_len = 0;
			UPgrade_UI(5,name_len);	
		}
		if(j >= 500)
		{
			printf("SPI ERASE TIMEOUT!\n");
			close(spi_fd);
			gpChunkMemFree(pInput);					
			return 0;
		}
	}while(buffer[0]& SPI_FLASH_SR_WIP != 0);				

	printf("============STEP 5==============\n");
	UPgrade_UI(2,0);
	UPgrade_UI(2,0);
	old_value = value = 0; 
	addr = 0;
	loop_num = file_size/256;
	if(file_size%256 != 0)
	{
		loop_num += 1;
	}
	printf("loop_num = [%d]\n",loop_num);


	for(i=0;i<loop_num;i++)
	{
		value = (i/(float)loop_num)*100;
		//printf("write addr = [0x%x]value =%x\n",addr,(int)value);
		if((int)old_value != (int)value)
		{
			LED_Set(NORMAL_LED,led_flash);
			led_flash = !led_flash;

			UPgrade_UI(3,(int)value);
			old_value = value;
			printf("%d\n",(int)value);
		}
		spi_addr[0] = 0x06;		
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 1);		
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);			
		spi_addr[0] = 0x02;		
		spi_addr[1] = ((addr>>16) & 0xFF);		
		spi_addr[2] = ((addr>>8) & 0xFF);		
		spi_addr[3] = (addr & 0xFF);		  			
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 4);
		write(spi_fd, pInput +i*256, 256);		
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
		j=0;
		do{
			spi_addr[0] = 0x05;		
			ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
			write(spi_fd, spi_addr, 1);
			read(spi_fd, buffer, 1);	
			ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
			/*j++;
			if(j >= 10000)
			{
				printf("SPI write TIMEOUT!\n");
				close(spi_fd);
				gpChunkMemFree(pInput);					
				return -1;
			}*/
		}while(buffer[0]& SPI_FLASH_SR_WIP != 0);			
		addr += 256;
		
	}	
	/*addr = 0;
	strcpy(file_path, Memory_path);
	strcat(file_path, "/GPL329XXB_SPI_ROM_read.bin");	
	read_sdc = open(file_path,O_CREAT | O_RDWR);
	for(i=0;i<loop_num;i++)
	{
		printf("read addr = [0x%x]\n",addr);
		spi_addr[0] = 0x03;		
		spi_addr[1] = ((addr>>16) & 0xFF);		
		spi_addr[2] = ((addr>>8) & 0xFF);		
		spi_addr[3] = (addr & 0xFF);				
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 4);
		read(spi_fd, buffer1, 256);	
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
		write(read_sdc,buffer1,256);
		addr += 256;
	}
	close(read_sdc);
	*/
	
	close(spi_fd);
	gpChunkMemFree(pInput);
	if(file_num == 2)
	{
		remove(file_path);
		system("sync");
	}
	UPgrade_UI(4,0);
	UPgrade_UI(4,0);
	LED_Set(NORMAL_LED,1);
	//Global_User_Optins.item.ifdirty = 1;
	//ap_state_config_store();
	printf("update finish,plut out sdc!\n");
	while(dv_set.sd_check == 1);
	return 1;
}
#define ASENSOR_IOCTL_ID	'G'
#define ASENSOR_IOCTL_GET_INT_ACTIVE	_IOR(ASENSOR_IOCTL_ID, 0x01, int)
#define ASENSOR_IOCTL_SET_SENSITIVE	_IOW(ASENSOR_IOCTL_ID, 0x02, int)
#define ASENSOR_IOCTL_PARK_MODE		_IOW(ASENSOR_IOCTL_ID, 0x04, int)
#define ASENSOR_IOCTL_PARK_MODE_INT	_IOR(ASENSOR_IOCTL_ID, 0x08, int)
char  Gsensor_read(void)
{
	int fd;
	int temp;
	fd = open("/dev/gp_asensor",O_RDWR);
	if(fd<0){		
		//printf("Can't open [/dev/gp_asensor]\n");		
		return 0;	
	}
	ioctl(fd, ASENSOR_IOCTL_GET_INT_ACTIVE, &temp);
	close(fd);
	return temp;
}
char  Gsensor_parking_mode_int_read(void)
{
	int fd;
	int temp;
	fd = open("/dev/gp_asensor",O_RDWR);
	if(fd<0){		
		//printf("Can't open [/dev/gp_asensor]\n");		
		return 0;	
	}	
	ioctl(fd, ASENSOR_IOCTL_PARK_MODE_INT, &temp);
	
	close(fd);
	return temp;
}

void  Gsensor_write(int value)
{
	int fd;
	int temp;
	//system("insmod /system/lib/modules/common/Asensor_DA380.ko"); 
	fd = open("/dev/gp_asensor",O_RDWR);
	if(fd<0){		
		//printf("Can't open [/dev/gp_asensor]\n");		
		return 0;	
	}
	ioctl(fd, ASENSOR_IOCTL_SET_SENSITIVE, value);
	close(fd);
	//system("rmmod Asensor_DA380");
}
void  Gsensor_parking_mode(int value)
{
	int fd;
	int temp;
	//system("insmod /system/lib/modules/common/Asensor_DA380.ko"); 
	fd = open("/dev/gp_asensor",O_RDWR);
	if(fd<0){		
		//printf("Can't open [/dev/gp_asensor]\n");		
		return 0;	
	}
	ioctl(fd, ASENSOR_IOCTL_PARK_MODE, value);
	close(fd);
	//system("rmmod Asensor_DA380");
}
void Get_cmputer_build_time(void)
{
	UINT8 temp[32] = __DATE__ ;
	UINT8 *temp0;
	UINT8 month[4]={0};
	UINT8 date[3]={0};
	UINT8 year[5]={0};
	UINT16 i,j,k;
	UINT8 Months[12][6]={"Jan01","Feb02","Mar03","Apr04","May05","Jun06","Jul07","Aug08","Sep09","Oct10","Nov11","Dec12"};
	
	
	temp0 = temp;
//	print_string("BUILD TIME: "__DATE__"-"__TIME__"\r\n" );
//	print_string("data:%s\r\n", temp );
//	print_string("data0:%s\r\n", temp0 );
	for(i=0;i<3;i++)
		month[i] = *(temp0+i);
	
	for(i=0;i<2;i++)
		date[i] = *(temp0+4+i);
	
	for(i=0;i<4;i++)
		year[i] = *(temp0+7+i);
	
//	print_string("month = %s\r\n", month );
//	print_string("date = %s\r\n", date );
//	print_string("year = %s\r\n", year );
	
	for(j=0;j<12;j++)
	{
//		gp_strncmp();
		for(i=0;i<3;i++)
		{
			if(month[i] == Months[j][i])
			{
				if(i == 2)
					break;
			}
			else
			{
				i = 0;
				break;
			}
			
		}
		if(i == 2)
		{
			for(k=0;k<2;k++)
			{
				month[k] = Months[j][i+1+k];	
			}
//			print_string("month = %s\r\n", month );
			break;
		}
	}
	
	for(i=0;i<4;i++)
		*(temp0+i) = year[i];
	for(i=0;i<2;i++)
		*(temp0+4+i) = month[i];
	for(i=0;i<2;i++)
		*(temp0+6+i) = date[i];
	
	//*(temp0+8) = ' ';
	sprintf(temp0+8, "%d", __PLATFORM_VER__);

	if(date[0] == ' ')
		*(temp0+6) = '0';	
	for(i=0;i<8+6;i++)
		Global_User_Optins.item.version[i] = *(temp0+i);

	//Global_User_Optins.item.version[8]= '\0';
	printf("data:%s\r\n", temp );
	printf("data0:%s\r\n", temp0 );
	
}
