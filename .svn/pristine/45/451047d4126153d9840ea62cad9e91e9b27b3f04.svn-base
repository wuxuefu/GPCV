/* Necessary includes for device drivers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mach/typedef.h"
#include "mach/gp_display.h"
#include "mach/gp_chunkmem.h"
#include "disp.h"
#include <mach/gp_hdmi.h>
#include "ap_state_config.h"
#include "mach/gp_scale.h"
#include "mach/gp_scale2.h"
#include "mach/typedef.h"
//#define dispscl_workaround
#define DRAW_HD
#include <mach/common.h>
#include <mach/typedef.h>

#include <mach/gp_display.h>
#include <mach/gp_ceva.h>
#include <mach/gp_scale.h>
#include "chunkmem.h"
#include <dlfcn.h>
#include "mach/gp_scale2.h"
#include "gp_ovg.h"
#include "ceva.h"
#include "image_decode.h"
#include <mach/gp_chunkmem.h>
#include "icver.h"
//#include "photo_decode.h" 
//#include "image_decode.h"
#include "gp_on2.h"

#define __MODULE__          "Disp"
#define __DBGLVL__          1 // 0=OFF, 1=ERROR, 2=WAENING 3=MSG 4=ALL
#define __DBGNAME__         printf
#include "dbgs.h"
//#include "../openplatform/sdk/include/dbgs.h"
extern dv_set_t dv_set;
char file_path[256];
extern UINT8 HDMIxNum;
extern LDW_DISPLINE_t ldw_get_displine;
extern LDW_DISPLINE_t ldw_displine;
iconManager_t Icon_Manager[ICON_MAX]={
    {0},
	{"Video.bin",		ICON_VIDEO						,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS,											TRANSPARENT_COLOR,NULL,0},              
	{"Video_m.bin",		ICON_VIDEO_M          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+19,UI_MENU_Y_POS+8,											TRANSPARENT_COLOR,NULL,0},         
	{"LoopRecording.bin",		ICON_LOOPRECORDING    ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},         
	{"LoopRecording2.bin",		ICON_LOOPRECORDING2   ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+ICON_HEIGHT,					TRANSPARENT_COLOR,NULL,0},         
	{"LoopRecording3.bin",		ICON_LOOPRECORDING3   ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+ICON_HEIGHT,					TRANSPARENT_COLOR,NULL,0},      
	{"LoopRecording5.bin",		ICON_LOOPRECORDING5   ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+ICON_HEIGHT,					TRANSPARENT_COLOR,NULL,0},        
	{"MotionDetect_f.bin",		ICON_MOTIONDETECT_F   ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},       
	{"MotionDetection.bin",		ICON_MOTIONDETECT_M   ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,						TRANSPARENT_COLOR,NULL,0},         
	{"Exposure.bin",		ICON_VIDEO_EXPOSURE   ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+4*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},          
 	{"Exposure.bin",		ICON_CAP_EXPOSURE   	,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,						TRANSPARENT_COLOR,NULL,0},
	{"Exposure_p2_0.bin",		ICON_EXPOSURE_P2_0    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},                                                                                       
	{"Exposure_p5_3.bin",		ICON_EXPOSURE_P5_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},    
	{"Exposure_p4_3.bin",		ICON_EXPOSURE_P4_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},         
	{"Exposure_p1_0.bin",		ICON_EXPOSURE_P1_0    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},   
	{"Exposure_p2_3.bin",		ICON_EXPOSURE_P2_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},   
	{"Exposure_p1_3.bin",		ICON_EXPOSURE_P1_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},   
	{"Exposure_p0_0.bin",		ICON_EXPOSURE_P0_0    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"Exposure_n1_3.bin",		ICON_EXPOSURE_N1_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},         
	{"Exposure_n2_3.bin",		ICON_EXPOSURE_N2_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},         
	{"Exposure_n1_0.bin",		ICON_EXPOSURE_N1_0    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},      
	{"Exposure_n4_3.bin",		ICON_EXPOSURE_N4_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},        
	{"Exposure_n5_3.bin",		ICON_EXPOSURE_N5_3    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},      
	{"Exposure_n2_0.bin",		ICON_EXPOSURE_N2_0    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},             
	{"WDR1.bin",		ICON_WDR_F            ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+2*ICON_WIDTH,ICON_LEFT_Y_POS,					TRANSPARENT_COLOR,NULL,0},                                                                                              
	{"WDR.bin",		ICON_WDR_M            ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},            
	{"Sdc.bin",		ICON_SDC              ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS+ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},           
	{"Int.bin",		ICON_INT              ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS+ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},           
	{"Charge.bin",		ICON_CHARGE           ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS,										TRANSPARENT_COLOR,NULL,0},       
	{"Battery0.bin",		ICON_BATTERY0         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS,										TRANSPARENT_COLOR,NULL,0},   
	{"Battery1.bin",		ICON_BATTERY1         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS,										TRANSPARENT_COLOR,NULL,0},
	{"Battery2.bin",		ICON_BATTERY2         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS,										TRANSPARENT_COLOR,NULL,0}, 
	{"Battery3.bin",		ICON_BATTERY3         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS,										TRANSPARENT_COLOR,NULL,0},
	{"Batteryx.bin",		ICON_BATTERYX         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS,										TRANSPARENT_COLOR,NULL,0},
	{"RedLight.bin",		ICON_REDLIGHT         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+ICON_WIDTH,ICON_LEFT_Y_POS,						TRANSPARENT_COLOR,NULL,0},                                                                                 
	{"Resolution.bin",		ICON_VIDEO_RESOLUTION ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,						TRANSPARENT_COLOR,NULL,0},       
	{"Resolution.bin",		ICON_CAP_RESOLUTION   ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},       
	{"BeepSound_Audio.bin",		ICON_RECORD_AUDIO     ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},     
	{"DateStamp.bin",		ICON_VIDEO_DATE_STAMP ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},      
	{"Protect_f.bin",		ICON_GSENSOR          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+4*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},     
	{"Menu.bin",		ICON_MENU             ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+28,UI_MENU_Y_POS+205,	TRANSPARENT_COLOR,NULL,0},      
	{"Up.bin",		ICON_UP               ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+142,UI_MENU_Y_POS+205,	TRANSPARENT_COLOR,NULL,0},         
	{"Down.bin",		ICON_DOWN             ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+142+25,UI_MENU_Y_POS+205,	TRANSPARENT_COLOR,NULL,0},          
	{"Left.bin",		ICON_LEFT             ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+142-25,UI_MENU_Y_POS+205,	TRANSPARENT_COLOR,NULL,0},          
	{"Right.bin",		ICON_RIGHT            ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+142+50,UI_MENU_Y_POS+205,	TRANSPARENT_COLOR,NULL,0},                                                                                                
	{"DateTime.bin",		ICON_DATE_TIME_SET    ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,						TRANSPARENT_COLOR,NULL,0},            
	{"BeepSound_Audio.bin",		ICON_BEEP_SOUND       ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},  
	{"Language.bin",		ICON_LANGUAGE         ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+4*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},              
	{"TVMode.bin",		ICON_TV_MODE          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},     
	{"Frequency.bin",		ICON_FREQUENCY        ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,						TRANSPARENT_COLOR,NULL,0}, 	     
	{"ScreenSaver.bin",		ICON_SCREEN_SAVER     ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},  
	{"IR_LED.bin",		ICON_IR_LED           ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+4*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},   
	{"Format.bin",		ICON_FORMAT           ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},   
	{"DefaultSetting.bin",		ICON_DEFAULT_SET      ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,						TRANSPARENT_COLOR,NULL,0}, 
	{"Version.bin",		ICON_VERSION          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},                                                                              
	{"Capture.bin",		ICON_CAPTURE          ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS,											TRANSPARENT_COLOR,NULL,0}, 	 
	{"Capture_m.bin",		ICON_CAPTURE_M        ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+19,UI_MENU_Y_POS+8,											TRANSPARENT_COLOR,NULL,0},      
	{"Capture.bin",		ICON_CAPTURE_M1       ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,						TRANSPARENT_COLOR,NULL,0},
	{"AWB.bin",		ICON_AWB_M            ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},
	{"AWB.bin",		ICON_AWB_F            ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},     
	{"Daylight.bin",		ICON_DAYLIGHT         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"Cloudy.bin",		ICON_CLOUDY           ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"Tungsten.bin",		ICON_TUNGSTEN         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"Fluorescent.bin",		ICON_FLUORESCENT      ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"Timer_2s.bin",		ICON_CAP_2S_TIMER     ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+ICON_WIDTH,ICON_LEFT_Y_POS,						TRANSPARENT_COLOR,NULL,0}, 
	{"Timer_5s.bin",		ICON_CAP_5S_TIMER     ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+ICON_WIDTH,ICON_LEFT_Y_POS,						TRANSPARENT_COLOR,NULL,0},  
	{"Timer_10s.bin",		ICON_CAP_10S_TIMER    ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+ICON_WIDTH,ICON_LEFT_Y_POS,						TRANSPARENT_COLOR,NULL,0},  
	{"Sequence.bin",		ICON_SEQUENCE         ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},
	{"Sequence.bin",		ICON_SEQUENCE_F         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+3*ICON_WIDTH,ICON_LEFT_Y_POS,					TRANSPARENT_COLOR,NULL,0},
	{"Quality.bin",		ICON_QUALITY          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+4*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},
	{"STAR_1.bin",		ICON_STAR_1           ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},
	{"STAR_2.bin",		ICON_STAR_2           ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},   
	{"STAR_3.bin",		ICON_STAR_3           ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_LEFT_Y_POS+2*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},   
	{"Sharpness.bin",		ICON_SHARPNESS        ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,						TRANSPARENT_COLOR,NULL,0},
	{"Color.bin",		ICON_COLOR            ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},
	{"ISO.bin",		ICON_ISO              ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+4*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},
	{"ISOAUTO.bin",		ICON_ISO_AUTO         ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+5*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"ISO100.bin",		ICON_ISO_100          ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+5*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"ISO200.bin",		ICON_ISO_200          ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+5*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"ISO400.bin",		ICON_ISO_400          ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS+5*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0}, 
	{"Anti_shaking.bin",		ICON_ANTI_SHAKING_M   ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},
	{"Anti_shaking.bin",		ICON_ANTI_SHAKING     ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+2*ICON_WIDTH,ICON_LEFT_Y_POS,					TRANSPARENT_COLOR,NULL,0},  
	{"no_Anti_shaking.bin",		ICON_NO_ANTI_SHAKING  ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+2*ICON_WIDTH,ICON_LEFT_Y_POS,					TRANSPARENT_COLOR,NULL,0},
	{"DateStamp_cap.bin",		ICON_CAP_DATE_STAMP   ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},
	{"setup.bin",		ICON_SETUP            ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+19+48,UI_MENU_Y_POS+8,										TRANSPARENT_COLOR,NULL,0},
	{"Playback_f.bin",		ICON_PLAYBACK_F       ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS,											TRANSPARENT_COLOR,NULL,0},
	{"Playback_m.bin",		ICON_PLAYBACK_M       ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+19,UI_MENU_Y_POS+8,										TRANSPARENT_COLOR,NULL,0},
	{"Delete.bin",		ICON_DELETE           ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,						TRANSPARENT_COLOR,NULL,0}, 	
	{"Protect.bin",		ICON_PROTECT          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},  
	{"Protect_f.bin",		ICON_PROTECT_F        ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS+3*ICON_WIDTH,ICON_LEFT_Y_POS,					TRANSPARENT_COLOR,NULL,0},
	{"Slide_show.bin",		ICON_SLIDE_SHOW       ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+3*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},  
	{"Play_start.bin",		ICON_PLAY_START       ,	ICON_WIDTH*2,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS,											TRANSPARENT_COLOR,NULL,0},
	{"Play_pause.bin",		ICON_PLAY_PAUSE       ,	ICON_WIDTH*2,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS,											TRANSPARENT_COLOR,NULL,0},
	{"volume.bin",		ICON_VOLUME           ,	ICON_WIDTH*4,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS,											TRANSPARENT_COLOR,NULL,0},
	{"Volume1.bin",		ICON_VOLUME1          ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS+ICON_HEIGHT,					TRANSPARENT_COLOR,NULL,0},
	{"Volume2.bin",		ICON_VOLUME2          ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS+ICON_HEIGHT,					TRANSPARENT_COLOR,NULL,0},
	{"Select.bin",		ICON_SELECT           ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+142+80,UI_MENU_Y_POS+205,	TRANSPARENT_COLOR,NULL,0},
	{"Date_d.bin",		ICON_DATE_D           ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+108,UI_MENU_Y_POS+68,	TRANSPARENT_COLOR,NULL,0},
	{"Date_y.bin",		ICON_DATE_Y           ,	ICON_WIDTH*2,ICON_HEIGHT,	UI_MENU_X_POS+92,UI_MENU_Y_POS+68,	TRANSPARENT_COLOR,NULL,0},
	{"Item_b.bin",		ICON_ITEM_B           ,	248,160,	UI_MENU_X_POS+56,UI_MENU_Y_POS+45,	TRANSPARENT_COLOR,NULL,0},
	{"Item_s.bin",		ICON_ITEM_S           ,	208,32,	UI_MENU_X_POS+76,UI_MENU_Y_POS+52,	TRANSPARENT_COLOR,NULL,0},
	{"Menu_b.bin",		ICON_MENU_B           ,	320,240,	UI_MENU_X_POS,UI_MENU_Y_POS,	TRANSPARENT_COLOR,NULL,0},
	{"Menu_s.bin",		ICON_MENU_S           ,	288,32,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+48,	TRANSPARENT_COLOR,NULL,0},
	{"Nofile.bin",		ICON_NOFILE           ,	216,128,	0,0,	TRANSPARENT_COLOR,NULL,0},
	{"Play_s.bin",		ICON_PLAY_S           ,	96,64,	40,140,	TRANSPARENT_COLOR,NULL,0},
	{"Top_s.bin",		ICON_TOP_S           ,	48,32,	UI_MENU_X_POS+12,UI_MENU_Y_POS+9,	TRANSPARENT_COLOR,NULL,0},
	{"Playback_p.bin",		ICON_PLAYBACK_P       ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_LEFT_Y_POS,											TRANSPARENT_COLOR,NULL,0},
	{"Mic_on.bin",		ICON_MIC_ON              ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},           
	{"Mic_off.bin",		ICON_MIC_OFF              ,	ICON_WIDTH,ICON_HEIGHT,	ICON_RIGHT_X_POS,ICON_RIGHT_Y_POS+3*ICON_HEIGHT,				TRANSPARENT_COLOR,NULL,0},           
	{"Gsensor.bin",		ICON_GSENSOR1          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},     
	{"LDW.bin",		ICON_LDW          ,	ICON_WIDTH,ICON_HEIGHT,	UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},     
	{"LDW_1.bin",		ICON_LDW_1          ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS+ICON_HEIGHT,			TRANSPARENT_COLOR,NULL,0},     
	{"Time_Lapse.bin",	   ICON_TIMELAPSE		  , ICON_WIDTH,ICON_HEIGHT, UI_MENU_X_POS+X_STEP,UI_MENU_Y_POS+2*Y_STEP+8,					TRANSPARENT_COLOR,NULL,0},	   
	{"SFCW.bin",	   ICON_TIMELAPSE			 ,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS,				   TRANSPARENT_COLOR,NULL,0},	  
	{"StopAndGo.bin",	   ICON_TIMELAPSE		,	ICON_WIDTH,ICON_HEIGHT,	ICON_LEFT_X_POS,ICON_RIGHT_Y_POS+ICON_HEIGHT,			   TRANSPARENT_COLOR,NULL,0},	  

};

menuManager_t PLAY_MENU[PLAY_MENU_NUM] = {
		//delete
		{
			.icon_Index = ICON_DELETE,
			.str_index =	STR_DELETE,
			.color = FONT_COLOR,
			.item_num = 2,	
			.item = 
			{
				{
					.str_index = STR_DELETE_CURRENT,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_DELETE_ALL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},	
				//
				{
					.str_index = STR_ERASE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},			
				{
					.str_index = STR_CANCEL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_OK,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},						
			}
			
		},

		//protect
		{
			.icon_Index = ICON_PROTECT,
			.str_index =	STR_PROTECT,
			.color = FONT_COLOR,
			.item_num = 4,	
			.item = 
			{
				{
					.str_index = STR_LOCK_CURRENT,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_UNLOCK_CURRENT,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
				{
					.str_index = STR_LOCK_ALL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_UNLOCK_ALL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},	
				//
				{
					.str_index = STR_LOCK_CURRENT,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},			
				{
					.str_index = STR_CANCEL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_OK,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},	
				
			}
			
		},

		//slide show
		{
			.icon_Index = ICON_SLIDE_SHOW,
			.str_index =	STR_SLIDE_SHOW,
			.color = FONT_COLOR,
			.item_num = 3,	
			.item = 
			{
			/*	{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},*/
				{
					.str_index = STR_2_SECOND,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_5_SECOND,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
				{
					.str_index = STR_8_SECOND,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},						
			}
			
		},		

};
menuManager_t DV_SETTING[DV_SETTING_NUM] = {
		//gsensor1
		{
			.icon_Index = ICON_GSENSOR1,
			.str_index =	STR_PARKING_MODE,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_HIGH,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_NORMAL,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_LOW,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				
			}
			
		},	

		//date/time
		{
			.icon_Index = ICON_DATE_TIME_SET,
			.str_index =	STR_DATE_TIME,
			.color = FONT_COLOR,
			.item_num = 3,	
			.item = 
			{
				{
					.str_index = 0,
					.xPos = 150,
					.yPos = 70,
				},
				{
					.str_index = 1,
					.xPos = 150,
					.yPos = 110,
				},	
				{
					.str_index = 2,
					.xPos = 150,
					.yPos = 150,
				},					
			}
			
		},
		//beep sound
		{
			.icon_Index = ICON_BEEP_SOUND,
			.str_index =	STR_BEEP_SOUND,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},
		//language
		{
			.icon_Index = ICON_LANGUAGE,
			.str_index =	STR_LANGUAGE,
			.color = FONT_COLOR,
			.item_num = 10,
			.item = 
			{
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},	
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},	
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_MULTI_LANGUAGE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},					
			}
			
		},	
		#ifdef SYSCONFIG_DISP0_TVOUT
		//TV mode
		{
			.icon_Index = ICON_TV_MODE,
			.str_index =	STR_TV_MODE,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_NTSC,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_PAL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},	
		#endif
		//frequency
		{
			.icon_Index = ICON_FREQUENCY,
			.str_index =	STR_FREQUENCY,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_50HZ,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_60HZ,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},
		//screen saver
		{
			.icon_Index = ICON_SCREEN_SAVER,
			.str_index =	STR_SCREEN_SAVER,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 240,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_3_MIN,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_5_MIN,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_10_MIN,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
					
			}
			
		},	
		//IR LED
		{
			.icon_Index = ICON_IR_LED,
			.str_index =	STR_IR_LED,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},		
		//format
		{
			.icon_Index = ICON_FORMAT,
			.str_index =	STR_FORMAT,
			.color = FONT_COLOR,
			.item_num = 1,			//show one in sec menu
			.item = 
			{
				{
					.str_index = STR_SD_CARD,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_FORMAT_SHOW1,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_CANCEL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
				{
					.str_index = STR_OK,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				{
					.str_index = STR_FORMAT_SHOW2,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP+30,
				},				
					
			}
			
		},	
		//defalit setting
		{
			.icon_Index = ICON_DEFAULT_SET,
			.str_index =	STR_DEFAULT_SETTING,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_DEFAULT_SETTING,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},	
				{
					.str_index = STR_DEFAULT_SETTING,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_CANCEL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
				{
					.str_index = STR_OK,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
					
			}
			
		},			
		//version
		{
			.icon_Index = ICON_VERSION,
			.str_index =	STR_VERSION,
			.color = FONT_COLOR,
			.item_num = 1,
			.item = 
			{
				{
					.str_index = STR_VERSION,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},		
			}
			
		},		
};


menuManager_t DV_MENU[DV_MENU_NUM] = {
		//resolution
		{
			.icon_Index = ICON_VIDEO_RESOLUTION,
			.str_index =	STR_RESOLUTION,
			.color = FONT_COLOR,
			.item_num = 5,
			.item = 
			{
				{
					.str_index = STR_1080FHD,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_720P60FPS,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_720P30FPS,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
				{
					.str_index = STR_WVGA,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				{
					.str_index = STR_VGA,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},				
			}
			
		},
		//loop recording
		{
			.icon_Index = ICON_LOOPRECORDING,
			.str_index =	STR_LOOPRECORDING,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 240,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},
				{
					.str_index = STR_2_MIN,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_3_MIN,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_5_MIN,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
					
			}
			
		},	
		//WDR
		{
			.icon_Index = ICON_WDR_M,
			.str_index =	STR_WDR,
			.color = FONT_COLOR,
			.item_num = 1,
			.item = 
			{
				{
					.str_index = STR_ON_ALLTIME,
					.xPos = 240,
					.yPos = UI_MENU_Y_POS+Y_STEP,
				},	
			}
			
		},	
		//Exposure
		{
			.icon_Index = ICON_VIDEO_EXPOSURE,
			.str_index =	STR_EXPOSURE,
			.color = FONT_COLOR,
			.item_num = 13,
			.item = 
			{
				//page0
				{
					.str_index = STR_P2_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_P5_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_P4_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_P1_0,
					.xPos = UI_MENU_Y_POS+4*Y_STEP,
					.yPos = 180,
				},
				//page1
				{
					.str_index = STR_P2_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_P1_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_P0_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_N1_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				//page2
				{
					.str_index = STR_N2_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_N1_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_N4_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_N5_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				//page3
				{
					.str_index = STR_N2_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},					
			}
			
		},	
		//motion detection
		{
			.icon_Index = ICON_MOTIONDETECT_M,
			.str_index =	STR_MOTIONDETECT,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},
		//record Audio
		{
			.icon_Index = ICON_RECORD_AUDIO,
			.str_index =	STR_RECORD_AUDIO,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},
		//date stamp
		{
			.icon_Index = ICON_VIDEO_DATE_STAMP,
			.str_index =	STR_DATE_STAMP,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},	
		//Gsensor
		{
			.icon_Index = ICON_GSENSOR,
			.str_index =	STR_GSENSOR,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_HIGH,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_NORMAL,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_LOW,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				
			}
			
		},	
		//LDW		
		{
			.icon_Index = ICON_LDW,
			.str_index =	STR_LDW,
			.color = FONT_COLOR,
#if LDW_FCW			
			.item_num = 8,
#else
			.item_num = 6,
#endif

			.item = 
			{
				{
					.str_index = STR_ON_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_CAR_TYPE,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_SENSITIVITY,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_AREA_CHOICE,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				{
					.str_index = STR_SRART_SPEED,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_LDW_SOUND,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},	
#if LDW_FCW				
				{//ldw FCW
					.str_index = STR_LDW_FCW_SOUND,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{//Ldw stop&&go
					.str_index = STR_LDW_STOPAndGO_SOUND,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
#endif				
				//6
				{
					.str_index = STR_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				// 8
				{
					.str_index = STR_SEDAN_CAR,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_SUV,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_TRUCK,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
					
				//11
				{
					.str_index = STR_HIGH,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_LOW,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},	
				{
					.str_index = STR_VERYHIGH,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},							
				//14
				{
					.str_index = STR_CHINA,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_TAIWAN,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},	
				// 16
				{
					.str_index = STR_LOWLOW_SPEED,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},					
				{
					.str_index = STR_LOW_SPEED,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_HIGH_SPEED,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				//19
				{
					.str_index = STR_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},		
#if LDW_FCW				
				//ldw FCW
				{
					.str_index = STR_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				//LDW Stop && go
				{
					.str_index = STR_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
#endif				
			}
			
		},
		//TIMELAPSE		

#ifdef TIME_LAPSE_ENABLE			
		
		{
			.icon_Index = ICON_TIMELAPSE,
			.str_index =	STR_TIME_LAPSE,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_TIME_LAPSE_100MS,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_TIME_LAPSE_200MS,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_TIME_LAPSE_500MS,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				
			}
			
		},
#endif		
};

menuManager_t DC_MENU[DC_MENU_NUM] = {
		//capture mode
		{
			.icon_Index = ICON_CAPTURE_M1,
			.str_index =	STR_CAPRUTE_MODE,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_SINGLE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_2S_TIMER,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_5S_TIMER,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},
				{
					.str_index = STR_10S_TIMER,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},				
			}
			
		},
		//resolution
		{
			.icon_Index = ICON_CAP_RESOLUTION,
			.str_index =	STR_RESOLUTION,
			.color = FONT_COLOR,
			.item_num = 8,
			.item = 
			{
				{
					.str_index = STR_12M,
					.xPos = 240,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_10M,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_8M,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_5M,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				{
					.str_index = STR_3M,
					.xPos = 240,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_2MHD,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_VGA,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_1_3M,
					.xPos = 230,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},				
					
			}
			
		},	
		//sequence
		{
			.icon_Index = ICON_SEQUENCE,
			.str_index =	STR_SEQUENCE,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},	
		//quality
		{
			.icon_Index = ICON_QUALITY,
			.str_index =	STR_QUALITY,
			.color = FONT_COLOR,
			.item_num = 3,
			.item = 
			{
				{
					.str_index = STR_FINE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_NORMAL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_ECONOMY,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},					
			}
			
		},	
		//SHARPNESS
		{
			.icon_Index = ICON_SHARPNESS,
			.str_index =	STR_SHARPNESS,
			.color = FONT_COLOR,
			.item_num = 3,
			.item = 
			{
				{
					.str_index = STR_STRONG,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_NORMAL,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_SOFT,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},				
			}
			
		},
		//write b(awb)
		{
			.icon_Index = ICON_AWB_M,
			.str_index =	STR_WRITE_BALANCE,
			.color = FONT_COLOR,
			.item_num = 5,
			.item = 
			{
				{
					.str_index = STR_AUTO,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_DAYLIGHT,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_CLOUDY,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_TUNGSTEN,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},	
				{
					.str_index = STR_FLUORESCENT,
					.xPos = 200,
					.yPos = 60,
				},				
			}
			
		},
		//color
		{
			.icon_Index = ICON_COLOR,
			.str_index =	STR_COLOR,
			.color = FONT_COLOR,
			.item_num = 3,
			.item = 
			{
				{
					.str_index = STR_COLOR,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_BLACK_WHITE,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_SEPIA,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},				
			}
			
		},	
		//iso
		{
			.icon_Index = ICON_ISO,
			.str_index =	STR_ISO,
			.color = FONT_COLOR,
			.item_num = 4,
			.item = 
			{
				{
					.str_index = STR_AUTO,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ISO100,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_ISO200,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_ISO400,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},	
			}
		},		
		//Exposure
		{
			.icon_Index = ICON_CAP_EXPOSURE,
			.str_index =	STR_EXPOSURE,
			.color = FONT_COLOR,
			.item_num = 13,
			.item = 
			{
				//page0
				{
					.str_index = STR_P2_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_P5_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_P4_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_P1_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				//page1
				{
					.str_index = STR_P2_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_P1_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_P0_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_N1_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				//page2
				{
					.str_index = STR_N2_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_N1_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},
				{
					.str_index = STR_N4_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},	
				{
					.str_index = STR_N5_3,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+4*Y_STEP,
				},
				//page3
				{
					.str_index = STR_N2_0,
					.xPos = 220,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},					
			}		
		},
		//anti shaking
		{
			.icon_Index = ICON_ANTI_SHAKING_M,
			.str_index =	STR_ANTI_SHAKING,
			.color = FONT_COLOR,
			.item_num = 2,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_ON,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},			
			}
			
		},	
		//date stamp
		{
			.icon_Index = ICON_CAP_DATE_STAMP,
			.str_index =	STR_DATE_TIME,
			.color = FONT_COLOR,
			.item_num = 3,
			.item = 
			{
				{
					.str_index = STR_OFF,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+1*Y_STEP,
				},
				{
					.str_index = STR_DATE_ONLY,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+2*Y_STEP,
				},	
				{
					.str_index = STR_DATE_TIME,
					.xPos = 200,
					.yPos = UI_MENU_Y_POS+3*Y_STEP,
				},					
			}
			
		},		
		
};


#define SETTING_DATE_TIME_DRAW_ALL 0xFF
char dt_str[] = "2000/00/00   00:00:00";
UINT8 setup_date_time[7];


typedef struct dispManager_s {
	UINT32 layer;
	int fdDisp;
	gp_disp_res_t resolution;

	int fdMem;
	chunk_block_t memBlock;
	gp_bitmap_t fb[2];		// framebuffer
	int fbIndex;
} dispManager_t;


UINT32
dispCreate(
	HANDLE *pHandle,
	UINT32 layer,
	UINT8 dis_type
)
{
	dispManager_t *pDisp;
	gp_disp_output_t dispOutput;
	gp_disp_pixelsize_t pixelSize;
	pDisp = (dispManager_t *)malloc(sizeof(dispManager_t));
	memset(pDisp, 0, sizeof(dispManager_t));
	pDisp->layer = layer;
	
	/* Opening the device dispDev */
	pDisp->fdDisp = open("/dev/disp0", O_RDWR);
	ioctl(pDisp->fdDisp, DISPIO_SET_INITIAL, 0);
	//ioctl(pDisp->fdDisp, DISPIO_GET_PANEL_RESOLUTION, &pDisp->resolution);
	ioctl(pDisp->fdDisp, DISPIO_GET_PANEL_PIXELSIZE, &pixelSize);
	if(pixelSize.width*3 == pixelSize.height * 4)
	{
		dv_set.LDW_v_offset_flag = 1;
	}
	else
	{
		dv_set.LDW_v_offset_flag = 0;
	}
	if(dis_type == SP_DISP_OUTPUT_HDMI)
	{	
		dispOutput.mode = HDMI_1920X1080I60;		
	}
	dispOutput.type = dis_type;
	ioctl(pDisp->fdDisp, DISPIO_SET_OUTPUT, &dispOutput);

	if(dis_type == SP_DISP_OUTPUT_TV)
	{
		pDisp->resolution.width = 640;
		pDisp->resolution.height = 426;
	}
	else if(dis_type == SP_DISP_OUTPUT_LCD)
	{
		pDisp->resolution.width = 320;
		pDisp->resolution.height = 240;		
	}
	else
	{
		pDisp->resolution.width = 1920;
		pDisp->resolution.height = 1080;		
	}		
	/* Allocate framebuffer */
	int bufferSize = pDisp->resolution.width * pDisp->resolution.height * 2;
	pDisp->fdMem = open("/dev/chunkmem", O_RDWR);
#ifdef DRAW_HD
	pDisp->memBlock.size = bufferSize * 2;	
#else
	if(dis_type == SP_DISP_OUTPUT_HDMI)
	pDisp->memBlock.size = bufferSize * 2+320*240*2;
	else
	pDisp->memBlock.size = bufferSize * 2;	
#endif
	ioctl(pDisp->fdMem, CHUNK_MEM_ALLOC, (unsigned long)&pDisp->memBlock);

	
	/* Setup bitmap double buffer */
	for (int i = 0; i < 2; i++) {
		gp_bitmap_t *fb = &pDisp->fb[i];
		fb->width = pDisp->resolution.width;
		fb->height = pDisp->resolution.height;
		fb->bpl = fb->width * 2;
		if (pDisp->layer == DISP_LAYER_PRIMARY) {
			//fb->type = SP_BITMAP_4Y4Cb4Y4Cr;
			fb->type = SP_BITMAP_RGB565;	
		}
		else {
			fb->type = SP_BITMAP_RGB565;	
		}
		fb->pData = pDisp->memBlock.addr + i * bufferSize;
	}
	if(pDisp->layer != DISP_LAYER_PRIMARY) {
	UINT16* addr1 = pDisp->memBlock.addr;
		for (int y = 0; y <pDisp->resolution.height*2; y++) {		
			for (int x = 0; x < pDisp->resolution.width; x++) {			
				*addr1++ = TRANSPARENT_COLOR;		
			}	
		}		
	}
	else {
		memset(pDisp->memBlock.addr, 0, bufferSize);
	}

	
	if (pDisp->layer == DISP_LAYER_PRIMARY) {
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_BITMAP, &pDisp->fb[pDisp->fbIndex]);
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		printf("init index %d\n", pDisp->fbIndex);
		//pDisp->fbIndex = pDisp->fbIndex ^ 1;
	}
	else {
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_BITMAP(0), &pDisp->fb[pDisp->fbIndex]);

		/* Set osd layer 0 scale */
		gp_disp_scale_t osdScale;
		osdScale.x = 0;
		osdScale.y = 0;
		if(dis_type == SP_DISP_OUTPUT_HDMI)
		{	
			dv_set.display_mode = SP_DISP_OUTPUT_HDMI;
			osdScale.width = 1920;
#ifdef dispscl_workaround
			osdScale.height = 240;
#else
			osdScale.height = 1080;
#endif
		}
		else if(dis_type == SP_DISP_OUTPUT_LCD)
		{
			dv_set.display_mode = SP_DISP_OUTPUT_LCD;
			osdScale.width = 320;
			osdScale.height = 240;					
		}
		else if(dis_type == SP_DISP_OUTPUT_TV)
		{
			dv_set.display_mode = SP_DISP_OUTPUT_TV;
			osdScale.x = 40;
			osdScale.y = 27;
			osdScale.width = 640;
			osdScale.height = 426;				
		}
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_SCALEINFO(0), &osdScale);

		/* Set osd alpha & color key */
		gp_disp_osdalpha_t osdAlphs;
		//osdAlphs.consta = SP_DISP_ALPHA_CONSTANT;
		//osdAlphs.ppamd = SP_DISP_ALPHA_PERPIXEL_ONLY;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_KEY(0), TRANSPARENT_COLOR);
		osdAlphs.consta = SP_DISP_ALPHA_PERPIXEL;
		osdAlphs.ppamd = SP_DISP_ALPHA_COLORKEY_ONLY;
		osdAlphs.alphasel = 1;
		osdAlphs.alpha = 100;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ALPHA(0), &osdAlphs);
		
		/* Enable osd layer 0 */
		//ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
#ifdef dispscl_workaround
		if(dis_type == SP_DISP_OUTPUT_HDMI)
		{
			ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
			ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);
			osdScale.width = 1920;
			osdScale.height = 1080;
			ioctl(pDisp->fdDisp, DISPIO_SET_OSD_SCALEINFO(0), &osdScale);
			ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
			ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);
		}
#endif		
	}

	*pHandle = (HANDLE)pDisp;
	
	
	/* Enable backlight */
	ioctl(pDisp->fdDisp, DISPIO_SET_BACKLIGHT, 1);

	return SP_OK;
}

UINT32 disp_OSD0_Set_Alpha(HANDLE handle,UINT32 value)
{
		dispManager_t *pDisp = (dispManager_t *)handle;
		gp_disp_osdalpha_t osdAlphs;
		//osdAlphs.consta = SP_DISP_ALPHA_CONSTANT;
		//osdAlphs.ppamd = SP_DISP_ALPHA_PERPIXEL_ONLY;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_KEY(0), TRANSPARENT_COLOR);
		osdAlphs.consta = SP_DISP_ALPHA_PERPIXEL;
		osdAlphs.ppamd = SP_DISP_ALPHA_COLORKEY_ONLY;
		osdAlphs.alphasel = 1;
		osdAlphs.alpha = value;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ALPHA(0), &osdAlphs);
}

UINT32 dispLayerEnable(HANDLE handle)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		gp_disp_scale_t osdScale;
		osdScale.x = 0;
		osdScale.y = 0;
		osdScale.width = pDisp->resolution.width;
		osdScale.height = pDisp->resolution.height;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_SCALEINFO(0), &osdScale);

		/* Set osd alpha & color key */
		gp_disp_osdalpha_t osdAlphs;
		//osdAlphs.consta = SP_DISP_ALPHA_CONSTANT;
		//osdAlphs.ppamd = SP_DISP_ALPHA_PERPIXEL_ONLY;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_KEY(0), TRANSPARENT_COLOR);
		osdAlphs.consta = SP_DISP_ALPHA_PERPIXEL;
		osdAlphs.ppamd = SP_DISP_ALPHA_COLORKEY_ONLY;
		osdAlphs.alpha = 80;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ALPHA(0), &osdAlphs);
		
		/* Enable osd layer 0 */
		//ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
	ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
	ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(1), 0);
	return 0;
}

