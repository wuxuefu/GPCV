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
#include "ipcam_audio_api.h"
#include "gp_video_channel.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define SFLAG_STREAM_TO_NET	0x1
#define SFLAG_SAVE_TO_FILE	0x2
#define SFLAG_SENT_TO_DISPLAY 0x4

#define DEFAULT_FRAME_RATE	25
#define VIDEO_STABILIZATION	0
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct streamInfo_s {
	void* handle;
	IPC_Chn_Attr_s attr;
	char *name;
	int width;
	int height;
	unsigned int flag;
	storageHandle_t *storageHandle;
	int frame_count;
	int display;
	pthread_t pthread;
	pthread_mutex_t buf_mutex;
} streamInfo_t;

typedef struct video_buffer_s
{
	IPC_Video_Frame_s* frame;
	int ref;
	pthread_mutex_t* pmutex;
}video_buffer_t;

typedef struct audio_buffer_s
{
	IPC_Audio_Frame_s* frame;
	int ref;
	pthread_mutex_t* pmutex;
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
