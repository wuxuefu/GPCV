#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "mach/typedef.h"
#include "disp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <mqueue.h>
#include "font.h"
#include "disp.h"
#include <linux/rtc.h>
#include <linux/input.h>
#include "ap_state_config.h"
#include "cvr_pipe.h"
#include <mach/audiodrv/audio_util.h>
#include <mach/audiodrv/soundcard.h>
#include "playback_pipe.h"
#include <mach/gp_usb.h>
#include <sys/mman.h>
#include <config/sysconfig.h>
#include <mach/gp_chunkmem.h>
#include "chunkmem.h"
#include <mach/gp_cdsp.h>
#include "uvc_pipe.h"

#include "gp_resample.h"
#include <errno.h>
/**
 * REMAP_MENU_MODE_KEY for remap long press menu key to mode key.
 */
#define REMAP_MENU_MODE_KEY 0


#define AUDIO_UPSAMPLE   1
#define UPSAMPLE_RATE    48000

#define RTC_TIME_SYNC  10*60
extern iconManager_t Icon_Manager[];
extern char dt_str[];
static pthread_t TimerThread = 0;
static pthread_t Timer2Thread = 0;
static pthread_t TimerThread_Ldwline = 0;
static pthread_t KeyThread = 0;
static pthread_t KeySoundThread = 0;
static pthread_t dispThread = 0;
UINT8 Playmode_flag;
extern UINT8 flip_flag;
extern UINT8 flip_flag1;
extern UINT32 time_num;
extern UINT32 USB_time_num;
extern UINT8 foreground_draw_flag;
extern UINT8 cur_draw_flag;
extern UINT32 free_size;
extern UINT8 Memory_path[64];
extern UINT32 free_size_flag;
extern UINT8 LDW_Change;
HANDLE hDisp;
HANDLE hDisp1;
mqd_t menumodeMsgQ;
mqd_t KeySoundMsgQ;
extern UINT8 setup_date_time[];
static UINT8 cap_timer[3]={2,5,10};
static UINT8 backlight_time[4]={0,3,5,10};
static UINT8 poweroff_time[4]={3,3,5,10};
static UINT8 key_timer;
UINT8 power_off = 0;
static struct mq_attr menumode_mq_attr = {
	.mq_maxmsg = 256,
	.mq_msgsize =  1
};
static struct mq_attr keysound_mq_attr = {
	.mq_maxmsg = 256,
	.mq_msgsize =  1
};
extern dv_set_t dv_set;
struct tm DV_start_time;
struct timeval starttime, endtime;
struct timeval cap_start, cap_end;
struct timeval timer_start, timer_end; //key or other
struct timeval powertime_s,powertime_e;
volatile int playback_menu_mode = -1;
char slide_show_flag = 0;
UINT32 time_sync=0;
static pthread_mutex_t key_mutex;

extern int DV_task_entry(void);
extern int DC_task_entry(void);
extern int PLAYBACK_task_entry(void);
extern int USB_task_entry(void);
void Play_Key_sound(UINT8 sound_type);
extern void DV_power_off();

int mem_fd, page_size, mapped_addr;

void UVC_Save(void)
{
	unsigned int msgId;
	void* msgPara;
	gp_cdsp_user_preference_t mode;
	char file_pa[256];
	char temp[256];
	int file_handle;
	uvc_pipemsg_send(UVC_G_PREFERENCE, 0, NULL);
	if(uvc_pipemsg_receive(&msgId,&msgPara) > 0) 
	{
		if(msgId == UVC_G_PREFERENCE_READY)
		{
			mode = *(gp_cdsp_user_preference_t*)msgPara; 
			printf("get cdsp OK!\n");
			if(dv_set.sd_check == 1)
			{
				printf("get cdsp OK!\n");
				strcpy(file_pa, Memory_path);
				strcat(file_pa, "/cdsp_preference.h"); 
				file_handle = open(file_pa,O_CREAT | O_RDWR);
				strncpy(temp, "#include \"mach/gp_cdsp.h\"\r\n\r\n", sizeof(temp));
				write(file_handle,temp,strlen(temp));
				strncpy(temp, "gp_cdsp_user_preference_t cdsp_user_preference =\r\n", sizeof(temp));
				write(file_handle,temp,strlen(temp));
				strncpy(temp, "{\r\n", sizeof(temp));
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.ae_target = %d,\r\n", mode.ae_target);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.y_scale_day = %d,\r\n", mode.y_scale_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.y_offset_day = %d,\r\n", mode.y_offset_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.y_scale_night = %d,\r\n", mode.y_scale_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.y_offset_night = %d,\r\n", mode.y_offset_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.u_offset_day = %d,\r\n", mode.u_offset_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.v_offset_day = %d,\r\n", mode.v_offset_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.u_offset_night = %d,\r\n", mode.u_offset_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.v_offset_night = %d,\r\n", mode.v_offset_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.u_scale_day = %d,\r\n", mode.u_scale_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.v_scale_day = %d,\r\n", mode.v_scale_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.u_scale_night = %d,\r\n", mode.u_scale_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.v_scale_night = %d,\r\n", mode.v_scale_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.edge_day = %d,\r\n", mode.edge_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.edge_night = %d,\r\n", mode.edge_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.wb_offset_day = %d,\r\n", mode.wb_offset_day);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.wb_offset_night = %d,\r\n", mode.wb_offset_night);
				write(file_handle,temp,strlen(temp));
				sprintf(temp,"\t.max_lum = %d,\r\n", mode.max_lum);
				write(file_handle,temp,strlen(temp));
				strncpy(temp, "};\r\n", sizeof(temp));
				write(file_handle,temp,strlen(temp)); 
				close(file_handle);
				system("sync");
			}
			else
			{
				printf("NO SDC!\n");
			}
		}
	}
} 

static int PLL_ctl_reqest()
{
	int offset_in_page, page_num, mapped_size;
	unsigned int scub_base_addr,
	
	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(mem_fd < 0)
	{
		printf("can not open /dev/mem !\n");
		return 0;
	}
	
	scub_base_addr = 0x90005000;
	
	page_size = getpagesize();
	offset_in_page = (unsigned int)(scub_base_addr & (page_size - 1));
	page_num = (offset_in_page + 0x600)/page_size;
	mapped_size = page_size * (page_num + 1);
	
	mapped_addr = (unsigned int)mmap(NULL, mapped_size, (PROT_READ | PROT_WRITE), 
							MAP_SHARED, mem_fd, scub_base_addr & ~(off_t)(page_size - 1));
	if (mapped_addr == (unsigned int*)MAP_FAILED)
	{
		close(mem_fd);
		printf("memory map fail\n");
		return 0;
	}
	
	return 1;
}

void PLL_ctl_en(int hd, int num, int enable)
{
	if(hd) {
		if(num==0) {
			if(enable)
				*(unsigned int*)((unsigned char*)mapped_addr + (0x64 & (unsigned int)(page_size - 1))) = 0x00000119;
			else
				*(unsigned int*)((unsigned char*)mapped_addr + (0x64 & (unsigned int)(page_size - 1))) = 0x00800119;
		}
		else if(num==1) {
			if(enable)
				*(unsigned int*)((unsigned char*)mapped_addr + (0x68 & (unsigned int)(page_size - 1))) = 0x00000119;
			else
				*(unsigned int*)((unsigned char*)mapped_addr + (0x68 & (unsigned int)(page_size - 1))) = 0x00800119;		
		}
	}
}

void PLL_ctl_release(int hd)
{
	if(hd) {
		munmap((void*)mapped_addr, page_size);
		close(mem_fd);
	}
}
void playback_set_menumode( int mode )
{
	playback_menu_mode = mode;
}