UINT32
dispDestroy(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;

	close(pDisp->fdDisp);
	ioctl(pDisp->fdMem, CHUNK_MEM_FREE, &pDisp->memBlock);
	close(pDisp->fdMem);	
	free(pDisp);

	return SP_OK;
}

void
dispGetResolution(
	HANDLE handle,
	gp_size_t *resolution
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;

	resolution->width = pDisp->resolution.width;
	resolution->height = pDisp->resolution.height;
	#ifndef DRAW_HD
	if(dv_set.display_mode== SP_DISP_OUTPUT_HDMI)	
	{
		resolution->width = 320;
		resolution->height = 240;
	}
	#endif
}
void dispCleanFramebuffer(HANDLE handle)
{
	
	dispManager_t *pDisp = (dispManager_t *)handle;
	int fbIndex;

	fbIndex = pDisp->fbIndex ^ 1;
	printf("clean index %d\n", fbIndex);
	memset((UINT8*) pDisp->fb[fbIndex].pData, 0, pDisp->fb[fbIndex].width*pDisp->fb[fbIndex].height*2);
}

UINT8*
dispGetFramebuffer(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	int fbIndex;

	fbIndex = pDisp->fbIndex ^ 1;
	#ifndef DRAW_HD
	if(dv_set.display_mode== SP_DISP_OUTPUT_HDMI)	
	{
		return pDisp->memBlock.addr + 1920*1080*2*2;
	}
	else
	#endif
	return (UINT8*) pDisp->fb[fbIndex].pData;
}

