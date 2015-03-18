/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2014 by Generalplus Inc.                         *
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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park                       *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
#ifndef __THREAD_VIDEO_DECODE_H__
#define __THREAD_VIDEO_DECODE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include "mach/gp_scale.h"
#include "mach/gp_display.h"

#ifdef CONFIG_ARCH_GPL32900B
#include "mach/gp_scale2.h"
#include "mach/gp_tv.h"
#include "mach/gp_hdmi.h"
#endif

#include "./../../libMcpThread/mcp_thread.h"
#include "./../../libMcpThread/mcp_queue.h"
#include "./../../libMultiMediaParser/MultiMediaParser.h"
#include "video_decoder_cfg.h"

#ifdef SPECIAL_MEM_MALLOC
#include <chunkmem.h>
#include <mach/gp_chunkmem.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
// file type
#define FILE_TYPE_AVI		1
#define FILE_TYPE_MOV		2

// codec type
#define C_MPEG4_CODEC			0x01
#define C_H263_CODEC			0x02 
#define C_SORENSON_H263_CODEC	0x04

// video_format
#define C_XVID_FORMAT		0x44495658
#define C_DIVX_FORMAT		0x58564944
#define C_M4S2_FORMAT		0x3253344D
#define C_H263_FORMAT		0x33363248
#define C_H264_FORMAT		0x34363248
#define C_SOREN_H263_FORMAT	0x00000000
#define C_MJPG_FORMAT		0x47504A4D

// mbox ack
#define C_ACK_SUCCESS		0x00000001
#define C_ACK_FAIL			0x80000000

// flag ack
#define C_FLAG_ACK_FAIL		(1 << 30)
#define C_FLAG_ACK_OK		(1 << 31)
#define C_FLAG_ACK_ALL		(C_FLAG_ACK_FAIL | C_FLAG_ACK_OK)

// return status
#define STATUS_OK			0
#define STATUS_FAIL			(-1)

#ifndef TRUE
	#define	TRUE			1
#endif

#ifndef FALSE
	#define	FALSE			0
#endif

// video_decode_status
#define C_VIDEO_DECODE_IDLE     	0x00000000
#define C_VIDEO_DECODE_PLAY			0x00000001		  	
#define C_VIDEO_DECODE_PARSER		0x00000002
#define C_VIDEO_DECODE_PAUSE		0x00000004
#define C_VIDEO_DECODE_PLAYING		0x00000010
#define C_AUDIO_DECODE_PLAYING		0x00000020
#define C_VIDEO_DECODE_PARSER_NTH	0x00000040
#define C_VIDEO_DECODE_ERR      	0x80000000
#define C_VIDEO_DECODE_ALL			C_VIDEO_DECODE_PLAY|C_VIDEO_DECODE_PARSER|C_VIDEO_DECODE_PAUSE|C_VIDEO_DECODE_PLAYING|C_AUDIO_DECODE_PLAYING|C_VIDEO_DECODE_ERR

#define WAVE_FORMAT_PCM				(0x0001)
#define WAVE_FORMAT_MULAW			(0x0007)
#define WAVE_FORMAT_ALAW			(0x0006) 
#define WAVE_FORMAT_ADPCM			(0x0002) 
#define WAVE_FORMAT_DVI_ADPCM		(0x0011) 
#define WAVE_FORMAT_MPEGLAYER3		(0x0055) 
#define WAVE_FORMAT_RAW_AAC1		(0x00FF)
#define WAVE_FORMAT_MPEG_ADTS_AAC	(0x1600)


// display device
#define C_DISP0_TFT					0
#define C_DISP1_TFT					1
#define C_DISP2_TV					2
#define C_DISP2_TV_QVGA				3
#define	C_DISP0_TV 					4
#define C_DISP0_TV_SCALE			5
#define C_DISP0_HDMI				6
#define C_DISP_BUFFER				7

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if defined(SPECIAL_MEM_MALLOC)
	#define osMalloc(x)	gpChunkMemAlloc(x) 
	#define osFree(x)	gpChunkMemFree((void *)(x)) 
#else
	#define osMalloc(x)	malloc(x) 
	#define osFree(x)	free((void *)(x)) 
#endif

#if 1
	#define MSG 	printf
#else
	#define MSG(...)
#endif
#if 0
	#define DBG 	printf
#else
	#define DBG(...)
#endif	
#define ERROR 		printf

