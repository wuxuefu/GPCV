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
#include <mach/hal/hal_scale2.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_cache.h>
#include <mach/gp_scale2.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/hal/hal_clock.h>
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define DERROR	printk
#if 0
#define DEBUG	printk
#else
#define DEBUG(...)
#endif

#define FORCE_INTP_EN	1
/* Scale2 timeout (ms) */
#define SCALE2_TIMEOUT	3000

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define RETURN(x, msg)\
{\
	nRet = x;\
	DERROR(msg);\
	goto __return;\
}\

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gpScale2Dev_s
{
	struct miscdevice dev;          /*!< @brief scale device */
	struct semaphore sem;           /*!< @brief mutex semaphore for scale ops */
	struct semaphore scale_sem;     /*!< @brief mutex semaphore for scale h/w busy ops */
	wait_queue_head_t wait_queue;   /*!< @brief scaling done wait queue */
	bool done;                      /*!< @brief scaling done flag */

	unsigned short open_cnt;
	unsigned short scale_cnt;
	unsigned int scale_status;
	unsigned int ext_buf_addr;
} gpScale2Dev_t;

typedef struct gpScale2Data_s
{
	gpScale2Format_t format;
	gpScale2Para_t	para;
}gpScale2Data_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gpScale2Dev_t *pScale2Dev;

static void
gp_scalar2_module_clk_en (
	int en
)
{
#ifdef CONFIG_PM
	if(en) {
		gpHalScuClkEnable(SCU_A_PERI_SCA, SCU_A, 1);
		//gpHalScuClkEnable(SCU_C_PERI_SCALING, SCU_C, 1);
		//gpHalScuClkEnable(SCU_C_PERI_2DSCALEABT, SCU_C, 1);
		gp_enable_clock((int*)"2DSCAABT", 1);
		gpHalScale2ModuleRest(1);
	} else {
		gpHalScuClkEnable(SCU_A_PERI_SCA, SCU_A, 0);
		//gpHalScuClkEnable(SCU_C_PERI_SCALING, SCU_C, 0);
		//gpHalScuClkEnable(SCU_C_PERI_2DSCALEABT, SCU_C, 0);
		gp_enable_clock((int*)"2DSCAABT", 0);
	}
#else
	if(en) {
		gpHalScuClkEnable( SCU_A_PERI_SCA, SCU_A, 1);
		//gpHalScuClkEnable( SCU_C_PERI_SCALING, SCU_C, 1);
		gpHalScale2ModuleRest(1);
	} else {
		//gpHalScuClkEnable( SCU_C_PERI_SCALING, SCU_C, 0);
		gpHalScuClkEnable( SCU_A_PERI_SCA , SCU_A, 0);
	}
#endif
}

static void
gp_scale2_init_para(
	gpScale2Data_t *argp
)
{
	memset((void *)argp, 0, sizeof(gpScale2Data_t));
	argp->para.boundary_mode = 1;
	argp->para.boundary_color = 0x008080;
	argp->para.gamma_en = 0;
	argp->para.color_matrix_en = 0;
	argp->para.yuv_type = C_SCALE2_CTRL_TYPE_YCBCR;

	argp->format.input_format = C_SCALE2_CTRL_IN_YUYV;
	argp->format.output_format = C_SCALE2_CTRL_OUT_YUYV;
	argp->format.fifo_mode = C_SCALE2_CTRL_FIFO_DISABLE;
	argp->format.scale_mode = C_SCALE2_FULL_SCREEN;
	argp->format.digizoom_m = 10;
	argp->format.digizoom_n = 10;
}

static void
gp_scale2_free_ext_buffer(
	void
)
{
	if(pScale2Dev->ext_buf_addr) {
		gp_chunk_free((void *)pScale2Dev->ext_buf_addr);
		pScale2Dev->ext_buf_addr = 0;
	}
}

static int
gp_scale2_alloc_ext_buffer(
	int size
)
{
	int nRet;
	unsigned int phy_addr;

	gp_scale2_free_ext_buffer();
	pScale2Dev->ext_buf_addr = (unsigned int)gp_chunk_malloc(size);
	if(pScale2Dev->ext_buf_addr == 0) {
		gpHalScale2SetLineBufferMode(C_SCALE2_INTERNAL_LINE_BUFFER);
		RETURN(-1, "scale2 alloc ext buf fail!\n");
	}

	nRet = 0;
	phy_addr = (unsigned int)gp_chunk_pa((void *)pScale2Dev->ext_buf_addr);
	DEBUG("phy_addr = 0x%x\n", phy_addr);
	gpHalScale2SetLineBufferMode(C_SCALE2_EXTERNAL_LINE_BUFFER);
	gpHalScale2SetExternalLineBuffer(phy_addr);
__return:
	return nRet;
}

static void
gp_scale2_set_para(
	gpScale2Para_t *argp
)
{
	gpHalScale2SetBoundaryMode(argp->boundary_mode);
	gpHalScale2SetBoundaryColor(argp->boundary_color);
	gpHalScale2SetYuvType(argp->yuv_type);
	if(argp->gamma_en) {
		int i;

		gpHalScale2SetGammaSwitch(ENABLE);
		for(i=0; i<256; i++) {
			gpHalScale2SetGamma(i, argp->gamma_table[i]);
		}
	} else {
		gpHalScale2SetGammaSwitch(DISABLE);
	}

	if(argp->color_matrix_en) {
		gpHalScale2SetColorMatrixSwitch(ENABLE);
		gpHalScale2SetColorMatrix(1, argp->A11, argp->A12, argp->A13);
		gpHalScale2SetColorMatrix(2, argp->A21, argp->A22, argp->A23);
		gpHalScale2SetColorMatrix(3, argp->A31, argp->A32, argp->A33);
	} else {
		gpHalScale2SetColorMatrixSwitch(DISABLE);
	}
}

