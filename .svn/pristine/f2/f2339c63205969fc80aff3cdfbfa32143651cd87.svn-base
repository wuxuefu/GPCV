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

#include "mach/typedef.h"

#include "gp_video_stream_api.h"
#include "gp_audio_stream_api.h"
#include "gp_cvr.h"
#include "gp_csi_attr.h"
#include "csi_md.h"

#include "stream.h"
#include "gp_mux.h"
#include "dvr_pipe.h"
#include "mach/aeawb.h"
#include "mach/gp_cdsp.h"

extern sensor_calibration_t ar0330_cdsp_calibration;
extern gp_cdsp_user_preference_t cdsp_user_preference;

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define BITRATE_1080P	16000
#define BITRATE_720P	6500
#define BITRATE_480P	3200

#define TIME_LAPSE_OFF	0
#define TIME_LAPSE_ON	1
#define TIME_LAPSE_INPUT_ONLY 2

//#define DISP_SCALE_TYPE	IPC_SCALE_BILINEAR
#define DISP_SCALE_TYPE	IPC_SCALE_NORMAL
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define CHECK_CHANGE(b1,b2) (((b1) && !(b2)) || (!(b1) && (b2)))

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
	int fifo;
	int loop_recording;
	int loop_file_time;
	int disable_audio;
	int time_stamp;
	int time_format;
	int ev;
	int ldw;
	int ldw_mode;
	int ldw_sen;
	int ldw_region;
	int ldw_speed;
	int ldw_sfcw ;
	int ldw_StopAndGo;
	int time_lapse;
	int time_lapse_duration;
	int real60fps;
	char outfile[256];
} dv_option_t;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern char cam_working_path[256];
extern int global_ev;
extern int global_dv_reset;
extern int global_dc_reset;
extern int global_time_format;
extern int global_ldw;
extern int global_60fps;
extern int global_frequency;
extern int global_flip;
extern int g_flip_on, g_flip_off;

extern int set_sharpness(void* csi_attr, int mode);
extern int set_awb(void* csi_attr, int mode);
extern int set_color(void* csi_attr, int mode);
extern int set_iso(void* csi_attr, int mode);

extern int display_in(int arg);
extern int display_out(int arg);
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static void *VideoTask(void *param);
static void *AudioTask(void *param);

extern void update_flag(unsigned int* flag, unsigned int value, int on);
/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
#define usage \
"\nUsage: %s [-s save dimension bitrate framerate] [-s ...] [-avi][-a type][-d][-scale][-rgb]\n"\
"\n"\
"-s:    add a stream\n"\
"       save:       0:no save to file\n"\
"                   1:save to file\n"\
"                   2:both\n"\
"       dimension:  0:1080p \n"\
"                   1:720p \n"\
"                   2:VGA \n"\
"                   3:QVGA\n"\
"       bitrate:    in kbps\n"\
"       framerate:  frame per second\n"\
"-d		display to panel \n"\
"-scale set scaler type \n"\
"		0: integration mode\n"\
"		1: bilinear mode (faster)\n"\
"-vstb n	enable video stabilization with (w+n)*(h+n) buffer \n"\
"-md		enable motion detection\n"

static streamInfo_t stream;
static dv_option_t opt;
extern int g_bQuit;
int g_bRestart = 0;
int g_bPause = 0;
int g_bMotion = 0;
int g_bSaveFail = 0;
int g_FrameCount = 0;
int g_bLDW_ON = 0;
int g_bLDW_OFF = 0;
int g_bLDW_ALARM = 0;
int g_bLDW_FCW_ON = 0;
int g_bLDW_GOALARM_ON = 0;
int g_bLDW_STOPALARM_ON = 0;
int g_bLDW_FCW_flag_bak = 0;
int g_bLDW_FCW_flag=0;
int g_bLDW_StopAndGo_flag_bak = 0;
int g_bLDW_StopAndGo_flag=0;
pthread_t audio_pthread;
pthread_mutex_t audio_buf_mutex;
void* audio_handle = NULL;
struct timeval md_time;
pthread_mutex_t md_mutex;
extern display_param_t disp_info;

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
LDW_DISPLINE_t ldw_get_displine;

video_buffer_t* vbuf_allocate(streamInfo_t* info, IPC_Video_Frame_s* frame)
{
	video_buffer_t* vbuf;
	
	vbuf = malloc(sizeof(video_buffer_t));
	vbuf->frame = frame;
	vbuf->info = info;
	vbuf->ref = 1;
	
	return vbuf;
}

void vbuf_add_ref(video_buffer_t* buf)
{
	pthread_mutex_lock(&buf->info->buf_mutex);
	buf->ref++;
	pthread_mutex_unlock(&buf->info->buf_mutex);
}

void vbuf_free(video_buffer_t* buf)
{
	pthread_mutex_t* pmutex = &buf->info->buf_mutex;
	pthread_mutex_lock(pmutex);
	buf->ref--;
	if (!buf->ref)
	{
		gp_IPC_VChn_FrameRelease(buf->info->handle, buf->frame);
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
		
		if (g_bQuit || g_bPause || g_bSaveFail)
			break;

		if (storageFileRequestHeaderQuery(info) > 0)
		{
			gp_IPC_VChn_Request_Header(info->handle);
		}

		//gp_IPC_VChn_Get_Frame(info->handle,&frame);
		gp_IPC_VChn_Get_FrameTimeOut(info->handle,&frame, 10*1000);
			
		if (!frame)
		{
			printf("cannot get frame\n");
			usleep(100);
			continue;
		}

		vbuf = vbuf_allocate(info, frame);
		
		if (info->flag & SFLAG_SAVE_TO_FILE)
			storageFileWrite(info, vbuf);
		
		g_FrameCount++;
		
		vbuf_free(vbuf);
		
		if ((info->flag & SFLAG_SAVE_TO_FILE) && !info->storageHandle)
			g_bSaveFail = 1;
	}
	
	printf("[%s](%d)] exit\n", __FUNCTION__, __LINE__);
}

