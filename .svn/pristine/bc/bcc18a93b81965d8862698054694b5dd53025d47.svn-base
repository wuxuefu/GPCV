#include "ap_state_config.h"
#include <linux/types.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "public_pipe.h"
#include "cvr_pipe.h"

#define LDW_TEST 1

extern struct tm DV_start_time;
extern dv_set_t dv_set;
extern UINT8 Memory_path[64];
extern mqd_t menumodeMsgQ;
extern mqd_t KeySoundMsgQ;
extern void Play_Key_sound(UINT8 sound_type);
extern UINT32 time_num;
extern struct timeval cap_start,cap_end;
LDW_ITEMS LDW_options;

extern LDW_DISPLINE_t ldw_displine;
extern int gb_disp_LDWlineDisp;
extern disp1manage_t dispmanager;


void CVR_receive_cmd_callback(publicPlayerCmdPacket_t *pCmdPacket, UINT8 *pData, pthread_mutex_t *mutex_resp)
{
	publicResponse_t *w ;
	UINT8 msg_id;
	SINT32 ret;
	time_t t;
	static int first_run  = 1;
//	printf("@@  pCmdPacket->infoID %d\n",pCmdPacket->infoID);
	if((DV_CMD_MIN < pCmdPacket->infoID) && (pCmdPacket->infoID < DV_CMD_MAX)){
		if(pCmdPacket->infoID == CMD_SET_MD_DV_START)
		{
			if(dv_set.upgrade_flag != 2)
			{
				dv_set.motiondetect_start = 1;
				printf("callback:DV Motion detect start DV !\n"); 
			}
		}
		else if(pCmdPacket->infoID == CMD_SET_MD_DV_STOP)
		{
			dv_set.motiondetect_end = 1;
			printf("callback:DV Motion detect stop DV !\n"); 
		}
		else if(pCmdPacket->infoID == CMD_SET_LDW_LOW_SPEED)
		{
			dv_set.LDW_speed_flag = 0;
			#if LDW_TEST
			if(ap_state_config_LDW_Sound_get()==1)
			{
				msg_id = SOUND_KEY;
				mq_send(KeySoundMsgQ, &msg_id, 1, 0);
				mq_send(KeySoundMsgQ, &msg_id, 1, 0);
			}
			#endif
			printf("callback:LDW LOW SPEED !\n"); 
		}	
		else if(pCmdPacket->infoID == CMD_SET_LDW_HIGH_SPEED)
		{
			dv_set.LDW_speed_flag = 1;
			#if LDW_TEST
			if(ap_state_config_LDW_Sound_get()==1)
			{
				msg_id = SOUND_KEY;
				mq_send(KeySoundMsgQ, &msg_id, 1, 0);
				mq_send(KeySoundMsgQ, &msg_id, 1, 0);
				mq_send(KeySoundMsgQ, &msg_id, 1, 0);
				mq_send(KeySoundMsgQ, &msg_id, 1, 0);
			}
			#endif
			printf("callback:LDW HIGH SPEED !\n"); 
		}
		else if(pCmdPacket->infoID == CMD_SET_LDW_INT)
		{
			//dv_set.LDW_INT_flag = 1;
			msg_id = SOUND_LDW_ALRAM;
			mq_send(KeySoundMsgQ, &msg_id, 1, 0);		
			printf("callback:LDW INT !\n"); 
		}		
		else if(pCmdPacket->infoID == CMD_SET_SDC_ERROR)
		{
			dv_set.sdc_error = 1;
			time_num = 2;
			printf("callback:SDC Error!\n"); 
		}
		else if(pCmdPacket->infoID == CMD_SET_LOOPRECORDING_INT)
		{			
			dv_set.lock_flag = pCmdPacket->data[0];
			time(&t);
			DV_start_time= *localtime(&t);	
			printf("CMD_SET_LOOPRECORDING_INT! locknext=%d \n",dv_set.lock_flag); 
		}
		else if(pCmdPacket->infoID == CMD_DO_CAPTURE)
		{
			dv_set.ui_show = 1;
			printf("CMD_DO_CAPTURE Done,show UI!\n"); 
			msg_id = dv_set.dv_UI_flag; 
			mq_send(menumodeMsgQ, &msg_id, 1, 0);	
		}
		else if(pCmdPacket->infoID == CMD_SET_SEQUENCE_INT)
		{
			Play_Key_sound(SOUND_CAMERA);	
		}
		else if(pCmdPacket->infoID == CMD_SET_LDW_LINE2DISP)
		{
			//dv_set.ui_show = 1;
			gb_disp_LDWlineDisp=1;	
			//	printf("CMD_SET_LDW_LINE2DISP,show UI! %d\n",pCmdPacket->dataSize); 
			ldw_displine.LT_alarmP_X = pCmdPacket->data[0]*256+pCmdPacket->data[1];
			ldw_displine.LT_alarmP_Y = pCmdPacket->data[2]*256+pCmdPacket->data[3];
			ldw_displine.LB_alarmP_X = pCmdPacket->data[4]*256+pCmdPacket->data[5];
			ldw_displine.LB_alarmP_Y = pCmdPacket->data[6]*256+pCmdPacket->data[7];
			
			ldw_displine.RT_alarmP_X = pCmdPacket->data[8]*256+pCmdPacket->data[9];
			ldw_displine.RT_alarmP_Y = pCmdPacket->data[10]*256+pCmdPacket->data[11];
			ldw_displine.RB_alarmP_X = pCmdPacket->data[12]*256+pCmdPacket->data[13];
			ldw_displine.RB_alarmP_Y = pCmdPacket->data[14]*256+pCmdPacket->data[15];
			

		
			ldw_displine.LTP_X = pCmdPacket->data[16]*256+pCmdPacket->data[17];
			ldw_displine.LTP_Y = pCmdPacket->data[18]*256+pCmdPacket->data[19];
			ldw_displine.LBP_X = pCmdPacket->data[20]*256+pCmdPacket->data[21];
			ldw_displine.LBP_Y = pCmdPacket->data[22]*256+pCmdPacket->data[23];

			ldw_displine.RTP_X = pCmdPacket->data[24]*256+pCmdPacket->data[25];
			ldw_displine.RTP_Y = pCmdPacket->data[26]*256+pCmdPacket->data[27];
			ldw_displine.RBP_X = pCmdPacket->data[28]*256+pCmdPacket->data[29];
			ldw_displine.RBP_Y = pCmdPacket->data[30]*256+pCmdPacket->data[31];

			ldw_displine.LLAlarmFlg = pCmdPacket->data[32];
			ldw_displine.LLcheckFlg = pCmdPacket->data[33];
			ldw_displine.RLAlarmFlg = pCmdPacket->data[34];
			ldw_displine.RLcheckFlg = pCmdPacket->data[35];	
			
			msg_id = dv_set.dv_UI_flag; 
			mq_send(menumodeMsgQ, &msg_id, 1, 0);		
		}
		else if(pCmdPacket->infoID == CMD_READY_KEY)
		{
			printf("get ready key======================\n");
			
			if((dv_set.sd_check == 1)&&(first_run == 1))
			{
				first_run = 0;
				gettimeofday(&cap_start,NULL);
				do
				{
					ret = CheckMemoryPath(); //get memory path
					if(ret != 0)
					{
						gettimeofday(&cap_end,NULL);
						if(((cap_end.tv_sec-cap_start.tv_sec)*1000+(cap_end.tv_usec-cap_start.tv_usec)/1000)>5000)
						{
							ret = 0;
							dv_set.sdc_error= 1;
							time_num = 2;
						}					
					}
					if((ret == 0)&&(dv_set.sdc_error != 1))
					{
						DV_set_dir(Memory_path);
						if((SDC_FreeSize()<=5*1024)&&(DV_config_get(DV_LOOPRECORDING)==0))
						dv_set.sdc_full = 1;
						else
						{
							dv_set.sdc_full = 0;
							check_cluster_size();
							printf("dv_set.upgrade_flag = %d\n",dv_set.upgrade_flag);
							if(((dv_set.usb_detect == 1&&dv_set.upgrade_flag != 2)||(dv_set.parking_mode_flag == 1))&&(dv_set.sdc_error != 1))
							{
								if(dv_set.display_mode != SP_DISP_OUTPUT_HDMI)
								{
									usleep(500000);
									CVR_Set_PIPE_With_CMD(CMD_START_DV,0);
									time(&t);
									DV_start_time= *localtime(&t);	
									gettimeofday(&cap_start,NULL);
									dv_set.dv_ctrl =1;
									printf("First start DV!\n"); 
								}
							}							
						}
											
					}
					usleep(10000);					
				}
				while(ret);
			}
			dv_set.dv_UI_flag = DV_EXIT_MODE + 1;
		}
		
		w = public_isInlist(pCmdPacket->infoID);
		if(w != NULL){
			pthread_mutex_lock(mutex_resp);
			w->result = RESPONSE_OK;
			pthread_mutex_unlock(mutex_resp);
		}
	}

		
}

