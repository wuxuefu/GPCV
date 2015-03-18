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
 * @file storage.c
 * @brief storage function
 * @author 
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
#include <sys/stat.h>

#include <pthread.h>
#include <mqueue.h>

#include "mach/typedef.h"

#include "stream.h"
#include "storage.h"

#include <sys/statfs.h>
#include "gp_mux.h"
//#include "vid_mp4_packer.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
//#define OUT_DEVICE "/media/sdcardc1/"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/



/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static pthread_t storageThread;
static mqd_t storageMsgQ;
static struct mq_attr mq_attr = {
	.mq_maxmsg = 4,
	.mq_msgsize = sizeof(storageMsg_t)
};
int g_loop_enable =0;
//extern struct gpMux_s MP4_packer;
//extern struct gpMux_s MOV_packer;
//extern struct gpMux_s AVI_packer;
extern struct gpMux_s loop_packer;
struct gpMux_s *g_save_Mux;
int g_cType = GP_CS_TYPE_UNKNOWN;

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
static int
ContainerInit(
	storageHandle_t *storageHandle,
	unsigned int width,
	unsigned int height
)
{
	struct statfs infoDisk;
	UINT64 freeDiskSize = 0;
	int audio_type, audio_bits, samplerate;
	char* out_path;
	int i;
	int reserve_size = 0;
	
	out_path = strdup(storageHandle->filename);
	
	for (i = strlen(out_path)-1; i >= 0; i--)
	{
		if (out_path[i] == '/')
		{
			out_path[i+1] = 0;
			break;
		}
	}
	
	if (i < 0)
	{
		free(out_path);
		out_path = strdup("./");
	}
	
	g_save_Mux->set(NULL, MUX_FILENAME_FORMAT, storageHandle->loop_file_name_format);

	if(g_loop_enable)
   		storageHandle->hdMux = g_save_Mux->open(storageHandle->filename);
	else
		storageHandle->hdMux = g_save_Mux->open(storageHandle->file);
	if (storageHandle->hdMux <= 0) {
		printf("mux open failed.\n");
		free(out_path);
		return -1;
	}

	if (statfs(out_path, &infoDisk) != 0) {
		printf("[%s:%d] statfs fail.\n", __FUNCTION__, __LINE__);
		free(out_path);
		return -1;
	}

	freeDiskSize = (UINT64)infoDisk.f_bavail * (UINT64)infoDisk.f_bsize;
	if (freeDiskSize > 0x500000)
		freeDiskSize -= 0x500000;
	else
		freeDiskSize = 0;
	if (freeDiskSize > MP4_MAX_FILE_SIZE)
		freeDiskSize = MP4_MAX_FILE_SIZE;

	g_save_Mux->set(storageHandle->hdMux, MUX_MAX_SIZE, (UINT32)freeDiskSize);

	if (storageHandle->video_codec == CODEC_ID_MJPEG)
		g_save_Mux->set(storageHandle->hdMux, MUX_VID_TYPE, VIDEO_TYPE_MJPEG);
	else
		g_save_Mux->set(storageHandle->hdMux, MUX_VID_TYPE, VIDEO_TYPE_H264_BP);

	g_save_Mux->set(storageHandle->hdMux, MUX_PATH_TEMP_FILE, (UINT32)out_path);
	g_save_Mux->set(storageHandle->hdMux, MUX_HEIGHT, height);
	g_save_Mux->set(storageHandle->hdMux, MUX_WIDTH, width);
	g_save_Mux->set(storageHandle->hdMux, MUX_FRMRATE, storageHandle->framerate);
#ifndef DISABLE_AUDIO_STREAM	
	switch(storageHandle->audio_codec)
	{
	case CODEC_ID_AAC:			audio_type = AUDIO_TYPE_AAC;	audio_bits = 16;	samplerate = 16000;	break;
	case CODEC_ID_PCM_MULAW:	audio_type = AUD_FORMAT_MULAW;	audio_bits = 8;		samplerate = 8000;	break;
	case CODEC_ID_PCM_ALAW:		audio_type = AUD_FORMAT_ALAW;	audio_bits = 8;		samplerate = 8000;	break;
	case CODEC_ID_ADPCM_G726:	audio_type = AUD_FORMAT_G726;	audio_bits = 4;		samplerate = 8000;	break;
	case CODEC_ID_PCM_S16LE:	audio_type = AUD_FORMAT_PCM;	audio_bits = 16;	samplerate = 8000;	break;
	}
	if (storageHandle->audio_codec != CODEC_ID_NONE)
	{
		g_save_Mux->set(storageHandle->hdMux, MUX_AUD_TYPE, audio_type);
		g_save_Mux->set(storageHandle->hdMux, MUX_AUD_CHANNEL, 1);
		g_save_Mux->set(storageHandle->hdMux, MUX_AUDSR, samplerate);
		g_save_Mux->set(storageHandle->hdMux, MUX_AUD_BIT, audio_bits);
	}
#endif
	g_save_Mux->set(storageHandle->hdMux, MUX_CLUSTER_SIZE, infoDisk.f_bsize);
	g_save_Mux->set(storageHandle->hdMux, MUX_FILE_TIME, storageHandle->loop_file_time*60);
	
	if (storageHandle->loop_file_time < 10)
	{
		switch(height)
		{
		case 1080:	reserve_size = 0x2A0000;	break;
		case 720:	reserve_size = 0x120000;	break;
		case 480:	reserve_size = 0xA0000;		break;
		default:	break;
		}
		if (storageHandle->video_codec == CODEC_ID_MJPEG)
			reserve_size *= 1.5;	
		reserve_size*= (60*storageHandle->loop_file_time);
		g_save_Mux->set(storageHandle->hdMux, MUX_RESERVE_SIZE, reserve_size);
	}
	
	free(out_path);

	return 0;
}

