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
#include <mach/module.h>
#include <mach/kernel.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <linux/cdev.h>
#include <mach/hal/hal_scale.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_cache.h>
#include <mach/gp_scale.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define SCALE_TIMEOUT   3000
#define DEBUG_DUMP      0
#define POLLING_TEST	0

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define DERROR	printk
#if DEBUG_DUMP == 1
#define DEBUG	printk
#else
#define DEBUG(...)
#endif

//#define DOWN(sem)	down(sem)
//#define DOWN(sem)	if(down_killable(sem) != 0) { DERROR("Scale0DOWNError\r\n"); return -ERESTARTSYS; }
#define DOWN(sem)	if(down_interruptible(sem) != 0) { DERROR("Scale0DOWNError\r\n"); return -ERESTARTSYS; }

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct scale_info_s {
	struct miscdevice dev;          /*!< @brief scale device */
	struct semaphore sem;           /*!< @brief mutex semaphore for scale ops */
	wait_queue_head_t done_wait;    /*!< @brief scaling done wait queue */
	bool done;                      /*!< @brief scaling done flag */
	unsigned int open_count;        /*!< @brief scale device open count */
} scale_info_t;

typedef struct gpScale0Format_s
{
	/* img input para */
	unsigned int input_format;			/* input format*/
	unsigned short input_width;	
	unsigned short input_height;	
	unsigned short act_width;		
	unsigned short act_height;			
	unsigned short act_x_offset;		
	unsigned short act_y_offset;
	
	unsigned int input_y_addr;			/* input y addr, must be 4-align */
	unsigned int input_cb_addr;			/* input cb addr, must be 4-align */
	unsigned int input_cr_addr;			/* input cr addr, must be 4-align */
	
	/* img output para */
	unsigned int output_format;			/* output format*/
	unsigned short dst_width;	
	unsigned short dst_height;	
	unsigned short dst_x_offset;	
	unsigned short dst_y_offset;	
	unsigned short output_buf_width;	
	unsigned short output_buf_height;
	unsigned int output_addr;			/* output y addr, must be 4-align */
	
	/* function */
	unsigned int stretch_en;
	unsigned char YMax;
	unsigned char YMin;
	unsigned char CbCrMax;
	unsigned char CbCrMin;
	unsigned int BoundaryColor;
}gpScale0Format_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static scale_info_t *scale = NULL;


static void 
gp_scale_param_dump(
	gpScale0Format_t Scale0
)
{
	DERROR(KERN_WARNING "in_fmt   = 0x%x\n", Scale0.input_format);
	DERROR(KERN_WARNING "input_wh = %d x %d\n", Scale0.input_width, Scale0.input_height);
	DERROR(KERN_WARNING "act_wh   = %d x %d\n", Scale0.act_width, Scale0.act_height);
	DERROR(KERN_WARNING "act_xy   = %d x %d\n", Scale0.act_x_offset, Scale0.act_y_offset);
	DERROR(KERN_WARNING "input_y  = 0x%x\n", Scale0.input_y_addr);
	DERROR(KERN_WARNING "input_cb = 0x%x\n", Scale0.input_cb_addr);
	DERROR(KERN_WARNING "input_cr = 0x%x\n", Scale0.input_cr_addr);

	DERROR(KERN_WARNING "out_fmt  = 0x%x\n", Scale0.output_format);
	DERROR(KERN_WARNING "dst_wh   = %d x %d\n", Scale0.dst_width, Scale0.dst_height);
	DERROR(KERN_WARNING "dst_xy   = %d x %d\n", Scale0.dst_x_offset, Scale0.dst_y_offset);
	DERROR(KERN_WARNING "output_wh= %d x %d\n", Scale0.output_buf_width, Scale0.output_buf_height);
	DERROR(KERN_WARNING "output   = 0x%x\n", Scale0.output_addr);

	DERROR(KERN_WARNING "stretch  = 0x%x\n", Scale0.stretch_en);
	DERROR(KERN_WARNING "YMax     = 0x%x\n", Scale0.YMax);
	DERROR(KERN_WARNING "YMin     = 0x%x\n", Scale0.YMin);
	DERROR(KERN_WARNING "CbCrMax  = 0x%x\n", Scale0.CbCrMax);
	DERROR(KERN_WARNING "CbCrMin  = 0x%x\n", Scale0.CbCrMin);
	DERROR(KERN_WARNING "BDColor  = 0x%x\n", Scale0.BoundaryColor);
}