pid_t dvr_pid = -1;

static int check_process_exist(int pid, const char* name)
{
	char path[64];
	FILE* fd;

	sprintf(path, "/proc/%d/cmdline", pid);	
	fd = fopen(path, "rb");

	if (!fd)
		return 0;
	else
	{
		fread(path, 1, strlen(name), fd);
		fclose(fd);
		path[strlen(name)] = 0;
		if (strcmp(path, name))
			return 0;
	}

	return 1;
}

static int dv_parameter(char** argv)
{
	int i, idx = 0;
	int fps = 30;
	int bitrate;
	int time_lapse_value;
	int real60fps = 0;
	
	argv[idx++] = strdup("-dv");		
	argv[idx++] = strdup("-s"); 		//add a stream
	argv[idx++] = strdup("1"); 		//enable record to file
	
	i = DV_config_get(DV_RESOLUTION);
	
	switch(i)
	{
	default:
	case 0:	i = 0;	bitrate = 12000;	break; // 1080p
	case 5: real60fps = 1;//720p 60fps
	case 1: fps *=2;
	case 2:	i = 1;	bitrate = 6500;		break;	//720p
	case 3: i = 2;	bitrate = 3200;		break; // WVGA
	case 4: i = 3;	bitrate = 3200;		break; // VGA
	}
	
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);

	argv[idx] = strdup("12000");	//bitrate in bps
	sprintf(argv[idx++], "%d", bitrate);
	
	argv[idx] = strdup("30");		//frame rate	
	sprintf(argv[idx++], "%d", fps);
	
	if (i == 1 && real60fps)
		argv[idx++] = strdup("-60fps");

	if(1)
	{
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
	else
	{
		argv[idx++] = strdup("-d");		//enable display
	}
	//argv[idx++] = strdup("-scale");	//set scaler quality
	//argv[idx++] = strdup("1");		//1 = bilnear (faster)
	argv[idx++] = strdup("-fifo");	//enable fifo command

	argv[idx++] = strdup("-loop");	//enable loop recording
	
	i = DV_config_get(DV_LOOPRECORDING);
	
	switch(i)
	{
	default:
	case 0:	i = 1440;	break; // 24 hour
	case 1:	i = 2;		break;
	case 2:	i = 3;		break;
	case 3: i = 5;		break;
	}
	
	argv[idx] = strdup("1440");
	sprintf(argv[idx++], "%d", i);	//loop recroding time in minutes

	argv[idx++] = strdup("-mov");	//use mov format
	
	i = DV_config_get(DV_AUDIO);
	if (!i)
		argv[idx++] = strdup("-noaudio");
		
	i = DV_config_get(DV_DATESTAMP);
	if (i)
		argv[idx++] = strdup("-timestamp");
		
	i = setting_config_get(SET_DATA_FORMAT);
	argv[idx++] = strdup("-timeformat");
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);
	
	i = DV_config_get(DV_EXPOSURE);
	argv[idx++] = strdup("-ev");
	argv[idx] = strdup("06");
	sprintf(argv[idx++], "%d", i);
	
	i = DV_config_get(DV_LDW);
	if (i > 0)
	{
		int sen = ap_state_config_LDW_Sensitivity_get();
		if (sen < 2)
			sen = (sen ? 0:1);
		argv[idx++] = strdup("-ldw");
		argv[idx] = strdup("0");
		sprintf(argv[idx++], "%d", ap_state_config_LDW_Car_type_get()+1);
		argv[idx] = strdup("0");
		sprintf(argv[idx++], "%d", sen);
		argv[idx] = strdup("0");
		sprintf(argv[idx++], "%d", ap_state_config_LDW_Area_Choice_get());
		argv[idx] = strdup("0");
		sprintf(argv[idx++], "%d", ap_state_config_LDW_SPEED_get());
	}
	//get DV_TIMELAPSE && set time_lapse_value	
	i = DV_config_get(DV_TIMELAPSE);	
	argv[idx++] = strdup("-timelapse");
	argv[idx] = strdup("0");
	if(i)	{	
	if(i==1)	
	{			
		time_lapse_value = 100;
		}else if(i==2)	
		{			
			time_lapse_value = 200;		
		}else if(i==3)		
		{			
			time_lapse_value = 500;		
		}else	
		{
			time_lapse_value = 33;//30		
		}
		sprintf(argv[idx++], "%d",1);	
		argv[idx] = strdup("0");
		sprintf(argv[idx++], "%d",time_lapse_value);
	}else	
	{
		sprintf(argv[idx++], "%d",0);	
		argv[idx] = strdup("0");	
		sprintf(argv[idx++], "%d",0);	
	}
	
	i = setting_config_get(SET_FREQUENCY);
	if (i == 1)
	{
		argv[idx++] = strdup("-freq");
		argv[idx++] = strdup("1");
	}
	
	if(dv_set.sd_check == 1)
	{			

		i = DV_config_get(DV_MOTIONDETECTION);
		if (i)
			argv[idx++] = strdup("-md");
		
		//argv[idx++] = strdup("-o");		//set output file name
		//argv[idx++] = strdup("/media/sdcarda/gprecord.mov");
		//argv[idx] = malloc(128);
		//sprintf(argv[idx++], "%s/GP.mov", Memory_path);
	}

		
	return idx;
}

