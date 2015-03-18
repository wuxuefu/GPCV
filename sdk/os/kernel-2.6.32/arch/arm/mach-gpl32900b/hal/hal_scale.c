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
#include <mach/hal/hal_scale.h>
#include <mach/hal/regmap/reg_scale.h>
#include <mach/hal/regmap/reg_scu.h>
#include <asm/div64.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
//color space matrix
#define RGB_TO_YUV_CSM_10_00	0x0004B04C
#define RGB_TO_YUV_CSM_30_20	0x0000001D
#define RGB_TO_YUV_CSM_11_01	0x0022A42B
#define RGB_TO_YUV_CSM_31_21	0x03FB707F
#define RGB_TO_YUV_CSM_12_02	0x0023587F
#define RGB_TO_YUV_CSM_32_22	0x03FB7414

#define YUV_TO_RGB_CSM_10_00	0x00000100
#define YUV_TO_RGB_CSM_30_20	0x85960167
#define YUV_TO_RGB_CSM_11_01	0x0022B900
#define YUV_TO_RGB_CSM_31_21	0x0436CCB6
#define YUV_TO_RGB_CSM_12_02	0x000E3100
#define YUV_TO_RGB_CSM_32_22	0x87102000

#define C_SCALE0_INT_EN				(1 << 15)
#define C_SCALE0_STRETCH_EN			(1 << 14)	//16~235 -> 0 ~ 255
#define C_SCALE0_COLOR_SPACE_EN		(1 << 13)

#define C_SCALE0_HSCL_EN			(1 << 5)
#define C_SCALE0_HSCL_UP			(0 << 4)
#define C_SCALE0_HSCL_DOWN			(1 << 4)
#define C_SCALE0_VSCL_EN			(1 << 3)
#define C_SCALE0_VSCL_UP			(0 << 2)
#define C_SCALE0_VSCL_DOWN			(1 << 2)
#define C_SCALE0_HVSCAL_MASK		(0xF << 2)	

#define C_SCALE0_RESET				(1 << 1)
#define C_SCALE0_START				(1 << 0)
#define C_SCALE0_BUSY				(1 << 0)
#define C_SCALE0_INT_DONE_BIT		(1 << 0)

#define C_SCALE0_IN_X_MAX			0xFFFF
#define C_SCALE0_IN_Y_MAX			0xFFFF

#define C_SCALE0_OUT_X_MAX			0xFFFF
#define C_SCALE0_OUT_Y_MAX			0xFFFF

#define C_SCALE0_STATUS_BUSY		0x00000001
#define C_SCALE0_STATUS_DONE		0x00000002
#define C_SCALE0_STATUS_STOP		0x00000004
#define C_SCALE0_STATUS_TIMEOUT		0x00000008
#define C_SCALE0_STATUS_INIT_ERR	0x00000010


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
static Scale0Reg_t *pScale0 = (Scale0Reg_t *)LOGI_ADDR_SCALE0_REG;


/**
 * @brief   Scale0 module reset function
 * @param   enable[in]: disable(0)/enable(1)
 * @return  None
 * @see
 */
void
gpHalScale0ModuleRest(
	UINT32 enable
)
{
	scuaReg_t *scuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG;

	if(enable) {
		scuaReg->scuaPeriRst |= (1<<27);
		scuaReg->scuaPeriRst &= ~(1<<27);
	}
}

/**
 * @brief   Scale0 hardware reset function
 * @param   
 * @return  
 */
void
gpHalScale0Reset(
	void
)
{
	pScale0->Scale0Ctrl = C_SCALE0_RESET;
#if 1
	while(pScale0->Scale0Ctrl & C_SCALE0_RESET) {
		udelay(1);
	}
#endif
}

/**
 * @brief   Scale0 hardware init function
 * @param   
 * @return 
 */
void
gpHalScale0Init(
	void
)
{
	pScale0->Scale0Ctrl = 0;
	pScale0->Scale0Status = 0;
}

/**
 * @brief   Set input buffer address
 * @param   YAddr [in]: input Y address
 * @param   YCbddr [in]: input Cb address
 * @param   YCrddr [in]: input Cr address
 * @return  >= 0: success, < 0: fail 
 */