void dispUpDataPrimary(
	HANDLE handle,
	gp_bitmap_t *fb
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;

	if (pDisp->layer == DISP_LAYER_PRIMARY) {
		printf("pri show: w %d h %d x %d y %d vw %d vh %d format %d\n", fb->width, fb->height, fb->validRect.x, fb->validRect.y, fb->validRect.width, fb->validRect.height, fb->type);
        ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_BITMAP, fb);
		ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
		ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);
	}
}
void dispDisablePrimary(HANDLE handle)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
    ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 0);	
}


void dispEnablePrimary(HANDLE handle)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
       ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
	ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
	ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);
	
}
static int 
scale1_process(
	unsigned int input_addr,
	unsigned short input_w,
	unsigned short input_h,
	unsigned int output_addr,
	unsigned short output_w,
	unsigned short output_h,
	int zoom
)
{
	int ret = 0;
	int isOutput4x3;
	int crop_w = input_w;
	int crop_h = input_h;
	int clip_y = 0;
	int handle;
	
	scale_content_t sct;
	
	handle = open("/dev/scalar", O_RDONLY);
	if (!handle)
	{
		printf("cannot open /dev/scalar \n");
		return -1;
	}
	isOutput4x3 = (output_w*3 == output_h*4) ? 1:0;
	
	if (isOutput4x3)
		crop_w = 4*input_h/3;

	memset(&sct, 0, sizeof(sct));
	sct.src_img.pData = (void *)input_addr;
	sct.src_img.width = input_w;
	sct.src_img.height = input_h;
	sct.src_img.bpl = input_w*2;
	sct.src_img.type = SP_BITMAP_RGB565;
	sct.dst_img.pData = (void *)output_addr;
	sct.dst_img.width = output_w;
	sct.dst_img.height = output_h;
	sct.dst_img.bpl = output_w*2;
	sct.dst_img.type = sct.src_img.type;
	
	if (zoom > 100)
	{
		crop_w = ((100*crop_w/zoom) >> 4) << 4;
		crop_h = crop_w * output_h / output_w;
		clip_y = (input_h - crop_h)/2;
	}
	if (output_w > 2048)
	{
		sct.clip_rgn.x = (input_w - crop_w)/2;
		sct.clip_rgn.y = clip_y;
		sct.clip_rgn.width = crop_w/2;
		sct.clip_rgn.height = crop_h;
		
		sct.scale_rgn.x = 0;
		sct.scale_rgn.y = 0;
		sct.scale_rgn.width = output_w/2;
		sct.scale_rgn.height = output_h;
		
		if (ioctl(handle/*scaleInfo.devHandle*/, SCALE_IOCTL_TRIGGER, &sct) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		sct.clip_rgn.x += crop_w/2;
		sct.clip_rgn.y = clip_y;
		sct.clip_rgn.width = crop_w/2;
		sct.clip_rgn.height = crop_h;
		
		sct.scale_rgn.x += output_w/2;
		sct.scale_rgn.y = 0;
		sct.scale_rgn.width = output_w/2;
		sct.scale_rgn.height = output_h;
		
		if (ioctl(handle/*scaleInfo.devHandle*/, SCALE_IOCTL_TRIGGER, &sct) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		return ret;
	}
	
	sct.clip_rgn.x = (input_w - crop_w)/2;
	sct.clip_rgn.y = clip_y;
	sct.clip_rgn.width = crop_w;
	sct.clip_rgn.height = crop_h;
		
	sct.scale_rgn.x = 0;
	sct.scale_rgn.y = 0;
	sct.scale_rgn.width = output_w;
	sct.scale_rgn.height = output_h;
	
	if (ioctl(handle/*scaleInfo.devHandle*/, SCALE_IOCTL_TRIGGER, &sct) < 0) {
		printf("scale_start fail\n");
		ret = -1;
	}
	
	close(handle);
	return ret;
}

void
dispFlip(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
       int ret=0;
	pDisp->fbIndex = pDisp->fbIndex ^ 1;
	if (pDisp->layer == DISP_LAYER_PRIMARY) {
		//ioctl(pDisp->fdDisp, DISPIO_CHANGE_PRI_BITMAP_BUF, &pDisp->fb[pDisp->fbIndex]);
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_BITMAP, &pDisp->fb[pDisp->fbIndex]);
        ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
		ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);
	}
	else {
		//scaler up
		#ifndef DRAW_HD
		if(dv_set.display_mode== SP_DISP_OUTPUT_HDMI)
		{
			ret = scale1_process(pDisp->memBlock.addr+1920*1080*2*2,320,240,pDisp->fb[pDisp->fbIndex].pData,1920,1080,4);
		}
		#endif
		if(ret == 0)
		{
			ioctl(pDisp->fdDisp, DISPIO_SET_OSD_BITMAP(0), &pDisp->fb[pDisp->fbIndex]);
			ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
			ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
			ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);		
		}
	}
}
int gp_block_cpy(void *destbuf, int x_dest, int y_dest,int widthdest, int heightdest, void *srcbuf,  int widthsrc, int heightsrc,UINT16 trancolor,UINT16 background_color)
{
	int	i,j;
	short *src;
	short *dest;
	char *src_char;
	char* dest_char;
	char temp;
	if(((x_dest + widthsrc) > widthdest)||((y_dest + heightsrc) > heightdest))
	{
	    __err("Icon > lcd Size widthdest:%d (iconX[%d]+iconW[%d]=%d)  heightdest:%d (iconY[%d]+iconH[%d]=%d)\n",\
	            widthdest,x_dest,widthsrc,(x_dest + widthsrc),heightdest,y_dest,heightsrc,(y_dest + heightsrc));
		return -1;
	}

	if(x_dest<0||y_dest<0||widthdest<=0||heightdest<=0||widthsrc<=0||heightsrc<=0)
	{
	    __err("Icon or lcd Size <= 0\n");
		return -1;
    }
#if BUFFER_COLOR_TYPE == RGB565_DRAW				
	__inf("widthsrc =%d,heightsrc=%d  ",widthsrc,heightsrc);
	dest = (short *)destbuf;
	src = (short *)srcbuf;
	dest = dest + (y_dest*widthdest + x_dest); 
	__inf("dest =0x%x,src=0x%x\n",dest,src);
	for(i=0;i<heightsrc*HDMIxNum;i++)
	{	
		src = (short *)srcbuf + (i/HDMIxNum)*widthsrc;
		//printf("i=%d,j=%d,dest =0x%x,src=0x%x\n",i,j,dest,src);
		for(j=0;j<widthsrc*HDMIxNum;j++)
		{
			if((*src&0xffff) != (trancolor&0xffff))
			{
				*dest++ = *src;
			}
			else if(background_color == trancolor)
			{
				*dest++ = background_color;
				*src;
			}
			else
			{
				*dest++ ;
				*src;
			}
			if(dv_set.display_mode== SP_DISP_OUTPUT_HDMI)
			{
				if((j%HDMIxNum==0)&&(j!=0))
					src++;
			}
			else
			{
				src++;
			}
		}
		dest = dest + widthdest - widthsrc*HDMIxNum;
	}
#elif BUFFER_COLOR_TYPE == COLOR256_DRAW
	dest_char = (char *)destbuf;
	src_char = (char *)srcbuf;
	dest_char = dest_char + (y_dest*widthdest + x_dest); 
	//printf("dest =0x%x,src=0x%x\n",dest,src);
	for(i=0;i<heightsrc;i++)
	{	
		for(j=0;j<widthsrc;j++)
		{
			//printf("i=%d,j=%d,dest =0x%x,src=0x%x\n",i,j,dest,src);
			if((*src_char&0xff) != (trancolor&0xff))
			{
				*dest_char++ = *src_char++;
			}
			else if((background_color&0xff) == (trancolor&0xff))
			{
				*dest_char++ = (background_color&0xff);
				*src_char++;
			}
			else
			{
				*dest_char++ ;
				*src_char++;
			}
		}
		dest_char = dest_char + widthdest - widthsrc;
	}
#elif BUFFER_COLOR_TYPE == COLOR16_DRAW
 //限制:请保证icon坐标及icon width为偶数
 	dest_char = (char *)destbuf;
	src_char = (char *)srcbuf;
	dest_char = dest_char + (y_dest*widthdest + x_dest)/2; 
	//printf("dest =0x%x,src=0x%x\n",dest,src);
	for(i=0;i<heightsrc;i++)
	{	
		for(j=0;j<widthsrc/2;j++)
		{
			temp = *src_char
			if(((*src_char)>>4&0xf) != (trancolor&0xf))				//high 4 bit
			{
				temp = (*src_char)&0xf0;
			}
			else if((background_color&0xf) == (trancolor&0xf))
			{
				temp = (background_color<<4)&0xf0;
			}
			else
			{
				temp = (*dest_char)&0xf0;
			}
			
			if(((*src_char)&0xf) != (trancolor&0xf))					//low 4 bit
			{
				*dest_char = ((*src_char)&0xf) |temp;
			}			
			else if((background_color&0xf) == (trancolor&0xf))
			{
				*dest_char = ((background_color)&0xf)|temp;
			}
			else
			{
				*dest_char = ((*dest_char)&0xf) | temp ;
			}
			*dest_char++ ;
			*src_char++;
		}
		dest_char = dest_char + (widthdest - widthsrc)/2;
	}
#else
	printf("color  is not support\n");
#endif
	return 1;
}		

