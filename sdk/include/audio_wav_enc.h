#ifndef __ap_def_h__
#define __ap_def_h__

#define ADPCM_INPUT_FRAME_LEN			1024	// samples per channel
#define ADPCM_OUTPUT_FRAME_MAX_LEN		800     // bytes per channel
#define AUD_ADPCM_SAMPLE_TIMES          1

//================wave enc==================//
typedef int (FWRITE_FCN)(const void *buffer, int size, int count, void *hRes);
typedef int (FSEEK_FCN)(void *hRes, long offset, int origin);

//=======Constant Definition================
#define WAV_ENC_MEMORY_BLOCK_SIZE		 40 
#define DVRWAVE_FRAME_SIZE	         512
#define DVRWAVE_MEMORY_BLOCK_SIZE    1096
#define WAVEENC_FORMAT_IMA_ADPCM	   0

typedef struct
{
	
	short Buf[DVRWAVE_FRAME_SIZE];					              //Encode In  Buffer	
	unsigned char dvrwaveenc[WAV_ENC_MEMORY_BLOCK_SIZE];	// DVRwave memory block					
	short *CurPtr;			
	void *hRes;
	int Flag;
	int BufCnt;
	unsigned int length;
	unsigned int NumSamples;                              // add by comi 2009.08.20

//	WAV_ENC_INFO info;	
	FWRITE_FCN *fwrite;
	FSEEK_FCN  *fseek;
	
} DVRWAVE_MEMORY_BLOCK;

extern int wavenc_GetWorkMemSize(void);
extern int wavenc_GetHeaderLength(unsigned char *p_workmem);
extern int wavenc_GetFrameSamples(unsigned char *p_workmem);
extern int wavenc_init( unsigned char *p_workmem, int nChannels, int SampleRate, int FormatTag);
extern int wavenc_encframe(unsigned char *pWorkMem, short *in_buffer, unsigned char *out_buffer);
extern int wavenc_end(unsigned char *pWorkMem,unsigned char *WaveHeader);
extern const char *wavenc_GetErrString(int ErrCode);
extern const char *wavenc_GetVersion(void);

//==========================================//

#endif //__ap_def_h__