#ifndef GP_SYNC_OPTION
static int
gp_scale2_get_bpl(
	unsigned int format,
	unsigned int width,
	unsigned int *bpl_y,
	unsigned int *bpl_u,
	unsigned int *bpl_v
)
{
	int nRet = 0;

	*bpl_u = 0;
	*bpl_v = 0;

	switch (format) 
	{
	case C_SCALE2_CTRL_IN_RGB1555:
	case C_SCALE2_CTRL_IN_RGB565:
	case C_SCALE2_CTRL_IN_RGBG:
	case C_SCALE2_CTRL_IN_GRGB:
	case C_SCALE2_CTRL_IN_YUYV:
	case C_SCALE2_CTRL_IN_UYVY:
	case C_SCALE2_CTRL_IN_VYUY:
	case C_SCALE2_CTRL_OUT_RGB565:
	case C_SCALE2_CTRL_OUT_RGBG:
	case C_SCALE2_CTRL_OUT_GRGB:
	case C_SCALE2_CTRL_OUT_YUYV:
	case C_SCALE2_CTRL_OUT_UYVY:
	case C_SCALE2_CTRL_OUT_VYUY:
		*bpl_y = width << 1;
		break;

	case C_SCALE2_CTRL_IN_YUYV8X32:
	case C_SCALE2_CTRL_IN_YUYV8X64:
	case C_SCALE2_CTRL_IN_YUYV16X32:
	case C_SCALE2_CTRL_IN_YUYV16X64:
	case C_SCALE2_CTRL_IN_YUYV32X32:
	case C_SCALE2_CTRL_IN_YUYV64X64:
	case C_SCALE2_CTRL_IN_VYUY8X32:
	case C_SCALE2_CTRL_IN_VYUY8X64:
	case C_SCALE2_CTRL_IN_VYUY16X32:
	case C_SCALE2_CTRL_IN_VYUY16X64:
	case C_SCALE2_CTRL_IN_VYUY32X32:
	case C_SCALE2_CTRL_IN_VYUY64X64:
	case C_SCALE2_CTRL_OUT_YUYV8X32:
	case C_SCALE2_CTRL_OUT_YUYV8X64:
	case C_SCALE2_CTRL_OUT_YUYV16X32:
	case C_SCALE2_CTRL_OUT_YUYV16X64:
	case C_SCALE2_CTRL_OUT_YUYV32X32:
	case C_SCALE2_CTRL_OUT_YUYV64X64:
	case C_SCALE2_CTRL_OUT_VYUY8X32:
	case C_SCALE2_CTRL_OUT_VYUY8X64:
	case C_SCALE2_CTRL_OUT_VYUY16X32:
	case C_SCALE2_CTRL_OUT_VYUY16X64:
	case C_SCALE2_CTRL_OUT_VYUY32X32:
	case C_SCALE2_CTRL_OUT_VYUY64X64:
		*bpl_y = width << 1;
		break;

	case C_SCALE2_CTRL_IN_ARGB4444:
	case C_SCALE2_CTRL_OUT_ARGB4444:
		*bpl_y = width << 1;
		break;

	case C_SCALE2_CTRL_IN_ARGB8888:
	case C_SCALE2_CTRL_OUT_ARGB8888:
		*bpl_y = width << 2;
		break;

	case C_SCALE2_CTRL_IN_YUV422:
	case C_SCALE2_CTRL_OUT_YUV422:
		*bpl_y = width;
		*bpl_u = width >> 1;
		*bpl_v = width >> 1;
		break;

	case C_SCALE2_CTRL_IN_YUV420:
	case C_SCALE2_CTRL_OUT_YUV420:
		*bpl_y = width;
		*bpl_u = width >> 2;
		*bpl_v = width >> 2;
		break;

	case C_SCALE2_CTRL_IN_YUV411:
	case C_SCALE2_CTRL_IN_YUV411V:
	case C_SCALE2_CTRL_OUT_YUV411:
		*bpl_y = width;
		*bpl_u = width >> 2;
		*bpl_v = width >> 2;
		break;

	case C_SCALE2_CTRL_IN_YUV444:
	case C_SCALE2_CTRL_OUT_YUV444:
		*bpl_y = width;
		*bpl_u = width;
		*bpl_v = width;
		break;

	case C_SCALE2_CTRL_IN_Y_ONLY:
	case C_SCALE2_CTRL_OUT_Y_ONLY:
		*bpl_y = width;
		break;

	default:
		nRet = -1;
		break;
	}

	return nRet;
}
#endif

static int
gp_scale2_clean_dcache(
	gpScale2Format_t *argp
)
{
#ifndef GP_SYNC_OPTION
	unsigned int bpl_y, bpl_u, bpl_v, h;
	
	if(argp->fifo_mode == C_SCALE2_CTRL_IN_FIFO_DISABLE) {
		h = argp->input_height;
	} else if(argp->fifo_mode == C_SCALE2_CTRL_IN_FIFO_16LINE) {
		h = 16;
	} else if(argp->fifo_mode == C_SCALE2_CTRL_IN_FIFO_32LINE) {
		h = 32;
	} else if(argp->fifo_mode == C_SCALE2_CTRL_IN_FIFO_64LINE) {
		h = 64;
	} else if(argp->fifo_mode == C_SCALE2_CTRL_IN_FIFO_128LINE) {
		h = 128;
	} else if(argp->fifo_mode == C_SCALE2_CTRL_IN_FIFO_256LINE) {
		h = 256;
	} else {
		h = 0;
	}

	DEBUG("Scale2InvalidDcache\n");
	gp_scale2_get_bpl(argp->input_format, argp->input_width, &bpl_y, &bpl_u, &bpl_v);
	if(bpl_y) {
		gp_clean_dcache_range(argp->input_y_addr, h * bpl_y);
	}

	if(bpl_u) {		gp_clean_dcache_range(argp->input_u_addr, h * bpl_u);
	}

	if(bpl_v) {
		gp_clean_dcache_range(argp->input_v_addr, h * bpl_v);
	}
#else
	GP_SYNC_CACHE();
#endif
	return 0;
}

