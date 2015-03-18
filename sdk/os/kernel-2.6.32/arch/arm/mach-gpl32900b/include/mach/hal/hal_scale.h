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
#ifndef _HAL_SCALE_H_
#define _HAL_SCALE_H_

#include <mach/hal/hal_common.h>
#include <mach/gp_scale.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#ifndef STATUS_OK
#define STATUS_OK		0
#endif

#ifndef STATUS_FAIL
#define STATUS_FAIL		-1
#endif

/* input format */ 
#define C_SCALE0_IN_ARGB8888		((0 << 9) | (0 << 27))
#define C_SCALE0_IN_ABGR8888		((0 << 9) | (5 << 27))
#define C_SCALE0_IN_RAGB8888		((0 << 9) | (7 << 27))
#define C_SCALE0_IN_RGAB8888		((0 << 9) | (8 << 27))
#define C_SCALE0_IN_RGBA8888		((0 << 9) | (9 << 27))

#define C_SCALE0_IN_RGB888			((1 << 9) | (0 << 27)) 
#define C_SCALE0_IN_BGR888			((1 << 9) | (5 << 27))

#define C_SCALE0_IN_RGB565			((2 << 9) | (0 << 27))
#define C_SCALE0_IN_RBG556			((2 << 9) | (1 << 27))
#define C_SCALE0_IN_GBR655			((2 << 9) | (2 << 27))

#define C_SCALE0_IN_ARGB1555		((3 << 9) | (0 << 27))
#define C_SCALE0_IN_ARGB5155		((3 << 9) | (1 << 27))
#define C_SCALE0_IN_ARGB5515		((4 << 9) | (0 << 27))

#define C_SCALE0_IN_YCbCr444		(5 << 9)

#define C_SCALE0_IN_CrYCbY422		((6 << 9) | (0 << 27))
#define C_SCALE0_IN_CbYCrY422		((6 << 9) | (14 << 27))
#define C_SCALE0_IN_YCrYCb422		((6 << 9) | (7 << 27))
#define C_SCALE0_IN_YCbYCr422		((6 << 9) | (9 << 27))

#define C_SCALE0_IN_YCbCr400				(7 << 9)
#define C_SCALE0_IN_YCbCr422_PLANAR			(8 << 9)
#define C_SCALE0_IN_YCbCr422_SEMI_PLANAR	(9 << 9)
#define C_SCALE0_IN_YCbCr444_PLANAR			(10 << 9)
#define C_SCALE0_IN_YCbCr444_SEMI_PLANAR	(11 << 9)
#define C_SCALE0_IN_YCbCr420_PLANAR			(12 << 9)
#define C_SCALE0_IN_YCbCr420_SEMI_PLANAR	(13 << 9)
#define C_SCALE0_IN_MASK					((0xF << 9) | (0x01F << 27))

//output format 
#define C_SCALE0_OUT_ARGB8888		((0 << 6) | (0 << 16))
#define C_SCALE0_OUT_ABGR8888		((0 << 6) | (5 << 16))
#define C_SCALE0_OUT_RAGB8888		((0 << 6) | (7 << 16))
#define C_SCALE0_OUT_RGAB8888		((0 << 6) | (8 << 16))
#define C_SCALE0_OUT_RGBA8888		((0 << 6) | (9 << 16))

#define C_SCALE0_OUT_RGB888			((1 << 6) | (0 << 16))
#define C_SCALE0_OUT_BGR888			((1 << 6) | (5 << 16))

#define C_SCALE0_OUT_RGB565			(2 << 6)
#define C_SCALE0_OUT_ARGB1555		(3 << 6)
#define C_SCALE0_OUT_ARGB5515		(4 << 6)
#define C_SCALE0_OUT_YCbCr444		(5 << 6)

#define C_SCALE0_OUT_CrYCbY422		(6 << 6)
#define C_SCALE0_OUT_CbYCrY422		((6 << 6) | (14 << 22))
#define C_SCALE0_OUT_YCrYCb422		((6 << 6) | (7 << 22))
#define C_SCALE0_OUT_YCbYCr422		((6 << 6) | (9 << 22))

