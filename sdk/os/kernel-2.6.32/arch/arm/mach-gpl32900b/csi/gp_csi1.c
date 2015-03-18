 /**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Technology Co., Ltd.         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Technology Co., Ltd. All rights are reserved by Generalplus Technology*
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Technology Co., Ltd. reserves the right to modify this    *
 *  software without notice.                                              *
 *                                                                        *
 *  Generalplus Technology Co., Ltd.                                      *
 *  3F, No.8, Dusing Rd., Science-Based Industrial Park,                  *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/
 
 /**
 * @file gp_csi.c
 * @brief CSI interface
 * @author Simon Hsu
 */

#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/diag.h>

#include <mach/clk/gp_clk_core.h>
#include <mach/hal/hal_csi1.h>

#include <mach/gp_chunkmem.h>
#include <mach/gp_cache.h>
#include <mach/gp_csi1.h>
#include <mach/gp_mipi.h>
#include <mach/gp_board.h>
#include <mach/sensor_mgr.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define	CSI_MINOR		0
#define CSI_NR_DEVS		1
#define CSI_MAX_BUF		5
#define CSI_MAX_QUE		5
#define	NO_INPUT		0xFFFFFFFF
#define USBPHY_CLK		96000000
#define CSI1_CHK_MIP_EN		0
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if 1
	#define DEBUG	DIAG_ERROR
#else
	#define DEBUG(...)
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gp_csi_dev_s {
	/*Sub device*/
	int32_t sdidx;
	struct v4l2_subdev *sd;
	callbackfunc_t *cb;
	
	/*buffer control*/
	int32_t bfidx;
	int32_t bfaddr[CSI_MAX_BUF];
	uint8_t in_que[CSI_MAX_QUE];
	uint8_t out_que[CSI_MAX_QUE];
	struct v4l2_buffer bf[CSI_MAX_BUF];
	struct v4l2_requestbuffers rbuf;
	
	int32_t major;
	int32_t csi_feint_flag;
	int32_t	csi_frame_cnt;
	struct semaphore sem;
	struct cdev c_dev;
	struct class *csi_class;

	/*MD buffer control*/
	uint32_t md_enable;
	uint32_t md_inque[CSI_MAX_QUE];
	uint32_t md_outque[CSI_MAX_QUE];
	uint32_t md_phy_addr[CSI_MAX_QUE];

	uint32_t CapCnt;
} gp_csi_dev_t;

DECLARE_WAIT_QUEUE_HEAD(csi1_fe_done);