static void
gp_scalar_module_clk_en (
	int en
)
{
	gp_enable_clock( (int*)"SCALER0", en );
}

static unsigned int 
gp_scale0_get_input_fmt(
	unsigned int ifmt
)
{
	switch(ifmt)
	{
		case SP_BITMAP_RGB565:
			DEBUG("C_SCALE0_IN_RGB565\n");
			return C_SCALE0_IN_RGB565;	

		case SP_BITMAP_RGAB5515:
			DEBUG("C_SCALE0_IN_ARGB5515\n");
			return C_SCALE0_IN_ARGB5515;

		case SP_BITMAP_ARGB1555:	
			DEBUG("C_SCALE0_IN_ARGB1555\n");
			return C_SCALE0_IN_ARGB1555;

		case SP_BITMAP_ARGB4444:
			DEBUG("NotSupportFmt\n");
			return -1;

		case SP_BITMAP_RGB888:
			DEBUG("C_SCALE0_IN_RGB888\n");
			return C_SCALE0_IN_RGB888;

		case SP_BITMAP_ARGB8888:
			DEBUG("C_SCALE0_IN_ARGB8888\n");
			return C_SCALE0_IN_ARGB8888;

		case SP_BITMAP_RGBA8888:
			DEBUG("C_SCALE0_IN_RGBA8888\n");
			return C_SCALE0_IN_RGBA8888;

		case SP_BITMAP_BGR565:		
		case SP_BITMAP_RGB555:		
		case SP_BITMAP_BGAR5515:	
		case SP_BITMAP_ABGR1555:	
		case SP_BITMAP_ABGR4444:
			DEBUG("NotSupportFmt\n");
			return -1;

		case SP_BITMAP_BGR888:
			DEBUG("C_SCALE0_IN_BGR888\n");
			return C_SCALE0_IN_BGR888;

		case SP_BITMAP_ABGR8888:
			DEBUG("C_SCALE0_IN_ABGR8888\n");
			return C_SCALE0_IN_ABGR8888;

		case SP_BITMAP_BGRG:
		case SP_BITMAP_GBGR:
		case SP_BITMAP_RGBG:
		case SP_BITMAP_GRGB:
		case SP_BITMAP_1BPP:
		case SP_BITMAP_2BPP:
		case SP_BITMAP_4BPP:
		case SP_BITMAP_8BPP:
			DEBUG("NotSupportFmt\n");
			return -1;
			
		case SP_BITMAP_YCbCr:	
		case SP_BITMAP_YUV:
			DEBUG("NotSupportFmt\n");
			return -1;

		case SP_BITMAP_YCbYCr:				
		case SP_BITMAP_YUYV:
			DEBUG("C_SCALE0_IN_CrYCbY422\n");
			return C_SCALE0_IN_CrYCbY422;

		case SP_BITMAP_YVYU:
			DEBUG("C_SCALE0_IN_CbYCrY422\n");
			return C_SCALE0_IN_CbYCrY422;

		case SP_BITMAP_UYVY:
			DEBUG("C_SCALE0_IN_YCrYCb422\n");
			return C_SCALE0_IN_YCrYCb422;

		case SP_BITMAP_VYUY:
			DEBUG("C_SCALE0_IN_YCbYCr422\n");
			return C_SCALE0_IN_YCbYCr422;
	
		case SP_BITMAP_4Y4U4Y4V:
		case SP_BITMAP_4Y4Cb4Y4Cr:
			DEBUG("C_SCALE0_IN_YCbCr444\n");
			return C_SCALE0_IN_YCbCr444;

		case SP_BITMAP_YCbCr400:
		case SP_BITMAP_YUV400:
		case SP_BITMAP_SEMI400:
			DEBUG("C_SCALE0_IN_YCbCr400\n");
			return C_SCALE0_IN_YCbCr400;

		case SP_BITMAP_YCbCr422:
		case SP_BITMAP_YUV422:	
			DEBUG("C_SCALE0_IN_YCbCr422_PLANAR\n");
			return C_SCALE0_IN_YCbCr422_PLANAR;

		case SP_BITMAP_YCbCr444:
		case SP_BITMAP_YUV444:
			DEBUG("C_SCALE0_IN_YCbCr444_PLANAR\n");
			return C_SCALE0_IN_YCbCr444_PLANAR;

		case SP_BITMAP_YCbCr420:
		case SP_BITMAP_YUV420: 
			DEBUG("C_SCALE0_IN_YCbCr420_PLANAR\n");
			return C_SCALE0_IN_YCbCr420_PLANAR;

		case SP_BITMAP_SEMI420: 
			DEBUG("C_SCALE0_IN_YCbCr420_SEMI_PLANAR\n");
			return C_SCALE0_IN_YCbCr420_SEMI_PLANAR;

		case SP_BITMAP_SEMI422:
			DEBUG("C_SCALE0_IN_YCbCr422_SEMI_PLANAR\n");
			return C_SCALE0_IN_YCbCr422_SEMI_PLANAR;

		case SP_BITMAP_SEMI444:
			DEBUG("C_SCALE0_IN_YCbCr444_SEMI_PLANAR\n");
			return C_SCALE0_IN_YCbCr444_SEMI_PLANAR;
	}

	DEBUG("NotSupportFmt\n");
	return -1;
}