int fill_rectangle(void *destbuf, int x_dest, int y_dest,int widthdest, int heightdest,int widthrec, int heightrec,UINT16 color)
{
	int	i,j;
	short *src;
	short *dest;
	//printf("fill_rectangle,x_dest=%d,y_dest=%d,widthdest=%d,heightdest=%d,widthrec=%d,heightrec=%d\n",x_dest,y_dest,widthdest,heightdest,widthrec,heightrec);
	
	if(((x_dest + widthrec) > widthdest)||((y_dest + heightrec) > heightdest))
		return -1;
	if(x_dest<0||y_dest<0||widthdest<=0||heightdest<=0||widthrec<=0||heightrec<=0)
		return -1;
	dest = (short *)destbuf;
	dest = dest + (y_dest*widthdest + x_dest); 
	for(i=0;i<heightrec;i++)
	{	
		for(j=0;j<widthrec;j++)
		{
			*dest++ = color;
		}
		dest = dest + widthdest - widthrec;
	}
	return 1;	
}

int draw_rectangle(void *destbuf, int x_dest, int y_dest,int widthdest, int heightdest,int widthrec, int heightrec,UINT16 color,UINT16 background_color)
{
	int	i,j;
	short *src;
	short *dest;
	
	if(((x_dest + widthrec) > widthdest)||((y_dest + heightrec) > heightdest))
		return -1;
	if(x_dest<0||y_dest<0||widthdest<=0||heightdest<=0||widthrec<=0||heightrec<=0)
		return -1;
	dest = (short *)destbuf;
	dest = dest + (y_dest*widthdest + x_dest); 
	for(i=0;i<heightrec;i++)
	{	

		if((i==0)||(i==(heightrec-1)))
		{
			for(j=0;j<widthrec;j++)
			{
				*dest++ = color;
			}
		}
		else
		{
			for(j=0;j<widthrec;j++)
			{
				if((j==0)||(j==(widthrec-1)))
					*dest++ = color;
				else
				*dest++ = background_color;
			}			
		}
		dest = dest + widthdest - widthrec;
	}
	return 1;	
	
}
int draw_rect(void *destbuf, gp_size_t *size,gp_rect_t *prect, UINT8 lineWidth, UINT16 color,UINT16 background_color)
{
	int	i,j;
	short *src;
	short *dest;
	short *pTop, *pBot;

	if(prect->x+prect->width > size->width) {
		//prect->width = size->width-prect->x;
		return -1;
	}
	if(prect->y+prect->height> size->height) {
		//prect->height = size->height-prect->y;
		return -1;
	}

	if(lineWidth<=0) lineWidth = 1;
	
	dest = (short *)destbuf;
	pTop = dest + (prect->y * size->width + prect->x); 
	pBot = dest + (prect->y+prect->height-lineWidth)*size->width + prect->x;
	for(i=0; i<lineWidth; i++) {
		for(j=0;j<prect->width;j++) {
			*pTop++ = color;
			*pBot++ = color;
		}
		pTop += size->width-prect->width;
		pBot += size->width-prect->width;
	}
	
	pTop = dest + (prect->y * size->width + prect->x); 
	pBot = dest + prect->y*size->width + prect->x + prect->width;
	for(i=0; i<prect->height; i++) {
		for(j=0;j<lineWidth;j++) {
			*pTop++ = color;
			*pBot++ = color;
		}
		pTop += size->width-lineWidth;
		pBot += size->width-lineWidth;
	}
	return 1;	
	
}