SINT32
gpHalScale0SetInputAddr(
	UINT32 YAddr,
	UINT32 CbAddr,
	UINT32 CrAddr
)
{
	SINT32 ret = STATUS_OK;
	
	// Make sure all addresses are 4-byte alignment
	if ((YAddr & 0x3) || (CbAddr & 0x3) || (CrAddr & 0x3)) {
		YAddr &= ~0x3;
		CbAddr &= ~0x3;
		CrAddr &= ~0x3;
		ret = STATUS_FAIL;
	}

	pScale0->Scale0SrcYAddr = YAddr;
	pScale0->Scale0SrcCbAddr = CbAddr;
	pScale0->Scale0SrcCrAddr = CrAddr;
	return ret;
}

/**
 * @brief   Set input format and input size
 * @param   ifmt [in]: input format
 * @param   input_x [in]: input source width
 * @param   input_y [in]: input source height
 * @return  >= 0: success, < 0: fail 
 */
SINT32
gpHalScale0SetInput(
	UINT32 ifmt, 
	UINT32 input_x, 
	UINT32 input_y
)
{
	UINT32 bpp;
	SINT32 ret = STATUS_OK;
	
	switch(ifmt & (0x0F << 9))
	{
		case C_SCALE0_IN_ARGB8888:
		case C_SCALE0_IN_ABGR8888:
		case C_SCALE0_IN_RAGB8888:
		case C_SCALE0_IN_RGAB8888:
		case C_SCALE0_IN_RGBA8888:
			bpp = input_x*4;
			break;
			
		case C_SCALE0_IN_RGB888:
		case C_SCALE0_IN_BGR888:
			bpp = input_x*3;
			break;
			
		case C_SCALE0_IN_RGB565:
		case C_SCALE0_IN_RBG556:
		case C_SCALE0_IN_GBR655:
		case C_SCALE0_IN_ARGB1555:
		case C_SCALE0_IN_ARGB5155:
		case C_SCALE0_IN_ARGB5515:
			bpp = input_x*2;
			break;

		case C_SCALE0_IN_CrYCbY422:
		case C_SCALE0_IN_CbYCrY422:
		case C_SCALE0_IN_YCrYCb422:
		case C_SCALE0_IN_YCbYCr422:
			bpp = input_x*2;
			break;

		case C_SCALE0_IN_YCbCr400:
			bpp = input_x*1;
			break;

		case C_SCALE0_IN_YCbCr444:
			bpp = input_x*3;
			break;

		case C_SCALE0_IN_YCbCr444_PLANAR:
		case C_SCALE0_IN_YCbCr444_SEMI_PLANAR:
		case C_SCALE0_IN_YCbCr422_PLANAR:
		case C_SCALE0_IN_YCbCr422_SEMI_PLANAR:
		case C_SCALE0_IN_YCbCr420_PLANAR:
		case C_SCALE0_IN_YCbCr420_SEMI_PLANAR:
			bpp = input_x*1;
			break;
			
		default:
			return STATUS_FAIL;
	}
	
	if((input_x > C_SCALE0_IN_X_MAX) || (input_y > C_SCALE0_IN_Y_MAX)) {
		input_x &= C_SCALE0_IN_X_MAX;
		input_y &= C_SCALE0_IN_Y_MAX;
		ret = STATUS_FAIL;
	}
	
	// src_width *bpp must be 4-align 
	if(bpp & 0x03) {
		ret = STATUS_FAIL;	
	}
	
	pScale0->Scale0Ctrl &= ~C_SCALE0_IN_MASK;
	pScale0->Scale0Ctrl |= ifmt;
	pScale0->Scale0SrcXYWidth = (input_x << 16) | input_y;
	return ret;
}

/**
 * @brief   Set input action offset and size
 * @param   act_x_offset [in]: action x coordinate
 * @param   act_y_offset [in]: action y coordinate
 * @param   act_width [in]: input action width
 * @param   act_height [in]: input action height
 * @return  >= 0: success, < 0: fail 
 */
