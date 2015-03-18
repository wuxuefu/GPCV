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
 * @file    gp_cpx.h
 * @brief   Declaration of Cpx driver.
 * @author  qinjian
 * @since   2010/10/13
 * @date    2010/10/13
 */
#ifndef _GP_CPX_H_
#define _GP_CPX_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define CPX_BIN_TYPE_CODE      0
#define CPX_BIN_TYPE_DATA      1

/* Ioctl for device node definition */
#define CPX_IOCTL_ID           'S'
#define CPX_IOCTL_RESET        _IOW(CPX_IOCTL_ID, 1, unsigned int)
#define CPX_IOCTL_LOAD         _IOW(CPX_IOCTL_ID, 2, cpx_load_bin_t)
#define CPX_IOCTL_BOOT         _IO(CPX_IOCTL_ID,  3)
#define CPX_IOCTL_INIT         _IOW(CPX_IOCTL_ID, 4, cpx_video_decode_t)
#define CPX_IOCTL_TRIGGER      _IOW(CPX_IOCTL_ID, 5, cpx_video_decode_t)
#define CPX_IOCTL_FREE         _IOW(CPX_IOCTL_ID, 6, cpx_video_decode_t)
#define CPX_IOCTL_CODEC_TYPE   _IOW(CPX_IOCTL_ID, 7, int)
#define CPX_IOCTL_CHECKSUM     _IOW(CPX_IOCTL_ID, 8, int)
#define CPX_IOCTL_DUMPCODE     _IOW(CPX_IOCTL_ID, 9, int)

/* Codec types */
#define CPX_DECODE         0
#define CPX_ENCODE         1
#define CPX_GAME           2



/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/* CPX_IOCTL_LOAD command argument */
typedef struct cpx_load_bin_s {
	unsigned int type;              /*!< @brief bin type: CPX_BIN_TYPE_CODE/DATA */
	unsigned int src_addr;          /*!< @brief start address of data/code in ARM */
	unsigned int dst_addr;          /*!< @brief loading address in CPX */
	unsigned int size;              /*!< @brief size of data in bytes */
	unsigned int need_reply;        /*!< @brief 0:no reply, 1:need reply */
} cpx_load_bin_t;

/* CPX_IOCTL_INIT/TRIGGER/FREE command argument */
typedef struct cpx_video_decode_s {
	unsigned int  time_stamp_in;    /*!< @brief [in] time stamp of input decoded frame */
	unsigned char *in_buf;          /*!< @brief [in] buffer to keep input bitstream */
	unsigned int  useful_bytes;     /*!< @brief [in] availabe bytes in in_buf */
	unsigned char *frame_buf;       /*!< @brief [in] large frame (requires 16-byte alignment) */
	unsigned int  frame_buf_size;   /*!< @brief [in/out] availabe/required large frame buffer size */

	unsigned int  time_stamp_out;   /*!< @brief [out] time stamp of output displayed frame */
	unsigned char *out_buf[3];      /*!< @brief [out] buffer to keep decoded YUV data */
	unsigned int  width;            /*!< @brief [out] frame width */
	unsigned int  height;           /*!< @brief [out] frame height */
	unsigned int  stride;           /*!< @brief [out] Y frame buffer stride */
	unsigned int  stride_chroma;    /*!< @brief [out] UV frame buffer stride */
	unsigned int  frame_type;       /*!< @brief [out] I/P/B etc. */

	unsigned int  flags;            /*!< @brief [in/out] ref VFLAG_XXX */
	void          *dec_handle;      /*!< @brief [out] codec private working space */
	unsigned char *version;         /*!< @brief [out] library version */
} cpx_video_decode_t;

typedef struct cpx_video_encode_s {
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
	unsigned char *out_buf;         /*!< @brief [out] buffer to keep output bitstream */
	unsigned int  out_bytes;        /*!< @brief [out] availabe bytes in out_buf */

	unsigned int  flags;            /*!< @brief [in/out] ref VFLAG_XXX */
	void          *enc_handle;      /*!< @brief [out] codec private working space */
	unsigned char *version;         /*!< @brief [out] library version */
} cpx_video_encode_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif /* _GP_CPX_H_ */