static int dc_parameter(char** argv)
{
	int i, idx = 0;
		
	argv[idx++] = strdup("-dc");

	argv[idx++] = strdup("-rel");
	i= DC_config_get(DC_RESOLUTION);
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);

	argv[idx++] = strdup("-d2");
	argv[idx] = strdup("0x00000000");
	sprintf(argv[idx++], "0x%08x", gpChunkMemVA2PA(dispmanager.addr)); //physical address
	argv[idx] = strdup("320"); //width
	sprintf(argv[idx++], "%d", dispmanager.buffer_width);
	argv[idx] = strdup("240"); //height
	sprintf(argv[idx++], "%d", dispmanager.buffer_height);
	argv[idx] = strdup("3"); //buffer number	
	sprintf(argv[idx++], "%d", dispmanager.buffer_num);
	
	argv[idx++] = strdup("-q");
	i= DC_config_get(DC_QUALITY);
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);
	
	argv[idx++] = strdup("-s");
	i= DC_config_get(DC_SHARPNESS);
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);
		
	argv[idx++] = strdup("-w");
	i= DC_config_get(DC_WHITE_BALANCE);
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);
	
	argv[idx++] = strdup("-c");
	i= DC_config_get(DC_COLOR);
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);
	
	argv[idx++] = strdup("-ev");
	i= DC_config_get(DC_EXPOSURE);
	argv[idx] = strdup("06");
	sprintf(argv[idx++], "%d", i);
	
	argv[idx++] = strdup("-iso");
	i= DC_config_get(DC_ISO);
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);
	
	i = DC_config_get(DC_DATE_STAMP);
	if (i)
		argv[idx++] = strdup("-timestamp");
	if (i == 1)
		argv[idx++] = strdup("1");
	else if (i == 2)
		argv[idx++] = strdup("2");
		
	i = setting_config_get(SET_DATA_FORMAT);
	argv[idx++] = strdup("-timeformat");
	argv[idx] = strdup("0");
	sprintf(argv[idx++], "%d", i);
		
	i = DC_config_get(DC_SEQUENCE);
	if (i)
	{
		argv[idx++] = strdup("-seq");
		argv[idx++] = strdup("3");
	}

	if(dv_set.sd_check == 1)
	{				
		//argv[idx++] = strdup("-o");		//set output file path
		//argv[idx++] = strdup(Memory_path);
	}
			
	return idx;
}
void DV_pipe_init(void)
{
	int i, idx = 0;
	char *argv[64];
	int fps = 30;
	int bitrate;
	
	public_pipeinit(CVR_receive_cmd_callback);

	if (dvr_pid > 0 && !check_process_exist(dvr_pid, "camcorder"))
		dvr_pid = -1;
	
	if (dvr_pid <= 0)
	{
		argv[idx++] = strdup("/system/app/camcorder");
		argv[idx++] = strdup("camcorder");

		idx += dv_parameter(&(argv[idx]));	
		idx += dc_parameter(&(argv[idx]));	

		argv[idx++] = (char *)0;
		signal(SIGCHLD, SIG_IGN);
		if((dvr_pid = fork()) <0)
		{
			perror("creat pid fail\n");
		}
		else if(dvr_pid==0)
		{
			execv(argv[0], &argv[1]);
			for(i = 0; i < idx; i++) {
				free(argv[i]);
				argv[i] = NULL;
			}	
		}		
		if (dvr_pid < 0)
			return;
	}
	else
	{
		kill(dvr_pid, SIGUSR1);
		kill(dvr_pid, SIGUSR1);
		kill(dvr_pid, SIGUSR1);
	}
	usleep(100);	

}

