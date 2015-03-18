/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2005 by Sunplus Technology Co., Ltd.             *
 *                                                                        *
 *  This software is copyrighted by and is the property of Sunplus        *
 *  Technology Co., Ltd. All rights are reserved by Sunplus Technology    *
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Sunplus Technology Co., Ltd.                       *
 *                                                                        *
 *  Sunplus Technology Co., Ltd. reserves the right to modify this        *
 *  software without notice.                                              *
 *                                                                        *
 *  Sunplus Technology Co., Ltd.                                          *
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 *  Author: Yi-Ming Kao                                                   *
 *                                                                        *
 *  Reviewer:                                                             *
 *                                                                        *
 **************************************************************************/
#ifndef __VID_MP4_PACKER_H__
#define __VID_MP4_PACKER_H__

#include "typedef.h"
#include "gp_avcodec.h"
#include "gp_mux.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                   G E N E R A L    C O N S T A N T S                   *
 **************************************************************************/
#define VIDEO_TRAK              1
#define AUDIO_TRAK              2
#define INFO_TRAK               3

#define IDX_NUM                 9
#define IDX_VID_STTS            0
#define IDX_VID_STSZ            1
#define IDX_VID_STCO            2
#define IDX_VID_STSS            3
#define IDX_VID_STSC            4
#define IDX_AUD_STSZ            5
#define IDX_AUD_STCO            6
#define IDX_AUD_STSC            7
#define IDX_AUD_STTS            8

#define FIRST_CHUNK_OFFSET      0x20
#define FTYP_BOX_SIZE           0x18
#define BOX_HEADER_SIZE         8

#define _3GP_ID_FTYP            0x66747970
#define _3GP_ID_MDAT            0x6D646174
#define _3GP_ID_MOOV            0x6D6F6F76
#define _3GP_ID_MVHD            0x6D766864
#define _3GP_ID_TKHD            0x746B6864
#define _3GP_ID_TRAK            0x7472616B
#define _3GP_ID_MDIA            0x6D646961
#define _3GP_ID_MDHD            0x6D646864
#define _3GP_ID_MINF            0x6D696E66
#define _3GP_ID_VMHD            0x766D6864
#define _3GP_ID_SMHD            0x736D6864
#define _3GP_ID_STBL            0x7374626C
#define _3GP_ID_STSD            0x73747364
#define _3GP_ID_STSC            0x73747363
#define _3GP_ID_STTS            0x73747473
#define _3GP_ID_STSZ            0x7374737A
#define _3GP_ID_STCO            0x7374636F
#define _3GP_ID_STSS            0x73747373
#define _3GP_ID_HMHD            0x6E6D6864
#define _3GP_ID_NMHD            0x686D6864

#define LONG_FILENAME_LEN       1024

#define ONLY_ONE_HOUR_REC

#define MODE_MP4  0x01
#define MODE_MOV  0x02

