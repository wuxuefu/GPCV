#ifndef __GP_VIDEOENGINE_API_H__
#define __GP_VIDEOENGINE_API_H__
#include <sys/types.h>
#include "video_decoder_api.h"

#define C_VIDEO_DECODE_IDLE 			0x00000000
#define C_VIDEO_DECODE_PLAY 			0x00000001
#define C_VIDEO_DECODE_PARSER			0x00000002
#define C_VIDEO_DECODE_PAUSE			0x00000004
#define C_VIDEO_DECODE_PLAYING			0x00000010
#define C_AUDIO_DECODE_PLAYING			0x00000020
#define C_VIDEO_DECODE_PARSER_NTH		0x00000040
#define C_VIDEO_DECODE_ERR				0x80000000


int ExtGpVideoEngineOpen(int disp, int width, int height, void (*callback_fun)(unsigned int id, UINT32 data), gp_disp_buffer_func get_disp_buf, gp_disp_buffer_func send_disp_buf );
UINT32 ExtGpVideoEngineSetUrl(char *pUrl);
UINT32 ExtGpVideoEnginePlay(UINT32 Stamp);
void ExtGpVideoEngineStop();
void ExtGpVideoEngineExit();
void ExtGpVideoEnginePause();
void ExtGpVideoEngineResume();
SINT32 ExtGpVideoEngineGetInfo(gp_size_t *size);
unsigned long ExtGpVideoEngineSeek(unsigned long nGotoTime);
SINT32 ExtGpVideoEngineGetStatus();
SINT32 ExtGpVideoEngineGetPlayingStatus();
UINT32 ExtGpVideoEngineSetSpeed(float spd);
int ExtGpVideoEngineReversePlay(int revers);
unsigned long ExtGpVideoEngineGetTotalTime();
SINT32 ExtGpVideoEngineGetCurTime();
void ExtGpVideoEngineSetVolume(int nGotoVol);
SINT32 ExtGpVideoEngineGetVolume();
int ExtGpVideoEngineGetThumbnail(gp_bitmap_t *bitmap);
unsigned long ExtGpVideoEngineGetTotalSample();
int ExtGpVideoEngineQtffGetThumbnail(char *filename, gp_bitmap_t *bitmap);
int ExtGpVideoEngineGetFrameRate(void);

#endif
