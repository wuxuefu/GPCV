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
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file    gp_lbp.h
 * @brief   Declaration of lbp driver.
 * @author  ytliao
 * @since   2014/3/20
 * @date    2014/3/20
 */
#ifndef _GP_LBP_H_
#define _GP_LBP_H_

#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define LBP_IOCTL_ID          'L'

#define LBP_IOCTL_GEN		_IOW(LBP_IOCTL_ID, 0, lbp_content_t)
#define LBP_IOCTL_AUTO		_IOW(LBP_IOCTL_ID, 1, lbp_content_t)
#define LBP_IOCTL_CMP		_IOW(LBP_IOCTL_ID, 2, lbp_cmp_t)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct lbp_content_s {
	unsigned int inImg;
	unsigned int outImg;
	unsigned int HammImg;
	int imageWidth;
	int imageHeight;
	int threshold;
	int format_mode;
} lbp_content_t;

typedef struct lbp_cmp_s {
	gp_bitmap_t srcImg;
	gp_bitmap_t dstImg;
	gp_rect_t srcRoi;
	gp_rect_t dstRoi;
	unsigned int* sum;
} lbp_cmp_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif /* _GP_LBP_H_ */
