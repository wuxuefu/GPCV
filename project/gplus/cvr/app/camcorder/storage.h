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
 * @file storage.h
 * @brief storage header file
 * @author 
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <sys/time.h>
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
enum {
	STORAGE_CMD_WRITE = 0,
	STORAGE_CMD_WRITE_AUDIO,
	STORAGE_CMD_FINISH,
	STORAGE_CMD_LOCK,
	STORAGE_CMD_CLOSE,
};

//custom muxer commands 
#define MUX_LOCK_FILE 		0x1000
#define MUX_CHANGE_FILE 	0x1001
#define MUX_REQUEST_HEADER	0x1002
#define MUX_FILENAME_FORMAT	0x1003
#define MUX_RESERVE_SIZE	0x1004

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct storageHandle_s {
	FILE *file;
	int hdMux;
	unsigned int last_pts;
	int framerate;
	struct timeval tv1;
	struct timeval tv2;
	unsigned int total_time;
	unsigned int total_frame;
	char *filename;
	int loop_file_time;
	int audio_codec;
	int video_codec;
	int time_lapse;
	int loop_file_name_format;
} storageHandle_t;

typedef struct storageMsg_s {
	int cmd;
	void *handle;
	void* buffer;
	unsigned int nBytes;
	unsigned int pts;
	unsigned int frameType;
} storageMsg_t;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
int storageServiceInit(int cType, int loop);
void storageServiceClose(void);

void *storageFileOpen(char *name, unsigned int width, unsigned int height, int framerate, 
						int file_time, int video_codec, int audio_codec, int time_lapse, int loop_file_name_format);
void storageFileClose(void *storageHandle);
void storageFileWrite(void *streamHandle, void* buffer);
void storageFileWriteAudio(void *streamHandle, void* buffer);
void storageFileLock(void *streamHandle);
void storageFileFinish(void *streamHandle);
int storageFileChangeQuery(void *streamHandle);
int storageFileRequestHeaderQuery(void *streamHandle);
int storageFileLockQuery(void *streamHandle);

#endif //endif _STORAGE_H_