SINT32
gpHalScale0SetInputAct(
	UINT32 act_x_offset, 
	UINT32 act_y_offset, 
	UINT32 act_width, 
	UINT32 act_height
)
{
	SINT32 ret = STATUS_OK;

	pScale0->Scale0ActOffset = (act_x_offset << 16) | act_y_offset;
	pScale0->Scale0ActWidth = (act_width << 16) | act_height;
	return ret;
}

/**
 * @brief   Set output base address
 * @param   base_addr [in]: output base address
 * @return  >= 0: success, < 0: fail 
 */
SINT32 
gpHalScale0SetOutputAddr(
	UINT32 base_addr
)
{
	SINT32 ret = STATUS_OK;
	
	// Make sure all addresses are 4-byte alignment
	if(base_addr & 0x3)  {
		base_addr &= ~0x3;
		ret = STATUS_FAIL;
	}

	pScale0->Scale0BGAddr = base_addr;
	return ret;
}

/**
 * @brief   Set output format, background offset and size 
 * @param   ofmt [in]: output format
 * @param   bg_width [in]: background output width
 * @param   bg_height [in]: background output height
 * @param   dst_x_offset [in]: destination x offset
 * @param   dst_y_offset [in]: destination y offset 
 * @return  >= 0: success, < 0: fail 
 */
SINT32
gpHalScale0SetOutput(
	UINT32 ofmt, 
	UINT32 bg_width, 
	UINT32 bg_height, 
	UINT32 dst_x_offset, 
	UINT32 dst_y_offset
)
{
	UINT32 bpp;
	SINT32 ret = STATUS_OK;
	
	switch(ofmt & (0x07 << 6))
	{
		case C_SCALE0_OUT_ARGB8888:
		case C_SCALE0_OUT_ABGR8888:
		case C_SCALE0_OUT_RAGB8888:
		case C_SCALE0_OUT_RGAB8888:
		case C_SCALE0_OUT_RGBA8888:
			bpp = 4;
			break;
					
		case C_SCALE0_OUT_RGB888:	
		case C_SCALE0_OUT_BGR888:
			bpp = 3;
			break;
					
		case C_SCALE0_OUT_RGB565:
		case C_SCALE0_OUT_ARGB1555:
		case C_SCALE0_OUT_ARGB5515:
			bpp = 2;
			break;		

		case C_SCALE0_OUT_YCbCr444:
			bpp = 3;
			break;
		
		case C_SCALE0_OUT_CrYCbY422:
		case C_SCALE0_OUT_CbYCrY422:
		case C_SCALE0_OUT_YCrYCb422:
		case C_SCALE0_OUT_YCbYCr422:
			bpp = 2;
			break;	
			
		case C_SCALE0_OUT_YCbCr400:
			bpp = 1;	
			break;
			
		default:
			return STATUS_FAIL;
	}

	// bg_width * bpp must be 4-align 
	if((bg_width * bpp) & 0x03) {
		ret = STATUS_FAIL;
	}
	
	// dst_x_offset * bpp must be 4-align 
	if((dst_x_offset * bpp) & 0x03) {
		ret = STATUS_FAIL;
	}

	pScale0->Scale0Ctrl &= ~C_SCALE0_OUT_MASK;
	pScale0->Scale0Ctrl |= ofmt;	
	pScale0->Scale0BGXYWidth = (bg_width << 16) | bg_height;
	pScale0->Scale0DstXYOffset = (dst_x_offset << 16) | dst_y_offset;
	return ret;	
}

/**
 * @brief   Set destination size
 * @param   dst_width [in]: destination width
 * @param   dst_height [in]: destination height
 * @return  >= 0: success, < 0: fail 
 */
SINT32 
gpHalScale0SetOutputDst(
	UINT32 dst_width, 
	UINT32 dst_height
)
{
	pScale0->Scale0DstXYWidth = (dst_width << 16) | dst_height;
	return STATUS_OK;	
}

/**
 * @brief   Set color space matrix for RGB to YUV or YUV to RGB
 * @param  
 * @return  
 */
