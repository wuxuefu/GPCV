#ifndef __MMP_H__
#define __MMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
	#include "stdio.h"
	#ifndef FILEHANDLE
		#define FILEHANDLE (FILE *)
	#endif
#else
	//__CC_ARM
	#ifndef FILEHANDLE
		#define FILEHANDLE int
	#endif
#endif

#ifndef __int64
	#define __int64		long long
#endif

//========================================================
//		 Error ID
//========================================================
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

//========================================================
//		 Audio Format
//========================================================
#define WAVE_FORMAT_PCM				(0x0001)
#define WAVE_FORMAT_MULAW			(0x0007)
#define WAVE_FORMAT_ALAW			(0x0006) 
#define WAVE_FORMAT_ADPCM			(0x0002) 
#define WAVE_FORMAT_DVI_ADPCM		(0x0011) 
#define WAVE_FORMAT_MPEGLAYER3		(0x0055) 
// #define WAVE_FORMAT_AAC			(0xA106) 
#define WAVE_FORMAT_RAW_AAC1		(0x00FF)
#define WAVE_FORMAT_MPEG_ADTS_AAC	(0x1600)

//========================================================
//		 Data Type
//========================================================
typedef struct 
{
	unsigned short	wFormatTag;
	unsigned short	nChannels;
	unsigned long	nSamplesPerSec;
	unsigned long	nAvgBytesPerSec;
	unsigned short	nBlockAlign;
	unsigned short	wBitsPerSample;
	unsigned short	cbSize;
} GP_WAVEFORMATEX;	// strf

typedef struct
{ 
	unsigned long biSize; 
	long biWidth; 
	long biHeight; 
	unsigned short biPlanes; 
	unsigned short  biBitCount;
	unsigned long biCompression; 
	unsigned long biSizeImage; 
	long biXPelsPerMeter; 
	long biYPelsPerMeter; 
	unsigned long biClrUsed; 
	unsigned long biClrImportant; 
} GP_BITMAPINFOHEADER; 

//========================================================
//		Parser function
//========================================================
//========================================================
// Function Name : MultiMediaParser_GetVersion
// Syntax        : const char *MultiMediaParser_GetVersion(void);
// Purpose       : get multi-media parser version number 
// Parameters    : none
// Return        : version string pointer.
//========================================================
const char *MultiMediaParser_GetVersion(void);

//========================================================
// Function Name : MultiMediaParser_Create
// Syntax        : void *MultiMediaParser_Create(int fid,
//										long FileSize,
//										int AudRingLen,
//										int VidBufSize,
//										int prio,
//										const void *FcnTab,
//										int (*ErrHandler)(int ErrID),
//										int *ErrID);
// Purpose      : create multi-media parser task 
// Parameters   : int fid: video file file handle.
//                long FileSize: video file size.
//                int AudRingLen: audio ring buffer size.
//                int VidBufSize: video bitstream buffer size.
//                int prio: multi-media task priority.
//                const void *FcnTab: multi-media parser function table.
//                int (*ErrHandler)(int ErrID): error handle api, register by user
//                int *ErrID: error ID number.
// Return       : work memory address
//========================================================
void *MultiMediaParser_Create(
	int fid,
	long FileSize,
	int AudRingLen,
	int VidBufSize,
	int prio,
	const void *FcnTab,
	int (*ErrHandler)(int ErrID),
	int *ErrID);

