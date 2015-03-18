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
#ifndef __THREAD_VIDEO_DECODE_CFG_H__
#define __THREAD_VIDEO_DECODE_CFG_H__

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
// configure
#define PARSER_DROP					0x00
#define DISP_DROP					0x01

#ifdef CONFIG_ARCH_GPL326XXB
#define VIDEO_FRAME_NO				3			//video frame number
#define DISP_FRAME_NO				3			//display buffer
#define WAV_DECODE_ENABLE			1			//0: disable, 1:enable
#define MP3_DECODE_ENABLE			1			//0: disable, 1:enable
#define AAC_DECODE_ENABLE			0			//0: disable, 1:enable	
#define	GPLIB_MP3_HW_EN				1			//0: disable, 1:enable
#define MPEG4_DECODE_ENABLE			1			//0: disable, 1:enable
#define PARSER_AUD_BITSTREAM_SIZE	(16*1024)	//unit: byte
#define PARSER_VID_BITSTREAM_SIZE	(32*1024)	//unit: byte
#define TIME_BASE_TICK_RATE			100			//video timer frequency
#define DROP_MENTHOD				PARSER_DROP
#endif

#ifdef CONFIG_ARCH_GPL32900B
#define VIDEO_FRAME_NO				6			//video frame number
#define DISP_FRAME_NO				6			//display buffer
#define WAV_DECODE_ENABLE			1			//0: disable, 1:enable
#define MP3_DECODE_ENABLE			1			//0: disable, 1:enable
#define AAC_DECODE_ENABLE			1			//0: disable, 1:enable	
#define	GPLIB_MP3_HW_EN				0			//fixed 0
#define MPEG4_DECODE_ENABLE			0			//fixed o
#define TIME_BASE_TICK_RATE			1000		//video timer frequency
#define PARSER_AUD_BITSTREAM_SIZE	(32*1024)	//unit: byte
#define PARSER_VID_BITSTREAM_SIZE	(128*1024)	//unit: byte
#define DROP_MENTHOD				DISP_DROP
#endif

#define FPS_CAL_EN					1			//caculate fps, 0: disable, 1:enable
#define FPS_CAL_SEC					5			//caculate second
#define AUDIO_PCM_BUFFER_SIZE		(4096<<1) 	//unit: byte
 
// thread priority
#define SYNC_PRIORITY				10
#define PARSER_PRIORITY				10
#define DISPLAY_PRIORITY			12
#define AUDIO_DECODE_PRIORITY		14
#define VIDEO_DECODE_PRIORITY		14
#define VIDEO_STATE_PRIORITY		16
#endif