void drawLine(unsigned short* drawImg, int startx, int starty, int endx, int endy, int colorR, int colorG, int colorB)
{
	int t, distance;
	int xerr=0, yerr=0, delta_x, delta_y;
	int incx, incy;

	unsigned short colorRGB565;

	colorRGB565 = 0;

	colorRGB565 += colorR;
	colorRGB565 = colorRGB565<<6;
	colorRGB565 += colorG;
	colorRGB565 = colorRGB565<<5;
	colorRGB565 += colorB;


	/* compute the distances in both directions */
	delta_x=endx-startx;
	delta_y=endy-starty;

	/* Compute the direction of the increment,
	an increment of 0 means either a horizontal or vertical
	line.
	*/
	if(delta_x>0) incx=1;
	else if(delta_x==0) incx=0;
	else incx=-1;

	if(delta_y>0) incy=1;
	else if(delta_y==0) incy=0;
	else incy=-1;

	/* determine which distance is greater */
	if(delta_x < 0)
		delta_x = -delta_x;
	if(delta_y < 0)
		delta_y = -delta_y;	
	//delta_x=ABS(delta_x);
	//delta_y=ABS(delta_y);
	if(delta_x>delta_y) distance=delta_x;
	else distance=delta_y;

	/* draw the line */
	for(t=0; t<=distance+1; t++)
	{
		if(dv_set.display_mode == SP_DISP_OUTPUT_TV)
		{
			if(startx > 0 && startx < 640 && starty > 0 && starty < 426)
				drawImg[startx + starty*640] = colorRGB565;
		}
		else
		{
			if(startx > 0 && startx < 320 && starty > 0 && starty < 240)
				drawImg[startx + starty*320] = colorRGB565;			
		}
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance) {
			xerr-=distance;
			startx+=incx;
		}
		if(yerr>distance) {
			yerr-=distance;
			starty+=incy;
		}
	}
}
void drawLine_RGB565(unsigned short* drawImg, int startx, int starty, int endx, int endy, UINT16 colorRGB565)
{
	int t, distance;
	int xerr=0, yerr=0, delta_x, delta_y;
	int incx, incy;


	/* compute the distances in both directions */
	delta_x=endx-startx;
	delta_y=endy-starty;

	/* Compute the direction of the increment,
	an increment of 0 means either a horizontal or vertical
	line.
	*/
	if(delta_x>0) incx=1;
	else if(delta_x==0) incx=0;
	else incx=-1;

	if(delta_y>0) incy=1;
	else if(delta_y==0) incy=0;
	else incy=-1;

	/* determine which distance is greater */
	if(delta_x < 0)
		delta_x = -delta_x;
	if(delta_y < 0)
		delta_y = -delta_y;	
	//delta_x=ABS(delta_x);
	//delta_y=ABS(delta_y);
	if(delta_x>delta_y) distance=delta_x;
	else distance=delta_y;

	/* draw the line */
	for(t=0; t<=distance+1; t++)
	{
		if(dv_set.display_mode == SP_DISP_OUTPUT_TV)
		{
			if(startx > 0 && startx < 640 && starty > 0 && starty < 426)
				drawImg[startx + starty*640] = colorRGB565;
		}
		else
		{
			if(startx > 0 && startx < 320 && starty > 0 && starty < 240)
				drawImg[startx + starty*320] = colorRGB565;			
		}
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance) {
			xerr-=distance;
			startx+=incx;
		}
		if(yerr>distance) {
			yerr-=distance;
			starty+=incy;
		}
	}
}