//========================================================
// Function Name : MultiMediaParser_Delete
// Syntax        : void MultiMediaParser_Delete(void *hWorkMem);
// Purpose       : delete multi-media parser task 
// Parameters    : void *hWorkMem: work memory address
// Return        : none
//========================================================	
void MultiMediaParser_Delete(void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_SeekTo
// Syntax        : int MultiMediaParser_SeekTo(void *hWorkMem, long Sec);
// Purpose       : seek play time
// Parameters    : void *hWorkMem: work memory address
//                 long Sec: seek time in second
// Return        : seek time
//========================================================	
int MultiMediaParser_SeekTo(void *hWorkMem, long Sec);

//========================================================
// Function Name : MultiMediaParser_SetFrameDropLevel
// Syntax        : void MultiMediaParser_SetFrameDropLevel(void *hWorkMem, int n);
// Purpose       : set drop farme number
// Parameters    : void *hWorkMem: work memory address
//                 int n: drop frame number
// Return        : none
//========================================================	
void MultiMediaParser_SetFrameDropLevel(void *hWorkMem, int n);

//========================================================
// Function Name : MultiMediaParser_EnableAudStreaming
// Syntax        : int MultiMediaParser_EnableAudStreaming(void *hWorkMem, int flag);
// Purpose       : enable seek audio stream when only play video
// Parameters    : void *hWorkMem: work memory address
//                 int flag: 0 is diasble, 1 is enable
// Return        : >=0 is success, <0 is fail
//========================================================	
int MultiMediaParser_EnableAudStreaming(void *hWorkMem, int flag);

//========================================================
// Function Name : MultiMediaParser_SetReversePlay
// Syntax        : int MultiMediaParser_SetReversePlay(void *hWorkMem, int flag);
// Purpose       : Set reverse play 
// Parameters    : void *hWorkMem: work memory address
//                 int flag: 0 is diasble, 1 is enable
// Return        : >=0 is success, <0 is fail
//========================================================	
int MultiMediaParser_SetReversePlay(void *hWorkMem, int flag);

//========================================================
//		Audio Functions
//========================================================
//========================================================
// Function Name : MultiMediaParser_GetAudInfo
// Syntax        : const GP_WAVEFORMATEX *MultiMediaParser_GetAudInfo(const void *hWorkMem, long *Len);
// Purpose       : Get audio information
// Parameters    : void *hWorkMem: work memory address
//                 long *Len: audio information length 
// Return        : audio information table point
//========================================================
const GP_WAVEFORMATEX *MultiMediaParser_GetAudInfo(const void *hWorkMem, long *Len);

//========================================================
// Function Name : MultiMediaParser_IsEOA
// Syntax        : int MultiMediaParser_IsEOA(const void *hWorkMem);
// Purpose       : check audio play end
// Parameters    : void *hWorkMem: work memory address
// Return        : 0 is not end, 1 is decode end, 2 is data read end
//========================================================
int MultiMediaParser_IsEOA(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetAudTrackDur
// Syntax        : long MultiMediaParser_GetAudTrackDur(const void *hWorkMem);
// Purpose       : Get audio track duration
// Parameters    : void *hWorkMem: work memory address
// Return        : audio total time
//========================================================
long MultiMediaParser_GetAudTrackDur(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetAudRing
// Syntax        : const char *MultiMediaParser_GetAudRing(void *hWorkMem);
// Purpose       : Get audio ring buffer address
// Parameters    : void *hWorkMem: work memory address
// Return        : ring buffer address
//========================================================
const char *MultiMediaParser_GetAudRing(void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetAudRingLen
// Syntax        : int MultiMediaParser_GetAudRingLen(void *hWorkMem);
// Purpose       : Get audio ring buffer length
// Parameters    : void *hWorkMem: work memory address
// Return        : ring buffer length
//========================================================
int MultiMediaParser_GetAudRingLen(void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetAudRingWI
// Syntax        : int MultiMediaParser_GetAudRingWI(void *hWorkMem);
// Purpose       : Get audio ring buffer write postion
// Parameters    : void *hWorkMem: work memory address
// Return        : ring buffer write postion
//========================================================
int MultiMediaParser_GetAudRingWI(void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_RefillAudBuf
// Syntax        : int MultiMediaParser_RefillAudBuf(void *hWorkMem, int RI);
// Purpose       : Refill audio data to ring buffer.
// Parameters    : void *hWorkMem: work memory address
//                 int RI: ring buffer read postion
// Return        : >=0 is success, <0 is fail
//========================================================
int MultiMediaParser_RefillAudBuf(void *hWorkMem, int RI);

//========================================================
//		Video Function
//========================================================
//========================================================
// Function Name : MultiMediaParser_GetVidInfo
// Syntax        : const GP_BITMAPINFOHEADER *MultiMediaParser_GetVidInfo(const void *hWorkMem, long *Len);
// Purpose       : Get video information
// Parameters    : void *hWorkMem: work memory address
//                 long *Len: video information length
// Return        : video information table point
//========================================================
const GP_BITMAPINFOHEADER *MultiMediaParser_GetVidInfo(const void *hWorkMem, long *Len);

//========================================================
// Function Name : MultiMediaParser_GetVidTickRate
// Syntax        : long MultiMediaParser_GetVidTickRate(const void *hWorkMem);
// Purpose       : Get video tick rate for play speed use
// Parameters    : void *hWorkMem: work memory address
// Return        : video tick rate
//========================================================
long MultiMediaParser_GetVidTickRate(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetVidTotalSamples
// Syntax        : long MultiMediaParser_GetVidTotalSamples(const void *hWorkMem);
// Purpose       : Get video total frame number
// Parameters    : void *hWorkMem: work memory address
// Return        : video total frame number
//========================================================
long MultiMediaParser_GetVidTotalSamples(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetVidTotalSamples
// Syntax        : long MultiMediaParser_GetVidTotalSamples(const void *hWorkMem);
// Purpose       : Get video total frame number
// Parameters    : void *hWorkMem: work memory address
// Return        : video total frame number
//========================================================
int MultiMediaParser_GetVidFrameRate(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_IsEOV
// Syntax        : int MultiMediaParser_IsEOV(const void *hWorkMem);
// Purpose       : check video play end
// Parameters    : void *hWorkMem: work memory address
// Return        : 0 is not end, 1 is end
//========================================================
int MultiMediaParser_IsEOV(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetNumberOfFrameInVidBuf
// Syntax        : int MultiMediaParser_GetNumberOfFrameInVidBuf(const void *hWorkMem);
// Purpose       : Get the ready video frame number
// Parameters    : void *hWorkMem: work memory address
// Return        : ready video frame number
//========================================================
int MultiMediaParser_GetNumberOfFrameInVidBuf(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetVidCurTime
// Syntax        : long MultiMediaParser_GetVidCurTime(const void *hWorkMem);
// Purpose       : Get the current video frame time
// Parameters    : void *hWorkMem: work memory address
// Return        : ready video number
//========================================================
long MultiMediaParser_GetVidCurTime(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_GetVidTrackDur
// Syntax        : long MultiMediaParser_GetVidTrackDur(const void *hWorkMem);
// Purpose       : Get the video track duration
// Parameters    : void *hWorkMem: work memory address
// Return        : video total time
//========================================================
long MultiMediaParser_GetVidTrackDur(const void *hWorkMem);

//========================================================
// Function Name : MultiMediaParser_RegisterReadFunc
// Syntax        : int MultiMediaParser_RegisterReadFunc(int ReadFunc(short, unsigned int, unsigned int));
// Purpose       : Register read function. default use read().
// Parameters    : int ReadFunc(short, unsigned int, unsigned int): Read function
// Return        : >=0 is success, <0 is fail
//========================================================
int MultiMediaParser_RegisterReadFunc(int ReadFunc(short, unsigned int, unsigned int));

//========================================================
// Function Name : MultiMediaParser_RegisterSeekFunc
// Syntax        : int MultiMediaParser_RegisterSeekFunc(int SeekFunc(short, int, short));
// Purpose       : Register seek function. default use lseek().
// Parameters    : int SeekFunc(short, int, short): seek function
// Return        : >=0 is success, <0 is fail
//========================================================
#ifndef SEEK64_EN
int MultiMediaParser_RegisterSeekFunc(int SeekFunc(short, int, short));
#else
int MultiMediaParser_RegisterSeekFunc(long long SeekFunc(short, long long, short));
#endif

//========================================================
// Function Name : MultiMediaParser_GetVidBuf
// Syntax        : const char *MultiMediaParser_GetVidBuf(void *hWorkMem, 
//                                                        long *Len, 
//                                                        long *Duration, 
//                                                        int *ErrID);
// Purpose       : Get ready video buffer
// Parameters    : void *hWorkMem: work memory address
//                 long *Len: video data length
//                 long *Duration: video duration
//                 int *ErrID: error ID
// Return        : video buffer address
//========================================================
const char *MultiMediaParser_GetVidBuf(void *hWorkMem, 
									long *Len, 
									long *Duration, 
									int *ErrID);

//========================================================
//		resume play function
//========================================================
//========================================================
// Function Name : MultiMediaParser_Create
// Syntax        : void *MultiMediaParser_ReCreate(int fid,
//                                                 long FileSize,
//                                                 int AudRingLen,
//                                                 int VidBufSize,
//                                                 int prio,
//                                                 const void *FcnTab,
//                                                 int (*ErrHandler)(int ErrID),
//                                                 int *ErrID);
// Purpose       : create multi-media parser task for resume play 
// Parameters    : int fid: video file file handle.
//                 long FileSize: video file size.
//                 int AudRingLen: audio ring buffer size.
//                 int VidBufSize: video bitstream buffer size.
//                 int prio: multi-media task priority.
//                 const void *FcnTab: multi-media parser function table.
//                 int (*ErrHandler)(int ErrID): error handle api, register by user
//                 int *ErrID: error ID number.
// Return        : work memory address
//========================================================
void *MultiMediaParser_ReCreate(
	int fid,
	long FileSize,
	int AudRingLen,
	int VidBufSize,
	int prio,
	const void *FcnTab,
	int (*ErrHandler)(int ErrID),
	int *ErrID);

//========================================================
// Function Name : MultiMediaParser_StoreState
// Syntax        : int MultiMediaParser_StoreState(void *hWorkMem);
// Purpose       : Store the play status for resume play use.
// Parameters    : void *hWorkMem: work memory address
// Return        : >=0 is success, <0 is fail 
//========================================================
int MultiMediaParser_StoreState(void *hWorkMem); 

//========================================================
// Function Name : MultiMediaParser_RegisterStoreFunc
// Syntax        : int MultiMediaParser_RegisterStoreFunc(int StoreFunc(unsigned int, unsigned int));
// Purpose       : Register store function for resume play use.
// Parameters    : int StoreFunc(unsigned int, unsigned int): store function 
// Return        : >=0 is success, <0 is fail 
//========================================================
int MultiMediaParser_RegisterStoreFunc(int StoreFunc(unsigned int, unsigned int));

//========================================================
// Function Name : MultiMediaParser_RegisterLoadFunc
// Syntax        : int MultiMediaParser_RegisterLoadFunc(int LoadFunc(unsigned int, unsigned int));
// Purpose       : Register Load function for resume play use.
// Parameters    : int LoadFunc(unsigned int, unsigned int): load function 
// Return        : >=0 is success, <0 is fail 
//========================================================
int MultiMediaParser_RegisterLoadFunc(int LoadFunc(unsigned int, unsigned int));
#ifdef __cplusplus
}
#endif

#endif // __MMP_H__