static int
gp_scale2_invalid_dcache(
	void
)
{
#ifndef GP_SYNC_OPTION
	unsigned int bpl_y, bpl_u, bpl_v, h;
	unsigned int OutFifoMode, OutFmt;
	unsigned int width, height;
	unsigned int OutYAddr, OutUAddr, OutVAddr;

	OutFifoMode = gpHalScale2GetOutputFifoSize();
	OutFmt = gpHalScale2GetOutputFifoSize();
	gpHalScale2GetOutputPixel(&width, &height);
	if(OutFifoMode == C_SCALE2_CTRL_OUT_FIFO_DISABLE) {
		h = width;
	} else if(OutFifoMode == C_SCALE2_CTRL_OUT_FIFO_16LINE) {
		h = 16;
	} else if(OutFifoMode == C_SCALE2_CTRL_OUT_FIFO_32LINE) {
		h = 32;
	} else if(OutFifoMode == C_SCALE2_CTRL_OUT_FIFO_64LINE) {
		h = 64;
	} else {
		h = 0;
	}

	DEBUG("Scale2InvalidDcache\n");
	gp_scale2_get_bpl(OutFmt, width, &bpl_y, &bpl_u, &bpl_v);
	gpHalScale2GetOutputAddr(&OutYAddr, &OutUAddr, &OutVAddr);
	if(bpl_y) {
		OutYAddr = (unsigned int)gp_chunk_va(OutYAddr);
		gp_invalidate_dcache_range(OutYAddr, h * bpl_y);
	}

	if(bpl_u) {
		OutUAddr = (unsigned int)gp_chunk_va(OutUAddr);
		gp_invalidate_dcache_range(OutUAddr, h * bpl_u);
	}

	if(bpl_v) {
		OutVAddr = (unsigned int)gp_chunk_va(OutVAddr);
		gp_invalidate_dcache_range(OutVAddr, h * bpl_v);
	}
#else
	GP_SYNC_CACHE();
#endif
	return 0;
}

