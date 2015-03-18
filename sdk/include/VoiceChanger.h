#ifndef __VOICE_CHANGER_H__
#define __VOICE_CHANGER_H__


#ifdef __cplusplus
extern "C" {
#endif

// version
const char *VoiceChanger_GetVersion(void);

// functions that used for memory allocated by VoiceChanger
void *VoiceChanger_Create(int InFrameSize, int fs, int ch, int OptFlag);
void VoiceChanger_Del(void *pWorkMem);

// functions that used for memory allocated by User
int VoiceChanger_GetMemBlockSize(int InFrameSize, int fs, int ch, int OptFlag);
void VoiceChanger_Initial(void *pWorkMem, int InFrameSize, int fs, int ch, int OptFlag);

// Initial Link and Unlink functions
void VoiceChanger_Link(
	void *pWorkMem,
	void *pFrontEndWorkMem,
	int (*GetOutput)(void *, short *, int),
	int fs,
	int ch,
	int OptionFlag);

// opt:
#define OPTFLAG_LEFT_CH_ONLY	1
// This flag only take effect if ch == 2
// If this flag is set, VoiceChanger module will convert 2-ch inuput to 1-ch(Left only) output
	

void VoiceChanger_Unlink(void *pWorkMem);

// set functions
extern void VoiceChanger_SetParam(void *pWorkMem, int speed, int pitch);
// speed : can be 0~24. Default 12, means using original speed as output.
// This setting represents the following times of original speed:
//	0.5x,	0.5x,	0.562x,	0.625x,	0.625x,	0.687x,	0.687x,	0.75x,	0.812x,	0.812x,	0.875x,	0.938x,	1.0x,
//	1.063x,	1.125x,	1.188x,	1.25x,	1.313x,	1.375x,	1.5x,	1.563x,	1.688x,	1.813x,	1.877x,	2.0x,
//
// pitch : can be 0~24. Default 12, means using original pitch as output.
// This setting represents the following times of original pitch:
//	0.5x,	0.5x,	0.562x,	0.625x,	0.625x,	0.687x,	0.687x,	0.75x,	0.812x,	0.812x,	0.875x,	0.938x,	1.0x,
//	1.063x,	1.125x,	1.188x,	1.25x,	1.313x,	1.375x,	1.5x,	1.563x,	1.688x,	1.813x,	1.877x,	2.0x,
//

// get output function
int VoiceChanger_GetOutput(void *pWorkMem, short *DstData, int Len);

// get functions
int VoiceChanger_GetSampleRate(void *pWorkMem);
int VoiceChanger_GetChannel(void *pWorkMem);


#ifdef __cplusplus
}
#endif


#endif // __VOICE_CHANGER_H__