void get_LDW_dynamicOrFixedLine(int fixed_line,int widthdest, int heightdest)
{
	int x_offset;//两边预留的不刷新的部分
	int y_fixed;//固定检测线的位置
	int y_offset;//上下留黑边的宽度 

		
	if(fixed_line&&((ldw_get_displine.LLcheckFlg && ldw_get_displine.RLcheckFlg && ldw_get_displine.LTP_X<=ldw_get_displine.RTP_X+60) || ldw_get_displine.LLAlarmFlg || ldw_get_displine.RLAlarmFlg))
	{//not fixed Line    &&   (LR checkLine <60pixcel[base 1920*1080] ,||  AlalrmLine  disp)				
		if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
		{	
			if(dv_set.LDW_v_offset_flag)
			{
				ldw_displine.LT_alarmP_X=(ldw_get_displine.LT_alarmP_X)*X_DIV_LCD_320240_4_3;
				ldw_displine.LT_alarmP_Y=(ldw_get_displine.LT_alarmP_Y)*Y_DIV_LCD_320240_4_3; 
				ldw_displine.LB_alarmP_X=	(ldw_get_displine.LB_alarmP_X)*X_DIV_LCD_320240_4_3; 
				ldw_displine.LB_alarmP_Y=	(ldw_get_displine.LB_alarmP_Y)*Y_DIV_LCD_320240_4_3; 
				ldw_displine.LTP_X = (ldw_get_displine.LTP_X)*X_DIV_LCD_320240_4_3;
				ldw_displine.LTP_Y= (ldw_get_displine.LTP_Y )*Y_DIV_LCD_320240_4_3;
				ldw_displine.LBP_X= (ldw_get_displine.LBP_X)*X_DIV_LCD_320240_4_3;
				ldw_displine.LBP_Y= (ldw_get_displine.LBP_Y)*Y_DIV_LCD_320240_4_3; 

				ldw_displine.RT_alarmP_X=	(ldw_get_displine.RT_alarmP_X)*X_DIV_LCD_320240_4_3;
				ldw_displine.RT_alarmP_Y=	(ldw_get_displine.RT_alarmP_Y)*Y_DIV_LCD_320240_4_3; 
				ldw_displine.RB_alarmP_X=	(ldw_get_displine.RB_alarmP_X)*X_DIV_LCD_320240_4_3; 
				ldw_displine.RB_alarmP_Y=	(ldw_get_displine.RB_alarmP_Y)*Y_DIV_LCD_320240_4_3; 
				ldw_displine.RTP_X= (ldw_get_displine.RTP_X)*X_DIV_LCD_320240_4_3 ;
				ldw_displine.RTP_Y= (ldw_get_displine.RTP_Y )*Y_DIV_LCD_320240_4_3;
				ldw_displine.RBP_X= (ldw_get_displine.RBP_X)*X_DIV_LCD_320240_4_3; 
				ldw_displine.RBP_Y= (ldw_get_displine.RBP_Y )*Y_DIV_LCD_320240_4_3; 
			}else
			{
				ldw_displine.LT_alarmP_X=(ldw_get_displine.LT_alarmP_X)*X_DIV_LCD_320240_16_9;
				ldw_displine.LT_alarmP_Y=(ldw_get_displine.LT_alarmP_Y)*Y_DIV_LCD_320240_16_9; 
				ldw_displine.LB_alarmP_X=	(ldw_get_displine.LB_alarmP_X)*X_DIV_LCD_320240_16_9; 
				ldw_displine.LB_alarmP_Y=	(ldw_get_displine.LB_alarmP_Y)*Y_DIV_LCD_320240_16_9; 
				ldw_displine.LTP_X = (ldw_get_displine.LTP_X)*X_DIV_LCD_320240_16_9;
				ldw_displine.LTP_Y= (ldw_get_displine.LTP_Y )*Y_DIV_LCD_320240_16_9;
				ldw_displine.LBP_X= (ldw_get_displine.LBP_X)*X_DIV_LCD_320240_16_9;
				ldw_displine.LBP_Y= (ldw_get_displine.LBP_Y)*Y_DIV_LCD_320240_16_9; 

				ldw_displine.RT_alarmP_X=	(ldw_get_displine.RT_alarmP_X)*X_DIV_LCD_320240_16_9;
				ldw_displine.RT_alarmP_Y=	(ldw_get_displine.RT_alarmP_Y)*Y_DIV_LCD_320240_16_9; 
				ldw_displine.RB_alarmP_X=	(ldw_get_displine.RB_alarmP_X)*X_DIV_LCD_320240_16_9; 
				ldw_displine.RB_alarmP_Y=	(ldw_get_displine.RB_alarmP_Y)*Y_DIV_LCD_320240_16_9; 
				ldw_displine.RTP_X= (ldw_get_displine.RTP_X)*X_DIV_LCD_320240_16_9 ;
				ldw_displine.RTP_Y= (ldw_get_displine.RTP_Y )*Y_DIV_LCD_320240_16_9;
				ldw_displine.RBP_X= (ldw_get_displine.RBP_X)*X_DIV_LCD_320240_16_9; 
				ldw_displine.RBP_Y= (ldw_get_displine.RBP_Y )*Y_DIV_LCD_320240_16_9;		
			}
		}
		else if(dv_set.display_mode == SP_DISP_OUTPUT_TV)
		{
			ldw_displine.LT_alarmP_X=(ldw_get_displine.LT_alarmP_X)*X_DIV_TV_640426;
			ldw_displine.LT_alarmP_Y=(ldw_get_displine.LT_alarmP_Y)*Y_DIV_TV_640426; 
			ldw_displine.LB_alarmP_X=	(ldw_get_displine.LB_alarmP_X)*X_DIV_TV_640426; 
			ldw_displine.LB_alarmP_Y=	(ldw_get_displine.LB_alarmP_Y)*Y_DIV_TV_640426; 
			ldw_displine.LTP_X = (ldw_get_displine.LTP_X)*X_DIV_TV_640426;
			ldw_displine.LTP_Y= (ldw_get_displine.LTP_Y )*Y_DIV_TV_640426;
			ldw_displine.LBP_X= (ldw_get_displine.LBP_X)*X_DIV_TV_640426;
			ldw_displine.LBP_Y= (ldw_get_displine.LBP_Y)*Y_DIV_TV_640426; 

			ldw_displine.RT_alarmP_X=	(ldw_get_displine.RT_alarmP_X)*X_DIV_TV_640426;
			ldw_displine.RT_alarmP_Y=	(ldw_get_displine.RT_alarmP_Y)*Y_DIV_TV_640426; 
			ldw_displine.RB_alarmP_X=	(ldw_get_displine.RB_alarmP_X)*X_DIV_TV_640426; 
			ldw_displine.RB_alarmP_Y=	(ldw_get_displine.RB_alarmP_Y)*Y_DIV_TV_640426; 
			ldw_displine.RTP_X= (ldw_get_displine.RTP_X)*X_DIV_TV_640426 ;
			ldw_displine.RTP_Y= (ldw_get_displine.RTP_Y )*Y_DIV_TV_640426;
			ldw_displine.RBP_X= (ldw_get_displine.RBP_X)*X_DIV_TV_640426; 
			ldw_displine.RBP_Y= (ldw_get_displine.RBP_Y )*Y_DIV_TV_640426;	
		}
		ldw_displine.LLcheckFlg = ldw_get_displine.LLcheckFlg;
		ldw_displine.RLcheckFlg = ldw_get_displine.RLcheckFlg;
		ldw_displine.LLAlarmFlg = ldw_get_displine.LLAlarmFlg;
		ldw_displine.RLAlarmFlg = ldw_get_displine.RLAlarmFlg;
	}
	else
	{		
		if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
		{
			if(dv_set.LDW_v_offset_flag)//4:3
			{
				x_offset = 46;
				y_fixed = 140;//660*180[height]/1080+30;	
				y_offset = 30;
			}	
			else//16:9
			{
				x_offset = 46;
				y_fixed = 146;//660*240[height]/1080;	
				y_offset = 0;	
			}			
		}
		else if(dv_set.display_mode == SP_DISP_OUTPUT_TV)
		{	
			x_offset = 50;
			y_fixed =  258;//660*405[height]/1080+11;			
			y_offset = 11;	
		}
		ldw_displine.LBP_X = x_offset;
		ldw_displine.LTP_X = widthdest/2-1;
		ldw_displine.RBP_X = widthdest-x_offset;
		ldw_displine.RTP_X = widthdest/2+1;
		ldw_displine.LTP_Y = ldw_displine.RTP_Y = y_fixed;//660*405[height]/1080;					
		ldw_displine.LBP_Y = ldw_displine.RBP_Y = heightdest-y_offset;
		ldw_displine.LLcheckFlg = 1;
		ldw_displine.RLcheckFlg = 1;
		ldw_displine.LLAlarmFlg = 0;
		ldw_displine.RLAlarmFlg = 0;		 
	}
}