static int 
gp_scale2_wait_done(
	void
)
{
	DEBUG("Scale2WaitDone.\n");
#if !POLLING_TEST
	if(wait_event_timeout(pScale2Dev->wait_queue, pScale2Dev->done, SCALE2_TIMEOUT * HZ / 1000) == 0) {
#else
	if (HAL_BUSY_WAITING(gpHalScale2PollStatus() != C_SCALE2_STATUS_BUSY, SCALE2_TIMEOUT) >= 0) {
		status = gpHalScale2PollStatus();
		DEBUG("gpHalScale2PollStatus: %d\n", status);
		if(status & C_SCALE2_STATUS_DONE) {
			DEBUG("Scale2Done\n");
			pScale2Dev->scale_status = C_SCALE2_STATUS_DONE;
			pScale2Dev->done = 1;
			up(&pScale2Dev->scale_sem);
		} else if(status & C_SCALE2_STATUS_INPUT_EMPTY) {
			DEBUG("Scale2Empty\n");
			pScale2Dev->scale_status = C_SCALE2_STATUS_INPUT_EMPTY;
			pScale2Dev->done = 1;
			up(&pScale2Dev->scale_sem);
		} else if(status & C_SCALE2_STATUS_OUTPUT_FULL) {
			DEBUG("Scale2Full\n");
			pScale2Dev->scale_status = C_SCALE2_STATUS_OUTPUT_FULL;
			pScale2Dev->done = 1;
			up(&pScale2Dev->scale_sem);
		}
	} else {
#endif
		DERROR("Scale2TimeOut!\n");
		pScale2Dev->scale_status = C_SCALE2_STATUS_TIMEOUT;
		gpHalScale2Stop();
		gp_scale2_free_ext_buffer();
	}

	/* scale2 finish */
	if(pScale2Dev->scale_status == C_SCALE2_STATUS_DONE) {
		DEBUG("Scale2Stop.\n");
		pScale2Dev->scale_status = C_SCALE2_STATUS_STOP;
		gpHalScale2Stop();
		gp_scale2_free_ext_buffer();
	}	

	return pScale2Dev->scale_status;
}

static int
gp_scale2_stop(
	void
)
{
	int nRet = 0;
	
	DEBUG("Scale2Status = 0x%x\n", pScale2Dev->scale_status);
	if(pScale2Dev->scale_status == C_SCALE2_STATUS_DONE ||
		pScale2Dev->scale_status == C_SCALE2_STATUS_BUSY) {
		/* waiting for done */
		if(gp_scale2_wait_done() == C_SCALE2_STATUS_TIMEOUT) {
			nRet = C_SCALE2_START_ERR;
		} else {
			/* clean dcache */
			gp_scale2_invalid_dcache(); 
		}
	} 
		
	pScale2Dev->scale_status = C_SCALE2_STATUS_STOP;
	gpHalScale2Stop();
	gp_scale2_free_ext_buffer();
	return nRet;
}

static int
gp_scale2_trigger(
	unsigned int WaitDoneFlag,
	gpScale2Format_t *argp,
	gpScale2Para_t *argpara
)
{
	unsigned char mode = 0;
	int status, nRet;
	unsigned int y_addr, u_addr, v_addr;
	unsigned int tempx, tempy;

	if(argp->input_visible_width > argp->input_width) {
		argp->input_visible_width = 0;
	}
	
	if(argp->input_visible_height > argp->input_height) {
		argp->input_visible_height = 0;
	}
	
	if(argp->output_width > argp->output_buf_width) {
		argp->output_width = argp->output_buf_width;
	}
	
	if(argp->output_height > argp->output_buf_height) {
		argp->output_height = argp->output_buf_height;
	}
	
	/* start scaling */
	gpHalScale2SetInit();
	switch(argp->scale_mode)
	{
	case C_SCALE2_FULL_SCREEN:
		nRet = gpHalScale2SetImgPixel(argp->input_width,
									argp->input_height,
									argp->output_buf_width,
									argp->output_buf_height);
		if(nRet < 0) { 
			RETURN(C_SCALE2_START_ERR, "SCALE2_START_ERR\n");
		}
	#if FORCE_INTP_EN == 1
		tempx = 0x8000;
		tempy = 0x8000;
		nRet = gpHalScale2SetInputOffset(tempx, tempy);
	#endif
		break;

	case C_SCALE2_BY_RATIO:
		mode = 1;
	case C_SCALE2_FULL_SCREEN_BY_RATIO:
		nRet = gpHalScale2SetInputPixel(argp->input_width, argp->input_height);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_SIZE_ERR, "SCALE2_INPUT_SIZE_ERR\n");
		}
		
		nRet = gpHalScale2SetInputVisiblePixel(argp->input_visible_width, 
												argp->input_visible_height);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_SIZE_ERR, "SCALE2_INPUT_VISIBLE_SIZE_ERR\n");
		}
		
		if(argp->input_visible_width) {
			tempx = argp->input_visible_width;
		} else {
			tempx = argp->input_width;
		}
		
		if(argp->input_visible_height) {
			tempy = argp->input_visible_height;
		} else {
			tempy = argp->input_height;
		}
		
		if(argp->input_x_offset > tempx) {
			argp->input_x_offset = 0;
		}
		
		if(argp->input_y_offset > tempy) {
			argp->input_y_offset = 0;
		}
		
		if(argp->input_x_offset) {
			tempx -= argp->input_x_offset;
		}
		
		if(argp->input_y_offset) {
			tempy -= argp->input_y_offset;
		}
		
		if(mode) {
			/* scale by ratio */
			tempx = (tempx << 16) / argp->output_width;
			tempy = (tempy << 16) / argp->output_height;
		} else {
			/* scale full screen by ratio */
			if (argp->output_buf_height*tempx > argp->output_buf_width*tempy) {
		      	tempx = tempy = (tempx << 16) / argp->output_buf_width;
		      	argp->output_height = (argp->output_buf_height << 16) / tempx;
		    } else {
		      	tempx = tempy = (tempy << 16) / argp->output_buf_height;
		      	argp->output_width = (argp->output_buf_width << 16) / tempy;
		    }
		}

		nRet = gpHalscale2SetOutputPixel(tempx, tempy, argp->output_buf_width, argp->output_buf_height);
		if(nRet < 0) {
			RETURN(C_SCALE2_OUTPUT_SIZE_ERR, "SCALE2_OUTPUT_SIZE_ERR\n");
		}
		
		tempx = argp->input_x_offset << 16;
		tempy = argp->input_y_offset << 16;
	#if FORCE_INTP_EN == 1
		tempx += 0x8000;
		tempy += 0x8000;
	#endif
		nRet = gpHalScale2SetInputOffset(tempx, tempy);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_OFFSET_ERR, "SCALE2_INPUT_OFFSET_ERR\n");
		}
		break;

	case C_SCALE2_FULL_SCREEN_BY_DIGI_ZOOM:
		nRet = gpHalScale2SetInputPixel(argp->input_width, argp->input_height);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_SIZE_ERR, "SCALE2_INPUT_SIZE_ERR\n");
		}
		
		nRet = gpHalScale2SetInputVisiblePixel(argp->input_visible_width, 
												argp->input_visible_height);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_SIZE_ERR, "SCALE2_INPUT_VISIBLE_SIZE_ERR\n");
		}
		
		/* mutiple * 100 */
		if(argp->digizoom_n == 0) {
			argp->digizoom_n = 10;
		}

		if(argp->digizoom_m == 0) {
			argp->digizoom_m = 10;
		}
		tempx = 100 * (argp->output_width * argp->digizoom_m) / (argp->input_width * argp->digizoom_n);
		tempx = 65536 * 100 / tempx;
		nRet = gpHalscale2SetOutputPixel(tempx, tempx, argp->output_buf_width, argp->output_buf_height);
		if(nRet < 0) {
			RETURN(C_SCALE2_OUTPUT_SIZE_ERR, "SCALE2_OUTPUT_SIZE_ERR\n");
		}
		
		tempx = argp->output_width * (abs(argp->digizoom_m - argp->digizoom_n));
		tempx >>= 1;
		tempx =	(tempx << 16) / argp->digizoom_n;
		tempy = argp->output_height * (abs(argp->digizoom_m - argp->digizoom_n));
		tempy >>= 1;
		tempy = (tempy << 16) / argp->digizoom_n;
		nRet = gpHalScale2SetInputOffset(tempx, tempy);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_OFFSET_ERR, "SCALE2_INPUT_OFFSET_ERR\n");
		}
		break;

	default:
		RETURN(C_SCALE2_START_ERR, "scale2 mode fail!\n");
	}

	/* alloc ext buffer for line buffer use*/
	if(argp->output_buf_width >= 1024) {
		nRet = gp_scale2_alloc_ext_buffer(argp->output_buf_width*4*2);
		if(nRet < 0) {
			RETURN(C_SCALE2_EXT_BUF_ERR, "SCALE2_EXT_BUF_ERR\n");
		}
	}
	
	nRet = gpHalscale2SetOutputOffset(argp->output_x_offset);
	if(nRet < 0) {
		RETURN(C_SCALE2_OUTPUT_OFFSET_ERR, "SCALE2_OUTPUT_OFFSET_ERR\n");
	}

	/* get y,u,v input physcial address */ 
	y_addr = u_addr = v_addr = 0; 
	if(argp->input_y_addr) {
		y_addr = gp_user_va_to_pa((void *)argp->input_y_addr);
	}
	
	if(argp->input_u_addr) {
		u_addr = gp_user_va_to_pa((void *)argp->input_u_addr);
	}
	
	if(argp->input_v_addr) {
		v_addr = gp_user_va_to_pa((void *)argp->input_v_addr);
	}

	nRet = gpHalScale2SetInputAddr(y_addr, u_addr, v_addr);
	if(nRet < 0) {
		RETURN(C_SCALE2_INPUT_BUF_ERR, "SCALE2_INPUT_BUF_ERR\n");
	}

	/* get y,u,v output physcial address */
	y_addr = u_addr = v_addr = 0; 
	if(argp->output_y_addr) {
		y_addr = gp_user_va_to_pa((void *)argp->output_y_addr);
	} 
	
	if(argp->output_u_addr) {
		u_addr = gp_user_va_to_pa((void *)argp->output_u_addr);
	}
	
	if(argp->output_v_addr) {
		v_addr = gp_user_va_to_pa((void *)argp->output_v_addr);
	}

	nRet = gpHalScale2SetOutputAddr(y_addr, u_addr, v_addr);
	if(nRet < 0) {
		RETURN(C_SCALE2_OUTPUT_BUF_ERR, "SCALE2_OUTPUT_BUF_ERR\n");
	}