enum MovChannelLayoutTag {
    MOV_CH_LAYOUT_UNKNOWN               = 0xFFFF0000,
    MOV_CH_LAYOUT_USE_DESCRIPTIONS      = (  0 << 16) | 0,
    MOV_CH_LAYOUT_USE_BITMAP            = (  1 << 16) | 0,
    MOV_CH_LAYOUT_DISCRETEINORDER       = (147 << 16) | 0,
    MOV_CH_LAYOUT_MONO                  = (100 << 16) | 1,
    MOV_CH_LAYOUT_STEREO                = (101 << 16) | 2,
    MOV_CH_LAYOUT_STEREOHEADPHONES      = (102 << 16) | 2,
    MOV_CH_LAYOUT_MATRIXSTEREO          = (103 << 16) | 2,
    MOV_CH_LAYOUT_MIDSIDE               = (104 << 16) | 2,
    MOV_CH_LAYOUT_XY                    = (105 << 16) | 2,
    MOV_CH_LAYOUT_BINAURAL              = (106 << 16) | 2,
    MOV_CH_LAYOUT_AMBISONIC_B_FORMAT    = (107 << 16) | 4,
    MOV_CH_LAYOUT_QUADRAPHONIC          = (108 << 16) | 4,
    MOV_CH_LAYOUT_PENTAGONAL            = (109 << 16) | 5,
    MOV_CH_LAYOUT_HEXAGONAL             = (110 << 16) | 6,
    MOV_CH_LAYOUT_OCTAGONAL             = (111 << 16) | 8,
    MOV_CH_LAYOUT_CUBE                  = (112 << 16) | 8,
    MOV_CH_LAYOUT_MPEG_3_0_A            = (113 << 16) | 3,
    MOV_CH_LAYOUT_MPEG_3_0_B            = (114 << 16) | 3,
    MOV_CH_LAYOUT_MPEG_4_0_A            = (115 << 16) | 4,
    MOV_CH_LAYOUT_MPEG_4_0_B            = (116 << 16) | 4,
    MOV_CH_LAYOUT_MPEG_5_0_A            = (117 << 16) | 5,
    MOV_CH_LAYOUT_MPEG_5_0_B            = (118 << 16) | 5,
    MOV_CH_LAYOUT_MPEG_5_0_C            = (119 << 16) | 5,
    MOV_CH_LAYOUT_MPEG_5_0_D            = (120 << 16) | 5,
    MOV_CH_LAYOUT_MPEG_5_1_A            = (121 << 16) | 6,
    MOV_CH_LAYOUT_MPEG_5_1_B            = (122 << 16) | 6,
    MOV_CH_LAYOUT_MPEG_5_1_C            = (123 << 16) | 6,
    MOV_CH_LAYOUT_MPEG_5_1_D            = (124 << 16) | 6,
    MOV_CH_LAYOUT_MPEG_6_1_A            = (125 << 16) | 7,
    MOV_CH_LAYOUT_MPEG_7_1_A            = (126 << 16) | 8,
    MOV_CH_LAYOUT_MPEG_7_1_B            = (127 << 16) | 8,
    MOV_CH_LAYOUT_MPEG_7_1_C            = (128 << 16) | 8,
    MOV_CH_LAYOUT_EMAGIC_DEFAULT_7_1    = (129 << 16) | 8,
    MOV_CH_LAYOUT_SMPTE_DTV             = (130 << 16) | 8,
    MOV_CH_LAYOUT_ITU_2_1               = (131 << 16) | 3,
    MOV_CH_LAYOUT_ITU_2_2               = (132 << 16) | 4,
    MOV_CH_LAYOUT_DVD_4                 = (133 << 16) | 3,
    MOV_CH_LAYOUT_DVD_5                 = (134 << 16) | 4,
    MOV_CH_LAYOUT_DVD_6                 = (135 << 16) | 5,
    MOV_CH_LAYOUT_DVD_10                = (136 << 16) | 4,
    MOV_CH_LAYOUT_DVD_11                = (137 << 16) | 5,
    MOV_CH_LAYOUT_DVD_18                = (138 << 16) | 5,
    MOV_CH_LAYOUT_AUDIOUNIT_6_0         = (139 << 16) | 6,
    MOV_CH_LAYOUT_AUDIOUNIT_7_0         = (140 << 16) | 7,
    MOV_CH_LAYOUT_AUDIOUNIT_7_0_FRONT   = (148 << 16) | 7,
    MOV_CH_LAYOUT_AAC_6_0               = (141 << 16) | 6,
    MOV_CH_LAYOUT_AAC_6_1               = (142 << 16) | 7,
    MOV_CH_LAYOUT_AAC_7_0               = (143 << 16) | 7,
    MOV_CH_LAYOUT_AAC_OCTAGONAL         = (144 << 16) | 8,
    MOV_CH_LAYOUT_TMH_10_2_STD          = (145 << 16) | 16,
    MOV_CH_LAYOUT_TMH_10_2_FULL         = (146 << 16) | 21,
    MOV_CH_LAYOUT_AC3_1_0_1             = (149 << 16) | 2,
    MOV_CH_LAYOUT_AC3_3_0               = (150 << 16) | 3,
    MOV_CH_LAYOUT_AC3_3_1               = (151 << 16) | 4,
    MOV_CH_LAYOUT_AC3_3_0_1             = (152 << 16) | 4,
    MOV_CH_LAYOUT_AC3_2_1_1             = (153 << 16) | 4,
    MOV_CH_LAYOUT_AC3_3_1_1             = (154 << 16) | 5,
    MOV_CH_LAYOUT_EAC3_6_0_A            = (155 << 16) | 6,
    MOV_CH_LAYOUT_EAC3_7_0_A            = (156 << 16) | 7,
    MOV_CH_LAYOUT_EAC3_6_1_A            = (157 << 16) | 7,
    MOV_CH_LAYOUT_EAC3_6_1_B            = (158 << 16) | 7,
    MOV_CH_LAYOUT_EAC3_6_1_C            = (159 << 16) | 7,
    MOV_CH_LAYOUT_EAC3_7_1_A            = (160 << 16) | 8,
    MOV_CH_LAYOUT_EAC3_7_1_B            = (161 << 16) | 8,
    MOV_CH_LAYOUT_EAC3_7_1_C            = (162 << 16) | 8,
    MOV_CH_LAYOUT_EAC3_7_1_D            = (163 << 16) | 8,
    MOV_CH_LAYOUT_EAC3_7_1_E            = (164 << 16) | 8,
    MOV_CH_LAYOUT_EAC3_7_1_F            = (165 << 16) | 8,
    MOV_CH_LAYOUT_EAC3_7_1_G            = (166 << 16) | 8,
    MOV_CH_LAYOUT_EAC3_7_1_H            = (167 << 16) | 8,
    MOV_CH_LAYOUT_DTS_3_1               = (168 << 16) | 4,
    MOV_CH_LAYOUT_DTS_4_1               = (169 << 16) | 5,
    MOV_CH_LAYOUT_DTS_6_0_A             = (170 << 16) | 6,
    MOV_CH_LAYOUT_DTS_6_0_B             = (171 << 16) | 6,
    MOV_CH_LAYOUT_DTS_6_0_C             = (172 << 16) | 6,
    MOV_CH_LAYOUT_DTS_6_1_A             = (173 << 16) | 7,
    MOV_CH_LAYOUT_DTS_6_1_B             = (174 << 16) | 7,
    MOV_CH_LAYOUT_DTS_6_1_C             = (175 << 16) | 7,
    MOV_CH_LAYOUT_DTS_6_1_D             = (182 << 16) | 7,
    MOV_CH_LAYOUT_DTS_7_0               = (176 << 16) | 7,
    MOV_CH_LAYOUT_DTS_7_1               = (177 << 16) | 8,
    MOV_CH_LAYOUT_DTS_8_0_A             = (178 << 16) | 8,
    MOV_CH_LAYOUT_DTS_8_0_B             = (179 << 16) | 8,
    MOV_CH_LAYOUT_DTS_8_1_A             = (180 << 16) | 9,
    MOV_CH_LAYOUT_DTS_8_1_B             = (181 << 16) | 9,
};
/**************************************************************************
 *                             M A C R O S                                *
 **************************************************************************/