void *Timer2_Thread(void *arg) 
{
	UINT8 msg_id;
	int ret;
	time_t t;
	int i = 0;
	printf("Timer2_Thread Enter\n");
	while(1)
	{
		//printf("Timer2_Thread,dv_set.dc_ctrl=[%d],dv_set.zoom_flag=[%d],dv_set.zoom_num=[%d]\n",dv_set.dc_ctrl,dv_set.zoom_flag,dv_set.zoom_num);
		//
		if(dv_set.power_off >= 2)
			continue;
		if(power_off == 1)
		{
				gettimeofday(&timer_end,NULL);
				if(((timer_end.tv_sec-timer_start.tv_sec)*1000+(timer_end.tv_usec-timer_start.tv_usec)/1000)>=1500)
				{
					Power_off();
					power_off = 2;
					printf("power key,power off\n");
					//continue;
				}
				usleep(100000);
				continue;
		}
		if(Playmode_flag==CMD_VIDEO_MODE)
		{
			if(dv_set.motiondetect_start == 1)
			{
				dv_set.motiondetect_start = 0;
				if(dv_set.motiondetect_end == 1)
				{
					dv_set.motiondetect_end = 0;
					dv_set.lock_flag = 0;
				}
				if((SDC_FreeSize()<=5*1024)&&(DV_config_get(DV_LOOPRECORDING)==0)) //if disk free size <= 5MB, show sdc full
				{
					if(dv_set.sdc_full != 2)
					{
						dv_set.sdc_full = 2;
						time_num = 2;
						msg_id = dv_set.dv_UI_flag;
						mq_send(menumodeMsgQ, &msg_id, 1, 0);	
					}
					CVR_Set_PIPE_With_CMD(CMD_STOP_DV,0);
				}
				else if(dv_set.dv_UI_flag == DV_FOREGROUND_MODE)
				{
					if(dv_set.display_mode != SP_DISP_OUTPUT_HDMI)
					{
						if(dv_set.sdc_error !=0)
						{
							dv_set.sdc_error = 1;
							time_num = 2;
							CVR_Set_PIPE_With_CMD(CMD_STOP_DV,0);
						}
						else
						{
							CVR_Set_PIPE_With_CMD(CMD_START_DV,0);
							time(&t);
							DV_start_time= *localtime(&t);	
							gettimeofday(&cap_start,NULL);
							dv_set.dv_ctrl =1;
							printf("DV Motion detect start DV!\n"); 
						}
					}
					else
					{
						CVR_Set_PIPE_With_CMD(CMD_STOP_DV,0);
					}
				}
				else
				{
					CVR_Set_PIPE_With_CMD(CMD_STOP_DV,0);
				}			
			}
			else if(dv_set.motiondetect_end == 1)
			{
				dv_set.motiondetect_end = 0;
				dv_set.dv_ctrl =0;
				dv_set.lock_flag = 0;
				if(SDC_FreeSize()<=5*1024)
				{
					time_num = 2;
					dv_set.sdc_full = 2;
				}
				LED_Set(NORMAL_LED,1);
				printf("DV Motion detect stop DV! \n"); 			
			}
		}
		else
		{
			dv_set.motiondetect_start = 0;
			dv_set.motiondetect_end = 0;
			dv_set.dv_ctrl =0;
		}
		if((dv_set.battery_state == 5)&&(dv_set.usb_fd != -1))
		{

			ioctl(dv_set.usb_fd, USBDEVFS_GET_CONNECT_TYPE, &ret);
			if(ret == USB_DEVICE_CONNECT_TO_HOST)
			{
				printf("USB_DEVICE_CONNECT_TO_HOST !\n");
				dv_set.battery_state = 4;
				close(dv_set.usb_fd);
				system("rmmod g_spmp_storage");	
				system("rmmod spmp_udc_b");
				system("modprobe gp_usb");	
				dv_set.usb_fd = -1;
				USB_entry_init();
			}
			if(USB_time_num ==0)
			{
				close(dv_set.usb_fd);
				system("rmmod g_spmp_storage");	
				system("rmmod spmp_udc_b");
				system("modprobe gp_usb");
				dv_set.usb_fd = -1;
			}
			USB_time_num --;
			printf("Double Check USB Mode[%d]!\n",USB_time_num);

		}
		
		if(dv_set.dc_ctrl == 1)
		{
				printf("CAPTURE TIMER TEST[%d]\n",time_num);
				gettimeofday(&cap_end,NULL);
				if(((cap_end.tv_sec-cap_start.tv_sec)*1000+(cap_end.tv_usec-cap_start.tv_usec)/1000)>1000)
				{
					printf("CAPTURE time_num[%d]\n",time_num);
					gettimeofday(&cap_start,NULL);
					time_num --;
					if(time_num == 0)
					{
						//start capture
						printf("start capture\n");
						CVR_Set_PIPE_With_CMD(CMD_DO_CAPTURE,0);
						if(DC_config_get(DC_SEQUENCE)==0) //判断是否为连拍
							free_size--;
						else
							free_size = free_size - 3;						
						Play_Key_sound(SOUND_CAMERA);
						dv_set.ui_show = 0;
						dv_set.dc_ctrl = 0;						
					}
					else
					{
						Play_Key_sound(SOUND_KEY);
					}			
					msg_id = dv_set.dv_UI_flag; 
					ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);						
				}
				usleep(100000); // 100ms
				continue;
		}
		if(dv_set.zoom_flag == 1)
		{
			if(dv_set.zoom_num != 0)
			{
				dv_set.zoom_num--;
				CVR_Set_PIPE(CMD_SET_ZOOM,dv_set.zoom_num);			
				msg_id = dv_set.dv_UI_flag;
				mq_send(menumodeMsgQ, &msg_id, 1, 0);									
			}
			printf("zoom num = %d\n",dv_set.zoom_num);
			usleep(100);
			continue;
		}
		else if(dv_set.zoom_flag == 2)
		{
			if(dv_set.zoom_num != 30)
			{
				dv_set.zoom_num++;
				CVR_Set_PIPE(CMD_SET_ZOOM,dv_set.zoom_num);
				msg_id = dv_set.dv_UI_flag;
				mq_send(menumodeMsgQ, &msg_id, 1, 0);									
			}
			printf("zoom num = %d\n",dv_set.zoom_num);
			usleep(100);
			continue;
		}
		if(dv_set.sd_check == 4)
		{
			dv_set.sd_check = 1;
			msg_id = dv_set.dv_UI_flag;
			mq_send(menumodeMsgQ, &msg_id, 1, 0);
		}
		if((setting_config_get(SET_SCREEN_SAVER) != 0)&&(dv_set.backlight_flag == 1)&&(Playmode_flag != 0xff)&&(dv_set.display_mode == SP_DISP_OUTPUT_LCD))
		{
			gettimeofday(&endtime,NULL);
			if(endtime.tv_sec-starttime.tv_sec>12*60)
			{
				printf("first get time\n");
				gettimeofday(&starttime,NULL);
				gettimeofday(&endtime,NULL);
			}
			//printf("lcd backlight last time is %d\n",backlight_time[setting_config_get(SET_SCREEN_SAVER)]*60 -(endtime.tv_sec-starttime.tv_sec));
			if(endtime.tv_sec-starttime.tv_sec >= backlight_time[setting_config_get(SET_SCREEN_SAVER)]*60)
			{
				LCD_Backlight_Set(0); //disable lcd bakclinght
				dv_set.backlight_flag = 0;
			}
		}
		if((Playmode_flag <= CMD_PLAYBACK_MODE)&&(dv_set.dv_ctrl == 0)&&(slide_show_flag != 1)&&(DV_config_get(DV_MOTIONDETECTION)==0)&&(dvr_thmb_getDispMode(NULL) !=DISP_VIDEO_SCREEN))
		{
			//printf("power 0ff num %d\n",powertime_e.tv_sec-powertime_s.tv_sec);
			gettimeofday(&powertime_e,NULL);
			if(powertime_e.tv_sec-powertime_s.tv_sec>12*60)
			{
				printf("first get time\n");
				gettimeofday(&powertime_s,NULL);
				gettimeofday(&powertime_e,NULL);
			}
			//printf("power 0ff num %d\n",powertime_e.tv_sec-powertime_s.tv_sec);
			if(powertime_e.tv_sec-powertime_s.tv_sec >= poweroff_time[setting_config_get(SET_SCREEN_SAVER)]*60)
			{
				printf("power 0ff num %d\n",powertime_e.tv_sec-powertime_s.tv_sec);
				Power_off();
			}			
		}
		else
		{
			gettimeofday(&powertime_s,NULL);
		}
		/*time_sync++;
		if(time_sync >= RTC_TIME_SYNC)
		{
			time_sync = 0;
			system("hwclock -s");
		}*/
		if((Playmode_flag == CMD_PLAYBACK_MODE)&&(dv_set.dv_UI_flag == DV_FOREGROUND_MODE))
		{
			if(dv_set.battery_state == 5)	//charger
			{
				flip_flag1 = flip_flag1^1;
				msg_id = dv_set.dv_UI_flag;
				mq_send(menumodeMsgQ, &msg_id, 1, 0);				
			}
		}
		if(dv_set.sdc_full == 1)
		{
			if(SDC_FreeSize()>5*1024)
			{
				cur_draw_flag = DV_EXIT_MODE;
				foreground_draw_flag=0;		
				dv_set.sdc_full = 0;
			}
		}
		statusScanThread();
		if(dv_set.no_power_flag == 1)
		{
			dv_set.no_power_flag = 2;
			dv_set.ui_show = 1;
			dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
			msg_id = dv_set.dv_UI_flag;
			mq_send(menumodeMsgQ, &msg_id, 1, 0);				
		}
		else if(dv_set.no_power_flag == 2)
		{
			dv_set.no_power_flag = 1;
			dv_set.ui_show = 0;
			dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
			msg_id = dv_set.dv_UI_flag;
			mq_send(menumodeMsgQ, &msg_id, 1, 0);				
		}
		if(dv_set.battery_state == 0)
		{
			msg_id = dv_set.dv_UI_flag;
			mq_send(menumodeMsgQ, &msg_id, 1, 0);			
		}
		
		
		sleep(1);
	}
	printf("Timer2_Thread Exit\n");
//	pthread_detach(pthread_self());
//	pthread_exit(NULL);
	return (void *)0;	
}

void *Timer_Thread_LDW_Line(void *arg) //ldw 动态显示timer
{
	printf("Timer_Thread_LDW_Line Enter\n");
	while(1)
	{	
		if(DV_config_get(DV_LDW)!=0 && (Playmode_flag==CMD_VIDEO_MODE)&&(dv_set.dv_UI_flag == DV_FOREGROUND_MODE))
		{
			CVR_Set_PIPE_With_CMD(CMD_SET_LDW_LINE,1);
			usleep(40000);//for ldwdispline 25 frame
		}else
		{
			sleep(1);
		}
	}
}	

void *Timer_Thread(void *arg) 
{

	UINT8 msg_id;
	int ret;
	int i = 0;
	char slide_time[] = {2,5,8};
	char foregroundUpdata = 0;
	time_t t;
	int tc_cnt=0;

	printf("Timer_Thread Enter\n");
	while(1)
	{
		//dv_set.sd_check = 2
		//when time update post message to main task
		//ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);		
		//printf("Timer_Thread,Playmode_flag=[%d],dv_set.dv_UI_flag=[%d]\n",Playmode_flag,dv_set.dv_UI_flag);
		if(dv_set.power_off >= 2)
			continue;
		if((Playmode_flag==CMD_VIDEO_MODE)&&(((dv_set.dv_UI_flag == DV_FOREGROUND_MODE))||(dv_set.dv_UI_flag >= DV_EXIT_MODE))) //video timer 
		{
			//printf("Timer_Thread,dv_set.dv_ctrl=[%d],dv_set.battery_state=[%d],dv_set.sd_check=[%d]\n",dv_set.dv_ctrl,dv_set.battery_state,dv_set.sd_check);
			if(DV_check_exist() == 0)
			{
				dv_set.dv_UI_flag = DV_EXIT_MODE;
				Enable_Sensor_Power(0);
				usleep(10000);
				Enable_Sensor_Power(1);
				usleep(10000);
				Enable_Sensor_Reset(0);				
				usleep(10000);
				Enable_Sensor_Reset(1);	
				usleep(10000);				
				Enable_Sensor_Reset(0);	
				usleep(10000);
				DV_pipe_init();
				while(dv_set.dv_UI_flag == DV_EXIT_MODE);
				if(dv_set.sd_check != 0)
				DV_set_dir(Memory_path);
				if(dv_set.dv_ctrl== 1)
				{
					CVR_Set_PIPE_With_CMD(CMD_START_DV,0);
					time(&t);
					DV_start_time= *localtime(&t);	
					gettimeofday(&cap_start,NULL);					
				}
				dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
			}
			if((dv_set.dv_ctrl== 1)&&(dv_set.parking_mode_flag == 1))
			{
				gettimeofday(&cap_end,NULL);
				if(((cap_end.tv_sec-cap_start.tv_sec)*1000+(cap_end.tv_usec-cap_start.tv_usec)/1000)>500)
				{
					if(dv_set.lock_flag == 0)
					{
						CVR_Set_PIPE_With_CMD(CMD_SET_LOCK_FILE,1);
						dv_set.lock_flag = 1;
					}
					if(dv_set.usb_detect == 1)
					{
						dv_set.parking_mode_flag = 0;
					}
				}
				if(((cap_end.tv_sec-cap_start.tv_sec)*1000+(cap_end.tv_usec-cap_start.tv_usec)/1000)>10000)
				{
					CVR_Set_PIPE_With_CMD(CMD_STOP_DV,0);
					usleep(100*1000);
					dv_set.dv_ctrl =0;//设置录影标志为没有在录影
					dv_set.lock_flag = 0; //清除lock文件标志
					LED_Set(NORMAL_LED,1);	
					Power_off();
				}				
			}
			
			if(dv_set.dv_ctrl== 1)
			{
				if(DV_config_get(DV_GSENSOR) !=0)
				{
					if(Gsensor_read())
					{
						CVR_Set_PIPE_With_CMD(CMD_SET_LOCK_FILE,1);
						dv_set.lock_flag = 1;
					}
				}
				flip_flag = flip_flag^1; //red light
				/*time_num++;	//strat dv timer
				if(time_num >= 86400)
				{
					time_num = 0;
				}*/
			}
			else
			{
				if(DV_config_get(DV_GSENSOR) !=0)
				{
					Gsensor_read();
					dv_set.lock_flag = 0;
				}				
			}
			if(dv_set.battery_state == 5)	//charger
			{
				flip_flag1 = flip_flag1^1;
			}

			if(dv_set.sd_check == 2)
			{
				if(time_num <= 0)
				{
					cur_draw_flag = DV_EXIT_MODE;
					foreground_draw_flag=0;		
					dv_set.sd_check = 0;
				}
				else
				{
					time_num--;
				}
			}
			if(dv_set.sdc_full== 2)
			{
				if(time_num <= 0)
				{
					cur_draw_flag = DV_EXIT_MODE;
					foreground_draw_flag=0;		
					dv_set.sdc_full = 1;
				}
				else
				{
					time_num--;
				}
			}
			if(dv_set.sdc_error== 1)
			{
				if(time_num <= 0)
				{
					cur_draw_flag = DV_EXIT_MODE;
					foreground_draw_flag=0;		
					dv_set.sdc_error = 2;
				}
				else
				{
					time_num--;
				}
			}	

			msg_id = dv_set.dv_UI_flag; 
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0); 

			sleep(1); // 1.0s
			continue;
		}
		else if((Playmode_flag==CMD_STILL_MODE)&&(dv_set.dv_UI_flag == DV_FOREGROUND_MODE))
		{

			if(DV_check_exist() == 0)
			{
				dv_set.dv_UI_flag = DV_EXIT_MODE;
				Enable_Sensor_Power(0);
				usleep(10000);
				Enable_Sensor_Power(1);
				usleep(10000);
				Enable_Sensor_Reset(0);				
				usleep(10000);
				Enable_Sensor_Reset(1);	
				usleep(10000);				
				Enable_Sensor_Reset(0);	
				usleep(10000);			
				DC_pipe_init();
				while(dv_set.dv_UI_flag == DV_EXIT_MODE);
				if(dv_set.sd_check != 0)
				DV_set_dir(Memory_path);
				dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
			}
			if(dv_set.battery_state == 5)	//charger
			{
				flip_flag1 = flip_flag1^1;
				//printf("flip_flag1 = %d\n",flip_flag1);
				msg_id = dv_set.dv_UI_flag; 
				ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);					
			}
			if(dv_set.sd_check == 2)
			{
				if(time_num <= 0)
				{
					cur_draw_flag = DV_EXIT_MODE;
					foreground_draw_flag=0;		
					dv_set.sd_check = 0;
				}
				else
				{
					time_num--;
				}				
				msg_id = dv_set.dv_UI_flag; 
				ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);			
				sleep(1); // 1.0s
				continue;				
			}
			if(dv_set.sdc_full == 2)
			{
				if(time_num <= 0)
				{
					cur_draw_flag = DV_EXIT_MODE;
					foreground_draw_flag=0;		
					dv_set.sdc_full = 1;
				}
				else
				{
					time_num--;
				}				
				msg_id = dv_set.dv_UI_flag; 
				ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);			
				sleep(1); // 1.0s
				continue;				
			}	
			if(dv_set.sdc_error== 1)
			{
				if(time_num <= 0)
				{
					cur_draw_flag = DV_EXIT_MODE;
					foreground_draw_flag=0;		
					dv_set.sdc_error = 2;
				}
				else
				{
					time_num--;
				}				
				msg_id = dv_set.dv_UI_flag; 
				ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);			
				sleep(1); // 1.0s
				continue;				
			}			
			sleep(1);
		}
		else if((Playmode_flag == CMD_PLAYBACK_MODE) && dv_set.dv_UI_flag == DV_FOREGROUND_MODE) {
			ret = dvr_thmb_getDispMode(NULL);
			if(ret == DISP_VIDEO_SCREEN) {
				ret = dvr_thmb_getPlayingTime();
				if(ret != -1) {
					msg_id = dv_set.dv_UI_flag;
					ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
				}
				usleep(250*1000);
				continue;
			}
			else if(ret == DISP_FULL_SCREEN ) {
				i++;
				//printf("Timer_Thread,slide_show_flag=[%d],i=[%d]\n",slide_show_flag,i);
				if(slide_show_flag == 0){
					i = 0;
				}
				else if(i == slide_time[Playback_config_get(DV_WDR)]) {
					if(dvr_thmb_mutex_lock() == 0) {

						if(slide_show_flag == 1) {
							printf("%s:%d slide mode show next file!\n", __FUNCTION__, __LINE__);
							i = 0;
							dvr_thmb_nextIdx(NULL);
							dvr_thmb_dispFullScreen(NULL);
							foregroundUpdata = 1;
						}
						dvr_thmb_mutex_unlock();
					}
					else {
						
					}
				}
				/*if(dv_set.battery_state == 5)	//charger
				{
					foregroundUpdata = 1;
					flip_flag1 = flip_flag1^1;
				}*/
				if(foregroundUpdata == 1) {
					foregroundUpdata = 0;	
					msg_id = dv_set.dv_UI_flag;
					ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
				}
			}
			else {
				i =0;
			}
			sleep(1);
		}
		else if((Playmode_flag == CMD_PLAYBACK_MODE) && dv_set.dv_UI_flag == DV_MENU_MODE) {
			if(playback_menu_mode == PLAYBACK_MENU_WAIT_END) {
				printf("%s:%d get wait end! and update info!\n", __FUNCTION__, __LINE__);
				playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
				dvr_thmb_UpdateAllInfo(NULL);
				msg_id = dv_set.dv_UI_flag;
				ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
			}
			usleep(200*1000);
		}
		else {

			if(dv_set.dv_UI_flag == DV_SETTING_MODE)
			{
				if(dv_set.sd_check == 2)
				{
					if(time_num <= 0)
					{
						cur_draw_flag = DV_EXIT_MODE;
						foreground_draw_flag=0;		
						dv_set.sd_check = 0;
					}
					else
					{
						time_num--;
					}
					msg_id = dv_set.dv_UI_flag; 
					ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
				}
			
			}
			sleep(1);
		}
		
	}
	printf("Timer_Thread Exit\n");