#if 0
	if((argp->input_format == C_SCALE2_CTRL_IN_VYUY) ||
		(argp->input_format == C_SCALE2_CTRL_IN_VYUY) ||
		(argp->input_format == C_SCALE2_CTRL_IN_VYUY) ||
		(argp->input_format == C_SCALE2_CTRL_IN_VYUY))
	{
		if((argp->output_format == C_SCALE2_CTRL_OUT_UYVY) ||
			(argp->output_format == C_SCALE2_CTRL_OUT_UYVY) ||
			(argp->output_format == C_SCALE2_CTRL_OUT_UYVY) ||
			(argp->output_format == C_SCALE2_CTRL_OUT_UYVY))
		{
			RETURN(-1, "OutFmtMustBe VYUY/VYUYnx32/VYUYnx64\n");
		}
	}
#endif
	nRet = gpHalScale2SetInputFormat(argp->input_format);
	if(nRet < 0) {
		RETURN(C_SCALE2_INPUT_FMT_ERR, "SCALE2_INPUT_FMT_ERR\n");
	}
	
	nRet = gpHalScale2SetOutputFormat(argp->output_format);
	if(nRet < 0) {
		RETURN(C_SCALE2_OUTPUT_FMT_ERR, "SCALE2_OUTPUT_FMT_ERR\n");
	}
	
	switch(argp->fifo_mode)
	{
	case C_SCALE2_CTRL_FIFO_DISABLE:
		nRet = gpHalScale2SetInputFifoSize(C_SCALE2_CTRL_IN_FIFO_DISABLE);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_FIFO_ERR, "SCALE2_INPUT_FIFO_ERR\n");
		}
		
		nRet = gpHalScale2SetOutputFifoSize(C_SCALE2_CTRL_OUT_FIFO_DISABLE);
		if(nRet < 0) {
			RETURN(C_SCALE2_OUTPUT_FIFO_ERR, "SCALE2_OUTPUT_FIFO_ERR\n");
		}
		break;

	case C_SCALE2_CTRL_IN_FIFO_16LINE:
	case C_SCALE2_CTRL_IN_FIFO_32LINE:
	case C_SCALE2_CTRL_IN_FIFO_64LINE:
	case C_SCALE2_CTRL_IN_FIFO_128LINE:
	case C_SCALE2_CTRL_IN_FIFO_256LINE:
		nRet = gpHalScale2SetInputFifoSize(argp->fifo_mode);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_FIFO_ERR, "SCALE2_INPUT_FIFO_ERR\n");
		}
		
		nRet = gpHalScale2SetOutputFifoSize(C_SCALE2_CTRL_OUT_FIFO_DISABLE);
		if(nRet < 0) {
			RETURN(C_SCALE2_OUTPUT_FIFO_ERR, "SCALE2_OUTPUT_FIFO_ERR\n");
		}
		break;

	case C_SCALE2_CTRL_OUT_FIFO_16LINE:
	case C_SCALE2_CTRL_OUT_FIFO_32LINE:
	case C_SCALE2_CTRL_OUT_FIFO_64LINE:
		nRet = gpHalScale2SetInputFifoSize(C_SCALE2_CTRL_IN_FIFO_DISABLE);
		if(nRet < 0) {
			RETURN(C_SCALE2_INPUT_FIFO_ERR, "SCALE2_INPUT_FIFO_ERR\n");
		}

		nRet = gpHalScale2SetOutputFifoSize(argp->fifo_mode);
		if(nRet < 0) {
			RETURN(C_SCALE2_OUTPUT_FIFO_ERR, "SCALE2_OUTPUT_FIFO_ERR\n");
		}
		break;
	}

	/* set scale other parameters */
	gp_scale2_set_para(argpara);
	status = gpHalScale2PollStatus();
	if(status != C_SCALE2_STATUS_STOP) {
		RETURN(C_SCALE2_OUTPUT_FIFO_ERR, "SCALE2_STATUS != STOP\n");
	}

	/* clean dcache */
	gp_scale2_clean_dcache(argp);

	/* scale2 start */
	pScale2Dev->done = 0;
	nRet = gpHalScale2Start();
	if(nRet < 0) {
		DERROR("Scale2StartFail!\n");
		pScale2Dev->scale_status = C_SCALE2_STATUS_INIT_ERR;
		gpHalScale2Stop();
		gp_scale2_free_ext_buffer();
		RETURN(C_SCALE2_START_ERR, "SCALE2_START_ERR\n");
	} else {
		pScale2Dev->scale_cnt = 1;
		pScale2Dev->scale_status = C_SCALE2_STATUS_BUSY;
	}