static unsigned int 
gp_scale0_get_output_fmt(
	unsigned int ofmt
)
{
	switch(ofmt)
	{
		case SP_BITMAP_RGB565:
			DEBUG("C_SCALE0_OUT_RGB565\n");
			return C_SCALE0_OUT_RGB565;	

		case SP_BITMAP_RGAB5515:
			DEBUG("C_SCALE0_OUT_ARGB5515\n");
			return C_SCALE0_OUT_ARGB5515;

		case SP_BITMAP_ARGB1555:
			DEBUG("C_SCALE0_OUT_ARGB1555\n");
			return C_SCALE0_OUT_ARGB1555;

		case SP_BITMAP_ARGB4444:	
			DEBUG("NotSupportFmt\n");
			return -1;

		case SP_BITMAP_RGB888:
			DEBUG("C_SCALE0_OUT_RGB888\n");
			return C_SCALE0_OUT_RGB888;

		case SP_BITMAP_ARGB8888:
			DEBUG("C_SCALE0_OUT_ARGB8888\n");
			return C_SCALE0_OUT_ARGB8888;

		case SP_BITMAP_RGBA8888:
			DEBUG("C_SCALE0_OUT_RGBA8888\n");
			return C_SCALE0_OUT_RGBA8888;

		case SP_BITMAP_BGR565:		
		case SP_BITMAP_RGB555:		
		case SP_BITMAP_BGAR5515:	
		case SP_BITMAP_ABGR1555:	
		case SP_BITMAP_ABGR4444:
			DEBUG("NotSupportFmt\n");
			return -1;

		case SP_BITMAP_BGR888:		
			DEBUG("C_SCALE0_OUT_BGR888\n");
			return C_SCALE0_OUT_BGR888;

		case SP_BITMAP_ABGR8888:
			DEBUG("C_SCALE0_OUT_ABGR8888\n");
			return C_SCALE0_OUT_ABGR8888;

		case SP_BITMAP_BGRG:
		case SP_BITMAP_GBGR:
		case SP_BITMAP_RGBG:
		case SP_BITMAP_GRGB:
		case SP_BITMAP_1BPP:
		case SP_BITMAP_2BPP:
		case SP_BITMAP_4BPP:
		case SP_BITMAP_8BPP:
			DEBUG("NotSupportFmt\n");
			return -1;
			
		case SP_BITMAP_YCbCr:	
		case SP_BITMAP_YUV:
			DEBUG("NotSupportFmt\n");
			return -1;

		case SP_BITMAP_YCbYCr:				
		case SP_BITMAP_YUYV:
			DEBUG("C_SCALE0_OUT_CrYCbY422\n");
			return C_SCALE0_OUT_CrYCbY422;
			
		case SP_BITMAP_YVYU:
			DEBUG("C_SCALE0_OUT_CbYCrY422\n");
			return C_SCALE0_OUT_CbYCrY422;
			
		case SP_BITMAP_UYVY:
			DEBUG("C_SCALE0_OUT_YCrYCb422\n");
			return C_SCALE0_OUT_YCrYCb422;

		case SP_BITMAP_VYUY:
			DEBUG("C_SCALE0_OUT_YCbYCr422\n");
			return C_SCALE0_OUT_YCbYCr422;			
	
		case SP_BITMAP_4Y4U4Y4V:
		case SP_BITMAP_4Y4Cb4Y4Cr:
			DEBUG("C_SCALE0_IN_YCbCr444\n");
			return C_SCALE0_OUT_YCbCr444;

		case SP_BITMAP_YCbCr400:
		case SP_BITMAP_YUV400:
		case SP_BITMAP_SEMI400:
			DEBUG("C_SCALE0_OUT_YCbCr400\n");
			return C_SCALE0_OUT_YCbCr400;

		case SP_BITMAP_YCbCr422:	
		case SP_BITMAP_YUV422:
		case SP_BITMAP_YCbCr444:
		case SP_BITMAP_YUV444:
		case SP_BITMAP_YCbCr420:	
		case SP_BITMAP_YUV420:
		case SP_BITMAP_SEMI420: 
		case SP_BITMAP_SEMI422:
		case SP_BITMAP_SEMI444:	
			DEBUG("NotSupportFmt\n");
			return -1;
	}

	DEBUG("NotSupportFmt\n");
	return -1;
}