static void *AudioTask(void *param)
{
	int i,ret=0;
	streamInfo_t** info = (streamInfo_t**)param;
	printf("[%s](%d)]\n", __FUNCTION__, __LINE__);
	
	while(1)
	{
		IPC_Audio_Frame_s* frame;
		audio_buffer_t* abuf;
		if (g_bQuit || g_bPause || g_bSaveFail)
			break;

		if (!g_FrameCount)
		{
			usleep(100);
			continue;
		}

		frame = malloc(sizeof(IPC_Audio_Frame_s));
		ret = gp_IPC_ADev_Get_Frame(audio_handle, frame);

		if (ret < 0)
		{
			free(frame);
			usleep(10*1000);
			continue;
		}
		
		abuf = abuf_allocate(frame);
		
		i = 0;
		while(info[i])
		{
			if (info[i]->flag & SFLAG_SAVE_TO_FILE)
			{
				if (!info[i]->storageHandle)
					g_bSaveFail = 1;
				else
					storageFileWriteAudio(info[i], abuf);
			}
				
			i++;
		}
		
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
#define ASENSOR_IOCTL_ID	'G'
#define ASENSOR_IOCTL_GET_G_VALUE	_IOR(ASENSOR_IOCTL_ID, 0x10, int)
int  ldw_get_gsensor_gValue(void)
{
	int fd;
	int temp;
	fd = open("/dev/gp_asensor",O_RDWR);
	if(fd<0){		
		printf("Can't open [/dev/gp_asensor]\n");		
		return 0;	
	}
	ioctl(fd, ASENSOR_IOCTL_GET_G_VALUE, &temp);
	close(fd);
	return temp;
}


int ldw_enable(int arg)
{
	g_bLDW_ON = g_bLDW_OFF = 0;
	if (arg)
		g_bLDW_ON = 1;
	else
		g_bLDW_OFF = 1;
	return 0;
}

int ldw_alarm(int arg)
{
	g_bLDW_ALARM = 1;
	return 0;
}
//////////////
int ldw_FCW_enable(int arg)
{	
	g_bLDW_FCW_ON = 0;	
	if (arg)
	{
		g_bLDW_FCW_ON = 1;
	}
	return 0;
}

int ldw_GOALARM_enable(int arg)
{	
	g_bLDW_GOALARM_ON =g_bLDW_STOPALARM_ON = 0;	
	if (arg==2)			
		g_bLDW_GOALARM_ON = 1;
	else if(arg==1)	
		g_bLDW_STOPALARM_ON = 1;	return 0;
}
int ldw_FCW_Disp_enable(int arg)
{	
	g_bLDW_FCW_flag = arg;//
	return 0;
}

int ldw_StopAndGO_Disp_enable(int arg)
{	
	g_bLDW_StopAndGo_flag =arg;
	return 0;
}
int dv_parse_command(int argc, char *argv[])
{
	int i;
	
	memset(&stream, 0, sizeof(streamInfo_t));
	memset(&opt, 0, sizeof(dv_option_t));
	opt.cType = GP_CS_TYPE_MOV;
	opt.ev = 6;
	
	for (i =1; i < argc; i++)
	{
		if (strcmp(argv[i], "-dv") == 0)
			break;
	}
	
	for (; i < argc; i++)
	{
		if (strcmp(argv[i], "-dc") == 0)
			break;
			
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
				stream.flag |= SFLAG_SAVE_TO_FILE;
							
			if(dimension == 0)
			{
				stream.attr.rsl = IPC_RESOLUTION_1080P;
				stream.name = "test1080p";
				stream.width = 1920;
				stream.height = 1080;
				stream.attr.bitrate = BITRATE_1080P;
			}
			else if(dimension == 1)
			{
				stream.attr.rsl = IPC_RESOLUTION_720P;
				stream.name = "test720p";
				stream.width = 1280;
				stream.height = 720;
				stream.attr.bitrate = BITRATE_720P;
			}
			else if(dimension == 2)
			{
				stream.attr.rsl = IPC_RESOLUTION_WVGA;
				stream.name = "testWVGA";
				stream.width = 848;
				stream.height = 480;
				stream.attr.bitrate = BITRATE_480P;
			}
			else if(dimension == 3)
			{
				stream.attr.rsl = IPC_RESOLUTION_VGA;
				stream.name = "test480p";
				stream.width = 640;
				stream.height = 480;
				stream.attr.bitrate = BITRATE_480P;
			}
			else if(dimension == 4)
			{
				stream.attr.rsl = IPC_RESOLUTION_QVGA;
				stream.name = "test240p";
				stream.width = 320;
				stream.height = 240;
				stream.attr.bitrate = 1600;
			}
			//stream.attr.bitrate = bitrate;
			stream.attr.framerate = framerate;
			stream.attr.bitrateMode=IPC_BITRATE_VBR;
			stream.attr.cache_buf_num = CACHE_NUM;
			stream.attr.ratio = 15;
		}
		else if (strcmp(argv[i], "-avi") == 0)
		{
			opt.cType = GP_CS_TYPE_AVI;
		}
		else if (strcmp(argv[i], "-mov") == 0)
		{
			opt.cType = GP_CS_TYPE_MOV;
		}
		else if (strcmp(argv[i], "-d") == 0)
		{
			opt.display = 1;
		}
		else if (strcmp(argv[i], "-d2") == 0)
		{
			opt.display = 0;
			sscanf(argv[++i],"0x%x", &disp_info.address);
			disp_info.width = atoi(argv[++i]);
			disp_info.height = atoi(argv[++i]);
			disp_info.buffer_num = atoi(argv[++i]);
			disp_info.disp_in_f = display_in;
			disp_info.disp_out_f = display_out;
		}
		else if (strcmp(argv[i], "-scale") == 0)
		{
			opt.scale_type = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-md") == 0)
		{
			opt.motion_detect = 1;
		}
		else if (strcmp(argv[i], "-vstb") == 0)
		{
			opt.vstb_offset = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			int j;
			strcpy(opt.outfile, argv[++i]);
			printf("outfile = %s\n", opt.outfile);
			strcpy(cam_working_path, opt.outfile);
			j = strlen(cam_working_path);
			while(j >=0 && cam_working_path[j] != '/') j--;
			cam_working_path[j+1] = 0;
		}
		else if (strcmp(argv[i], "-fifo") == 0)
		{
			opt.fifo = 1;
		}
		else if (strcmp(argv[i], "-loop") == 0)
		{
			opt.loop_recording = 1;
			opt.loop_file_time = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-noaudio") == 0)
		{
			opt.disable_audio = 1;
		}
		else if (strcmp(argv[i], "-timestamp") == 0)
		{
			opt.time_stamp = 1;
		}
		else if (strcmp(argv[i], "-timeformat") == 0)
		{
			opt.time_format = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-ev") == 0)
		{
			opt.ev = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-ldw") == 0)
		{
			opt.ldw = atoi(argv[++i]);
			
			if (opt.ldw > 0)
			{
				opt.ldw_mode = opt.ldw - 1;
				opt.ldw = 1;
			}
			opt.ldw_sen = atoi(argv[++i]);
			opt.ldw_region = atoi(argv[++i]);
			opt.ldw_speed = atoi(argv[++i]);
			opt.ldw_sfcw =  atoi(argv[++i]);
			opt.ldw_StopAndGo=  atoi(argv[++i]);
			global_ldw = opt.ldw;
		}
		else if (strcmp(argv[i], "-timelapse") == 0)
		{
			opt.time_lapse = atoi(argv[++i]); //time lapse mode: 0: off 1: on 2:no speed up video
			opt.time_lapse_duration = atoi(argv[++i]); // in msec
		}
		else if (strcmp(argv[i], "-60fps") == 0)
		{
			opt.real60fps = 1;
			global_60fps = 1;
		}
		else if (strcmp(argv[i], "-freq") == 0)
		{
			global_frequency = atoi(argv[++i]);
		}
	}

	if (opt.scale_type == 1)
		stream.attr.scaler_type = IPC_SCALE_BILINEAR;
	else
		stream.attr.scaler_type = IPC_SCALE_NORMAL;
		
	stream.attr.video_stb_offset = opt.vstb_offset;
	stream.attr.enable_thumb = 1;
		
	if (stream.attr.rsl != IPC_RESOLUTION_720P || stream.attr.framerate != 60)
		global_60fps = 0;
		
	if (!opt.fifo && !stream.width) {
		printf(usage, argv[0]);
		return -1;
	}
	
	return 0;
}

static int enable_file_saving(dv_option_t opt, streamInfo_t* info)
{	
	if (info->flag & SFLAG_SAVE_TO_FILE)
	{	
		if (strlen(opt.outfile))
		{
			char outname[256];
			char *outdir = NULL;
			int vcodec;
			
			outdir = strdup("DCIM");
			vcodec = CODEC_ID_H264;

			sprintf(outname, "%s/%s. .MOV", opt.outfile, outdir);
			
			free(outdir);
			
			info->storageHandle = storageFileOpen(outname, info->width, info->height, info->attr.framerate,
										opt.loop_file_time, vcodec, CODEC_ID_PCM_S16LE, opt.time_lapse == TIME_LAPSE_ON ? 1:0, 0);
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
										opt.loop_file_time, CODEC_ID_H264, CODEC_ID_NONE, opt.time_lapse == TIME_LAPSE_ON ? 1:0, 0);
		}
		if (!info->storageHandle)
		{
			printf("%s: cannot create file storage\n", __FUNCTION__);
			return -1;
		}
	}
	
	return 0;
}

static int disable_file_saving(streamInfo_t* info)
{
	if (info->flag & SFLAG_SAVE_TO_FILE)
	{
		storageFileFinish(info);
		while(info->storageHandle)
			usleep(100);
	}
}

static int enable_video_stream(streamInfo_t* info)
{
	info->handle = gp_IPC_VChn_Open(&info->attr);
	
	pthread_mutex_init(&info->buf_mutex, NULL);
	
	gp_IPC_VChn_Enable(info->handle);
	
	pthread_create(&(info->pthread), NULL, (void*)VideoTask, info);
	
	return 0;
}

static int disable_video_stream(streamInfo_t* info)
{
	//pthread_join(info->pthread, NULL);
	
	gp_IPC_VChn_Disable(info->handle);
	gp_IPC_VChn_close(info->handle);
	
	pthread_mutex_destroy(&(info->buf_mutex));
	
	return 0;
}

static void* audio_init()
{
	IPC_ADev_Attr_s audio_attr;
	
	audio_attr.status = 0;
	audio_attr.bitrate = 0;
	audio_attr.samplerate = 8000;
	audio_attr.bitrate = audio_attr.samplerate*2*8;
	
	audio_attr.bitwidth = 1;
	audio_attr.soundmode = 1;
	//audio_attr.format = CODEC_ID_AAC;
	audio_attr.format = CODEC_ID_PCM_S16LE;
	audio_attr.volume = 70;
	audio_attr.frameNum = 0;
	audio_attr.aec = 0;
	audio_attr.anr = 0;

	return gp_IPC_ADev_Open(&audio_attr);
}

static int add_audio(streamInfo_t* info)
{
	pthread_create(&audio_pthread, NULL, (void*)AudioTask, info);
	
	return 0;
}
static int enable_audio(streamInfo_t** stream_list)
{
	pthread_mutex_init(&audio_buf_mutex, NULL);
	
	gp_IPC_ADev_Enable(audio_handle);
	
	pthread_create(&audio_pthread, NULL, (void*)AudioTask, stream_list);
	
	return 0;
}

static int disable_audio()
{
	//pthread_join(audio_pthread, NULL);
	
	gp_IPC_ADev_Disable(audio_handle);
	
	pthread_mutex_destroy(&audio_buf_mutex);
	
	return 0;
}

static int enable_motion_detect()
{
	int mwidth, mheight;
	
	printf("[dvr] enable motion detection\n");
	
	pthread_mutex_init(&md_mutex, NULL);
	gd_md_open();
	gd_md_get_resolution(&mwidth,&mheight);
	return gp_md_register_area(0, 0 , mwidth, mheight,mdetect,NULL);
}

static void disable_motion_detect(int handle)
{
	gd_md_unregister_area(handle);
	gd_md_close();
	pthread_mutex_destroy(&md_mutex);
	printf("[dvr] disable motion detection\n");
}

static void dv_reset(streamInfo_t* info, dv_option_t* opt)
{
	info->width = 1920;
	info->height = 1080;
	info->attr.bitrate = BITRATE_1080P;
	info->attr.framerate = 30;
	info->attr.rsl = IPC_RESOLUTION_1080P;

	opt->motion_detect = 0;
	opt->loop_recording = 1;
	opt->loop_file_time = 3;
	opt->disable_audio = 0;
	opt->time_stamp = 1;
	opt->time_format = 0;
	opt->ev = 6;
	opt->ldw = 0;
	opt->ldw_mode = 0;
	opt->ldw_sen = 1;
	opt->ldw_region = 0;
	opt->ldw_speed = 0;
	opt->ldw_sfcw = 0;
	opt->ldw_StopAndGo= 0;
	opt->time_lapse= 0;
	opt->time_lapse_duration= 0;
	opt->real60fps = 0;
}

int dv_main(int argc, char *argv[])
{
	void *handle;
	int i = 0;
	streamInfo_t* stream_list[2];
	pthread_t main_thread;
	int md_handle = -1;
	int w,h, bitrate, fps;
	int md_wait_UI = 0;
	IPC_Resolution_e rsl;
	void* csi_attr;
	int disp4x3 = 0;
	int ldw_en = 0;
	int display_skip = 0;

/*======================================
Initialize
======================================*/

	g_bRestart = 0;
	
/* sync status with DC */
	if (global_dc_reset)
	{
		dv_reset(&stream, &opt);
		global_dc_reset = 0;
	}
	if (global_ev >= 0)
		opt.ev = global_ev;
	
	if (global_time_format >= 0)
		opt.time_format = global_time_format;
		
	if (strlen(cam_working_path) > 0)
		strcpy(opt.outfile, cam_working_path);

/* display info */
	printf("[dvr] resolution: %d x %d\n", stream.width, stream.height);
	printf("[dvr] bitrate: %d kbps\n", stream.attr.bitrate);
	printf("[dvr] framerate: %d fps\n", stream.attr.framerate);

	if (opt.cType == GP_CS_TYPE_MOV)
		printf("[dvr] output format: mov\n");
	else if (opt.cType == GP_CS_TYPE_AVI)
		printf("[dvr] output format: mov\n");
	else if (opt.cType == GP_CS_TYPE_MP4)
		printf("[dvr] output format: mov\n");

	printf("[dvr] display: %d\n", opt.display);

	if (opt.scale_type)
		printf("[dvr] scaler: bilinear (faster)\n");
	else
		printf("[dvr] scaler: normal\n");
		
	printf("[dvr] motion detection: %d\n", opt.motion_detect);
	printf("[dvr] video stabilization: %d\n", opt.vstb_offset);
	printf("[dvr] fifo enable : %d\n", opt.fifo);
	printf("[dvr] loop recording: %d, %d min\n", opt.loop_recording , opt.loop_file_time);
	printf("[dvr] audio disable : %d\n", opt.disable_audio);
	printf("[dvr] time stamp : %d\n", opt.time_stamp);
	printf("[dvr] ldw : %d\n", opt.ldw);
	printf("[dvr] ldw mode : %d\n", opt.ldw_mode);
	printf("[dvr] ldw sensitivity: %d\n", opt.ldw_sen);
	printf("[dvr] ldw region: %d\n", opt.ldw_region);
	printf("[dvr] ldw speed: %d\n", opt.ldw_speed);
	printf("[dvr] ldw SFCW: %d\n", opt.ldw_sfcw);
	printf("[dvr] ldw stopAndGo: %d\n", opt.ldw_StopAndGo);

	printf("[dvr] time_lapse: %d\n", opt.time_lapse);
	printf("[dvr] time_lapse_duration: %d\n", opt.time_lapse_duration);
	
/* initial pipe commnication with CVR UI */
	
	if (opt.fifo)
		dvr_pipe_init();

start_dvr:
	ldw_en = (opt.ldw && (stream.attr.rsl == IPC_RESOLUTION_1080P)) ? 1:0;
	display_skip = ldw_en ? 2:0;

/* check if need restart sensor inside DV */	

	if (g_bRestart)
	{
		int ret;
		gpCVR_VStream_Cfg cfg;
	
		memset(&cfg, 0, sizeof(gpCVR_VStream_Cfg));
		cfg.mode = MODE_H264;
		cfg.sensor_calibrate = 	&ar0330_cdsp_calibration;
		cfg.cdsp_user_preference = &cdsp_user_preference;
		update_flag(&cfg.flag, CFG_LDW, ldw_en);
		if (opt.real60fps)
		{
			cfg.width = 1280;
			cfg.height = 720;
			cfg.fps = 60;
		}
			
		printf("[camcorder] restart DV\n");
		gp_IPC_VStream_Close();
		do {
			ret = gp_IPC_VStream_Open(&cfg);
			
			if (ret < 0)
			{
				printf("[camcorder] restart DV Fail!\n");
				usleep(500000);
			}
		}while (ret < 0);
		
		gp_IPC_VStream_Start();
	}
	g_bQuit = 0;
	g_bPause = 1;
	md_wait_UI = 0;

/* time lapse parameters */
	if (opt.time_lapse)
	{
		stream.attr.target_fps = 1000/opt.time_lapse_duration;
	}
	else
		stream.attr.target_fps = 0;

/* Initial audio */
#ifndef DISABLE_AUDIO_STREAM
	if (opt.time_lapse != TIME_LAPSE_ON)
		audio_handle = audio_init();
#endif

/* sensor image attribute parameters */

	rsl	= stream.attr.rsl;
	gp_IPC_VStream_Set_Resolution(rsl);
	gp_IPC_Set_Zoom(0);
	gp_IPC_Set_Mode(MODE_H264);
			
	csi_attr = gp_IPC_VDev_Open(NULL);

	gp_IPC_VDev_Set_Frequency(csi_attr, global_frequency);

	gp_IPC_VDev_MirrorImage(csi_attr, global_flip);
	gp_IPC_VDev_FlipImage(csi_attr, global_flip);
	
	gp_IPC_VDev_Set_Exposure(csi_attr, opt.ev);
	set_sharpness(csi_attr, 1);
	gp_IPC_VDev_Set_AWB(csi_attr, AWB_AUTO_CVR);
	set_color(csi_attr, 0);
	set_iso(csi_attr,0);

/* aspect ratio, display and timestamp */
	if (rsl == IPC_RESOLUTION_VGA || rsl == IPC_RESOLUTION_QVGA)
		disp4x3 = 1;
	else
		disp4x3 = 0;

	if (opt.display == 1)
		gp_IPC_Enable_Display(display_skip, DISP_SCALE_TYPE, disp4x3);
	
	gp_IPC_Enable_TimeStamp(opt.time_stamp ? 2:0, opt.time_format);
	
/* File Storage Service init */
	if (storageServiceInit(opt.cType, opt.loop_recording) < 0) {
		printf("Storage service init fail.\n");
		if (opt.fifo && !g_bRestart)
			dvr_pipemsg_send(CMD_READY_KEY, 0, NULL);
		dvr_pipe_close();
		return -1;
	}
		
/* Motion Detect Initial */	
	if (opt.motion_detect)
	{
		md_handle = enable_motion_detect();
	}
	
/* LDW initial */
	if (ldw_en)
	{
		LDW_param_t ldw_param;
		
		ldw_param.mode = opt.ldw_mode;
		ldw_param.sensitivity = opt.ldw_sen;
		ldw_param.region = opt.ldw_region;
		ldw_param.turn_on_speed = opt.ldw_speed;
		ldw_param.sfcw = opt.ldw_sfcw ;
		ldw_param.StopAndGo=opt.ldw_StopAndGo;
		ldw_param.enable_f = ldw_enable;
		ldw_param.alarm_f = ldw_alarm;
		ldw_param.getgValue = ldw_get_gsensor_gValue;	
		ldw_param.FCW_enable = ldw_FCW_enable;			
		ldw_param.GOALARM_enable = ldw_GOALARM_enable;			
		ldw_param.FCW_dispFlag = ldw_FCW_Disp_enable;	
		ldw_param.StopAndGOdispFlag = ldw_StopAndGO_Disp_enable;			
		gp_CVR_LDW_Set(&ldw_param);
		gp_CVR_LDW_Start();
	}
	
/* for console mode with No CVR UI, directly start video and audio recording */
	if (!opt.fifo)
	{
		g_bPause =0;
		g_FrameCount = 0;

		enable_file_saving(opt, &stream);
		enable_video_stream(&stream);
#ifndef DISABLE_AUDIO_STREAM
		if (opt.time_lapse != TIME_LAPSE_ON)
		{
			gp_IPC_ADev_Mute(audio_handle, opt.disable_audio);
			
			stream_list[0] = &stream;
			stream_list[1] = NULL;
			enable_audio(&stream_list[0]);
		}
#endif
	}	
		
/* notify UI for application ready */
	if (opt.fifo && !g_bRestart)
		dvr_pipemsg_send(CMD_READY_KEY, 0, NULL);

	g_bRestart = 0;

/*======================================
Main Loop & command parsing
======================================*/			
	while(1)
	{
		unsigned int msgId;
		void* msgPara = NULL;
		unsigned int mode;
		
		if (0 > gp_IPC_VStream_Query_Status())
		{
			g_bQuit = 1;
			g_bRestart = 1;
		}
		
		if (g_bQuit)
			break;
		
		/* receive UI command */
		if(g_bSaveFail || (opt.fifo && dvr_pipemsg_receive(&msgId, &msgPara) > 0))
		{
			if (msgId > DV_CMD_MIN && msgId < DV_CMD_MAX)
			{
				printf("msgId=%d\n", msgId);
				if (msgPara)
				{
					mode = *(unsigned int*)msgPara;
					printf("mode = %d\n", mode);
				}
			}
			
			/* Notify UI for SD Error or Disk Full */
			if (g_bSaveFail)
			{
				g_bSaveFail = 0;
				msgId = CMD_STOP_DV;
				if (stream.status == STATUS_DISK_ERROR)
					dvr_pipemsg_send(CMD_SET_SDC_ERROR, 0, NULL);
				else
					dvr_pipemsg_send(CMD_SET_MD_DV_STOP, 0, NULL); // disk full
			}
			
			/* handle UI command */
			switch(msgId)
			{
			case CMD_START_DV:
				md_wait_UI = 0;
				if (g_bPause)
				{
					g_FrameCount = 0;
					g_bPause = 0;

					if (0 > enable_file_saving(opt, &stream))
					{
						dvr_pipemsg_send(CMD_SET_MD_DV_STOP, 0, NULL);
						g_bPause = 1;
						break;
					}
						
					enable_video_stream(&stream);
#ifndef DISABLE_AUDIO_STREAM
					if (opt.time_lapse != TIME_LAPSE_ON)
					{
						gp_IPC_ADev_Mute(audio_handle, opt.disable_audio);	
						stream_list[0] = &stream;
						stream_list[1] = NULL;
						enable_audio(&stream_list[0]);
					}
#endif
						
					if (opt.motion_detect)
					{
						pthread_mutex_lock(&md_mutex);
						gettimeofday(&md_time, NULL);
						pthread_mutex_unlock(&md_mutex);
					}
					printf("++++++++++DV Started++++++++++\n");
				}
				break;
			case CMD_STOP_DV:
				md_wait_UI = 0;
				if (!g_bPause)
				{
					g_bPause = 1;
					pthread_join(stream.pthread, NULL);
#ifndef DISABLE_AUDIO_STREAM
					if (!opt.disable_audio && opt.time_lapse != TIME_LAPSE_ON)
						pthread_join(audio_pthread, NULL);
					
					disable_file_saving(&stream);
					if (opt.time_lapse != TIME_LAPSE_ON)
						disable_audio();
#else
					disable_file_saving(&stream);
#endif
					disable_video_stream(&stream);
					printf("++++++++++DV Stopped++++++++++\n");
				}
				break;
			case CMD_SET_DIR:
				sprintf(opt.outfile, "%s", (char*)msgPara);
				strcpy(cam_working_path, (char*)msgPara);
				printf("[dvr] get output path = %s\n", opt.outfile);
				break;
			case CMD_SET_DV_RESOLUTION:
				fps = 30;
				if (mode == 0)	{ w = 1920; h = 1080; bitrate = BITRATE_1080P; 	rsl = IPC_RESOLUTION_1080P; }
				else if (mode == 1 || mode == 5){ w = 1280; h = 720; bitrate = BITRATE_720P*2; rsl = IPC_RESOLUTION_720P; fps *=2;}
				else if (mode == 2)	{ w = 1280; h = 720; bitrate = BITRATE_720P; rsl = IPC_RESOLUTION_720P; }
				else if (mode == 3)	{ w = 848; h = 480; bitrate = BITRATE_480P; rsl = IPC_RESOLUTION_WVGA; }
				else if (mode == 4)	{ w = 640; h = 480; bitrate = BITRATE_480P; rsl = IPC_RESOLUTION_VGA; }
				
				if (CHECK_CHANGE(mode == 5, opt.real60fps))
				{
					opt.real60fps = (mode == 5)? 1:0;
					g_bRestart = 1;
					g_bQuit = 1;
				}

				if (stream.attr.rsl != rsl)
				{
					if (CHECK_CHANGE(ldw_en, (opt.ldw && (rsl == IPC_RESOLUTION_1080P))))
					{
						g_bRestart = 1;
						g_bQuit = 1;
					}
					if (md_handle >= 0 && opt.motion_detect)
					{
						disable_motion_detect(md_handle);
						md_handle = -1;
					}
					stream.width = w;
					stream.height = h;
					stream.attr.bitrate = bitrate;
					stream.attr.rsl = rsl;
					stream.attr.framerate = fps;
					
					if (!g_bRestart)
					{
						if (opt.display)
						{
							if (((rsl == IPC_RESOLUTION_VGA) ? 1: 0) != disp4x3)
							{
								disp4x3 = (rsl == IPC_RESOLUTION_VGA) ? 1: 0 ;
								if (opt.display == 1)
								{
									gp_IPC_Disable_Display();
									gp_IPC_Enable_Display(display_skip, DISP_SCALE_TYPE, disp4x3);
								}
								else
								{
									gp_IPC_Disable_Display();
									gp_IPC_Enable_Display2(display_skip, DISP_SCALE_TYPE, disp4x3, &disp_info);
								}
							}
						}
                    	
						gp_IPC_VStream_Set_Resolution(rsl);
					}
	
					printf("[dvr] change mode to %d x %d\n", stream.width, stream.height);
					
					if (opt.motion_detect)
						md_handle = enable_motion_detect();
				}
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
				//if (!opt.motion_detect && mode)
				if (md_handle < 0 && mode)
				{
					md_handle = enable_motion_detect();
					opt.motion_detect = 1;
				}
				//else if (opt.motion_detect && !mode)
				else if (md_handle >=0 && !mode)
				{
					disable_motion_detect(md_handle);
					opt.motion_detect = 0;
					md_handle = -1;
				}
				break;
			case CMD_SET_RECORD_AUDIO:
				opt.disable_audio = mode ? 0:1;
				if (mode)
					printf("[dvr] enable audio recording\n");
				else
					printf("[dvr] disable audio recording\n");
#ifndef DISABLE_AUDIO_STREAM
				if (opt.time_lapse != TIME_LAPSE_ON)
					gp_IPC_ADev_Mute(audio_handle, opt.disable_audio);
#endif
				break;
			case CMD_SET_DV_DATE_STAMP:
				opt.time_stamp = mode;
				gp_IPC_Enable_TimeStamp(mode ? 2:0, opt.time_format);
				if (mode)
					printf("[dvr] enable time stamp\n");
				else
					printf("[dvr] disable time stamp\n");
				break;
			case CMD_SET_DATE_TYPE:
				opt.time_format = mode;
				printf("[dvr] set time format = %d\n", mode);
				if (opt.time_stamp)
					gp_IPC_Enable_TimeStamp(2, opt.time_format);
				break;
			case CMD_SET_EXPOSURE:
				gp_IPC_VDev_Set_Exposure(csi_attr, mode);
				printf("[dvr] set Exposure: %d\n", mode);
				opt.ev = mode;
				break;
			case CMD_SET_FREQUENCY:
				global_frequency = mode;
				gp_IPC_VDev_Set_Frequency(csi_attr, mode);
				printf("[dvr] set Frequency: %d\n", mode);
				break;
			case CMD_SET_ZOOM:
				mode = mode ? mode + 10: 0;
				gp_IPC_Set_Zoom(mode);
				break;
			case CMD_SET_LOCK_FILE:
				if (!g_bPause)
					storageFileLock(&stream);
				break;
			case CMD_SET_DEFAULT_SETTING:
				if (ldw_en || opt.real60fps)
				{
					g_bRestart = 1;
					g_bQuit = 1;
				}
				dv_reset(&stream, &opt);
				gp_IPC_VStream_Set_Resolution(stream.attr.rsl);
				
				if (opt.display && !g_bRestart)
				{
					if (disp4x3)
					{
						disp4x3 = 0;
						if (opt.display == 1)
						{
							gp_IPC_Disable_Display();
							gp_IPC_Enable_Display(0, DISP_SCALE_TYPE, disp4x3);
						}
						else
						{
							gp_IPC_Disable_Display();
							gp_IPC_Enable_Display2(display_skip, DISP_SCALE_TYPE, disp4x3, &disp_info);
						}
					}
				}

				//if (opt.motion_detect)
				if (md_handle >= 0 && !opt.motion_detect)
				{
					disable_motion_detect(md_handle);
					md_handle = -1;
				}
				gp_IPC_Enable_TimeStamp(opt.time_stamp ? 2:0, opt.time_format);
				gp_IPC_VDev_Set_Exposure(csi_attr, opt.ev);
				global_dv_reset = 1;
				break;
			case CMD_SET_LDW_EN:
				{
					LDW_ITEMS ldw;
					int change = 0;
					int sen;
					ldw = *(LDW_ITEMS*)msgPara;
					
					sen = ldw.LDW_Sensitivity;
					if (sen < 2)
						sen = (sen ? 0:1);
					if ((opt.ldw == 1 && ldw.LDW_Enable == 0)||
						(opt.ldw == 0 && ldw.LDW_Enable > 0))
					{
						opt.ldw = (ldw.LDW_Enable > 0) ? 1:0;
						
						if (stream.attr.rsl <= IPC_RESOLUTION_720P)
						{
							g_bRestart = 1;
							g_bQuit = 1;
						}
					}
					
					if (ldw.LDW_Enable && opt.ldw_mode != ldw.LDW_Enable-1)
					{
						opt.ldw_mode = ldw.LDW_Enable-1;
						change = 1;
					}
					if (opt.ldw_sen != sen)
					{
						opt.ldw_sen = sen;
						change = 1;
					}
					if (opt.ldw_region != ldw.LDW_Area_Choice)
					{
						opt.ldw_region = ldw.LDW_Area_Choice;
						change = 1;
					}
					if (opt.ldw_speed != ldw.LDW_Speed)
					{
						opt.ldw_speed = ldw.LDW_Speed;
						change = 1;
					}
					if (opt.ldw_sfcw!= ldw.LDW_SFCW)
					{
						opt.ldw_sfcw= ldw.LDW_SFCW;
						change = 1;
					}
					
					if (opt.ldw_StopAndGo!= ldw.LDW_StopAndGo)
					{
						opt.ldw_StopAndGo= ldw.LDW_StopAndGo;
						change = 1;
					}
					if (ldw_en && change && !g_bRestart)
					{
						LDW_param_t ldw_param;
						
						gp_CVR_LDW_Stop();
						
						ldw_param.mode = opt.ldw_mode;
						ldw_param.sensitivity = opt.ldw_sen;
						ldw_param.region = opt.ldw_region;
						ldw_param.turn_on_speed = opt.ldw_speed;
						ldw_param.sfcw=opt.ldw_sfcw;
						ldw_param.StopAndGo=opt.ldw_StopAndGo;
						ldw_param.enable_f = ldw_enable;
						ldw_param.alarm_f = ldw_alarm;
						ldw_param.getgValue = ldw_get_gsensor_gValue;	
						ldw_param.FCW_enable = ldw_FCW_enable;							
						ldw_param.GOALARM_enable = ldw_GOALARM_enable;	
						ldw_param.FCW_dispFlag = ldw_FCW_Disp_enable;	
						ldw_param.StopAndGOdispFlag = ldw_StopAndGO_Disp_enable;	
						gp_CVR_LDW_Set(&ldw_param);
						gp_CVR_LDW_Start();
					}
				}
				break;
			case CMD_SET_EXIT:
				g_bQuit = 1;
				break;

			case CMD_SET_TIME_LAPSE:
				switch(mode){
					case 0:
						opt.time_lapse = 0;
						opt.time_lapse_duration = 0;
						stream.attr.target_fps = 0;
						break;
					case 1:
						opt.time_lapse = 1;
						opt.time_lapse_duration = 100;						
						stream.attr.target_fps = 1000/opt.time_lapse_duration;
						break;	
					case 2:
						opt.time_lapse = 1;
						opt.time_lapse_duration = 200;						
						stream.attr.target_fps = 1000/opt.time_lapse_duration;
						break;	
					case 3:
						opt.time_lapse = 1;
						opt.time_lapse_duration = 500;						
						stream.attr.target_fps = 1000/opt.time_lapse_duration;
						break;	
				}
					
				
				printf("SetTimeLapse-[%d]-[%d]-\n",opt.time_lapse,opt.time_lapse_duration);	
				break;
				
			case CMD_GET_LDW_LINE:
				gp_CVR_LDW_GetLine(&ldw_get_displine);
				{
					unsigned char arg[36];
					arg[0]=ldw_get_displine.LT_alarmP_X/256;
					arg[1]=ldw_get_displine.LT_alarmP_X%256;
					arg[2]=ldw_get_displine.LT_alarmP_Y/256;
					arg[3]=ldw_get_displine.LT_alarmP_Y%256;
					arg[4]=ldw_get_displine.LB_alarmP_X/256;
					arg[5]=ldw_get_displine.LB_alarmP_X%256;
					arg[6]=ldw_get_displine.LB_alarmP_Y/256;
					arg[7]=ldw_get_displine.LB_alarmP_Y%256;
					
					arg[8]=ldw_get_displine.RT_alarmP_X/256;
					arg[9]=ldw_get_displine.RT_alarmP_X%256;
					arg[10]=ldw_get_displine.RT_alarmP_Y/256;
					arg[11]=ldw_get_displine.RT_alarmP_Y%256;
					arg[12]=ldw_get_displine.RB_alarmP_X/256;
					arg[13]=ldw_get_displine.RB_alarmP_X%256;
					arg[14]=ldw_get_displine.RB_alarmP_Y/256;
					arg[15]=ldw_get_displine.RB_alarmP_Y%256;

					arg[16]=ldw_get_displine.LTP_X/256;
					arg[17]=ldw_get_displine.LTP_X%256;
					arg[18]=ldw_get_displine.LTP_Y/256;
					arg[19]=ldw_get_displine.LTP_Y%256;
					arg[20]=ldw_get_displine.LBP_X/256;
					arg[21]=ldw_get_displine.LBP_X%256;
					arg[22]=ldw_get_displine.LBP_Y/256;
					arg[23]=ldw_get_displine.LBP_Y%256;

					arg[24]=ldw_get_displine.RTP_X/256;
					arg[25]=ldw_get_displine.RTP_X%256;
					arg[26]=ldw_get_displine.RTP_Y/256;
					arg[27]=ldw_get_displine.RTP_Y%256;
					arg[28]=ldw_get_displine.RBP_X/256;
					arg[29]=ldw_get_displine.RBP_X%256;
					arg[30]=ldw_get_displine.RBP_Y/256;
					arg[31]=ldw_get_displine.RBP_Y%256;

					arg[32]=ldw_get_displine.LLAlarmFlg;
					arg[33]=ldw_get_displine.LLcheckFlg;
					arg[34]=ldw_get_displine.RLAlarmFlg;
					arg[35]=ldw_get_displine.RLcheckFlg;
				
					dvr_pipemsg_send(CMD_GET_LDW_LINE2DISP, 36,arg);	
				}
				break;
			case CMD_SET_SENSOR_FLIP:
				mode = mode ? g_flip_on : g_flip_off;
				if(global_flip != mode) {
					global_flip = mode;
					gp_IPC_VDev_MirrorImage(csi_attr, global_flip);
					gp_IPC_VDev_FlipImage(csi_attr, global_flip);
				}
				break;
			case CMD_SET_DISPLAY_MODE:
				if (opt.display != mode+1)
				{
					if (opt.display == 2)
						disp_pipe_close();

					gp_IPC_Disable_Display();

					opt.display = mode+1;

					if (opt.display == 1)
						gp_IPC_Enable_Display(display_skip, DISP_SCALE_TYPE, disp4x3);
					else if (opt.display == 2)
					{
						disp_pipe_init();
						gp_IPC_Enable_Display2(display_skip, DISP_SCALE_TYPE, disp4x3, &disp_info);
					}
				}
				break;
			}
		
			if (msgId > DV_CMD_MIN && msgId < DV_CMD_MAX && msgId != CMD_SET_EXIT)// && msgId != CMD_SET_LDW_EN)
			{
				dvr_response_cmd(msgId);
				msgId = 0;
			}
		}
		
		/* check LDW status, and notify UI for status change */
		if (opt.ldw)
		{
			if (g_bLDW_OFF)
			{
				printf("-----------------------------------------------------LDW OFF\n");
				g_bLDW_OFF = 0;
				dvr_pipemsg_send(CMD_SET_LDW_LOW_SPEED, 0, NULL);
			}
			
			if (g_bLDW_ON)
			{
				printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++LDW ON\n");
				g_bLDW_ON = 0;
				dvr_pipemsg_send(CMD_SET_LDW_HIGH_SPEED, 0, NULL);
			}
			
			if (g_bLDW_ALARM)
			{
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!LDW ALARM\n");
				g_bLDW_ALARM = 0;
				dvr_pipemsg_send(CMD_SET_LDW_INT, 0, NULL);
			}
			if (g_bLDW_FCW_ON)		
			{		
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!LDW FCW ALARM\n");
				g_bLDW_FCW_ON = 0;				
				dvr_pipemsg_send(CMD_SET_LDW_FCW, 0, NULL);	
			}						

			if (g_bLDW_STOPALARM_ON)			
			{				
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!LDW STOP ALARM\n");	
				g_bLDW_STOPALARM_ON = 0;			
				dvr_pipemsg_send(CMD_SET_LDW_STOPALRAM, 0, NULL);	
			}		
			if (g_bLDW_GOALARM_ON)		
			{				
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!LDW GO ALARM\n");	
				g_bLDW_GOALARM_ON = 0;			
				dvr_pipemsg_send(CMD_SET_LDW_GOALRAM, 0, NULL);	
			}			
			////////////////
			if (g_bLDW_FCW_flag!=g_bLDW_FCW_flag_bak)		
			{				
				char arg;

				g_bLDW_FCW_flag_bak = g_bLDW_FCW_flag;
				arg = g_bLDW_FCW_flag;
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FCW -%d-\n",g_bLDW_FCW_flag);	
				dvr_pipemsg_send(CMD_SET_LDW_FCW_FLAG, 1, &arg);	
			}	
			if (g_bLDW_StopAndGo_flag!=g_bLDW_StopAndGo_flag_bak)		
			{				
				char arg;

				g_bLDW_StopAndGo_flag_bak = g_bLDW_StopAndGo_flag;
				arg = g_bLDW_StopAndGo_flag;
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!stop&&go -%d-\n",g_bLDW_FCW_flag);	
				dvr_pipemsg_send(CMD_SET_LDW_STOPandGOALRAM_FLAG, 1, &arg);	
			}	
		}
		
		/* check Motion Detect status, stop recording after 10 sec with no detection */
		if (!g_bQuit && opt.motion_detect)
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
				pthread_join(stream.pthread, NULL);
#ifndef DISABLE_AUDIO_STREAM
				if (!opt.disable_audio && (opt.time_lapse != TIME_LAPSE_ON))
					pthread_join(audio_pthread, NULL);
						
				disable_file_saving(&stream);
				if (opt.time_lapse != TIME_LAPSE_ON)
					disable_audio();
#else
				disable_file_saving(&stream);
#endif
				disable_video_stream(&stream);
				printf("++++++++++DV Stopped++++++++++\n");
				dvr_pipemsg_send(CMD_SET_MD_DV_STOP, 0, NULL);
			}
			else if (!md_wait_UI && detected && g_bPause)
			{
				md_wait_UI = 1;
				printf("send CMD_SET_MD_DV_START\n");
				dvr_pipemsg_send(CMD_SET_MD_DV_START, 0, NULL);
			}
			
		}
		
		/* Check loop recording file change */
		if (!g_bQuit && opt.fifo && storageFileChangeQuery(&stream) > 0)
		{
			int Loop_lock_next_file = storageFileLockQuery(&stream);
			dvr_pipemsg_send(CMD_SET_LOOPRECORDING_INT, 1, &Loop_lock_next_file);
		}
		//sleep(1);
		usleep(1000);
	}
/*======================================
Exit DV
======================================*/
	if (!opt.fifo || !g_bPause)
	{
		pthread_join(stream.pthread, NULL);
#ifndef DISABLE_AUDIO_STREAM
		if (!opt.disable_audio && opt.time_lapse != TIME_LAPSE_ON)
		{
			pthread_join(audio_pthread, NULL);
			disable_audio();
		}
#endif
		disable_video_stream(&stream);
		disable_file_saving(&stream);
	}
	
	storageServiceClose();
	
	if (opt.motion_detect)
	{
		gd_md_close();
		pthread_mutex_destroy(&md_mutex);
	}

	gp_IPC_Enable_TimeStamp(0, 0);
	gp_IPC_Disable_Display();		
	gp_IPC_VDev_Close(csi_attr);
	
	//gp_IPC_VStream_Close();
#ifndef DISABLE_AUDIO_STREAM
	if (opt.time_lapse != TIME_LAPSE_ON)
		gp_IPC_ADev_Close(audio_handle);
#endif

	if (opt.display == 2)
		disp_pipe_close();
	
	if (g_bRestart)
		goto start_dvr;

	if (!opt.real60fps)
		gp_IPC_VStream_Set_Resolution(IPC_RESOLUTION_1080P);
		
	if(opt.fifo)
		dvr_response_cmd(CMD_SET_EXIT);	

	dvr_pipe_close();

	global_ev = opt.ev;
	global_time_format = opt.time_format;
	global_ldw = ldw_en;
	global_60fps = opt.real60fps;
		
	printf("[camcorder] exit dv mode finish\n");
	return 0;
}