#if 0
	/* dump  all register */
	DEBUG("InWH[%d, %d]\n", argp->input_width, argp->input_height);
	DEBUG("InVisWH[%d, %d]\n", argp->input_visible_width, argp->input_visible_height);
	DEBUG("InXYOffset[%x, %x]\n", argp->input_x_offset, argp->input_y_offset);
	DEBUG("outWH[%d, %d]\n", argp->output_width, argp->output_height);
	DEBUG("outBufWH[%d, %d]\n", argp->output_buf_width, argp->output_buf_height);
	DEBUG("format[0x%x, 0x%x]\n", argp->input_format, argp->output_format);
	DEBUG("outXOffset = 0x%x\n", argp->output_x_offset);
	DEBUG("InYAddr = 0x%x\n", argp->input_y_addr);
	DEBUG("InUAddr = 0x%x\n", argp->input_u_addr);
	DEBUG("InVAddr = 0x%x\n", argp->input_v_addr);
	DEBUG("OutYAddr = 0x%x\n", argp->output_y_addr);
	DEBUG("OutUAddr = 0x%x\n", argp->output_u_addr);
	DEBUG("OutVAddr = 0x%x\n", argp->output_v_addr);
	DEBUG("ScaleMode = 0x%x\n", argp->scale_mode);
	gpHalScale2RegDump();
#endif
	/* waiting for done */
	DEBUG("WaitDoneFlag = %d\n", WaitDoneFlag);
	if(WaitDoneFlag) {
		if(gp_scale2_wait_done() == C_SCALE2_STATUS_TIMEOUT) {
			RETURN(C_SCALE2_START_ERR, "SCALE2_TIMEOUT\n");
		}

		/* clean dcache */
		gp_scale2_invalid_dcache();
	} 
	
	nRet = pScale2Dev->scale_status;
__return:
	if(nRet < 0) {
		DERROR("Scale2Fail!\n");
		gpHalScale2RegDump();
	}
	return nRet;
}

static int
gp_scale2_retrigger(
	gpScale2Format_t *argp
)
{
	int nRet;

	/* clean dcache */
	gp_scale2_clean_dcache(argp);

	/* scale restart */
	pScale2Dev->done = 0;
	nRet = gpHalScale2Restart();
	if(nRet < 0) {
		/* already finish */
		DEBUG("Scale2FifoModeFinish\n");
		pScale2Dev->scale_status = C_SCALE2_STATUS_STOP;
		gpHalScale2Stop();
		gp_scale2_free_ext_buffer();

		nRet = pScale2Dev->scale_status;
		goto __return;
	}
	
#if 0
	/* dump  all register */
	DEBUG("InWH[%d, %d]\n", argp->input_width, argp->input_height);
	DEBUG("InVisWH[%d, %d]\n", argp->input_visible_width, argp->input_visible_height);
	DEBUG("InXYOffset[%x, %x]\n", argp->input_x_offset, argp->input_y_offset);
	DEBUG("outWH[%d, %d]\n", argp->output_width, argp->output_height);
	DEBUG("outBufWH[%d, %d]\n", argp->output_buf_width, argp->output_buf_height);
	DEBUG("format[0x%x, 0x%x]\n", argp->input_format, argp->output_format);
	DEBUG("outXOffset = 0x%x\n", argp->output_x_offset);
	DEBUG("InYAddr = 0x%x\n", argp->input_y_addr);
	DEBUG("InUAddr = 0x%x\n", argp->input_u_addr);
	DEBUG("InVAddr = 0x%x\n", argp->input_v_addr);
	DEBUG("OutYAddr = 0x%x\n", argp->output_y_addr);
	DEBUG("OutUAddr = 0x%x\n", argp->output_u_addr);
	DEBUG("OutVAddr = 0x%x\n", argp->output_v_addr);
	DEBUG("ScaleMode = 0x%x\n", argp->scale_mode);
	gpHalScale2RegDump();
#endif	
	/* waiting for done */
	if(gp_scale2_wait_done() == C_SCALE2_STATUS_TIMEOUT) {
		RETURN(C_SCALE2_START_ERR, "SCALE2_TIMEOUT\n");
	}
	
	/* clean dcache */
	gp_scale2_invalid_dcache(); 
	pScale2Dev->scale_cnt++;
	nRet = pScale2Dev->scale_status;
__return:
	if(nRet < 0) {
		gpHalScale2RegDump();
	}
	return nRet;
}