#ifndef GP_SYNC_OPTION
static int
gp_scale0_get_bpl(
	unsigned int format,
	unsigned int width,
	unsigned int *bpl_y,
	unsigned int *bpl_u,
	unsigned int *bpl_v
)
{
	int ret = 0;

	*bpl_y = 0;
	*bpl_u = 0;
	*bpl_v = 0;
	switch (format) 
	{
		case C_SCALE0_IN_ARGB8888:
		case C_SCALE0_IN_ABGR8888:
		case C_SCALE0_IN_RAGB8888:
		case C_SCALE0_IN_RGAB8888:
		case C_SCALE0_IN_RGBA8888:
			*bpl_y = width << 2;
			break;
		
		case C_SCALE0_IN_RGB888:
		case C_SCALE0_IN_BGR888:
			*bpl_y = width * 3;
			break;
			
		case C_SCALE0_IN_RGB565:
		case C_SCALE0_IN_RBG556:
		case C_SCALE0_IN_GBR655:
		case C_SCALE0_IN_ARGB1555:
		case C_SCALE0_IN_ARGB5155:
		case C_SCALE0_IN_ARGB5515:
		case C_SCALE0_IN_CrYCbY422:
		case C_SCALE0_IN_CbYCrY422:	
		case C_SCALE0_IN_YCrYCb422:
		case C_SCALE0_IN_YCbYCr422:		
			*bpl_y = width << 1;
			break;	

		case C_SCALE0_IN_YCbCr400:
			*bpl_y = width;
			break;	

		case C_SCALE0_IN_YCbCr422_PLANAR:
			*bpl_y = width;
			*bpl_u = width >> 1;
			*bpl_v = width >> 1;
			break;
			
		case C_SCALE0_IN_YCbCr422_SEMI_PLANAR:
			*bpl_y = width;
			*bpl_u = width;
			*bpl_v = 0;
			break;

		case C_SCALE0_IN_YCbCr444_PLANAR:
			*bpl_y = width;
			*bpl_u = width;
			*bpl_v = width;
			break;
			
		case C_SCALE0_IN_YCbCr444_SEMI_PLANAR:
			*bpl_y = width;
			*bpl_u = width << 1;
			*bpl_v = 0;
			break;

		case C_SCALE0_IN_YCbCr420_PLANAR:
			*bpl_y = width;
			*bpl_u = width >> 2;
			*bpl_v = width >> 2;
			break;
			
		case C_SCALE0_IN_YCbCr420_SEMI_PLANAR:
			*bpl_y = width;
			*bpl_u = width >> 1;
			*bpl_v = 0;
			break;

		//case C_SCALE0_OUT_ARGB8888:
		case C_SCALE0_OUT_ABGR8888:
		case C_SCALE0_OUT_RAGB8888:
		case C_SCALE0_OUT_RGAB8888:
		case C_SCALE0_OUT_RGBA8888:
			*bpl_y = width << 2;
			*bpl_u = 0;
			*bpl_v = 0;
			break;

		case C_SCALE0_OUT_RGB888:
		case C_SCALE0_OUT_BGR888:	
			*bpl_y = width * 3;
			*bpl_u = 0;
			*bpl_v = 0;
			break;

		case C_SCALE0_OUT_RGB565:
		case C_SCALE0_OUT_ARGB1555:
		case C_SCALE0_OUT_ARGB5515:
		case C_SCALE0_OUT_CrYCbY422:
		case C_SCALE0_OUT_CbYCrY422:
		case C_SCALE0_OUT_YCrYCb422:
		case C_SCALE0_OUT_YCbYCr422:
			*bpl_y = width * 2;
			*bpl_u = 0;
			*bpl_v = 0;
			break;
			
		case C_SCALE0_OUT_YCbCr400:
			*bpl_y = width;
			*bpl_u = 0;
			*bpl_v = 0;
			break;
			
		default:
			ret = -1;
			break;
	}

	return ret;
}
#endif

