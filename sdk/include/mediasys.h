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
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
 
#ifndef _MEDIA_SYSTEM_H_
#define _MEDIA_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

// Create/Destroy mediaSystem 
void *mediaSystem_Create(const char *pname);
int mediaSystem_Destroy(void *hMedia);


// Create Filter
void *mediaSystem_CreateFilter(void *hSys, const void *filterOp, const char *pname);


// Link Filter Path
void *mediaSystem_LinkPath(void *hMedia, const char *name, ...);


// FilterLinkPath Start and Stop
int mediaSystemPath_Stop(void *hFilterPath);
int mediaSystemPath_Start(void *hFilterPath);


// mediaSystem Start and Stop
int mediaSystem_Start(void *hSys);
int mediaSystem_Stop(void *hSys);


// Seek
int mediaSystem_Seek(void *hSys, int *ms);


// Callback function
void mediaSystem_SetCallback(
	void *hMedia,
	int (*callback)(void *param, int msg, int val),
	void *callback_param);
// Callback messages
enum {
	MEDIASYSTEM_MSG_END = 0x00000001,
	MEDIASYSTEM_MSG_GET_WAVEFORM_DONE = 0x00000002,
	MEDIASYSTEM_MSG_ERR = 0x80000000,
};


// Timing control
int mediaSystem_SetTime(void *hMedia, int time);
int mediaSystem_GetTime(void *hMedia, int *time);


// Debug
void mediaSystem_SetDebugLevel(void *hMedia, int dbgLevel);


// Send Command to Filter
int filter_SendCmd(void *hFilter, int cmd, int param);
// Command List
enum
{
	FILTER_WAVEOUT_CMD_PAUSE = 0,
	FILTER_WAVEOUT_CMD_RESUME,
	FILTER_WAVEOUT_CMD_SET_DEVICE,
	FILTER_WAVEOUT_CMD_SET_VOL,
	FILTER_WAVEOUT_CMD_ALLOW_TIME_BACK,
	FILTER_WAVEOUT_CMD_SET_EQFUNCTION,
};
enum
{
	FILTER_INSTREAM_CMD_LOAD = 0,
	FILTER_INSTREAM_CMD_INFO,
	FILTER_INSTREAM_CMD_TIME,
	FILTER_INSTREAM_CMD_SEEK,
	FILTER_INSTREAM_CMD_LOOP,
};
enum
{
	FILTER_AUDDEC_CMD_GET_WAVEFORM = 0,
};

typedef struct
{
	int bufmax;		// input  : max size of output buffer (in byte)
	char *buf;		// input  : pointer to output buffer
	int *outlen;	// output : data size in output buffer
} FILTER_AUDDEC_GET_WAVEFORM_S;

enum
{
	FILTER_SPEEDCTRL_CMD_SET_SPEED = 0,
};
enum
{
	FILTER_SUBTITLEOUT_CMD_SET_DEVICE = 0,
};
enum
{
	FILTER_VOICE_CMD_SET_VOICE =0,
	FILTER_VOICE_CMD_SET_SEEK =1,
};

#ifdef __cplusplus
}
#endif


#endif /* END */