#define C_SCALE0_OUT_YCbCr400		(7 << 6)
#define C_SCALE0_OUT_MASK			((7 << 6) | (0x7FF << 16))

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct Scale0ColorSpaceTrans_s
{
	unsigned short csm00;
	unsigned short csm01;
	unsigned short csm02;
	unsigned short csm10;
	unsigned short csm11;
	unsigned short csm12;
	unsigned short csm20;
	unsigned short csm21;
	unsigned short csm22;
	unsigned short csm30;
	unsigned short csm31;
	unsigned short csm32;
}Scale0ColorSpaceTrans_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**
 * @brief   Scale0 module reset function
 */
void gpHalScale0ModuleRest(UINT32 enable);

/**
 * @brief   Scale0 hardware reset function
 */
void gpHalScale0Reset(void);

/**
 * @brief   Scale0 hardware init function
 */
void gpHalScale0Init(void);

/**
 * @brief   Set input buffer address
 * @param   YAddr [in]: input Y address
 * @param   YCbddr [in]: input Cb address
 * @param   YCrddr [in]: input Cr address
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetInputAddr(UINT32 YAddr, UINT32 CbAddr, UINT32 CrAddr);

/**
 * @brief   Set input format and input size
 * @param   ifmt [in]: input format
 * @param   input_x [in]: input source width
 * @param   input_y [in]: input source height
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetInput(UINT32 ifmt, UINT32 input_x, UINT32 input_y);


/**
 * @brief   Set input action offset and size
 * @param   act_x_offset [in]: action x coordinate
 * @param   act_y_offset [in]: action y coordinate
 * @param   act_width [in]: input action width
 * @param   act_height [in]: input action height
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetInputAct(UINT32 act_x_offset, UINT32 act_y_offset, UINT32 act_width, UINT32 act_height);

/**
 * @brief   Set output base address
 * @param   base_addr [in]: output base address
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetOutputAddr(UINT32 base_addr);

/**
 * @brief   Set output format, background offset and size 
 * @param   ofmt [in]: output format
 * @param   bg_width [in]: background output width
 * @param   bg_height [in]: background output height
 * @param   dst_x_offset [in]: destination x offset
 * @param   dst_y_offset [in]: destination y offset 
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetOutput(UINT32 ofmt, UINT32 bg_width, UINT32 bg_height, UINT32 dst_x_offset, UINT32 dst_y_offset);

/**
 * @brief   Set destination size
 * @param   dst_width [in]: destination width
 * @param   dst_height [in]: destination height
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetOutputDst(UINT32 dst_width, UINT32 dst_height);

/**
 * @brief   Scale0 Start
 * @param  	int_en[in]: 1: enable interrupt
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0Start(UINT32 int_en);

/**
 * @brief   Scale0 get Status
 * @param  
 * @return  1: idle, 0: busy 
 */
SINT32 gpHalScale0GetStatus(void);

/**
 * @brief   Scale0 get IRQ Status
 * @param  
 * @return  0: not happened, 1: happened 
 */
SINT32 gpHalScale0GetIRQStatus(void);

/**
 * @brief   Scale0 color space matrix
 * @param   enable[in]: 1:enable 0:disable
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetColorSpaceMatrix(UINT32 enable, Scale0ColorSpaceTrans_t *matrix);

/**
 * @brief   Scale0 set stretch
 * @param   enable[in]: 1: 0~255, 0: 16~235
 * @return  >= 0: success, < 0: fail 
 */
SINT32 gpHalScale0SetStretch(UINT32 enable);

/**
 * @brief   Scale0 set chorma threshold
 * @param   YMax[in]: Y max
 * @param   YMin[in]: Y min
 * @param   CbCrMax[in]: CbCr max
 * @param   CbCrMin[in]: CbCr min
 * @return  >= 0: success, < 0: fail 
 */
void gpHalScale0SetChormaThr(UINT32 YMax, UINT32 YMin, UINT32 CbCrMax, UINT32 CbCrMin);

/**
 * @brief   Scale0 Set Dst BG Color
 * @param   color[in]
 * @return  
 */
void gpHalScale0SetBoundaryColor(UINT32 color);

/**
 * @brief   Scale hardware register dump function
 */
void gpHalScaleRegDump(void);

#endif /* _HAL_SCALE_H_ */