int 
scale_trigger(
	scale_content_t *ctx
)
{
	int ret = 0;
#ifndef GP_SYNC_OPTION
	unsigned int addr, bpl_y, bpl_u, bpl_v;
#endif
	gpScale0Format_t Scale0;

	DEBUG(KERN_WARNING "%s\n", __FUNCTION__);
	memset((void *)&Scale0, 0x00, sizeof(Scale0));

	DOWN(&scale->sem);	

	/* get internal format */
	Scale0.input_format = gp_scale0_get_input_fmt(ctx->src_img.type);
	if(Scale0.input_format == 0xFFFFFFFF) {
		DERROR("input_format Err\n"); 
		ret = -1;
		goto out;
	}
	
	Scale0.output_format = gp_scale0_get_output_fmt(ctx->dst_img.type);
	if(Scale0.output_format == 0xFFFFFFFF) {
		DERROR("output_format Err\n"); 
		ret = -1;
		goto out;
	}
	
	Scale0.input_width = ctx->src_img.width;
	Scale0.input_height = ctx->src_img.height;

	Scale0.act_width = ctx->clip_rgn.width;
	Scale0.act_height = ctx->clip_rgn.height;
	
	Scale0.act_x_offset = ctx->clip_rgn.x;
	Scale0.act_y_offset = ctx->clip_rgn.y;

	Scale0.dst_width = ctx->scale_rgn.width;
	Scale0.dst_height = ctx->scale_rgn.height; 	

	Scale0.dst_x_offset = ctx->scale_rgn.x;
	Scale0.dst_y_offset = ctx->scale_rgn.y;

	Scale0.output_buf_width = ctx->dst_img.width; 
	Scale0.output_buf_height = ctx->dst_img.height;

	Scale0.stretch_en = 0;
	Scale0.YMax = 0xFF;
	Scale0.YMin = 0x00;
	Scale0.CbCrMax = 0xFF;
	Scale0.CbCrMin = 0x00;

	if(Scale0.act_width > 2048) {
		DERROR("Scale0.act_width > 2048\n");
		ret = -1;
		goto out;
	}

	// check input size 
	if((Scale0.input_width == 0) || (Scale0.input_height == 0)) {
		DERROR("Scale0.input_size Err\n");
		ret = -1;
		goto out;
	}

	// check clip size 
	if(Scale0.act_width == 0) {
		Scale0.act_width = Scale0.input_width;
		DERROR("AdJ act_width = %d\r\n", Scale0.act_width);
	}

	if(Scale0.act_height == 0) {
		Scale0.act_height = Scale0.input_height;
		DERROR("AdJ act_height = %d\r\n", Scale0.act_height);
	}
	
	if(((Scale0.act_width + Scale0.act_x_offset) > Scale0.input_width) || 
		((Scale0.act_height + Scale0.act_y_offset) > Scale0.input_height)) {
		DERROR("clip region must in src image.\n");
		ret = -1;
		goto out;
	}

	// check output size 
	if((Scale0.output_buf_width == 0) || (Scale0.output_buf_height == 0)) {
		DERROR("Scale0.output_size Err\n");
		ret = -1;
		goto out;
	}

	// check rgn size
	if(Scale0.dst_width == 0) {
		Scale0.dst_width = Scale0.output_buf_width;
		DERROR("AdJ dst_width = %d\r\n", Scale0.dst_width);
	}

	if(Scale0.dst_height == 0) {
		Scale0.dst_height = Scale0.output_buf_height;
		DERROR("AdJ dst_height = %d\r\n", Scale0.dst_height);
	}
	
	if(((Scale0.dst_width + Scale0.dst_x_offset) > Scale0.output_buf_width) || 
		((Scale0.dst_height + Scale0.dst_y_offset) > Scale0.output_buf_height)) {
		DERROR("rgn region must in dst image.\n");
		ret = -1;
		goto out;
	}
	
	/* translate address from user_va to pa */
	Scale0.input_y_addr = 0;
	Scale0.input_cb_addr = 0;
	Scale0.input_cr_addr = 0;
	Scale0.output_addr = 0;
	if(ctx->src_img.pData != NULL) {
		Scale0.input_y_addr = (unsigned int)gp_user_va_to_pa((void *)ctx->src_img.pData);
		if(Scale0.input_y_addr == 0) {
			Scale0.input_y_addr = (unsigned int)gp_chunk_pa((void *)ctx->src_img.pData);
		}
		if(Scale0.input_y_addr == 0) {
			Scale0.input_y_addr = (unsigned int)ctx->src_img.pData;
		}
	} 

	if(ctx->src_img.pDataU != NULL) {
		Scale0.input_cb_addr = (unsigned int)gp_user_va_to_pa((void *)ctx->src_img.pDataU);
		if(Scale0.input_cb_addr == 0) {
			Scale0.input_cb_addr = (unsigned int)gp_chunk_pa((void *)ctx->src_img.pDataU);
		}
		if(Scale0.input_cb_addr == 0) {
			Scale0.input_cb_addr = (unsigned int)ctx->src_img.pDataU;
		}
	} 

	if(ctx->src_img.pDataV != NULL) {
		Scale0.input_cr_addr = (unsigned int)gp_user_va_to_pa((void *)ctx->src_img.pDataV);
		if(Scale0.input_cr_addr == 0) {
			Scale0.input_cr_addr = (unsigned int)gp_chunk_pa((void *)ctx->src_img.pDataV);
		}
		if(Scale0.input_cr_addr == 0) {
			Scale0.input_cr_addr = (unsigned int)ctx->src_img.pDataV;
		}
	} 

	if(ctx->dst_img.pData != NULL) {
		Scale0.output_addr = (unsigned int)gp_user_va_to_pa((void *)ctx->dst_img.pData);
		if(Scale0.output_addr == 0) {
			Scale0.output_addr = (unsigned int)gp_chunk_pa((void *)ctx->dst_img.pData);
		}
		if(Scale0.output_addr == 0) {
			Scale0.output_addr = (unsigned int)ctx->dst_img.pData;
		}
	}

	DEBUG(KERN_WARNING "YAddr =  0x%x->0x%x\n", (unsigned int)ctx->src_img.pData, Scale0.input_y_addr);
	DEBUG(KERN_WARNING "CbAddr = 0x%x->0x%x\n", (unsigned int)ctx->src_img.pDataU, Scale0.input_cb_addr);
	DEBUG(KERN_WARNING "CrAddr = 0x%x->0x%x\n", (unsigned int)ctx->src_img.pDataV, Scale0.input_cr_addr);
	DEBUG(KERN_WARNING "OutAddr= 0x%x->0x%x\n", (unsigned int)ctx->dst_img.pData, Scale0.output_addr);

	/* clean dcache */
#ifndef GP_SYNC_OPTION
	gp_scale0_get_bpl(Scale0.input_format, Scale0.input_width, &bpl_y, &bpl_u, &bpl_v);
	if(bpl_y) {
		gp_clean_dcache_range((unsigned int)ctx->src_img.pData, Scale0.input_height * bpl_y);
	}
	
	if(bpl_u) {
		gp_clean_dcache_range((unsigned int)ctx->src_img.pDataU, Scale0.input_height * bpl_u);
	}

	if(bpl_v) {
		gp_clean_dcache_range((unsigned int)ctx->src_img.pDataU, Scale0.input_height * bpl_v);
	}
#else
	GP_SYNC_CACHE();
#endif

	/* scale0 init */
	scale->done = false;
	
	gpHalScale0Reset();
	
	gpHalScale0Init();

	//scale0 set input
	ret = gpHalScale0SetInputAddr(Scale0.input_y_addr, Scale0.input_cb_addr, Scale0.input_cr_addr);
	if(ret < 0) {
		DERROR("Scale0 Input Address must be 4 Align\n"); 
		goto out; 
	}

	ret = gpHalScale0SetInput(Scale0.input_format, Scale0.input_width, Scale0.input_height);
	if(ret < 0) {
		DERROR("Scale0 input bpl must be 4 Align\n"); 
		goto out; 
	}

	ret = gpHalScale0SetInputAct(Scale0.act_x_offset, Scale0.act_y_offset, Scale0.act_width, Scale0.act_height);
	if(ret < 0) {
		DERROR("Scale0 input Act Err\n"); 
		goto out; 
	}

	//scale0 set output
	ret = gpHalScale0SetOutputAddr(Scale0.output_addr);
	if(ret < 0) {
		DERROR("Scale0 output Address must be 4 Align\n"); 
		goto out; 
	}

	ret = gpHalScale0SetOutputDst(Scale0.dst_width, Scale0.dst_height);
	if(ret < 0) {
		DERROR("Scale0 input Dst Err\n");  
		goto out; 
	}

	ret = gpHalScale0SetOutput(Scale0.output_format, 
								Scale0.output_buf_width, 
								Scale0.output_buf_height, 
								Scale0.dst_x_offset, 
								Scale0.dst_y_offset);
	if(ret < 0) {
		DERROR("Scale0 output bpl must be 4 Align\n"); 
		goto out; 
	}

	gpHalScale0SetStretch(Scale0.stretch_en);
	gpHalScale0SetChormaThr(Scale0.YMax, Scale0.YMin, Scale0.CbCrMax, Scale0.CbCrMin);
	gpHalScale0SetBoundaryColor(Scale0.BoundaryColor);

	//HW limit
	if(Scale0.input_format == C_SCALE0_IN_YCbCr444_SEMI_PLANAR) {
		ret = ((Scale0.act_width + Scale0.act_x_offset) * 1) % 4;
		if(ret == 2) {
			DERROR("Can't let ((act_w+act_x)*1)/4 remain 2\n"); 
			ret = -1;
			goto out; 	
		}
	}
	
	if((Scale0.input_format == C_SCALE0_IN_YCbCr422_PLANAR) || 
		(Scale0.input_format == C_SCALE0_IN_YCbCr420_PLANAR)) {
		ret = Scale0.act_x_offset % 8;
		switch(ret)
		{
			case 0:
			case 1:
			case 2:
				break;

			default:
				DERROR("Must let act_x/8 remain 0, 1 or 2\n"); 
				ret = -1;
				goto out; 	
		}
	}
	
	if(ctx->timeout == 0) {
		ctx->timeout = SCALE_TIMEOUT;
	}

	DEBUG("Scale0Start.\n");
#if POLLING_TEST == 1
	if(gpHalScale0Start(0) < 0) {
		DERROR("Scale0Start Err\n");  
		ret = -1;
		goto out; 
	}

	/* waiting for done */
	DEBUG("Waiting for Scaling Done\n");
	if(HAL_BUSY_WAITING(gpHalScale0GetStatus(), ctx->timeout) >= 0) {
		DEBUG("Scale0 Done\n");
	} else {
		DERROR("Scaler0 Timeout (polling %dms) !!!!!!!!!!!!\n", ctx->timeout);
		ret = -1;
		goto out; 	
	}
#else
	if(gpHalScale0Start(1) < 0) {
		DERROR("Scale0Start Err\n");  
		ret = -1;
		goto out; 
	}

	/* waiting for done */
	DEBUG("Waiting for Scaling Done\n");
	if(wait_event_timeout(scale->done_wait, scale->done, (ctx->timeout * HZ) / 1000) == 0) {
		DERROR("Scaler0 Timeout (polling %dms) !!!!!!!!!!!!\n", ctx->timeout);
		ret = -1;
		goto out; 	
	}
	DEBUG("Scale0 Done\n");
#endif

	/* invalidate dcache */
#ifndef GP_SYNC_OPTION
	bpl_y = ctx->dst_img.bpl;
	addr = (unsigned int)ctx->dst_img.pData + bpl_y * Scale0.dst_y_offset;
	ret = bpl_y * Scale0.dst_height;
	DEBUG("Invalid = 0x%x, Size = 0x%x\n", addr, ret);
	gp_invalidate_dcache_range(addr, ret);
#else
	GP_SYNC_CACHE();
#endif

out:
#if DEBUG_DUMP == 0
	if(ret < 0) {
		gp_scale_param_dump(Scale0);
		gpHalScaleRegDump();
	}
#else
	gp_scale_param_dump(Scale0);
	gpHalScaleRegDump();
#endif	
	up(&scale->sem);
	return ret;
}
EXPORT_SYMBOL(scale_trigger);

