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
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file    gp_ceva.h
 * @brief   Declaration of Ceva driver.
 * @author  qinjian
 * @since   2010/10/13
 * @date    2010/10/13
 */
#ifndef _GP_CEVA_H_
#define _GP_CEVA_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define CEVA_OK                  0
#define CEVA_ERR                 -1
#define CEVA_ERR_RESUMED         0x7FFFFFFF
#define CEVA_ERR_LOADED          0x7FFFFFFE

#define CEVA_BIN_TYPE_CODE       0
#define CEVA_BIN_TYPE_DATA       1

/* Ioctl for device node definition */
#define CEVA_IOCTL_ID            'S'
#define CEVA_IOCTL_RESET         _IOW(CEVA_IOCTL_ID, 1, unsigned int)
#define CEVA_IOCTL_LOAD          _IOW(CEVA_IOCTL_ID, 2, ceva_load_bin_t)
#define CEVA_IOCTL_BOOT          _IO(CEVA_IOCTL_ID,  3)
#define CEVA_IOCTL_INIT          _IOW(CEVA_IOCTL_ID, 4, ceva_video_decode_t)
#define CEVA_IOCTL_TRIGGER       _IOW(CEVA_IOCTL_ID, 5, ceva_video_decode_t)
#define CEVA_IOCTL_FREE          _IOW(CEVA_IOCTL_ID, 6, ceva_video_decode_t)
#define CEVA_IOCTL_CODEC_TYPE    _IOW(CEVA_IOCTL_ID, 7, int)
#define CEVA_IOCTL_CHECKSUM      _IOW(CEVA_IOCTL_ID, 8, int)
#define CEVA_IOCTL_DUMPCODE      _IOW(CEVA_IOCTL_ID, 9, int)
#define CEVA_IOCTL_SET_CLOCK     _IOW(CEVA_IOCTL_ID, 10, ceva_clock_t)
#define CEVA_IOCTL_SET_DBGFLAG   _IOW(CEVA_IOCTL_ID, 11, unsigned int)
#define CEVA_IOCTL_SET_PRIORITY  _IOW(CEVA_IOCTL_ID, 12, unsigned int)

/* Codec types */
#define CEVA_DECODE              0
#define CEVA_ENCODE              1
#define CEVA_GAME                2

#define CEVA_CODEC_MAIN_TYPE_MASK    0x000000FF
#define CEVA_CODEC_SUB_TYPE_MASK     0x00FFFF00
#define CEVA_CODEC_SUB_TYPE_SHIFT    8

/* Ceva codec image types */
#define CEVA_DEC_MPEG4SP		CXDEC(0x0001)
#define CEVA_DEC_MPEG4HD		CXDEC(0x0002)
#define CEVA_DEC_MPEG4ASP	CXDEC(0x0003)
#define CEVA_DEC_RV8			CXDEC(0x0004)
#define CEVA_DEC_RV9			CXDEC(0x0005)
#define CEVA_DEC_MJPEG		CXDEC(0x0006)
#define CEVA_DEC_H264		CXDEC(0x0007)
#define CEVA_DEC_H264HP		CXDEC(0x0008)
#define CEVA_DEC_H263		CXDEC(0x0009)
#define CEVA_DEC_VC1			CXDEC(0x000A)
#define CEVA_DEC_MPEG2		CXDEC(0x000B)
#define CEVA_DEC_DIVX3		CXDEC(0x000C)
#define CEVA_DEC_WMV78		CXDEC(0x000D)
#define CEVA_DEC_JPEG		CXDEC(0x000E)
#define CEVA_DEC_VP6			CXDEC(0x000F)
#define CEVA_DEC_THEORA		CXDEC(0x0010)

#define CEVA_ENC_JPEG		CXENC(0x0001)
#define CEVA_ENC_MPEG4SP		CXENC(0x0002)
#define CEVA_ENC_H264		CXENC(0x0003)
#define CEVA_ENC_MJPEG		CXENC(0x0004)

#define CEVA_GAME_KGB		CXGAME(0x0001)
#define CEVA_GAME_PSX       	CXGAME(0x0002)

/* Ceva debug flags */
#define DBGFLAG_MAIN_LVL0           0x01
#define DBGFLAG_MAIN_LVL1           0x02
#define DBGFLAG_MAIN_LVL2           0x04

#define DBGFLAG_CMDDISP_LVL0        0x10
#define DBGFLAG_CMDDISP_LVL1        0x20
#define DBGFLAG_CMDDISP_LVL2        0x40
#define DBGFLAG_CMDDISP_OS_LOAD     0x80
#define DBGFLAG_CMDDISP_DECODE      0x100

#define DBGFLAG_GLOBAL_MSG2ARM      0x4000
#define DBGFLAG_GLOBAL_MSG_OFF      0x8000

/* NOTE: the highest 8 bits of debug variables are reserved for
 *		ARM global system control. Device driver, including DSP, should
 *		not use these 8 bits.
 */
#define DBGFLAG_GLOBAL_L1_OFF       0x00100000
#define DBGFLAG_GLOBAL_L2_OFF       0x00200000
#define DBGFLAG_GLOBAL_L2_BK0_OFF   0x00200000
#define DBGFLAG_GLOBAL_L2_BK1_OFF   0x00400000

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

#define CEVA_CODEC_MAIN_TYPE(t)      (t & CEVA_CODEC_MAIN_TYPE_MASK)
#define CEVA_CODEC_SUB_TYPE(t)       ((t & CEVA_CODEC_SUB_TYPE_MASK) >> CEVA_CODEC_SUB_TYPE_SHIFT)