static void
gpHalScale0SetCorSpacMartix(
	void
)
{
	UINT32 reg, ifmt, ofmt;
	
	reg = pScale0->Scale0Ctrl; 
	ifmt = reg & (0x0F << 9);
	ofmt = reg & (0x07 << 6);
	switch(ifmt)
	{
		case C_SCALE0_IN_ARGB8888:
		case C_SCALE0_IN_RGB888:
		case C_SCALE0_IN_RGB565:
		case C_SCALE0_IN_ARGB1555:
		case C_SCALE0_IN_ARGB5515:
			switch(ofmt)
			{
				case C_SCALE0_OUT_YCbCr444:
				case C_SCALE0_OUT_CrYCbY422:
				case C_SCALE0_OUT_YCbCr400:
					pScale0->Scale0Ctrl |= C_SCALE0_COLOR_SPACE_EN;
					pScale0->Scale0CSM_10_00 = RGB_TO_YUV_CSM_10_00;
					pScale0->Scale0CSM_30_20 = RGB_TO_YUV_CSM_30_20;
					pScale0->Scale0CSM_11_01 = RGB_TO_YUV_CSM_11_01;
					pScale0->Scale0CSM_31_21 = RGB_TO_YUV_CSM_31_21;
					pScale0->Scale0CSM_12_02 = RGB_TO_YUV_CSM_12_02;
					pScale0->Scale0CSM_32_22 = RGB_TO_YUV_CSM_32_22;
					break;
					
				default:
					pScale0->Scale0Ctrl &= ~C_SCALE0_COLOR_SPACE_EN;		
			}
			break;
			
		case C_SCALE0_IN_YCbCr444:
		case C_SCALE0_IN_CrYCbY422:
		case C_SCALE0_IN_YCbCr400:
		case C_SCALE0_IN_YCbCr422_PLANAR:
		case C_SCALE0_IN_YCbCr422_SEMI_PLANAR:
		case C_SCALE0_IN_YCbCr444_PLANAR:
		case C_SCALE0_IN_YCbCr444_SEMI_PLANAR:
		case C_SCALE0_IN_YCbCr420_PLANAR:
		case C_SCALE0_IN_YCbCr420_SEMI_PLANAR:
			switch(ofmt)
			{
				case C_SCALE0_OUT_ARGB8888:
				case C_SCALE0_OUT_RGB888:
				case C_SCALE0_OUT_RGB565:
				case C_SCALE0_OUT_ARGB1555:
				case C_SCALE0_OUT_ARGB5515:
					pScale0->Scale0Ctrl |= C_SCALE0_COLOR_SPACE_EN;
					pScale0->Scale0CSM_10_00 = YUV_TO_RGB_CSM_10_00;
					pScale0->Scale0CSM_30_20 = YUV_TO_RGB_CSM_30_20;
					pScale0->Scale0CSM_11_01 = YUV_TO_RGB_CSM_11_01;
					pScale0->Scale0CSM_31_21 = YUV_TO_RGB_CSM_31_21;
					pScale0->Scale0CSM_12_02 = YUV_TO_RGB_CSM_12_02;
					pScale0->Scale0CSM_32_22 = YUV_TO_RGB_CSM_32_22;
					break;
					
				default:
					pScale0->Scale0Ctrl &= ~C_SCALE0_COLOR_SPACE_EN;		
			}
			break;
			
		default:
			return;
	}
}

/**
 * @brief   Scale0 Start
 * @param   int_en[in]: 1: enable interrupt
 * @return  >= 0: success, < 0: fail 
 */