// for measure use.
#ifdef CONFIG_ARCH_GPL326XXB
#define R_IOA_I_DATA        	    (*((volatile UINT32 *) 0xC0000000))
#define R_IOA_O_DATA        	    (*((volatile UINT32 *) 0xC0000004))
#define R_IOA_DIR	                (*((volatile UINT32 *) 0xC0000008))
#define R_IOA_ATT	                (*((volatile UINT32 *) 0xC000000C))
#define R_IOA_DRV                   (*((volatile UINT32 *) 0xC0000010))

#define R_SYSTEM_CLK_EN0  	      	(*((volatile UINT32 *) 0xD0000010))
#define R_SYSTEM_CLK_EN1         	(*((volatile UINT32 *) 0xD0000014))
#endif

#ifdef CONFIG_ARCH_GPL32900B
typedef void (*mcpCallMsg_t)(UINT32 msg, UINT32 data);
typedef int (*gp_disp_buffer_func)(int arg);
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/ 
typedef struct thread_s
{
	UINT32		 running;
	mcp_thread_t thread;
	mcp_flag_t	 flag;
	mcp_flag_t	 ackflag;
} thread_t;	

typedef struct VidDec_s
{
	thread_t	state;
	thread_t	display;
	thread_t	display_smooth;
	thread_t	audio;
	thread_t	video;
	thread_t	sync;
	
	// buffer control
	mcp_mutex_t TimeMutx;
	mcp_mbox_t	VidDecEmptyBuf;	
	mcp_mbox_t	VidDecReadyBuf;
	mcp_mbox_t	DispEmptyBuf;
	mcp_mbox_t	DispReadyBuf;
	
	SINT32		FileHandle;
	SINT32		FileType;
	UINT32		FileSize;
	UINT32		Status;
	
	// parser
	void		*MediaHandle;
	const GP_WAVEFORMATEX		*pWaveInfo;
	long		WaveInfoLen;
	const GP_BITMAPINFOHEADER	*pBitmapInfo;
	long		BitmapInfoLen;
	
	// video & audio sync 
	SINT64		TickTime;
	SINT64		TickTime2;
	SINT64		TimeRange;
	SINT32		n;
	SINT64		ta;			//audio play time
	SINT64		Ta;			//audio decode time
	SINT64		tv;			//video sync time
	SINT64		Tv;			//video decode time
	UINT32		FailCnt;
	
	// video speed control	
	UINT32		PlaySpeed;
	UINT8		ReversePlaySet;
	UINT8		ReversePlayFlag;
	UINT8		ffAudFlag;
	UINT8		RSD0;
	
	// video decoder
	UINT32		VidCodec;
	UINT32		CevaCodec;
	UINT16		DecWidth;
	UINT16		DecHeight;
	UINT32		DecFmt;
	UINT32		VidDecOutAddr[VIDEO_FRAME_NO];
	UINT32		VidTickRate;
	UINT32		VidTotalSample;
	UINT32		VidFrameRate;
	UINT32		VidTotalTime;
	UINT32		VidCurTime;
	UINT32		VidSeekTime;
	float		VidRealFR;
#if CONFIG_ARCH_GPL32900B
	UINT32		stride;
	UINT32		strideUV;
	mcpCallMsg_t sendMsg;
#endif
	
	// audio decoder
	UINT32		AudCodec;
	UINT16		nChannels;
	UINT16		SampleRate;
	UINT32		AudFrameSize;
	UINT16*		AudDecOutAddr;
	UINT32		AudWorkMemSize;
	UINT32		AudRingBufSize;
	UINT8		*AudWorkMemAddr;
	UINT8		*AudRingBufAddr;
	UINT8		UpEn;	//upsample 
	UINT8		VCEn;	//voice changer
	UINT8		VCSpeed;//voice changer speed
	UINT8		VCPitch;//voice changer pitch
	
	// display
	UINT32		DispDev;
	UINT8		aspect;	//0: disable scale mode, and using display full screen.1: enable scale mode.
	UINT8		RSD1;
	UINT8		RSD2;
	UINT8		RSD3;
	UINT8		BufferIdx; // for decode to buffer
	UINT32		BufferWidth;
	UINT32		BufferHeight;
	gp_disp_buffer_func get_disp_buf;
	gp_disp_buffer_func send_disp_buf;
	
	gp_disp_res_t PanelRes;
	gp_bitmap_t DispBitmap;
	UINT16		DispX;
	UINT16		DispY;
	UINT16		DispWidth;
	UINT16		DispHeight;
	UINT16		par_w;
	UINT16		par_h;
	UINT32		DispFmt;
	UINT32		DisplayAddr[DISP_FRAME_NO];
	UINT32		PostCnt;
	UINT32		PendCnt; 

	// decode one frame
	UINT32		OneFrameFmt;
	UINT16		OneFrameWidth;
	UINT16		OneFrameHeight;
	UINT32		OneFrameAddr;
} VidDec_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
//state
SINT32 video_decode_entrance(UINT8 DispDevNo, UINT8 DispFmt);
void   video_decode_exit(void);