void drawLDWLine(unsigned short* drawImg, int startx, int starty, int endx, int endy,int width, UINT16 color)
{//min=3,
	if(width<=3)
	{
		width=3;
	}else
	{
		width=5;
	}

	if(width==3)
	{
		drawLine_RGB565(drawImg, startx, starty, endx, endy, color);
		drawLine_RGB565(drawImg, startx-1, starty, endx-1, endy, color);
		drawLine_RGB565(drawImg, startx+1, starty, endx+1, endy, color);
		drawLine_RGB565(drawImg, startx, starty-1, endx, endy-1, color);
		drawLine_RGB565(drawImg, startx, starty+1, endx, endy+1, color);		
	}else if(width==5)
	{
		drawLine_RGB565(drawImg, startx, starty, endx, endy, color);
		drawLine_RGB565(drawImg, startx-1, starty, endx-1, endy, color);
		drawLine_RGB565(drawImg, startx-2, starty, endx-2, endy, color);
		drawLine_RGB565(drawImg, startx+1, starty, endx+1, endy, color);
		drawLine_RGB565(drawImg, startx+2, starty, endx+2, endy, color);
		drawLine_RGB565(drawImg, startx, starty-1, endx, endy-1, color);
		drawLine_RGB565(drawImg, startx, starty-2, endx, endy-2, color);
		drawLine_RGB565(drawImg, startx, starty+1, endx, endy+1, color);	
		drawLine_RGB565(drawImg, startx, starty+2, endx, endy+2, color);	

		drawLine_RGB565(drawImg, startx+1, starty+1, endx+1, endy+1, color);		
		drawLine_RGB565(drawImg, startx+1, starty-1, endx+1, endy-1, color);	
	}	
}




UINT8 *open_icon_bin(UINT8 index)
{
	UINT16 i;
	SINT32 fd;
	UINT32 file_size;
	UINT8 * addr;

	if(index < ICON_MAX)
	{
			if(Icon_Manager[index].addr != NULL)
			{
				return (UINT8 *)Icon_Manager[index].addr;
			}
			strcpy(file_path, Icon_path);
			strcat(file_path, Icon_Manager[index].icon_name);	
			__msg("open file %s\n",file_path);
			fd = open(file_path,O_RDONLY);
			if(fd >=0)
			{
				file_size = lseek(fd,0,SEEK_END);
				lseek(fd,0,SEEK_SET);
				addr = (UINT8 *)malloc(file_size);	
				read(fd,addr,file_size);
				close(fd);	
				Icon_Manager[index].addr = (UINT32 *)addr;
				return addr;
			}
	}	
	return NULL;
}
void close_icon_bin(UINT8 index)
{
	if(index < ICON_MAX)
	{
		if(Icon_Manager[index].addr != NULL)
		{
			free(Icon_Manager[index].addr);
			Icon_Manager[index].addr = NULL;
		}
	}
}

SINT32 gp_Icon_draw(void *destbuf,int x_dest, int y_dest,int widthdest, int heightdest,UINT8 icon_index,UINT16 background_color)
{
	UINT8* addr=NULL;
	if(NULL==Icon_Manager[icon_index].icon_Index)
	{
	    __inf("Icon Index Is Null\n");
	    return -1;
	}
	addr = open_icon_bin(icon_index);

	if(addr != NULL)
	{
		//printf("ICON Dist:0x%X index = %d Name:%s X=%d,Y=%d\n",destbuf,icon_index,Icon_Manager[icon_index].icon_name,x_dest,y_dest);
		gp_block_cpy(destbuf,x_dest,y_dest,widthdest, heightdest,addr,Icon_Manager[icon_index].icon_width,Icon_Manager[icon_index].icon_height,Icon_Manager[icon_index].tran_color,background_color);
		return 0;
	}
	return -1;
}