SINT32 
gpHalScale0Start(
	UINT32 int_en
)
{
	UINT32 act_width, act_height;
	UINT32 dst_width, dst_height;
	UINT32 reg;
	UINT64 temp;
	
	reg = pScale0->Scale0ActWidth;
	act_width = reg >> 16;
	act_height = reg & 0xFFFF;
	
	reg = pScale0->Scale0DstXYWidth;
	dst_width = reg >> 16;
	dst_height = reg & 0xFFFF;	
	
	pScale0->Scale0Ctrl &= ~C_SCALE0_HVSCAL_MASK;
	pScale0->ScaleOHInitValue = 0x00;
	pScale0->ScaleOVInitValue = 0x00;
	
	if(dst_width > act_width) {
		//h scale up
		temp = (UINT64)act_width << 24;
		reg = do_div(temp, dst_width);
		pScale0->Scale0HFactor = (UINT32)temp;
		pScale0->Scale0Ctrl |= C_SCALE0_HSCL_UP;
		pScale0->Scale0Ctrl |= C_SCALE0_HSCL_EN;
	} else if(dst_width < act_width) {
		//h scale down
		temp = (UINT64)dst_width << 24;
		reg = do_div(temp, act_width);
		if(reg) {
			temp++;
		}
		
		pScale0->Scale0HFactor = (UINT32)temp;
		pScale0->Scale0Ctrl |= C_SCALE0_HSCL_DOWN;
		pScale0->Scale0Ctrl |= C_SCALE0_HSCL_EN;
	} else {
		//h no scale
		pScale0->Scale0Ctrl &= ~C_SCALE0_HSCL_EN;
	}
	
	if(dst_height > act_height) {
		//v scale up
		temp = (UINT64)act_height << 24;
		reg = do_div(temp, dst_height);
		pScale0->Scale0VFactor = (UINT32)temp;
		pScale0->Scale0Ctrl |= C_SCALE0_VSCL_UP;
		pScale0->Scale0Ctrl |= C_SCALE0_VSCL_EN;
	} else if(dst_height < act_height) {
		//v scale down
		temp = (UINT64)dst_height << 24;
		reg = do_div(temp, act_height);
		if(reg) {
			temp++;
		}
		
		pScale0->Scale0VFactor = (UINT32)temp;
		pScale0->Scale0Ctrl |= C_SCALE0_VSCL_DOWN;
		pScale0->Scale0Ctrl |= C_SCALE0_VSCL_EN;
	} else {
		//v no scale
		pScale0->Scale0Ctrl &= ~C_SCALE0_VSCL_EN;
	}
	
	// set color space transfer
	gpHalScale0SetCorSpacMartix();
	
	if(int_en) {
		pScale0->Scale0Ctrl |= C_SCALE0_INT_EN | C_SCALE0_START;
	} else {
		pScale0->Scale0Ctrl |= C_SCALE0_START;
	}
	return STATUS_OK;	
}

/**
 * @brief   Scale0 get Status
 * @param  
 * @return  1: idle, 0: busy 
 */
SINT32 
gpHalScale0GetStatus(
	void
)
{
	if(pScale0->Scale0Ctrl & C_SCALE0_BUSY) {
		return 0;
	}
 
	return 1;
}

/**
 * @brief   Scale0 get IRQ Status
 * @param  
 * @return  0: not happened, 1: happened 
 */
SINT32 
gpHalScale0GetIRQStatus(
	void
)
{
	if(pScale0->Scale0Status & C_SCALE0_INT_DONE_BIT) {
		pScale0->Scale0Status = 0;
		return 1;
	}
 
	return 0;
}

/**
 * @brief   Scale0 color space matrix
 * @param   enable[in]: 1:enable 0:disable
 * @return  >= 0: success, < 0: fail 
 */
SINT32 
gpHalScale0SetColorSpaceMatrix(
	UINT32 enable,
	Scale0ColorSpaceTrans_t *matrix
)
{
	if(enable) {
		pScale0->Scale0Ctrl |= C_SCALE0_COLOR_SPACE_EN;	
		pScale0->Scale0CSM_10_00 = ((UINT32)matrix->csm10 << 16) | matrix->csm00;	
		pScale0->Scale0CSM_30_20 = ((UINT32)matrix->csm30 << 16) | matrix->csm20;	
		pScale0->Scale0CSM_11_01 = ((UINT32)matrix->csm11 << 16) | matrix->csm01;	
		pScale0->Scale0CSM_31_21 = ((UINT32)matrix->csm31 << 16) | matrix->csm21;	
		pScale0->Scale0CSM_12_02 = ((UINT32)matrix->csm12 << 16) | matrix->csm02;	
		pScale0->Scale0CSM_32_22 = ((UINT32)matrix->csm32 << 16) | matrix->csm22;	
	} else {
		pScale0->Scale0Ctrl &= ~C_SCALE0_COLOR_SPACE_EN;
	}
}