void DC_pipe_init(void)
{
	int i, idx=0;
	char *argv[32];
	
	public_pipeinit(CVR_receive_cmd_callback);
	
	if (dvr_pid && !check_process_exist(dvr_pid, "camcorder"))
		dvr_pid = -1;
		
	if (dvr_pid <= 0)
	{
		argv[idx++] = strdup("/system/app/camcorder");
		argv[idx++] = strdup("camcorder");

		idx += dc_parameter(&(argv[idx]));	
		idx += dv_parameter(&(argv[idx]));	
		
		argv[idx++] = (char *)0;

		signal(SIGCHLD, SIG_IGN);
		if((dvr_pid = fork()) <0)
		{
			perror("creat pid fail\n");
		}
		else if(dvr_pid==0)
		{
			execv(argv[0], &argv[1]);
			for(i = 0; i < idx; i++) {
				free(argv[i]);
				argv[i] = NULL;
			}	
		}			

		if (dvr_pid < 0)
			return;
	}
	else
	{
		kill(dvr_pid, SIGUSR2);
		kill(dvr_pid, SIGUSR2);
		kill(dvr_pid, SIGUSR2);
	}
	usleep(100);

}

int DV_check_exist()
{
	if (dvr_pid > 0 && !check_process_exist(dvr_pid, "camcorder"))
		dvr_pid = -1;
		
	return (dvr_pid > 0) ? 1: 0;
}