static irqreturn_t 
scale_irq_handler(
	int irq, 
	void *dev_id
)
{
	if(gpHalScale0GetIRQStatus()) {
		DEBUG("Scaling Done\n");
		scale->done = true;
		wake_up(&scale->done_wait);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

/**
 * @brief   Scale device open function
 */
static int 
scale_open(
	struct inode *ip, 
	struct file *fp
)
{
	DOWN(&scale->sem);

	if (scale->open_count == 0) {
		gp_scalar_module_clk_en(1);
		gpHalScale0ModuleRest(1);
	}
	scale->open_count++;

	up(&scale->sem);
	return 0;
}

/**
 * @brief   Scale device release function
 */
static int 
scale_release(
	struct inode *ip, 
	struct file* fp
)
{
	DOWN(&scale->sem);

	scale->open_count--;
	if (scale->open_count == 0) {
		gp_scalar_module_clk_en(0);
	}
	
	if (fp->private_data) {
		kfree(fp->private_data);
	}

	up(&scale->sem);
	return 0;
}

/**
 * @brief   Scale device ioctl function
 */
static long 
scale_ioctl(
	struct file* file, 
	unsigned int cmd, 
	unsigned long arg
)
{
	long ret = 0;
	scale_content_t ctx;

	switch(cmd) 
	{
		case SCALE_IOCTL_TRIGGER:
			DEBUG("SCALE_IOCTL_TRIGGER\n");
			if (copy_from_user(&ctx, (void __user*)arg, sizeof(ctx))) {
				ret = -EFAULT;
				break;
			}
			
			ret = scale_trigger(&ctx);
			break;

		case SCALE_IOCTL_DITHER:
			DEBUG("SCALE_IOCTL_DITHER\n");
			break;

		default:
			DEBUG("SCALE_IOCTL ERR\n");
			ret = -ENOTTY; /* Inappropriate ioctl for device */
			break;
	}

	return ret;
}

static const struct file_operations scale_fops = {
	.owner          = THIS_MODULE,
	.open           = scale_open,
	.release        = scale_release,
	.unlocked_ioctl = scale_ioctl,
};


#ifdef CONFIG_PM
static void gp_scalar_suspend_set(void)
{
	if (down_killable(&scale->sem) != 0) {
		return;
	}
	gp_scalar_module_clk_en(0);
	up(&scale->sem);
}

static void gp_scalar_resume_set(void)
{
	gp_scalar_module_clk_en(1);
}

static int gp_scalar_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (scale->open_count > 0) {
		gp_scalar_suspend_set();
	}
	return 0;
}

static int gp_scalar_resume(struct platform_device *pdev)
{
	if (scale->open_count > 0) {
		gp_scalar_resume_set();
	}
	return 0;
}
#else
#define gp_scalar_suspend NULL
#define gp_scalar_resume NULL
#endif

/**
 * @brief   scalar device release
 */
static void gp_scalar_device_release(struct device *dev)
{
	DIAG_INFO("remove scalar device ok\n");
}

static struct platform_device gp_scalar_device = {
	.name	= "gp-scalar",
	.id	= -1,
	.dev	= {
		.release = gp_scalar_device_release,
	}
};

static struct platform_driver gp_scalar_driver = {
	.driver		= {
		.name	= "gp-scalar",
		.owner	= THIS_MODULE,
	},
	.suspend	= gp_scalar_suspend,
	.resume		= gp_scalar_resume,
};

/**
 * @brief   Scale driver init function
 */
static int __init scale_init(void)
{
	int ret = -ENXIO;

	DEBUG(KERN_WARNING "ModuleInit: scale0 \n");
	scale = (scale_info_t *)kzalloc(sizeof(scale_info_t), GFP_KERNEL);
	if (scale == NULL) {
		ret = -ENOMEM;
		DERROR("scale0 kmalloc fail\n");
		goto fail_kmalloc;
	}

	ret = request_irq(IRQ_SCALE_ENGINE_0,
					  scale_irq_handler,
					  IRQF_SHARED,
					  "SCALE_IRQ",
					  scale);
	if (ret < 0) {
		DERROR("scale0 request irq fail\n");
		goto fail_request_irq;
	}

	/* initialize */
	init_MUTEX(&scale->sem);
	init_waitqueue_head(&scale->done_wait);

	scale->dev.name  = "scalar";
	scale->dev.minor = MISC_DYNAMIC_MINOR;
	scale->dev.fops  = &scale_fops;

	/* register device */
	ret = misc_register(&scale->dev);
	if (ret != 0) {
		DERROR("scale0 device register fail\n");
		goto fail_device_register;
	}

	platform_device_register(&gp_scalar_device);
	platform_driver_register(&gp_scalar_driver);

	return 0;

	/* error rollback */
fail_device_register:
	free_irq(IRQ_SCALE_ENGINE_0, scale);
fail_request_irq:
	kfree(scale);
	scale = NULL;
fail_kmalloc:
	return ret;
}

/**
 * @brief   Scale driver exit function
 */
static void __exit scale_exit(void)
{
	misc_deregister(&scale->dev);
	free_irq(IRQ_SCALE_ENGINE_0, scale);
	kfree(scale);
	scale = NULL;

	platform_device_unregister(&gp_scalar_device);
	platform_driver_unregister(&gp_scalar_driver);
}

module_init(scale_init);
module_exit(scale_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus Scale0 Driver");
MODULE_LICENSE_GP;
MODULE_VERSION("1.0");
