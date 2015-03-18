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
 *                                                                        *
 **************************************************************************/
/**
 * @file ipcam_api.h
 * @brief ipcamera audio api header file
 * @author 
 */

#ifndef __AUDIO_STREAM_API_H_
#define __AUDIO_STREAM_API_H_

#include "gp_avcodec.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define IPCAM_AUDIO_FMT_G726 		CODEC_ID_ADPCM_G726
#define IPCAM_AUDIO_FMT_AAC			CODEC_ID_AAC
#define IPCAM_AUDIO_FMT_G711_ALAW	CODEC_ID_PCM_ALAW
#define IPCAM_AUDIO_FMT_G711_MULAW	CODEC_ID_PCM_MULAW
#define IPCAM_AUDIO_FMT_PCM			CODEC_ID_PCM_S16LE
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct gpIPC_ADev_ATTR_S {
    UINT32 status;          /*audio device status, 0:Disble, 1:Enable*/
    UINT32 samplerate;      /*sample rate*/
    UINT32 bitrate;         /*audio frame bitrate*/
    UINT32 bitwidth;        /*bit width*/
    UINT32 soundmode;       /*sound mode*/
    UINT32 format;          /*encode format*/
    UINT32 volume;          	/*audio record volume*/
    UINT32 frameNum;        /*frame number in buffer not used*/		  
    UINT32 aec;             /*audio aec status, 0:Disable, 1:Enable*/
    UINT32 anr;             /*audio Anr status, 0:Disable, 1:Enable*/
}IPC_ADev_Attr_s;

typedef struct gpIPC_AUDIO_FRAME_S {
    UINT32 bitwidth;        /*audio frame bitwidth*/
    UINT32 bitrate;         /*audio frame bitrate*/
    UINT32 soundmode;       /*audio frame mono or stereo mode*/
    UINT32 *pAddr;          /*audio frame address*/
    UINT32 size;             /*audio frame size*/
    UINT64 duration;       /*audio frame duration*/
    UINT32 seq;             /*audio frame seq*/
    void* priv;
}IPC_Audio_Frame_s;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
 
/****************************************************************
 * description: 打开音频设备函数。该函数需要与gp_IPC_ADev_Close配合使用。
 * 打开设备后，处于disable状态，可以调用其他函数进行设置。通过调用
 * gp_IPC_ADev_Enable函数使得设备进入enable状态。打开成功返回设备的
 * handle。
 * param: dev_attr [in]: IPC_ADev_Attr_s *, 需要设置的参数。
 * return: NULL:打开失败，其他值: 设备的handle。
 * **************************************************************/
void *gp_IPC_ADev_Open(IPC_ADev_Attr_s *dev_attr);

/*****************************************************************
 * description: 关闭音频设备函数。
 * param: handle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Close(void *handle);

/****************************************************************
 * description: 获得设备设置参数。
 * param: handle [in]: 设备handle.
 * param: dev_attr [out]: IPC_ADev_Attr_s *, 保存获得的参数。
 * return: NULL:打开失败，其他值: 设备的handle。
 * **************************************************************/
SINT32 gp_IPC_ADev_Get_Attr(void *handle, IPC_ADev_Attr_s *dev_attr);

/*****************************************************************
 * description: enable 音频设备函数。open设备后需要调用该函数，设备才
 * 可以使用。
 * param: handle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Enable(void *handle);

/*****************************************************************
 * description: disable 音频设备函数。
 * param: handle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Disable(void *handle);

/****************************************************************
 * description:  设置音量
 * param: handle [in]: 设备handle.
 * param: volume [in]: 音量大小：0~100。
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Set_Volume(void *handle, SINT32 volume);

/****************************************************************
 * description: 获取音量大小 
 * param: handle [in]: 设备handle.
 * param: volume [out]: 音量大小：0~100。
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Get_Volume(void *handle, SINT32 *volume);

/****************************************************************
 * description: enable 回音消除函数。
 * param: handle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Enable_AEC(void *handle);

/****************************************************************
 * description: disable 回音消除函数。
 * param: handle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Disable_AEC(void *handle);

/****************************************************************
 * description: enable 消除噪音函数。
 * param: handle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Enable_Anr(void *handle);

/****************************************************************
 * description: disable 消除噪音函数。
 * param: handle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Disable_Anr(void *handle);

/****************************************************************
 * description: 获取audio编码帧的函数
 * param: handle [in]: 设备handle.
 * param: audio_frame [out]: IPC_Audio_Frame_s *, 保存获得的帧
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Get_Frame(void *handle, IPC_Audio_Frame_s *audio_frame);


/****************************************************************
 * description: 释放audio编码帧的函数。通过该帧去释放memory。
 * param: handle [in]: 设备handle.
 * param: audio_frame [in]: IPC_Audio_Frame_s *, 帧的结构
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_FrameRelease(void *handle, IPC_Audio_Frame_s *audio_frame);

/****************************************************************
 * description: Mute Audio, insert blank audio data 
 * param: handle [in]: handle
 * param: mute [in]: 0: no mute 1: mute
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_ADev_Mute(void *handle, SINT32 mute);

#endif //endif _IPCAM_AUDIO_API_H_