UINT8 CalendarCalculateDays(UINT8 month, UINT8 year)
{
	UINT8 DaysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31}; 
	 
	year += 2000;
	if ((year&0x3) || (!(year%100) && (year%400))) {
		return DaysInMonth[month - 1];
	} else {
		if (month == 2) {
			return 29;
		} else {
			return DaysInMonth[month - 1];
		}
	}
}
void ap_setting_date_time_string_process(UINT8 dt_tag)
{
	//UINT8 i, data_tmp, days, tag[] = {2, 3, 7, 8, 12, 13, 17, 18, 22, 23};
	UINT8 i, data_tmp, days, tag[] = {2, 3, 5, 6, 8, 9, 		13, 14, 16, 17,19,20};
	
	days = CalendarCalculateDays(setup_date_time[1], setup_date_time[0]);
	if (setup_date_time[2] > days) {
		setup_date_time[2] = days;
	}	
	if (dt_tag == SETTING_DATE_TIME_DRAW_ALL) {
		for (i=0 ; i<6 ; i++) {
			data_tmp = setup_date_time[i]/10;
			*(dt_str + tag[(i<<1)]) = data_tmp + 0x30;
			data_tmp = (setup_date_time[i] - (10*data_tmp));
			*(dt_str + tag[(i<<1)+1]) = data_tmp + 0x30;
		}
	} else {
		data_tmp = setup_date_time[dt_tag]/10;
		*(dt_str + tag[(dt_tag<<1)]) = data_tmp + 0x30;
		data_tmp = (setup_date_time[dt_tag] - (10*data_tmp));
		*(dt_str + tag[(dt_tag<<1)+1]) = data_tmp + 0x30;
	}
}
void set_date_time_string_process(void*str, UINT8 index,UINT8 type)
{
	UINT8 i, data_tmp, days, tag[] = {2, 3, 7, 8, 12, 13};
	UINT8 tag1[] = {0, 1, 5, 6, 10, 11};
	UINT8 tag2[] = {0,1,5,6,12,13};
	days = CalendarCalculateDays(setup_date_time[1], setup_date_time[0]);
	if (setup_date_time[2] > days) {
		setup_date_time[2] = days;
	}	
	//time 
	if(index == 1)
		{
		for(i=0;i<3;i++){
			data_tmp = setup_date_time[3+i]/10;
			*((UINT8*)str + tag1[((i)<<1)]) = data_tmp + 0x30;
			data_tmp = (setup_date_time[3+i] - (10*data_tmp));
			*((UINT8*)str + tag1[((i)<<1)+1]) = data_tmp + 0x30;
		}	
	}
	//date
	if(index  == 0)
	{
		if(type == 0)
		{
			for(i=0;i<3;i++){
				data_tmp = setup_date_time[i]/10;
				*((UINT8*)str + tag[((i)<<1)]) = data_tmp + 0x30;
				data_tmp = (setup_date_time[i] - (10*data_tmp));
				*((UINT8*)str + tag[((i)<<1)+1]) = data_tmp + 0x30;
			}
		}
		else if(type == 1)
		{
			for(i=0;i<3;i++){
				data_tmp = setup_date_time[(i+1)%3]/10;
				*((UINT8*)str + tag2[((i)<<1)]) = data_tmp + 0x30;
				data_tmp = (setup_date_time[(i+1)%3] - (10*data_tmp));
				*((UINT8*)str + tag2[((i)<<1)+1]) = data_tmp + 0x30;
			}			
		}	
		else if(type == 2)
		{
			for(i=0;i<3;i++){

				if(i==0){
					data_tmp = setup_date_time[2]/10;
					*((UINT8*)str + tag2[((i)<<1)]) = data_tmp + 0x30;
					data_tmp = (setup_date_time[2] - (10*data_tmp));
					*((UINT8*)str + tag2[((i)<<1)+1]) = data_tmp + 0x30;
				}
				else if(i == 1)
				{
					data_tmp = setup_date_time[1]/10;
					*((UINT8*)str + tag2[((i)<<1)]) = data_tmp + 0x30;
					data_tmp = (setup_date_time[1] - (10*data_tmp));
					*((UINT8*)str + tag2[((i)<<1)+1]) = data_tmp + 0x30;					
				}
				else if(i == 2)
				{
					data_tmp = setup_date_time[0]/10;
					*((UINT8*)str + tag2[((i)<<1)]) = data_tmp + 0x30;
					data_tmp = (setup_date_time[0] - (10*data_tmp));
					*((UINT8*)str + tag2[((i)<<1)+1]) = data_tmp + 0x30;					
				}				
			}			
		}		
	}
	
}
void set_date_time(UINT8 index,UINT8 value)
{
	setup_date_time[index] = value;
}
UINT8 get_date_time(UINT8 index)
{
	return setup_date_time[index];
}
int 
jpg_scaler_process(
	scale_content_t sct
)
{
	int ret = 0;
	int devHandle;
	
	scale_content_t sct2;
	
	devHandle = open("/dev/scalar", O_RDONLY);
	
	if (devHandle < 0)
		return -1;
		
	if (sct.src_img.width > 2048 || sct.dst_img.width > 2048)
	{
		sct2 = sct;
		
		sct2.clip_rgn.x = 0;
		sct2.clip_rgn.y = 0;
		sct2.clip_rgn.width = sct.src_img.width/2;
		sct2.clip_rgn.height = sct.src_img.height;
			
		sct2.scale_rgn.x = 0;
		sct2.scale_rgn.y = 0;
		sct2.scale_rgn.width = sct.dst_img.width/2;
		sct2.scale_rgn.height = sct.dst_img.height;
		
		if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct2) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		sct2.clip_rgn.x = sct.src_img.width/2;
		sct2.clip_rgn.y = 0;
			
		sct2.scale_rgn.x = sct.dst_img.width/2;
		sct2.scale_rgn.y = 0;
		
		if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct2) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		close(devHandle);
		
		return ret;
	}
	
	if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct) < 0) {
		printf("scale_start fail\n");
		ret = -1;
	}
	close(devHandle);
	
	return ret;
}
SINT32 Gdjpegdecode(void *filepath, gp_bitmap_t *bitmap, int owidth, int oheight,gp_size_t* rect, int thumbnail)
{
	FILE* input;
	int size, out_size, format;
	char* pInput;
	char* pOutput = NULL;
	int ret = 0;
	cevaDecode_t vdt;

	ret = gpOn2Codec_Load(CEVA_CODEC_TYPE_JPG);
	if (ret < 0)
	{
		printf("Load Codec fail\n");
		return 0;
	}
	ret = gpOn2Codec_Init(NULL);
	if (ret < 0)
	{
		printf("Codec Initial fail\n");
		gpOn2Codec_Unload();
		return 0;
	}
	
	input = fopen(filepath, "rb");
	if (!input)
	{
		printf("cannot open file %s\n", filepath);
		goto exit;
	}
	
	fseek(input, 0, SEEK_END);
	size = ftell(input);
	fseek(input, 0, SEEK_SET);
	
	pInput = gpChunkMemAlloc(size);
	
	if (!pInput)
	{
		printf("Cannot allocate chunk memory buffer.\n");
		fclose(input);
		goto exit;
	}
	
	fread(pInput, 1, size, input);
	fclose(input);
		
	vdt.pInBuf    = pInput;
	vdt.nUsefulByte = size;
	vdt.flags = VFLAG_GET_INFO_ONLY | VFLAG_RGB565OUT;
	
	if (thumbnail)
		vdt.flags |= VFLAG_THUMBNAIL;
		
	ret = gpOn2Codec_Exec(&vdt);
	
	if (ret < 0)
	{
		if (thumbnail)
		{
			thumbnail = 0;
			vdt.flags &= ~VFLAG_THUMBNAIL;
			ret = gpOn2Codec_Exec(&vdt);
		}
		if (ret < 0)
		{
			printf("Get image info fail\n");
			goto exit;
		}
	}
	
	if (vdt.flags & VFLAG_RGB565OUT) 
	{
		out_size = vdt.width * vdt.height * 2;
		format = SP_BITMAP_RGB565;
	} 
	else if(vdt.flags & VFLAG_YUV422OUT) 
	{
		out_size = vdt.width * vdt.height * 2;
		format = SP_BITMAP_YCbYCr;
	} 
	else if (vdt.flags & VFLAG_YUV422SEMI) 
	{
		out_size = vdt.width * vdt.height *  2;
		format = SP_BITMAP_SEMI422;
	} 
	else if (vdt.flags & VFLAG_YUV400) 
	{
		out_size = vdt.width * vdt.height;
		format = SP_BITMAP_YCbCr400;
	}
	else if (vdt.flags & VFLAG_YUV444SEMI) 
	{
		out_size = vdt.width * vdt.height * 3;
		format = SP_BITMAP_SEMI444;
	} 
	else 
	{
		out_size = vdt.width * vdt.height * 3 /2 ;
		format  = SP_BITMAP_SEMI420;
	}
	
	printf("get image width = %d height = %d stride = %d strideChroma = %d format = %d\n", vdt.width, vdt.height, vdt.stride, vdt.strideChroma, format);
	
	pOutput = gpChunkMemAlloc(out_size);
	if (!pOutput)
	{
		printf("Cannot allocate chunk memory buffer.\n");
		goto exit;
	}
	
	vdt.pFrameBuf = pOutput;
	vdt.nFrameBufSize = out_size;
	vdt.flags = VFLAG_RGB565OUT;
	
	if (thumbnail)
		vdt.flags |= VFLAG_THUMBNAIL;
	
	ret = gpOn2Codec_Exec(&vdt);
	
	if (ret < 0 || (vdt.flags & VFLAG_FRAMEOUT) == 0)
	{
		printf("Decode Image fail\n");
		gpChunkMemFree(pOutput);
		pOutput = NULL;
		goto exit;
	}

	rect->width = vdt.width;
	rect->height = vdt.height;
	
	//if ((format != SP_BITMAP_RGB565) ||
	//	(owidth && oheight && (owidth != vdt.width || oheight != vdt.height)))
	{
		char* out;
		scale_content_t sct;
		
		if(!owidth) owidth = vdt.width;
		if(!oheight) oheight = vdt.height;
			
		memset(&sct, 0, sizeof(sct));
		
		sct.src_img.width    = vdt.width;
		sct.src_img.height   = vdt.height;
		sct.src_img.type	 = format;
		sct.src_img.bpl      = vdt.stride;
		sct.src_img.pData    = vdt.pOutBuf[0];
		sct.src_img.pDataU   = vdt.pOutBuf[1];
		sct.src_img.pDataV   = vdt.pOutBuf[2];
		sct.src_img.strideUV = vdt.strideChroma;
		

			sct.dst_img.pData = (void *)bitmap->pData;
			sct.dst_img.validRect.x = 0;
			sct.dst_img.validRect.y = 0;
			sct.dst_img.validRect.width = owidth;
			sct.dst_img.validRect.height = oheight;

			sct.scale_rgn.x      = bitmap->validRect.x;
			sct.scale_rgn.y      = bitmap->validRect.y;
			sct.scale_rgn.width  = bitmap->validRect.width;
			sct.scale_rgn.height = bitmap->validRect.height;
	
		sct.dst_img.width = owidth;
		sct.dst_img.height = oheight;
		sct.dst_img.bpl = owidth*2;
		sct.dst_img.type = SP_BITMAP_RGB565;
		
		sct.clip_rgn.x       = 0;
		sct.clip_rgn.y       = 0;
		sct.clip_rgn.width   = vdt.width;
		sct.clip_rgn.height  = vdt.height;
		printf("scaler start.\n");
		jpg_scaler_process(sct);
		printf("scaler end.\n");
		gpChunkMemFree(pOutput);
		if(!thumbnail) {
			pOutput = out;
		}
		else {
			pOutput = bitmap->pData;
		}
		
		rect->width = owidth;
		rect->height = oheight;
	}	
exit:
	gpOn2Codec_Uninit(NULL);
	gpOn2Codec_Unload();
	
	if (pInput)
		gpChunkMemFree(pInput);
	
	return pOutput;
}