//	pthread_detach(pthread_self());
//	pthread_exit(NULL);
	return (void *)0;
}
void ap_setting_mode_key_active(void)
{
	UINT8 msg_id;
	int ret;
	if((dv_set.dv_ctrl == 1)||(dv_set.dc_ctrl == 1)||(Playmode_flag == CMD_USB_MODE))
	{
		printf("usb mode or dc camera or dv have start,do nothing mode key\n");
		return;
	}
	if(dv_set.dv_UI_flag >= DV_EXIT_MODE)
	{
		return;
	}
	cur_draw_flag = DV_EXIT_MODE;
	foreground_draw_flag=0;
	dv_set.zoom_num = 0;
	if(dv_set.sd_check == 2)
	dv_set.sd_check = 0;
	if(dv_set.sdc_full == 2)
	dv_set.sdc_full= 1;	
	Playmode_flag++;
	if(Playmode_flag > CMD_PLAYBACK_MODE)
		Playmode_flag = CMD_VIDEO_MODE;
	dv_set.dv_UI_flag = DV_EXIT_MODE;	
	msg_id = dv_set.dv_UI_flag;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);		
}

void ap_setting_menu_key_active(void)
{	
	UINT8 msg_id;
	int ret;
	if((dv_set.sd_check == 2)||(dv_set.sdc_full == 2))
	{
		if(dv_set.sd_check == 2)
		dv_set.sd_check = 0;
		else
		dv_set.sdc_full = 1;	
		if(dv_set.dv_UI_flag == DV_SETTING_MODE)			//??
		{
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);
			//printf("@@@@@@@@@@@@@@@@@@@@\n");
			//return;
		}
	}
	if((dv_set.dc_ctrl == 1)||(Playmode_flag == CMD_USB_MODE))
	{
		printf("usb mode or dc camera,do nothing menu key\n");
		return;
	}
	if(dv_set.dv_ctrl == 1)
	{
		CVR_Set_PIPE_With_CMD(CMD_SET_LOCK_FILE,1);
		dv_set.lock_flag = 1;
		msg_id = dv_set.dv_UI_flag;
		ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
		return;
	}

	if(dv_set.menu_select_flag >= 1)//szk add 20141017
	{
		msg_id = dv_set.dv_UI_flag;
		dv_set.menu_select_flag = 0;
		ret = mq_send(menumodeMsgQ, &msg_id, 1, 0); 
		return; 
	}

	dv_set.dv_UI_flag ++;
	if(dv_set.dv_UI_flag > DV_SETTING_MODE)
		dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
	dv_set.menu_select_count=0;
	dv_set.menu_select_flag = 0;
		msg_id = dv_set.dv_UI_flag;
		ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
	UINT8 days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
void ap_setting_down_key_active(void)
{	
	UINT8 msg_id;
	SINT8 value;
	int ret;

	if(dv_set.dc_ctrl == 1)
		return;	
	if(Playmode_flag == CMD_USB_MODE)
	{
		if(dv_set.menu_select_flag == 0)
		{
			dv_set.menu_select_count++;
		}
		else
		{
			printf("usb mode,do nothing down key\n");
			return;
		}		
	}	
	else if(dv_set.dv_UI_flag == DV_FOREGROUND_MODE)
	{
		
	}
	else if(dv_set.dv_UI_flag == DV_MENU_MODE)
	{
		if(dv_set.menu_select_flag == 0)
		{
			dv_set.menu_select_count++;
		}
		else if((dv_set.menu_select_flag == 1)||(Playmode_flag != CMD_VIDEO_MODE))
		{
			dv_set.item_select_count++;
		}	
		else
		{
			dv_set.item_select_count1++;
		}
	}
	else if(dv_set.dv_UI_flag == DV_SETTING_MODE)
	{
		if(dv_set.menu_select_flag == 0)
		{
			dv_set.menu_select_count++;
		}	
		else if((dv_set.menu_select_flag == 1)||(dv_set.menu_select_flag == 2))
		{
			if(dv_set.menu_select_count == SET_DATA_FORMAT)
			{
				if(dv_set.item_select_count==6)
				{
					value = setting_config_get(SET_DATA_FORMAT);
					value --;
					if(value < 0)
					{
						value = 2;
					}
					setting_config_set(SET_DATA_FORMAT,value);
				}
				 else if(dv_set.item_select_count > 2)
				{
					value = get_date_time(dv_set.item_select_count);
					value--;				
					set_date_time(dv_set.item_select_count,value);
				 }else
				 {
					if(setting_config_get(dv_set.menu_select_count)==0)
					{
						value = get_date_time(dv_set.item_select_count);
						value--;							
						set_date_time(dv_set.item_select_count,value);
					}
					else if(setting_config_get(dv_set.menu_select_count)==1)
					{
						value = get_date_time((dv_set.item_select_count+1)%3);	
						value --;
						set_date_time((dv_set.item_select_count+1)%3,value);
					}
					else if(setting_config_get(dv_set.menu_select_count)==2)
					{
						if(dv_set.item_select_count==0)
						{
							value = get_date_time(2);	
							value --;
							set_date_time(2,value);
						}
						else if(dv_set.item_select_count==1)
						{
							value = get_date_time(1);	
							value --;
							set_date_time(1,value);
						}
						else if(dv_set.item_select_count==2)
						{
							value = get_date_time(0);	
							value --;
							set_date_time(0,value);
						}		
					}
				 }
				for(int i=0;i<6;i++)
				{
					value = get_date_time(i);
					printf("value = %d\n",value);
					if(i==0)
					{
						if(value < 12)
							value = 37;						
					}
					else if(i==1)
					{
						if(value < 1)
							value = 12;
					}
					else if(i==2)
					{
						if(value < 1)
						{
							value = days[get_date_time(1)-1];
							if(get_date_time(1) == 2)
							{
								if((((get_date_time(0)+2000)%4==0)&&((get_date_time(0)+2000)%100!=0))||((get_date_time(0)+2000)%400==0))
								{	
									printf("this year is leap year \n");
									value += 1;
								}
							}
						}
					}
					else if(i==3)
					{
						if(value < 0)
							value = 23;
					}
					else if ((i==4) ||(i==5))
					{
						if(value < 0)
							value = 59;
					}	
					set_date_time(i,value);
				}

				RTC_set();
				
			}
			else
			{
				dv_set.item_select_count++;
			}
		}		
	}


	msg_id = dv_set.dv_UI_flag;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}