void *
storageFileOpen(
	char *name,
	unsigned int width,
	unsigned int height,
	int framerate,
	int loop_file_time,
	int video_codec,
	int audio_codec,
	int time_lapse,
	int loop_file_name_format
)
{
	storageHandle_t *storageHandle;
	char filename[256], ext[4];
	unsigned int len;

	storageHandle = (storageHandle_t *) malloc(sizeof(*storageHandle));
	if (!storageHandle) {
		printf("[%s:%d], malloc fail.\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	memset((void*) storageHandle, 0, sizeof(*storageHandle));

	//if (g_cType == GP_CS_TYPE_MP4)
	//	sprintf(ext, ".mp4");
	//else if (g_cType == GP_CS_TYPE_AVI)
	//	sprintf(ext, ".avi");
	//snprintf(filename, sizeof(filename), "%s%s%s", OUT_DEVICE, name, ext);
	strcpy(filename, name);
	storageHandle->filename = strdup(name);
	if(!g_loop_enable)
	{
		storageHandle->file = fopen(filename, "w+b");
		if (!storageHandle->file) {
			printf("[%s:%d], open file %s fail.\n", __FUNCTION__, __LINE__, filename);
			free(storageHandle);
			return NULL;
		}
		printf("[%s:%d], open file %s.\n", __FUNCTION__, __LINE__, filename);
	}
	storageHandle->loop_file_time = loop_file_time;
	storageHandle->video_codec = video_codec;
	storageHandle->audio_codec = audio_codec;
	storageHandle->loop_file_name_format = loop_file_name_format;
	/* check ext name */
	//len = strlen(name);
	//if (name[len-3] == 'm' && name[len-2] == 'p' && name[len-1] == '4') {
	{
		if (ContainerInit(storageHandle, width, height) < 0) {
			if(!g_loop_enable)
				fclose(storageHandle->file);
			free(storageHandle);
			return NULL;
		}
	}
	
	storageHandle->last_pts = 0;
	storageHandle->framerate = framerate;
	storageHandle->time_lapse = time_lapse;
	
	return (void*) storageHandle;
}

void
storageFileClose(
	void *handle
)
{
	storageHandle_t *storageHandle = (storageHandle_t *) handle;

	if (storageHandle->hdMux) {
		g_save_Mux->close(storageHandle->hdMux);
	}
	fclose(storageHandle->file);
	printf("[%s:%d], close file %s.\n", __FUNCTION__, __LINE__);
	if(storageHandle->filename)
		free(storageHandle->filename);
	free(storageHandle);
	sync();
}

void
storageFileWrite(
	void *streamHandle,
	void* buffer

)
{
	storageMsg_t msg;
	video_buffer_t* vbuf = (video_buffer_t*)buffer;

	if (!((streamInfo_t*)streamHandle)->storageHandle)
		return;
	
	vbuf_add_ref(vbuf);
	
	msg.cmd = STORAGE_CMD_WRITE;
	msg.handle = streamHandle;
	msg.buffer = vbuf;
	msg.nBytes = vbuf->frame->size;
	msg.pts = vbuf->frame->pts;
	msg.frameType = vbuf->frame->frameType;
	mq_send(storageMsgQ, (const char *) &msg, sizeof(storageMsg_t), 0);
}

void
storageFileWriteAudio(
	void *streamHandle,
	void* buffer
)
{
	storageMsg_t msg;
	audio_buffer_t* abuf = (audio_buffer_t*)buffer;

	if (!((streamInfo_t*)streamHandle)->storageHandle)
		return;
	
	abuf_add_ref(abuf);
	
	msg.cmd = STORAGE_CMD_WRITE_AUDIO;
	msg.handle = streamHandle;
	msg.buffer = abuf;
	msg.nBytes = abuf->frame->size;
	msg.pts = abuf->frame->duration;
	mq_send(storageMsgQ, (const char *) &msg, sizeof(storageMsg_t), 0);
}

void
storageFileLock(
	void *streamHandle
)
{
	storageMsg_t msg;
	if (!((streamInfo_t*)streamHandle)->storageHandle)
		return;
		
	msg.cmd = STORAGE_CMD_LOCK;
	msg.handle = streamHandle;
	mq_send(storageMsgQ, (const char *) &msg, sizeof(storageMsg_t), 0);	
}

void
storageFileFinish(
	void *streamHandle
)
{
	storageMsg_t msg;
	if (!((streamInfo_t*)streamHandle)->storageHandle)
		return;

	printf("%s\n", __FUNCTION__);
	msg.cmd = STORAGE_CMD_FINISH;
	msg.handle = streamHandle;
	mq_send(storageMsgQ, (const char *) &msg, sizeof(storageMsg_t), 0);	
}

int
storageFileChangeQuery(
	void *streamHandle
)
{
	storageHandle_t *storageHandle = (storageHandle_t*) ((streamInfo_t *)streamHandle)->storageHandle;
	int file_change = -1;
	
	if(storageHandle && storageHandle->hdMux)
		g_save_Mux->get(storageHandle->hdMux, MUX_CHANGE_FILE, &file_change);
		
	return file_change;
}

int
storageFileLockQuery(
	void* streamHandle
)
{
	storageHandle_t *storageHandle = (storageHandle_t*) ((streamInfo_t *)streamHandle)->storageHandle;
	int file_lock = -1;
	
	if(storageHandle && storageHandle->hdMux)
		g_save_Mux->get(storageHandle->hdMux, MUX_LOCK_FILE, &file_lock);
		
	return file_lock;
}

int
storageFileRequestHeaderQuery(
	void *streamHandle
)
{
	storageHandle_t *storageHandle = (storageHandle_t*) ((streamInfo_t *)streamHandle)->storageHandle;
	int request_header = -1;
	
	if(storageHandle && storageHandle->hdMux)
		g_save_Mux->get(storageHandle->hdMux, MUX_REQUEST_HEADER, &request_header);
		
	return request_header;
}
void
storageTask(
	void *param
)
{
	int ret;
	storageMsg_t msgData;
	storageMsg_t *msg = &msgData;
	unsigned int msg_prio;
	//int is_finish =0;
	printf("\nStorage thread enter ...\n\n");

	while (1) {
		ret = mq_receive(storageMsgQ, (char *) msg, sizeof(storageMsg_t), &msg_prio);
		if (ret < 0) {
			printf("[%s:%d], storage receive msg fail.\n", __FUNCTION__, __LINE__);
			continue;
		}

		//printf("receive storage msg %d, 0x%x\n", msg->cmd, msg->handle);
		if(msg->cmd == STORAGE_CMD_WRITE || msg->cmd == STORAGE_CMD_WRITE_AUDIO) {
			streamInfo_t *streamHandle = (streamInfo_t *) msg->handle;
			storageHandle_t *storageHandle = (storageHandle_t*) streamHandle->storageHandle;
			gpMuxPkt_t MuxPkt;
			
			int isAudio = (msg->cmd == STORAGE_CMD_WRITE_AUDIO) ? 1: 0;

			if (storageHandle && storageHandle->hdMux) {
				ret = 0;
				if (isAudio)
				{
					MuxPkt.data = (UINT8*)((audio_buffer_t*)(msg->buffer))->frame->pAddr;
					MuxPkt.size = msg->nBytes;
					MuxPkt.pts = msg->pts;//AUDIO_BUFFER_TIME;
					//printf("audio time =%d\n",msg->pts);
					if(storageHandle->hdMux)
						ret = g_save_Mux->pack(storageHandle->hdMux, &MuxPkt, GP_ES_TYPE_AUDIO);
						
					abuf_free((audio_buffer_t*)(msg->buffer));
				}
				else
				{
					IPC_Video_Frame_s* frame = ((video_buffer_t*)(msg->buffer))->frame;
					if (frame->thumb && storageHandle->hdMux)
					{
						MuxPkt.data = frame->thumb;
						MuxPkt.size = frame->thumb_size;
						ret = g_save_Mux->pack(storageHandle->hdMux, &MuxPkt, GP_ES_TYPE_THUMB);
					}
					MuxPkt.data = frame->pFrameAddr;
					MuxPkt.size = msg->nBytes;
					
					if (!storageHandle->time_lapse && (storageHandle->last_pts > 0) && (msg->pts > storageHandle->last_pts))
						MuxPkt.pts = msg->pts - storageHandle->last_pts;
					else
						MuxPkt.pts = 1000/storageHandle->framerate;
						
					if (MuxPkt.pts > 3600*1000) //avoid unexpected frame time 
					{
						printf("unexpected large frame time %u\n", MuxPkt.pts);
						MuxPkt.pts = 1000/storageHandle->framerate;
					}
					
					storageHandle->last_pts = msg->pts;
					MuxPkt.frameType = msg->frameType;
					if(storageHandle->hdMux)
						ret = g_save_Mux->pack(storageHandle->hdMux, &MuxPkt, GP_ES_TYPE_VIDEO);
						
					vbuf_free((video_buffer_t*)(msg->buffer));
				}
				
				if (ret != 0)
				{
					if (ret == MUX_MEM_FULL)
					{
						printf("[dvr] disk is full, stop recording.\n");
						streamHandle->status = STATUS_DISK_FULL;
					}
					else if (ret == MUX_FILE_SIZE_REACH)
					{
						printf("[dvr] reach maximum file size (2G), stop recording.\n");
						streamHandle->status = STATUS_FILE_SIZE_LIMIT;
					}
					else
					{
						printf("[dvr] unknown file error. stop recording.\n");
						streamHandle->status = STATUS_DISK_ERROR;
					}
					g_save_Mux->close(storageHandle->hdMux);
					storageHandle->hdMux = 0;
					if(storageHandle->file)
						fclose(storageHandle->file);
					free(storageHandle);
					streamHandle->storageHandle = NULL;
					sync();
				}
			}
			else
			{
				if (isAudio)
					abuf_free((audio_buffer_t*)(msg->buffer));
				else
					vbuf_free((video_buffer_t*)(msg->buffer));
			}
		}
		else if (msg->cmd == STORAGE_CMD_LOCK) {
			streamInfo_t *streamHandle = (streamInfo_t *) msg->handle;
			storageHandle_t *storageHandle = (storageHandle_t*) streamHandle->storageHandle;
			
			if(storageHandle->hdMux)
				g_save_Mux->set(storageHandle->hdMux, MUX_LOCK_FILE, 0);
		}
		else if (msg->cmd == STORAGE_CMD_FINISH) {
			streamInfo_t *streamHandle = (streamInfo_t *) msg->handle;
			storageHandle_t *storageHandle = (storageHandle_t*) streamHandle->storageHandle;
			//is_finish =1;
			if (storageHandle){
				if (storageHandle->hdMux) {
					g_save_Mux->close(storageHandle->hdMux);
					storageHandle->hdMux = 0;
				}
				if(storageHandle->file)
					fclose(storageHandle->file);
            	
				free(storageHandle);
				streamHandle->storageHandle = NULL;
				sync(); 
			}
		}
		else if (msg->cmd == STORAGE_CMD_CLOSE) {
			mq_close(storageMsgQ);
			break;
		}
	}
	printf("\nStorage thread exit ...\n\n");
}

int
storageServiceInit(
	int cType,
	int loop
)
{
	/* init message Q */
	mq_unlink("/storageMsgQ");
	storageMsgQ = mq_open("/storageMsgQ", O_CREAT | O_RDWR, S_IREAD | S_IWRITE, &mq_attr);
	if (storageMsgQ == (mqd_t) -1) {
		perror("storage thread msg queue open fail.\n");
		return -1;
	}

	/* init thread */
	if (pthread_create(&storageThread, NULL, (void*)storageTask, NULL) != 0) {
		printf("[%s:%d], storage input create thread fail.\n", __FUNCTION__, __LINE__);
		mq_close(storageMsgQ);
		return -1;
	}

	//if(g_loop_enable==1)
	g_loop_enable = 0;
	if (loop)
	{
		g_loop_enable = 1;
		g_save_Mux=&loop_packer;
		//cType = GP_CS_TYPE_MP4;
	}
	else if (cType == GP_CS_TYPE_MP4)
		g_save_Mux=&MP4_packer;
	else if (cType == GP_CS_TYPE_MOV)
		g_save_Mux=&MOV_packer;
	else if (cType == GP_CS_TYPE_AVI)
		g_save_Mux=&AVI_packer;
	else 
	{
		printf("unknown format type\n");
		return -1;
	}
	
	g_cType = cType;

	if(g_save_Mux->init)
		g_save_Mux->init();
	return 0;
}

void storageServiceClose()
{
	storageMsg_t msg;

	msg.cmd = STORAGE_CMD_CLOSE;
	msg.handle = 0;
	mq_send(storageMsgQ, (const char *) &msg, sizeof(storageMsg_t), 0);
	
	pthread_join(storageThread, NULL);
	if(g_save_Mux->uninit)
		g_save_Mux->uninit();
}