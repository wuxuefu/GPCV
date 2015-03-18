#ifndef __MEDIA_SYSTEM_API_H__
#define __MEDIA_SYSTEM_API_H__

#include "mediaSystem_typedef.h"

enum {
	MEDIA_MSG_EOA = 0x00000001,
	MEDIA_MSG_GET_WAVEFORM_DONE,
	MEDIA_MSG_ERR = 0x80000000,
};
enum {
	audPlayer_add_speed=0x01,
	audPlayer_add_voice=0x02,
	audPlayer_add_subtitle=0x04,
};

#ifdef __cplusplus
extern "C" {
#endif

// Create and Destroy
void *audPlayer_Create_Ex(int (*Callback)(void *param, int msg, int val), void *param, int dbgLevel,int fun);
void *audPlayer_Create(int (*Callback)(void *param, int msg, int val), void *param, int dbgLevel);
int audPlayer_Destroy(void *hMedia);

// Device Setup, used ONLY in STOP state
int audPlayer_SetAudioDevice(void *hMedia, const char *WaveOutDevice);

// Playback Setup, used ONLY in STOP state
int audPlayer_EnableStreamLevelLooping(void *hMedia, int enable);

int audPlayer_OpenFile(void *hMedia, const char *filename);
int audPlayer_CloseFile(void *hMedia);

// Play and Stop
int audPlayer_Play(void *hMedia, const char *file_name, int *ms);
int audPlayer_Stop(void *hMedia);

// Playback Setup, used both in PLAY and STOP state
int audPlayer_Pause(void *hMedia);
int audPlayer_Resume(void *hMedia);
int audPlayer_SetVolume(void *hMedia, int vol);
int audPlayer_GetPlayTime(void *hMedia, int *err);
int audPlayer_SetSpeed(void *hMedia, int Speed);
int audPlayer_AttachSubtitle(void *hMedia, const char *filename);
int audPlayer_DetachSubtitle(void *hMedia);
int audPlayer_GetAllStreamInfo(void *hMedia, int type, const GP_STREAM_INFO **Info);
int audPlayer_UseStream(void *hMedia, int type, int idx);

// Playback Setup, used ONLY in PLAY state
int audPlayer_Seek(void *hMedia, int *ms);

// Current Playing Stream Information, used ONLY in PLAY state
int audPlayer_GetStreamInfo(void *hMedia, GP_STREAM_INFO *Info);

int audPlayer_GetWaveform(void *hMedia, int buflen, void *buf, int *outlen);
int audPlayer_SetVoice(void *hMedia,int value);
//set EQ callback function
int audPlayer_SetEQFunction(void *hMedia,int (*callback)(const char *src,char *dest,int length));

#ifdef __cplusplus
}
#endif

#endif // __MEDIA_SYSTEM_API_H__