static irqreturn_t
gp_scale2_irq_handler(
	int irq,
	void *dev_id
)
{
	int status;
	gpScale2Dev_t *pdev = (gpScale2Dev_t *)dev_id;

	status = gpHalScale2GetIntFlag();
	if(status == C_SCALE2_STATUS_DONE) {
		DEBUG("Scale2Done\n");
		pdev->scale_status = C_SCALE2_STATUS_DONE;
		pdev->done = 1;
		wake_up(&pdev->wait_queue);
		up(&pdev->scale_sem);
	} else if(status == C_SCALE2_STATUS_INPUT_EMPTY) {
		DEBUG("Scale2Empty\n");
		pdev->scale_status = C_SCALE2_STATUS_INPUT_EMPTY;
		pdev->done = 1;
		wake_up(&pdev->wait_queue);
		up(&pdev->scale_sem);
	} else if(status == C_SCALE2_STATUS_OUTPUT_FULL) {
		DEBUG("Scale2Full\n");
		pdev->scale_status = C_SCALE2_STATUS_OUTPUT_FULL;
		pdev->done = 1;
		wake_up(&pdev->wait_queue);
		up(&pdev->scale_sem);
	} else {
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}

static long
gp_scale2_ioctl(
	struct file *filp,
	unsigned int cmd,
	unsigned long arg
)
{
	int WatDoneFlag = 1;
	long nRet = 0;
	gpScale2Data_t *hd;

	if(down_interruptible(&pScale2Dev->sem) != 0) {
		return -ERESTARTSYS;
	}
	
	hd = (gpScale2Data_t *)filp->private_data;
 	if(hd == NULL) {
		RETURN(-EINVAL, "gp_scale2_ioctl\n");
 	}
	
	switch(cmd)
	{
	case SCALE2_IOCTL_G_PARA:
		nRet = copy_to_user((void __user*)arg, (void *)&hd->para, sizeof(gpScale2Para_t));
		if(nRet < 0) {
			RETURN(-EINVAL, "SCALE2_IOCTL_G_PARA\n");
		}
		break;

	case SCALE2_IOCTL_S_PARA:
		nRet = copy_from_user((void *)&hd->para, (void __user*)arg, sizeof(gpScale2Para_t));
		if(nRet < 0) {
			RETURN(-EINVAL, "SCALE2_IOCTL_S_PARA\n");
		}
		break;

	case SCALE2_IOCTL_S_START_WITHOUT_WAIT:
		DEBUG("Scale2StartWithoutWait\n");
		WatDoneFlag = 0;
	case SCALE2_IOCTL_S_START:
		nRet = copy_from_user((void *)&hd->format, (void __user*)arg, sizeof(gpScale2Format_t));
		if(nRet < 0) {
			RETURN(-EINVAL, "SCALE2_IOCTL_S_START\n");
		}

		/* wait scale2 stop */
		nRet = gp_scale2_stop();
		if(nRet < 0) {
			up(&pScale2Dev->scale_sem);
			DERROR("Scale2StopFail\n");
		}
	
		/* setup scale2 h/w sem */
		if(down_interruptible(&pScale2Dev->scale_sem) != 0) {
			RETURN(-ERESTARTSYS, "Scale2SemFail!\n");
		}

		/* scale2 start and wait done */
		nRet = gp_scale2_trigger(WatDoneFlag, &hd->format, &hd->para);
		if(nRet < 0) {
			up(&pScale2Dev->scale_sem);
			RETURN(nRet, "Scale2TriggerFail\n");
		}
		break;
	
	case SCALE2_IOCTL_S_RESTART:
		if(hd->format.fifo_mode == C_SCALE2_CTRL_FIFO_DISABLE) {
			RETURN(-1, "Scale2RestartFifoModeFail!\n");
		}

		/* check scale2 status is input empty or output full */
		if(pScale2Dev->scale_status != C_SCALE2_STATUS_INPUT_EMPTY && 
			pScale2Dev->scale_status != C_SCALE2_STATUS_OUTPUT_FULL) {
			RETURN(C_SCALE2_START_ERR, "SCALE2_STATUS != INPUT_EMPTY / OUTPUT_FULL!\n");
		} 

		/* setup scale2 h/w sem */
		if(down_interruptible(&pScale2Dev->scale_sem) != 0) {
			RETURN(-ERESTARTSYS, "Scale2SemFail!\n");
		}

		/* scale2 restart with wait done */
		nRet = gp_scale2_retrigger(&hd->format);
		if(nRet < 0) {
			up(&pScale2Dev->scale_sem);
			RETURN(nRet, "Scale2RetriggerFail\n");
		}
		break;

	case SCALE2_IOCTL_S_PAUSE:
		nRet = gp_scale2_alloc_ext_buffer(hd->format.output_buf_width*4*2);
		if(nRet < 0) {
			RETURN(nRet, "Scale2PauseFail\n");
		}
		gpHalScale2Pause();
		break;

	case SCALE2_IOCTL_S_RESUME:
		gpHalScale2Resume();
		break;

	case SCALE2_IOCTL_S_STOP:
		nRet = gp_scale2_stop();
		if(nRet < 0) {
			up(&pScale2Dev->scale_sem);
			RETURN(nRet, "Scale2StopFail\n");
		}
		break;
	
	default:
		RETURN(-ENOTTY, "ENOTTY\n");	/* Inappropriate ioctl for device */
		break;
	}

__return:
	up(&pScale2Dev->sem);
	return nRet;
}

static int
gp_scale2_open(
	struct inode *inode,
	struct file *filp
)
{
	int nRet = 0;
	gpScale2Data_t *hd = NULL;

	if(down_interruptible(&pScale2Dev->sem) != 0) {
		return -ERESTARTSYS;
	}
	
	hd = (gpScale2Data_t *)kzalloc(sizeof(gpScale2Data_t), GFP_KERNEL);
	if(hd == NULL) {
		RETURN(-ENOMEM, "gp_scale2_open\n");
	}
	
	gp_scale2_init_para(hd);
	filp->private_data = (gpScale2Data_t *)hd;
	if(pScale2Dev->open_cnt == 0) {
		gp_scalar2_module_clk_en(1);
	}

	pScale2Dev->open_cnt++;
__return:
	up(&pScale2Dev->sem);
	return nRet;
}

static int
gp_scale2_release(
	struct inode *inode,
	struct file *filp
)
{
	int nRet = 0;
	gpScale2Data_t *hd;

	if(down_interruptible(&pScale2Dev->sem) != 0) {
		return -ERESTARTSYS;
	}
	
	hd = (gpScale2Data_t *)filp->private_data;
 	if(hd == NULL) {
		RETURN(-ENOMEM, "gp_scale2_release\n");
 	}
	
	kfree(hd);
	filp->private_data = NULL;
	pScale2Dev->open_cnt--;
	if(pScale2Dev->open_cnt == 0){
		gp_scale2_stop();
		gp_scalar2_module_clk_en(0);
	}

__return:
	up(&pScale2Dev->sem);
	return nRet;
}

struct file_operations scale2_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = gp_scale2_ioctl,
	.open = gp_scale2_open,
	.release = gp_scale2_release,
};