void ap_setting_up_key_active(void)
{	
	UINT8 msg_id;
	UINT8 value;
	int ret;
	if(dv_set.dc_ctrl == 1)
		return;	
	
	if(Playmode_flag == CMD_USB_MODE)
	{
		if(dv_set.menu_select_flag == 0)
		{
			dv_set.menu_select_count--;
		}
		else
		{
			printf("usb mode,do nothing up key\n");
			return;
		}
	}
	else if(dv_set.dv_UI_flag == DV_MENU_MODE)
	{
		if(dv_set.menu_select_flag == 0)
		{
			dv_set.menu_select_count--;
		}
		else if((dv_set.menu_select_flag == 1)||(Playmode_flag != CMD_VIDEO_MODE))
		{
			dv_set.item_select_count--;
		}	
		else
		{
			dv_set.item_select_count1--;
		}
	}
	else if(dv_set.dv_UI_flag == DV_SETTING_MODE)
	{
		if(dv_set.menu_select_flag == 0)
		{
			dv_set.menu_select_count--;
		}
		else if((dv_set.menu_select_flag == 1)||(dv_set.menu_select_flag == 2))
		{
			if(dv_set.menu_select_count == SET_DATA_FORMAT)
			{

				 if(dv_set.item_select_count==6)
				{
					value = setting_config_get(SET_DATA_FORMAT);
					value ++;
					if(value > 2)
					{
						value = 0;
					}
					setting_config_set(SET_DATA_FORMAT,value);
				}
				 else if(dv_set.item_select_count > 2)
				{
					value = get_date_time(dv_set.item_select_count);
					value++;				
					set_date_time(dv_set.item_select_count,value);
				 }else
				 {
					if(setting_config_get(dv_set.menu_select_count)==0)
					{
						value = get_date_time(dv_set.item_select_count);
						value++;							
						set_date_time(dv_set.item_select_count,value);
					}
					else if(setting_config_get(dv_set.menu_select_count)==1)
					{
						value = get_date_time((dv_set.item_select_count+1)%3);	
						value ++;
						set_date_time((dv_set.item_select_count+1)%3,value);
					}
					else if(setting_config_get(dv_set.menu_select_count)==2)
					{

						if(dv_set.item_select_count==0)
						{	
							value = get_date_time(2);	
							value ++;
							set_date_time(2,value);
						}
						else if(dv_set.item_select_count==1)
						{
							value = get_date_time(1);	
							value ++;
							set_date_time(1,value);
						}
						else if(dv_set.item_select_count==2)
						{
							value = get_date_time(0);	
							value ++;
							set_date_time(0,value);
						}	
					}
				 }
				for(int i=0;i<6;i++)
				{
					value = get_date_time(i);
					printf("value = %d\n",value);
					if(i==0)
					{
						if(value > 37)
							value = 12;						
					}
					else if(i==1)
					{
						if(value>12)
							value = 1;
					}
					else if(i==2)
					{

						if(value >days[get_date_time(1)-1])
						{
							if((get_date_time(1) == 2)&&(value <= 29))
							{
								if((((get_date_time(0)+2000)%4==0)&&((get_date_time(0)+2000)%100!=0))||((get_date_time(0)+2000)%400==0))
								{	
										printf("this year is leap year \n");
										value = 29;
								}
								else
									value = 1;
							}
							else
								value = 1;
						}
					}
					else if(i==3)
					{
						if(value > 23)
							value = 0;
					}
					else if ((i==4) ||(i==5))
					{
						if(value > 59)
							value = 0;
					}	
					set_date_time(i,value);
				}
				RTC_set();
				
			}
			else
			{
				dv_set.item_select_count--;
			}			
		}

		
	}
	msg_id = dv_set.dv_UI_flag;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
void ap_setting_enter_key_active(void)
{	
	UINT8 msg_id;
	int ret;
	time_t t;
	
	if(dv_set.dc_ctrl == 1) //在定时拍照中，拍照键无效
		return;	
	if(Playmode_flag == CMD_USB_MODE) //usb模式
	{
		if(dv_set.menu_select_flag == 0) //usb一级菜单，选择mass storage or pc Camera
		{
			dv_set.menu_select_flag = 1;	//按下确定键后，进入二级显示
			msg_id = dv_set.dv_UI_flag;
			ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);				
			if(dv_set.menu_select_count == 0||dv_set.menu_select_count == 2)//mass storage
			{

				if(dv_set.sd_check == 1)
				{
					do
					{
						ret = CheckMemoryPath(); //get memory path
						usleep(10000);
					}
					while(ret);
					usb_plugin();	
					unmount_dev();
				}
			}
			else
			{
				uvc_pipe_init();
				system("/system/app/uvc_csi -i 11 -s 1080p &");
				printf("@@@@@@@@@@@Enter PC Camera@@@@@@@@@@@@@@@@@@@@\n");
				//PC Camera
			}
			return;
		}
		else //进入2级模式后，按键将失效。
		{
			printf("usb mode,do nothing enter key\n");
			UVC_Save();
			return;
		}
	}
	
	if(dv_set.dv_UI_flag == DV_FOREGROUND_MODE) //前景模式下的确定键功能
	{
		if(Playmode_flag == CMD_VIDEO_MODE) //DV模式下，前景状态下的确定键定义
		{
			if((dv_set.sd_check == 0)||(dv_set.sd_check == 2))//当未插入sdc or 正在显示请插入sdc时
			{
				if(dv_set.sd_check == 0) //当未插入sdc时按下确定键，显示请插入sdc，显示时间为3s
				{
					dv_set.sd_check = 2;//show insert sdc
					time_num = 2; //设置显示时间
					msg_id = dv_set.dv_UI_flag;
					mq_send(menumodeMsgQ, &msg_id, 1, 0);						
				}
				return;
			}
			if((SDC_FreeSize()<=5*1024)&&(dv_set.dv_ctrl  !=1 )&&(DV_config_get(DV_LOOPRECORDING) == 0)) //if disk free size <= 5MB, show sdc full
			{
				if(dv_set.sdc_full != 2)
				{
					dv_set.sdc_full = 2;
					time_num = 2;
					msg_id = dv_set.dv_UI_flag;
					mq_send(menumodeMsgQ, &msg_id, 1, 0);	
				}
				printf("show SDC FULL!");
				return;		
			}			
			if(dv_set.dv_ctrl == 0) //判断录影状态，如果没有录影，则开始录影
			{
				//start dv
				if(dv_set.sdc_error !=0)
				{
					dv_set.sdc_error = 1;
					time_num = 2;
				}
				else if(dv_set.display_mode != SP_DISP_OUTPUT_HDMI)
				{
					CVR_Set_PIPE_With_CMD(CMD_START_DV,0);
					time(&t); //开始计算录影时间
					DV_start_time= *localtime(&t);
					gettimeofday(&cap_start,NULL);
					dv_set.dv_ctrl =1; //设置录影标志为录影中
				}
				else
				{
					printf("Disp is not LCD,can not start DV!");
					return;
				}
			}
			else //如果判断为在录影中，则按下确定键为结束录影
			{
				//stop dv
				gettimeofday(&cap_end,NULL);
				if(((cap_end.tv_sec-cap_start.tv_sec)*1000+(cap_end.tv_usec-cap_start.tv_usec)/1000)>1500)
				{
					CVR_Set_PIPE_With_CMD(CMD_STOP_DV,0);
					usleep(100*1000);
					dv_set.dv_ctrl =0;//设置录影标志为没有在录影
					dv_set.lock_flag = 0; //清除lock文件标志
					LED_Set(NORMAL_LED,1);
				}
				else
				{
					printf("dv have start,time < 1s,can not stop!");
					printf("cap_start.tv_sec=%d,cap_end.tv_sec=%d\n",cap_start.tv_sec,cap_end.tv_sec);
					return;
				}
			}
		}
		else if(Playmode_flag == CMD_STILL_MODE) //拍照模式
		{
			if((dv_set.sd_check == 0)||(dv_set.sd_check == 2))
			{
				Play_Key_sound(SOUND_KEY);//拍照声
				if(dv_set.sd_check == 0)
				{
					dv_set.sd_check = 2;//show insert sdc
					time_num = 2;
					msg_id = dv_set.dv_UI_flag;
					mq_send(menumodeMsgQ, &msg_id, 1, 0);						
				}
				return;
			}
			if((SDC_FreeSize()<=5*1024)) //if disk free size <= 5MB, show sdc full
			{
				Play_Key_sound(SOUND_KEY);//拍照声
				if(dv_set.sdc_full != 2)
				{
					dv_set.sdc_full = 2;
					time_num = 2;
					msg_id = dv_set.dv_UI_flag;
					mq_send(menumodeMsgQ, &msg_id, 1, 0);	
				}
				return;		
			}
			if(dv_set.sdc_error !=0)
			{
				Play_Key_sound(SOUND_KEY);//拍照声
				dv_set.sdc_error = 1;
				time_num = 2;
			}
			else if(DC_config_get(DC_CAPTURE_MODE)==0) //非定时拍照
			{
				CVR_Set_PIPE_With_CMD(CMD_DO_CAPTURE,0);
				if(DC_config_get(DC_SEQUENCE)==0) //判断是否为连拍
					free_size--;
				else
					free_size = free_size - 3;
				Play_Key_sound(SOUND_CAMERA);//拍照声
				dv_set.ui_show = 0;//不显示UI
				//start dc	
			}
			else //定时拍照
			{
				dv_set.dc_ctrl = 1;
				gettimeofday(&cap_start,NULL);
				time_num = cap_timer[DC_config_get(DC_CAPTURE_MODE)-1];//获得定时时长
				msg_id = dv_set.dv_UI_flag;
				ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);					
				return;
			}
		}
	}

	//menu UI
	else if(dv_set.dv_UI_flag == DV_MENU_MODE)
	{

		if((Playmode_flag == CMD_PLAYBACK_MODE)&&(dv_set.menu_select_flag == 1)&&(dv_set.menu_select_count == 0))
		{
			printf("%s:%d\n", __FUNCTION__, __LINE__);
			printf("enter menu_select_count= %d,item_select_count = %d flag %d\n",dv_set.menu_select_count,dv_set.item_select_count, dv_set.menu_select_flag);
			/*add by deyue for playback */
			if(dv_set.item_select_count == 0) {
				printf("delete cur!\n");
				playback_menu_mode = PLAYBACK_MENU_DELETE_CUR_S;
			}
			else if(dv_set.item_select_count == 1) {
				printf("delete all!\n");
				playback_menu_mode = PLAYBACK_MENU_DELETE_ALL_S;
			}
			else {
				playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
			}
			/*add end*/
			dv_set.menu_select_flag = 2;
			dv_set.item_select_count=2;
		}
		else if((Playmode_flag == CMD_PLAYBACK_MODE)&&(dv_set.menu_select_flag == 1)&&(dv_set.menu_select_count == 1))
		{
					/*deyue add for lock/unlock file*/
			if( dv_set.item_select_count == 0) {
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				playback_menu_mode = PLAYBACK_MENU_LOCK_CUR_S;
			}
			else if(dv_set.item_select_count == 1) {
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				playback_menu_mode = PLAYBACK_MENU_UNLOCK_CUR_S;
			}
			else if( dv_set.item_select_count == 2) {
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				playback_menu_mode = PLAYBACK_MENU_LOCK_ALL_S;
			}
			else if(dv_set.item_select_count == 3) {
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				playback_menu_mode = PLAYBACK_MENU_UNLOCK_ALL_S;
			}
			else {
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
			}
			dv_set.menu_select_flag = 2;
			dv_set.item_select_count=2;			
			/*add end*/
		}
		else if((Playmode_flag == CMD_PLAYBACK_MODE)&&(dv_set.menu_select_flag == 2))
		{
			printf("%s:%d\n", __FUNCTION__, __LINE__);
	printf("enter menu_select_count= %d,item_select_count = %d flag %d\n",dv_set.menu_select_count,dv_set.item_select_count, dv_set.menu_select_flag);
			/*add by deyue for playback */
			if(dv_set.item_select_count == 2 ) {
				printf("delete or protect control!!\n");
				playback_menu_mode = PLAYBACK_MENU_DELETE_NONE;
			}
			else if(dv_set.item_select_count == 3 ) {
				printf("delete or protect ok %d \n", playback_menu_mode);
				if(playback_menu_mode == PLAYBACK_MENU_DELETE_ALL_S) {
					playback_menu_mode = PLAYBACK_MENU_DELETE_ALL;
				}
				else if(playback_menu_mode == PLAYBACK_MENU_DELETE_CUR_S) {
					playback_menu_mode = PLAYBACK_MENU_DELETE_CUR;
				}
				else if(playback_menu_mode == PLAYBACK_MENU_LOCK_CUR_S) {
					playback_menu_mode = PLAYBACK_MENU_LOCK_CUR;
				}
				else if(playback_menu_mode == PLAYBACK_MENU_UNLOCK_CUR_S) {
					playback_menu_mode = PLAYBACK_MENU_UNLOCK_CUR;
				}	
				else if(playback_menu_mode == PLAYBACK_MENU_LOCK_ALL_S) {
					playback_menu_mode = PLAYBACK_MENU_LOCK_ALL;
				}
				else if(playback_menu_mode == PLAYBACK_MENU_UNLOCK_ALL_S) {
					playback_menu_mode = PLAYBACK_MENU_UNLOCK_ALL;
				}				
				else {
					playback_menu_mode = PLAYBACK_MENU_DELETE_NONE;
				}
			}
			else {
				playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
			}
			/*add end*/
			dv_set.menu_select_flag = 0;
			dv_set.item_select_count=0;
		}
		else if((dv_set.menu_select_count == DV_LDW)&&(Playmode_flag == CMD_VIDEO_MODE)&&(dv_set.dv_UI_flag == DV_MENU_MODE))
		{
			if(dv_set.menu_select_flag == 0)
			{
				dv_set.menu_select_flag = 1;
				dv_set.item_select_count = 0;
			}
			else if(dv_set.menu_select_flag == 1)
			{
				//printf("dv_set.menu_select_flag= %d,dv_set.item_select_count=%d\n",dv_set.menu_select_flag,dv_set.item_select_count);
				dv_set.menu_select_flag = 2;
				if(dv_set.item_select_count == 0)//enable
				dv_set.item_select_count1 = DV_config_get(DV_LDW);	
				else if(dv_set.item_select_count == 1)//car type
					dv_set.item_select_count1 = ap_state_config_LDW_Car_type_get();
				else if(dv_set.item_select_count == 2)//Sensitivity
					dv_set.item_select_count1 = ap_state_config_LDW_Sensitivity_get();
				else if(dv_set.item_select_count == 3)//Area_Choice
					dv_set.item_select_count1 = ap_state_config_LDW_Area_Choice_get();
				else if(dv_set.item_select_count == 4)//speed
					dv_set.item_select_count1 = ap_state_config_LDW_SPEED_get();	
				else if(dv_set.item_select_count == 5)//sound
					dv_set.item_select_count1 = ap_state_config_LDW_Sound_get();					
				//printf("dv_set.menu_select_flag= %d,dv_set.item_select_count1=%d\n",dv_set.menu_select_flag,dv_set.item_select_count1);
			}
			else if(dv_set.menu_select_flag == 2)
			{
				//printf("dv_set.menu_select_flag= %d,dv_set.item_select_count=%d\n",dv_set.menu_select_flag,dv_set.item_select_count);
				dv_set.menu_select_flag = 1;
				if(dv_set.item_select_count == 0)//enable
					DV_config_set(DV_LDW,dv_set.item_select_count1);	
				else if(dv_set.item_select_count == 1)//car type
					 ap_state_config_LDW_Car_type_set(dv_set.item_select_count1);
				else if(dv_set.item_select_count == 2)//Sensitivity
					 ap_state_config_LDW_Sensitivity_set(dv_set.item_select_count1);
				else if(dv_set.item_select_count == 3)//Area_Choice
					 ap_state_config_LDW_Area_Choice_set(dv_set.item_select_count1);	
				else if(dv_set.item_select_count == 4)//speed
					 ap_state_config_LDW_SPEED_set(dv_set.item_select_count1);	
				else if(dv_set.item_select_count == 5)//sound
				{
					ap_state_config_LDW_Sound_set(dv_set.item_select_count1);					
					ap_state_config_store();
				}
				//printf("dv_set.menu_select_flag= %d,dv_set.item_select_count1=%d\n",dv_set.menu_select_flag,dv_set.item_select_count1);
				
				if(LDW_Change == 1)
				{
					LDW_Change = 0;
					ap_state_config_store();
					CVR_SET_LDW();
				}
			}
		}
		else if(dv_set.menu_select_flag == 0)
		{
			dv_set.menu_select_flag = 1;
			if(Playmode_flag == CMD_VIDEO_MODE)
				dv_set.item_select_count = DV_config_get(dv_set.menu_select_count);
			else if(Playmode_flag == CMD_STILL_MODE)
				dv_set.item_select_count = DC_config_get(dv_set.menu_select_count);
			else if(Playmode_flag == CMD_PLAYBACK_MODE)
				dv_set.item_select_count = Playback_config_get(dv_set.menu_select_count);
			printf("DV_DC_PLAY_config_get menu_select_count= %d,item_select_count = %d\n",dv_set.menu_select_count,dv_set.item_select_count);
		}
		else
		{	
			dv_set.menu_select_flag = 0;
			printf("DV_DC_PLAY_config_set menu_select_count= %d,item_select_count = %d\n",dv_set.menu_select_count,dv_set.item_select_count);
			if(Playmode_flag == CMD_VIDEO_MODE)
				DV_config_set(dv_set.menu_select_count,dv_set.item_select_count);
			else if(Playmode_flag == CMD_STILL_MODE)
				DC_config_set(dv_set.menu_select_count,dv_set.item_select_count);
			else if(Playmode_flag == CMD_PLAYBACK_MODE) {
				Playback_config_set(dv_set.menu_select_count,dv_set.item_select_count);
				if(dv_set.menu_select_count == DV_WDR) {
					// begin slide show
					printf("enter slide mode!!!\n");
					slide_show_flag = 1;
					dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
				}
			}
			dv_set.item_select_count = 0;
		}
	}
	//setting UI
	else if(dv_set.dv_UI_flag == DV_SETTING_MODE)
	{

		if(dv_set.menu_select_count == SET_DATA_FORMAT)
		{		
			if(dv_set.menu_select_flag == 0)
			{
				dv_set.menu_select_flag = 1;
				dv_set.item_select_count=0;
			}
			else if(dv_set.menu_select_flag == 1)
			{
				dv_set.item_select_count++;
				if(dv_set.item_select_count >= 7)
				{
					RTC_set();
					dv_set.item_select_count=0;
					dv_set.menu_select_flag = 0;
				}
			}
		}
		else if(((dv_set.menu_select_count>=SET_SOUND)&&(dv_set.menu_select_count<=SET_IR_LED))||(dv_set.menu_select_count==SET_GSENSOR1))
		{
			if(dv_set.menu_select_flag == 0)
			{
				dv_set.menu_select_flag = 1;
				dv_set.item_select_count = setting_config_get(dv_set.menu_select_count);
			}
			else
			{
				dv_set.menu_select_flag = 0;
				printf("DV_config_set menu_select_count= %d,item_select_count = %d\n",dv_set.menu_select_count,dv_set.item_select_count);
				setting_config_set(dv_set.menu_select_count,dv_set.item_select_count);
				dv_set.item_select_count = 0;
			}			
		}
		else if(dv_set.menu_select_count == SET_FORMAT) //format
		{
			if(dv_set.menu_select_flag == 0)
			{
				dv_set.menu_select_flag = 1;
				dv_set.item_select_count=0;
			}
			else if(dv_set.menu_select_flag == 1)
			{
				dv_set.menu_select_flag = 2;
				dv_set.item_select_count=2;				
			}
			else
			{
				if(dv_set.item_select_count == 3) //OK
				{
					//format sdc
					if(dv_set.sd_check == 1)
					{
						dv_set.sd_check = 3;
						msg_id = dv_set.dv_UI_flag;
						mq_send(menumodeMsgQ, &msg_id, 1, 0);
						pthread_mutex_lock(&key_mutex);
						if(CheckMemoryPath() ==0)
						{
							ret = SDC_Format();
							dv_set.sdc_error = 0;
							dv_set.sd_check = 4;
							free_size_flag = 1;	
							SDC_FreeSize();
						}
						else
						{
							if(Format2()==0)
							{
								dv_set.sdc_error = 0;
								dv_set.sd_check = 4;
								free_size_flag = 1;
								DV_set_dir(Memory_path);
								SDC_FreeSize();
							}
							else
							{
								Power_off();
							}
						}
						pthread_mutex_unlock(&key_mutex);

						//msg_id = dv_set.dv_UI_flag;
						//mq_send(menumodeMsgQ, &msg_id, 1, 0);							
					}
					else
					{
						if(dv_set.sd_check == 0)
						{
							dv_set.sd_check = 2;
							time_num = 2;
							msg_id = dv_set.dv_UI_flag;
							mq_send(menumodeMsgQ, &msg_id, 1, 0);	
							return;
						}
					}
				}
				dv_set.menu_select_flag = 0;
				dv_set.item_select_count = 0;
			}
		}
		else if(dv_set.menu_select_count == SET_DEFAULT_SETTING) //default setting
		{
			if(dv_set.menu_select_flag == 0)
			{
				dv_set.menu_select_flag = 2;
				dv_set.item_select_count=2;
			}
			else
			{
				if(dv_set.item_select_count == 3) //OK
				{
					//default setting
					ap_state_config_default_set();
					//ap_state_config_store();
					if(Playmode_flag != CMD_PLAYBACK_MODE)
					{
						CVR_Set_PIPE_With_CMD(CMD_SET_DEFAULT_SETTING,1);
					}
					else
					{
						dv_set.default_setting_flag = 1;
						dv_set.date_type_flag = 0;
					}
				}
				dv_set.menu_select_flag = 0;
				dv_set.item_select_count = 0;
			}
		}	
		else if(dv_set.menu_select_count == SET_VERSION) //version
		{
			if(dv_set.menu_select_flag == 0)
			{
				/*if(dv_set.ldws_on == 0)
					dv_set.ldws_on = 1;
				else
					dv_set.ldws_on = 0;*/
				dv_set.menu_select_flag = 3;
				dv_set.item_select_count=0;				
			}
			else
			{
				dv_set.menu_select_flag = 0;
				dv_set.item_select_count = 0;
			}
		}				
	}
	msg_id = dv_set.dv_UI_flag;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}