/**************************************************************************
 *                         D A T A   T Y P E S                            *
 **************************************************************************/


/**************************************************************************
 *                        G L O B A L   D A T A                           *
 **************************************************************************/


/**************************************************************************
 *                 E X T E R N A L   R E F E R E N C E S                  *
 **************************************************************************/


/**************************************************************************
 *               F U N C T I O N   D E C L A R A T I O N S                *
 **************************************************************************/

typedef struct MP4MuxInfo_s 
{
	vidVideoType_t vidType;       /* e.g. MPEG4, MJPG, etc */
	audAudioType_t audType;       /* e.g. AMR, AAC, etc */
	UINT32 width;         /* pixels in x-axis */
	UINT32 height;        /* pixels in y-axis */
	UINT32 frmRate;       /* frame rate (fps) */	
	UINT32 dpEn;          /* Data Partitioned enable */

	UINT32 vidExtraLen;   /* The count in bytes of the size of  video extra information */ 
	UINT8* pVidExtraData; /* video extra information */
	UINT32 audExtraLen;   /* The count in bytes of the size of  audio extra information */ 
	UINT8* pAudExtraData; /* audio extra information */	
	UINT32 vidStreamNum;  /* video stream number */
	UINT32 audStreamNum;  /* audio stream number */

	UINT32 audBitrate;    /* audio bitrate*/
	UINT16 audSR;         /* audio sample rate */
	UINT8  audChannel;    /* audio channel */
	UINT8  audBits;       /* Bits of per audio sample */

	UINT32 maxFileSize;
	UINT32 clusterSize;     /* jerry: add for cluster alighment */	
	UINT8  tempFile[LONG_FILENAME_LEN];

	UINT8* gpMP4PktBufStart;
	UINT8* gpMP4PktBufCurr;
	UINT8* gpMP4PktBufEnd;
	UINT8* gpMP4IdxBufStart[IDX_NUM];
	UINT8* gpMP4IdxBufCurr[IDX_NUM];
	UINT8* gpMP4IdxBufEnd[IDX_NUM];
	UINT8* gpMP4HeaderStart;

	UINT8  gbIdxFd[IDX_NUM];
	FILE*  gPackFd;
	UINT32 gEstSize;

	UINT32 gMP4CurOff;
	UINT32 gPrevSampleTime;
	UINT32 gVidRecSize;
	UINT32 gVidFrmNo;
	UINT32 gAudRecSize;
	UINT32 gAudFrmNo;

	UINT32 gSTTSEntryCnt;

	UINT32 gPackAudDuration;
	UINT32 gPackAudDurationT;
	UINT32 gPackVidDuration;
	UINT8  gbReserved;
	UINT32 gVidIFrmNo;
	UINT8* gFreeBuf;

	UINT32 mode;
}MP4MuxInfo_t;

SINT32 muxMP4Open(HANDLE out);
SINT32 muxMOVOpen(HANDLE out);
SINT32 muxMP4Close(SINT32 hd);
SINT32 muxMP4Pack(SINT32 hd, gpMuxPkt_t *pkt, gpEsType_t type);
SINT32 muxMP4Set(SINT32 hd, UINT32 param, UINT32 value);
SINT32 muxMP4Get(SINT32 hd, UINT32 param, UINT32 *value);

#ifdef __cplusplus
}
#endif
#endif /*__VID_MP4_PACKER_H__*/


