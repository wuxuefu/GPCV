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
 * @file stream.h
 * @brief stream header file
 * @author 
 */

#ifndef _STREAM_H_
#define _STREAM_H_

#include "storage.h"
//#include "list.h"
#include <pthread.h>
#include "gp_video_stream_api.h"
#include "gp_audio_stream_api.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define SFLAG_STREAM_TO_NET	0x1		//stream to net, not implement
#define SFLAG_SAVE_TO_FILE	0x2		//save to sd card
#define SFLAG_SENT_TO_DISPLAY 0x4	//send to display, no use right now.

#define DEFAULT_FRAME_RATE	25
#define VIDEO_STABILIZATION	0

#define STATUS_OK				0
#define STATUS_DISK_FULL		1
#define STATUS_FILE_SIZE_LIMIT	2
#define STATUS_DISK_ERROR		3

#define CACHE_NUM	360
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct streamInfo_s {
	void* handle;
	IPC_Chn_Attr_s attr;			//video stream attribute for pass to gp_video_encode API
	char *name;						//stream name, no use right now
	int width;						//video width
	int height;						//video height
	unsigned int flag;				//function flag, the value SFLAG_* is defined above, 
	unsigned int status;			//stream status, the value STATUS_* is defined above
	storageHandle_t *storageHandle;	//pointer to storage service
	int frame_count;				
	int display;
	pthread_t pthread;				//pthread for VideoTask 
	pthread_mutex_t buf_mutex;		//pthread mutex for frame buffer.
} streamInfo_t;

typedef struct video_buffer_s
{
	IPC_Video_Frame_s* frame;		//video frame get from gp_video_encode library
	int ref;						//reference count
	streamInfo_t* info;				//pointer to stream info
}video_buffer_t;

typedef struct audio_buffer_s
{
	IPC_Audio_Frame_s* frame;		//audio frame get from gp_video_encode library
	int ref;						//reference cout
	pthread_mutex_t* pmutex;		//mutex for audio buffer
}audio_buffer_t;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
void vbuf_add_ref(video_buffer_t* buf);
void vbuf_free(video_buffer_t* buf);
void abuf_add_ref(audio_buffer_t* buf);
void abuf_free(audio_buffer_t* buf);
//extern struct list_head gStreamList;

#endif //endif _STREAM_H_