void ap_setting_up_L_key_active(void)
{
	UINT8 msg_id;
	int ret;
	
	/*if(dv_set.dv_UI_flag == DV_MENU_MODE) {
		dv_set.dv_UI_flag = DV_SETTING_MODE;
	}
	else if(dv_set.dv_UI_flag == DV_SETTING_MODE) {
		dv_set.dv_UI_flag = DV_MENU_MODE;
	}
	dv_set.menu_select_count=0;
	dv_set.menu_select_flag = 0;*/
	msg_id = dv_set.dv_UI_flag;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}

void ap_setting_down_L_key_active(void)
{
	UINT8 msg_id;
	int ret;
	
	/*if(dv_set.dv_UI_flag == DV_MENU_MODE) {
		dv_set.dv_UI_flag = DV_SETTING_MODE;
	}
	else if(dv_set.dv_UI_flag == DV_SETTING_MODE) {
		dv_set.dv_UI_flag = DV_MENU_MODE;
	}
	dv_set.menu_select_count=0;
	dv_set.menu_select_flag = 0;*/
	msg_id = dv_set.dv_UI_flag;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}

void ap_send_down_L_key_active(void) 
{
	UINT8 msg_id;
	int ret = -1;
	msg_id = DV_DOWN_L_KEY;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
void ap_send_up_L_key_active(void) 
{
	UINT8 msg_id;
	int ret = -1;
	msg_id = DV_UP_L_KEY;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
void ap_send_down_key_active(void) 
{
	UINT8 msg_id;
	int ret = -1;
	msg_id = DV_DOWN_KEY;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
void ap_send_up_key_active(void) 
{
	UINT8 msg_id;
	int ret = -1;
	msg_id = DV_UP_KEY;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
void ap_send_menu_key_active(void) 
{
	UINT8 msg_id;
	int ret = -1;
	msg_id = DV_MENU_KEY;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
void ap_send_enter_key_active(void) 
{
	UINT8 msg_id;
	int ret = -1;
	msg_id = DV_ENTER_KEY;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}
void ap_send_mode_key_active(void) 
{
	UINT8 msg_id;
	int ret = -1;
	msg_id = DV_MODE_KEY;
	ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);	
}


void Play_Key_sound(UINT8 sound_type)
{
	UINT8 msg_id;
	if(setting_config_get(SET_SOUND) == 1)
	{
		if(dvr_check_video_playing()) {
			msg_id = sound_type;
			mq_send(KeySoundMsgQ, &msg_id, 1, 0);			
		}		
	}
	else if(sound_type == SOUND_POWER_OFF)
	{
		dv_set.power_off = 1;
	}
}

static void Dsp_set_Vol(int nGotoVol)
{
	printf("mmmmm set vol %d\n", nGotoVol);
	//using audio_mixer
	SINT32 volume = nGotoVol&0xff;
	volume |= volume<<8;
    SINT32 MixHandle = open("/dev/mixer", O_RDONLY);

    ioctl(MixHandle, SOUND_MIXER_WRITE_VOLUME, &volume);
    close(MixHandle);
}
static int keysound_handle=-1;

void Dsp_Open()
{
	int freq, ch;
	if(keysound_handle == -1)
	{
		keysound_handle = open("/dev/dsp", O_WRONLY);
		if(keysound_handle<0) {
			keysound_handle = -1;
			printf("open dsp error!\n");
			return;
		}
		//int flag = fcntl(keysound_handle, F_GETFD);
		//flag |= FD_CLOEXEC;
		fcntl(keysound_handle, F_SETFD, 1);

		int path = I2S_TX; //HDMI_TX
		if(dv_set.display_mode == SP_DISP_OUTPUT_HDMI) {
			path = HDMI_TX;
		}
		ioctl(keysound_handle, SNDCTL_DSP_GP_INIT, &path);	

		int fmt = AFMT_S16_LE;
		ioctl(keysound_handle, SNDCTL_DSP_SETFMT, &fmt);	

if(path == HDMI_TX) {
		ch = 2;
		freq = UPSAMPLE_RATE;
} else {
		ch = 1;
		freq = 16000;
}
		
		ioctl(keysound_handle, SNDCTL_DSP_CHANNELS, &ch);
		ioctl(keysound_handle, SNDCTL_DSP_SPEED, &freq);
	}
}
void Dsp_close(void)
{
	int ret = 0;
	if(keysound_handle != -1)
	{
        ioctl(keysound_handle, SNDCTL_DSP_SYNC, &ret);	
		close(keysound_handle);
		keysound_handle = -1;
	}
}
void *KeySound_Thread(void *arg)
{
	UINT8 msg_id, msg_prio;
	int ret, handle;
	FILE *fptr;
	char rbuf[512]={0};
	int freq;
	struct timespec timeout;
	int upsample=0;
	static int upsample_init=0;

//#if AUDIO_UPSAMPLE	
	gp_resample_config_t config;
	gp_resample_state_t* resample;
	gp_resample_data_t data;
	UINT8* up_buf;
	UINT32 up_ratio = 0;
	SINT32 error;
//#endif
	
	Dsp_Open();

	while(1)
	{
		ret = mq_receive(KeySoundMsgQ, (char *)&msg_id, 1, (unsigned int *)&msg_prio);
//restart:
		if(dv_set.display_mode == SP_DISP_OUTPUT_HDMI)
			upsample = 1;
		else {
			upsample_init = 0;
			upsample = 0;
		}

		if(upsample && upsample_init==0) {
			/* create resample context */
			config.input_format = GP_RESAMPLE_FORMAT_16BIT;
			config.output_format = GP_RESAMPLE_FORMAT_16BIT;
			config.input_sample_rate = 16000;
			config.output_sample_rate = UPSAMPLE_RATE;
			config.converter_type = GP_RESAMPLE_LINEAR;
			config.input_channels = 1;
			config.output_channels = 2;
			resample = gp_resample_new(config, &error);
			
			up_ratio = UPSAMPLE_RATE / config.input_sample_rate;
			up_buf = malloc(512*up_ratio*config.output_channels);
			
			if (!resample)
			{
				printf("[%s:%d] create resample failed!! keysound return to 16K 1 channel\r\n", __FUNCTION__, __LINE__);
				
				ioctl(keysound_handle, SNDCTL_DSP_CHANNELS, &config.input_channels);
				ioctl(keysound_handle, SNDCTL_DSP_SPEED, &config.input_sample_rate);
			}
			upsample_init = 1;
		}			
		
		if(keysound_handle != -1)
		{
			if(msg_id == SOUND_KEY)
			{
				fptr = fopen("/system/resource/sound/BEEP.pcm", "rb");
			}
			else if(msg_id == SOUND_CAMERA)
			{
				fptr = fopen("/system/resource/sound/CAMERA.pcm", "rb");
			}
			else if(msg_id == SOUND_POWER_OFF)
			{
				fptr = fopen("/system/resource/sound/poweroff_audio.pcm", "rb");
			}
			else if(msg_id == SOUND_LDW_ALRAM)
			{
				fptr = fopen("/system/resource/sound/LDW_Alarm.pcm", "rb");
			}			
			while(1) {					
				ret = fread(rbuf, 1, 512, fptr);
				
				
if(upsample) {

				if (ret > 0)
				{
					if (resample)
					{
						data.input_frames_used = 0;
						data.output_frames_gen = 0;
						data.input = (void*)rbuf;
						data.output = (void*)up_buf;

						/* bytes per channel */
						data.input_frames = 512;
						data.output_frames = 512*3;
						data.end_of_input = 0;
						data.volume = 255;
						
						if (gp_resample_process (resample, &data)) {
							printf("[%s:%d]resample fail!!\r\n", __FUNCTION__, __LINE__);
						}
						else {
							write(keysound_handle, up_buf, data.output_frames_gen << 2);
						}
					}
					else
						write(keysound_handle, rbuf, sizeof(rbuf));
				}
				else
					break;
} else {
				if(ret>0)
					write(keysound_handle, rbuf, sizeof(rbuf));
				else
					break;
}			
			}
			ret = ioctl(keysound_handle, SNDCTL_DSP_SYNC, 0);
			fclose(fptr);
			if(msg_id == SOUND_POWER_OFF)
			{
				dv_set.power_off = 1;
			}
		}
	}
	upsample_init = 0;
	if (up_buf)
		free (up_buf);
	
	Dsp_close();
}
void *Key_Thread(void *arg) 
{

	UINT8 msg_id;
	int ret;
	int fd;
	fd_set readfd;
	struct input_event event;
	struct timeval timeout={0, 0};
	time_t t;
	struct tm tm;
	//system("insmod /system/lib/modules/common/Asensor_DA380.ko");
	//sleep(1);
	fd = open("/dev/event0",O_RDONLY);	
	if (fd < 0){
		printf("Can't open [/dev/event0]\n");
		fd = open("/dev/input/event0",O_RDONLY);
		if(fd < 0) {
			while(1)
			{
				printf("Can't open [/dev/input/event0]\n");
				sleep(1);
			}
		}
	}else
	{
		printf(" open [/dev/event0] OK\n");
	}
	timer_start.tv_sec = 0;
	timer_start.tv_usec = 0;
	while(1)
	{
		//when key update post message to main task
		FD_ZERO(&readfd);
		FD_SET(fd, &readfd);
		if( select(fd+1, &readfd, NULL, NULL, NULL) > 0) { /*polling*/
		//if( select(fd+1, &readfd, NULL, NULL, &timeout) > 0) { /*polling*/
				if(FD_ISSET(fd, &readfd)) {
					read(fd, &event, sizeof(struct input_event));
					printf("read key key code=%d,key value=%d\n",event.code,event.value);
					if(dv_set.parking_mode_flag == 1)
						dv_set.parking_mode_flag = 0;
					gettimeofday(&powertime_s,NULL);

					if(power_off == 2)
					{
						if((event.code == KEY_ESC)&&(event.value == 0))
						{
							power_off = 3;
						}
						continue;
						
					}
					if(setting_config_get(SET_SCREEN_SAVER) != 0)
					{
						if(dv_set.backlight_flag == 1)
						{
							gettimeofday(&starttime,NULL);
						}
						else
						{
							if(event.value == 1)
								continue;
							if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
							{
								LCD_Backlight_Set(1);
								dv_set.backlight_flag = 1;
								gettimeofday(&starttime,NULL);
								if(event.code == KEY_ESC)
								{
									gettimeofday(&timer_start,NULL);
									power_off = 1;
								}
								continue;
							}
						}
					}					
					if((event.code == KEY_ESC)&&(event.value == 0))
					{
						if(dv_set.power_off == 2)
						{
							dv_set.power_off = 3;
						}
						else
						{
							if(power_off == 1)
							{
								if((Playmode_flag ==CMD_VIDEO_MODE)&&(dv_set.dv_UI_flag ==DV_FOREGROUND_MODE))
								{
									cur_draw_flag = DV_EXIT_MODE;
									foreground_draw_flag=0;	
									if(setting_config_get(SET_GSENSOR1)!=0)
										setting_config_set(SET_GSENSOR1,0);
									else
										setting_config_set(SET_GSENSOR1,2);
								}	
							}
							timer_start.tv_sec = 0;
							timer_start.tv_usec = 0;
							power_off = 0;
						}
					}
					else if((event.code == KEY_ESC)&&(event.value == 1))
					{
						gettimeofday(&timer_start,NULL);
						power_off = 1;
						continue;
					}

					if((dv_set.dv_UI_flag >= DV_EXIT_MODE)||(dv_set.sd_check >= 3)||(dv_set.ui_show == 0))
					{
						printf("Key no function,please wait\n");
						printf("dv_set.sd_check=%d,dv_set.ui_show=%d,dv_set.dv_UI_flag=%d\n",dv_set.sd_check,dv_set.ui_show,dv_set.dv_UI_flag);
						continue;
					}
					if(dv_set.dv_UI_flag == DV_MENU_MODE && (playback_menu_mode == PLAYBACK_MENU_WAIT_END || playback_menu_mode == PLAYBACK_MENU_WAIT))
					{
						printf("in delete file mode,please wait\n");
						printf("dv_set.dv_UI_flag=%d,playback_menu_mode=%d\n",dv_set.dv_UI_flag,playback_menu_mode);
						continue;
					}
					if((event.code == KEY_UP)&&(event.value == 1))
					{
#if REMAP_MENU_MODE_KEY
						gettimeofday(&timer_start,NULL);

#else
						if(dv_set.dv_ctrl == 0)
							Play_Key_sound(SOUND_KEY);
						ap_send_menu_key_active();
						ap_setting_menu_key_active();
						printf("event.code is MENU key\n");
#endif
					}
#if REMAP_MENU_MODE_KEY
					if((event.code == KEY_UP)&&(event.value == 0)) {

						gettimeofday(&timer_end,NULL);
						if((((timer_end.tv_sec-timer_start.tv_sec)*1000+(timer_end.tv_usec-timer_start.tv_usec)/1000)<1000)||((timer_start.tv_sec == 0)&&(timer_start.tv_usec==0)))
						{
							if(dv_set.dv_ctrl == 0)
								Play_Key_sound(SOUND_KEY);
							ap_send_menu_key_active();
							ap_setting_menu_key_active();
							printf("event.code is MENU key\n");
						}
						else { // L press menu key
							if(dv_set.dv_ctrl == 0)
								Play_Key_sound(SOUND_KEY);
							printf("event.code is Mode Key\n");
							ap_send_mode_key_active();
							if((Playmode_flag ==CMD_PLAYBACK_MODE)&&(dvr_check_video_playing() != 1)) 
							{//playing 

								msg_id = dv_set.dv_UI_flag;
								ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);		
								timer_start.tv_sec = 0;
								timer_start.tv_usec = 0;								
								continue;
							}
							ap_setting_mode_key_active();
						}
						timer_start.tv_sec = 0;
						timer_start.tv_usec = 0;								
					}
#endif
					else if((event.code == KEY_EXIT)&&(event.value == 1))
					{
						if((Playmode_flag>CMD_STILL_MODE)||(dv_set.dv_UI_flag!=DV_FOREGROUND_MODE))
							Play_Key_sound(SOUND_KEY);
						ap_send_enter_key_active();
						ap_setting_enter_key_active();
						printf("event.code is Enter\n");
					}
#if !REMAP_MENU_MODE_KEY
					else if((event.code == KEY_LEFT)&&(event.value == 1))
					{
						if(dv_set.dv_ctrl == 0)
							Play_Key_sound(SOUND_KEY);
						printf("event.code is Mode Key\n");
						ap_send_mode_key_active();
						if((Playmode_flag ==CMD_PLAYBACK_MODE)&&(dvr_check_video_playing() != 1)) 
						{//playing 

							msg_id = dv_set.dv_UI_flag;
							ret = mq_send(menumodeMsgQ, &msg_id, 1, 0);		
							continue;
						}
						ap_setting_mode_key_active();
						
					}		
#endif
					/*else if((event.code == KEY_DOWN)&&(event.value == 1))
					{

						ap_send_down_key_active();
						ap_setting_down_key_active();
						printf("event.code is DOWN key\n");
					}
					else if((event.code == KEY_RIGHT)&&(event.value == 1))
					{
						ap_send_up_key_active();
						ap_setting_up_key_active();
						printf("event.code is UP key\n");
					}					
					else if((event.code == KEY_1) && (event.value == 1)) { //key up long
						
						ap_send_up_L_key_active();
						ap_setting_up_L_key_active();
						printf("event.code is KEY_PLUS_L\n");
					}
					else if((event.code == KEY_2) && (event.value == 1)) { // key down long
						

						printf("event.code is KEY_SUB_L\n");
					}*/
					else if((event.code == KEY_DOWN)&&(event.value == 1))
					{
						if((Playmode_flag !=CMD_VIDEO_MODE)||(dv_set.dv_UI_flag !=DV_FOREGROUND_MODE))
						Play_Key_sound(SOUND_KEY);
						if(Playmode_flag == CMD_USB_MODE)
						{
							continue;
						}
						if((Playmode_flag == CMD_STILL_MODE)&&(dv_set.dv_UI_flag == DV_FOREGROUND_MODE))
						{
							if(dv_set.dc_ctrl != 1)
							{
								if(dv_set.zoom_num != 0)
								{
									dv_set.zoom_num--;
									CVR_Set_PIPE(CMD_SET_ZOOM,dv_set.zoom_num);
									msg_id = dv_set.dv_UI_flag;
									mq_send(menumodeMsgQ, &msg_id, 1, 0);									
								}
								dv_set.zoom_flag = 1;
								printf("dv_set.zoom_flag = %d\n",dv_set.zoom_flag);
							}
						}
						else if(dv_set.dv_ctrl == 1)
						{
							msg_id = DV_config_get(DV_AUDIO);
							DV_config_set(DV_AUDIO,!msg_id);
							cur_draw_flag = DV_EXIT_MODE;
							foreground_draw_flag=0;	
							msg_id = dv_set.dv_UI_flag;
							mq_send(menumodeMsgQ, &msg_id, 1, 0);								
						}
						else
						{
							gettimeofday(&timer_start,NULL);
						}						
						printf("event.code is DOWN key start\n");
					}
					else if((event.code == KEY_RIGHT)&&(event.value == 1))
					{
						if((Playmode_flag !=CMD_VIDEO_MODE)||(dv_set.dv_UI_flag !=DV_FOREGROUND_MODE))
						Play_Key_sound(SOUND_KEY);
						if(Playmode_flag == CMD_USB_MODE)
						{
							continue;
						}
						if((Playmode_flag == CMD_STILL_MODE)&&(dv_set.dv_UI_flag == DV_FOREGROUND_MODE))
						{
							if(dv_set.dc_ctrl != 1)
							{
								if(dv_set.zoom_num != 30)
								{
									dv_set.zoom_num++;
									CVR_Set_PIPE(CMD_SET_ZOOM,dv_set.zoom_num);
									msg_id = dv_set.dv_UI_flag;
									mq_send(menumodeMsgQ, &msg_id, 1, 0);									
								}
								dv_set.zoom_flag = 2;
								printf("dv_set.zoom_flag = %d\n",dv_set.zoom_flag);
							}
						}
						else
						{
							gettimeofday(&timer_start,NULL);
						}
						printf("event.code is UP key start\n");
					}					
					else if((event.code == KEY_DOWN)&&(event.value == 0))
					{

						if(Playmode_flag == CMD_USB_MODE)
						{
							ap_setting_down_key_active();
							continue;
						}
						if((Playmode_flag < CMD_PLAYBACK_MODE)&&(dv_set.dv_UI_flag == DV_FOREGROUND_MODE))
						{
							//printf("zoom num = %d\n",dv_set.zoom_num);						
							dv_set.zoom_flag = 0;
							printf("dv_set.zoom_flag = %d\n",dv_set.zoom_flag);
						}else{
							gettimeofday(&timer_end,NULL);
							if((((timer_end.tv_sec-timer_start.tv_sec)*1000+(timer_end.tv_usec-timer_start.tv_usec)/1000)<1000)||((timer_start.tv_sec == 0)&&(timer_start.tv_usec==0)))
							{
								ap_send_down_key_active();
								ap_setting_down_key_active();
							}
							else
							{
								ap_send_down_L_key_active();
								ap_setting_down_L_key_active();							
							}
							timer_start.tv_sec = 0;
							timer_start.tv_usec = 0;								
						}
					}
					else if((event.code == KEY_RIGHT)&&(event.value == 0))
					{
						if(Playmode_flag == CMD_USB_MODE)
						{
							ap_setting_up_key_active();
							continue;
						}
						if((Playmode_flag < CMD_PLAYBACK_MODE)&&(dv_set.dv_UI_flag == DV_FOREGROUND_MODE))
						{
							//printf("zoom num = %d\n",dv_set.zoom_num);	
							dv_set.zoom_flag = 0;
							printf("dv_set.zoom_flag = %d\n",dv_set.zoom_flag);
						}else{
							gettimeofday(&timer_end,NULL);
							if((((timer_end.tv_sec-timer_start.tv_sec)*1000+(timer_end.tv_usec-timer_start.tv_usec)/1000)<1000)||((timer_start.tv_sec == 0)&&(timer_start.tv_usec==0)))
							{
								ap_send_up_key_active();
								ap_setting_up_key_active();
							}
							else
							{
								ap_send_up_L_key_active();
								ap_setting_up_L_key_active();							
							}
							timer_start.tv_sec = 0;
							timer_start.tv_usec = 0;							
						}
					}					
				}
		}
	//usleep(100);	
	}

	pthread_mutex_destroy(&key_mutex);
//	pthread_detach(pthread_self());
//	pthread_exit(NULL);
	return (void *)0;
}
/****************************************************************************************************
pipe
*******************************************************************************************************/
typedef struct menuCmdPacket_s {	
	UINT32 infoID;						/*ID of Message, Setup or Command*/	
	UINT32 dataSize;					/*Size of variable lenth data*/	
	UINT8  data[0];						/*Variable length data*/
} menuCmdPacket_t;


typedef struct disp1Manager_s {
	UINT32 layer;
	int fdDisp1;
	gp_disp_res_t resolution;

	int fdMem;
	chunk_block_t memBlock;
	gp_bitmap_t fb[2];		// framebuffer
	int fbIndex;
} disp1Manager_t;
#define DISP_FIFO2 "/tmp/.disp-cmdin"
#define DISP_FIFO1 "/tmp/.disp-cmdout"
#define MAX_DATASIZE 64 
#define DISP_BUF_MAX_SIZE_WRITE (sizeof(menuCmdPacket_t) + MAX_DATASIZE)
static int g_DISP_CmdOut = 0;
static int g_DISP_CmdIn = 0;
static unsigned int pCmdBuf[DISP_BUF_MAX_SIZE_WRITE / 4];
static pthread_mutex_t mutex_pipewrite;

disp1manage_t dispmanager;
static int ap_pipe_init(void)
{
	int err = 0;
	printf("pipe create begin\n");
	unlink(DISP_FIFO1);
	if ((mkfifo(DISP_FIFO1, O_CREAT | O_EXCL) < 0) && (errno != EEXIST)) {
	//if ((mkfifo(FIFO1, 0777) < 0) && (errno != EEXIST)) {
		printf("can't create fifo server\n");
		printf("error: %s!\n", strerror(errno));	//wwj add
		return -1;
	}
	unlink(DISP_FIFO2);
	if ((mkfifo(DISP_FIFO2, O_CREAT | O_EXCL) < 0) && (errno != EEXIST)) {
	//if ((mkfifo(FIFO2, 0777) < 0) && (errno != EEXIST)) {
		printf("can't create fifoserver\n");
		printf("error: %s!\n", strerror(errno));	//wwj add
		return -1;
	}
	printf("pipe create end\n");	
	g_DISP_CmdIn = open(DISP_FIFO1, O_RDWR, 0666);
	printf("@@ Open: %s for g_DISP_CmdIn\n", DISP_FIFO1);
	if (g_DISP_CmdIn < 0) {
		printf("Open error\n");
		err = 1;
	}
	g_DISP_CmdOut= open(DISP_FIFO2, O_RDWR, 0666);
	printf("@@ Open: %s for g_DISP_CmdOut\n", DISP_FIFO2);
	if (g_DISP_CmdOut < 0) {
		printf("Open error\n");
		err = 1;
	}
	printf("@@ OK! Init FIFO!\n");
	
	if( err ) {
		if(g_DISP_CmdIn){
			close(g_DISP_CmdIn);
			g_DISP_CmdIn = 0;
		}
		if(g_DISP_CmdOut) {
			close(g_DISP_CmdOut);
			g_DISP_CmdOut = 0;
		}
	}	
	
	return 0;	
}

static int gp_pipemsg_receive(unsigned int *msgId, void **para)
{
	unsigned int dataSize;
	int readSize = 0;
	int writeSize = 0;
	unsigned char *pData = NULL;
	unsigned int cmddata = 0;
	menuCmdPacket_t *pCmdPacket = NULL;

	fd_set fds;
	struct timeval timeout = {0,100*1000}; // timeout 100ms
	int	maxfdp;

	FD_ZERO(&fds);
	FD_SET(g_DISP_CmdIn, &fds);
	
	if(select(g_DISP_CmdIn+1,&fds,NULL,NULL,&timeout)>0)
	{
		pCmdPacket = (menuCmdPacket_t *)pCmdBuf;
		pData = (unsigned char *)(((unsigned char *)pCmdBuf) + sizeof(menuCmdPacket_t));
		dataSize = sizeof(menuCmdPacket_t);

		readSize = read(g_DISP_CmdIn, (unsigned char*)pCmdPacket, dataSize);		
		if (readSize <= 0)
			return readSize;	// no cmd

		if (pCmdPacket->dataSize > 0) 
		{
			readSize = read(g_DISP_CmdIn, pData, pCmdPacket->dataSize);
			if (readSize <= 0)
				printf("cmd %d error, no data\n", pCmdPacket->infoID);
		}
		else
			pData = NULL;
	
		*msgId = pCmdPacket->infoID;

		if(pData != NULL)
		{
			*para = (void *)pData;
		}
	}
	return readSize;
}

void gp_pipemsg_send( unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	unsigned char *pData = NULL;
	menuCmdPacket_t *pCmdPacket = NULL;
	int writesize = sizeof(menuCmdPacket_t) + len;
	int ret = 0;
	
	pthread_mutex_lock(&mutex_pipewrite);
	memset(pCmdBuf, 0, sizeof(pCmdBuf));

	if (len > (sizeof(pCmdBuf)-sizeof(menuCmdPacket_t)))
	{
		printf("datasize too large\n");
		len = (sizeof(pCmdBuf)-sizeof(menuCmdPacket_t));
	}	

	pData = (unsigned char *)pCmdBuf;
	pCmdPacket = (menuCmdPacket_t *)pData;
	pCmdPacket->infoID = msgId;
	pCmdPacket->dataSize = len;
	memcpy(pData + sizeof(menuCmdPacket_t), cmddata, len);
	ret = write(g_DISP_CmdOut, pData, writesize);
	pthread_mutex_unlock(&mutex_pipewrite);
}

void gp_pipemsg_send1( unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	unsigned char *pData = NULL;
	menuCmdPacket_t *pCmdPacket = NULL;
	int writesize = sizeof(menuCmdPacket_t) + len;
	int ret = 0;
	pthread_mutex_lock(&mutex_pipewrite);
	memset(pCmdBuf, 0, sizeof(pCmdBuf));
	if (len > (sizeof(pCmdBuf)-sizeof(menuCmdPacket_t)))
	{
		printf("datasize too large\n");
		len = (sizeof(pCmdBuf)-sizeof(menuCmdPacket_t));
	}	
	pData = (unsigned char *)pCmdBuf;
	pCmdPacket = (menuCmdPacket_t *)pData;
	pCmdPacket->infoID = msgId;
	pCmdPacket->dataSize = len;
	memcpy(pData + sizeof(menuCmdPacket_t), cmddata, len);
	ret = write(g_DISP_CmdIn, pData, writesize);
	pthread_mutex_unlock(&mutex_pipewrite);
}

void disp_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	return gp_pipemsg_send(msgId, len, cmddata);
}
void disp_pipemsg_send1(unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	return gp_pipemsg_send1(msgId, len, cmddata);
}
int test = 0;
struct timeval test_s, test_e;
int g_flipUI = 0;
void *Disp_Thread(void *arg)
{
	unsigned int msgId;
	void* msgPara;
	unsigned int mode;
	UINT32 *addr;
	dispmanager.buffer_num = 0;
	ap_pipe_init();
	if (disp1Create(&hDisp1, DISP_LAYER_PRIMARY) != SP_OK) {
			printf("dispCreate error\n");
			return -1;
	}
	disp1Manager_t *pDisp1= (disp1Manager_t *)hDisp1;
	if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
	{
		hDisp = hDisp1;
	}
	dispmanager.addr = gpChunkMemAlloc((pDisp1->resolution.width)*(pDisp1->resolution.height)*2*3);
	printf("@@ dispmanager.addr = 0x%x!\n",dispmanager.addr );
	dispmanager.buffer_width = pDisp1->resolution.width;
	dispmanager.buffer_height = pDisp1->resolution.height;
	dispmanager.buffer_num = 3;	
	ap_ppu_init(hDisp1);
	ap_ppu_text_load(hDisp1);	
	
	while(1)
	{
		//printf("@@ Disp_Thread!\n");
		if(gp_pipemsg_receive(&msgId,&msgPara) <= 0) {
			/*printf("%s:%d wait disp msg\n", __FUNCTION__, __LINE__);
			if(g_flipUI) {
				g_flipUI = 0;
				disp1FlipUI(hDisp1);
			}*/
			continue;
		}
		//printf("*************************msgId = %d*****************************\n",msgId);
		mode = *(unsigned int*)msgPara;
		if(msgId == DISP_BUF_READY)
		{
			if(dv_set.change_disp == 1)
			{
				printf("@@@DiSP change,free buffer\n");
				disp_pipemsg_send(DISP_BUF_READY,4, &mode);
				continue;
			}			
			addr = dispmanager.addr+ (pDisp1->resolution.width)*(pDisp1->resolution.height)*2*mode/4;			
			//printf("pDisp1->resolution.width= %d,pDisp1->resolution.height=%d,mode = %d,addr = 0x%x\n",pDisp1->resolution.width,pDisp1->resolution.height,mode,addr);
			//dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
			//UINT8 msg_id = dv_set.dv_UI_flag;
			//mq_send(menumodeMsgQ, &msg_id, 1, 0);	
			disp1Flip(hDisp1,addr);
			disp_pipemsg_send(DISP_BUF_READY,4, &mode);
		}
		else if(msgId == DISP_BUF_READ_UI) { //only update UI layer
			//printf("@@@DiSP change UI buffer\n");
			disp1FlipUI(hDisp1);
		}
		else
		{
			printf("*************************DISP_BUF_FREE*****************************\n");
			disp1Flip(hDisp1,0);
		}
	}
}

int g_bQuit = 0;
void sigint( int signo )
{
	Dsp_close();
	g_bQuit = 1;
}

int RegisterSigint()
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sigint;
	act.sa_flags   = SA_RESTART;
	if(sigaction(SIGINT, &act, NULL) != 0)
	{
		return -1;
	}

	return 1;
}
int pllhd = 0;
int
main(
	int argc,
	char **argv
)
{

	pllhd = PLL_ctl_reqest();
	PLL_ctl_en(pllhd, 0, 0);
	PLL_ctl_en(pllhd, 1, 0);

	LCD_Backlight_Set(0);
	
	RegisterSigint();
	printf("===========Enter CVR============\n");
	printf("BUILD TIME: "__DATE__"-"__TIME__"\r\n" );
	mq_unlink("/menumodeMsgQ");
	menumodeMsgQ = mq_open("/menumodeMsgQ", O_CREAT | O_RDWR /*| O_NONBLOCK*/, S_IREAD | S_IWRITE, &menumode_mq_attr);
	if (menumodeMsgQ== (mqd_t) -1) {
		perror("menumodeMsgQ msg queue open fail.\n");
		return -1;
	}
	
	mq_unlink("/KeySoundMsgQ");
	KeySoundMsgQ = mq_open("/KeySoundMsgQ", O_CREAT | O_RDWR /*| O_NONBLOCK*/, S_IREAD | S_IWRITE, &keysound_mq_attr);
	if (KeySoundMsgQ== (mqd_t) -1) {
		perror("KeySoundMsgQ msg queue open fail.\n");
		return -1;
	}

	Dsp_set_Vol(128);
	
	pthread_mutex_init(&key_mutex, NULL);
	board_config_set_init(0x01);
	ap_state_resource_init();
	test_config_set();	
	LED_Set(NORMAL_LED,1);

	gettimeofday(&powertime_s,NULL);
	Playmode_flag = 0xff;
	if(TimerThread == 0)
	pthread_create(&TimerThread, NULL, Timer_Thread, 0);
	if(Timer2Thread == 0)
	pthread_create(&Timer2Thread, NULL, Timer2_Thread, 0);
#if LDW_LINEDISP	
	if(TimerThread_Ldwline == 0)
	pthread_create(&TimerThread_Ldwline, NULL, Timer_Thread_LDW_Line, 0);
#endif		
	if(KeyThread == 0)
	pthread_create(&KeyThread, NULL, Key_Thread, 0);
	if(KeySoundThread == 0)
	pthread_create(&KeySoundThread, NULL, KeySound_Thread, 0);

	if(dv_set.display_mode != SP_DISP_OUTPUT_LCD)
	{
		if (dispCreate(&hDisp, DISP_LAYER_OSD,dv_set.display_mode) != SP_OK) {
				printf("dispCreate error\n");
				return -1;
		}
	}
	sleep(2);
	CheckMemoryPath();
	if(dispThread == 0)
	pthread_create(&dispThread, NULL, Disp_Thread, 0);
		
	if(setting_config_get(SET_GSENSOR1) !=0)
	{
		dv_set.parking_mode_flag = Gsensor_parking_mode_int_read();
		printf("Parking mode int =%d\n",dv_set.parking_mode_flag);
	}	
	if((dv_set.sd_check == 1)&&(USB_Check()==1)&&(dv_set.parking_mode_flag == 0) )
	{
		dv_set.upgrade_flag = SPI_Flash_update();	
	}
	else
	{
		dv_set.upgrade_flag = 0;	//0 update fail, 1,update OK, 2:get path fail.
	}
	gettimeofday(&starttime,NULL);
	printf("dv_set.upgrade_flag = %d\n",dv_set.upgrade_flag);
	if(dv_set.upgrade_flag != 1)
	{
		Playmode_flag = Get_play_mode();	
		//Playmode_flag = CMD_PLAYBACK_MODE;	
		//load Global_User_Optins ,means read defalt info from memory
		printf("play mode = %d\n",Playmode_flag);
		board_config_set_init(0x02);
		Enable_Sensor_Power(1);
		Enable_Sensor_Reset(0);		
		LED_Set(IR_LED,setting_config_get(SET_IR_LED));
	}
	LED_Set(MUTE_ON,1);


	if(DV_config_get(DV_GSENSOR) !=0)
	{
		Gsensor_write(DV_config_get(DV_GSENSOR));
	}
	if(dv_set.display_mode != SP_DISP_OUTPUT_LCD) {
		Speaker_set_power(0);
		LCD_Backlight_Set(0);
	}
	else {
		Speaker_set_power(1);
		LCD_Backlight_Set(1);
	}
	if((dv_set.parking_mode_flag == 1)&&(dv_set.sd_check == 0))
	{
		Power_off();
	}
	while(dispmanager.buffer_num != 3);
	while(1)
	{
		switch(Playmode_flag)
		{
			case CMD_VIDEO_MODE:
				DV_task_entry();
			break;
			case CMD_STILL_MODE:
				DC_task_entry();
			break;
			
			case CMD_PLAYBACK_MODE:
				DV_idle();
				PLL_ctl_en(pllhd, 1, 1);
				PLAYBACK_task_entry();
				PLL_ctl_en(pllhd, 1, 0);
			
			break;
			case CMD_USB_MODE:
				DV_power_off();
				USB_task_entry();
			break;
			case CMD_POWEROFF_MODE:
				DV_power_off();
				PLL_ctl_en(pllhd, 1, 1);
				PowerOff_entry();
			break;			
			default:
			break;
		}
		usleep(100);
		if(g_bQuit) {
			break;
		}
	}
	if(TimerThread) {		
		pthread_join(TimerThread, NULL);		
		TimerThread = 0;	
	}	
	if(Timer2Thread) {		
		pthread_join(Timer2Thread, NULL);		
		Timer2Thread = 0;	
	}		
#if LDW_LINEDISP	
	if(TimerThread_Ldwline) {		
		pthread_join(TimerThread_Ldwline, NULL);		
		TimerThread_Ldwline = 0;	
	}	
#endif	
	if(KeyThread) {		
		pthread_join(KeyThread, NULL);		
		KeyThread = 0;	
	}	
	if(KeySoundThread) {
		pthread_join(KeySoundThread, NULL);		
		KeySoundThread = 0;			
	}
	
	dispDestroy(hDisp);
	printf("%s:%d car_record exit\n", __FUNCTION__, __LINE__);
	
	//save Global_User_Optins ,means write defalt info to memory
	
	return 0;
}