/**
 * @brief   Scale0 set stretch
 * @param   enable[in]: 1: 0~255, 0: 16~235
 * @return  >= 0: success, < 0: fail 
 */
SINT32 
gpHalScale0SetStretch(
	UINT32 enable
)
{
	if(enable) {
		pScale0->Scale0Ctrl |= C_SCALE0_STRETCH_EN;	
	} else {
		pScale0->Scale0Ctrl &= ~C_SCALE0_STRETCH_EN;
	}
}

/**
 * @brief   Scale0 set chorma threshold
 * @param   YMax[in]: Y max
 * @param   YMin[in]: Y min
 * @param   CbCrMax[in]: CbCr max
 * @param   CbCrMin[in]: CbCr min
 * @return  >= 0: success, < 0: fail 
 */
void 
gpHalScale0SetChormaThr(
	UINT32 YMax, 
	UINT32 YMin, 
	UINT32 CbCrMax, 
	UINT32 CbCrMin
)
{
	pScale0->Scale0ChormaThr = (CbCrMax & 0xFF) << 24 | 
							(CbCrMin & 0xFF) << 16 | 
							(YMax & 0xFF) << 8 | 
							(YMin & 0xFF);
}

/**
 * @brief   Scale0 Set Dst BG Color
 * @param   color[in]
 * @return  
 */
void 
gpHalScale0SetBoundaryColor(
	UINT32 color
)
{
	pScale0->Scale0BoundaryColor = color;
}

/**
 * @brief   Scale hardware register dump function
 */
void
gpHalScaleRegDump(
	void
)
{
	printk("=== SCALER0 REG DUMP ===\n");
	printk("Scale0SrcYAddr    = %08X\n", pScale0->Scale0SrcYAddr);
	printk("Scale0SrcCbAddr   = %08X\n", pScale0->Scale0SrcCbAddr);
	printk("Scale0SrcCrAddr   = %08X\n", pScale0->Scale0SrcCrAddr);
	printk("Scale0SrcXYWidth  = %08X\n", pScale0->Scale0SrcXYWidth);

	printk("Scale0ActOffset   = %08X\n", pScale0->Scale0ActOffset);
	printk("Scale0ActWidth    = %08X\n", pScale0->Scale0ActWidth);
	printk("Scale0BGAddr      = %08X\n", pScale0->Scale0BGAddr);
	printk("Scale0DstXYWidth  = %08X\n", pScale0->Scale0DstXYWidth);

	printk("Scale0HInitValue  = %08X\n", pScale0->ScaleOHInitValue);
	printk("Scale0HFactor     = %08X\n", pScale0->Scale0HFactor);
	printk("Scale0VInitValue  = %08X\n", pScale0->ScaleOVInitValue);
	printk("Scale0VFactor     = %08X\n", pScale0->Scale0VFactor);

	printk("Scale0CSM_10_00   = %08X\n", pScale0->Scale0CSM_10_00);
	printk("Scale0CSM_30_20   = %08X\n", pScale0->Scale0CSM_30_20);
	printk("Scale0CSM_11_01   = %08X\n", pScale0->Scale0CSM_11_01);
	printk("Scale0CSM_31_21   = %08X\n", pScale0->Scale0CSM_31_21);

	printk("Scale0CSM_12_02   = %08X\n", pScale0->Scale0CSM_12_02);
	printk("Scale0CSM_32_22   = %08X\n", pScale0->Scale0CSM_32_22);
	printk("Scale0ChormaThr   = %08X\n", pScale0->Scale0ChormaThr);
	printk("Scale0Arbit       = %08X\n", pScale0->Scale0Arbit);

	printk("Scale0Ctrl        = %08X\n", pScale0->Scale0Ctrl);
	printk("Scale0Status      = %08X\n", pScale0->Scale0Status);
	printk("Scale0BGXYWidth   = %08X\n", pScale0->Scale0BGXYWidth);
	printk("Scale0DstXYOffset = %08X\n", pScale0->Scale0DstXYOffset);
	printk("Scale0BoundColor  = %08X\n", pScale0->Scale0BoundaryColor);
}

