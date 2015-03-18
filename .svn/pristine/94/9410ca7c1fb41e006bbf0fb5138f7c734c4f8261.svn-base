
/***************************************************************************
 * Name: gp_mux.h
 *
 * Purpose:
 *
 * Developer:
 *     liuliang, 2010-12-20
 *
 * Copyright (c) 2010-2011 by Sunplus mMobile Inc.
 ***************************************************************************/

#ifndef _GP_MUX_H_
#define _GP_MUX_H_

/***************************************************************************
 * Header Files
 ***************************************************************************/

#include "typedef.h"
#include "gp_avcodec.h"
#include "gp_stream.h"

#ifdef cplusplus
extern "C" {
#endif

/***************************************************************************
 * Constants
 ***************************************************************************/

enum muxParam {
    MUX_MAX_SIZE = 0,
    MUX_AUD_TYPE,   
    MUX_VID_TYPE,               
    MUX_WIDTH,                
    MUX_HEIGHT,                   
    MUX_FRMRATE,               
    MUX_DPEN,                
    MUX_AUDSR,                      
    MUX_AUD_CHANNEL,
    MUX_AUD_BIT,
    MUX_PATH_TEMP_FILE,
    MUX_CLUSTER_SIZE,
    MUX_FILE_TIME,
    MUX_PARAM_END
};

    
#define MUX_RET_OK           0
#define MUX_RET_ERROR        -1
#define MUX_MEM_FULL 1
#define MUX_FILE_SIZE_REACH 2
#define SUCCESS 0
#define FAIL -1

/* WAVE format wFormatTag IDs */
#define AUD_FORMAT_UNKNOWN             0x0000 /* Microsoft Corporation */
#define AUD_FORMAT_PCM                 0x0001 /* Microsoft Corporation */
#define AUD_FORMAT_ADPCM               0x0002 /* Microsoft Corporation */
#define AUD_FORMAT_IEEE_FLOAT          0x0003 /* Microsoft Corporation */
#define AUD_FORMAT_ALAW                0x0006 /* Microsoft Corporation */
#define AUD_FORMAT_MULAW               0x0007 /* Microsoft Corporation */
#define AUD_FORMAT_DTS_MS              0x0008 /* Microsoft Corporation */
#define AUD_FORMAT_IMA_ADPCM           0x0011 /* Intel Corporation */
#define AUD_FORMAT_GSM610              0x0031 /* Microsoft Corporation */
#define AUD_FORMAT_MSNAUDIO            0x0032 /* Microsoft Corporation */
#define AUD_FORMAT_G726                0x0045 /* ITU-T standard  */
#define AUD_FORMAT_MPEG                0x0050 /* Microsoft Corporation */
#define AUD_FORMAT_MPEGLAYER3          0x0055 /* ISO/MPEG Layer3 Format Tag */
#define AUD_FORMAT_DOLBY_AC3_SPDIF     0x0092 /* Sonic Foundry */

#define AUD_FORMAT_A52                 0x2000
#define AUD_FORMAT_DTS                 0x2001
#define AUD_FORMAT_WMA1                0x0160
#define AUD_FORMAT_WMA2                0x0161
#define AUD_FORMAT_WMA3                0x0162
#define AUD_FORMAT_DIVIO_AAC           0x4143
#define AUD_FORMAT_AMR_CBR             0x7A21
#define AUD_FORMAT_AMR_VBR             0x7A22

/* Need to check these */
#define AUD_FORMAT_DK3                 0x0061
#define AUD_FORMAT_DK4                 0x0062

#define AVI_MAX_FILE_SIZE           0x80000000-0xC00000 /*2G*/
#define MP4_MAX_FILE_SIZE           0x80000000-0xC00000 /*2G*/

#define DISK_MIN_FREE_SPACE         0xC00000   /*12M*/

/***************************************************************************
 * Macros
 ***************************************************************************/



/***************************************************************************
 * Data Types
 ***************************************************************************/
 
typedef struct gpMuxPkt_s 
{
    UINT8* data;//packet date
    UINT32 size; // packet size 
    UINT32 pts; // ms
    UINT32 keyFrame; // 0 = I, 1 = P,  2 = B for video, 1 = key frame for audio
}gpMuxPkt_t;

typedef struct gpMux_s 
{
	SINT32 (*init)();
	SINT32 (*uninit)();
    SINT32 (*open)(HANDLE out); 
    SINT32 (*close)(SINT32 hd);
    SINT32 (*pack)(SINT32 hd, gpMuxPkt_t *pkt, gpEsType_t type);
    SINT32 (*set)(SINT32 hd, UINT32 param, UINT32 value);
    SINT32 (*get)(SINT32 hd, UINT32 param, UINT32 *value);
}gpMux_t;


/***************************************************************************
 * Global Data
 ***************************************************************************/

/***************************************************************************
 * Function Declarations
 ***************************************************************************/
SINT32 gpMuxLoad(gpMux_t *mux, gpCsType_t ctype);
/***************************************************************************
 * Inline Function Definitions
 ***************************************************************************/

#ifdef cplusplus
}
#endif

#endif   /* _GP_MUX_H_  */

/***************************************************************************
 * The gp_mux.h file end
 ***************************************************************************/
