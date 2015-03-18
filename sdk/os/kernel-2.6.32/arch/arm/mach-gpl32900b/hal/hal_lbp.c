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
#include <mach/kernel.h>
#include <mach/hal/hal_lbp.h>
#include <mach/hal/regmap/reg_lbp.h>
#include <mach/hal/regmap/reg_scu.h>
//#include <asm/div64.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define LBP_USE_IRQ			0x00040000
#define LBP_AUTO_EN_ON		0x00000001
#define LBP_GEN_EN_ON		0x00000002
#define LBP_CMP_EN_ON		0x00000004

#define YUV_FMT_YONLY		0x00000000
#define YUV_FMT_YUYV		0x00010000
#define YUV_FMT_UYVY		0x00020000

#define LBP_AUTO_DONE		0x00000001
#define LBP_GEN_DONE		0x00000002
#define LBP_CMP_DONE		0x00000004
#define LBP_CMP_CLEAR		0x00000008

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static LbpReg_t *pLBP = (LbpReg_t *)LOGI_ADDR_LBP_REG;
static UINT32 LbpMode = 0;
static int cmp_sum = 0;

SINT32
gpHalLBPGenMode(
	UINT32 inImg,
	UINT32 outImg,
	SINT16 width,
	SINT16 height,
	SINT16 threshold,
	SINT32 format_mode
)
{
	SINT32 yuv_mode = YUV_FMT_YUYV;
	
	pLBP->LbpCtrl &= 0xf8;
	LbpMode = 0;

	pLBP->ImgStrAddr = inImg;
	pLBP->ImgSize = (width << 8) | height;	
	pLBP->LbpGenOBAddr = outImg;
	
	if(format_mode == SP_BITMAP_YCbCr400 || format_mode == SP_BITMAP_YUV400)	
		yuv_mode = YUV_FMT_YONLY;
	else if(format_mode == SP_BITMAP_YUYV || format_mode == SP_BITMAP_YCbYCr)
		yuv_mode = YUV_FMT_YUYV;	
	else if(format_mode == SP_BITMAP_UYVY)
		yuv_mode = YUV_FMT_UYVY;
	
	LbpMode = LBP_GEN_EN_ON;
	pLBP->LbpCtrl = LBP_USE_IRQ | yuv_mode | (threshold << 8) | LBP_GEN_EN_ON;
	//while(1)
	//{
	//	if(pLBP->LbpStatus & LBP_GEN_DONE) break;
	//}
	//
	//pLBP->LbpStatus = LBP_GEN_DONE;
	
	return 0;
}

SINT32
gpHalLBPAutoMode(
	UINT32 inImg,
	UINT32 outImg,
	UINT32 HammImg,
	SINT16 width,
	SINT16 height,
	SINT16 threshold,
	SINT32 format_mode
)
{
	SINT32 yuv_mode = YUV_FMT_YUYV;
	
	pLBP->LbpCtrl &= 0xf8;
	LbpMode = 0;

	pLBP->ImgStrAddr = inImg;
	pLBP->ImgSize = (width << 8) | height;	
	pLBP->LbpGenOBAddr = outImg;
	pLBP->LbpCmpOBAddr = HammImg;
	
	if(format_mode == SP_BITMAP_YCbCr400 || format_mode == SP_BITMAP_YUV400)	
		yuv_mode = YUV_FMT_YONLY;
	else if(format_mode == SP_BITMAP_YUYV || format_mode == SP_BITMAP_YCbYCr)
		yuv_mode = YUV_FMT_YUYV;	
	else if(format_mode == SP_BITMAP_UYVY)
		yuv_mode = YUV_FMT_UYVY;
	
	pLBP->LbpCtrl = LBP_USE_IRQ | yuv_mode | (threshold << 8) | LBP_AUTO_EN_ON;
	LbpMode = LBP_AUTO_EN_ON;
	//while(1)
	//{
	//	if(pLBP->LbpStatus & LBP_AUTO_DONE) break;
	//}
	//
	//pLBP->LbpStatus = LBP_AUTO_DONE;
	
	return 0;
}

SINT32
gpHalLBPCmpMode(
	UINT32 srcImg,
	UINT32 dstImg,
	SINT32 width,
	SINT32 height,
	UINT32 stride,
	SINT32 format_mode,
	gp_rect_t srcRoi,
	gp_rect_t dstRoi
)
{
	SINT32 yuv_mode;
	UINT32 srcImg_ptr, dstImg_ptr;
	
	
	pLBP->LbpCtrl &= 0xf8;
	LbpMode = 0;
		
	yuv_mode = YUV_FMT_YONLY;
			
	srcImg_ptr = srcImg + srcRoi.y * stride + srcRoi.x;
	dstImg_ptr = dstImg + dstRoi.y * stride + dstRoi.x;
		
	pLBP->LbpOCodeAddr = srcImg_ptr;
	pLBP->LbpNCodeAddr = dstImg_ptr;
	pLBP->LbpImgSize = (width << 8) | height;
	pLBP->LbpBlkSize = (srcRoi.width << 8) | srcRoi.height;	
	
	pLBP->LbpCtrl = LBP_USE_IRQ | (yuv_mode << 16) | (0 << 8) | LBP_CMP_EN_ON;
	LbpMode = LBP_CMP_EN_ON;
	// wait for total sum done
	//while(1) {
	//	
	//	if(pLBP->LbpStatus & LBP_CMP_DONE) break;
	//}	
    //
	//*sum = pLBP->TotalSum;
	//pLBP->LbpStatus = LBP_CMP_DONE;
	
	return 0;
}

SINT32
gpHalLBPGetTotalSum(
	void
)
{
	return cmp_sum;	
}

SINT32 
gpHalLBPGetIRQStatus(
	void
)
{
	if (LbpMode == LBP_AUTO_EN_ON && (pLBP->LbpStatus & LBP_AUTO_DONE))
		pLBP->LbpStatus = LBP_AUTO_DONE;
	else if (LbpMode == LBP_GEN_EN_ON && (pLBP->LbpStatus & LBP_GEN_DONE))
		pLBP->LbpStatus = LBP_GEN_DONE;
	else if (LbpMode == LBP_CMP_EN_ON && (pLBP->LbpStatus & LBP_CMP_DONE))
	{
		cmp_sum = pLBP->TotalSum;
		pLBP->LbpStatus = LBP_CMP_DONE;
	}
	else
		return 0;
 
	return 1;
}