void DV_idle()
{
	if (dvr_pid)
	{
		kill(dvr_pid, SIGTERM);
		kill(dvr_pid, SIGTERM);
		kill(dvr_pid, SIGTERM);
	}
}

void DV_power_off()
{
	printf("DV_power_off\n");
	if (dvr_pid)
	{
		if (kill(dvr_pid, SIGINT) == 0)
		{
			while(check_process_exist(dvr_pid, "camcorder"))
			{
					sleep(1);
					printf("camcorder have exist\n");
			}
		}
	}
}

void CVR_Set_PIPE_With_CMD(UINT32 CMD,UINT8 wait)
{
	UINT32 datasize;
	publicPlayerCmdPacket_t *packet;
	//printf("CVR_Set_PIPE_With_CMD  CMD=[%d]\n", CMD);
	datasize = sizeof(publicPlayerCmdPacket_t);
	packet =(publicPlayerCmdPacket_t *) malloc(datasize*sizeof(char));
	packet->infoID = CMD;
	packet->dataSize =0;
	while(dv_set.pipe_lock == 0)
	{
		usleep(10);
		//printf("CVR_CMD is sending,wait!\n ");
	}
	dv_set.pipe_lock = 0;
	sendPacket(packet,wait);
	dv_set.pipe_lock = 1;
	free(packet);	
}