static struct v4l2_capability csi1_cap= {
	.driver = "csi1 driver",
	.card = "CSI1",
	.bus_info = "Sensor interface",
	.version = 0,
	.capabilities = V4L2_CAP_VIDEO_CAPTURE
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/


/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_csi_dev_t *csi1_devices=NULL;
static sensor_config_t *sensor=NULL;
static char *port;
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
/**
 * @brief   ioctrl to check request buffer
 * @return  success: 0, fail: -EINVAL(<0)
 * @see
 */ 
static int32_t 
check_rqbuf_type(
	void
)
{
	if(csi1_devices->rbuf.count>CSI_MAX_BUF){
		DIAG_ERROR("too many buffers\n");
		return -EINVAL;
	}
	
	if(csi1_devices->rbuf.type!=V4L2_BUF_TYPE_VIDEO_CAPTURE){
		DIAG_ERROR("only support video capture mode");
		return -EINVAL;
	}
	
	if(csi1_devices->rbuf.memory!=V4L2_MEMORY_USERPTR){
		DIAG_ERROR("only support userptr I/O strea\n");
		return -EINVAL;
	}
	
	return 0;
}

/**
 * @brief   charater device open function
 * @return  success: 0
 * @see
 */ 
static int32_t 
gp_csi1_open(
	struct inode *inode, 
	struct file *filp
)
{
	gp_csi_dev_t *dev = NULL; /* device information */
	
	dev = container_of(inode->i_cdev, gp_csi_dev_t, c_dev);
	filp->private_data = dev;
#if USE_PPU_IRQ == 1 
	gp_enable_clock( (int*)"PPU", 1 );
#else
	gp_enable_clock( (int*)"PPU_REG", 1 );
#endif
	gp_enable_clock( (int*)"PPU_STN", 1 );
	gp_enable_clock( (int*)"CMOS_CTRL", 1 );
	gpHalCsi1Init();
	DEBUG(KERN_WARNING "Csi1Open.\n");
	return 0;
}

/**
 * @brief   charater device release function
 * @return  success: 0
 * @see
 */ 
static int32_t 
gp_csi1_release(
	struct inode *inode, 
	struct file *filp
)
{
#if 1	
	/* suspend sensor */
	if(csi1_devices->sd && csi1_devices->sd->ops->ext->suspend) {
		csi1_devices->sd->ops->ext->suspend(csi1_devices->sd);
	}
	
	if(csi1_devices->cb && csi1_devices->cb->standby != NULL) {
		csi1_devices->cb->standby(1); 
	}
	
	if(csi1_devices->cb && csi1_devices->cb->powerctl != NULL) {
		csi1_devices->cb->powerctl(0);
	}
#endif
	/* disable mclk & csi1 */
	gpHalCsi1SetMclk(0, 0, 0, 0);
	gpHalCsi1Close();
#if USE_PPU_IRQ == 1 	
	gp_enable_clock( (int*)"PPU", 0 );
#else
	gp_enable_clock( (int*)"PPU_REG", 0 );
#endif
	gp_enable_clock( (int*)"PPU_STN", 0 );
	gp_enable_clock( (int*)"CMOS_CTRL", 0 );
	/* clear flag */
	csi1_devices->sdidx = NO_INPUT;
	memset(csi1_devices->bfaddr, 0, CSI_MAX_BUF);
	memset(csi1_devices->bf, 0, CSI_MAX_BUF);
	memset(csi1_devices->in_que, 0xFF, CSI_MAX_QUE);
	memset(csi1_devices->out_que, 0xFF, CSI_MAX_QUE);
	csi1_devices->csi_feint_flag = 0;
	csi1_devices->csi_frame_cnt = 0;
	DEBUG(KERN_WARNING "Csi1close.\n");
	return 0;
}

static int32_t 
gp_csi1_s_fmt(
	struct v4l2_format *fmt
)
{
	int32_t ret, div;
	uint32_t i, idx;
	struct clk *clock;

	if(fmt->fmt.pix.priv >= sensor->sensor_fmt_num) {
		for(i=0; i<sensor->sensor_fmt_num; i++) {
			if((fmt->fmt.pix.pixelformat == sensor->fmt[i].pixelformat) &&
				(fmt->fmt.pix.width <= sensor->fmt[i].hpixel) && 
				(fmt->fmt.pix.height <= sensor->fmt[i].vline)) {
				fmt->fmt.pix.priv = i;
				break;
			}
		}

		if(sensor->sensor_fmt_num == i) {
			DEBUG("FmtSizeErr=%dx%d\n", fmt->fmt.pix.width, fmt->fmt.pix.height);
			return -1;
		}
	} 
	
	idx = fmt->fmt.pix.priv;
	if(fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
		gpHalCsi1SetInputFmt(CSI1_YUYV_IN);
		gpHalCsi1SetDataLatchTiming(0);	/*data delay 1 clk*/
	} else if(fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_UYVY) {
		gpHalCsi1SetInputFmt(CSI1_UYVY_IN);
		gpHalCsi1SetDataLatchTiming(0);	/*data delay 1 clk*/
	} else if(fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_YVYU) {
		gpHalCsi1SetInputFmt(CSI1_YUYV_IN);
		gpHalCsi1SetDataLatchTiming(1);	/*data delay 2 clk*/
	} else if(fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_VYUY) {
		gpHalCsi1SetInputFmt(CSI1_UYVY_IN);
		gpHalCsi1SetDataLatchTiming(1);	/*data delay 2 clk*/
	} else if(fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_RGB565) {
		gpHalCsi1SetInputFmt(CSI1_RGB565_IN);
		gpHalCsi1SetDataLatchTiming(0);
	} else if(fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_BGR32) {
		gpHalCsi1SetInputFmt(CSI1_BGRG_IN);
		gpHalCsi1SetDataLatchTiming(0);
	} else if(fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_RGB32) {
		gpHalCsi1SetInputFmt(CSI1_GBGR_IN);
		gpHalCsi1SetDataLatchTiming(0);
	} else {
		DEBUG("FromatErr!!!\n");
		return -1;
	}

	DEBUG("Csi1HV = %dx%d\n", sensor->fmt[idx].hpixel, sensor->fmt[idx].vline);
	gpHalCsi1SetResolution(sensor->fmt[idx].hpixel, sensor->fmt[idx].vline);
	gpHalCsi1SetHVStart(sensor->fmt[idx].hoffset, sensor->fmt[idx].voffset, sensor->fmt[idx].voffset);
	gpHalCsi1SetInterface(sensor->sensor_interlace_mode, FIELD_ODDL, sensor->sensor_timing_mode);
	gpHalCsi1SetHVSync(sensor->sensor_hsync_mode, sensor->sensor_vsync_mode, sensor->sensor_pclk_mode);
	if(sensor->sensor_data_mode == 1) {
		gpHalCsi1SetOutputFmt(CSI1_VYUY_OUT);
	} else {
		gpHalCsi1SetOutputFmt(CSI1_RGB565_OUT);
	}

	//set final mclk
	clock = clk_get(NULL, "clk_ref_sys");
	ret = clk_get_rate(clock);
	div = ret/sensor->fmt[idx].mclk;
	if((ret % sensor->fmt[idx].mclk) == 0) {
		div--;
	}
	DEBUG("MCLK = %d\n", ret/(div + 1));
	gpHalCsi1SetMclk(sensor->fmt[idx].mclk_src, div, 0, 0);
	return 0;
}

static int32_t
gp_csi1_g_ctrl(
	struct v4l2_control* ctrl
)
{
	int32_t ret;
	
	switch(ctrl->id) 
	{
		case MSG_CSI1_CUBIC:
		{
			gpCsi1Cubic_t Cubic;
			gpHalCsi1GetCubic(&Cubic.CubicEn, &Cubic.CubicMode);
			ret = copy_to_user((void __user*)ctrl->value, (void*)&Cubic, sizeof(Cubic));
			break;
		}
		case MSG_CSI1_BLACKSCREEN:
		{
			gpCsi1BlackScreen_t BS;
			gpHalCsi1GetBlackScreen(&BS.hstart, &BS.vstart, &BS.view_hsize, &BS.view_vsize);
			gpHalCsi1GetBlueScreen(&BS.BlueScreenEn, &BS.r_upper, &BS.g_upper, &BS.b_upper,
									&BS.r_lower, &BS.g_lower, &BS.b_lower);
			ret = copy_to_user((void __user*)ctrl->value, (void*)&BS, sizeof(BS));
			break;
		}
		case MSG_CSI1_BLENDING:
		{
			gpCsi1Blending_t blending;
			gpHalCsi1GetBlending(&blending.BlendEn, &blending.BlendLevel);
			ret = copy_to_user((void __user*)ctrl->value, (void*)&blending, sizeof(blending));
			break;
		}
		case MSG_CSI1_MD:
		{
			gpCsi1MD_t MD;
			ret = copy_to_user((void __user*)ctrl->value, (void*)&MD, sizeof(MD));
			break;
		}
		default:
		{
			struct v4l2_subdev *sd;
			struct v4l2_control csi_ctrl;	

			sd = csi1_devices->sd;
			if(sd && sd->ops->core->g_ctrl) {
				ret = sd->ops->core->g_ctrl(sd, &csi_ctrl);
				ret = copy_to_user((void __user*)ctrl, (void*)&csi_ctrl, sizeof(csi_ctrl));
			} else {
				ret = -1;
			}
			break;
		}
	}
	
	return ret;
}

static int32_t
gp_csi1_s_ctrl(
	struct v4l2_control* ctrl
)
{
	int32_t ret;
	
	switch(ctrl->id) 
	{
		case MSG_CSI1_CUBIC:
		{
			gpCsi1Cubic_t Cubic;
			ret = copy_from_user((void*)&Cubic, (void __user*)ctrl->value, sizeof(Cubic));
			if(ret >= 0) {
				gpHalCsi1SetCubic(Cubic.CubicEn, Cubic.CubicMode);
			}
			break;
		}
		case MSG_CSI1_BLACKSCREEN:
		{
			gpCsi1BlackScreen_t BS;
			ret = copy_from_user((void*)&BS, (void __user*)ctrl->value, sizeof(BS));
			if(ret >= 0) {
				gpHalCsi1SetBlackScreen(BS.hstart, BS.vstart, BS.view_hsize, BS.view_vsize);
				gpHalCsi1SetBlueScreen(BS.BlueScreenEn, BS.r_upper, BS.g_upper, BS.b_upper,
									BS.r_lower, BS.g_lower, BS.b_lower);
			}
			break;
		}
		case MSG_CSI1_BLENDING:
		{
			gpCsi1Blending_t blending;
			ret = copy_from_user((void*)&blending, (void __user*)ctrl->value, sizeof(blending));
			if(ret >= 0) {
				gpHalCsi1SetBlending(blending.BlendEn, blending.BlendLevel);
			}
			break;
		}
		case MSG_CSI1_MD:
		{
			gpCsi1MD_t MD;
			ret = copy_from_user((void*)&MD, (void __user*)ctrl->value, sizeof(MD));
			if(ret >= 0) {
				memset(csi1_devices->md_inque, 0x00, CSI_MAX_QUE);
				memset(csi1_devices->md_outque, 0x00, CSI_MAX_QUE);
				memset(csi1_devices->md_phy_addr, 0x00, CSI_MAX_QUE);

				csi1_devices->md_enable = MD.MDEn;
				gpHalCsi1SetMD(MD.MDFrame, MD.MDVGA, MD.MDYUV, 
								MD.MDMode, MD.MDBlk8x8, MD.threshold);
				gpHalCsi1SetMDHVPos(MD.hpos, MD.vpos);
				gpHalCsi1SetMDArbRes(MD.MD_X_Width,MD.MD_Y_Width);
			}
			break;
		}
		case MSG_CSI1_JPEG:
		{
			unsigned int jpeg_en;
			ret = copy_from_user((void*)&jpeg_en, (void __user*)ctrl->value, sizeof(jpeg_en));
			if(ret >= 0) {
				gpHalCsi1SetJpegModeEn(jpeg_en);
			}
			break;
		}		
		default:
		{
			struct v4l2_subdev *sd;
			struct v4l2_control csi_ctrl;

			ret = copy_from_user((void*)&csi_ctrl, (void __user*)ctrl, sizeof(csi_ctrl));
			if(ret >= 0) {
				sd = csi1_devices->sd;
				if(sd && sd->ops->core->s_ctrl) {
					ret = sd->ops->core->s_ctrl(sd, &csi_ctrl);
				} else {
					ret = -1;
				}
			}
			break;
		}
	}	
	return ret;
}

static int32_t 
gp_csi1_s_capture(
	gpCsiCapture_t *pCap
)
{
	int32_t i, ret;
	uint32_t addr;
	struct v4l2_format bkfmt, fmt;
	struct v4l2_subdev *sd;
	
	memset(&bkfmt, 0, sizeof(bkfmt));
	memset(&fmt, 0, sizeof(fmt));
	
	/* 1.csi disable and backup preview format */
#if CSI1_CHK_MIP_EN == 1
	if(strcmp("MIPI", port) == 0) {
		gp_mipi_set_irq(0);
	}
#endif
	gpHalCsi1SetIRQ(CSI1_FRAME_END, 0);
	gpHalCsi1SetEnable(0);
	sd = csi1_devices->sd;
	ret = sd->ops->video->g_fmt(sd, &bkfmt);
	if(ret < 0) {
		DEBUG("backup g_fmt Err\n");
		return -1;
	}

#if 1
	/* 1-1.clear out_que and line after in_que */
	for(i=0; i<csi1_devices->rbuf.count; i++) {
		int32_t j, in_que, out_que;
		
		out_que = csi1_devices->out_que[i];
		if(out_que != 0xFF) {
			csi1_devices->out_que[i] = 0xFF;
			csi1_devices->bf[out_que].flags = V4L2_BUF_FLAG_QUEUED;
			DEBUG("out_que = 0x%x\n", out_que);

			for(j=0; j<csi1_devices->rbuf.count; j++) {
				in_que = csi1_devices->in_que[j];
				DEBUG("in_que[%d] = 0x%x\n", j, in_que);
				if(in_que == 0xFF) {
					csi1_devices->in_que[j] = out_que;
					break;
				}
			}
		}	
	}
#else
	/* 1-1.discard all done buffer */
	for(i=0; i<csi1_devices->rbuf.count; i++){
		csi1_devices->out_que[i] = 0xFF;
		csi1_devices->in_que[i] = 0xFF;
		csi1_devices->bf[i].flags = V4L2_BUF_FLAG_QUEUED;
	}

	for(i=0; i<csi1_devices->rbuf.count; i++){
		csi1_devices->in_que[i]	= i;
	}
#endif

	/* 2.change to capture */
	addr = (uint32_t)gp_user_va_to_pa((int16_t *)pCap->buffaddr);	
	gpHalCsi1SetBuf(addr);
	
	fmt.fmt.pix.width = pCap->width;
	fmt.fmt.pix.height = pCap->height;
	fmt.fmt.pix.pixelformat = pCap->pixelformat;
	fmt.fmt.pix.priv = pCap->priv;	
	if(gp_csi1_s_fmt(&fmt) < 0) {
		DEBUG("capture gp_csi1_s_fmt Err\n");
		return -1;
	}

	ret = sd->ops->video->s_fmt(sd, &fmt);
	if(ret < 0) {
		DEBUG("capture s_fmt Err\n");
		return -1;
	}
		
	/* 3.irq enable and wait N frame */
	csi1_devices->csi_feint_flag = 0;
	csi1_devices->CapCnt = pCap->waitcnt;
	if(csi1_devices->CapCnt < 1) {
		csi1_devices->CapCnt = 2;
	}
	gpHalCsi1SetEnable(1);
	gpHalCsi1SetIRQ(CSI1_FRAME_END, 1);
#if CSI1_CHK_MIP_EN == 1
	if(strcmp("MIPI", port) == 0) {
		gp_mipi_set_irq(1);
	}
#endif
	wait_event_interruptible(csi1_fe_done, (csi1_devices->csi_feint_flag != 0));
	
	/* 4. resume to preview */
#if CSI1_CHK_MIP_EN == 1	
	if(strcmp("MIPI", port) == 0) {
		gp_mipi_set_irq(0);
	}
#endif
	gpHalCsi1SetIRQ(CSI1_FRAME_END, 0);
	gpHalCsi1SetEnable(0);
	csi1_devices->csi_feint_flag = 0;
	
	gpHalCsi1SetBuf(csi1_devices->bfaddr[csi1_devices->in_que[0]]);
	if(gp_csi1_s_fmt(&bkfmt) < 0) {
		DEBUG("resume gp_csi1_s_fmt Err\n");
		return -1;
	}
	
	ret = sd->ops->video->s_fmt(sd, &bkfmt);
	if(ret < 0) {
		DEBUG("resume s_fmt Err\n");
		return -1;
	}

	gpHalCsi1SetEnable(1);
	gpHalCsi1SetIRQ(CSI1_FRAME_END, 1);
#if CSI1_CHK_MIP_EN == 1	
	if(strcmp("MIPI", port) == 0) {
		gp_mipi_set_irq(1);
	}
#endif
	return 0;
}			

/**
 * @brief   charater device ioctl function
 * @return  success: 0
 * @see
 */ 
static long 
gp_csi1_ioctl(
	struct file *filp, 
	uint32_t cmd, 
	unsigned long arg
)
{
	struct v4l2_input in;
	struct v4l2_queryctrl qc;
	struct v4l2_fmtdesc fmtd;
	struct v4l2_format fmt;
	struct v4l2_buffer bf;
	struct v4l2_control ctrl;
	struct v4l2_streamparm param;
	struct v4l2_subdev *sd;
	struct v4l2_cropcap cc;
	struct v4l2_crop crop;
	struct v4l2_interface interface;
	struct clk *clock;
	v4l2_std_id std;	
	gpCsiCapture_t capture;
	callbackfunc_t *cb;
	gpCsiMclk_t mclk;
	uint8_t div; 
	int32_t idx;
	int32_t ret=0;
	int32_t i;
	int32_t setctrl;
	int16_t *addr;
	
	if(down_interruptible(&csi1_devices->sem)) {
		return -ERESTARTSYS;
	}
	
	/* Success */
	switch(cmd) {
		case VIDIOC_QUERYCAP:
			copy_to_user((void __user*)arg, &csi1_cap, sizeof(struct v4l2_capability));
			break;
		
		case VIDIOC_ENUMINPUT:
			ret = copy_from_user((void*)&in, (void __user*)arg, sizeof(struct v4l2_input));			
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			ret = gp_get_sensorinfo(in.index, (int*)&sd, (int*)&cb, (int*)&port, (int*)&sensor);
			if(ret == -1) {
				DIAG_ERROR("Index Out of bound or unregister\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			DEBUG("Device[%d]:%s\n", in.index, sd->name);
			in.type = V4L2_INPUT_TYPE_CAMERA;
			strcpy(in.name, sd->name);

			ret = copy_to_user((void __user*)arg, (void*)&in, sizeof(struct v4l2_input));
			break;
		
		case VIDIOC_S_INPUT:
			if((int32_t)arg == csi1_devices->sdidx) {
				DIAG_ERROR("The Same input\n");
				goto ioctlret;
			}

			ret = gp_get_sensorinfo((int32_t)arg, (int*)&csi1_devices->sd, (int*)&(csi1_devices->cb), (int*)&port, (int*)&sensor);
			if(ret == -1) {
				DIAG_ERROR("Set input fail\n");
				ret = -EINVAL;
				goto ioctlret;
			}
			
			csi1_devices->sdidx = (int32_t)arg;
			if(csi1_devices->cb && csi1_devices->cb->powerctl != NULL) {
				csi1_devices->cb->powerctl(1);
			}
			
			if(csi1_devices->cb && csi1_devices->cb->standby != NULL) {
				csi1_devices->cb->standby(0); 
			}
			
			if(csi1_devices->cb && csi1_devices->cb->set_port != NULL) {
				csi1_devices->cb->set_port(port);
			}

			/* open mclk, some sensor need mclk befor init */
			clock = clk_get(NULL, "clk_ref_sys");
			ret = clk_get_rate(clock);
			div = ret/sensor->fmt[0].mclk;
			if((ret % sensor->fmt[0].mclk) == 0) {
				div--;
			}
			DEBUG("MCLK = %d\n", ret/(div + 1));
			gpHalCsi1SetMclk(sensor->fmt[0].mclk_src, div, 0, 0);

			sd = csi1_devices->sd;
			if(sd && sd->ops->core->reset) {
				ret = sd->ops->core->reset(sd, 0);
			}

			if(sd && sd->ops->core->init) {
				ret = sd->ops->core->init(sd, 0);
				if(ret < 0) {
					DIAG_ERROR("sensor init fail\n");
					ret=-EINVAL;
					goto ioctlret;
				}
			}
			break;
		
		case VIDIOC_G_INPUT:
			ret = csi1_devices->sdidx;
			break;
		
		case VIDIOC_S_FMT:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&fmt, (void __user*)arg, sizeof(struct v4l2_format));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			if(gp_csi1_s_fmt(&fmt) < 0) {
				ret = -EINVAL;
				goto ioctlret;
			}
		
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->s_fmt) {
				ret = sd->ops->video->s_fmt(sd, &fmt);
			}
			break;

		case VIDIOC_G_FMT:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->g_fmt) {
				ret = sd->ops->video->g_fmt(sd, &fmt);
				if(ret < 0) {				
					ret = -EINVAL;
					goto ioctlret;		
				}
			}
			
			ret = copy_to_user((void __user*)arg, (void*)&fmt, sizeof(struct v4l2_format));
			break;
		
		case VIDIOC_TRY_FMT:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&fmt, (void __user*)arg, sizeof(struct v4l2_format));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->try_fmt) {
				ret = sd->ops->video->try_fmt(sd, &fmt);
			}
			break;

		case VIDIOC_ENUM_FMT:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&fmtd, (void __user*)arg, sizeof(struct v4l2_fmtdesc));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->enum_fmt) {
				ret = sd->ops->video->enum_fmt(sd, &fmtd);
				if(ret < 0) {				
					ret = -EINVAL;
					goto ioctlret;		
				}
			}

			ret = copy_to_user((void __user*)arg, (void*)&fmtd, sizeof(struct v4l2_fmtdesc));
			break;

		case VIDIOC_QUERYCTRL:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&qc, (void __user*)arg, sizeof(struct v4l2_queryctrl));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->core->queryctrl) {
				ret = sd->ops->core->queryctrl(sd, &qc);
				if(ret < 0) {				
					ret = -EINVAL;
					goto ioctlret;		
				}
			}
			
			ret = copy_to_user((void __user*)arg, (void*)&qc, sizeof(struct v4l2_queryctrl));
			break;
	
		case VIDIOC_G_CTRL:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}
			
			ret = copy_from_user((void*)&ctrl, (void __user*)arg, sizeof(struct v4l2_control));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			ret = gp_csi1_g_ctrl(&ctrl);
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}			

			ret = copy_to_user((void __user*)arg, (void*)&ctrl, sizeof(struct v4l2_control));
			break;
		
		case VIDIOC_S_CTRL:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}
			
			ret = copy_from_user((void*)&ctrl, (void __user*)arg, sizeof(struct v4l2_control));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			ret = gp_csi1_s_ctrl(&ctrl);
			break;
		
		case VIDIOC_S_INTERFACE:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&interface, (void __user*)arg, sizeof(struct v4l2_interface));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			gpHalCsi1SetInterface(interface.Interlace, interface.Field, interface.Interface);
			gpHalCsi1SetHVSync(interface.HsyncAct, interface.VsyncAct, interface.SampleEdge);
			if(interface.FmtOut == 1) {
				gpHalCsi1SetOutputFmt(CSI1_VYUY_OUT);
			} else {
				gpHalCsi1SetOutputFmt(CSI1_RGB565_OUT);
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->ext->s_interface) {
				ret = sd->ops->ext->s_interface(sd, &interface);
			}
			break;
		
		case VIDIOC_G_INTERFACE:
			setctrl = gpHalCsi1GetCtrl(2);
			if(setctrl & CSI1_CCIR656) {
				interface.Interface = CCIR656;
			} else if(setctrl & CSI1_HREF) {
				interface.Interface = HREF;
			} else {
				interface.Interface = CCIR601;
			}
			
			interface.HsyncAct = (setctrl & CSI1_HRST_RISE) ? HSYNC_HACT : HSYNC_LACT;
			interface.VsyncAct = (setctrl & CSI1_VRST_RISE) ? VSYNC_HACT : VSYNC_LACT;
			interface.Field = (setctrl & CSI1_FIELDINV) ? FIELD_ODDL : FIELD_ODDH;
			interface.Interlace = (setctrl & CSI1_INTERLACE) ? INTERLACE : NON_INTERLACE;
			interface.FrameEndMode = EVERYFRM;
			interface.SampleEdge = (setctrl & CSI1_CLKIINV) ? SAMPLE_NEG : SAMPLE_POSI;
			interface.FmtOut = YUVOUT;
			
			ret = copy_to_user((void __user*)arg, (void*)&interface, sizeof(struct v4l2_interface));
			break;

		case VIDIOC_S_MCLK:
			ret = copy_from_user((void*)&mclk, (void __user*)arg, sizeof(mclk));
			if(ret < 0) {
				DIAG_ERROR("mclk set error\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			if(mclk.mclk_out == 0) {
				mclk.mclk_sel = div = 0;
				mclk.pclk_dly = 0;
				mclk.pclk_revb = 0;
				DEBUG("mclk = 0\n");
			} else {
			  clock = clk_get(NULL, "clk_ref_sys");
			  ret = clk_get_rate(clock);

				div = ret/mclk.mclk_out;
				if((ret % mclk.mclk_out) == 0) {
					div--;
				}
				DEBUG("MCLK = %d\n", ret/(div + 1));
			}
			gpHalCsi1SetMclk(mclk.mclk_sel, div, mclk.pclk_dly, mclk.pclk_revb);
			gpHalCsi1SetDataLatchTiming(mclk.prvi);
			break;
			
		case VIDIOC_G_MCLK:
			gpHalCsi1GetMclk(&mclk.mclk_sel, &div, &mclk.pclk_dly, &mclk.pclk_revb);
			if(mclk.mclk_sel == CSI_CLK_SPLL) {
				clock = clk_get(NULL, "clk_ref_ceva");
				ret = clk_get_rate(clock);
			} else {
				ret = USBPHY_CLK;
			}
			
			mclk.mclk_out = ret/(div + 1);
			mclk.prvi = gpHalCsi1GetCtrl(1) & 0x07;
			ret = copy_to_user((void __user*)arg, (void*)&mclk, sizeof(mclk));
			if(ret < 0) {
				DIAG_ERROR("mclk get error\n");
				ret = -EINVAL;
				goto ioctlret;
			}
			break;
			
		case VIDIOC_CROPCAP:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&cc, (void __user*)arg, sizeof(struct v4l2_cropcap));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->cropcap) {
				ret = sd->ops->video->cropcap(sd, &cc);
				if(ret < 0) {				
					ret = -EINVAL;
					goto ioctlret;		
				}
			}

			ret = copy_to_user((void __user*)arg, (void*)&cc, sizeof(struct v4l2_cropcap));
			break;
		
		case VIDIOC_G_CROP:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&crop, (void __user*)arg, sizeof(struct v4l2_crop));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->g_crop) {
				ret = sd->ops->video->g_crop(sd, &crop);
				if(ret < 0) {				
					ret = -EINVAL;
					goto ioctlret;		
				}
			}

			ret = copy_to_user((void __user*)arg, (void*)&crop, sizeof(struct v4l2_crop));
			break;
		
		case VIDIOC_S_CROP:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&crop, (void __user*)arg, sizeof(struct v4l2_crop));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->s_crop) {
				ret = sd->ops->video->s_crop(sd, &crop);
			}
			break;
		
		case VIDIOC_G_PARM:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&param, (void __user*)arg, sizeof(struct v4l2_streamparm));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->g_parm) {
				ret = sd->ops->video->g_parm(sd, &param);
				if(ret < 0) {				
					ret = -EINVAL;
					goto ioctlret;		
				}
			}

			ret = copy_to_user((void __user*)arg, (void*)&param, sizeof(struct v4l2_streamparm));
			break;
		
		case VIDIOC_S_PARM:
			if(csi1_devices->sdidx == NO_INPUT) {
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			ret = copy_from_user((void*)&param, (void __user*)arg, sizeof(struct v4l2_streamparm));
			if(ret < 0) {				
				ret = -EINVAL;
				goto ioctlret;		
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->s_parm) {
				ret = sd->ops->video->s_parm(sd, &param);
			}
			break;

		case VIDIOC_REQBUFS:
			ret = copy_from_user(&(csi1_devices->rbuf),(struct v4l2_requestbuffers*)arg, sizeof(struct v4l2_requestbuffers));
			ret = check_rqbuf_type();
			break;

		case VIDIOC_STREAMON:
			if(arg == (unsigned long)NULL) {
				if(csi1_devices->in_que[0] == 0xFF) {
					DIAG_ERROR("No buffer in Que\n");
					ret = -EINVAL;
					goto ioctlret;
				}
				gpHalCsi1SetBuf(csi1_devices->bfaddr[csi1_devices->in_que[0]]);
				gpHalCsi1Start();
				
				/* MD */
				if(csi1_devices->md_enable) {
					if(csi1_devices->md_inque[0] != 0) {		
						gpHalCsi1SetMDFbAddr(csi1_devices->md_phy_addr[0]);
						gpHalCsi1SetIRQ(CSI1_MOTION_DECT, 1);
						gpHalCsi1SetMDEn(1);
					} else {
						csi1_devices->md_enable = 0;
						gpHalCsi1SetIRQ(CSI1_MOTION_DECT, 0);
						gpHalCsi1SetMDEn(0);
					}
				} 
			} else {
				DIAG_ERROR("csi1 start fail\n");
				ret=-EINVAL;
			}
			break;
		
		case VIDIOC_STREAMOFF:
		#if 0
			/* suspend sensor */
			csi1_devices->sd->ops->ext->suspend(csi1_devices->sd);
			if(csi1_devices->cb && csi1_devices->cb->standby != NULL) {
				csi1_devices->cb->standby(1); 
			}
			
			if(csi1_devices->cb && csi1_devices->cb->powerctl != NULL) {
				csi1_devices->cb->powerctl(0);
			}
			
			/* disable mclk & csi1 */
			gpHalCsi1SetMclk(0, 0, 0, 0);
			gpHalCsi1Stop();

			/* clear flag */
			memset(csi1_devices->bfaddr, 0, CSI_MAX_BUF);
			memset(csi1_devices->bf, 0, CSI_MAX_BUF);
			memset(csi1_devices->in_que, 0xFF, CSI_MAX_QUE);
			memset(csi1_devices->out_que, 0xFF, CSI_MAX_QUE);
			csi1_devices->csi_feint_flag = 0;
			csi1_devices->CapCnt = 0;
			csi1_devices->md_enable = 0;
		#else
			/* disable csi1 */
			gpHalCsi1SetEnable(0);
		
			/* clear out_que and line after in_que */
			for(i=0; i<csi1_devices->rbuf.count; i++) {
				int32_t j, in_que, out_que;
		
				out_que = csi1_devices->out_que[i];
				if(out_que != 0xFF) {
					csi1_devices->out_que[i] = 0xFF;
					csi1_devices->bf[out_que].flags = V4L2_BUF_FLAG_QUEUED;
					DEBUG("out_que = 0x%x\n", out_que);

					for(j=0; j<csi1_devices->rbuf.count; j++) {
					in_que = csi1_devices->in_que[j];
						DEBUG("in_que[%d] = 0x%x\n", j, in_que);
						if(in_que == 0xFF) {
							csi1_devices->in_que[j] = out_que;
							break;
						}
					}
				}	
			}	

			/* MD */
			if(csi1_devices->md_enable) {
				gpHalCsi1SetIRQ(CSI1_MOTION_DECT, 0);
				gpHalCsi1SetMDEn(0);
				
				memset(csi1_devices->md_inque, 0x00, CSI_MAX_QUE);
				memset(csi1_devices->md_outque, 0x00, CSI_MAX_QUE);
				memset(csi1_devices->md_phy_addr, 0x00, CSI_MAX_QUE);
			}					
		#endif
			break;

		case VIDIOC_QBUF:
			ret = copy_from_user(&bf, (struct v4l2_buffer*)arg, sizeof(struct v4l2_buffer));
			if(ret < 0) {
				ret = -EINVAL;
				goto ioctlret;
			}
			
			if(bf.type != csi1_devices->rbuf.type) {
				DIAG_ERROR("QBuf Type error\n");
				ret = -EINVAL;
				goto ioctlret;
			}
			
			if(bf.index >= csi1_devices->rbuf.count) {
				DIAG_ERROR("QBuf index out of bound\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			bf.flags = V4L2_BUF_FLAG_QUEUED;
			addr = (int16_t *)gp_user_va_to_pa((int16_t *)bf.m.userptr);
			if(addr == 0) {
				ret = -EINVAL;
				goto ioctlret;
			}
			
			/* find empty index */	
			gpHalCsi1SetIRQ(CSI1_FRAME_END, 0);
			for(i=0; i<CSI_MAX_QUE; i++) {
				if(csi1_devices->in_que[i] == bf.index) {
					DIAG_ERROR("csi1: in_que error\n");
					ret = -EINVAL;
					goto ioctlret;
				}
				
				if(csi1_devices->in_que[i] == 0xFF)	{
					csi1_devices->in_que[i] = bf.index;
					idx = bf.index;
					break;
				}
			}
			
			if(i == CSI_MAX_QUE) {
				DIAG_ERROR("csi1: in_que not find\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			/* MD */
			if(csi1_devices->md_enable && bf.reserved) {
				for(i=0; i<CSI_MAX_QUE; i++) {
					if(csi1_devices->md_inque[i] == 0x00) {
						csi1_devices->md_inque[i] = (uint32_t)bf.reserved;
						csi1_devices->md_phy_addr[i] = (uint32_t)gp_user_va_to_pa((int16_t *)csi1_devices->md_inque[0]);
						break;
					}
				}
				
				if(i == CSI_MAX_QUE) {
					DIAG_ERROR("md Que is full\n");
				}
			}

			csi1_devices->bfaddr[idx] = (int)addr;
			memcpy(&csi1_devices->bf[idx], &bf, sizeof(struct v4l2_buffer));
			gpHalCsi1SetIRQ(CSI1_FRAME_END, 1);
			break;
		
		case VIDIOC_DQBUF:
			ret = copy_from_user(&bf, (struct v4l2_buffer*)arg, sizeof(struct v4l2_buffer));
			if(ret < 0) {
				ret = -EINVAL;
				goto ioctlret;
			}
			
			if(bf.type != csi1_devices->rbuf.type) {
				ret = -EINVAL;
				goto ioctlret;
			}
			
			if(csi1_devices->out_que[0] == 0xFF) {
				DIAG_ERROR("no buffer ready\n");
				ret = -EINVAL;
				goto ioctlret;
			}

			gpHalCsi1SetIRQ(CSI1_FRAME_END, 0);
			idx = csi1_devices->out_que[0];
			
			/* MD */
			if(csi1_devices->md_enable && csi1_devices->md_outque[0]) {
					csi1_devices->bf[idx].reserved = csi1_devices->md_outque[0];
					for(i=0; i<(CSI_MAX_QUE-1); i++) {
						csi1_devices->md_outque[i] = csi1_devices->md_outque[i+1];
					}
					csi1_devices->md_outque[CSI_MAX_QUE-1] = 0x00;
			} else {
				csi1_devices->bf[idx].reserved = 0x00;
			}

			memcpy(&bf, &(csi1_devices->bf[idx]), sizeof(struct v4l2_buffer));
		
			/* shift the out_queue buffer */	
			for(i=0; i<(CSI_MAX_QUE-1); i++) {
				csi1_devices->out_que[i] = csi1_devices->out_que[i+1];
			}
			
			csi1_devices->out_que[CSI_MAX_QUE-1] = 0xFF;
			gpHalCsi1SetIRQ(CSI1_FRAME_END, 1);

			ret = copy_to_user((struct v4l2_buffer*)arg, &bf, sizeof(struct v4l2_buffer));
			break;
		
		case VIDIOC_QUERYSTD:
			if(csi1_devices->sdidx == NO_INPUT){
				DIAG_ERROR("please set input first\n");
				ret = -EINVAL;
				goto ioctlret;
			}
			
			sd = csi1_devices->sd;
			if(sd && sd->ops->video->querystd) {
				ret = sd->ops->video->querystd(sd, &std);
				if(ret < 0) {
					ret = -EINVAL;
					goto ioctlret;
				}
				
				ret = copy_to_user((void __user*)arg, (void*)&std, sizeof(v4l2_std_id));
			} else {
				ret = ENOIOCTLCMD;
			}
			break;
		
		case VIDIOC_CAPTURE:
			ret = copy_from_user((void*)&capture, (void __user*)arg, sizeof(capture));
			if(ret < 0) {
				ret = -EINVAL;
				goto ioctlret;
			}
			
			ret = gp_csi1_s_capture(&capture);
			break;

		default:		
			ret = -ENOTTY;	/* Inappropriate ioctl for device */
			goto ioctlret;
	}
	
ioctlret:
	up(&csi1_devices->sem);
	return ret;
}

static int32_t 
gp_csi1_handle_capture(
	void *dev_id
)
{
	DEBUG("%d\n", csi1_devices->CapCnt);
	csi1_devices->CapCnt--;
	if(csi1_devices->CapCnt == 0) {
		if(csi1_devices->csi_feint_flag == 0) {
			csi1_devices->csi_feint_flag = 1;
			wake_up_interruptible(&csi1_fe_done);
		} else {
			csi1_devices->CapCnt++;
		}
	}
	return 0;
}

static int32_t 
gp_csi1_handle_FrameEnd(
	void *dev_id
)
{
	uint32_t i, EmptyIdx, ReadyIdx;
	
	/* find empty frame */
	EmptyIdx = csi1_devices->in_que[1];
	if(EmptyIdx == 0xFF) {
		DIAG_ERROR("csi1: no empty buf\n");
		return 0; 
	}
	
	/* get/set empty buffer to h/w */
	gpHalCsi1SetBuf(csi1_devices->bfaddr[EmptyIdx]);

	/* get/set ready buffer */
	ReadyIdx = csi1_devices->in_que[0];
	csi1_devices->bf[ReadyIdx].flags = V4L2_BUF_FLAG_DONE;
	do_gettimeofday(&csi1_devices->bf[ReadyIdx].timestamp);

	/* push the ready index into out_que buffer */
	for(i=0; i<CSI_MAX_QUE; i++) {
		if(csi1_devices->out_que[i] == ReadyIdx) {
			DIAG_ERROR("csi1: out_que error\n");
			return 0;
		}
		
		if(csi1_devices->out_que[i] == 0xFF) {
			csi1_devices->out_que[i] = ReadyIdx;
			break;
		}
	}
	
	if(i == CSI_MAX_QUE) {
		DIAG_ERROR("csi1: out_que not find\n");
		return 0;
	}
	
	/* shift the in_que buffer */
	for(i=0; i<(CSI_MAX_QUE-1); i++) {
		csi1_devices->in_que[i] = csi1_devices->in_que[i+1];
	}
	
	csi1_devices->in_que[CSI_MAX_QUE-1]=0xFF;

	/* wake up poll */
	csi1_devices->csi_frame_cnt++;
	wake_up_interruptible(&csi1_fe_done);
	return 0;
}

static int32_t 
gp_csi1_handle_MD(
	void *dev_id
)
{
	uint32_t i, rdy_addr;
	
	/* find empty frame in_que[1]*/
	if(csi1_devices->md_inque[1] == 0x00) {
		return 0; 
	}
	
	/* set buffer to h/w */
	gpHalCsi1SetMDFbAddr(csi1_devices->md_phy_addr[1]);

	/* get/set ready buffer */
	rdy_addr = csi1_devices->md_inque[0];
	
	/* shift the in_que buffer */
	for(i=0; i<(CSI_MAX_QUE-1); i++) {
		csi1_devices->md_inque[i] = csi1_devices->md_inque[i+1];
		csi1_devices->md_phy_addr[i] = csi1_devices->md_phy_addr[i+1];
	}
	csi1_devices->md_inque[CSI_MAX_QUE-1] = 0x00;
	csi1_devices->md_phy_addr[CSI_MAX_QUE-1] = 0x00;
	
	/* push the ready addr into out_que */
	for(i=0; i<CSI_MAX_QUE; i++) {
		if(csi1_devices->md_outque[i] == 0x00) {
			csi1_devices->md_outque[i] = rdy_addr;
			break;
		}
	}
	
	if(i == CSI_MAX_QUE) {
		return 0;
	}
	
	return 0;
}

/**
 * @brief   sensor interrupt handler
 * @return  success: IRQ_HANDLED
 * @see
 */ 
static irqreturn_t 
gp_csi1_irq_handler(
	int32_t irq, 
	void *dev_id
)
{
	uint32_t status;
	int32_t ret = -1;

	status = gpHalCsi1Clearisr();
	if(status & CSI1_FRAME_END) {
	#if CSI1_CHK_MIP_EN == 1	
		if((strcmp("MIPI", port) == 0) && 
			(gp_mipi_get_curframe_status() == 0)){
			DEBUG("MIPIFrameFail\n");
			ret = 0;
	#else 
		if(0) {
	#endif
		} else {
			if(csi1_devices->CapCnt > 0) {
				ret = gp_csi1_handle_capture(dev_id);
			} else {
				ret = gp_csi1_handle_FrameEnd(dev_id);
			}
		}
	}

	if(status & CSI1_MOTION_DECT) {
		ret = gp_csi1_handle_MD(dev_id);
	}

	if(status & CSI1_MD_UNDER_RUN) {
		DEBUG("MDUnderRun\n");
		ret = 0;
	}

	if(status & CSI1_UNDER_RUN) {
		DEBUG("UnderRun\n");
		ret = 0;
	}

	if(ret >= 0) {
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

/**
 * @brief   character device poll function
 * @return  success: POLLIN
 * @see
 */ 
static uint32_t 
gp_csi1_poll(
	struct file *filp,
	poll_table *poll
)
{
	uint32_t mask = 0;

#if 0
	wait_event_interruptible(csi1_fe_done, (csi1_devices->csi_frame_cnt > 0));
#else
	poll_wait(filp, &csi1_fe_done, poll);
#endif

	if(csi1_devices->csi_frame_cnt > 0) {
		gpHalCsi1SetIRQ(CSI1_FRAME_END, 0);
		csi1_devices->csi_frame_cnt--;
		gpHalCsi1SetIRQ(CSI1_FRAME_END, 1);
		mask = POLLIN | POLLRDNORM;
	}
	return mask;
}

struct file_operations csi1_fops = {
	.owner = THIS_MODULE,
	.open = gp_csi1_open,
	.unlocked_ioctl = gp_csi1_ioctl,
	.release = gp_csi1_release,
	.poll = gp_csi1_poll,
};


/**
 * @brief display device release                                              
 */                                                                         
static void
csi1_device_release(
	struct device *dev
)
{
	printk("remove csi device ok\n");
}

static struct platform_device csi1_device = {
	.name	= "gp-csi1",
	.id		= 0,
	.dev	= {
		.release = csi1_device_release,
	},
};

#ifdef CONFIG_PM
static int
csi1_suspend(
	struct platform_device *pdev,
	pm_message_t state
)
{
	gpHalCsi1Suspend();
	return 0;
}

static int
csi1_resume(
	struct platform_device *pdev
)
{
	gpHalCsi1Resume();
	return 0;
}
#else
#define csi1_suspend NULL
#define	csi1_resume NULL
#endif


/**                                                                         
 * @brief audio driver define                                               
 */                                                                         
static struct platform_driver csi1_driver = {
	.suspend = csi1_suspend,
	.resume = csi1_resume,
	.driver = {
		.owner = THIS_MODULE,
		.name = "gp-csi1"
	}
};


/**
 * @brief   character device module exit function
 * @see
 */ 
static void __exit 
gp_csi1_module_exit(
	void
)
{
	dev_t devno = MKDEV(csi1_devices->major, CSI_MINOR);
	
	device_destroy(csi1_devices->csi_class, devno);
	cdev_del(&csi1_devices->c_dev);	
#if USE_PPU_IRQ == 1 
	free_irq(IRQ_PPU, csi1_devices);
#else
	free_irq(IRQ_PPU_SENSOR, csi1_devices);
#endif
	class_destroy(csi1_devices->csi_class);
	kfree(csi1_devices);
	csi1_devices = NULL;
	unregister_chrdev_region(devno, CSI_NR_DEVS);
	platform_device_unregister(&csi1_device);
	platform_driver_unregister(&csi1_driver);
}

/**
 * @brief   character device module init function
 * @return  success: 0
 * @see
 */ 
static int32_t __init 
gp_csi1_module_init(
	void
)
{
	int32_t ret;
	int32_t devno;
	dev_t dev;
	struct device *device;

	DEBUG(KERN_WARNING "ModuleInit: csi1 \n");
	/* allocate a major number to csi module */
	ret = alloc_chrdev_region( &dev, CSI_MINOR, 1, "csi1" );
	if( ret<0 )	{
		DIAG_ERROR("CSI: can't get major\n");
		goto fail_init;
	}
	
	/* allocate a structure for csi device */
	csi1_devices = kmalloc(sizeof(gp_csi_dev_t), GFP_KERNEL);
	if(!csi1_devices) {
		DIAG_ERROR("CSI: can't kmalloc\n");
		ret = -ENOMEM;
		goto fail_kmalloc;
	}
	memset(csi1_devices, 0, sizeof(gp_csi_dev_t));
	csi1_devices->major = MAJOR(dev);
	
	/* create class for csi character device */
	csi1_devices->csi_class = class_create(THIS_MODULE, "csi1");
	if (IS_ERR(csi1_devices->csi_class))
	{
		DIAG_ERROR("CSI: can't create class\n");
		ret = -EFAULT;
		goto fail_create_class;
	}
	
	/* request a irq for csi interrupt service */
#if USE_PPU_IRQ == 1 
	ret = request_irq(IRQ_PPU, gp_csi1_irq_handler, IRQF_SHARED, "CSI1_IRQ", csi1_devices);
#else 
	ret = request_irq(IRQ_PPU_SENSOR, gp_csi1_irq_handler, IRQF_SHARED, "CSI1_IRQ", csi1_devices);
#endif
	if (ret < 0) {
		DIAG_ERROR("CSI: request csi irq fail\n");
		goto fail_request_irq;
	}
	
	/* create character device node */
	devno = MKDEV(csi1_devices->major, CSI_MINOR);
	cdev_init(&(csi1_devices->c_dev), &csi1_fops);
	csi1_devices->c_dev.owner = THIS_MODULE;
	csi1_devices->c_dev.ops = &csi1_fops;
	ret = cdev_add(&(csi1_devices->c_dev), devno, 1);	
	if(ret < 0){
		DIAG_ERROR("CSI: cdev_add error\n");
		goto fail_device_register;
	}
	device = device_create( csi1_devices->csi_class, NULL, devno, NULL, "csi%d", 1);
	if(!device){
		DIAG_ERROR("CSI: device_create error\n");
		goto fail_device_create;
	}
	csi1_devices->sdidx = NO_INPUT;
	memset(csi1_devices->in_que, 0xFF, CSI_MAX_QUE);
	memset(csi1_devices->out_que, 0xFF, CSI_MAX_QUE);
	/* initial the semaphore */
	init_MUTEX(&(csi1_devices->sem));

	platform_device_register(&csi1_device);
	return platform_driver_register(&csi1_driver);	

fail_device_create:	
	cdev_del(&csi1_devices->c_dev);
fail_device_register:
#if USE_PPU_IRQ == 1 
	free_irq(IRQ_PPU, csi1_devices);
#else
	free_irq(IRQ_PPU_SENSOR, csi1_devices);
#endif
fail_request_irq:
	class_destroy(csi1_devices->csi_class);
fail_create_class:
	kfree(csi1_devices);
fail_kmalloc:
	unregister_chrdev_region(dev, CSI_NR_DEVS);
fail_init:
	return ret;
}

module_init(gp_csi1_module_init);
module_exit(gp_csi1_module_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP CSI driver");
MODULE_LICENSE_GP;
