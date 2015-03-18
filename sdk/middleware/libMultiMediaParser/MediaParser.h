#ifndef __MEDIA_PARSER_H__
#define __MEDIA_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include "stdio.h"
	#ifndef FILEHANDLE
		#define FILEHANDLE (FILE *)
	#endif
#else //__CC_ARM
	#ifndef FILEHANDLE
		#define FILEHANDLE int
	#endif
#endif

#ifndef __int64
	#define __int64		long long
#endif

#define MEDIAPARSER_FILE_READ_ERR		0x80000001
#define MEDIAPARSER_FORMAT_ERR			0x80000002
#define MEDIAPARSER_KERNEL_ERR			0x80000003
#define MEDIAPARSER_MEM_ERR				0x80000004
#define MEDIAPARSER_VID_BUF_SIZE_ERR	0x80000005
#define MEDIAPARSER_EXDATA_OVERFLOW		0x80000006
#define MEDIAPARSER_PARAM_ERR			0x80000007
#define MEDIAPARSER_FILE_SEEK_ERR		0x80000008
#define MEDIAPARSER_NO_STREAM_EXIST		0x80000009
#define MEDIAPARSER_NO_INDEX			0x8000000A

#define MEDIAPARSER_OS_ERR				0x8000000B


// Sample information
typedef struct
{
	long Offset;
	long Size;
	long Count;
	long dTick;
	__int64 Tick64;
	long IsKey;
} MEDIA_PARSER_SAMPLE_INFO;

#define WAVE_FORMAT_PCM				(0x0001)
#define WAVE_FORMAT_MULAW			(0x0007)
#define WAVE_FORMAT_ALAW			(0x0006) 
#define WAVE_FORMAT_ADPCM			(0x0002) 
#define WAVE_FORMAT_DVI_ADPCM		(0x0011) 
#define WAVE_FORMAT_MPEGLAYER3		(0x0055) 
#define WAVE_FORMAT_RAW_AAC1		(0x00FF)
#define WAVE_FORMAT_MPEG_ADTS_AAC	(0x1600)

typedef struct
{
	int NumberOfEntries;
	int (*Init)(void *WorkMem, char *ExData, int ExDataLen, FILEHANDLE fid, long FileSize);
	int (*GetWorkMemSize)(void);
	void *(*GetAudHandle)(void *WorkMem);
	void *(*GetVidHandle)(void *WorkMem);
	
	// Track level function
	long (*GetTotalSamples)(const void *hTrk);
	long (*GetTickRate)(const void *hTrk);
	long (*GetTotalSec)(const void *hTrk);		// return second (Q8)
	const char *(*GetExData)(const void *hTrk, long *Len);
	long (*Tick2Sample)(void *hTrk, __int64 Tick64);	// time to sample
														// To compatible with AVI format, Tick must be 64-bit
	long (*GetFormatFourCC)(const void *hTrk);
	int (*GetFrameRate)(const void *hTrk);

	// Track control
	int (*GetSampleInfo)(void *hTrk, long tarSample, MEDIA_PARSER_SAMPLE_INFO *Info);
	int (*GetKeyFrame)(void *hTrk, long tarSample);
	
	// other
	char *(*GetExtHeaderData)(void *WorkMem, long *Len);
} MEDIA_PARSER_FCN_TAB;



#ifdef __cplusplus
}
#endif

#endif // __MEDIA_PARSER_H__
