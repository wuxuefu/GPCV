/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file main.c
 * @brief main function
 * @author Anson Chuang
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>

#include <chunkmem.h>
#include "mach/typedef.h"

#include "stream.h"
#include "gp_video_channel.h"
#include "gp_mux.h"
#include "dvr_pipe.h"
#include "csi_md.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct option_s {
	int cType;
	int display;
	int scale_type;
	int rgb_mode;
	int motion_detect;
	int vstb_offset;
	char outfile[256];
	int fifo;
	int loop_recording;
	int loop_file_time;
	int disable_audio;
	int time_stamp;
	int ev;
} option_t;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
//extern struct list_head gStreamList;
extern int changeFileNotify;
extern int LockFileNotify;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static void *VideoTask(void *param);
static void *AudioTask(void *param);


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
#define usage \
"\nUsage: %s [-s save dimension bitrate framerate] [-s ...] [-avi][-a type][-d][-scale][-rgb]\n \
\n \
-s:    add a stream\n \
       save:       0:no save to file\n \
                   1:save to file\n \
                   2:both\n \
       dimension:  0:1080p \n \
                   1:720p \n \
                   2:VGA \n \
                   3:QVGA\n \
       bitrate:    in kbps\n \
       framerate:  frame per second\n \
-d		display to panel \n \
-scale set scaler type \n \
		0: integration mode\n \
		1: bilinear mode (faster)\n \
-vstb n	enable video stabilization with (w+n)*(h+n) buffer \n \
-md		enable motion detection\n"