void CVR_Set_PIPE(UINT32 CMD,int mode)
{
	UINT32 datasize;
	publicPlayerCmdPacket_t *packet;
	printf("CVR_Set_PIPE CMD=[%d],mode= [%d]\n", CMD,mode);
	
	datasize = sizeof(publicPlayerCmdPacket_t);
	packet =(publicPlayerCmdPacket_t *) malloc(datasize*sizeof(char)+sizeof(int));
	packet->infoID = CMD;
	packet->dataSize =sizeof(unsigned int);
	*(UINT32 *)((char *)packet + sizeof(publicPlayerCmdPacket_t) + 0) = mode;
	while(dv_set.pipe_lock == 0)
	{
		usleep(10);
		//printf("CVR_CMD is sending,wait!\n ");
	}
	dv_set.pipe_lock = 0;
	if(dv_set.date_type_flag == 0)
	sendPacket(packet, 1);
	else
	sendPacket(packet, 0);	
	dv_set.pipe_lock = 1;	
	free(packet);		
}
void DV_set_dir(char *dir)
{
	UINT32 datasize;
	publicPlayerCmdPacket_t *packet;
	printf("@@@@@@@@ Set File Dir [%s]\n", dir);
	datasize = sizeof(publicPlayerCmdPacket_t)+strlen(dir)+1;
	packet =(publicPlayerCmdPacket_t *) malloc(datasize*sizeof(char));
	packet->infoID = CMD_SET_DIR;
	packet->dataSize = strlen(dir)+1;
	strcpy((char *)((char *)packet + sizeof(publicPlayerCmdPacket_t) + 0), dir);
	printf("@@@@@@@@ Set File Dir [%s]\n", (char *)((char *)packet + sizeof(publicPlayerCmdPacket_t) + 0));
	while(dv_set.pipe_lock == 0)
	{
		usleep(10);
		//printf("CVR_CMD is sending,wait!\n ");
	}
	dv_set.pipe_lock = 0;		
	sendPacket(packet, 0);
	dv_set.pipe_lock = 1;	
	free(packet);
}
void CVR_SET_LDW(void)
{
	UINT32 datasize;
	publicPlayerCmdPacket_t *packet;

	datasize = sizeof(publicPlayerCmdPacket_t)+sizeof((LDW_ITEMS)LDW_options);
	packet =(publicPlayerCmdPacket_t *) malloc(datasize*sizeof(char));
	packet->infoID = CMD_SET_LDW_EN;
	packet->dataSize = sizeof((LDW_ITEMS)LDW_options);
	if(DV_config_get(DV_LDW)==0)
	LDW_options.LDW_EN = 0;
	else 
	{
		LDW_options.LDW_EN = ap_state_config_LDW_Car_type_get()+1;
	}
	LDW_options.LDW_Sensitivity = ap_state_config_LDW_Sensitivity_get();
	LDW_options.LDW_Area_Choice = ap_state_config_LDW_Area_Choice_get();
	LDW_options.LDW_Speed = ap_state_config_LDW_SPEED_get();
	printf("LDW_options.EN = %d\n",LDW_options.LDW_EN);
	printf("LDW_options.LDW_Sensitivity = %d\n",LDW_options.LDW_Sensitivity);
	printf("LDW_options.LDW_Area_Choice = %d\n",LDW_options.LDW_Area_Choice);
	printf("LDW_options.LDW_Speed = %d\n",LDW_options.LDW_Speed);
	memcpy((UINT8*)packet + sizeof(publicPlayerCmdPacket_t), &LDW_options, sizeof(LDW_ITEMS));
	while(dv_set.pipe_lock == 0)
	{
		usleep(10);
		//printf("CVR_CMD is sending,wait!\n ");
	}
	dv_set.pipe_lock = 0;		
	sendPacket(packet, 0);
	dv_set.pipe_lock = 1;	
	free(packet);	
}