SINT32 video_decode_state_thread_create(VidDec_t *pVidDec, UINT32 priority);
void   video_decode_state_thread_delete(void);
SINT32 video_decode_parser_start(char *filename);
SINT32 video_decode_parser_stop(void);
SINT32 video_decode_start(void);
SINT32 video_decode_stop(void);
SINT32 video_decode_pause(void);
SINT32 video_decode_resume(void);
SINT32 video_decode_status(void);
SINT32 video_decode_get_video_info(UINT16 *width, UINT16 *height);
SINT32 video_decode_get_audio_info(UINT16 *channel, UINT16 *sample_rate);
SINT32 video_decode_get_total_samples(void);
SINT32 video_decode_get_total_time(void);
SINT32 video_decode_get_current_time(void);
SINT32 video_decode_get_frame_rate(void);
SINT32 video_decode_set_play_time(UINT32 second);
SINT32 video_decode_set_play_speed(float speed);
SINT32 video_decode_set_reverse_play(UINT8 flag);
SINT32 video_decode_1st_frame(UINT32 Fmt, UINT16 Width, UINT16 Height, UINT32 OutputBuf);
SINT32 viddec_end_callback(UINT32 flag);
SINT32 video_decode_set_play_callback(mcpCallMsg_t sendMsg);
SINT32 video_decode_get_thumbnail(gp_bitmap_t *bitmap);
SINT32 video_decode_playing_status(void);
SINT32 qtff_get_thumbnail(char *filename, gp_bitmap_t *bp);
SINT32 video_decode_set_display_aspect(int enable);
SINT32 video_decode_set_panel_ratio(int is4x3);

//audio
SINT32 audio_decode_thread_create(VidDec_t *pVidDec, UINT32 priority);
SINT32 audio_decode_thread_start(void);
SINT32 audio_decode_thread_stop(void); 
SINT32 audio_decode_thread_pause(void);
SINT32 audio_decode_thread_resume(void);

//video 
SINT32 video_decode_thread_create(VidDec_t *pVidDec, UINT32 priority);
SINT32 video_decode_thread_start(void);
SINT32 video_decode_thread_stop(void);
SINT32 video_decode_thread_pause(void);
SINT32 video_decode_thread_resume(void);
SINT32 video_decode_first_rawdata(VidDec_t *pVidDec, UINT32 outbuffer);

//display
SINT32 display_thread_create(VidDec_t *pVidDec, UINT32 priority);
void   display_thread_delete(void);
SINT32 display_set_start(void);
SINT32 display_set_stop(void);
SINT32 display_set_disp_once(void);

//sync
SINT32 viddec_timer_start(void);
void   viddec_timer_stop(void);

//API
void video_decode_audio_disable(void);
void video_decode_video_disable(void);
void viddec_set_status(UINT32 flag);
void viddec_clear_status(UINT32 flag);
UINT32 viddec_test_status(UINT32 flag);
SINT32 viddec_get_video_format(UINT32 biCompression);
SINT32 viddec_get_file_format(SINT8 *pdata);
SINT32 viddec_video_malloc(UINT32 size);
void   viddec_video_mfree(void);
SINT32 viddec_display_malloc(UINT32 size);
void   viddec_display_mfree(void);
SINT32 viddec_set_auddec_malloc(UINT32 work_mem_size);
SINT32 viddec_set_auddec_ring_buffer(void);
void   viddec_set_auddec_mfree(void);

// flag API
SINT32 FlagSendMsg(mcp_flag_t *flag, mcp_flag_t *ackflag, UINT32 bits);
void   FlagSendAckOK(mcp_flag_t *ackflag);
void   FlagSendAckFail(mcp_flag_t *ackflag);

SINT32 video_decode_set_display_buffer_param(int w, int h, gp_disp_buffer_func get_disp_buf, gp_disp_buffer_func send_disp_buf);
#endif