/**
 * @brief   scalar2 device release
 */
static void gp_scalar2_device_release(struct device *dev)
{
	DIAG_INFO("remove scalar2 device ok\n");
}

static struct platform_device gp_scalar2_device = {
	.name	= "gp-scalar2",
	.id	= 0,
	.dev	= {
		.release = gp_scalar2_device_release,
	},
};

#ifdef CONFIG_PM
static void gp_scalar2_suspend_set( void ){

	if(down_interruptible(&pScale2Dev->sem) != 0) {
		return;
	}
	
	gp_scale2_stop();
	gp_scalar2_module_clk_en(0);
	up(&pScale2Dev->sem);
}

static void gp_scalar2_resume_set( void ){
	gp_scalar2_module_clk_en(1);
}

static int gp_scalar2_suspend(struct platform_device *pdev, pm_message_t state){
	if( pScale2Dev->open_cnt > 0) {
		gp_scalar2_suspend_set();
	}
	return 0;
}

static int gp_scalar2_resume(struct platform_device *pdev){
	if( pScale2Dev->open_cnt > 0) {
		gp_scalar2_resume_set();
	}
	return 0;
}
#else
#define gp_scalar2_suspend NULL
#define gp_scalar2_resume NULL
#endif

/**
 * @brief   wdt driver define
 */
static struct platform_driver gp_scalar2_driver = {
	.suspend = gp_scalar2_suspend,
	.resume = gp_scalar2_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "gp-scalar2"
	}
};

static int __init
scale2_init_module(
	void
)
{
	int nRet;

	DEBUG(KERN_WARNING "ModuleInit: scale2 \n");
	pScale2Dev = (gpScale2Dev_t *)kzalloc(sizeof(gpScale2Dev_t), GFP_KERNEL);
	if(pScale2Dev == NULL) {
		RETURN(-ENOMEM, "scale2 kmalloc fail\n");
	}

	/* reguest irq */
	nRet = request_irq(IRQ_SCALE_ENGINE,
					  gp_scale2_irq_handler,
					  IRQF_SHARED,
					  "SCALE2_IRQ",
					  pScale2Dev);
	if(nRet < 0) {
		RETURN(-ENXIO, "scale2 request irq fail\n");
	}

	/* initialize */
	init_MUTEX(&pScale2Dev->sem);
	init_MUTEX(&pScale2Dev->scale_sem);
	init_waitqueue_head(&pScale2Dev->wait_queue);

	pScale2Dev->dev.name  = "scale2";
	pScale2Dev->dev.minor = MISC_DYNAMIC_MINOR;
	pScale2Dev->dev.fops  = &scale2_fops;

	pScale2Dev->open_cnt = 0;
	pScale2Dev->scale_cnt = 0;
	pScale2Dev->scale_status = C_SCALE2_STATUS_STOP;
	pScale2Dev->ext_buf_addr = 0;

	/* register device */
	nRet = misc_register(&pScale2Dev->dev);
	if(nRet != 0) {
		RETURN(-ENXIO, "scale2 device register fail\n");
	}

	nRet = 0;
	platform_device_register(&gp_scalar2_device);
	platform_driver_register(&gp_scalar2_driver);

__return:
	if(nRet < 0) {
		DERROR(KERN_WARNING "Scale2InitFail\n");
		free_irq(IRQ_SCALE_ENGINE, pScale2Dev);
		kfree(pScale2Dev);
		pScale2Dev = NULL;
	}
	return nRet;
}

static void __exit
scale2_exit_module(
	void
)
{
	misc_deregister(&pScale2Dev->dev);
	free_irq(IRQ_SCALE_ENGINE, pScale2Dev);
	kfree(pScale2Dev);
	pScale2Dev = NULL;

	platform_device_unregister(&gp_scalar2_device);
	platform_driver_unregister(&gp_scalar2_driver);
}

module_init(scale2_init_module);
module_exit(scale2_exit_module);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus SCALE2 Driver");
MODULE_LICENSE_GP;
MODULE_VERSION("1.0");