//"Usage : %s SensorType HD_EN bitrate VGA_EN bitrate QVGA_EN bitrate\n \
//\tSensorType 0:720P 1:1080P\n \
//\t*_EN 0: disable 1: stream to net 2: save to file\n"
int g_bQuit = 0;
int g_bPause = 0;
int g_bMotion = 0;
pthread_t audio_pthread;
pthread_mutex_t audio_buf_mutex;
void* audio_handle = NULL;
struct timeval md_time;
pthread_mutex_t md_mutex;

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
 void sigint( int signo )
{
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

video_buffer_t* vbuf_allocate(streamInfo_t* info, IPC_Video_Frame_s* frame)
{
	video_buffer_t* vbuf;
	
	vbuf = malloc(sizeof(video_buffer_t));
	vbuf->frame = frame;
	vbuf->pmutex = &(info->buf_mutex);
	vbuf->ref = 1;
	
	return vbuf;
}

void vbuf_add_ref(video_buffer_t* buf)
{
	pthread_mutex_lock(buf->pmutex);
	buf->ref++;
	pthread_mutex_unlock(buf->pmutex);
}

void vbuf_free(video_buffer_t* buf)
{
	pthread_mutex_t* pmutex = buf->pmutex;
	pthread_mutex_lock(pmutex);
	buf->ref--;
	if (!buf->ref)
	{
		gp_IPC_VChn_FrameRelease(NULL, buf->frame);
		free(buf);
	}
	pthread_mutex_unlock(pmutex);
}

audio_buffer_t* abuf_allocate(IPC_Audio_Frame_s* frame)
{
	audio_buffer_t* abuf;
	
	abuf = malloc(sizeof(audio_buffer_t));
	abuf->frame = frame;
	abuf->pmutex = &audio_buf_mutex;
	abuf->ref = 1;
	
	return abuf;
}
void abuf_add_ref(audio_buffer_t* buf)
{
	pthread_mutex_lock(buf->pmutex);
	buf->ref++;
	pthread_mutex_unlock(buf->pmutex);
}

void abuf_free(audio_buffer_t* buf)
{
	pthread_mutex_t* pmutex = buf->pmutex;
	pthread_mutex_lock(pmutex);
	buf->ref--;
	if (!buf->ref)
	{
		gp_IPC_ADev_FrameRelease(NULL, buf->frame);
		free(buf->frame);
		free(buf);
	}
	pthread_mutex_unlock(pmutex);
}

static void *VideoTask(void *param)
{
	streamInfo_t* info = (streamInfo_t*)param;
	printf("[%s](%d)]\n", __FUNCTION__, __LINE__);
	
	while(1)
	{
		IPC_Video_Frame_s *frame;
		video_buffer_t* vbuf;
		
		if (g_bQuit || g_bPause)
			break;

		gp_IPC_VChn_Get_Frame(info->handle,&frame);
			
		if (!frame)
		{
			printf("cannot get frame\n");
			usleep(100);
			continue;
		}

		vbuf = vbuf_allocate(info, frame);
		
		if (info->flag & SFLAG_SAVE_TO_FILE)
			storageFileWrite(info, vbuf);
		
		vbuf_free(vbuf);
	}
}

static void *AudioTask(void *param)
{
	int i,ret=0;
	streamInfo_t* info = (streamInfo_t*)param;
	printf("[%s](%d)]\n", __FUNCTION__, __LINE__);
	
	while(1)
	{
		IPC_Audio_Frame_s* frame;
		audio_buffer_t* abuf;
		if (g_bQuit || g_bPause)
			break;

		frame = malloc(sizeof(IPC_Audio_Frame_s));
		ret = gp_IPC_ADev_Get_Frame(audio_handle, frame);

		if (ret < 0)
		{
			free(frame);
			usleep(10*1000);
			continue;
		}
		
		abuf = abuf_allocate(frame);
		
		if (info->flag & SFLAG_SAVE_TO_FILE)
			storageFileWriteAudio(info, abuf);
		
		abuf_free(abuf);
		
	}
	printf("[%s](%d)] exit\n", __FUNCTION__, __LINE__);
}

int mdetect(void* arg)
{
	//printf("motion detected!\n");
	pthread_mutex_lock(&md_mutex);
	gettimeofday(&md_time, NULL);
	g_bMotion = 1;
	pthread_mutex_unlock(&md_mutex);
	return 0;
}

int parse_command(int argc, char *argv[], option_t* opt, streamInfo_t* info)
{
	int i;
	for (i =1; i < argc; i++)
	{
		if (strcmp(argv[i], "-s") == 0)
		{
			int save, dimension, bitrate,framerate;
			
			if (argc <= i+4)
			{
				printf("parameter error, not enough stream information for stream\n");
				break;
			}
			save = atoi(argv[++i]);
			dimension = atoi(argv[++i]);
			bitrate=atoi(argv[++i]);
			framerate=atoi(argv[++i]);
			
			if(save)
				info->flag |= SFLAG_SAVE_TO_FILE;
							
			if(dimension == 0)
			{
				info->attr.rsl = IPC_RESOLUTION_1080P;
				info->name = "test1080p";
				info->width = 1920;
				info->height = 1080;
			}
			else if(dimension == 1)
			{
				info->attr.rsl = IPC_RESOLUTION_720P;
				info->name = "test720p";
				info->width = 1280;
				info->height = 720;
			}
			else if(dimension == 2)
			{
				info->attr.rsl = IPC_RESOLUTION_WVGA;
				info->name = "testWVGA";
				info->width = 848;
				info->height = 480;
			}
			else if(dimension == 3)
			{
				info->attr.rsl = IPC_RESOLUTION_VGA;
				info->name = "test480p";
				info->width = 640;
				info->height = 480;
			}
			else if(dimension == 4)
			{
				info->attr.rsl = IPC_RESOLUTION_QVGA;
				info->name = "test240p";
				info->width = 320;
				info->height = 240;
			}
			info->attr.bitrate = bitrate;
			info->attr.framerate = framerate;
			info->attr.bitrateMode=IPC_BITRATE_VBR;
			info->attr.cache_buf_num = 8;
			info->attr.ratio = 15;
		}
		else if (strcmp(argv[i], "-avi") == 0)
		{
			opt->cType = GP_CS_TYPE_AVI;
		}
		else if (strcmp(argv[i], "-mov") == 0)
		{
			opt->cType = GP_CS_TYPE_MOV;
		}
		else if (strcmp(argv[i], "-d") == 0)
		{
			opt->display = 1;
		}
		else if (strcmp(argv[i], "-scale") == 0)
		{
			opt->scale_type = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-md") == 0)
		{
			opt->motion_detect = 1;
		}
		else if (strcmp(argv[i], "-vstb") == 0)
		{
			opt->vstb_offset = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			strcpy(opt->outfile, argv[++i]);
			printf("outfile = %s\n", opt->outfile);
		}
		else if (strcmp(argv[i], "-fifo") == 0)
		{
			opt->fifo = 1;
		}
		else if (strcmp(argv[i], "-loop") == 0)
		{
			opt->loop_recording = 1;
			opt->loop_file_time = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-noaudio") == 0)
		{
			opt->disable_audio = 1;
		}
		else if (strcmp(argv[i], "-timestamp") == 0)
		{
			opt->time_stamp = 1;
		}
		else if (strcmp(argv[i], "-ev") == 0)
		{
			opt->ev = atoi(argv[++i]);
		}
	}

	if (opt->scale_type == 1)
		info->attr.scaler_type = IPC_SCALE_BILINEAR;
	else
		info->attr.scaler_type = IPC_SCALE_NORMAL;
		
	info->attr.video_stb_offset = opt->vstb_offset;
		
	if (!info->width) {
		printf(usage, argv[0]);
		return -1;
	}
	
	printf("[dvr] resolution: %d x %d\n", info->width, info->height);
	printf("[dvr] bitrate: %d kbps\n", info->attr.bitrate);
	printf("[dvr] framerate: %d fps\n", info->attr.framerate);

	if (opt->cType == GP_CS_TYPE_MOV)
		printf("[dvr] output format: mov\n");
	else if (opt->cType == GP_CS_TYPE_AVI)
		printf("[dvr] output format: mov\n");
	else if (opt->cType == GP_CS_TYPE_MP4)
		printf("[dvr] output format: mov\n");

	printf("[dvr] display: %d\n", opt->display);

	if (opt->scale_type)
		printf("[dvr] scaler: bilinear (faster)\n");
	else
		printf("[dvr] scaler: normal\n");
		
	printf("[dvr] motion detection: %d\n", opt->motion_detect);
	printf("[dvr] video stabilization: %d\n", opt->vstb_offset);
	printf("[dvr] fifo enable : %d\n", opt->fifo);
	printf("[dvr] loop recording: %d, %d min\n", opt->loop_recording , opt->loop_file_time);
	printf("[dvr] audio disable : %d\n", opt->disable_audio);
	printf("[dvr] time stamp : %d\n", opt->time_stamp);
	return 0;
}

int enable_file_saving(option_t opt, streamInfo_t* info)
{	
	if (info->flag & SFLAG_SAVE_TO_FILE)
	{	
		if (strlen(opt.outfile))
		{
			info->storageHandle = storageFileOpen(opt.outfile, info->width, info->height, info->attr.framerate, 
										opt.loop_file_time, opt.disable_audio ? CODEC_ID_NONE: CODEC_ID_PCM_S16LE);
		}
		else
		{
			char name[128];
			const char* ext;
			
			if (opt.cType == GP_CS_TYPE_MP4) ext = "mp4";
			else if (opt.cType == GP_CS_TYPE_AVI) ext = "avi";
			else if (opt.cType == GP_CS_TYPE_MOV) ext = "mov";

			if (access("/media/sdcardc", F_OK) == 0)			
				snprintf(name, sizeof(name), "/media/sdcardc/%s.%s", info->name, ext);
			else
				snprintf(name, sizeof(name), "/media/sdcardc1/%s.%s", info->name, ext);
			info->storageHandle = storageFileOpen(name, info->width, info->height, info->attr.framerate, 
										opt.loop_file_time, opt.disable_audio ? CODEC_ID_NONE: CODEC_ID_AAC);
		}
		if (!info->storageHandle)
		{
			printf("%s: cannot create file storage\n", __FUNCTION__);
			return -1;
		}
	}
	
	return 0;
}

int disable_file_saving(streamInfo_t* info)
{
	if (info->flag & SFLAG_SAVE_TO_FILE)
		storageFileFinish(info);
}

int enable_video_stream(streamInfo_t* info)
{
	info->handle = gp_IPC_VChn_Open(&info->attr);
	
	pthread_mutex_init(&info->buf_mutex, NULL);
	
	gp_IPC_VChn_Enable(info->handle);
	
	pthread_create(&(info->pthread), NULL, (void*)VideoTask, info);
	
	return 0;
}

int disable_video_stream(streamInfo_t* info)
{
	pthread_join(info->pthread, NULL);
	
	gp_IPC_VChn_Disable(info->handle);
	gp_IPC_VChn_close(info->handle);
	
	pthread_mutex_destroy(&(info->buf_mutex));
	
	return 0;
}

int enable_audio(streamInfo_t* info)
{
	pthread_mutex_init(&audio_buf_mutex, NULL);
	
	gp_IPC_ADev_Enable(audio_handle);
	
	pthread_create(&audio_pthread, NULL, (void*)AudioTask, info);
	
	return 0;
}

int disable_audio()
{
	pthread_join(audio_pthread, NULL);
	
	gp_IPC_ADev_Disable(audio_handle);
	
	pthread_mutex_destroy(&audio_buf_mutex);
	
	return 0;
}

int main(int argc, char *argv[])
{
	void *handle;
	int i = 0;
	streamInfo_t info;
	pthread_t main_thread;
	IPC_ADev_Attr_s audio_attr;
	int md_handle;
	option_t opt;
	int w,h, bitrate, fps;
	int sensorMode = 0;
	int restart = 0;
	int md_wait_UI = 0;
	IPC_Resolution_e rsl;
	void* csi_attr;
	int disp4x3 = 0;

	
	memset(&info, 0, sizeof(streamInfo_t));
	memset(&opt, 0, sizeof(option_t));
	opt.cType = GP_CS_TYPE_MP4;
	opt.ev = 6;
	//memset(outfile, 0, 256);
	if (0 > parse_command(argc, argv, &opt, &info))
		return -1;			

	dvr_pipe_init();

start_dvr:
	g_bQuit = 0;
	g_bPause = 1;
	restart = 0;
	md_wait_UI = 0;
		
	audio_attr.status = 0;
	audio_attr.bitrate = 0;
	audio_attr.samplerate = 16000;
	audio_attr.bitrate = 64000;
	
	audio_attr.bitwidth = 1;
	audio_attr.soundmode = 1;
	//audio_attr.format = CODEC_ID_AAC;
	audio_attr.format = CODEC_ID_PCM_S16LE;
	audio_attr.volume = 100;
	audio_attr.frameNum = 0;
	audio_attr.aec = 0;
	audio_attr.anr = 0;
	
	if (info.width == 1920 && info.height == 1080)
		sensorMode = 0;
	else
		sensorMode = 1;
	
	gp_IPC_VStream_Open(opt.rgb_mode ? IPC_COLOR_RGB565 : IPC_COLOR_YUYV, 0, 0, ON2_H264);
#ifndef DISABLE_AUDIO_STREAM
	audio_handle = gp_IPC_ADev_Open(&audio_attr);
#endif

	csi_attr = gp_IPC_VDev_Open(NULL);
	
	gp_IPC_VDev_Set_Exposure(csi_attr, opt.ev);

	rsl	= info.attr.rsl;
	if (rsl == IPC_RESOLUTION_VGA || rsl == IPC_RESOLUTION_QVGA)
		disp4x3 = 1;
	else
		disp4x3 = 0;
		
	if (opt.display)
		gp_IPC_Enable_Display(1, 0, IPC_SCALE_BILINEAR, disp4x3);
	
	gp_IPC_Enable_TimeStamp(opt.time_stamp ? 2:0);
	
	/* storage init */
	if (storageServiceInit(opt.cType, opt.loop_recording) < 0) {
		printf("Storage service init fail.\n");
		return -1;
	}
			
	if (opt.motion_detect)
	{
		int mwidth, mheight;
		gd_md_open();
		gd_md_get_resolution(&mwidth,&mheight);
		
		if (opt.time_stamp)
			mheight -= 80;
			
		md_handle = gp_md_register_area(0, 0 , mwidth, mheight,mdetect,NULL);
		pthread_mutex_init(&md_mutex, NULL);
	}
	
	if (!opt.fifo)
	{
		if (opt.display)
			gp_IPC_Enable_Display(1, 2, IPC_SCALE_BILINEAR, disp4x3);
		enable_file_saving(opt, &info);
		enable_video_stream(&info);
		if (!opt.disable_audio)
			enable_audio(&info);
			
		g_bPause =0;
	}	
	
	RegisterSigint();
	
	if (opt.fifo)
		dvr_pipemsg_send(CMD_READY_KEY, 0, NULL);
		
	while(1)
	{
		unsigned int msgId;
		void* msgPara;
		unsigned int mode;
		
		if (g_bQuit)
			break;
		
		if(opt.fifo && dvr_pipemsg_receive(&msgId, &msgPara) > 0)
		{
			printf("msgId=%d\n", msgId);
			if (msgPara)
			{
				mode = *(unsigned int*)msgPara;
				printf("mode = %d\n", mode);
			}
			switch(msgId)
			{
			case CMD_START_DV:
				md_wait_UI = 0;
				if (g_bPause)
				{
					g_bPause = 0;
					if (opt.display)
						gp_IPC_Enable_Display(1, 2, IPC_SCALE_BILINEAR, disp4x3);
					enable_file_saving(opt, &info);
					enable_video_stream(&info);
					if (!opt.disable_audio)
						enable_audio(&info);
					printf("++++++++++DV Started++++++++++\n");
				}
				break;
			case CMD_STOP_DV:
				md_wait_UI = 0;
				if (!g_bPause)
				{
					g_bPause = 1;
					disable_file_saving(&info);
					if (!opt.disable_audio)
						disable_audio();
					disable_video_stream(&info);
					if (opt.display)
						gp_IPC_Enable_Display(1, 0, IPC_SCALE_BILINEAR, disp4x3);
					printf("++++++++++DV Stopped++++++++++\n");
				}
				break;
			case CMD_SET_DV_RESOLUTION:
				fps = 30;
				if (mode == 0)	{ w = 1920; h = 1080; bitrate = 12000; 	rsl = IPC_RESOLUTION_1080P; }
				else if (mode == 1)	{ w = 1280; h = 720; bitrate = 6500; rsl = IPC_RESOLUTION_720P; fps *=2;}
				else if (mode == 2)	{ w = 1280; h = 720; bitrate = 6500; rsl = IPC_RESOLUTION_720P; }
				else if (mode == 3)	{ w = 848; h = 480; bitrate = 3200; rsl = IPC_RESOLUTION_WVGA; }
				else if (mode == 4)	{ w = 640; h = 480; bitrate = 3200; rsl = IPC_RESOLUTION_VGA; }
				
				if (info.width != w || info.height != h)
				{
					info.width = w;
					info.height = h;
					info.attr.bitrate = bitrate;
					//info.attr.rsl = rsl;
					info.attr.framerate = fps;

					//if (sensorMode != ((mode == 0) ? 0:1))
					//{
					//	//restart sensor
					//	restart = 1;
					//	g_bQuit = 1;
					//}
					
					if (opt.display)
					{
						if (((rsl == IPC_RESOLUTION_VGA) ? 1: 0) != disp4x3)
						{
							disp4x3 = (rsl == IPC_RESOLUTION_VGA) ? 1: 0 ;
							gp_IPC_Enable_Display(0, 0, 0, 0);
							gp_IPC_Enable_Display(1, 0, IPC_SCALE_BILINEAR, disp4x3);
						}
					}
					info.attr.rsl = rsl;
	
					printf("[dvr] change mode to %d x %d\n", info.width, info.height);
				}
				break;
			case CMD_SET_DIR:
				break;
			case CMD_SET_LOOPRECORDING:
				if (mode == 0)
					opt.loop_file_time = 1440; //24 hour
				else if (mode == 1)
					opt.loop_file_time = 2;
				else if (mode == 2)
					opt.loop_file_time = 3;
				else if (mode == 3)
					opt.loop_file_time = 5;
					
				if (mode)
					printf("[dvr] loop recording every %d min\n", opt.loop_file_time);
				else
					printf("[dvr] disable loop recording\n");
					
				break;
			case CMD_SET_MD:
				if (!opt.motion_detect && mode)
				{
					int mwidth, mheight;
					pthread_mutex_init(&md_mutex, NULL);
					gd_md_open();
					gd_md_get_resolution(&mwidth,&mheight);
					md_handle = gp_md_register_area(0, 0 , mwidth, mheight,mdetect,NULL);
					printf("[dvr] enable motion detection\n");
					opt.motion_detect = 1;
				}
				else if (opt.motion_detect && !mode)
				{
					gd_md_unregister_area(md_handle);
					gd_md_close();
					pthread_mutex_destroy(&md_mutex);
					printf("[dvr] disable motion detection\n");
					opt.motion_detect = 0;
				}
				break;
			case CMD_SET_RECORD_AUDIO:
				opt.disable_audio = mode ? 0:1;
				if (mode)
					printf("[dvr] enable audio recording\n");
				else
					printf("[dvr] disable audio recording\n");
				break;
			case CMD_SET_DV_DATE_STAMP:
				opt.time_stamp = mode;
				gp_IPC_Enable_TimeStamp(mode ? 2:0);
				if (mode)
				{
					printf("[dvr] enable time stamp\n");
				}
				else
				{
					printf("[dvr] disable time stamp\n");
				}
				break;
			case CMD_SET_EXPOSURE:
				gp_IPC_VDev_Set_Exposure(csi_attr, mode);
				printf("[dvr] set Exposure: %d\n", mode);
				break;
			case CMD_SET_ZOOM:
				mode = mode ? mode + 10: 0;
				gp_IPC_Set_Zoom(mode);
				break;
			case CMD_SET_LOCK_FILE:
				if (!g_bPause)
					LockFileNotify = 1;
				break;
			case CMD_SET_EXIT:
				g_bQuit = 1;
				break;
			}
		
			if (msgId != CMD_SET_EXIT)
				dvr_response_cmd(msgId);
		}
		
		if (opt.motion_detect)
		{
			struct timeval tv;
			int t, detected;
			pthread_mutex_lock(&md_mutex);
			gettimeofday(&tv, NULL);
			t = tv.tv_sec - md_time.tv_sec;
			detected = g_bMotion;
			g_bMotion = 0;
			pthread_mutex_unlock(&md_mutex);
			if ( t > 10 && !g_bPause)
			{
				g_bPause = 1;
				disable_file_saving(&info);
				if (!opt.disable_audio)
					disable_audio();
				disable_video_stream(&info);
				if (opt.display)
					gp_IPC_Enable_Display(1, 0, IPC_SCALE_BILINEAR, disp4x3);
				printf("++++++++++DV Stopped++++++++++\n");
				dvr_pipemsg_send(CMD_SET_MD_DV_STOP, 0, NULL);
			}
			else if (!md_wait_UI && detected && g_bPause)
			{
				md_wait_UI = 1;
				dvr_pipemsg_send(CMD_SET_MD_DV_START, 0, NULL);
			}
			
		}
		
		if (opt.fifo && changeFileNotify)
		{
			changeFileNotify = 0;
			dvr_pipemsg_send(CMD_SET_LOOPRECORDING_INT, 0, NULL);
		}
		//sleep(1);
		usleep(1000);
	}

	if (!opt.fifo || !g_bPause)
	{
		if (!opt.disable_audio)
			disable_audio();
		disable_video_stream(&info);
		disable_file_saving(&info);
	}
	
	storageServiceClose();
	
	if (opt.motion_detect)
	{
		gd_md_close();
		pthread_mutex_destroy(&md_mutex);
	}
		
	gp_IPC_VDev_Close(csi_attr);
	gp_IPC_VStream_Close();
#ifndef DISABLE_AUDIO_STREAM
	gp_IPC_ADev_Close(audio_handle);
#endif

	if (restart)
		goto start_dvr;
		
	if(opt.fifo)
		dvr_response_cmd(CMD_SET_EXIT);	
	
	return 0;
}