#define CXIMG(type,id)	((type) | ((id) << CEVA_CODEC_SUB_TYPE_SHIFT))
#define CXDEC(id)		CXIMG(CEVA_DECODE, id)
#define CXENC(id)		CXIMG(CEVA_ENCODE, id)
#define CXGAME(id)		CXIMG(CEVA_GAME, id)

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/* CEVA_IOCTL_SET_CLOCK command argument */
typedef struct ceva_clock_s {
	unsigned int ceva_ratio;         /*!< @brief CEVA clock ratio */
	unsigned int ceva_ahb_ratio;     /*!< @brief CEVA AHB clock ratio */
	unsigned int ceva_apb_ratio;     /*!< @brief CEVA APB clock ratio */
} ceva_clock_t;

/* CEVA_IOCTL_LOAD command argument */
typedef struct ceva_load_bin_s {
	unsigned int type;              /*!< @brief bin type: CEVA_BIN_TYPE_CODE/DATA */
	unsigned int src_addr;          /*!< @brief start address of data/code in ARM */
	unsigned int dst_addr;          /*!< @brief loading address in CEVA */
	unsigned int size;              /*!< @brief size of data in bytes */
	unsigned int need_reply;        /*!< @brief 0:no reply, 1:need reply */
} ceva_load_bin_t;

/* CEVA_IOCTL_INIT/TRIGGER/FREE command argument */
typedef struct ceva_video_decode_s {
	unsigned int  time_stamp_in;    /*!< @brief [in] time stamp of input decoded frame */
	unsigned char *in_buf;          /*!< @brief [in] buffer to keep input bitstream */
	unsigned int  useful_bytes;     /*!< @brief [in] availabe bytes in in_buf */
	unsigned char *frame_buf;       /*!< @brief [in] large frame (requires 16-byte alignment) */
	unsigned int  frame_buf_size;   /*!< @brief [in/out] availabe/required large frame buffer size */

	unsigned int  time_stamp_out;   /*!< @brief [out] time stamp of output displayed frame */
	unsigned char *out_buf[3];      /*!< @brief [out] buffer to keep decoded YUV data */
	unsigned int  width;            /*!< @brief [in/out] frame width (for JPEG PARTDEC (x0<<16)|y0) */
	unsigned int  height;           /*!< @brief [in/out] frame height (for JPEG PARTDEC (x1<<16)|y1) */
	unsigned int  stride;           /*!< @brief [out] Y frame buffer stride */
	unsigned int  stride_chroma;    /*!< @brief [out] UV frame buffer stride */
	unsigned int  frame_type;       /*!< @brief [out] I/P/B etc. */

	unsigned int  flags;            /*!< @brief [in/out] ref VFLAG_XXX */
	void          *dec_handle;      /*!< @brief [out] codec private working space */
	unsigned char *version;         /*!< @brief [out] library version */
} ceva_video_decode_t;

typedef struct ceva_video_encode_s {
	unsigned int  time_stamp_in;    /*!< @brief [in] time stamp of input decoded frame */
	unsigned char *in_buf[3];       /*!< @brief [in] buffer to keep YUV input data */
	unsigned char *frame_buf;       /*!< @brief [in] large frame(requires 16-byte alignment) */
	unsigned int  frame_buf_size;   /*!< @brief [in/out] availabe/required large frame buffer size */
	unsigned int  width;            /*!< @brief [in] frame width */
	unsigned int  height;           /*!< @brief [in] frame height */
	unsigned int  bit_rate;         /*!< @brief [in] bitrate in kbps */
	unsigned int  pic_rate;         /*!< @brief [in] picture rate */
	unsigned int  gop_len;          /*!< @brief [in] key-frame interval */

	/* if profile==0 then bframes=QType=interlaced=0 */
	unsigned int  profile;          /*!< @brief [in] 0(baseline or basic profile)/1(main profile)/2(advance or high profile) */
	unsigned int  b_frames;         /*!< @brief [in] Number of B-frames between I and P */
	unsigned int  q_type;           /*!< @brief [in] Quantization type: 0(H.263)/1(MPEG) */
	unsigned int  interlaced;       /*!< @brief [in] 0(progressive coding)/1(interlace coding) */
	unsigned int  dis_inloop_filter;/*!< @brief [in] disable inloop filter */

	unsigned int  vbr;              /*!< @brief [in] vbr? 0(CBR)/1(VBR) */
	unsigned int  rc_mode;          /*!< @brief [in] bitrate control mode; 1(frame level)/0(macroblock level) */

	unsigned int  time_stamp_out;   /*!< @brief [out] time stamp of output displayed frame */
	unsigned char *out_buf;         /*!< @brief [in/out] buffer to keep output bitstream */
	unsigned int  out_bytes;        /*!< @brief [out] availabe bytes in out_buf */

	unsigned int  flags;            /*!< @brief [in/out] ref VFLAG_XXX */
	void          *enc_handle;      /*!< @brief [out] codec private working space */
	unsigned char *version;         /*!< @brief [out] library version */
} ceva_video_encode_t;

typedef struct ceva_game_kgb_s {
    unsigned short *pRegBuf;
    unsigned short *pOamBuf;
    unsigned char  *pVramBuf;
    unsigned char  *pOutBuf;
    unsigned int   reserve[13];
} ceva_game_kgb_t;

typedef struct ceva_game_psx_s {
	unsigned short *baseAddress;     //BaseAddress = (unsigned short *)pcsxMalloc(PSX4ALL_VSIZE);
	unsigned short *frameBuffer;     //1024*1024
	unsigned int reserve[10];
} ceva_game_psx_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**
 * @brief   Ceva run NOP code
 */
int gp_ceva_nop(void);

/**
 * @brief   Ceva clock enable/disable
 * @param   enable [in] 0:disable, 1:enable
 * @return  None
 * @see
 */
void ceva_clock_enable(int enable);

#endif /* _GP_CEVA_H_ */
