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
#include <mach/module.h>
#include <mach/kernel.h>
#include <mach/diag.h>
#include <mach/cdev.h>
#include <linux/cdev.h>
#include <linux/delay.h>

#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

#include <mach/gp_chunkmem.h>
#include <mach/gp_cache.h>
#include <mach/hal/hal_cdsp.h>
#include <mach/hal/hal_front.h>
#include <mach/hal/hal_clock.h>
#include <mach/gp_cdsp.h>
#include <mach/gp_mipi.h>
#include <mach/gp_board.h>
#include "mach/gp_gpio.h"
#include <mach/clk/gp_clk_core.h>
#include <mach/sensor_mgr.h>
#include <mach/aeawb.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define C_BUFFER_MAX		12
#define C_DMA_CH			0			//0: auto switch, 1: use DMA A, 2: use DMA B
#define	NO_INPUT			0xFFFFFFFF
#define USBPHY_CLK			96000000
#define CDSP_CHK_MIPI_EN	0
#define SKIP_CNT			3

#define _SET_YUV_PATH_		1
#define RAW_BIT				10
#define RAW_MODE			0x01

#define CDSP_AE_CTRL_EN		0x01
#define CDSP_AWB_CTRL_EN	0x02
#define CDSP_AF_CTRL_EN		0x04
#define CDSP_HIST_CTRL_EN	0x08
#define CDSP_LUM_STATUS		0x10
#define CDSP_LOW_LUM		0x20
#define CDSP_HIGH_LUM		0x40
#define CDSP_AWB_SET_GAIN	0x80
#define CDSP_SAT_SWITCH		0x100
#define CDSP_AE_UPDATE		0x200


#define FRAME_SET 			0
#define FRAME_UNSET 		(-1)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define RETURN(x)	{nRet = x; goto __return;}

#define DERROR 	printk
#if 1
#define DEBUG	printk
#else
#define DEBUG(...)
#endif

#define ABS(x)          ((x) < (0) ? -(x) : (x))
#define MK_GPIO_INDEX(ch, func, gid, pin) ((ch<<24) | (func<<16) | (gid<<8) | pin)
#define ReadX(addr)		(*(volatile unsigned *)(addr))

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gpCdspDev_s
{
	struct miscdevice dev;
	struct semaphore sem;
	wait_queue_head_t cdsp_wait_queue;
	bool cdsp_feint_flag;
	int cdsp_eof_cnt;

	unsigned char OpenCnt;	/* open count */
	unsigned char RunFlag;	/* Cdsp running flag */
	unsigned char SyncFlag;	/* Cdsp Sync Flag */
	unsigned char MclkEn;	/* mclk enable flag */
	
	gpCsiMclk_t  Mclk;		/* mclk clock */
	unsigned int CapCnt;	/* capture count */
	unsigned int SkipCnt;	/* skip count */
	unsigned int TotalCnt;	/* total frame count */

	unsigned char abBufFlag;/* auto dma buffer flag */
	unsigned char aInQueIdx;/* auto dma a index */
	unsigned char bInQueIdx;/* auto dma b index */
	unsigned char sensor_status;
	int gpio_handle;
	
	/*Sub device*/
	unsigned int sdidx;	 
	struct v4l2_subdev *sd;
	callbackfunc_t *cb_func;
	char *port;

	/*buffer control*/
	unsigned int bfidx;
	unsigned int bfaddr[C_BUFFER_MAX];
	unsigned char in_que[C_BUFFER_MAX];
	unsigned char out_que[C_BUFFER_MAX];
	struct v4l2_buffer bf[C_BUFFER_MAX];
	struct v4l2_requestbuffers rbuf;

	/* AE buffer */
	unsigned char *aeAddr[2];

	/* format set */	
	unsigned int imgSrc; /* image input source, sdram, front, mipi */
	unsigned int inFmt; /* input format */
	unsigned int rawFmtFlag; /* raw Format Flag */
	unsigned int rawFmt; /* raw Format Flag */
	unsigned short imgWidth; /* image/csi h size */
	unsigned short imgHeight; /* image/csi v size */
	unsigned short imgRbWidth; /* output buffer h size */
	unsigned short imgRbHeight; /* output buffer v size */

	/* cdsp module set */	
	gpCdspScalePara_t scale; /* scale down set */
	gpCdspSuppression_t suppression; /* suppression mode */	
	gpCdsp3aResult_t result;

	sensor_calibration_t	sensor_cdsp;

	/* cdsp 2A */
	wait_queue_head_t ae_awb_wait_queue;
	struct task_struct *ae_awb_task;
	unsigned int ae_awb_flag;
	int color_temp_init_val;
	int sensor_ev_idx;
	int ae_target, ae_target_night;

	//spinlock_t cdsp_splock;
	struct semaphore aeawb_sem;

	int getSensorInfo;
	int sensor_gain_thr, sensor_time_thr;
	int night_gain_thr;

	int wb_offset_night, wb_offset_day;
	int edge_day, edge_night;

	int intpl_hi_thr, intpl_low_thr;

	int sat_yuv_level[4][6];
	int sat_yuv_thr[4];
	int edge_level[4];
	int sat_contr_idx;	
	uv_divide_t UVDivide;

	sensor_exposure_t sInfo;
	
	unsigned char *ae_workmem, *awb_workmem;

	AWB_MODE	awbmode;
	
}gpCdspDev_t;



/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int gp_ae_awb_process(void *arg);
static void sensor_set_exp_do_tasklet(unsigned long); 


//DECLARE_TASKLET(sensor_set_exp_tasklet, sensor_set_exp_do_tasklet, 0); 
DECLARE_TASKLET_DISABLED(sensor_set_exp_tasklet, sensor_set_exp_do_tasklet, 0); 
//static struct tasklet_struct *sensor_set_exp;


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
const unsigned short g_lenscmp_table[] =
{
	0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,0x108,0x109,0x10A,0x10B,0x10C,0x10D,0x10E,0x10F,
	0x110,0x111,0x112,0x113,0x114,0x115,0x116,0x117,0x118,0x119,0x11A,0x11B,0x11C,0x11D,0x11E,0x11F,
	0x120,0x121,0x122,0x123,0x124,0x125,0x126,0x127,0x128,0x129,0x12A,0x12B,0x12C,0x12D,0x12E,0x12F,
	0x130,0x131,0x132,0x133,0x134,0x135,0x136,0x137,0x138,0x139,0x13A,0x13B,0x13C,0x13D,0x13E,0x13F,
	0x140,0x141,0x142,0x143,0x144,0x145,0x146,0x147,0x148,0x149,0x14A,0x14B,0x14C,0x14D,0x14E,0x14F,
	0x150,0x151,0x152,0x153,0x154,0x155,0x156,0x157,0x158,0x159,0x15A,0x15B,0x15C,0x15D,0x15E,0x15F,
	0x160,0x161,0x162,0x163,0x164,0x165,0x166,0x167,0x168,0x169,0x16A,0x16B,0x16C,0x16D,0x16E,0x16F,
	0x170,0x171,0x172,0x173,0x174,0x175,0x176,0x177,0x178,0x179,0x17A,0x17B,0x17C,0x17D,0x17E,0x17F,
	0x180,0x181,0x182,0x183,0x184,0x185,0x186,0x187,0x188,0x189,0x18A,0x18B,0x18C,0x18D,0x18E,0x18F,
	0x190,0x191,0x192,0x193,0x194,0x195,0x196,0x197,0x198,0x199,0x19A,0x19B,0x19C,0x19D,0x19E,0x19F,
	0x1A0,0x1A1,0x1A2,0x1A3,0x1A4,0x1A5,0x1A6,0x1A7,0x1A8,0x1A9,0x1AA,0x1AB,0x1AC,0x1AD,0x1AE,0x1AF,
	0x1B0,0x1B1,0x1B2,0x1B3,0x1B4,0x1B5,0x1B6,0x1B7,0x1B8,0x1B9,0x1BA,0x1BB,0x1BC,0x1BD,0x1BE,0x1BF,
	0x1C0,0x1C1,0x1C2,0x1C3,0x1C4,0x1C5,0x1C6,0x1C7,0x1C8,0x1C9,0x1CA,0x1CB,0x1CC,0x1CD,0x1CE,0x1CF,
	0x1D0,0x1D1,0x1D2,0x1D3,0x1D4,0x1D5,0x1D6,0x1D7,0x1D8,0x1D9,0x1DA,0x1DB,0x1DC,0x1DD,0x1DE,0x1DF,
	0x1E0,0x1E1,0x1E2,0x1E3,0x1E4,0x1E5,0x1E6,0x1E7,0x1E8,0x1E9,0x1EA,0x1EB,0x1EC,0x1ED,0x1EE,0x1EF,
	0x1F0,0x1F1,0x1F2,0x1F3,0x1F4,0x1F5,0x1F6,0x1F7,0x1F8,0x1F9,0x1FA,0x1FB,0x1FC,0x1FD,0x1FE,0x1FF,
};

const unsigned char g_gamma_table[512] =
{
	0x00,0x50,0x15,0x00,0x06,0x55,0x11,0x00,0x0d,0x45,0x15,0x00,0x13,0x55,0x14,0x00,
	0x1a,0x51,0x14,0x00,0x1f,0x15,0x05,0x00,0x25,0x51,0x11,0x00,0x2a,0x11,0x05,0x00,
	0x2f,0x14,0x11,0x00,0x33,0x11,0x11,0x00,0x37,0x44,0x10,0x00,0x3a,0x41,0x04,0x00,
	0x3e,0x44,0x10,0x00,0x41,0x41,0x04,0x00,0x44,0x11,0x11,0x00,0x48,0x44,0x04,0x00,
	0x4c,0x10,0x11,0x00,0x4f,0x41,0x04,0x00,0x52,0x11,0x04,0x00,0x56,0x10,0x11,0x00,
	0x59,0x41,0x04,0x00,0x5c,0x11,0x04,0x00,0x60,0x10,0x01,0x00,0x63,0x10,0x01,0x00,
	0x66,0x04,0x01,0x00,0x69,0x04,0x01,0x00,0x6c,0x10,0x01,0x00,0x6f,0x10,0x04,0x00,
	0x72,0x10,0x04,0x00,0x74,0x41,0x10,0x00,0x77,0x04,0x01,0x00,0x7a,0x10,0x04,0x00,
	0x7c,0x41,0x10,0x00,0x7f,0x04,0x01,0x00,0x82,0x40,0x10,0x00,0x84,0x04,0x01,0x00,
	0x86,0x41,0x10,0x00,0x89,0x10,0x04,0x00,0x8b,0x01,0x01,0x00,0x8e,0x40,0x10,0x00,
	0x90,0x10,0x04,0x00,0x92,0x04,0x04,0x00,0x94,0x01,0x01,0x00,0x96,0x01,0x01,0x00,
	0x98,0x41,0x00,0x00,0x9b,0x40,0x00,0x00,0x9d,0x40,0x00,0x00,0x9f,0x40,0x00,0x00,
	0xa0,0x01,0x01,0x00,0xa2,0x01,0x01,0x00,0xa4,0x04,0x04,0x00,0xa6,0x04,0x10,0x00,
	0xa8,0x10,0x00,0x00,0xaa,0x00,0x01,0x00,0xab,0x01,0x04,0x00,0xad,0x10,0x00,0x00,
	0xaf,0x40,0x00,0x00,0xb0,0x01,0x04,0x00,0xb2,0x10,0x00,0x00,0xb3,0x01,0x04,0x00,
	0xb5,0x10,0x00,0x00,0xb7,0x00,0x04,0x00,0xb8,0x10,0x00,0x00,0xb9,0x01,0x04,0x00,
	0xbb,0x40,0x00,0x00,0xbc,0x04,0x00,0x00,0xbe,0x00,0x01,0x00,0xbf,0x10,0x00,0x00,
	0xc0,0x01,0x00,0x00,0xc2,0x00,0x04,0x00,0xc3,0x40,0x00,0x00,0xc4,0x04,0x00,0x00,
	0xc5,0x01,0x00,0x00,0xc7,0x00,0x04,0x00,0xc8,0x00,0x01,0x00,0xc9,0x40,0x00,0x00,
	0xca,0x10,0x00,0x00,0xcb,0x04,0x00,0x00,0xcc,0x01,0x00,0x00,0xcd,0x01,0x04,0x00,
	0xcf,0x10,0x00,0x00,0xd1,0x00,0x01,0x00,0xd2,0x00,0x04,0x00,0xd3,0x00,0x04,0x00,
	0xd4,0x00,0x04,0x00,0xd5,0x00,0x01,0x00,0xd6,0x00,0x01,0x00,0xd7,0x00,0x01,0x00,
	0xd8,0x00,0x01,0x00,0xd9,0x00,0x01,0x00,0xda,0x10,0x00,0x00,0xdc,0x00,0x01,0x00,
	0xdd,0x04,0x10,0x00,0xdf,0x00,0x00,0x00,0xe0,0x00,0x00,0x00,0xe1,0x00,0x00,0x00,
	0xe2,0x00,0x01,0x00,0xe3,0x04,0x00,0x00,0xe5,0x00,0x00,0x00,0xe6,0x00,0x00,0x00,
	0xe6,0x04,0x00,0x00,0xe8,0x00,0x04,0x00,0xe9,0x40,0x00,0x00,0xea,0x00,0x01,0x00,
	0xeb,0x00,0x01,0x00,0xec,0x00,0x01,0x00,0xed,0x40,0x00,0x00,0xee,0x00,0x00,0x00,
	0xef,0x00,0x00,0x00,0xef,0x40,0x00,0x00,0xf0,0x04,0x00,0x00,0xf2,0x00,0x04,0x00,
	0xf3,0x00,0x00,0x00,0xf3,0x10,0x00,0x00,0xf4,0x40,0x00,0x00,0xf5,0x40,0x00,0x00,
	0xf6,0x40,0x00,0x00,0xf7,0x00,0x00,0x00,0xf7,0x01,0x00,0x00,0xf8,0x40,0x00,0x00,
	0xf9,0x04,0x00,0x00,0xfb,0x00,0x04,0x00,0xfc,0x00,0x00,0x00,0xfc,0x10,0x00,0x00,
	0xfd,0x00,0x10,0x00,0xfe,0x00,0x00,0x00,0xfe,0x00,0x10,0x00,0xff,0x00,0x00,0x00,
};

const unsigned char g_edge_table[256] =
{
	0x00,0x01,0x02,0x04,0x07,0x0a,0x0e,0x12,0x17,0x1d,0x23,0x29,0x31,0x38,0x41,0x4a,
	0x53,0x5d,0x68,0x73,0x7f,0x8b,0x98,0xa5,0xb3,0xc2,0xd0,0xe0,0xef,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfd,0xfb,0xf8,0xf6,0xf4,0xf1,0xef,0xed,
	0xeb,0xe9,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,0xda,0xd8,0xd6,0xd5,0xd3,0xd1,0xcf,0xcd,
	0xcb,0xca,0xc8,0xc6,0xc5,0xc3,0xc1,0xc0,0xbe,0xbc,0xbb,0xb9,0xb8,0xb6,0xb5,0xb3,
	0xb2,0xb0,0xaf,0xae,0xac,0xab,0xa9,0xa8,0xa7,0xa5,0xa4,0xa3,0xa2,0xa0,0x9f,0x9e
};

static int uv_div[4][6] = 
{
/*	{ 4,  6,  8, 10, 12, 16},
	{ 8, 14, 20, 26, 32, 38},
	{12, 19, 26, 33, 40, 48},
	{16, 24, 32, 40, 48, 56}	*/
	
 /*	{ 4,  8, 12, 16, 20, 24},
	{ 8, 12, 16, 20, 24, 32},
	{12, 16, 20, 24, 32, 40},
	{16, 20, 24, 32, 40, 48}*/

	{ 4,  8, 12, 16, 20, 24},
	{16, 32, 48, 64, 80, 96},
	{32, 64, 96, 128, 160, 192},
	{64, 102, 140, 178, 216, 255}

};

static gpCdspDev_t *CdspDev;
static sensor_config_t *sensor=NULL;
static struct v4l2_capability g_cdsp_cap= {
	.driver = "/dev/cdsp",
	.card = "Generalplus CDSP",
	.bus_info = "CDSP interface",
	.version = 0x020620,
	.capabilities = V4L2_CAP_VIDEO_CAPTURE
};

	
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void 
gpCdspSetMclkout(
	int enable, 
	gpCsiMclk_t *pMclk
)
{
	int speed, div;
	struct clk *clock;

	if(enable == 0) {
		gpHalCdspSetMclk(0, 0, 0, 0);
		return;
	}

	clock = clk_get(NULL, "clk_ref_sys");
	speed = clk_get_rate(clock);
	div = speed / pMclk->mclk_out;
	if((speed % pMclk->mclk_out) == 0) { 
		div--;
	}

	gpHalCdspSetMclk(pMclk->mclk_sel, div, pMclk->pclk_dly, pMclk->pclk_revb);
	DEBUG("SysClock = %d\n", speed);
	DEBUG("MCLK = %d\n", speed/(div + 1));
}

///////////////////////////////////////////////////////////////////////////
// CDSP
///////////////////////////////////////////////////////////////////////////
void
gpCdspClockEnable(
	unsigned int enable
)
{
	if(enable) {
		/*For CDSP write*/
		gpHalCdspSetClk(C_CDSP_CLK_ENABLE, 0);
	} else {
		gpHalCdspSetClk(C_CDSP_CLK_DISABLE, 0);	
	}
}

int
gpCdspSetClkPath(
	int imgSrc,
	int rawFmtFlag
)
{
	switch(imgSrc)
	{
	case C_CDSP_SDRAM:
		if(rawFmtFlag) {
			gpHalCdspSetClk(C_CDSP_CLK_FB, 0);
		} else {
			gpHalCdspSetClk(C_CDSP_CLK_FB, 1);
		}
		break;

	case C_CDSP_FRONT:
		if(rawFmtFlag) {
			gpHalCdspSetClk(C_CDSP_CLK_FRONT, 0);
		} else {
			gpHalCdspSetClk(C_CDSP_CLK_FRONT, 1);
		}
		break;
			
	case C_CDSP_MIPI:
		if(rawFmtFlag) {
			gpHalCdspSetClk(C_CDSP_CLK_MIPI, 0);
		} else {
			gpHalCdspSetClk(C_CDSP_CLK_MIPI, 1);
		}
		break;
		
	default:
		gpHalCdspSetClk(C_CDSP_CLK_DISABLE, 0);
		return -1;
	}
	return 0;
}

unsigned int
gpCdspSetFifoSize(
	unsigned short width
)
{
#if 0 
	if(width >= 1280) {
		width = 0x100;
	} else {
		while(width > 0x100) {
			width >>= 1;
		}
	}
#else
	width = 0x100;
#endif	

	DEBUG(KERN_WARNING "fifo=0x%x.\n", width);
	gpHalCdspSetSRAM(ENABLE, width);
	gpHalCdspSetLineInterval(0x28);
	return width;
}

void
gpCdspSetBadPixOb(
	gpCdspBadPixOB_t *argp
)
{
	/* bad pixel */
	DEBUG("%s = %d\n", __FUNCTION__, argp->badpixen);
	if(argp->badpixen) {
		gpHalCdspSetBadPixel(argp->bprthr, argp->bpgthr, argp->bpbthr);
		gpHalCdspSetBadPixelEn(argp->badpixen, argp->badpixmirr); 
	} else {
		gpHalCdspSetBadPixelEn(DISABLE, 0x0);
	}

	/* optical black */
	if(argp->wboffseten == ENABLE)
	{
		gpHalCdspSetWbOffset(argp->roffset, argp->groffset, argp->boffset, argp->gboffset);
		gpHalCdspSetWbOffsetEn(argp->wboffseten);
		gpHalCdspSetManuOB(argp->manuoben, argp->manuob);
		gpHalCdspSetAutoOB(argp->autooben, argp->obtype, argp->obHOffset, argp->obVOffset);
	}
	else {
		gpHalCdspSetWbOffsetEn(DISABLE);
		gpHalCdspSetManuOB(DISABLE, argp->manuob);
		gpHalCdspSetAutoOB(DISABLE, argp->obtype, argp->obHOffset, argp->obVOffset);
	}
}

void
gpCdspGetBadPixOb(
	gpCdspBadPixOB_t *argp
)
{
	/* bad pixel */
	gpHalCdspGetBadPixelEn(&argp->badpixen, &argp->badpixmirr);
	gpHalCdspGetBadPixel(&argp->bprthr, &argp->bpgthr, &argp->bpbthr);

	/* optical black */
	argp->wboffseten = gpHalCdspGetWbOffset(&argp->roffset, &argp->groffset, &argp->boffset, &argp->gboffset);
	gpHalCdspGetManuOB(&argp->manuoben, &argp->manuob);
	gpHalCdspGetAutoOB(&argp->autooben, &argp->obtype, &argp->obHOffset, &argp->obVOffset);
	gpHalCdspGetAutoOBAvg(&argp->Ravg, &argp->GRavg, &argp->Bavg, &argp->GBavg);
}

void
gpCdspSetLensCmp(
	unsigned char raw_flag,
	gpCdspLenCmp_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->lcen);
	if(argp->lcen) {
		if(argp->lenscmp_table) {
			gpHalCdspInitLensCmp(argp->lenscmp_table);
		} else {
			gpHalCdspInitLensCmp((unsigned short *)g_lenscmp_table);
		}
		
		gpHalCdspSetLensCmpPos(argp->centx, argp->centy, argp->xmoffset, argp->ymoffset);
		gpHalCdspSetLensCmp(argp->stepfactor, argp->xminc, argp->ymoinc, argp->ymeinc);
		gpHalCdspSetLensCmpEn(argp->lcen);
	} else {
		gpHalCdspSetLensCmpEn(DISABLE);
	}

	if(raw_flag) {
		gpHalCdspSetLensCmpPath(0);
	} else {
		gpHalCdspSetLensCmpPath(1);
	}
}

void
gpCdspGetLensCmp(
	gpCdspLenCmp_t *argp
)
{
	gpHalCdspGetLensCmpPos(&argp->centx, &argp->centy, &argp->xmoffset, &argp->ymoffset);
	gpHalCdspGetLensCmp(&argp->stepfactor, &argp->xminc, &argp->ymoinc, &argp->ymeinc);
	argp->lcen = gpHalCdspGetLensCmpEn();
	argp->lenscmp_table = NULL;
}

void
gpCdspSetWhiteBalance(
	gpCdspWhtBal_t *argp
)
{
	//DEBUG("%s = %d\n", __FUNCTION__, argp->wbgainen);	
	gpHalCdspSetWbGain(argp->rgain, argp->grgain, argp->bgain, argp->gbgain);
	gpHalCdspSetWbGainEn(argp->wbgainen); 
	if(argp->wbgainen) {
		gpHalCdspSetGlobalGain(argp->global_gain);
	}
}

void
gpCdspGetWhiteBalance(
	gpCdspWhtBal_t *argp
)
{
	argp->wbgainen = gpHalCdspGetWbGain(&argp->rgain, &argp->grgain, &argp->bgain, &argp->gbgain);
	argp->global_gain =	gpHalCdspGetGlobalGain();
}

void
gpCdspSetLutGamma(
	gpCdspGamma_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->lut_gamma_en);
	if(argp->lut_gamma_en)
	{
		if(argp->gamma_table) {
			gpHalCdspInitGamma(argp->gamma_table);
		} else {
			gpHalCdspInitGamma((unsigned int *)g_gamma_table);
		}
		gpHalCdspSetLutGammaEn(argp->lut_gamma_en);
	}
	else
	{
		gpHalCdspSetLutGammaEn(DISABLE);
	}
}

void
gpCdspGetLutGamma(
	gpCdspGamma_t *argp
)
{
	argp->lut_gamma_en = gpHalCdspGetLutGammaEn();
	argp->gamma_table = NULL;
}

void
gpCdspSetIntpl(
	unsigned short width,
	gpCdspIntpl_t *argp
)
{
#if 1
	/* down mirror enable */
	gpHalCdspSetIntplMirEn(0xF, 1, 0);
	/* down mirror, need enable extline */
	gpHalCdspSetLineCtrl(0);
	gpHalCdspSetExtLine(width, 0x280);
	gpHalCdspSetExtLinePath(1, 0);
#else
	/* down mirror disable */
	gpHalCdspSetIntplMirEn(0x7, 0, 0);
	gpHalCdspSetLineCtrl(0);
	gpHalCdspSetExtLine(width, 0x280);
	gpHalCdspSetExtLinePath(0, 0);
#endif
	gpHalCdspSetIntplThr(argp->int_low_thr, argp->int_hi_thr);
	gpHalCdspSetRawSpecMode(argp->rawspecmode);

	/* disbale suppression */
	gpHalCdspSetUvSupprEn(DISABLE);
}

void
gpCdspGetIntpl(
	gpCdspIntpl_t *argp
)
{
	gpHalCdspGetIntplThr(&argp->int_low_thr, &argp->int_hi_thr);
	argp->rawspecmode = gpHalCdspGetRawSpecMode();
}

void
gpCdspSetEdge(
	unsigned char raw_flag,
	gpCdspEdge_t *argp
)
{
	//DEBUG("%s = %d\n", __FUNCTION__, argp->edgeen);
	if(argp->edgeen) {
		if(argp->edge_table) {
			gpHalCdspInitEdgeLut(argp->edge_table);
		} else {
			gpHalCdspInitEdgeLut((unsigned char *)g_edge_table);
		}
		
		gpHalCdspSetEdgeLutTableEn(argp->eluten);
		gpHalCdspSetEdgeEn(argp->edgeen);
		gpHalCdspSetEdgeLCoring(argp->lhdiv, argp->lhtdiv, argp->lhcoring, argp->lhmode);
		gpHalCdspSetEdgeAmpga(argp->ampga);
		gpHalCdspSetEdgeDomain(argp->edgedomain);
		gpHalCdspSetEdgeQthr(argp->Qthr);

		if(raw_flag) {
			gpHalCdspSetEdgeSrc(0);
		} else {
			gpHalCdspSetEdgeSrc(1);
		}
		
		/*3x3 programing matrix */
		if(argp->lhmode == 0) {
			edge_filter_t LPF;

			LPF.LPF00 = argp->lf00;
			LPF.LPF01 = argp->lf01;
			LPF.LPF02 = argp->lf02;
			
			LPF.LPF10 = argp->lf10;
			LPF.LPF11 = argp->lf11;
			LPF.LPF12 = argp->lf12;

			LPF.LPF20 = argp->lf20;
			LPF.LPF21 = argp->lf21;
			LPF.LPF22 = argp->lf22;
			gpHalCdspSetEdgeFilter(&LPF);
		}
	} else {
		gpHalCdspSetEdgeEn(DISABLE);
		gpHalCdspSetEdgeLutTableEn(DISABLE);
	}
}

void
gpCdspGetEdge(
	gpCdspEdge_t *argp
)
{
	edge_filter_t LPF;
	
	argp->eluten = gpHalCdspGetEdgeLutTableEn();
	argp->edge_table = NULL;
	argp->edgeen = gpHalCdspGetEdgeEn();
	gpHalCdspGetEdgeLCoring(&argp->lhdiv, &argp->lhtdiv, &argp->lhcoring, &argp->lhmode);
	argp->ampga = gpHalCdspGetEdgeAmpga();
	argp->edgedomain = gpHalCdspGetEdgeDomain();
	argp->Qthr = gpHalCdspGetEdgeQCnt();

	gpHalCdspGetEdgeFilter(&LPF);
	argp->lf00 = LPF.LPF00;
	argp->lf01 = LPF.LPF01;
	argp->lf02 = LPF.LPF02;

	argp->lf10 = LPF.LPF10;
	argp->lf11 = LPF.LPF11;
	argp->lf12 = LPF.LPF12;

	argp->lf20 = LPF.LPF20;
	argp->lf21 = LPF.LPF21;
	argp->lf22 = LPF.LPF22;
}

void
gpCdspSetColorMatrix(
	gpCdspCorMatrix_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->colcorren);
	if(argp->colcorren) {
		color_matrix_t CM;

		CM.CcCof00 = argp->a11;
		CM.CcCof01 = argp->a12;
		CM.CcCof02 = argp->a13;

		CM.CcCof10 = argp->a21;
		CM.CcCof11 = argp->a22;
		CM.CcCof12 = argp->a23;

		CM.CcCof20 = argp->a31;
		CM.CcCof21 = argp->a32;
		CM.CcCof22 = argp->a33;
		gpHalCdspSetColorMatrix(&CM);
		gpHalCdspSetColorMatrixEn(argp->colcorren);
	} else {
		gpHalCdspSetColorMatrixEn(DISABLE);
	}
}

void
gpCdspGetColorMatrix(
	gpCdspCorMatrix_t *argp
)
{
	color_matrix_t CM;

	gpHalCdspGetColorMatrix(&CM);
	argp->a11 = CM.CcCof00;
	argp->a12 = CM.CcCof01;
	argp->a13 = CM.CcCof02;

	argp->a21 = CM.CcCof10;
	argp->a22 = CM.CcCof11;
	argp->a23 = CM.CcCof12;
	
	argp->a31 = CM.CcCof20;
	argp->a32 = CM.CcCof21;
	argp->a33 = CM.CcCof22;
	argp->colcorren = gpHalCdspGetColorMatrixEn();
}

void
gpCdspSetRgbToYuv(
	gpCdspRgb2Yuv_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->uvdiven);
	gpHalCdspSetPreRBClamp(argp->pre_rbclamp);
	gpHalCdspSetRBClamp(argp->rbclampen, argp->rbclamp);
	if(argp->uvdiven) {
		uv_divide_t UVDivide;

		UVDivide.YT1 = argp->Yvalue_T1;
		UVDivide.YT2 = argp->Yvalue_T2;
		UVDivide.YT3 = argp->Yvalue_T3;
		UVDivide.YT4 = argp->Yvalue_T4;
		UVDivide.YT5 = argp->Yvalue_T5;
		UVDivide.YT6 = argp->Yvalue_T6;
		gpHalCdspSetUvDivide(&UVDivide);
		gpHalCdspSetUvDivideEn(argp->uvdiven);
	} else {
		gpHalCdspSetUvDivideEn(DISABLE);
	}
}

void
gpCdspGetRgbToYuv(
	gpCdspRgb2Yuv_t *argp
)
{
	uv_divide_t UVDivide;
	
	argp->pre_rbclamp = gpHalCdspGetPreRBClamp();
	gpHalCdspGetRBClamp(&argp->rbclampen, (unsigned char *)&argp->rbclamp);
	argp->uvdiven = gpHalCdspGetUvDivideEn();
	gpHalCdspGetUvDivide(&UVDivide);
	argp->Yvalue_T1 = UVDivide.YT1;
	argp->Yvalue_T2 = UVDivide.YT2;
	argp->Yvalue_T3 = UVDivide.YT3;
	argp->Yvalue_T4 = UVDivide.YT4;
	argp->Yvalue_T5 = UVDivide.YT5;
	argp->Yvalue_T6 = UVDivide.YT6;
}

void
gpCdspSetYuv444Insert(
	gpCdspYuvInsert_t *argp
)
{
	unsigned char y_value, u_value, v_value;

	DEBUG("%s = %d\n", __FUNCTION__, argp->yuv444_insert);
	y_value = ((argp->y_corval & 0x0F) << 4)|(argp->y_coring & 0xF);
	u_value = ((argp->u_corval & 0x0F) << 4)|(argp->u_coring & 0xF);
	v_value = ((argp->v_corval & 0x0F) << 4)|(argp->v_coring & 0xF);
	gpHalCdspSetYuv444InsertEn(argp->yuv444_insert);
	gpHalCdspSetYuvCoring(y_value, u_value, v_value);
}

void
gpCdspGetYuv444Insert(
	gpCdspYuvInsert_t *argp
)
{
	unsigned char y_value, u_value, v_value;

	argp->yuv444_insert = gpHalCdspGetYuv444InsertEn();
	gpHalCdspGetYuvCoring(&y_value, &u_value, &v_value);
	argp->y_coring = y_value & 0x0F;
	argp->y_corval = (y_value >> 4) & 0x0F;
	argp->u_coring = v_value & 0x0F;
	argp->u_corval = (v_value >> 4) & 0x0F;
	argp->v_coring = v_value & 0x0F;
	argp->v_corval = (v_value >> 4) & 0x0F;
}

void
gpCdspSetYuvHavg(
	gpCdspYuvHAvg_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, 0x03);
	gpHalCdspSetYuvHAvg(0x03, argp->ytype, argp->utype, argp->vtype);
}

void
gpCdspGetYuvHavg(
	gpCdspYuvHAvg_t *argp
)
{
	gpHalCdspGetYuvHAvg(&argp->reserved, &argp->ytype, &argp->utype, &argp->vtype);
}

void
gpCdspSetSpecialMode(
	gpCdspSpecMod_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->yuvspecmode);
	gpHalCdspSetYuvSpecModeBinThr(argp->binarthr);
	gpHalCdspSetYuvSpecMode(argp->yuvspecmode);
}

void
gpCdspGetSpecialMode(
	gpCdspSpecMod_t *argp
)
{
	argp->binarthr = gpHalCdspGetYuvSpecModeBinThr();
	argp->yuvspecmode = gpHalCdspGetYuvSpecMode();
}

void
gpCdspSetSatHue(
	gpCdspSatHue_t *argp
)
{
	//DEBUG("%s = %d\n", __FUNCTION__, argp->YbYcEn);
	gpHalCdspSetYuvSPHue(argp->u_huesindata, argp->u_huecosdata, argp->v_huesindata, argp->v_huecosdata);
	gpHalCdspSetYuvSPEffOffset(argp->y_offset, argp->u_offset, argp->v_offset);
	gpHalCdspSetYuvSPEffScale(argp->y_scale, argp->u_scale, argp->v_scale);
	gpHalCdspSetBriContEn(argp->YbYcEn);
}

void
gpCdspGetSatHue(
	gpCdspSatHue_t *argp
)
{
	gpHalCdspGetYuvSPHue(&argp->u_huesindata, &argp->u_huecosdata, &argp->v_huesindata, &argp->v_huecosdata);
	gpHalCdspGetYuvSPEffOffset(&argp->y_offset, &argp->u_offset, &argp->v_offset);
	gpHalCdspGetYuvSPEffScale(&argp->y_scale, &argp->u_scale, &argp->v_scale);
	argp->YbYcEn = gpHalCdspGetBriContEn();
}

void
gpCdspSetSuppression(
	unsigned short width,
	gpCdspSuppression_t *argp,
	gpCdspEdge_t *edge
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->suppressen);
	if(argp->suppressen) {
	#if 1
		/* down mirror enable */
		gpHalCdspSetUvSuppr(1, 1, 0xF);
		/* use down mirror must enable extline */
		gpHalCdspSetLineCtrl(1);
		gpHalCdspSetExtLine(width, 0x280);
		gpHalCdspSetExtLinePath(1, 1);
	#else
		/* down mirror disable */
		gpHalCdspSetUvSuppr(1, 1, 0xD);
		gpHalCdspSetLineCtrl(1);
		gpHalCdspSetExtLine(width, 0x280);
		gpHalCdspSetExtLinePath(0, 0);
	#endif
		gpHalCdspSetUvSupprEn(ENABLE);
		if(argp->suppr_mode >  2)
			argp->suppr_mode = 2;

		switch(argp->suppr_mode)
		{
		case 0:
			gpHalCdspSetYDenoiseEn(DISABLE);
			gpHalCdspSetYLPFEn(DISABLE);
			gpCdspSetEdge(0, edge);
			break;
		case 1:
			DEBUG("%s: enable denoise\r\n", __FUNCTION__);
			gpHalCdspSetEdgeEn(DISABLE);
			gpHalCdspSetEdgeLutTableEn(DISABLE);
			gpHalCdspSetYLPFEn(DISABLE);
			gpHalCdspSetYDenoise(argp->denoisethrl, argp->denoisethrwth, argp->yhtdiv);
			gpHalCdspSetYDenoiseEn(argp->denoisen);
			break;
		case 2:
			gpHalCdspSetEdgeEn(DISABLE);
			gpHalCdspSetEdgeLutTableEn(DISABLE);
			gpHalCdspSetYDenoiseEn(DISABLE);
			gpHalCdspSetYLPFEn(argp->lowyen);
			break;
		}
	} else {
		/* dow mirror disable */
		gpHalCdspSetUvSuppr(0, 0, 0);
		gpHalCdspSetLineCtrl(0);
		gpHalCdspSetExtLine(width, 0x280);
		gpHalCdspSetExtLinePath(0, 0);

		gpHalCdspSetUvSupprEn(DISABLE);
		gpHalCdspSetEdgeEn(DISABLE);
		gpHalCdspSetEdgeLutTableEn(DISABLE);
		gpHalCdspSetYDenoiseEn(DISABLE);
		gpHalCdspSetYLPFEn(DISABLE);
	}
}

void
gpCdspGetSuppression(
	gpCdspSuppression_t *argp
)
{
	gpHalCdspGetYDenoise(&argp->denoisethrl, &argp->denoisethrwth, &argp->yhtdiv);
	argp->denoisen = gpHalCdspGetYDenoiseEn();
	argp->lowyen = gpHalCdspGetYLPFEn();
}

void
gpCdspSetNewDenoise(
	gpCdspNewDenoise_t *argp
)
{
	//DEBUG("\n\n==== %s: newdenoiseen = %d ====\n", __FUNCTION__, argp->newdenoiseen);
	
	if(argp->newdenoiseen) {
	/*	DEBUG("ndmirvsel= %d, ndmiren = %d\n", argp->ndmirvsel, argp->ndmiren);
		DEBUG("ndlhdiv= %d, ndlhtdiv= %d, ndlhcoring = %d, ndlhmode = %d\n", argp->ndlhdiv, argp->ndlhtdiv, argp->ndlhcoring, argp->ndlhmode);
		DEBUG("ndampga= %d, ndeluten = %d, ndedge_table = %d\n", argp->ndampga, argp->ndeluten, argp->ndedge_table);
		DEBUG("filter0: %d, %d, %d\n", argp->ndlf00, argp->ndlf01, argp->ndlf02);
		DEBUG("filter1: %d, %d, %d\n", argp->ndlf10, argp->ndlf11, argp->ndlf12);
		DEBUG("filter2: %d, %d, %d\n", argp->ndlf20, argp->ndlf21, argp->ndlf22);*/
		
		gpHalCdspSetNewDenoiseEn(argp->newdenoiseen);
		gpHalCdspSetNewDenoise(argp->ndmirvsel, argp->ndmiren);
		gpHalCdspSetNdEdgeEn(argp->ndedgeen, argp->ndeluten);
		gpHalCdspSetNdEdgeLCoring(argp->ndlhdiv, argp->ndlhtdiv, argp->ndlhcoring, argp->ndlhmode);
		gpHalCdspSetNdEdgeAmpga(argp->ndampga);
		if(argp->ndeluten) {
			if(argp->ndedge_table) {
				gpHalCdspInitEdgeLut(argp->ndedge_table);
			} else {
				gpHalCdspInitEdgeLut((unsigned char *)g_edge_table);
			}
		}

		/*3x3 programing matrix */
		if(argp->ndlhmode == 0) {
			edge_filter_t NDEdgeFilter;

			NDEdgeFilter.LPF00 = argp->ndlf00;
			NDEdgeFilter.LPF01 = argp->ndlf01;
			NDEdgeFilter.LPF02 = argp->ndlf02;
			
			NDEdgeFilter.LPF10 = argp->ndlf10;
			NDEdgeFilter.LPF11 = argp->ndlf11;
			NDEdgeFilter.LPF12 = argp->ndlf12;

			NDEdgeFilter.LPF20 = argp->ndlf20;
			NDEdgeFilter.LPF21 = argp->ndlf21;
			NDEdgeFilter.LPF22 = argp->ndlf22;
			gpHalCdspSetNdEdgeFilter(&NDEdgeFilter);
		}
	} else {
		gpHalCdspSetNewDenoiseEn(DISABLE);
		gpHalCdspSetNdEdgeEn(DISABLE, DISABLE);
	}
}

void
gpCdspGetNewDenoise(
	gpCdspNewDenoise_t *argp
)
{
	edge_filter_t NDEdgeFilter;
	
	argp->newdenoiseen = gpHalCdspGetNewDenoiseEn();
	gpHalCdspGetNewDenoise(&argp->ndmirvsel, &argp->ndmiren);
	gpHalCdspGetNdEdgeEn(&argp->ndedgeen, &argp->ndeluten);
	//DEBUG("%s: ndedgeen = %d, ndeluten = %d\n", __FUNCTION__, argp->ndedgeen, argp->ndeluten);
	
	gpHalCdspGetNdEdgeLCoring(&argp->ndlhdiv, &argp->ndlhtdiv, &argp->ndlhcoring, &argp->ndlhmode);
	argp->ndampga =	gpHalCdspGetNdEdgeAmpga();
		
	gpHalCdspGetNdEdgeFilter(&NDEdgeFilter);
	argp->ndlf00 = NDEdgeFilter.LPF00;
	argp->ndlf01 = NDEdgeFilter.LPF01;
	argp->ndlf02 = NDEdgeFilter.LPF02;
	
	argp->ndlf10 = NDEdgeFilter.LPF10;
	argp->ndlf11 = NDEdgeFilter.LPF11;
	argp->ndlf12 = NDEdgeFilter.LPF12;

	argp->ndlf20 = NDEdgeFilter.LPF20;
	argp->ndlf21 = NDEdgeFilter.LPF21;
	argp->ndlf22 = NDEdgeFilter.LPF22;
}

void
gpCdspSetRawWin(
	unsigned short width,
	unsigned short height,
	gpCdspRawWin_t *argp
)
{
	unsigned int x, y;

	//width -= 8;
	//height -= 8;
	if(argp->hwdoffset == 0) {
		argp->hwdoffset = 1;
	}
	
	if(argp->vwdoffset == 0) {
		argp->vwdoffset = 1;
	}
		
	if(argp->aeawb_src == 0) {
		x = argp->hwdoffset + argp->hwdsize * 8;
		y = argp->vwdoffset + argp->vwdsize * 8;
		if(x >= width) {
			x = width - argp->hwdoffset;
			argp->hwdsize = x / 8;
		}

		if(y >= height) {
			y = height - argp->vwdoffset;
			argp->vwdsize = y / 8;
		}
	} else {
		x = argp->hwdoffset*2 + argp->hwdsize*2 * 8;
		y = argp->vwdoffset*2 + argp->vwdsize*2 * 8;
		if(x >= width) {
			x = width - argp->hwdoffset*2;
			argp->hwdsize = x / 8;
			argp->hwdsize >>= 1;
		}

		if(y >= height) {
			y = height - argp->vwdoffset*2;
			argp->vwdsize = y / 8;
			argp->vwdsize >>= 1;
		}
	}

	DEBUG("AeWinTest = %d\n", argp->AeWinTest);
	DEBUG(KERN_WARNING "RawWinOffset[%d,%d]\n", argp->hwdoffset, argp->vwdoffset);
	DEBUG(KERN_WARNING "RawWinCellSize[%d,%d]\n", argp->hwdsize, argp->vwdsize);
	
	gpHalCdspSetAeAwbSrc(argp->aeawb_src);
	gpHalCdspSetAeAwbSubSample(argp->subsample);
	gpHalCdspSetRGBWin(argp->hwdoffset, argp->vwdoffset, argp->hwdsize, argp->vwdsize);
	gpHalCdspSet3ATestWinEn(argp->AeWinTest, argp->AfWinTest);
}

void
gpCdspGetRawWin(
	gpCdspRawWin_t *argp
)
{
	argp->aeawb_src = gpHalCdspGetAeAwbSrc();
	argp->subsample = gpHalCdspGetAeAwbSubSample();
	gpHalCdspGet3ATestWinEn(&argp->AeWinTest, &argp->AfWinTest);
	gpHalCdspGetRGBWin(&argp->hwdoffset, &argp->vwdoffset, &argp->hwdsize, &argp->vwdsize);
}

void
gpCdspSetAF(
	unsigned short width,
	unsigned short height,
	gpCdspAF_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->af_win_en);
	if(argp->af_win_en) {
		unsigned int x, y;
		/* af1 */
		if(argp->af1_hsize >= width) {
			argp->af1_hsize = width;
		}

		if(argp->af1_vsize >= height) {
			argp->af1_hsize = height;
		}
		
		x = argp->af1_hoffset + argp->af1_hsize;
		y = argp->af1_voffset + argp->af1_vsize;
		if(x >= width) {
			argp->af1_hoffset = width - 1 - argp->af1_hsize;
		}
		
		if(y >= height) {
			argp->af1_voffset = height - 1 - argp->af1_vsize;
		}
		
		/* af2 */
		if(argp->af2_hsize >= width) {
			argp->af2_hsize = width;
		}

		if(argp->af2_vsize >= height) {
			argp->af2_hsize = height;
		}

		if(argp->af2_hsize < 64) {
			argp->af2_hsize = 64;
		}
		
		if(argp->af2_vsize < 64) {
			argp->af2_vsize = 64;
		}
		
		x = argp->af2_hoffset + argp->af2_hsize;
		y = argp->af2_voffset + argp->af2_vsize;
		if(x >= width) {
			argp->af2_hoffset = width - 1 - argp->af2_hsize;
		}

		if(y >= height) {
			argp->af2_voffset = height - 1 - argp->af2_vsize;
		}
		
		/* af3 */
		if(argp->af3_hsize >= width) {
			argp->af3_hsize = width;
		}
		
		if(argp->af3_vsize >= height) {
			argp->af3_hsize = height;
		}

		if(argp->af3_hsize < 64) {
			argp->af3_hsize = 64;
		}

		if(argp->af3_vsize < 64) {
			argp->af3_vsize = 64;
		}
		
		x = argp->af3_hoffset + argp->af3_hsize;
		y = argp->af3_voffset + argp->af3_vsize;
		if(x >= width) {
			argp->af3_hoffset = width - 1 - argp->af3_hsize;
		}

		if(y >= height) {
			argp->af3_voffset = height - 1 - argp->af3_vsize;
		}

		DEBUG(KERN_WARNING "Af1Offset[%d,%d]\n", argp->af1_hoffset, argp->af1_voffset);
		DEBUG(KERN_WARNING "Af1Size[%d,%d]\n", argp->af1_hsize, argp->af1_vsize);
		DEBUG(KERN_WARNING "Af2Offset[%d,%d]\n", argp->af2_hoffset, argp->af2_voffset);
		DEBUG(KERN_WARNING "Af2Size[%d,%d]\n", argp->af2_hsize, argp->af2_vsize);
		DEBUG(KERN_WARNING "Af3Offset[%d,%d]\n", argp->af3_hoffset, argp->af3_voffset);
		DEBUG(KERN_WARNING "Af3Size[%d,%d]\n", argp->af3_hsize, argp->af3_vsize);
		
		gpHalCdspSetAfWin1(argp->af1_hoffset, argp->af1_voffset, argp->af1_hsize, argp->af1_hsize);
		gpHalCdspSetAfWin2(argp->af2_hoffset, argp->af2_voffset, argp->af2_hsize, argp->af2_hsize);
		gpHalCdspSetAfWin3(argp->af3_hoffset, argp->af3_voffset, argp->af3_hsize, argp->af3_hsize);
		gpHalCdspSetAFEn(argp->af_win_en, argp->af_win_hold);
		gpHalCdspSetIntEn(ENABLE, CDSP_AFWIN_UPDATE);
	} else {
		gpHalCdspSetAFEn(DISABLE, DISABLE);
		gpHalCdspSetIntEn(DISABLE, CDSP_AFWIN_UPDATE);
	}
}

void
gpCdspGetAF(
	gpCdspAF_t *argp
)
{
	gpHalCdspGetAfWin1(&argp->af1_hoffset, &argp->af1_voffset, &argp->af1_hsize, &argp->af1_hsize);
	gpHalCdspGetAfWin2(&argp->af2_hoffset, &argp->af2_voffset, &argp->af2_hsize, &argp->af2_hsize);
	gpHalCdspGetAfWin3(&argp->af3_hoffset, &argp->af3_voffset, &argp->af3_hsize, &argp->af3_hsize);
	gpHalCdspGetAFEn(&argp->af_win_en, &argp->af_win_hold);
}

void
gpCdspSetAE(
	gpCdspAE_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->ae_win_en);
	if(argp->ae_win_en) {
		unsigned int ae0, ae1;
		
		ae0 = (unsigned int)gp_chunk_pa(CdspDev->aeAddr[0]);
		ae1 = (unsigned int)gp_chunk_pa(CdspDev->aeAddr[1]);
		DEBUG(KERN_WARNING "AeWinAddr0 = 0x%x.\n", ae0);
		DEBUG(KERN_WARNING "AeWinAddr1 = 0x%x.\n", ae1);

		gpHalCdspSetAEBuffAddr(ae0, ae1);
		gpHalCdspSetAEEn(argp->ae_win_en, argp->ae_win_hold);
		gpHalCdspSetAEWin(argp->phaccfactor, argp->pvaccfactor);
		gpHalCdspSetIntEn(ENABLE, CDSP_AEWIN_SEND);
	} else {
		gpHalCdspSetAEEn(DISABLE, DISABLE);
		gpHalCdspSetIntEn(DISABLE, CDSP_AEWIN_SEND);
	}
}

void
gpCdspGetAE(
	gpCdspAE_t *argp
)
{
	gpHalCdspGetAEEn(&argp->ae_win_en, &argp->ae_win_hold);
	gpHalCdspGetAEWin(&argp->phaccfactor, &argp->pvaccfactor);
}

void
gpCdspSetAWB(
	gpCdspAWB_t *argp
)
{
	DEBUG("%s = %d\n", __FUNCTION__, argp->awb_win_en);
	if(argp->awb_win_en) {
		awb_uv_thr_t UVthr;

		UVthr.UL1N1 = argp->UL1N1;
		UVthr.UL1P1 = argp->UL1P1;
		UVthr.VL1N1	= argp->VL1N1;
		UVthr.VL1P1 = argp->VL1P1;

		UVthr.UL1N2 = argp->UL1N2;
		UVthr.UL1P2 = argp->UL1P2;
		UVthr.VL1N2	= argp->VL1N2;
		UVthr.VL1P2 = argp->VL1P2;

		UVthr.UL1N3 = argp->UL1N3;
		UVthr.UL1P3 = argp->UL1P3;
		UVthr.VL1N3	= argp->VL1N3;
		UVthr.VL1P3 = argp->VL1P3;
		gpHalCdspSetAWB(argp->awbclamp_en, argp->sindata, argp->cosdata, argp->awbwinthr);
		gpHalCdspSetAwbYThr(argp->Ythr0, argp->Ythr1, argp->Ythr2, argp->Ythr3);
		gpHalCdspSetAwbUVThr(&UVthr);
		gpHalCdspSetAWBEn(argp->awb_win_en, argp->awb_win_hold);
		gpHalCdspSetIntEn(ENABLE, CDSP_AWBWIN_UPDATE);
	} else {
		gpHalCdspSetAWBEn(DISABLE, DISABLE);
		gpHalCdspSetIntEn(DISABLE, CDSP_AWBWIN_UPDATE);
	}
}

void
gpCdspGetAWB(
	gpCdspAWB_t *argp
)
{
	awb_uv_thr_t UVthr;

	gpHalCdspGetAWB(&argp->awbclamp_en, &argp->sindata, &argp->cosdata, &argp->awbwinthr);
	gpHalCdspGetAwbYThr(&argp->Ythr0, &argp->Ythr1, &argp->Ythr2, &argp->Ythr3);
	gpHalCdspGetAwbUVThr(&UVthr);
	gpHalCdspGetAWBEn(&argp->awb_win_en, &argp->awb_win_hold);
	
	argp->UL1N1 = UVthr.UL1N1;
	argp->UL1P1 = UVthr.UL1P1;
	argp->VL1N1 = UVthr.VL1N1;
	argp->VL1P1 = UVthr.VL1P1;

	argp->UL1N2 = UVthr.UL1N2;
	argp->UL1P2 = UVthr.UL1P2;
	argp->VL1N2 = UVthr.VL1N2;
	argp->VL1P2 = UVthr.VL1P2;

	argp->UL1N3 = UVthr.UL1N3;
	argp->UL1P3 = UVthr.UL1P3;
	argp->VL1N3 = UVthr.VL1N3;
	argp->VL1P3 = UVthr.VL1P3;
}

void
gpCdspSetWBGain2(
	gpCdspWbGain2_t *argp
)
{
	//DEBUG("%s = %d\n", __FUNCTION__, argp->wbgain2en);
	if(argp->wbgain2en) {
		gpHalCdspSetWbGain2(argp->rgain2, argp->ggain2, argp->bgain2);
		gpHalCdspSetWbGain2En(ENABLE);
	} else {
		gpHalCdspSetWbGain2En(DISABLE);
	}
}

void
gpCdspGetWBGain2(
	gpCdspWbGain2_t *argp
)
{
	gpHalCdspGetWbGain2(&argp->rgain2, &argp->ggain2, &argp->bgain2);
	argp->wbgain2en = gpHalCdspGetWbGain2En();
}

void
gpCdspSetHistgm(
	gpCdspHistgm_t *argp
)
{
	if(argp->his_en) {
		gpHalCdspSetHistgm(argp->hislowthr, argp->hishighthr);
		gpHalCdspSetHistgmEn(ENABLE, argp->his_hold_en);
	} else {
		gpHalCdspSetHistgmEn(DISABLE, DISABLE);
	}
}

void
gpCdspGetHistgm(
	gpCdspHistgm_t *argp
)
{
	gpHalCdspGetHistgm(&argp->hislowthr, &argp->hishighthr);
	gpHalCdspGetHistgmEn(&argp->his_en, &argp->his_hold_en);
}

int
gpCdspSetScaleCrop(
	gpCdspScalePara_t *argp
)
{
	unsigned char clamphsizeen;
	unsigned short clamphsize, src_width, src_height;
	unsigned int temp;

	src_width = CdspDev->imgWidth;
	src_height = CdspDev->imgHeight;
	clamphsizeen = 0;
	clamphsize = CdspDev->imgRbWidth;

	/* raw h scale down*/
	if(CdspDev->rawFmtFlag) {
		if(argp->hscale_en && (src_width > argp->dst_hsize)) {
			DEBUG(KERN_WARNING "HScaleEn\n");
			argp->dst_hsize &= ~(0x1); 	/* 2 align */
			src_width = argp->dst_hsize;
			clamphsize = argp->dst_hsize;
			gpHalCdspSetRawHScale(src_width, argp->dst_hsize);
			gpHalCdspSetRawHScaleEn(argp->hscale_en, argp->hscale_mode);
		} else {
			argp->hscale_en = 0;
			gpHalCdspSetRawHScaleEn(DISABLE, argp->hscale_mode);
		}
	} else {
		argp->hscale_en = 0;
		gpHalCdspSetRawHScaleEn(DISABLE, argp->hscale_mode);
	}

	/* crop */
	if((argp->hscale_en == 0) &&
		argp->crop_en &&
		(src_width > argp->crop_hsize) &&
		(src_height > argp->crop_vsize)) {
		
		DEBUG(KERN_WARNING "CropEn\n");
		if(argp->crop_hoffset == 0) {
			argp->crop_hoffset = 1;
		}

		if(argp->crop_voffset == 0) {
			argp->crop_voffset = 1;
		}
		
		temp = argp->crop_hoffset + argp->crop_hsize;
		if(temp > src_width) {
			argp->crop_hsize = src_width - argp->crop_hoffset;
		}
		
		temp = argp->crop_voffset + argp->crop_vsize;
		if(temp > src_height) {
			argp->crop_vsize = src_height - argp->crop_voffset;
		}
		
		src_width = argp->crop_hsize;
		src_height = argp->crop_vsize;
		clamphsize = argp->crop_hsize;
		gpHalCdspSetCrop(argp->crop_hoffset, argp->crop_voffset, argp->crop_hsize, argp->crop_vsize);
		gpHalCdspSetCropEn(argp->crop_en);
	} else {
		argp->crop_en = 0;
		gpHalCdspSetCropEn(DISABLE);
	}

	/* yuv h scale down*/
	if(argp->yuvhscale_en && (src_width > argp->yuv_dst_hsize)) {
		DEBUG(KERN_WARNING "YuvHScaleEn, %d -> %d\n", src_width, argp->yuv_dst_hsize);
		if(argp->yuv_dst_hsize > argp->img_rb_h_size) {
			argp->yuv_dst_hsize = argp->img_rb_h_size;
		}
		
		argp->yuv_dst_hsize &= ~(0x1); 	/* 2 align */
		clamphsizeen = 1;
		clamphsize = argp->yuv_dst_hsize;
		temp = (argp->yuv_dst_hsize<<16)/src_width + 1;
		gpHalCdspSetYuvHScale(temp, temp);
		gpHalCdspSetYuvHScaleEn(ENABLE, argp->yuvhscale_mode);
	} else if(src_width > argp->img_rb_h_size) {
		DEBUG(KERN_WARNING "YuvHScaleEn1, %d -> %d\n", src_width, argp->img_rb_h_size);
		argp->yuv_dst_hsize &= ~(0x1); 	/* 2 align */
		clamphsizeen = 1;
		clamphsize = argp->img_rb_h_size;
		temp = (argp->img_rb_h_size<<16)/src_width + 1;
		gpHalCdspSetYuvHScale(temp, temp);
		gpHalCdspSetYuvHScaleEn(ENABLE, argp->yuvhscale_mode);
	} else {
		argp->yuvhscale_en = 0;
		gpHalCdspSetYuvHScaleEn(DISABLE, argp->yuvhscale_mode);
	}

	/* yuv v scale down*/
	if(argp->yuvvscale_en && (src_height > argp->yuv_dst_vsize)) {
		DEBUG(KERN_WARNING "YuvVScaleEn, %d -> %d\n", src_height, argp->yuv_dst_vsize);
		if(argp->yuv_dst_vsize >  argp->img_rb_v_size) {
			argp->yuv_dst_vsize = argp->img_rb_v_size;
		}
		
		argp->yuv_dst_vsize &= ~(0x1); 	/* 2 align */
		temp = (argp->yuv_dst_vsize<<16)/src_height + 1;
		gpHalCdspSetYuvVScale(temp, temp);
		gpHalCdspSetYuvVScaleEn(ENABLE, argp->yuvvscale_mode);
	} else if(src_height > argp->img_rb_v_size) {
		DEBUG(KERN_WARNING "YuvVScaleEn1, %d -> %d\n", src_height, argp->img_rb_v_size);
		argp->yuv_dst_vsize &= ~(0x1); 	/* 2 align */
		temp = (argp->img_rb_v_size<<16)/src_height + 1;
		gpHalCdspSetYuvVScale(temp, temp);
		gpHalCdspSetYuvVScaleEn(ENABLE, argp->yuvvscale_mode);
	} else {
		argp->yuvvscale_en = 0;
		gpHalCdspSetYuvVScaleEn(DISABLE, argp->yuvhscale_mode);
	}

	/* set clamp enable and clamp h size */
	gpHalCdspSetClampEn(clamphsizeen, clamphsize);
	CdspDev->imgRbWidth = argp->img_rb_h_size;
	CdspDev->imgRbHeight = argp->img_rb_v_size;

	/* set fifo */	
	gpCdspSetFifoSize(CdspDev->imgRbWidth);
	
	DEBUG(KERN_WARNING "Clampen = %d\n", clamphsizeen);
	DEBUG(KERN_WARNING "ClampSize = %d\n", clamphsize);
	DEBUG(KERN_WARNING "rbSize = %dx%d\n", CdspDev->imgRbWidth, CdspDev->imgRbHeight);
	DEBUG(KERN_WARNING "SrcSize = %dx%d\n", CdspDev->imgWidth, CdspDev->imgHeight);
	return 0;
}

void
gpCdspGetScaleCrop(
	gpCdspScalePara_t *argp
)
{
	gpHalCdspGetRawHScaleEn(&argp->hscale_en, &argp->hscale_mode);

	gpHalCdspGetCrop(&argp->crop_hoffset, &argp->crop_voffset, &argp->crop_hsize, &argp->crop_vsize);
	argp->crop_en = gpHalCdspGetCropEn();

	gpHalCdspGetYuvHScaleEn(&argp->yuvhscale_en, &argp->yuvhscale_mode);
	gpHalCdspGetYuvVScaleEn(&argp->yuvvscale_en, &argp->yuvvscale_mode);
}

void
gpCdspSetScaleAeAf(
	gpCdspScalePara_t *argp
)
{
	unsigned short w, h;
	//gpCdspRawWin_t raw_win;
	//gpCdspAE_t ae;
	gpCdspAF_t af;

	if(argp->crop_en) {
		w = argp->crop_hsize;
		h = argp->crop_vsize;
	} else {
		w = CdspDev->imgWidth;
		h = CdspDev->imgHeight;
	}

	//gpCdspGetRawWin(&raw_win);
	//gpCdspSetRawWin(w, h, &raw_win);

	gpCdspGetAF(&af);
	gpCdspSetAF(w, h, &af);

	//gpCdspGetAE(&ae);
	//gpCdspSetAE(&ae);
}


static void gp_cdsp_edge_level_init(void)
{
	int step;

	CdspDev->edge_level[0] = CdspDev->edge_day;
	CdspDev->edge_level[3] = CdspDev->edge_night;

	step = (CdspDev->edge_day - CdspDev->edge_night + 1) / 3;

	CdspDev->edge_level[1] = CdspDev->edge_level[0] - step;
	CdspDev->edge_level[2] = CdspDev->edge_level[3] + step;	
}


static void gp_cdsp_sat_contrast_init(void)
{
	int i;
	int step;
	
	for(i = 0 ; i < 6 ; i++)
	{
		step = (abs(CdspDev->sat_yuv_level[0][i] - CdspDev->sat_yuv_level[3][i]) + 1) /3;
		if(CdspDev->sat_yuv_level[0][i] > CdspDev->sat_yuv_level[3][i]) step = -step;
		
		CdspDev->sat_yuv_level[1][i] = CdspDev->sat_yuv_level[0][i] + step;
		CdspDev->sat_yuv_level[2][i] = CdspDev->sat_yuv_level[3][i] - step;
	}
		

	#if 0
	{
		int i, j;
		DEBUG("\r\n\r\nsat constrast level:\r\n");
		for(i = 0;i<4;i++)
		{
			DEBUG("Thr[%d]: YUV = ", CdspDev->sat_yuv_thr[i]);
			for(j = 0;j<6;j++)	DEBUG(" %d,", CdspDev->sat_yuv_level[i][j]);
			
			DEBUG("\r\n");
		}
	}
	#endif
	
}

static void gp_cdsp_sat_contrast_thr_init(int max_ev_idx, int night_ev_idx)
{
	int i;
	int step;
	int thr0, thr3;
		
	// thr
	thr0 = night_ev_idx;
	thr3 = max_ev_idx - 8;
	step = (thr3 - thr0 + 1) /3;
	if(step < 0 ) step = 0;

	//printk("max_ev_idx = %d, night_ev_idx = %d, step = %d\r\n", max_ev_idx, night_ev_idx, step);
	
	CdspDev->sat_yuv_thr[0] = thr0;
	CdspDev->sat_yuv_thr[1] = thr0 + step;
	CdspDev->sat_yuv_thr[2] = thr3 - step;
	CdspDev->sat_yuv_thr[3] = thr3;

	#if 0
	{
		int i, j;
		DEBUG("\r\n\r\nsat constrast level:\r\n");
		for(i = 0;i<4;i++)
		{
			DEBUG("Thr[%d]: YUV = ", CdspDev->sat_yuv_thr[i]);
			for(j = 0;j<6;j++)	DEBUG(" %d,", CdspDev->sat_yuv_level[i][j]);
			
			DEBUG("\r\n");
		}
	}
	#endif
	
}



int
check_rqbuf_type(
	void
)
{
#if (C_DMA_CH == 0)
	if(CdspDev->rbuf.count <= 2) {
		DEBUG("too few buffers\n");
		return -EINVAL;
	}
#endif
	
	if(CdspDev->rbuf.count >= C_BUFFER_MAX){
		DEBUG("too many buffers\n");
		return -EINVAL;
	}
		
	if(CdspDev->rbuf.type != V4L2_BUF_TYPE_VIDEO_CAPTURE){
		DEBUG("only support video capture mode");
		return -EINVAL;
	}
	
	if(CdspDev->rbuf.memory != V4L2_MEMORY_USERPTR){
		DIAG_ERROR("only support userptr I/O strea\n");
		return -EINVAL;
	}
	
	return 0;
}

unsigned int 
cdsp_get_ready_buffer(
	unsigned short *width,
	unsigned short *height
)
{
	unsigned int ready_buf = 0;
	struct v4l2_buffer *pbuf;
	
	if(CdspDev->out_que[0] == 0xFF) {
		*width = 0;
		*height = 0;
		DEBUG("no buffer ready\n");
		return 0;
	}

	pbuf = &CdspDev->bf[CdspDev->out_que[0]];
	ready_buf = pbuf->m.userptr;
	*width = CdspDev->imgRbWidth;
	*height = CdspDev->imgRbWidth;
	return ready_buf; 
}
EXPORT_SYMBOL(cdsp_get_ready_buffer);

static int
gpCdspPostProcess(
	gpCdspPostProcess_t	*PostPress
)
{
	unsigned int cdsp_inFmt;
	unsigned int addr, size;
	gpCdspYuvHAvg_t yuv_havg;	
	gpCdspLenCmp_t lens_cmp;

	DEBUG("%s\n", __FUNCTION__);
	if(CdspDev->imgSrc != C_CDSP_SDRAM) {
		DEBUG(KERN_WARNING "imag source is not SDRAM\n");
		return -1;
	}
	
	switch(PostPress->inFmt)	
	{	
	case V4L2_PIX_FMT_VYUY:
		cdsp_inFmt = C_SDRAM_FMT_VY1UY0;				
		CdspDev->rawFmtFlag = 0;				
		CdspDev->rawFmt = 0;		
		break;

	case V4L2_PIX_FMT_SBGGR8:
		cdsp_inFmt = C_SDRAM_FMT_RAW8;
		CdspDev->rawFmtFlag = 1;				
		CdspDev->rawFmt = 2;
		break;

	case V4L2_PIX_FMT_SGBRG8:
		cdsp_inFmt = C_SDRAM_FMT_RAW8;
		CdspDev->rawFmtFlag = 1;				
		CdspDev->rawFmt = 3;
		break;
		
	case V4L2_PIX_FMT_SGRBG8:
		cdsp_inFmt = C_SDRAM_FMT_RAW8;
		CdspDev->rawFmtFlag = 1;				
		CdspDev->rawFmt = 0;
		break;

	case V4L2_PIX_FMT_SGRBG10:	
		cdsp_inFmt = C_SDRAM_FMT_RAW10;
		CdspDev->rawFmtFlag = 1;				
		CdspDev->rawFmt = 1;	
		break;

	default:
		DEBUG(KERN_WARNING "input Format ERROR\n");
		return -1;	
	}

	/* cdsp clock */
	gpCdspSetClkPath(CdspDev->imgSrc, CdspDev->rawFmtFlag);

	/* cdsp reset */
	gpHalCdspReset();
	gpHalFrontReset();
	gpHalCdspDataSource(C_CDSP_SDRAM);

	/* set module */	
	if(CdspDev->rawFmtFlag) {		
		gpCdspIntpl_t intpl;		
		gpCdspEdge_t edge;
		//gpCdspNewDenoise_t newdenoise;	
		
		gpCdspGetIntpl(&intpl);		
		gpCdspSetIntpl(CdspDev->imgWidth, &intpl);		
		
		gpCdspGetEdge(&edge);		
		gpCdspSetEdge(CdspDev->rawFmtFlag, &edge);
		
		//gpCdspGetNewDenoise(&newdenoise);
		//gpCdspSetNewDenoise(&newdenoise);
		
	} else {				
		gpCdspEdge_t edge;
		
		gpCdspGetEdge(&edge);		
		gpCdspSetSuppression(PostPress->width, &CdspDev->suppression, &edge);	
	}	

	gpCdspGetLensCmp(&lens_cmp);	
	gpCdspSetLensCmp(CdspDev->rawFmtFlag, &lens_cmp);	
	gpCdspGetYuvHavg(&yuv_havg);	
	gpCdspSetYuvHavg(&yuv_havg);	

	/* set scale & crop */	
	gpCdspSetScaleCrop(&CdspDev->scale);
	gpCdspSetScaleAeAf(&CdspDev->scale);

	/* set fifo */	
	gpCdspSetFifoSize(PostPress->width);
	
	/* set interface */
#if _SET_YUV_PATH_
	switch(cdsp_inFmt)
	{
	case C_SDRAM_FMT_VY1UY0:
	case C_SDRAM_FMT_RAW8:
		gpHalCdspSetRawPath(0, 1, 1, 0); //yuyv out
		break;
	case C_SDRAM_FMT_RAW10:
		gpHalCdspSetRawPath(0, 0, 1, 0); //yuyv out
		break;
	}
#else
	if(RAW_BIT == 8)
		gpHalCdspSetRawPath(1, 1, 1, 0); //8bit raw out in 8-bit format
	else if(RAW_BIT == 10)
		gpHalCdspSetRawPath(RAW_MODE, 0, 1, 0); // packed 10bit raw out 
	else
		DERROR("Err: Please set bit of raw\r\n");
#endif	

	gpHalFrontYuvOrderSet(0);
		
	gpHalFrontInputPathSet(0);		//mipi disable	
	
	if(CdspDev->rawFmtFlag) {
		DEBUG(KERN_WARNING "SDRAMRawFmt\n");
		gpHalCdspSetYuvRange(0x00);
		gpHalCdspSetRawDataFormat(CdspDev->rawFmt);
		gpHalFrontInputGate(0x1FF);
		gpHalCdspSetMuxPath(0);			//raw path
		
		addr = (unsigned int)gp_user_va_to_pa((void *)PostPress->inAddr);
		gpHalCdspSetRawBuff(addr);
		if(cdsp_inFmt == C_SDRAM_FMT_RAW10) {
			gpHalCdspSetRawBuffSize(PostPress->width, PostPress->height, 0x00, 10);
		} else {
			gpHalCdspSetRawBuffSize(PostPress->width, PostPress->height, 0x00, 8);
		}
		
		addr = (unsigned int)gp_user_va_to_pa((void *)PostPress->outAddr);
		gpHalCdspSetYuvBuffA(PostPress->width, PostPress->height, addr);
		gpHalCdspSetDmaBuff(RD_A_WR_A);

		size = PostPress->width*PostPress->height;
		gpHalCdspSetReadBackSize(0x00, 0x00, PostPress->width, PostPress->height);
	} else {
		DEBUG(KERN_WARNING "SDRAMYuvFmt\n");
		gpHalCdspSetYuvRange(PostPress->yuvRange);
		gpHalCdspSetRawDataFormat(0x00);
		gpHalFrontInputGate(0x1FF);
		gpHalCdspSetMuxPath(1);			//yuv path
		
		addr = (unsigned int)gp_user_va_to_pa((void *)PostPress->inAddr);
		gpHalCdspSetYuvBuffA(PostPress->width, PostPress->height, addr);

		addr = (unsigned int)gp_user_va_to_pa((void *)PostPress->outAddr);
		gpHalCdspSetYuvBuffB(PostPress->width, PostPress->height, addr);
		gpHalCdspSetDmaBuff(RD_A_WR_B);

		size = PostPress->width*PostPress->height*2;
		gpHalCdspSetReadBackSize(0x00, 0x00, PostPress->width, PostPress->height);
	}
	
	/* clean dcache */
#ifndef GP_SYNC_OPTION
	gp_clean_dcache_range(PostPress->inAddr, size);
#else
	GP_SYNC_CACHE();
#endif
	
	/* start & wait finish*/
	CdspDev->cdsp_feint_flag = 0;
	gpHalCdspClrIntStatus(CDSP_INT_ALL);
	gpHalCdspSetIntEn(ENABLE, CDSP_OVERFOLW|CDSP_EOF);
	gpHalCdspDataSource(C_CDSP_SDRAM);
	gpHalCdspRedoTriger(1);
	wait_event_interruptible(CdspDev->cdsp_wait_queue, (CdspDev->cdsp_feint_flag != 0));

	/* stop */
	gpHalCdspSetIntEn(DISABLE, CDSP_OVERFOLW|CDSP_EOF);
	gpHalCdspDataSource(C_CDSP_SDRAM);
	gpHalCdspRedoTriger(0);
	CdspDev->cdsp_feint_flag = 0;
	
	/* invalid cache */
#ifndef GP_SYNC_OPTION
	gp_invalidate_dcache_range(PostPress->outAddr, PostPress->width*PostPress->height*2);
#else
	GP_SYNC_CACHE();
#endif
	return 0;
}

static int
gp_cdsp_s_fmt(
	struct v4l2_format *fmt
)
{
	int i, idx;
	unsigned int cdsp_inFmt;
	gpCdspYuvHAvg_t yuv_havg;	
	gpCdspLenCmp_t lens_cmp;
	gpHalFrontRoi_t roi;
	//gpHalFrontReshape_t reshapeCtl;	
	
	DEBUG("%s\n", __FUNCTION__);
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
			DIAG_ERROR("FmtSizeErr=%dx%d\n", fmt->fmt.pix.width, fmt->fmt.pix.height);
			return -1;
		}
	} 

	idx = fmt->fmt.pix.priv;
	CdspDev->inFmt = sensor->fmt[idx].pixelformat;
	CdspDev->imgWidth = CdspDev->imgRbWidth = sensor->fmt[idx].hpixel;
	CdspDev->imgHeight = CdspDev->imgRbHeight = sensor->fmt[idx].vline;
	CdspDev->scale.img_rb_h_size = sensor->fmt[idx].hpixel;
	CdspDev->scale.img_rb_v_size = sensor->fmt[idx].vline;	
	
	switch(CdspDev->inFmt)
	{
		case V4L2_PIX_FMT_YUYV:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_VY1UY0;//C_FRONT_FMT_Y1UY0V;			
				CdspDev->rawFmtFlag = 0;
				CdspDev->rawFmt = 0;
			} else {
				DIAG_ERROR("FmtErr\n");
				return -1;
			}
			break;
			
		case V4L2_PIX_FMT_YVYU:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_UY1VY0;//C_FRONT_FMT_Y1VY0U;	
			} else {
				cdsp_inFmt = C_MIPI_FMT_Y1VY0U;			
			}
			CdspDev->rawFmtFlag = 0;
			CdspDev->rawFmt = 0;
			break;
			
		case V4L2_PIX_FMT_UYVY:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_Y1UY0V;//C_FRONT_FMT_UY1VY0;			
				CdspDev->rawFmtFlag = 0;
				CdspDev->rawFmt = 0;	
			} else {
				DIAG_ERROR("FmtErr\n");
				return -1;
			}
			break;
			
		case V4L2_PIX_FMT_VYUY:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_Y1VY0U;//C_FRONT_FMT_VY1UY0;			
				CdspDev->rawFmtFlag = 0;
				CdspDev->rawFmt = 0;	
			} else {
				DIAG_ERROR("FmtErr\n");
				return -1;
			}
			break;

		case V4L2_PIX_FMT_SBGGR8:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_RAW8;			
			} else {
				cdsp_inFmt = C_MIPI_FMT_RAW8;
			}
			CdspDev->rawFmtFlag = 1;			
			CdspDev->rawFmt = 2;
			break;

		case V4L2_PIX_FMT_SGBRG8:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_RAW8;			
			} else {
				cdsp_inFmt = C_MIPI_FMT_RAW8;
			}
			CdspDev->rawFmtFlag = 1;				
			CdspDev->rawFmt = 3;		
			break;
			
		case V4L2_PIX_FMT_SGRBG8:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_RAW8;			
			} else {
				cdsp_inFmt = C_MIPI_FMT_RAW8;
			}
			CdspDev->rawFmtFlag = 1;				
			CdspDev->rawFmt = 0;	
			break;

/*		case V4L2_PIX_FMT_SRGGB8:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_RAW8;			
			} else {
				cdsp_inFmt = C_MIPI_FMT_RAW8;
			}
			CdspDev->rawFmtFlag = 1;				
			CdspDev->rawFmt = 1;	
			break;*/
			
			
		case V4L2_PIX_FMT_SGRBG10:
			if(CdspDev->imgSrc == C_CDSP_FRONT) {
				cdsp_inFmt = C_FRONT_FMT_RAW10;			
			} else {
				cdsp_inFmt = C_MIPI_FMT_RAW10;
			}
			CdspDev->rawFmtFlag = 1;				
			CdspDev->rawFmt = 0;	
			break;

		default:
			DIAG_ERROR("FmtNotSupport!\n");
			return -1;
	}

	DEBUG("cdsp_inFmt = %d , CdspDev->inFmt = %d, CdspDev->rawFmt = %d\r\n", cdsp_inFmt, CdspDev->inFmt, CdspDev->rawFmt);

	/* cdsp clock */
	gpCdspSetClkPath(CdspDev->imgSrc, CdspDev->rawFmtFlag);
	
	/* cdsp reset */
	gpHalCdspReset();
	gpHalFrontReset();
	gpHalCdspDataSource(C_CDSP_SKIP_WR);

	/* set module */	
	if(CdspDev->rawFmtFlag) {		
		gpCdspIntpl_t intpl;		
		gpCdspEdge_t edge;
		gpCdspNewDenoise_t newdenoise;
		
		gpCdspGetIntpl(&intpl);		
		gpCdspSetIntpl(CdspDev->imgWidth, &intpl);		
		
		gpCdspGetEdge(&edge);		
		gpCdspSetEdge(CdspDev->rawFmtFlag, &edge);
		
		gpCdspGetNewDenoise(&newdenoise);
		gpCdspSetNewDenoise(&newdenoise);
	} else {				
		gpCdspEdge_t edge;
		
		gpCdspGetEdge(&edge);		
		gpCdspSetSuppression(CdspDev->imgWidth, &CdspDev->suppression, &edge);	
	}	

	gpCdspGetLensCmp(&lens_cmp);	
	gpCdspSetLensCmp(CdspDev->rawFmtFlag, &lens_cmp);	
	
	gpCdspGetYuvHavg(&yuv_havg);	
	gpCdspSetYuvHavg(&yuv_havg);	

	/* set scale & crop */
	gpCdspSetScaleCrop(&CdspDev->scale);	
	gpCdspSetScaleAeAf(&CdspDev->scale);	

	/* set fifo */	
	gpCdspSetFifoSize(CdspDev->imgRbWidth);

	switch(CdspDev->imgSrc)
	{
	case C_CDSP_FRONT:
		if(CdspDev->rawFmtFlag) {
			DEBUG(KERN_WARNING "FrontRawFmt = %d\n", CdspDev->rawFmt);
			gpHalCdspSetYuvRange(0x00);
			gpHalCdspSetRawDataFormat(CdspDev->rawFmt);
		} else {
			DEBUG(KERN_WARNING "FrontYuvFmt\n");
			gpHalCdspSetYuvRange(0x00);
			gpHalCdspSetRawDataFormat(0x00);
		}

		/* set interface */
#if _SET_YUV_PATH_
		gpHalCdspSetRawPath(0, 1, 1, 0); //yuyv out		
#else
		if(RAW_BIT == 8)
			gpHalCdspSetRawPath(1, 1, 1, 0); //8bit raw out in 8-bit format
		else if(RAW_BIT == 10)
			gpHalCdspSetRawPath(RAW_MODE, 0, 1, 0); // packed 10bit raw out 
		else
			DERROR("Err: Please set bit of raw\r\n");
#endif

		if(cdsp_inFmt == C_FRONT_FMT_UY1VY0) {
			gpHalFrontYuvOrderSet(0);
		} else if(cdsp_inFmt == C_FRONT_FMT_Y1VY0U) {
			gpHalFrontYuvOrderSet(1);
		} else if(cdsp_inFmt == C_FRONT_FMT_VY1UY0) {
			gpHalFrontYuvOrderSet(2);
		} else if(cdsp_inFmt == C_FRONT_FMT_Y1UY0V) {
			gpHalFrontYuvOrderSet(3);
		} else {
			gpHalFrontYuvOrderSet(3);
		}
		
		gpHalFrontInputPathSet(0);		//mipi disable	
		
		if(CdspDev->rawFmtFlag) {
			gpHalFrontInputModeSet(0);	//raw format
			gpHalFrontYuvSubEn(0, 0, 0);
			gpHalFrontInputGate(0x1FC);
			gpHalCdspSetMuxPath(0);		//raw path
			DEBUG("raw path\r\n");
		} else {
			gpHalFrontInputModeSet(1);  //yuv format
			gpHalFrontYuvSubEn(0, 1, 1);
			gpHalFrontInputGate(0x1E0);
			gpHalCdspSetMuxPath(1);		//yuv path
			DEBUG("yuyv path\r\n");
		}

		if(sensor->sensor_timing_mode == CCIR601) {
			gpHalFrontDataTransMode(0);	//ccir601 interfcae
		} else {
			gpHalFrontDataTransMode(1); //ccir656 interface
		}
		
	#if 0 //resharp
		if(CdspDev->rawFmtFlag) {
			reshapeCtl.mode = 0; //RAW
		} else {
			reshapeCtl.mode = 1; //YUV
		}
		
		reshapeCtl.hReshapeEn = 1;	
		reshapeCtl.vReshapeEn = 1;	
		reshapeCtl.vReshapeClkType = 0; // 0: H-sync ; 1: pclk	
		reshapeCtl.vBackOffEn = 0;	
		reshapeCtl.hRise = 2;	
		reshapeCtl.hFall = 1;	
		reshapeCtl.vRise = 2;	
		reshapeCtl.vFall = 1;	
		gpHalFrontReshapeSet(reshapeCtl);
	#endif
	
		gpHalFrontSyncPolaritySet(sensor->sensor_hsync_mode ? 0 : 1, 
								sensor->sensor_vsync_mode ? 0 : 1,
								CdspDev->SyncFlag);
	
	
		roi.hOffset = sensor->fmt[idx].hoffset;	
		roi.vOffset = sensor->fmt[idx].voffset;	
		if(roi.hOffset == 0) {
			roi.hOffset = 1;	
		} 

		if(roi.vOffset == 0) {
			roi.vOffset = 1;
		} 
		
		roi.hSize = sensor->fmt[idx].hpixel;		
		roi.vSize = sensor->fmt[idx].vline;
		if(sensor->sensor_timing_mode == HREF) {
			roi.vSize -= 1;
		} 
		
		gpHalFrontRoiSet(roi, 0);
		break;

	case C_CDSP_MIPI:
		if(CdspDev->rawFmtFlag) {		
			DEBUG(KERN_WARNING "MipiRawFmt\n");		
			gpHalCdspSetYuvRange(0x00);		
			gpHalCdspSetRawDataFormat(CdspDev->rawFmt);	
		} else {		
			DEBUG(KERN_WARNING "MipiYuvFmt\n");		
			gpHalCdspSetYuvRange(0x00);		
			gpHalCdspSetRawDataFormat(0x00);	
		}	

		/* set interface */	
#if _SET_YUV_PATH_
		gpHalCdspSetRawPath(0, 0, 1, 0); //yuyv out		
#else
		if(RAW_BIT == 8)
			gpHalCdspSetRawPath(1, 1, 1, 0); //8bit raw out in 8-bit format
		else if(RAW_BIT == 10)
			gpHalCdspSetRawPath(RAW_MODE, 0, 1, 0); // packed 10bit raw out 
		else
			DERROR("Err: Please set bit of raw\r\n");
#endif		

		gpHalFrontYuvOrderSet(0);
		
		gpHalFrontInputPathSet(1);		//mipi enable	

		if(CdspDev->rawFmtFlag) {
			gpHalFrontInputModeSet(0);	//raw format
			gpHalFrontYuvSubEn(0, 0, 0);
			gpHalFrontInputGate(0x1FC);	//0x1FF
			gpHalCdspSetMuxPath(0);		//raw path
		} else {
			gpHalFrontInputModeSet(1);  //yuv format
			gpHalFrontYuvSubEn(0, 1, 1);
			gpHalFrontInputGate(0x1E0);	//0x1FF
			gpHalCdspSetMuxPath(1);		//yuv path
		}
		
		gpHalFrontDataTransMode(0);		//ccir601 interfcae

	#if 0 //resharp		
		if(CdspDev->rawFmtFlag) {
			reshapeCtl.mode = 0; //RAW
		} else {
			reshapeCtl.mode = 1; //YUV
		}
		
		reshapeCtl.hReshapeEn = 1;	
		reshapeCtl.vReshapeEn = 1;	
		reshapeCtl.vReshapeClkType = 0; // 0: H-sync ; 1: pclk	
		reshapeCtl.vBackOffEn = 0;	
		reshapeCtl.hRise = 2;	
		reshapeCtl.hFall = 1;	
		reshapeCtl.vRise = 2;	
		reshapeCtl.vFall = 1;	
		gpHalFrontReshapeSet(reshapeCtl);
	#endif
	
		gpHalFrontSyncPolaritySet(0, 0, 1);

		roi.hOffset = sensor->fmt[idx].hoffset;	
		roi.vOffset = sensor->fmt[idx].voffset;	
		roi.hSize = sensor->fmt[idx].hpixel;	
		roi.vSize = sensor->fmt[idx].vline;	
		gpHalFrontRoiSet(roi, 0);
		break;
		
	default:
		DEBUG(KERN_WARNING "ImgSrcErr!\n");
		return -1;
	}

	//set mclk clock 
	CdspDev->MclkEn = 1;
	CdspDev->Mclk.mclk_out = sensor->fmt[idx].mclk;
	CdspDev->Mclk.mclk_sel = sensor->fmt[idx].mclk_src;
	gpCdspSetMclkout(ENABLE, &CdspDev->Mclk);
	return 0;
}

int
gp_cdsp_g_ctrl(
	struct v4l2_control *ctrl
)
{
	int nRet = 0;

	//DEBUG("%s\n", __FUNCTION__);
	switch(ctrl->id)
	{
		case MSG_CDSP_SCALE_CROP:
		{
			gpCdspGetScaleCrop(&CdspDev->scale);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&CdspDev->scale, sizeof(CdspDev->scale));
			break;
		}

		case MSG_CDSP_BADPIX_OB:
		{
			gpCdspBadPixOB_t bad_pixel;

			gpCdspGetBadPixOb(&bad_pixel);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&bad_pixel, sizeof(bad_pixel));
			break;
		}

		case MSG_CDSP_LENS_CMP:
		{
			gpCdspLenCmp_t lens_cmp;

			gpCdspGetLensCmp(&lens_cmp);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&lens_cmp, sizeof(lens_cmp));
			break;
		}

		case MSG_CDSP_WBGAIN:
		{
			gpCdspWhtBal_t wht_bal;
			gpCdspGetWhiteBalance(&wht_bal);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&wht_bal, sizeof(wht_bal));
			break;
		}

		case MSG_CDSP_LUT_GAMMA:
		{
			gpCdspGamma_t lut_gamma;

			gpCdspGetLutGamma(&lut_gamma);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&lut_gamma, sizeof(lut_gamma));
			break;
		}

		case MSG_CDSP_INTERPOLATION:
		{
			gpCdspIntpl_t intpl;

			gpCdspGetIntpl(&intpl);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&intpl, sizeof(intpl));
			break;
		}

		case MSG_CDSP_EDGE:
		{
			gpCdspEdge_t edge;
			gpCdspGetEdge(&edge);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&edge, sizeof(edge));
			break;
		}

		case MSG_CDSP_COLOR_MATRIX:
		{
			gpCdspCorMatrix_t matrix;

			gpCdspGetColorMatrix(&matrix);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&matrix, sizeof(matrix));
			break;
		}

		case MSG_CDSP_POSWB_RGB2YUV:
		{
			gpCdspRgb2Yuv_t rgb2yuv;

			gpCdspGetRgbToYuv(&rgb2yuv);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&rgb2yuv, sizeof(rgb2yuv));
			break;
		}

		case MSG_CDSP_YUV_INSERT:
		{
			gpCdspYuvInsert_t yuv_insert;

			gpCdspGetYuv444Insert(&yuv_insert);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&yuv_insert, sizeof(yuv_insert));
			break;
		}

		case MSG_CDSP_YUV_HAVG:
		{
			gpCdspYuvHAvg_t yuv_havg;

			gpCdspGetYuvHavg(&yuv_havg);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&yuv_havg, sizeof(yuv_havg));
			break;
		}

		case MSG_CDSP_SPEC_MODE:
		{
			gpCdspSpecMod_t spec_mode;

			gpCdspGetSpecialMode(&spec_mode);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&spec_mode, sizeof(spec_mode));
			break;
		}

		case MSG_CDSP_TARGET_AE:
		{
			int targetLum;
			targetLum = CdspDev->ae_target;
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&targetLum, sizeof(int));
			break;
		}

		case MSG_CDSP_WB_OFFSET_DAY:
		{
			int wb_offset;

			wb_offset = CdspDev->wb_offset_day;
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&wb_offset, sizeof(wb_offset));
		}
		break;

		case MSG_CDSP_WB_OFFSET_NIGHT:
		{
			int wb_offset;
			
			wb_offset = CdspDev->wb_offset_night;
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&wb_offset, sizeof(wb_offset));
		}
		break;


		case MSG_CDSP_SAT_HUE:
		{
			gpCdspSatHue_t sat_hue;

			gpCdspGetSatHue(&sat_hue);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&sat_hue, sizeof(sat_hue));
			break;
		}

		case MSG_CDSP_SAT_HUE_DAY:
		{
			gpCdspSatHue_t sat_hue;

			gpCdspGetSatHue(&sat_hue);
			sat_hue.y_offset = CdspDev->sat_yuv_level[0][0];
			sat_hue.u_offset = CdspDev->sat_yuv_level[0][1];
			sat_hue.v_offset = CdspDev->sat_yuv_level[0][2];
			sat_hue.y_scale = CdspDev->sat_yuv_level[0][3];
			sat_hue.u_scale = CdspDev->sat_yuv_level[0][4];
			sat_hue.v_scale = CdspDev->sat_yuv_level[0][5];
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&sat_hue, sizeof(sat_hue));
			break;
		}

		case MSG_CDSP_SAT_HUE_NIGHT:
		{
			gpCdspSatHue_t sat_hue;
			
			gpCdspGetSatHue(&sat_hue);
			sat_hue.y_offset = CdspDev->sat_yuv_level[3][0];
			sat_hue.u_offset = CdspDev->sat_yuv_level[3][1];
			sat_hue.v_offset = CdspDev->sat_yuv_level[3][2];
			sat_hue.y_scale = CdspDev->sat_yuv_level[3][3];
			sat_hue.u_scale = CdspDev->sat_yuv_level[3][4];
			sat_hue.v_scale = CdspDev->sat_yuv_level[3][5];
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&sat_hue, sizeof(sat_hue));
			break;
		}

		case MSG_CDSP_SUPPRESSION:
		{
			gpCdspGetSuppression(&CdspDev->suppression);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&CdspDev->suppression, sizeof(CdspDev->suppression));
			break;
		}

		case MSG_CDSP_NEWDENOISE:
		{
			gpCdspNewDenoise_t NewDenoise;

			gpCdspGetNewDenoise(&NewDenoise);			
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&NewDenoise, sizeof(NewDenoise));
			break;
		}	

		case MSG_CDSP_RAW_WIN:
		{
			gpCdspRawWin_t raw_win;

			gpCdspGetRawWin(&raw_win);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&raw_win, sizeof(raw_win));
			break;
		}

		case MSG_CDSP_AE_WIN:
		{
			gpCdspAE_t ae;

			gpCdspGetAE(&ae);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&ae, sizeof(ae));
			break;
		}

		case MSG_CDSP_AF_WIN:
		{
			gpCdspAF_t af;

			gpCdspGetAF(&af);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&af, sizeof(af));
			break;
		}

		case MSG_CDSP_AWB_WIN:
		{
			gpCdspAWB_t awb;

			gpCdspGetAWB(&awb);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&awb, sizeof(awb));
			break;
		}

		case MSG_CDSP_WBGAIN2:
		{
			gpCdspWbGain2_t wbgain2;

			gpCdspGetWBGain2(&wbgain2);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&wbgain2, sizeof(wbgain2));
			break;
		}

		case MSG_CDSP_HISTGM:
		{
			gpCdspHistgm_t histgm;

			gpCdspGetHistgm(&histgm);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&histgm, sizeof(histgm));
			break;
		}

		case MSG_CDSP_3A_STATISTIC:
		{
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&CdspDev->result, sizeof(CdspDev->result));
			break;
		}

		default:
		{
			struct v4l2_subdev *sd;
			
			sd = CdspDev->sd;
			nRet = sd->ops->core->g_ctrl(sd, ctrl);
			if(nRet < 0) {
				CdspDev->sensor_status = CMS_I2C_TIMEOUT;
				RETURN(-EINVAL);
			}
			break;
		}
	}

__return:
	return nRet;
}
EXPORT_SYMBOL(gp_cdsp_g_ctrl);

int
gp_cdsp_s_ctrl(
	struct v4l2_control *ctrl
)
{
	unsigned short w, h;
	int nRet = 0;

	//DEBUG("%s\n", __FUNCTION__);
	switch(ctrl->id)
	{
		case MSG_CDSP_POSTPROCESS:
		{
			gpCdspPostProcess_t post;

			nRet = copy_from_user((void*)&post, (void __user*)ctrl->value, sizeof(post));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			CdspDev->imgSrc = C_CDSP_SDRAM;
			CdspDev->inFmt = post.inFmt;
			CdspDev->imgWidth = CdspDev->imgRbWidth = post.width;
			CdspDev->imgHeight = CdspDev->imgRbHeight = post.height;
			CdspDev->scale.img_rb_h_size = post.width;
			CdspDev->scale.img_rb_v_size = post.height;
			CdspDev->suppression.suppressen = 1;
			switch(CdspDev->inFmt)
			{
			case V4L2_PIX_FMT_VYUY:
			case V4L2_PIX_FMT_SBGGR8:
			case V4L2_PIX_FMT_SGBRG8:
			case V4L2_PIX_FMT_SGRBG8:
			case V4L2_PIX_FMT_SGRBG10:
				nRet = gpCdspPostProcess(&post);
				break;

			default:
				RETURN(-EINVAL);
			}
			break;
		}
		
		case MSG_CDSP_SCALE_CROP:
		{
			nRet = copy_from_user((void*)&CdspDev->scale, (void __user*)ctrl->value, sizeof(CdspDev->scale));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->RunFlag) {
				CdspDev->SkipCnt = SKIP_CNT;
			} else {
				CdspDev->SkipCnt = 0;
				nRet = gpCdspSetScaleCrop(&CdspDev->scale);
				gpCdspSetScaleAeAf(&CdspDev->scale);
			}
			break;
		}

		case MSG_CDSP_BADPIX_OB:
		{
			gpCdspBadPixOB_t bad_pixel;
			
			nRet = copy_from_user((void*)&bad_pixel, (void __user*)ctrl->value, sizeof(bad_pixel));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(CdspDev->rawFmtFlag) {
				bad_pixel.autooben = CdspDev->sensor_cdsp.ob[0];				
				bad_pixel.obtype= CdspDev->sensor_cdsp.ob[1];
				bad_pixel.obHOffset = CdspDev->sensor_cdsp.ob[2];
				bad_pixel.obVOffset = CdspDev->sensor_cdsp.ob[3];

				bad_pixel.manuoben =  CdspDev->sensor_cdsp.ob[4];
				bad_pixel.manuob =  CdspDev->sensor_cdsp.ob[5];
				bad_pixel.wboffseten = CdspDev->sensor_cdsp.ob[6];
				bad_pixel.roffset = CdspDev->sensor_cdsp.ob[7];
				bad_pixel.groffset = CdspDev->sensor_cdsp.ob[8];
				bad_pixel.gboffset = CdspDev->sensor_cdsp.ob[9];
				bad_pixel.boffset = CdspDev->sensor_cdsp.ob[10];

				gpCdspSetBadPixOb(&bad_pixel);
			}
			break;
		}

		case MSG_CDSP_LENS_CMP:
		{
			gpCdspLenCmp_t lens_cmp;

			nRet = copy_from_user((void*)&lens_cmp, (void __user*)ctrl->value, sizeof(lens_cmp));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			lens_cmp.lenscmp_table = (unsigned short  *)CdspDev->sensor_cdsp.lenscmp;
			gpCdspSetLensCmp(CdspDev->rawFmtFlag, &lens_cmp);
			break;
		}

		case MSG_CDSP_WBGAIN:
		{
			gpCdspWhtBal_t wht_bal;
			
			
			nRet = copy_from_user((void*)&wht_bal, (void __user*)ctrl->value, sizeof(wht_bal));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->rawFmtFlag) {
				gpCdspSetWhiteBalance(&wht_bal);
			}			
		}
		break;

		case MSG_CDSP_WB_OFFSET_DAY:
		{
			int wb_offset;
			
			nRet = copy_from_user((void*)&wb_offset, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			CdspDev->wb_offset_day = wb_offset;
		}
		break;

		case MSG_CDSP_WB_OFFSET_NIGHT:
		{
			int wb_offset;
			
			nRet = copy_from_user((void*)&wb_offset, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			CdspDev->wb_offset_night = wb_offset;
		}
		break;

		case MSG_CDSP_LUT_GAMMA:
		{
			gpCdspGamma_t lut_gamma;

			nRet = copy_from_user((void*)&lut_gamma, (void __user*)ctrl->value, sizeof(lut_gamma));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->rawFmtFlag) {
				lut_gamma.gamma_table = (unsigned int *)CdspDev->sensor_cdsp.gamma1;
				gpCdspSetLutGamma(&lut_gamma);
			}			
		}
		break;

		case MSG_CDSP_INTERPOLATION:
		{
			gpCdspIntpl_t intpl;

			nRet = copy_from_user((void*)&intpl, (void __user*)ctrl->value, sizeof(intpl));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->rawFmtFlag) {
				gpCdspSetIntpl(CdspDev->imgWidth, &intpl);
			}

			CdspDev->intpl_low_thr = intpl.int_low_thr;
			CdspDev->intpl_hi_thr = intpl.int_hi_thr;
			
			break;
		}


		case MSG_CDSP_RAW_SPEF:
		{
			gpCdspIntpl_t intpl;
			CDSP_RAW_SPECIAL_MODE special_mode;

			nRet = copy_from_user((void*)&special_mode, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->rawFmtFlag) {
				gpCdspGetIntpl(&intpl);
				intpl.rawspecmode = special_mode;
				gpCdspSetIntpl(CdspDev->imgWidth, &intpl);

				DEBUG("RAW special mode = %d\r\n", special_mode);
			}			
		}
		break;

		case MSG_CDSP_EDGE:
		{
			gpCdspEdge_t edge;

			nRet = copy_from_user((void*)&edge, (void __user*)ctrl->value, sizeof(edge));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			gpCdspSetEdge(CdspDev->rawFmtFlag, &edge);
		}
		break;

		case MSG_CDSP_EDGE_AMPGA_DAY:
		{
			int edge_ampga;

			nRet = copy_from_user((void*)&edge_ampga, (void __user*)ctrl->value, sizeof(edge_ampga));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			CdspDev->edge_day = edge_ampga;
			DEBUG("Day Edge AMPGA= %d\r\n", edge_ampga);

			gp_cdsp_edge_level_init();
		}
		break;

		case MSG_CDSP_EDGE_AMPGA_NIGHT:
		{
			int edge_ampga;

			nRet = copy_from_user((void*)&edge_ampga, (void __user*)ctrl->value, sizeof(edge_ampga));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			CdspDev->edge_night = edge_ampga;
			DEBUG("Night Edge AMPGA= %d\r\n", edge_ampga);

			gp_cdsp_edge_level_init();
		}
		break;

		case MSG_CDSP_COLOR_MATRIX:
		{
			gpCdspCorMatrix_t matrix;
			
			nRet = copy_from_user((void*)&matrix, (void __user*)ctrl->value, sizeof(matrix));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(CdspDev->rawFmtFlag) {
				matrix.a11 = CdspDev->sensor_cdsp.color_matrix1[0];
				matrix.a12 = CdspDev->sensor_cdsp.color_matrix1[1];
				matrix.a13 = CdspDev->sensor_cdsp.color_matrix1[2];
				matrix.a21 = CdspDev->sensor_cdsp.color_matrix1[3];
				matrix.a22 = CdspDev->sensor_cdsp.color_matrix1[4];
				matrix.a23 = CdspDev->sensor_cdsp.color_matrix1[5];
				matrix.a31 = CdspDev->sensor_cdsp.color_matrix1[6];
				matrix.a32 = CdspDev->sensor_cdsp.color_matrix1[7];
				matrix.a33 = CdspDev->sensor_cdsp.color_matrix1[8];
				gpCdspSetColorMatrix(&matrix);
			}
		}
		break;

		case MSG_CDSP_POSWB_RGB2YUV:
		{
			gpCdspRgb2Yuv_t rgb2yuv;
			
			nRet = copy_from_user((void*)&rgb2yuv, (void __user*)ctrl->value, sizeof(rgb2yuv));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(CdspDev->rawFmtFlag) {
				gpCdspSetRgbToYuv(&rgb2yuv);
			}
			break;
		}

		case MSG_CDSP_YUV_INSERT:
		{
			gpCdspYuvInsert_t yuv_insert;

			nRet = copy_from_user((void*)&yuv_insert, (void __user*)ctrl->value, sizeof(yuv_insert));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			gpCdspSetYuv444Insert(&yuv_insert);
			break;
		}

		case MSG_CDSP_YUV_HAVG:
		{
			gpCdspYuvHAvg_t yuv_havg;

			nRet = copy_from_user((void*)&yuv_havg, (void __user*)ctrl->value, sizeof(yuv_havg));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			gpCdspSetYuvHavg(&yuv_havg);
			break;
		}

		case MSG_CDSP_SPEC_MODE:
		{
			gpCdspSpecMod_t spec_mode;

			nRet = copy_from_user((void*)&spec_mode, (void __user*)ctrl->value, sizeof(spec_mode));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			gpCdspSetSpecialMode(&spec_mode);
			break;
		}

		case MSG_CDSP_SAT_HUE:
		{
			gpCdspSpecMod_t spec_mode;
			gpCdspSatHue_t sat_hue;

			nRet = copy_from_user((void*)&sat_hue, (void __user*)ctrl->value, sizeof(sat_hue));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
	
			spec_mode.yuvspecmode = SP_YUV_YbYcSatHue;
			gpCdspSetSpecialMode(&spec_mode);
			gpCdspSetSatHue(&sat_hue);
			break;
		}


		case MSG_CDSP_SAT_HUE_DAY:
		{
			gpCdspSpecMod_t spec_mode;
			gpCdspSatHue_t sat_hue;

			nRet = copy_from_user((void*)&sat_hue, (void __user*)ctrl->value, sizeof(sat_hue));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
	
			spec_mode.yuvspecmode = SP_YUV_YbYcSatHue;
			gpCdspSetSpecialMode(&spec_mode);
			gpCdspSetSatHue(&sat_hue);

			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			CdspDev->sat_yuv_level[0][0] = sat_hue.y_offset;
			CdspDev->sat_yuv_level[0][1] = sat_hue.u_offset;
			CdspDev->sat_yuv_level[0][2] = sat_hue.v_offset;
			CdspDev->sat_yuv_level[0][3] = sat_hue.y_scale;
			CdspDev->sat_yuv_level[0][4] = sat_hue.u_scale;
			CdspDev->sat_yuv_level[0][5] = sat_hue.v_scale;		

			gp_cdsp_sat_contrast_init();
			up(&CdspDev->aeawb_sem);
			break;
		}

		case MSG_CDSP_SAT_HUE_NIGHT:
		{
			gpCdspSpecMod_t spec_mode;
			gpCdspSatHue_t sat_hue;

			nRet = copy_from_user((void*)&sat_hue, (void __user*)ctrl->value, sizeof(sat_hue));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			CdspDev->sat_yuv_level[3][0] = sat_hue.y_offset;
			CdspDev->sat_yuv_level[3][1] = sat_hue.u_offset;
			CdspDev->sat_yuv_level[3][2] = sat_hue.v_offset;
			CdspDev->sat_yuv_level[3][3] = sat_hue.y_scale;
			CdspDev->sat_yuv_level[3][4] = sat_hue.u_scale;
			CdspDev->sat_yuv_level[3][5] = sat_hue.v_scale;

			gp_cdsp_sat_contrast_init();
			up(&CdspDev->aeawb_sem);
			break;
		}

		

		case MSG_CDSP_SUPPRESSION:
		{
			gpCdspEdge_t edge;

			nRet = copy_from_user((void*)&CdspDev->suppression, (void __user*)ctrl->value, sizeof(CdspDev->suppression));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->rawFmtFlag == 0) {
				gpCdspGetEdge(&edge);
				gpCdspSetSuppression(CdspDev->imgRbWidth, &CdspDev->suppression, &edge);
			}
			break;
		}

		case MSG_CDSP_NEWDENOISE:
		{
			//gpCdspEdge_t edge;
			gpCdspNewDenoise_t NewDenoise;
			
			nRet = copy_from_user((void*)&NewDenoise, (void __user*)ctrl->value, sizeof(NewDenoise));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			//gpCdspGetEdge(&edge);
			gpCdspSetNewDenoise(&NewDenoise);
			break;
		}	

		case MSG_CDSP_RAW_WIN:
		{
			gpCdspRawWin_t raw_win;

			nRet = copy_from_user((void*)&raw_win, (void __user*)ctrl->value, sizeof(raw_win));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(CdspDev->scale.crop_en) {
				w = CdspDev->scale.crop_hsize;
				h = CdspDev->scale.crop_vsize;
			} else {
				w = CdspDev->imgWidth;
				h = CdspDev->imgHeight;
			}
			

			
			gpCdspSetRawWin(w, h, &raw_win);
			gpCdspGetRawWin(&raw_win);
			
			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			gp_cdsp_awb_set_cnt_thr(CdspDev->awb_workmem, CdspDev->imgWidth, CdspDev->imgHeight, raw_win.subsample, 0);
			gp_cdsp_ae_set_lum_bound(CdspDev->ae_workmem, CdspDev->imgWidth, CdspDev->imgHeight, raw_win.hwdsize, raw_win.vwdsize);
			up(&CdspDev->aeawb_sem);
			
			break;
		}

		case MSG_CDSP_AE_WIN:
		{
			gpCdspAE_t ae;
			//gpCdspRawWin_t raw_win;
			

			nRet = copy_from_user((void*)&ae, (void __user*)ctrl->value, sizeof(ae));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			gp_cdsp_ae_set_meter(ae.ae_meter, CdspDev->ae_workmem);
			up(&CdspDev->aeawb_sem);
			
			//gpCdspGetRawWin(&raw_win);
			gpCdspSetAE(&ae);
			break;
		}

		case MSG_CDSP_AF_WIN:
		{
			gpCdspAF_t af;

			nRet = copy_from_user((void*)&af, (void __user*)ctrl->value, sizeof(af));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(CdspDev->scale.crop_en) {
				w = CdspDev->scale.crop_hsize;
				h = CdspDev->scale.crop_vsize;
			} else {
				w = CdspDev->imgWidth;
				h = CdspDev->imgHeight;
			}
			
			gpCdspSetAF(w, h, &af);
			break;
		}

		case MSG_CDSP_AWB_WIN:
		{
			gpCdspAWB_t awb;

			nRet = copy_from_user((void*)&awb, (void __user*)ctrl->value, sizeof(awb));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			awb.awbwinthr = CdspDev->sensor_cdsp.awb_thr[0];
			
			awb.sindata = CdspDev->sensor_cdsp.awb_thr[1];
			awb.cosdata = CdspDev->sensor_cdsp.awb_thr[2];

			awb.Ythr0 = CdspDev->sensor_cdsp.awb_thr[3];
			awb.Ythr1 = CdspDev->sensor_cdsp.awb_thr[4];
			awb.Ythr2 = CdspDev->sensor_cdsp.awb_thr[5];
			awb.Ythr3 = CdspDev->sensor_cdsp.awb_thr[6];

			awb.UL1N1 = CdspDev->sensor_cdsp.awb_thr[7]; 
			awb.UL1P1 = CdspDev->sensor_cdsp.awb_thr[8]; 
			awb.VL1N1 = CdspDev->sensor_cdsp.awb_thr[9]; 
			awb.VL1P1 = CdspDev->sensor_cdsp.awb_thr[10]; 

			awb.UL1N2 = CdspDev->sensor_cdsp.awb_thr[11]; 
			awb.UL1P2 = CdspDev->sensor_cdsp.awb_thr[12]; 
			awb.VL1N2 = CdspDev->sensor_cdsp.awb_thr[13]; 
			awb.VL1P2 = CdspDev->sensor_cdsp.awb_thr[14]; 

			awb.UL1N3 = CdspDev->sensor_cdsp.awb_thr[15]; 
			awb.UL1P3 = CdspDev->sensor_cdsp.awb_thr[16]; 
			awb.VL1N3 = CdspDev->sensor_cdsp.awb_thr[17]; 
			awb.VL1P3 = CdspDev->sensor_cdsp.awb_thr[18]; 
			
			gpCdspSetAWB(&awb);
			break;
		}

		case MSG_CDSP_WBGAIN2:
		{
			gpCdspWbGain2_t wbgain2;

			nRet = copy_from_user((void*)&wbgain2, (void __user*)ctrl->value, sizeof(wbgain2));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			gpCdspSetWBGain2(&wbgain2);
			break;
		}

		case MSG_CDSP_HISTGM:
		{
			gpCdspHistgm_t histgm;

			nRet = copy_from_user((void*)&histgm, (void __user*)ctrl->value, sizeof(histgm));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			gpCdspSetHistgm(&histgm);
			break;
		}

		case MSG_CDSP_TARGET_AE:
		{
			int targetLum;
			nRet = copy_from_user((void*)&targetLum, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			CdspDev->ae_target = targetLum;
			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			gp_cdsp_ae_set_target_lum(CdspDev->ae_workmem, targetLum);
			up(&CdspDev->aeawb_sem);
			DEBUG("AE target = %d\r\n", targetLum);
			break;
		}

		case MSG_CDSP_TARGET_AE_NIGHT:
		{
			int targetLum;
			nRet = copy_from_user((void*)&targetLum, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			CdspDev->ae_target_night = targetLum;
			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			gp_cdsp_ae_set_target_lum_night(CdspDev->ae_workmem, targetLum);
			up(&CdspDev->aeawb_sem);

			DEBUG("AE target night = %d\r\n", targetLum);
			break;
		}

		case MSG_CDSP_WB_MODE:
		{
			int wbmode;
			
			nRet = copy_from_user((void*)&wbmode, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			gp_cdsp_awb_set_mode(CdspDev->awb_workmem, wbmode);
			up(&CdspDev->aeawb_sem);
			break;
		}


		case MSG_CDSP_EV:
		{
			int val;
			nRet = copy_from_user((void*)&val, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			DEBUG("Set EV idx = %d\r\n", val);
			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			gp_cdsp_ae_set_ev(CdspDev->ae_workmem, val);
			up(&CdspDev->aeawb_sem);
			break;
		}

		case MSG_CDSP_BACKLIGHT_DETECT:
		{
			int val;
			nRet = copy_from_user((void*)&val, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			DEBUG("Set backlight = %d\r\n", val);
			if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
			gp_cdse_ae_set_backlight_detect(CdspDev->ae_workmem, val);
			up(&CdspDev->aeawb_sem);
			break;
		}

		case MSG_CDSP_ISO:
		{
			int iso;
			struct v4l2_subdev *sd;
			struct v4l2_control csi_ctrl;
			sensor_exposure_t *seinfo;
			
			nRet = copy_from_user((void*)&iso, (void __user*)ctrl->value, sizeof(int));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(iso != ISO_AUTO) {
				DEBUG("Set ISO = %d00\r\n", iso);
			} else {
				DEBUG("Set AUTO ISO\r\n");
			}
			
			sd = CdspDev->sd;
			csi_ctrl.id = V4L2_CID_EXPOSURE;
			
			
			nRet = sd->ops->core->g_ctrl(sd, &csi_ctrl);
			if(nRet < 0) {
				CdspDev->sensor_status = CMS_I2C_TIMEOUT;
			}
			
			seinfo = (sensor_exposure_t *) csi_ctrl.value;
			DEBUG("Set ISO[addr = %x]: time = 0x%x, a_gain = 0x%x, d_gain = 0x%x\n", csi_ctrl.value, seinfo->time, seinfo->analog_gain, seinfo->digital_gain);
			if(down_interruptible(&CdspDev->aeawb_sem) != 0) {
				return -ERESTARTSYS;
			}
			gp_cdsp_ae_set_iso(CdspDev->ae_workmem, iso, seinfo);
			up(&CdspDev->aeawb_sem);
			
			csi_ctrl.value = (int) seinfo;
			nRet = sd->ops->core->s_ctrl(sd, &csi_ctrl);
			if(nRet < 0) {
				CdspDev->sensor_status = CMS_I2C_TIMEOUT;
			}
			
			
			break;
		}

		default:
		{
			struct v4l2_subdev *sd;
			
			sd = CdspDev->sd;
			nRet = sd->ops->core->s_ctrl(sd, ctrl);
			if(nRet < 0) {
				CdspDev->sensor_status = CMS_I2C_TIMEOUT;
			}
			break;
		}
	}

__return:
	return nRet;
}
EXPORT_SYMBOL(gp_cdsp_s_ctrl);

static int 
gp_cdsp_s_capture(
	gpCsiCapture_t *pCap
)
{
	int i, ret;
	unsigned int addr;
	struct v4l2_format bkfmt, fmt;
	struct v4l2_subdev *sd;

	DEBUG("%s\n", __FUNCTION__);
	memset(&bkfmt, 0, sizeof(bkfmt));
	memset(&fmt, 0, sizeof(fmt));
	
	/* 1.csi disable and backup preview format */
#if CDSP_CHK_MIPI_EN == 1	
	if(strcmp("MIPI", CdspDev->port) == 0) {
		gp_mipi_set_irq(0);
	}
#endif
	gpHalCdspSetIntEn(DISABLE, CDSP_OVERFOLW|CDSP_EOF);
	gpHalCdspDataSource(C_CDSP_SDRAM);
	sd = CdspDev->sd;
	ret = sd->ops->video->g_fmt(sd, &bkfmt);
	if(ret < 0) {
		DEBUG("backup g_fmt Err\n");
		return -1;
	}

#if 1
	/* 1-1.clear out_que and line after in_que */
	for(i=0; i<CdspDev->rbuf.count; i++) {
		int j, in_que, out_que;
		
		out_que = CdspDev->out_que[i];
		if(out_que != 0xFF) {
			CdspDev->out_que[i] = 0xFF;
			CdspDev->bf[out_que].flags = V4L2_BUF_FLAG_QUEUED;
			DEBUG("out_que = 0x%x\n", out_que);

			for(j=0; j<CdspDev->rbuf.count; j++) {
				in_que = CdspDev->in_que[j];
				DEBUG("in_que[%d] = 0x%x\n", j, in_que);
				if(in_que == 0xFF) {
					CdspDev->in_que[j] = out_que;
					break;
				}
			}
		}	
	}
#else
	/* 1-1.discard all done buffer */
	for(i=0; i<CdspDev->rbuf.count; i++){
		CdspDev->out_que[i] = 0xFF;
		CdspDev->in_que[i] = 0xFF;
		CdspDev->bf[i].flags = V4L2_BUF_FLAG_QUEUED;
	}

	for(i=0; i<CdspDev->rbuf.count; i++){
		CdspDev->in_que[i]	= i;
	}
#endif

	/* 2.change to capture */
	addr = (unsigned int)gp_user_va_to_pa((unsigned short *)pCap->buffaddr);	
#if (C_DMA_CH == 1) || (C_DMA_CH == 2)
#if _SET_YUV_PATH_
#if (C_DMA_CH == 1)
	gpHalCdspSetYuvBuffA(pCap->width, pCap->height, addr);
	gpHalCdspSetDmaBuff(RD_A_WR_A);
#else
	gpHalCdspSetYuvBuffB(pCap->width, pCap->height, addr);
	gpHalCdspSetDmaBuff(RD_B_WR_B);
#endif
#else
	gpHalCdspSetRawBuff(addr);
	gpHalCdspSetRawBuffSize(pCap->width, pCap->height, 0, RAW_BIT);
	gpHalCdspSetReadBackSize(0, 0, pCap->width, pCap->height);
#endif

#else
	gpHalCdspSetYuvBuffA(pCap->width, pCap->height, addr);
	gpHalCdspSetYuvBuffB(pCap->width, pCap->height, addr);
	gpHalCdspSetDmaBuff(AUTO_SWITCH);
#endif
	
	fmt.fmt.pix.width = pCap->width;
	fmt.fmt.pix.height = pCap->height;
	fmt.fmt.pix.pixelformat = pCap->pixelformat;
	fmt.fmt.pix.priv = pCap->priv;	
	if(gp_cdsp_s_fmt(&fmt) < 0) {
		DEBUG("capture gp_cdsp_s_fmt Err\n");
		return -1;
	}

	ret = sd->ops->video->s_fmt(sd, &fmt);
	if(ret < 0) {
		DEBUG("capture s_fmt Err\n");
		return -1;
	}
		
	/* 3.irq enable and wait N frame */
	CdspDev->cdsp_feint_flag = 0;
	CdspDev->CapCnt = pCap->waitcnt;
	if(CdspDev->CapCnt < 1) {
		CdspDev->CapCnt = 2;
	}
	
	gpHalCdspDataSource(CdspDev->imgSrc);
	gpHalCdspSetIntEn(ENABLE, CDSP_OVERFOLW|CDSP_EOF);
#if CDSP_CHK_MIPI_EN == 1	
	if(strcmp("MIPI", CdspDev->port) == 0) {
		gp_mipi_set_irq(1);
	}
#endif	
	wait_event_interruptible(CdspDev->cdsp_wait_queue, (CdspDev->cdsp_feint_flag != 0));
	
	/* 4. resume to preview */
#if CDSP_CHK_MIPI_EN == 1	
	if(strcmp("MIPI", CdspDev->port) == 0) {
		gp_mipi_set_irq(0);
	}
#endif
	gpHalCdspSetIntEn(DISABLE, CDSP_OVERFOLW|CDSP_EOF);
	gpHalCdspDataSource(C_CDSP_SDRAM);
	CdspDev->cdsp_feint_flag = 0;

#if (C_DMA_CH == 1) || (C_DMA_CH == 2)	
#if _SET_YUV_PATH_
#if C_DMA_CH == 1
	gpHalCdspSetYuvBuffA(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
	gpHalCdspSetDmaBuff(RD_A_WR_A);
#else
	gpHalCdspSetYuvBuffB(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
	gpHalCdspSetDmaBuff(RD_B_WR_B);
#endif
#else
	gpHalCdspSetRawBuff(CdspDev->bfaddr[CdspDev->in_que[0]]);
	gpHalCdspSetRawBuffSize(CdspDev->imgRbWidth, CdspDev->imgRbHeight, 0, RAW_BIT);
	gpHalCdspSetReadBackSize(0, 0, CdspDev->imgRbWidth, CdspDev->imgRbHeight);
#endif

#else 
	CdspDev->abBufFlag = 0;
	CdspDev->aInQueIdx = CdspDev->in_que[0];
	CdspDev->bInQueIdx = CdspDev->in_que[1];
	gpHalCdspSetYuvBuffA(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
	gpHalCdspSetYuvBuffB(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[1]]);
	gpHalCdspSetDmaBuff(AUTO_SWITCH);
#endif

	if(gp_cdsp_s_fmt(&bkfmt) < 0) {
		DEBUG("resume gp_cdsp_s_fmt Err\n");
		return -1;
	}
	
	ret = sd->ops->video->s_fmt(sd, &bkfmt);
	if(ret < 0) {
		DEBUG("resume s_fmt Err\n");
		return -1;
	}

	gpHalCdspDataSource(CdspDev->imgSrc);
	gpHalCdspSetIntEn(ENABLE, CDSP_OVERFOLW|CDSP_EOF);
#if CDSP_CHK_MIPI_EN == 1	
	if(strcmp("MIPI", CdspDev->port) == 0) {
		gp_mipi_set_irq(1);
	}
#endif
	return 0;
}			


static long
gp_cdsp_ioctl(
	struct file *filp,
	unsigned int cmd,
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
	struct v4l2_interface Interface;
	v4l2_std_id std;
	gpCsiCapture_t capture;
	unsigned char idx;
	unsigned short *addr;
	int i, nRet = 0;
	sensor_calibration_t calibrate;

	if(down_interruptible(&CdspDev->sem) != 0) {
		return -ERESTARTSYS;
	}

	switch(cmd)
	{
		case VIDIOC_QUERYCAP:
			nRet = copy_to_user((void __user*)arg, (void*)&g_cdsp_cap, sizeof(g_cdsp_cap));
			break;
		
		case VIDIOC_ENUMINPUT:
		{
			callbackfunc_t	*cb;
			char			*port;
			
			nRet = copy_from_user((void*)&in, (void __user*)arg, sizeof(struct v4l2_input));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = gp_get_sensorinfo(in.index, (int*)&sd, (int*)&cb, (int*)&port, (int*)&sensor);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			DEBUG("Device[%d]:%s\n", in.index, sd->name);
			in.type = V4L2_INPUT_TYPE_CAMERA;
			strcpy(in.name, sd->name);

			nRet = copy_to_user((void __user*)arg, (void*)&in, sizeof(struct v4l2_input));
			break;
		}
		case VIDIOC_S_CALIBRATE:
		{
			if(CdspDev->sensor_cdsp.ob)
				kfree(CdspDev->sensor_cdsp.ob);
			if(CdspDev->sensor_cdsp.lenscmp)
				kfree(CdspDev->sensor_cdsp.lenscmp);
			if(CdspDev->sensor_cdsp.wb_gain)
				kfree(CdspDev->sensor_cdsp.wb_gain);
			if(CdspDev->sensor_cdsp.gamma1)
				kfree(CdspDev->sensor_cdsp.gamma1);
			if(CdspDev->sensor_cdsp.color_matrix1)
				kfree(CdspDev->sensor_cdsp.color_matrix1);
			if(CdspDev->sensor_cdsp.gamma2)
				kfree(CdspDev->sensor_cdsp.gamma2);
			if(CdspDev->sensor_cdsp.color_matrix2)
				kfree(CdspDev->sensor_cdsp.color_matrix2);
			memset(&CdspDev->sensor_cdsp, 0x0, sizeof(CdspDev->sensor_cdsp));
		
			copy_from_user((void*)&calibrate, (void __user*)arg, sizeof(sensor_calibration_t));
			
			CdspDev->sensor_cdsp.ob = kmalloc(calibrate.ob_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.ob, (void __user*)calibrate.ob, calibrate.ob_size);
			CdspDev->sensor_cdsp.lenscmp = kmalloc(calibrate.lenscmp_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.lenscmp, (void __user*)calibrate.lenscmp, calibrate.lenscmp_size);
			CdspDev->sensor_cdsp.wb_gain = kmalloc(calibrate.wbgain_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.wb_gain, (void __user*)calibrate.wb_gain, calibrate.wbgain_size);
			CdspDev->sensor_cdsp.gamma1 = kmalloc(calibrate.gamma1_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.gamma1, (void __user*)calibrate.gamma1, calibrate.gamma1_size);
			CdspDev->sensor_cdsp.color_matrix1 = kmalloc(calibrate.matrix1_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.color_matrix1, (void __user*)calibrate.color_matrix1, calibrate.matrix1_size);
			CdspDev->sensor_cdsp.gamma2 = kmalloc(calibrate.gamma2_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.gamma2, (void __user*)calibrate.gamma2, calibrate.gamma2_size);
			CdspDev->sensor_cdsp.color_matrix2 = kmalloc(calibrate.matrix2_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.color_matrix2, (void __user*)calibrate.color_matrix2, calibrate.matrix2_size);
			CdspDev->sensor_cdsp.awb_thr = kmalloc(calibrate.awb_size, GFP_KERNEL);
			copy_from_user((void*)CdspDev->sensor_cdsp.awb_thr, (void __user*)calibrate.awb_thr, calibrate.awb_size);
			break;
		}
		
		case VIDIOC_S_INPUT:
			if(arg == CdspDev->sdidx) {
				DEBUG("The Same Device\n");
			}
			
			nRet = gp_get_sensorinfo(arg, (int*)&CdspDev->sd, (int*)&CdspDev->cb_func, (int*)&CdspDev->port, (int*)&sensor);
			if(nRet < 0) { 
				RETURN(-EINVAL);
			}

			CdspDev->sdidx = arg;
			if(strcmp("PORT0", CdspDev->port) == 0) {
				CdspDev->imgSrc = C_CDSP_FRONT;
				DEBUG("PORT0(FRONT)\n");
			} else if(strcmp("MIPI", CdspDev->port) == 0) {
				CdspDev->imgSrc = C_CDSP_MIPI;
				DEBUG("MIPI\n");
			} else {
				DEBUG("DrvPortError!\n");
				RETURN(-EINVAL);
			}

			if(CdspDev->cb_func->powerctl) {
				CdspDev->cb_func->powerctl(1);
			}
			if(CdspDev->cb_func->standby) {
				CdspDev->cb_func->standby(0); 
			}
			if(CdspDev->cb_func->reset) {
				CdspDev->cb_func->reset(0);
				mdelay(10);
				CdspDev->cb_func->reset(1);
			}
			if(CdspDev->cb_func->set_port) {
				CdspDev->cb_func->set_port(CdspDev->port);
			}

			/* open mclk, some sensor need mclk befor init */
			CdspDev->MclkEn = 1;
			CdspDev->Mclk.mclk_out = sensor->fmt[0].mclk;
			CdspDev->Mclk.mclk_sel = sensor->fmt[0].mclk_src;
			gpCdspSetMclkout(ENABLE, &CdspDev->Mclk);
			
			// for IQ turning
			
	/*		if(sensor->fmt[0].cdsp_calibration) {
				CdspDev->sensor_cdsp.ob = sensor->fmt[0].cdsp_calibration->ob;
				CdspDev->sensor_cdsp.lenscmp= sensor->fmt[0].cdsp_calibration->lenscmp;
				CdspDev->sensor_cdsp.wb_gain= sensor->fmt[0].cdsp_calibration->wb_gain;
				CdspDev->sensor_cdsp.gamma1  = sensor->fmt[0].cdsp_calibration->gamma1;
				CdspDev->sensor_cdsp.color_matrix1 = sensor->fmt[0].cdsp_calibration->color_matrix1;
				CdspDev->sensor_cdsp.gamma2  = sensor->fmt[0].cdsp_calibration->gamma2;
				CdspDev->sensor_cdsp.color_matrix2 = sensor->fmt[0].cdsp_calibration->color_matrix2;
				CdspDev->sensor_cdsp.awb_thr = sensor->fmt[0].cdsp_calibration->awb_thr;
			}*/

			sd = CdspDev->sd;
			if(sd && sd->ops->core) {
				struct v4l2_control csi_ctrl;
				sensor_exposure_t *seInfo;
				
				nRet = sd->ops->core->reset(sd, 0);
				nRet = sd->ops->core->init(sd, 0);
				if(nRet < 0) {
					DEBUG("sensor init fail\n");
					RETURN(-EINVAL);
				}
				
				csi_ctrl.id = V4L2_CID_EXPOSURE;
				nRet = sd->ops->core->g_ctrl(sd, &csi_ctrl);
				if(nRet < 0) {
					CdspDev->sensor_status = CMS_I2C_TIMEOUT;
					RETURN(nRet);
				}
				
				seInfo = (sensor_exposure_t	*) csi_ctrl.value;
				if(seInfo) {
					gp_cdsp_ae_set_sensor_exp_time(CdspDev->ae_workmem, seInfo);				
					CdspDev->sensor_gain_thr = seInfo->max_analog_gain >> 2;
					CdspDev->night_gain_thr = CdspDev->sensor_gain_thr << 2;
					if(CdspDev->sensor_gain_thr < (1*256)) CdspDev->sensor_gain_thr = 1*256;		
					if(CdspDev->night_gain_thr < (1*256)) CdspDev->night_gain_thr = 1*256;
					if(CdspDev->night_gain_thr > seInfo->max_analog_gain) CdspDev->night_gain_thr = seInfo->max_analog_gain;
					

					CdspDev->sensor_time_thr = seInfo->max_time >> 1;
					if(CdspDev->sensor_time_thr < (8)) CdspDev->sensor_time_thr = 8;

					DEBUG("Sensor: Gain thr = 0x%x, time thr = 0x%x, night_gain_thr = 0x%x\r\n", 
							CdspDev->sensor_gain_thr, CdspDev->sensor_time_thr, CdspDev->night_gain_thr);
				} 
			}

			CdspDev->getSensorInfo = 1;
			break;
		
		case VIDIOC_G_INPUT:
			RETURN(CdspDev->sdidx);
			break;
		
		case VIDIOC_S_FMT:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			nRet = copy_from_user((void*)&fmt, (void __user*)arg, sizeof(struct v4l2_format));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = gp_cdsp_s_fmt(&fmt);
			if(nRet < 0) {
				RETURN(nRet);
			}

			sd = CdspDev->sd;
			nRet = sd->ops->video->s_fmt(sd, &fmt);
			if(nRet >= 0) {
				CdspDev->sensor_status = CMS_RUNING;
			} else {
				CdspDev->sensor_status = CMS_I2C_FAIL;
			}
			break;
		
		case VIDIOC_G_FMT:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->g_fmt(sd, &fmt);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			nRet = copy_to_user((void __user*)arg, (void*)&fmt, sizeof(struct v4l2_format));
			break;
		
		case VIDIOC_TRY_FMT:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			nRet = copy_from_user((void*)&fmt, (void __user*)arg, sizeof(struct v4l2_format));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->try_fmt(sd, &fmt);
			break;
		
		case VIDIOC_ENUM_FMT:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&fmtd, (void __user*)arg, sizeof(struct v4l2_fmtdesc));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->enum_fmt(sd, &fmtd);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = copy_to_user((void __user*)arg, (void*)&fmtd, sizeof(struct v4l2_fmtdesc));
			break;
		
		case VIDIOC_QUERYCTRL:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&qc, (void __user*)arg, sizeof(struct v4l2_queryctrl));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->core->queryctrl(sd, &qc);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = copy_to_user((void __user*)arg, (void*)&qc, sizeof(struct v4l2_queryctrl));
			break;
		
		case VIDIOC_G_CTRL:
			//if(CdspDev->sdidx == NO_INPUT) {
			//	DEBUG("please set input first\n");
			//	RETURN(-EINVAL);
			//}

			nRet = copy_from_user((void*)&ctrl, (void __user*)arg, sizeof(struct v4l2_control));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = gp_cdsp_g_ctrl(&ctrl);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = copy_to_user((void __user*)arg, (void*)&ctrl, sizeof(struct v4l2_control));
			break;
		
		case VIDIOC_S_CTRL:
			//if(CdspDev->sdidx == NO_INPUT) {
			//	DEBUG("please set input first\n");
			//	RETURN(-EINVAL);
			//}

			nRet = copy_from_user((void*)&ctrl, (void __user*)arg, sizeof(struct v4l2_control));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			nRet = gp_cdsp_s_ctrl(&ctrl);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = copy_to_user((void __user*)arg, (void*)&ctrl, sizeof(struct v4l2_control));
			break;
		
		case VIDIOC_S_INTERFACE:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&Interface, (void __user*)arg, sizeof(Interface));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(Interface.Interface == CCIR601) {
				gpHalFrontDataTransMode(0);	//ccir601 interfcae
			} else {
				gpHalFrontDataTransMode(1); //ccir656 interface
			}

			gpHalFrontSyncPolaritySet(Interface.HsyncAct ? 0 : 1, 
									Interface.VsyncAct ? 0 : 1,
									CdspDev->SyncFlag);

			if(Interface.FmtOut != YUVOUT) {
				DERROR("Only Support YUVOUT\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->ext->s_interface(sd, &Interface);
			break;
		
		case VIDIOC_G_INTERFACE:
			gpHalFrontSyncPolarityGet(&Interface.HsyncAct, &Interface.VsyncAct, (UINT8 *)&CdspDev->SyncFlag);
			Interface.SampleEdge = MODE_POSITIVE_EDGE;
			Interface.Interlace = MODE_NONE_INTERLACE;
			Interface.Field = FIELD_ODDL;
			nRet = copy_to_user((void __user*)arg, (void*)&Interface, sizeof(Interface));
			break;
		
		case VIDIOC_S_MCLK:	
			nRet = copy_from_user((void*)&CdspDev->Mclk, (void __user*)arg, sizeof(gpCsiMclk_t));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(CdspDev->Mclk.mclk_out == 0) {
				CdspDev->MclkEn = 0;
				gpCdspSetMclkout(DISABLE, NULL);
			} else {
				CdspDev->MclkEn = 1;
				gpCdspSetMclkout(ENABLE, &CdspDev->Mclk);
			}
			CdspDev->SyncFlag = CdspDev->Mclk.prvi;
			break;
		
		case VIDIOC_G_MCLK:
			nRet = copy_to_user((void __user*)arg, (void*)&CdspDev->Mclk, sizeof(gpCsiMclk_t));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			break;
		
		case VIDIOC_CROPCAP:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&cc, (void __user*)arg, sizeof(struct v4l2_cropcap));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->cropcap(sd, &cc);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			nRet = copy_to_user((void __user*)arg, (void*)&cc, sizeof(struct v4l2_cropcap));
			break;
		
		case VIDIOC_G_CROP:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&crop, (void __user*)arg, sizeof(struct v4l2_crop));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->g_crop(sd, &crop);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			nRet = copy_to_user((void __user*)arg, (void*)&crop, sizeof(struct v4l2_crop));
			break;

		case VIDIOC_S_CROP:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&crop, (void __user*)arg, sizeof(struct v4l2_crop));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->s_crop(sd, &crop);
			break;

		case VIDIOC_G_PARM:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&param, (void __user*)arg, sizeof(struct v4l2_streamparm));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->g_parm(sd, &param);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			nRet = copy_to_user((void __user*)arg, (void*)&param, sizeof(struct v4l2_streamparm));
			break;
		
		case VIDIOC_S_PARM:
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&param, (void __user*)arg, sizeof(struct v4l2_streamparm));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->s_parm(sd, &param);
			break;
		
		case VIDIOC_REQBUFS:
			nRet = copy_from_user((void*)&(CdspDev->rbuf), (void __user*)arg, sizeof(struct v4l2_requestbuffers));
			nRet = check_rqbuf_type();
			break;
		
		case VIDIOC_STREAMON:
			if(arg == (unsigned long)NULL) {
				unsigned char ae, awb, hold;
				
				if(CdspDev->in_que[0] == 0xFF) {
					DEBUG("No buffer in Que\n");
					RETURN(-EINVAL);
				}

				/* resume sensor */
				if(CdspDev->imgSrc != C_CDSP_SDRAM) {
					if(CdspDev->MclkEn == 0) {
						CdspDev->MclkEn = 1;
						gpCdspSetMclkout(ENABLE, &CdspDev->Mclk);

						if(CdspDev->cb_func->powerctl) {
							CdspDev->cb_func->powerctl(1);
						}
						if(CdspDev->cb_func->standby) {
							CdspDev->cb_func->standby(0); 
						}
						CdspDev->sd->ops->ext->resume(CdspDev->sd);
					}
				}

				// enable sensor set exp tasklet
				tasklet_enable(&sensor_set_exp_tasklet);

				//ytliao 2014.05 wake up awb task here
				if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
				gp_cdsp_aeawb_reset(CdspDev->ae_workmem, CdspDev->awb_workmem); // add by Comi 2014.07.24
				up(&CdspDev->aeawb_sem);

				CdspDev->getSensorInfo |= 0x20;
				DEBUG("getSensorInfo = 0x%x\n", CdspDev->getSensorInfo);
				CdspDev->ae_awb_flag = 0;
				
				gpHalCdspGetAEEn(&ae, &hold);
				gpHalCdspGetAEEn(&awb, &hold);
				if(ae || awb) {
					CdspDev->ae_awb_task = kthread_create(gp_ae_awb_process, 0, "AE_AWB_CTRL");
					if (IS_ERR(CdspDev->ae_awb_task)) {
						CdspDev->ae_awb_task = NULL;
					}
					else {
						wake_up_process(CdspDev->ae_awb_task);
					}
				} else {
					CdspDev->ae_awb_task = NULL;
				}
				
			#if (C_DMA_CH == 1) || (C_DMA_CH == 2)
			#if _SET_YUV_PATH_
			#if (C_DMA_CH == 1)
				//CdspDev->aInQueIdx = CdspDev->in_que[0];
				//CdspDev->bInQueIdx = CdspDev->in_que[1];
				//memset((void *)gp_chunk_va(CdspDev->bfaddr[CdspDev->aInQueIdx]), 0x55, CdspDev->imgRbWidth*CdspDev->imgRbHeight*2);
				//memset((void *)gp_chunk_va(CdspDev->bfaddr[CdspDev->bInQueIdx]), 0xAA, CdspDev->imgRbWidth*CdspDev->imgRbHeight*2);
				gpHalCdspSetYuvBuffA(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
				gpHalCdspSetDmaBuff(RD_A_WR_A);
			#else
				gpHalCdspSetYuvBuffB(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
				gpHalCdspSetDmaBuff(RD_B_WR_B);
			#endif
			#else
				gpHalCdspSetRawBuff(CdspDev->bfaddr[CdspDev->in_que[0]]);
				gpHalCdspSetRawBuffSize(CdspDev->imgRbWidth, CdspDev->imgRbHeight, 0, RAW_BIT);
				gpHalCdspSetReadBackSize(0, 0, CdspDev->imgRbWidth, CdspDev->imgRbHeight);
			#endif
				gpHalCdspClrIntStatus(CDSP_INT_ALL);
				gpHalCdspSetIntEn(ENABLE, CDSP_EOF);
				gpHalCdspDataSource(CdspDev->imgSrc);
			
			#else
				/* dma auto switch mode */
				if((CdspDev->aInQueIdx == 0) && (CdspDev->bInQueIdx == 0)) {
					CdspDev->abBufFlag = 0;
					CdspDev->aInQueIdx = CdspDev->in_que[0];
					CdspDev->bInQueIdx = CdspDev->in_que[1];
					//memset((void *)gp_chunk_va(CdspDev->bfaddr[CdspDev->aInQueIdx]), 0x55, CdspDev->imgRbWidth*CdspDev->imgRbHeight*2);					
					//memset((void *)gp_chunk_va(CdspDev->bfaddr[CdspDev->bInQueIdx]), 0xAA, CdspDev->imgRbWidth*CdspDev->imgRbHeight*2);					
					gpHalCdspSetYuvBuffA(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->aInQueIdx]);
					gpHalCdspSetYuvBuffB(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->bInQueIdx]);
					gpHalCdspSetDmaBuff(AUTO_SWITCH);

					gpHalCdspClrIntStatus(CDSP_INT_ALL);
					gpHalCdspSetIntEn(ENABLE, CDSP_OVERFOLW|CDSP_EOF);
					gpHalCdspDataSource(CdspDev->imgSrc);
				}
			#endif

				gpHalFrontIntCfg(0x100, 0, 0, 0); // enable VD INT
				
				CdspDev->RunFlag = 1;
				DEBUG("\n\n***** CdspStart. %dx%d ******\n\n\n", CdspDev->imgRbWidth, CdspDev->imgRbHeight);
			} else {
				DEBUG("cdsp start fail\n");
				RETURN(-EINVAL);
			}
			break;
	
		case VIDIOC_STREAMOFF:
			//ytliao 2014.05 stop awb task here
			if(CdspDev->ae_awb_task != NULL)
			{
				kthread_stop(CdspDev->ae_awb_task);
				CdspDev->ae_awb_task = NULL;

				//wait thread stop 
				msleep(10);
			}			

			/* sensor suspend */
			if(CdspDev->imgSrc != C_CDSP_SDRAM) {
				if(CdspDev->MclkEn) {
					CdspDev->sd->ops->ext->suspend(CdspDev->sd);
					if(CdspDev->cb_func->standby) {
						CdspDev->cb_func->standby(1);
					}

					if(CdspDev->cb_func->powerctl) {
						CdspDev->cb_func->powerctl(0);
					}

					CdspDev->MclkEn = 0;
					gpCdspSetMclkout(DISABLE, NULL);					
				}
			}

			// disable sensor set exp tasklet
			tasklet_disable(&sensor_set_exp_tasklet);

			/* disable 3A and IRQ */
			gpHalCdspSetAWBEn(DISABLE, DISABLE);
			gpHalCdspSetAEEn(DISABLE, DISABLE);
			gpHalCdspSetAFEn(DISABLE, DISABLE);	
		#if (C_DMA_CH == 1) || (C_DMA_CH == 2)
			gpHalCdspSetIntEn(DISABLE, CDSP_INT_ALL);
			gpHalCdspClrIntStatus(CDSP_INT_ALL);
		#else
			gpHalCdspSetIntEn(DISABLE, CDSP_AFWIN_UPDATE|CDSP_AWBWIN_UPDATE|CDSP_AEWIN_SEND);
			//gpHalCdspSetIntEn(DISABLE, CDSP_OVERFOLW | CDSP_EOF);
			//gpHalCdspClrIntStatus(CDSP_INT_ALL);
		#endif

			/* clear variable */
			//CdspDev->cdsp_feint_flag = 0;
			//CdspDev->cdsp_eof_cnt = 0;
			CdspDev->RunFlag = 0;
			//CdspDev->CapCnt = 0;
			//CdspDev->SkipCnt = 0;
			//CdspDev->TotalCnt = 0;
			//CdspDev->sensor_status = CMS_IDLE;
			//memset(CdspDev->bfaddr, 0, C_BUFFER_MAX);
			//memset(CdspDev->bf, 0, C_BUFFER_MAX);
			//memset(CdspDev->in_que, 0xFF, C_BUFFER_MAX);
			//memset(CdspDev->out_que, 0xFF, C_BUFFER_MAX);
			DEBUG("CdspStop.\n");
			break;

		case VIDIOC_QBUF:
			nRet = copy_from_user((void*)&bf, (void __user*)arg, sizeof(struct v4l2_buffer));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(bf.type != CdspDev->rbuf.type) {
				DEBUG("cdsp: QBUF Type error\n");
				RETURN(-EINVAL);
			}

			if(bf.index >= CdspDev->rbuf.count ) {
				DEBUG("cdsp: QBUF index out of bound\n");
				RETURN(-EINVAL);
			}

			bf.bytesused = 0;
			bf.flags = V4L2_BUF_FLAG_QUEUED;
			bf.field = V4L2_FIELD_NONE;
			bf.timestamp.tv_sec = 0;
			bf.timestamp.tv_usec = 0;
			bf.sequence = 0;
			bf.input = FRAME_UNSET;
			addr = (unsigned short *)gp_user_va_to_pa((unsigned short *)bf.m.userptr);
			if(addr == 0) {
				RETURN(-EINVAL);
			}
			
			/* find empty index */
			gpHalCdspSetVicIntEn(0);
			for(i=0; i<C_BUFFER_MAX; i++) {
				if(CdspDev->in_que[i] == bf.index) {
					DEBUG("cdsp: in_que error\n");
					gpHalCdspSetVicIntEn(1);
					RETURN(-EINVAL);
				}
				
				if(CdspDev->in_que[i] == 0xFF)	{
					CdspDev->in_que[i] = bf.index;
					idx = bf.index;
					break;
				}
			}
	
			if(i == C_BUFFER_MAX) {
				DEBUG("cdsp: in_que not find\n");
				gpHalCdspSetVicIntEn(1);
				RETURN(-EINVAL);
			}

			CdspDev->bfaddr[idx] = (int)addr;
			memcpy(&CdspDev->bf[idx], &bf, sizeof(struct v4l2_buffer));
			gpHalCdspSetVicIntEn(1);
			break;
	
		case VIDIOC_DQBUF:
			nRet = copy_from_user((void*)&bf, (void __user*)arg, sizeof(struct v4l2_buffer));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(bf.type != CdspDev->rbuf.type) {
				RETURN(-EINVAL);
			}

			gpHalCdspSetVicIntEn(0);
			if(CdspDev->out_que[0] == 0xFF) {
				DIAG_ERROR("cdsp: no buffer ready\n");
				gpHalCdspSetVicIntEn(1);
				RETURN(-EINVAL);
			}
			
			memcpy(&bf, &CdspDev->bf[CdspDev->out_que[0]], sizeof(struct v4l2_buffer));
			
            /* shift the out_queue buffer */
			for(i=0; i<(C_BUFFER_MAX-1); i++) {
				CdspDev->out_que[i] = CdspDev->out_que[i+1];
			}
			
			CdspDev->out_que[C_BUFFER_MAX-1] = 0xFF;
			gpHalCdspSetVicIntEn(1);

			nRet = copy_to_user((void __user*)arg, (void*)&bf, sizeof(struct v4l2_buffer));
			break;

		case VIDIOC_QUERYSTD:
			if(CdspDev->sdidx == NO_INPUT){
				DIAG_ERROR("please set input first\n");
				RETURN(-EINVAL);	
			}

			sd = CdspDev->sd;
			if(sd->ops->video->querystd) {
				nRet = sd->ops->video->querystd(sd, &std);
				if(nRet < 0) {
					RETURN(-EINVAL);
				}
				
				nRet = copy_to_user((void __user*)arg, (void*)&std, sizeof(v4l2_std_id));
			} else {
				nRet = ENOIOCTLCMD;
			}
			break;
		
		case VIDIOC_CAPTURE:
			nRet = copy_from_user((void*)&capture, (void __user*)arg, sizeof(capture));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			nRet = gp_cdsp_s_capture(&capture);
			break;

		case VIDIOC_G_CMS_STS:
			nRet = CdspDev->sensor_status;
			break;
	
	default:
		RETURN(-ENOTTY);	/* Inappropriate ioctl for device */
	}

__return:
	up(&CdspDev->sem);
	return nRet;
}

static int
gp_cdsp_handle_af(
	void *dev_id
)
{
	gpCdspDev_t *pDev = (gpCdspDev_t *)dev_id;
	gpCdsp3aResult_t *argp = &pDev->result;
	af_windows_value_t af_value;

	if(pDev->RunFlag == 0) {
		return 0;
	}
	
	gpHalCdspGetAFWinVlaue(1, &af_value);
	argp->af1_h_value = (af_value.h_value_h << 16) | af_value.h_value_l;
	argp->af1_v_value = (af_value.v_value_h << 16) | af_value.v_value_l;

	gpHalCdspGetAFWinVlaue(2, &af_value);
	argp->af2_h_value = (af_value.h_value_h << 16) | af_value.h_value_l;
	argp->af2_v_value = (af_value.v_value_h << 16) | af_value.v_value_l;

	gpHalCdspGetAFWinVlaue(3, &af_value);
	argp->af3_h_value = (af_value.h_value_h << 16) | af_value.h_value_l;
	argp->af3_v_value = (af_value.v_value_h << 16) | af_value.v_value_l;
	return 0;
}



static int
gp_cdsp_handle_awb(
	void *dev_id
)
{
	gpCdspDev_t *pDev = (gpCdspDev_t *)dev_id;
	AWB_RESULT_t *awb_result = &pDev->result.awb_result;
	signed int tempsh;
	unsigned int cnt, temph, templ, tempsl;	
	
	if(pDev->RunFlag == 0) {
		return 0;
	}
	
	//DEBUG("1 AWB ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);	
	if((pDev->ae_awb_flag & CDSP_AWB_CTRL_EN) != 0) {
		return 0;
	}

	gpHalCdspGetAwbSumCnt(1, &cnt);
	awb_result->sumcnt[0] = cnt;
	gpHalCdspGetAwbSumCnt(2, &cnt);
	awb_result->sumcnt[1] = cnt;
	gpHalCdspGetAwbSumCnt(3, &cnt);
	awb_result->sumcnt[2] = cnt;

	gpHalCdspGetAwbSumG(1, &templ, &temph);
	awb_result->sumg[0] = temph;
	awb_result->sumg[0] <<= 16;
	awb_result->sumg[0] |= templ;
	gpHalCdspGetAwbSumG(2, &templ, &temph);
	awb_result->sumg[1] = temph;
	awb_result->sumg[1] <<= 16;
	awb_result->sumg[1] |= templ;
	gpHalCdspGetAwbSumG(3, &templ, &temph);
	awb_result->sumg[2] = temph;
	awb_result->sumg[2] <<= 16;
	awb_result->sumg[2] |= templ;

	gpHalCdspGetAwbSumRG(1, &tempsl, &tempsh);
	//DEBUG("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumrg[0] = tempsh;
	awb_result->sumrg[0] <<= 32;
	awb_result->sumrg[0] |= tempsl;
	//DEBUG("awb_result->sumrg[0] = 0x%010lx\r\n", awb_result->sumrg[0]);
	
	gpHalCdspGetAwbSumRG(2, &tempsl, &tempsh);
	//DEBUG("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumrg[1] = tempsh;
	awb_result->sumrg[1] <<= 32;
	awb_result->sumrg[1] |= tempsl;
	//DEBUG("awb_result->sumrg[1] = 0x%010lx\r\n", awb_result->sumrg[1]);	
	
	gpHalCdspGetAwbSumRG(3, &tempsl, &tempsh);
	//DEBUG("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumrg[2] = tempsh;
	awb_result->sumrg[2] <<= 32;
	awb_result->sumrg[2] |= tempsl;
	//DEBUG("awb_result->sumrg[2] = 0x%010lx\r\n", awb_result->sumrg[2]);	
	 

	gpHalCdspGetAwbSumBG(1, &tempsl, &tempsh);
	//DEBUG("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumbg[0] = tempsh;
	awb_result->sumbg[0] <<= 32;
	awb_result->sumbg[0] |= tempsl;
	//DEBUG("awb_result->sumbg[0] = 0x%llx\r\n", awb_result->sumbg[0]);
	
	gpHalCdspGetAwbSumBG(2, &tempsl, &tempsh);
	//DEBUG("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumbg[1] = tempsh;
	awb_result->sumbg[1] <<= 32;	
	awb_result->sumbg[1] |= tempsl;
	//DEBUG("awb_result->sumbg[1] = 0x%010lx\r\n", awb_result->sumbg[1]);
	
	gpHalCdspGetAwbSumBG(3, &tempsl, &tempsh);
	//DEBUG("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumbg[2] = tempsh;
	awb_result->sumbg[2] <<= 32;
	awb_result->sumbg[2] |= tempsl;
	//DEBUG("awb_result->sumbg[2] = 0x%010lx\r\n", awb_result->sumbg[2]);
	
	pDev->ae_awb_flag |= CDSP_AWB_CTRL_EN;
	wake_up_interruptible(&pDev->ae_awb_wait_queue);
	
	//DEBUG("2 AWB ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
	return 0;
}



static int
gp_cdsp_handle_ae(
	void *dev_id
)
{
	gpCdspDev_t *pDev = (gpCdspDev_t *)dev_id;
	gpCdsp3aResult_t *argp = &pDev->result;
	unsigned int *ptr;
	int i;

	if(pDev->RunFlag == 0) {
		return 0;
	}
	
	//DEBUG("1 AE ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
	if((pDev->ae_awb_flag & CDSP_AE_CTRL_EN) != 0) {
		return 0;
	}
	
		
	if(gpHalCdspGetAEActBuff()) {
		//memcpy(argp->ae_win, (void*)CdspDev->aeAddr[1], 64);
		ptr = (unsigned int *)pDev->aeAddr[1];
	} else {
		//memcpy(argp->ae_win, (void*)CdspDev->aeAddr[0], 64);
		ptr = (unsigned int *)pDev->aeAddr[0];
	}

	// re-map ae value for HW
	for(i = 0 ; i < 8 ; i++)
	{
		unsigned int val;
		unsigned char t;
		unsigned char *pae;
		
		pae = argp->ae_win + (i << 3);
		val = *ptr++;
		t = val & 0x00ff;
		pae[4] = t;
		val >>= 8;
		t = val & 0x00ff;
		pae[5] = t;
		val >>= 8;
		t = val & 0x00ff;
		pae[6] = t;
		val >>= 8;
		pae[7] = val;

		val = *ptr++;
		t = val & 0x00ff;
		pae[0] = t;
		val >>= 8;
		t = val & 0x00ff;
		pae[1] = t;
		val >>= 8;
		t = val & 0x00ff;
		pae[2] = t;
		val >>= 8;
		pae[3] = val;
	}
	

	pDev->ae_awb_flag |= CDSP_AE_CTRL_EN;
	wake_up_interruptible(&pDev->ae_awb_wait_queue);
	//DEBUG("2 AE ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
	return 0;
}

static int
gp_cdsp_handle_of(
	void *dev_id
)
{
	DERROR("CDSP: overflow\n");
	return 0;
}

static int
gp_cdsp_handle_facwr(
	void *dev_id
)
{
	DERROR("CDSP: facwr\n");
	return 0;
}

static int 
gp_cdsp_handle_postprocess(
	void *dev_id
)
{
	gpCdspDev_t *pDev = (gpCdspDev_t *)dev_id;
	
	if(pDev->cdsp_feint_flag == 0) {
		pDev->cdsp_feint_flag = 1;
		wake_up_interruptible(&pDev->cdsp_wait_queue);
	} else {
		return -1;
	}
	return 0;
}


static int 
gp_cdsp_handle_capture(
	void *dev_id
)
{
	gpCdspDev_t *pDev = (gpCdspDev_t *)dev_id;

#if CDSP_CHK_MIPI_EN == 1
	if(strcmp("MIPI", pDev->port) == 0) {
		if(gp_mipi_get_curframe_status() == 0) {
			DERROR("MIPIFrameFail\n");
			return 0;
		}
	}
#endif 

	DEBUG("%d\n", pDev->CapCnt);
	pDev->CapCnt--;
	if(pDev->CapCnt == 0) {
		if(pDev->cdsp_feint_flag == 0) {
			pDev->cdsp_feint_flag = 1;
			wake_up_interruptible(&pDev->cdsp_wait_queue);
		} else {
			pDev->CapCnt++;
		}
	}
	return 0;
}

static int
gp_cdsp_handle_eof(
	void *dev_id

)
{
	gpCdspDev_t *pDev = (gpCdspDev_t *)dev_id;
	int i, EmptyIdx, ReadyIdx;

#if 0
	{
		struct timeval tv;
		static struct timeval oldtv;
		int time, ret; 

		/* gpio */
		if(pDev->gpio_handle == 0) {
			pDev->gpio_handle = gp_gpio_request(MK_GPIO_INDEX(2, 2, 4, 0), "CDSP_EOF"); 	
			gp_gpio_set_output(pDev->gpio_handle, 1, 0);
		}
		
		gp_gpio_set_value(pDev->gpio_handle, pDev->abBufFlag);

		/* check frame rate */
		do_gettimeofday(&tv);
		ret = tv.tv_sec - oldtv.tv_sec;
		if(ret > 0) {
			time = (ret*1000000 + tv.tv_usec - oldtv.tv_usec)/1000;
		} else {
			time = (tv.tv_usec - oldtv.tv_usec)/1000;
		}

		oldtv.tv_sec = tv.tv_sec;
		oldtv.tv_usec = tv.tv_usec;

		if(time != 33) {
			DEBUG("EOF=%d\n", time);
		} else {
			DEBUG("   =%d\n", time);
		}
	}
#endif
#if 0	
	{
		unsigned int addr;
		unsigned int a[4], b[4];

		addr = pDev->bfaddr[pDev->aInQueIdx];
		addr = (unsigned int)gp_chunk_va(addr);
		a[0] = ReadX(addr);
		a[1] = ReadX(addr + (pDev->imgRbWidth * 2 * pDev->imgRbHeight/2));
		a[2] = ReadX(addr + (pDev->imgRbWidth * 2 * (pDev->imgRbHeight - 1)));
		a[3] = ReadX(addr + (pDev->imgRbWidth * 2 * pDev->imgRbHeight) - 4);
		addr = pDev->bfaddr[pDev->bInQueIdx];
		addr = (unsigned int)gp_chunk_va(addr);
		b[0] = ReadX(addr);
		b[1] = ReadX(addr + (pDev->imgRbWidth * 2 * pDev->imgRbHeight/2));
		b[2] = ReadX(addr + (pDev->imgRbWidth * 2 * (pDev->imgRbHeight - 1)));
		b[3] = ReadX(addr + (pDev->imgRbWidth * 2 * pDev->imgRbHeight) - 4);
		DEBUG("a:0x%x, 0x%x, 0x%x, 0x%x\n", a[0], a[1], a[2], a[3]);
		DEBUG("b:0x%x, 0x%x, 0x%x, 0x%x\n", b[0], b[1], b[2], b[3]);
		DEBUG("\n"); 
		return 0;
	}
#endif

	/* switch frame flag */
	pDev->abBufFlag ^= 0x01;
	if(pDev->RunFlag == 0) {
		//DEBUG("cdsp: idle\n");
		return 0;
	}

	/* set scale crop & skip frame */
	if(pDev->SkipCnt > 0) {
		if(pDev->SkipCnt == SKIP_CNT) {
			gpCdspSetScaleCrop(&pDev->scale);
		}

		if(pDev->abBufFlag) {
			gpHalCdspSetYuvBuffA(pDev->imgRbWidth, pDev->imgRbHeight, 0);
		} else {
			gpHalCdspSetYuvBuffB(pDev->imgRbWidth, pDev->imgRbHeight, 0);
		}
		
		pDev->SkipCnt--;
		return 0;
	}
 	
/* USE DMA A or B, real time switch buffer at EOF */
#if (C_DMA_CH == 1) || (C_DMA_CH == 2)	
#if CDSP_CHK_MIPI_EN == 1
	if(strcmp("MIPI", pDev->port) == 0) {
		if(gp_mipi_get_curframe_status() == 0) {
			DERROR("MIPIFrameFail\n");
			return 0;
		}
	}
#endif 

	/* find empty frame */
	EmptyIdx = pDev->in_que[1];
	if(EmptyIdx == 0xFF) {
		DERROR("cdsp: no empty buffer, skip frame\n");
		return 0; 
	}
	
	/* set empty buffer to h/w */
#if _SET_YUV_PATH_
#if (C_DMA_CH == 1)
	gpHalCdspSetYuvBuffA(pDev->imgRbWidth, pDev->imgRbHeight, pDev->bfaddr[EmptyIdx]);
#else
	gpHalCdspSetYuvBuffB(pDev->imgRbWidth, pDev->imgRbHeight, pDev->bfaddr[EmptyIdx]);
#endif
#else
	gpHalCdspSetRawBuff(pDev->bfaddr[EmptyIdx]);
	gpHalCdspSetRawBuffSize(pDev->imgRbWidth, pDev->imgRbHeight, 0, RAW_BIT);
	gpHalCdspSetReadBackSize(0, 0, pDev->imgRbWidth, pDev->imgRbHeight);
#endif
	//DEBUG("Img: buf = 0x%x, w = %d, h = %d\n", CdspDev->bfaddr[EmptyIdx], CdspDev->imgRbWidth, CdspDev->imgRbHeight);

	/* get/set ready buffer */
	ReadyIdx = pDev->in_que[0];
	pDev->bf[ReadyIdx].bytesused = (pDev->imgRbWidth*pDev->imgRbHeight)<<1;
	pDev->bf[ReadyIdx].flags = V4L2_BUF_FLAG_DONE;
	do_gettimeofday(&pDev->bf[ReadyIdx].timestamp);
	pDev->bf[ReadyIdx].sequence = pDev->TotalCnt++;
	pDev->bf[ReadyIdx].input = FRAME_SET;
	
	/* push the ready index into out_que buffer */
	for(i=0; i<C_BUFFER_MAX; i++) {
		if(pDev->out_que[i] == ReadyIdx) {
			DERROR("cdsp: out_que error\n");
			return 0;
		}
		
		if(pDev->out_que[i] == 0xFF) {
			pDev->out_que[i] = ReadyIdx;
			break;
		}
	}
	
	if(i == C_BUFFER_MAX) {
		DERROR("cdsp: out_que not find\n");
		return 0;
	}
	
	/* shift the in_que buffer */
	for(i=0; i<(C_BUFFER_MAX-1); i++) {
		pDev->in_que[i] = pDev->in_que[i+1];
	}
	
	pDev->in_que[C_BUFFER_MAX-1] = 0xFF;

#else
/* USE DMA auto switch, ready buffer sequence is BABABA... */
#if CDSP_CHK_MIPI_EN == 1
	if(strcmp("MIPI", pDev->port) == 0) {
		if(gp_mipi_get_curframe_status() == 0) {
			DERROR("MIPIFrameFail\n");
			return 0;
		}
	}
#endif 
	
	/* find empty frame */
	EmptyIdx = pDev->in_que[2];
	if(EmptyIdx == 0xFF) {
		DERROR("cdsp: no empty buffer, skip frame\n");
		return 0; 
	}

	if(pDev->abBufFlag) {
		gpHalCdspSetYuvBuffA(pDev->imgRbWidth, pDev->imgRbHeight, pDev->bfaddr[EmptyIdx]);
		ReadyIdx = pDev->aInQueIdx;
		pDev->aInQueIdx = EmptyIdx;
	} else {
		gpHalCdspSetYuvBuffB(pDev->imgRbWidth, pDev->imgRbHeight, pDev->bfaddr[EmptyIdx]);
		ReadyIdx = pDev->bInQueIdx;
		pDev->bInQueIdx = EmptyIdx;	
	}

	/* set ready buffer */
	//DEBUG("rdy:%d\n", ReadyIdx);
	pDev->bf[ReadyIdx].bytesused = (pDev->imgRbWidth*pDev->imgRbHeight)<<1;
	pDev->bf[ReadyIdx].flags = V4L2_BUF_FLAG_DONE;
	do_gettimeofday(&pDev->bf[ReadyIdx].timestamp);
	pDev->bf[ReadyIdx].sequence = pDev->TotalCnt++;
	pDev->bf[ReadyIdx].input = FRAME_SET;
	
	/* push the ready index into out_que buffer */
	for(i=0; i<C_BUFFER_MAX; i++) {
		if(pDev->out_que[i] == ReadyIdx) {
			DERROR("cdsp: find out_que error\n");
			return 0;
		}
		
		if(pDev->out_que[i] == 0xFF) {
			pDev->out_que[i] = ReadyIdx;
			break;
		}
	}
	
	if(i == C_BUFFER_MAX) {
		DERROR("cdsp: out_que not find\n");
		return 0;
	}
	
	/* shift the in_que buffer */
	//DEBUG("inn:%d, %d, %d, %d, %d\n", pDev->in_que[0], pDev->in_que[1], pDev->in_que[2], pDev->in_que[3], pDev->in_que[4]);
	for(i=0; i<C_BUFFER_MAX; i++) {
		if(pDev->in_que[i] == ReadyIdx) {
			break;
		}
	}

	if(i == C_BUFFER_MAX) {
		DERROR("cdsp: find in_que error\n");
		return 0;
	}

	for(; i<(C_BUFFER_MAX-1); i++) {
		pDev->in_que[i] = pDev->in_que[i+1];
	}

	pDev->in_que[C_BUFFER_MAX-1] = 0xFF;
	//DEBUG("ins:%d, %d, %d, %d, %d\n", pDev->in_que[0], pDev->in_que[1], pDev->in_que[2], pDev->in_que[3], pDev->in_que[4]);
	//DEBUG("out:%d, %d, %d, %d, %d\n", pDev->out_que[0], pDev->out_que[1], pDev->out_que[2], pDev->out_que[3], pDev->out_que[4]);
#endif

	/* wake up poll */
	pDev->cdsp_eof_cnt++;
	wake_up_interruptible(&pDev->cdsp_wait_queue);
	return 0;
}

static irqreturn_t
gp_cdsp_irq_handler(
	int irq,
	void *dev_id
)
{
	int status;
	int ret = -1;
	gpCdspDev_t *pDev = (gpCdspDev_t *)dev_id;

	status = gpHalCdspGetGlbIntStatus();
	if(status & CDSP_INT_BIT) {
		status = gpHalCdspGetIntStatus();
		if(status & CDSP_EOF) { 
			if(pDev->imgSrc == C_CDSP_SDRAM) {
				ret = gp_cdsp_handle_postprocess(dev_id);
			} else {
				if(pDev->CapCnt > 0) {
					ret = gp_cdsp_handle_capture(dev_id);
				} else {
					
					
					ret = gp_cdsp_handle_eof(dev_id);

					// histgm
					if((pDev->ae_awb_flag & CDSP_HIST_CTRL_EN) == 0) {
						unsigned int hislowcnt, hishicnt;
					
						gpHalCdspGetHistgmCount(&hislowcnt, &hishicnt);
						pDev->result.hislowcnt = hislowcnt;
						pDev->result.hishicnt = hishicnt;
						pDev->ae_awb_flag |= CDSP_HIST_CTRL_EN;
					}
				}
			}

			#if 1
			// add by Comi for changing the CDSP setting
			// set AWB gain
			if((pDev->ae_awb_flag & CDSP_AWB_SET_GAIN) != 0)
			{
				int rgain, bgain;
				bgain = gp_cdsp_awb_get_b_gain(pDev->awb_workmem);
				rgain = gp_cdsp_awb_get_r_gain(pDev->awb_workmem);
				
				gpHalCdspSetWb_r_b_Gain(rgain, bgain);
				pDev->ae_awb_flag &= (~CDSP_AWB_SET_GAIN);						
			}
			
			
			// set YUV sat/hue
			if((pDev->ae_awb_flag & CDSP_SAT_SWITCH) != 0)
			{	
				int *p_sat;
				int idx = pDev->sat_contr_idx;

				p_sat = &pDev->sat_yuv_level[idx][0];
				gpHalCdspSetYuvSPEffOffset(p_sat[0], p_sat[1], p_sat[2]);
				gpHalCdspSetYuvSPEffScale(p_sat[3], p_sat[4], p_sat[5]);
				gpHalCdspSetUvDivide(&pDev->UVDivide);
/*
				if(pDev->sat_contr_idx <= 2) 
				{
					gpHalCdspSetEdgeAmpga(pDev->edge_day);
					//DEBUG("edge_day = %d\r\n", pDev->edge_day);
				}
				else 
				{
					gpHalCdspSetEdgeAmpga(pDev->edge_night);
					//DEBUG("edge_night = %d\r\n", pDev->edge_night);
				}*/
				gpHalCdspSetEdgeAmpga(pDev->edge_level[idx]);
				//DEBUG("edge[%d] = %d\r\n", idx, pDev->edge_level[idx]);
				
				pDev->ae_awb_flag &= (~CDSP_SAT_SWITCH);
			}
			
			if((pDev->ae_awb_flag & CDSP_LUM_STATUS) != 0) {			
				if((pDev->ae_awb_flag & CDSP_HIGH_LUM) != 0) {		
					gpHalCdspSetNewDenoiseEn(DISABLE);
					//DEBUG("Denoise disable\n");
				} else if((pDev->ae_awb_flag & CDSP_LOW_LUM) != 0) {
					//gpHalCdspSetEdgeAmpga(2);
					gpHalCdspSetNewDenoiseEn(ENABLE); 					
					//DEBUG("Denoise enable\n");
				}				
				
				pDev->ae_awb_flag &= (~CDSP_LUM_STATUS);
			}	
			#endif
		}
		
		if(status & CDSP_OVERFOLW) {
			ret = gp_cdsp_handle_of(dev_id);
		}
		
		if(status & CDSP_FACWR) {
			ret = gp_cdsp_handle_facwr(dev_id);
		}
		
		if(status & CDSP_AFWIN_UPDATE) {
			ret = gp_cdsp_handle_af(dev_id);
		}
		
		if(status & CDSP_AWBWIN_UPDATE) {
			ret = gp_cdsp_handle_awb(dev_id);
		}
		
		if(status & CDSP_AEWIN_SEND) {
			ret = gp_cdsp_handle_ae(dev_id);
		}
	} else if(status & FRONT_VD_INT_BIT) {
		UINT32 status;
		
		gpHalFrontIntEventGet(&status);
		gpHalFrontIntEventClr(status);
		//printk("FRONT_VD_INT_BIT = 0x%x\n", status);

		if((pDev->ae_awb_flag & CDSP_AE_UPDATE) != 0)
		{
			#if 0
			struct v4l2_control csi_ctrl;
			struct v4l2_subdev *sd;
			
			csi_ctrl.value = (int) &pDev->sInfo;
			csi_ctrl.id = V4L2_CID_EXPOSURE;
			sd = CdspDev->sd;
			ret = sd->ops->core->s_ctrl(sd, &csi_ctrl);
			if(ret < 0) {
				CdspDev->sensor_status = CMS_I2C_TIMEOUT;
			}
		       else 					
				pDev->ae_awb_flag &= (~CDSP_AE_UPDATE);
			#else
			tasklet_schedule(&sensor_set_exp_tasklet); 
			#endif
		}
		
	} else if(status & FRONT_INT_BIT) {
		ret = 0;
	} 

	if(ret >= 0) {
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}



static void sensor_set_exp_do_tasklet(unsigned long para)
{
	int ret;
	struct v4l2_control csi_ctrl;
	struct v4l2_subdev *sd;

	//printk("%s\r\n", __FUNCTION__);
	
	csi_ctrl.value = (int) &CdspDev->sInfo;
	csi_ctrl.id = V4L2_CID_EXPOSURE;
	sd = CdspDev->sd;
	ret = sd->ops->core->s_ctrl(sd, &csi_ctrl);
	if(ret < 0) {
		//printk("set sensor failed!\r\n");
		CdspDev->sensor_status = CMS_I2C_TIMEOUT;
	}
       else {					
		CdspDev->ae_awb_flag &= (~CDSP_AE_UPDATE);
		//printk("set sensor success!\r\n");
       }
}

static int gp_ae_awb_process(void *arg)
{
	unsigned int ae_frame, awb_frame;
	int ret, ae_stable = 1;
	gpCdsp3aResult_t *p_3a_result = &CdspDev->result;	
	unsigned char *awb;
	unsigned char *ae;
	struct v4l2_subdev *sd;
	struct v4l2_control csi_ctrl;
	sensor_exposure_t	*p_seInfo, seInfo;
	int low_lum_switch_cnt, high_lum_switch_cnt;
	gpCdspCorMatrix_t mtx1, mtx2;
	int awb_low_lum_cnt, targetY_low;
	int pre_awb_ct;
	//struct  semaphore  sem;

	awb = CdspDev->awb_workmem;
	ae = CdspDev->ae_workmem;
	
	//init_MUTEX(&sem);
	

	DEBUG("\r\n\r\n=========== AE & AWB Process Init =========\r\n\r\n");
	DEBUG("\r\n%s\r\n\r\n", gp_cdsp_aeawb_get_version());

	{	// reset wb gain by initial color temperature
		gpCdspWhtBal_t wb_gain;
		//printk("color temperature init = %d\n", CdspDev->color_temp_init_val);
		gp_cdsp_awb_reset_wb_gain(awb, CdspDev->color_temp_init_val, CdspDev->sensor_cdsp.wb_gain);
		wb_gain.wbgainen  = ENABLE;
		wb_gain.bgain = gp_cdsp_awb_get_b_gain(CdspDev->awb_workmem);
		wb_gain.rgain = gp_cdsp_awb_get_r_gain(CdspDev->awb_workmem);	
		wb_gain.gbgain = 64;
		wb_gain.grgain = 64;
		wb_gain.global_gain = 32;
		gpCdspSetWhiteBalance(&wb_gain);
	}

	CdspDev->UVDivide.YT1 = uv_div[0][0];	
	CdspDev->UVDivide.YT2 = uv_div[0][1];
	CdspDev->UVDivide.YT3 = uv_div[0][2];	
	CdspDev->UVDivide.YT4 = uv_div[0][3];
	CdspDev->UVDivide.YT5 = uv_div[0][4];	
	CdspDev->UVDivide.YT6 = uv_div[0][5];
	gpHalCdspSetUvDivide(&CdspDev->UVDivide);	
	gpHalCdspSetUvDivideEn(DISABLE);
	
	awb_frame = 0;
	ae_frame = 0;
	CdspDev->ae_awb_flag = 0;
	CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS | CDSP_HIGH_LUM);

	#if 1
	while(1)
	{
		if(CdspDev->getSensorInfo == 0x21)
		{
			//DEBUG("Get Sensor Info: 0x%x\r\n", CdspDev->getSensorInfo);
			break;
		}

		awb_frame++;
		if(awb_frame >= 500)
		{
			DERROR("Err: Not Get Sensor Info: 0x%x\r\n", CdspDev->getSensorInfo);
			break;
		}
		
		msleep(1);
	}
	#else
	while(1)	{		if(CdspDev->getSensorInfo == 0x21 || awb_frame == 0x10000000) break;		awb_frame++;	}
	if(CdspDev->getSensorInfo != 0x21) DEBUG("Err: Not Get Sensor Info: 0x%x\r\n", CdspDev->getSensorInfo);
	#endif
	awb_frame = 0;
		
	
	CdspDev->sensor_gain_thr = (1*256);
	CdspDev->sensor_time_thr = 0x100;
	if(CdspDev->getSensorInfo == 0x21)
	{
		csi_ctrl.id = V4L2_CID_EXPOSURE;
		sd = CdspDev->sd;
		ret = sd->ops->core->g_ctrl(sd, &csi_ctrl);
		p_seInfo = (sensor_exposure_t	*) csi_ctrl.value;		
		if(ret < 0) {
			CdspDev->sensor_status = CMS_I2C_TIMEOUT;
			CdspDev->sensor_time_thr = 0x200;
			CdspDev->night_gain_thr = 0x200;
			CdspDev->sensor_gain_thr = 0x200;
		} else {					
							
			CdspDev->sensor_gain_thr = p_seInfo->max_analog_gain >> 2;
			CdspDev->night_gain_thr = CdspDev->sensor_gain_thr << 2;
			if(CdspDev->sensor_gain_thr < (1*256)) CdspDev->sensor_gain_thr = 1*256;		
			if(CdspDev->night_gain_thr < (1*256)) CdspDev->night_gain_thr = 1*256;
								
			if(CdspDev->night_gain_thr > p_seInfo->max_analog_gain)
				CdspDev->night_gain_thr = p_seInfo->max_analog_gain;

			CdspDev->sensor_time_thr = (int)(p_seInfo->max_time/3);
			if(CdspDev->sensor_time_thr < (8)) CdspDev->sensor_time_thr = 8;

		} 
		gp_cdsp_ae_set_sensor_exp_time(CdspDev->ae_workmem, p_seInfo);

		CdspDev->sat_contr_idx = 0;
		gp_cdsp_sat_contrast_thr_init(p_seInfo->max_ev_idx, p_seInfo->night_ev_idx);
	}
	memcpy(&CdspDev->sInfo, &seInfo, sizeof(sensor_exposure_t));
	
/*	if(CdspDev->sensor_ev_idx != -1)
	{
		csi_ctrl.id = V4L2_CID_EXPOSURE;
		sd = CdspDev->sd;
		ret = sd->ops->core->g_ctrl(sd, &csi_ctrl);
		if(ret < 0) {
			CdspDev->sensor_status = CMS_I2C_TIMEOUT;
		}	
		p_seInfo = (sensor_exposure_t	*) csi_ctrl.value;
		p_seInfo->ae_ev_idx = CdspDev->sensor_ev_idx - p_seInfo->sensor_ev_idx;
		printk("ReInit: sensor ev idx = %d, ae idx = %d\n", p_seInfo->sensor_ev_idx, p_seInfo->ae_ev_idx);
		// reset sensor to saved value
		ret = sd->ops->core->s_ctrl(sd, &csi_ctrl);
		if(ret < 0) {
			CdspDev->sensor_status = CMS_I2C_TIMEOUT;
		}					
	}*/

	
	
	DEBUG("\r\n\r\n=========== AE & AWB Process Start =========\r\n\r\n");

	mtx1.colcorren = ENABLE;
	mtx1.a11 = CdspDev->sensor_cdsp.color_matrix1[0];
	mtx1.a12 = CdspDev->sensor_cdsp.color_matrix1[1];
	mtx1.a13 = CdspDev->sensor_cdsp.color_matrix1[2];
	mtx1.a21 = CdspDev->sensor_cdsp.color_matrix1[3];
	mtx1.a22 = CdspDev->sensor_cdsp.color_matrix1[4];
	mtx1.a23 = CdspDev->sensor_cdsp.color_matrix1[5];
	mtx1.a31 = CdspDev->sensor_cdsp.color_matrix1[6];
	mtx1.a32 = CdspDev->sensor_cdsp.color_matrix1[7];
	mtx1.a33 = CdspDev->sensor_cdsp.color_matrix1[8];

	mtx2.colcorren = ENABLE;
	mtx2.a11 = CdspDev->sensor_cdsp.color_matrix2[0];
	mtx2.a12 = CdspDev->sensor_cdsp.color_matrix2[1];
	mtx2.a13 = CdspDev->sensor_cdsp.color_matrix2[2];
	mtx2.a21 = CdspDev->sensor_cdsp.color_matrix2[3];
	mtx2.a22 = CdspDev->sensor_cdsp.color_matrix2[4];
	mtx2.a23 = CdspDev->sensor_cdsp.color_matrix2[5];
	mtx2.a31 = CdspDev->sensor_cdsp.color_matrix2[6];
	mtx2.a32 = CdspDev->sensor_cdsp.color_matrix2[7];
	mtx2.a33 = CdspDev->sensor_cdsp.color_matrix2[8];
	
	low_lum_switch_cnt = high_lum_switch_cnt = 0;
	awb_low_lum_cnt = 0;
	targetY_low = 10;
	pre_awb_ct = 50;
	while(1) {
		
		if (kthread_should_stop()) 
		{
			DEBUG("kthread aeawb process should stop!\n");
			break;
		}

		//wait_event_interruptible(CdspDev->ae_awb_wait_queue, (CdspDev->ae_awb_flag != 0));
		wait_event_interruptible_timeout(CdspDev->ae_awb_wait_queue, 
			((CdspDev->ae_awb_flag & CDSP_AE_CTRL_EN) != 0 || (CdspDev->ae_awb_flag & CDSP_AWB_CTRL_EN) != 0), 
			10);

		//DEBUG("0: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
		
		if((CdspDev->ae_awb_flag & CDSP_AE_CTRL_EN) != 0)
		{
			if((CdspDev->ae_awb_flag & CDSP_AE_UPDATE) == 0)	ae_frame++;
			
			if(ae_frame >= 3)
			{
				int ret;
				unsigned int hist_lo, hist_hi;

				ae_frame = 0;	

				if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
				
				//DEBUG("1: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);					
#if 1			
				csi_ctrl.id = V4L2_CID_EXPOSURE;		
				sd = CdspDev->sd;				
				ret = sd->ops->core->g_ctrl(sd, &csi_ctrl);
				if(ret < 0) {
					CdspDev->sensor_status = CMS_I2C_TIMEOUT;
				}
				
				p_seInfo = (sensor_exposure_t	*) csi_ctrl.value;

				//DEBUG("SensorExposure: time = 0x%x, analogGain = 0x%x, digitalGain = 0x%x\r\n", p_seInfo->time, p_seInfo->analog_gain, p_seInfo->digital_gain);

				memcpy(&seInfo, p_seInfo, sizeof(sensor_exposure_t));

				//DEBUG("Digital Gain: %x, %x, %x\n", seInfo.digital_gain, seInfo.max_digital_gain, seInfo.min_digital_gain);
				//DEBUG("Analog Gain: %x, %x, %x\n", seInfo.analog_gain, seInfo.max_analog_gain, seInfo.min_analog_gain);
				//DEBUG("Time: %x, %x, %x\n", seInfo.time, seInfo.max_time, seInfo.min_time);
				hist_lo = 0;
				hist_hi = 0;
				if((CdspDev->ae_awb_flag & CDSP_HIST_CTRL_EN) != 0)
				{
					hist_lo = CdspDev->result.hislowcnt;
					hist_hi = CdspDev->result.hishicnt;
					CdspDev->ae_awb_flag &= (~CDSP_HIST_CTRL_EN);
					//DEBUG("hist_hi = %d, hist_lo = %d\n", hist_hi, hist_lo);
				}
				

				ret = gp_cdsp_ae_calc_exp(ae, p_3a_result->ae_win, &seInfo, hist_hi, hist_lo);
				seInfo.ae_ev_idx = gp_cdsp_ae_get_result_ev(ae);
				
				if(seInfo.ae_ev_idx != 0)
				{	
				/*	csi_ctrl.value = (int) &seInfo;
					csi_ctrl.id = V4L2_CID_EXPOSURE;
					sd = CdspDev->sd;
					ret = sd->ops->core->s_ctrl(sd, &csi_ctrl);
					if(ret < 0) {
						CdspDev->sensor_status = CMS_I2C_TIMEOUT;
					}*/
					//if((CdspDev->ae_awb_flag & CDSP_AE_UPDATE) == 0)
					{
						memcpy(&CdspDev->sInfo, &seInfo, sizeof(sensor_exposure_t));

						CdspDev->ae_awb_flag |= CDSP_AE_UPDATE;
						//DEBUG("ae update\r\n");
					}

					//DEBUG("lum = 0x%x\r\n",  gp_cdsp_ae_get_result_lum(ae));
					//DEBUG("SetSensor: time = 0x%x, analogGain = 0x%x, digitalGain = 0x%x\r\n", seInfo.time, seInfo.analog_gain, seInfo.digital_gain);

					//DEBUG("Digital Gain: %x, %x, %x\n", seInfo.digital_gain, seInfo.max_digital_gain, seInfo.min_digital_gain);
					//DEBUG("Analog Gain: %x, %x, %x\n", seInfo.analog_gain, seInfo.max_analog_gain, seInfo.min_analog_gain);
					//DEBUG("Time: %x, %x, %x\n", seInfo.time, seInfo.max_time, seInfo.min_time);
					//DEBUG("analog_gain = 0x%x, sensor_gain_thr = 0x%x\n", seInfo.analog_gain,  CdspDev->sensor_gain_thr);
					#if 1
					//if(seInfo.analog_gain >= (int)(256*2.0))//CdspDev->sensor_gain_thr)
					if(seInfo.sensor_ev_idx >= seInfo.night_ev_idx)
					{ 	// low light
						//int y_offset;
						high_lum_switch_cnt = 0;

						//DEBUG("1:low light: flag = 0x%x\n", CdspDev->ae_awb_flag);
						if((CdspDev->ae_awb_flag & CDSP_LOW_LUM) == 0)
						{
							low_lum_switch_cnt++;							
							if(low_lum_switch_cnt >= 20) {
								CdspDev->ae_awb_flag &=( ~(CDSP_LUM_STATUS | CDSP_HIGH_LUM | CDSP_LOW_LUM));
								CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS | CDSP_LOW_LUM);
							//DEBUG("2:low light: flag = 0x%x\n", CdspDev->ae_awb_flag);
								low_lum_switch_cnt = 0;
								//gpHalCdspInitGamma(CdspDev->sensor_cdsp.gamma2);
								//gpCdspSetColorMatrix(&mtx2);
								//gpHalCdspSetYuvSPEffOffset(4, 0, 4);
								//gpHalCdspSetYuvSPEffScale(0x20, 0x18, 0x18);
							}
						}						
					}
					else if(seInfo.sensor_ev_idx <= (seInfo.night_ev_idx-35))//if(seInfo.analog_gain <= (int)(256*1.2))
					{ 	// Daylight
						low_lum_switch_cnt = 0;
						if((CdspDev->ae_awb_flag & CDSP_HIGH_LUM) == 0)
						{							
							high_lum_switch_cnt++;
							if(high_lum_switch_cnt >= 20)
							{
								//CdspDev->y_offset = 0; // -128 ~ +127
								CdspDev->ae_awb_flag &= (~(CDSP_LUM_STATUS | CDSP_HIGH_LUM | CDSP_LOW_LUM));
								CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS | CDSP_HIGH_LUM);
								high_lum_switch_cnt = 0;
								//gpHalCdspInitGamma(CdspDev->sensor_cdsp.gamma1);
								//gpCdspSetColorMatrix(&mtx1);
								//gpHalCdspSetYuvSPEffOffset(-4, 1, 5);
								//gpHalCdspSetYuvSPEffScale(0x21, 0x22, 0x22);	
							}	
						}
					}
					else
					{
						low_lum_switch_cnt--;
						if(low_lum_switch_cnt < 0) low_lum_switch_cnt = 0;
						high_lum_switch_cnt--;
						if(high_lum_switch_cnt < 0) high_lum_switch_cnt = 0;
					}
					#endif


					if(seInfo.sensor_ev_idx >= CdspDev->sat_yuv_thr[3] && CdspDev->sat_contr_idx != 3)
					{						
						CdspDev->sat_contr_idx = 3;
						CdspDev->UVDivide.YT1 = uv_div[3][0];	
						CdspDev->UVDivide.YT2 = uv_div[3][1];
						CdspDev->UVDivide.YT3 = uv_div[3][2];	
						CdspDev->UVDivide.YT4 = uv_div[3][3];
						CdspDev->UVDivide.YT5 = uv_div[3][4];	
						CdspDev->UVDivide.YT6 = uv_div[3][5];
						CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
						//DEBUG("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
						gpHalCdspSetIntplThr(64, 240);
						gpHalCdspSetBadPixel(80,80,80);
					}
					else if(seInfo.sensor_ev_idx < CdspDev->sat_yuv_thr[3] && seInfo.sensor_ev_idx >= CdspDev->sat_yuv_thr[2] && CdspDev->sat_contr_idx != 2)
					{						
						CdspDev->sat_contr_idx = 2;
						CdspDev->UVDivide.YT1 = uv_div[2][0];	
						CdspDev->UVDivide.YT2 = uv_div[2][1];
						CdspDev->UVDivide.YT3 = uv_div[2][2];	
						CdspDev->UVDivide.YT4 = uv_div[2][3];
						CdspDev->UVDivide.YT5 = uv_div[2][4];	
						CdspDev->UVDivide.YT6 = uv_div[2][5];
						CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
						//DEBUG("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
						gpHalCdspSetIntplThr(48, 240);
						gpHalCdspSetBadPixel(120,120,120);
					}
					else if(seInfo.sensor_ev_idx < CdspDev->sat_yuv_thr[2] && seInfo.sensor_ev_idx >= CdspDev->sat_yuv_thr[1] && CdspDev->sat_contr_idx != 1)
					{						
						CdspDev->sat_contr_idx = 1;
						CdspDev->UVDivide.YT1 = uv_div[1][0];	
						CdspDev->UVDivide.YT2 = uv_div[1][1];
						CdspDev->UVDivide.YT3 = uv_div[1][2];	
						CdspDev->UVDivide.YT4 = uv_div[1][3];
						CdspDev->UVDivide.YT5 = uv_div[1][4];	
						CdspDev->UVDivide.YT6 = uv_div[1][5];
						CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
						//DEBUG("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
						gpHalCdspSetIntplThr(36, 240);
						gpHalCdspSetBadPixel(120,120,120);
					}
					else if(seInfo.sensor_ev_idx < CdspDev->sat_yuv_thr[1] && CdspDev->sat_contr_idx != 0)
					{						
						CdspDev->sat_contr_idx = 0;
						CdspDev->UVDivide.YT1 = uv_div[0][0];	
						CdspDev->UVDivide.YT2 = uv_div[0][1];
						CdspDev->UVDivide.YT3 = uv_div[0][2];	
						CdspDev->UVDivide.YT4 = uv_div[0][3];
						CdspDev->UVDivide.YT5 = uv_div[0][4];	
						CdspDev->UVDivide.YT6 = uv_div[0][5];
						CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;

						gpHalCdspSetIntplThr(CdspDev->intpl_low_thr, CdspDev->intpl_hi_thr);
						gpHalCdspSetBadPixel(160,160,160);
						//DEBUG("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
					}

					ae_stable = 0;
				}
				else
				{					
					ae_stable = 1;
					ae_frame += 2;
				}
#endif
				up(&CdspDev->aeawb_sem);

				//DEBUG("ae_awb_flag = 0x%x\r\n", CdspDev->ae_awb_flag);
			}

			//ae_stable = 1;

			CdspDev->ae_awb_flag &= (~CDSP_AE_CTRL_EN);
			//DEBUG("AE process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
		}


		if( (CdspDev->ae_awb_flag & CDSP_AWB_CTRL_EN) != 0) 
		{		
			awb_frame++;
			//if(ae_stable == 1 && awb_frame >= 2)
			if(awb_frame >= 2 && (CdspDev->ae_awb_flag & CDSP_AWB_SET_GAIN) == 0)
			{
				gpCdspWbGain2_t wb_gain2;
				int ret, lum;
								
				awb_frame = 0;
				
				if(down_interruptible(&CdspDev->aeawb_sem) != 0) return -ERESTARTSYS;
				
				//DEBUG("2: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
#if 1				
				ret = gp_cdsp_awb_calc_gain(awb, &p_3a_result->awb_result, CdspDev->sensor_cdsp.wb_gain); 

				lum = gp_cdsp_ae_get_result_lum(ae);
				if((ret == AWB_FAIL) && ((CdspDev->ae_awb_flag & CDSP_LOW_LUM) != 0) && (lum <= targetY_low))
				{
					awb_low_lum_cnt++;					
					if(awb_low_lum_cnt >= 3)
					{
						if(pre_awb_ct > 45)
						{
							// could be night)
							gp_cdsp_awb_reset_wb_gain(awb, 40, CdspDev->sensor_cdsp.wb_gain);
													
							CdspDev->ae_awb_flag |= CDSP_AWB_SET_GAIN;
							
							//DBG_PRINT("AWB low light\r\n");
							awb_low_lum_cnt = -512;
							pre_awb_ct = 40;
						}
						else
						{
							awb_low_lum_cnt = 0;
						}
					}
				}
				else if(ret != AWB_FAIL && ret != AWB_RET && awb_low_lum_cnt > 0)
				{
					awb_low_lum_cnt = 0;
				}
				
				if(ret != AWB_FAIL || ret != AWB_RET)
				{
					CdspDev->ae_awb_flag |= CDSP_AWB_SET_GAIN;
				}
					
				#if 1
				if(ret != AWB_RET)
				{
					int awbmode;
					awbmode = gp_cdsp_awb_get_mode(awb);
					//DEBUG("awb mode = 0x%x\n", awbmode);
					//if(ret != AWB_SUCCESS_CVR)
					if(awbmode == AWB_AUTO_CVR || awbmode == AWB_AUTO_CVR_DAYLIGHT || awbmode == AWB_AUTO_CVR_NIGHT)
					{
						csi_ctrl.id = V4L2_CID_EXPOSURE;
						sd = CdspDev->sd;
						ret = sd->ops->core->g_ctrl(sd, &csi_ctrl);
						if(ret < 0) {
							CdspDev->sensor_status = CMS_I2C_TIMEOUT;
						}
						
						p_seInfo = (sensor_exposure_t	*) csi_ctrl.value;
						//DEBUG("SensorExposure: time = 0x%x, analogGain = 0x%x, digitalGain = 0x%x\r\n", p_seInfo->time, p_seInfo->analog_gain, p_seInfo->digital_gain);
						//DEBUG("Thr: gain = 0x%x, daylight ev idx =%d\n", CdspDev->night_gain_thr, p_seInfo->daylight_ev_idx);
					
						// Auto mode for CVR
						if(gp_cdsp_ae_is_night(ae) == 1) 
						{
							//DEBUG("AWB Night\r\n");
							awbmode = AWB_AUTO_CVR_NIGHT;
							gp_cdsp_awb_set_ct_offset(awb, CdspDev->wb_offset_night);
						}
						else if(p_seInfo->sensor_ev_idx <= p_seInfo->daylight_ev_idx)
						{
							//DEBUG("AWB Daylight\r\n");
							awbmode = AWB_AUTO_CVR_DAYLIGHT;
							gp_cdsp_awb_set_ct_offset(awb, CdspDev->wb_offset_day);
						}
						else 				
						{
							//DEBUG("AWB Auto\r\n");
							awbmode = AWB_AUTO_CVR;
							gp_cdsp_awb_set_ct_offset(awb, CdspDev->wb_offset_day);
						}
						gp_cdsp_awb_set_mode(awb, awbmode);
						gp_cdsp_awb_calc_gain(awb, &p_3a_result->awb_result, CdspDev->sensor_cdsp.wb_gain);

						//printk("colorT = %d, calc colorT = %d\n", gp_cdsp_awb_get_color_temperature(awb), gp_cdsp_awb_get_calc_color_temperature(awb));
					}
				}
				#endif
				wb_gain2.wbgain2en = ENABLE;
				wb_gain2.rgain2 = gp_cdsp_awb_get_r_gain2(awb);
				wb_gain2.bgain2 = gp_cdsp_awb_get_b_gain2(awb);
				wb_gain2.ggain2 = 64;			
				gpCdspSetWBGain2(&wb_gain2);

				pre_awb_ct = gp_cdsp_awb_get_color_temperature(awb);

			/*	{
				int rgain, bgain;
				bgain = gp_cdsp_awb_get_b_gain(CdspDev->awb_workmem);
				rgain = gp_cdsp_awb_get_r_gain(CdspDev->awb_workmem);
				//printk("r gain = %d, %d, b gain = %d, %d\r\n", rgain, wb_gain2.rgain2,bgain,wb_gain2.bgain2);
				}*/
#endif				
				up(&CdspDev->aeawb_sem);
			}

			CdspDev->ae_awb_flag &= (~CDSP_AWB_CTRL_EN);
			//DEBUG("AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
		}
		//DEBUG("3: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);

		//msleep(1);
	}

	csi_ctrl.id = V4L2_CID_EXPOSURE;		
	sd = CdspDev->sd;				
	ret = sd->ops->core->g_ctrl(sd, &csi_ctrl);
	if(ret < 0) {
		CdspDev->sensor_status = CMS_I2C_TIMEOUT;
	}	
	p_seInfo = (sensor_exposure_t	*) csi_ctrl.value;
	CdspDev->sensor_ev_idx = p_seInfo->sensor_ev_idx;
	printk("aeawb stop: sensor_ev_idx = %d\n", CdspDev->sensor_ev_idx);

	CdspDev->color_temp_init_val = gp_cdsp_awb_get_color_temperature(awb);
	CdspDev->ae_awb_flag = 0;
	CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS | CDSP_HIGH_LUM);

	DEBUG("\r\n\r\n=========== AE & AWB Process Stop =========\r\n\r\n");
	return 0;
}



static unsigned int
gp_cdsp_poll(
	struct file *filp,
	struct poll_table_struct *poll
)
{
	unsigned int mask = 0;

#if 0
	wait_event_interruptible(CdspDev->cdsp_wait_queue, (CdspDev->cdsp_eof_cnt > 0));
#else
	poll_wait(filp, &CdspDev->cdsp_wait_queue, poll);
#endif

	if(CdspDev->cdsp_eof_cnt > 0) {
		gpHalCdspSetVicIntEn(0);
		CdspDev->cdsp_eof_cnt--;
		gpHalCdspSetVicIntEn(1);
		mask = POLLIN | POLLRDNORM;
	}

	return mask;
}

static int
gp_cdsp_open(
	struct inode *inode,
	struct file *filp
)
{
	int ret = 0;
	
	if(CdspDev->OpenCnt == 0) 
	{
		int i, j;
		
		CdspDev->getSensorInfo = 0;
		
		gpCdspClockEnable(1);
		gpHalCdspModuleRest(1);
		
		gpHalCdspReset();
		gpHalFrontReset();
		gpHalCdspDataSource(C_CDSP_SKIP_WR);

		gpHalCdspSetIntEn(DISABLE, CDSP_INT_ALL);
		gpHalCdspClrIntStatus(CDSP_INT_ALL);
		
		CdspDev->OpenCnt = 1;

		/* create kernel thread for AE/AWB */
		CdspDev->ae_workmem = 0;
		CdspDev->awb_workmem = 0;
		CdspDev->ae_workmem = (unsigned char *) gp_chunk_malloc(gp_cdsp_ae_get_workmem_size());		
		if(CdspDev->ae_workmem == 0)
		{
			DERROR("ae_workmem alloc Err!\r\n");
			ret =  -1;
			goto RETURN;
		}
		
		CdspDev->awb_workmem = (unsigned char *) gp_chunk_malloc(gp_cdsp_awb_get_workmem_size());
		if(CdspDev->awb_workmem == 0)
		{
			DERROR("awb_workmem alloc Err!\r\n");
			ret =  -1;
			goto RETURN;
		}
	
		// aeawb initial
		CdspDev->color_temp_init_val = 50;
		CdspDev->sensor_ev_idx = -1;
		CdspDev->awbmode = AWB_AUTO_CVR;
		gp_cdsp_aeawb_init(CdspDev->ae_workmem, CdspDev->awb_workmem, AWB_AUTO_CVR);		
		CdspDev->ae_awb_flag = 0;

		for(i = 0 ; i < 4 ; i++)
		{
			for(j = 0 ; j < 3 ; j++) 	CdspDev->sat_yuv_level[i][j] = 0;
			for(        ; j < 6 ; j++) 	CdspDev->sat_yuv_level[i][j] = 0x20;
		}


		
		
		
		DEBUG(KERN_WARNING "CdspOpen.\n");
	} else {
		DERROR(KERN_WARNING "CdspOpenFail!\n");
		ret =  -1;
	}
	
RETURN:	
	return ret;
}

static int
gp_cdsp_release(
	struct inode *inode,
	struct file *filp
)
{	
	if(CdspDev->OpenCnt == 1) 
	{	
		/* stop kthread for AE AWB */
		if(CdspDev->ae_awb_task != NULL)
		{
			kthread_stop(CdspDev->ae_awb_task);		
			CdspDev->ae_awb_task = NULL;
		}
		if(CdspDev->ae_workmem != 0) gp_chunk_free(CdspDev->ae_workmem);
		if(CdspDev->awb_workmem != 0) gp_chunk_free(CdspDev->awb_workmem);
		
		CdspDev->getSensorInfo = 0;
		CdspDev->ae_workmem = 0;
		CdspDev->awb_workmem = 0;
		CdspDev->ae_awb_flag = 0;

		// kill tasklet of sensor_set_exp_tasklet 
		tasklet_kill(&sensor_set_exp_tasklet);

		gpHalCdspSetIntEn(DISABLE, CDSP_INT_ALL);	
		gpHalCdspClrIntStatus(CDSP_INT_ALL);
		gpHalCdspDataSource(C_CDSP_SKIP_WR);
		gpHalCdspRedoTriger(DISABLE);

		gpHalCdspReset();
		gpHalFrontReset();
		gpHalCdspDataSource(C_CDSP_SKIP_WR);
		gpHalCdspModuleRest(1);
		gpCdspClockEnable(0);
		
		CdspDev->cdsp_feint_flag = 0;
		CdspDev->cdsp_eof_cnt = 0;
		CdspDev->OpenCnt = 0;
		CdspDev->RunFlag = 0;
		CdspDev->SyncFlag = 0;
		CdspDev->MclkEn = 0;
		CdspDev->CapCnt = 0;
		CdspDev->SkipCnt = 0;
		CdspDev->TotalCnt = 0;
		CdspDev->abBufFlag = 0;
		CdspDev->aInQueIdx = 0;
		CdspDev->bInQueIdx = 0;
		CdspDev->sensor_status = CMS_IDLE;
		CdspDev->sdidx = NO_INPUT;
		
		memset(CdspDev->bfaddr, 0, C_BUFFER_MAX);
		memset(CdspDev->bf, 0, C_BUFFER_MAX);
		memset(CdspDev->in_que, 0xFF, C_BUFFER_MAX);
		memset(CdspDev->out_que, 0xFF, C_BUFFER_MAX);
		CdspDev->imgSrc = C_CDSP_SDRAM;
		CdspDev->inFmt = 0;
		CdspDev->rawFmtFlag = 0;
		CdspDev->imgWidth = CdspDev->imgHeight = 0;
		CdspDev->imgRbWidth = CdspDev->imgRbHeight = 0;

		memset(&CdspDev->scale, 0x00, sizeof(CdspDev->scale));
		memset(&CdspDev->suppression, 0x00, sizeof(CdspDev->suppression));
		memset(&CdspDev->result, 0x00, sizeof(CdspDev->result));
		CdspDev->suppression.suppressen = ENABLE;

		if(CdspDev->sensor_cdsp.ob)
			kfree(CdspDev->sensor_cdsp.ob);
		if(CdspDev->sensor_cdsp.lenscmp)
			kfree(CdspDev->sensor_cdsp.lenscmp);
		if(CdspDev->sensor_cdsp.wb_gain)
			kfree(CdspDev->sensor_cdsp.wb_gain);
		if(CdspDev->sensor_cdsp.gamma1)
			kfree(CdspDev->sensor_cdsp.gamma1);
		if(CdspDev->sensor_cdsp.color_matrix1)
			kfree(CdspDev->sensor_cdsp.color_matrix1);
		if(CdspDev->sensor_cdsp.gamma2)
			kfree(CdspDev->sensor_cdsp.gamma2);
		if(CdspDev->sensor_cdsp.color_matrix2)
			kfree(CdspDev->sensor_cdsp.color_matrix2);
		memset(&CdspDev->sensor_cdsp, 0x0, sizeof(CdspDev->sensor_cdsp));
		
		if(CdspDev->sensor_cdsp.awb_thr) {
			kfree(CdspDev->sensor_cdsp.awb_thr);
			CdspDev->sensor_cdsp.awb_thr = 0;
		}
		
		DEBUG(KERN_WARNING "CdspClose.\n");
	} 
	else {	
		DEBUG(KERN_WARNING "CdspCloseFail.\n");
		return -1;
	}

	return 0;
}

struct file_operations cdsp_fops =
{
	.owner = THIS_MODULE,
	.poll = gp_cdsp_poll,
	.unlocked_ioctl = gp_cdsp_ioctl,
	.open = gp_cdsp_open,
	.release = gp_cdsp_release,
};

static void
gp_cdsp_device_release(
	struct device *dev
)
{
	DIAG_INFO("remove cdsp device ok\n");
}

static struct platform_device gp_cdsp_device = {
	.name = "gp-cdsp",
	.id	= 0,
	.dev =
	{
		.release = gp_cdsp_device_release,
	},
};

#ifdef CONFIG_PM
static int
gp_cdsp_suspend(
	struct platform_device *pdev,
	pm_message_t state
)
{
	if(CdspDev->OpenCnt > 0) {
		if(CdspDev->cb_func->standby) {
			CdspDev->cb_func->standby(1);
		}
		gpCdspClockEnable(0);
	}
	return 0;
}

static int
gp_cdsp_resume(
	struct platform_device *pdev
)
{
	if(CdspDev->OpenCnt > 0) {
		gpCdspClockEnable(1);
		gpHalCdspReset();
		gpHalFrontReset();
		if(CdspDev->cb_func->standby) {
			CdspDev->cb_func->standby(0);
		}
	}
	return 0;
}
#else
#define gp_cdsp_suspend NULL
#define gp_cdsp_resume NULL
#endif

static struct platform_driver gp_cdsp_driver =
{
	.suspend = gp_cdsp_suspend,
	.resume = gp_cdsp_resume,
	.driver	=
	{
		.owner	= THIS_MODULE,
		.name	= "gp-cdsp"
	},
};

static int __init
cdsp_init_module(
	void
)
{
	int nRet = -ENOMEM;

	DEBUG(KERN_WARNING "ModuleInit: cdsp \n");
	/* memory alloc */
	CdspDev = (gpCdspDev_t *)kzalloc(sizeof(gpCdspDev_t), GFP_KERNEL);
	if(!CdspDev) {
		RETURN(-1);
	}

	CdspDev->aeAddr[0] = (unsigned char *)gp_chunk_malloc(64*2);
	CdspDev->aeAddr[1] = CdspDev->aeAddr[0] + 64;	
	if(CdspDev->aeAddr[0] == 0) {
		RETURN(-1);
	}

	CdspDev->ae_awb_task = NULL;
	CdspDev->sdidx = NO_INPUT;
	CdspDev->suppression.suppressen = ENABLE;
	CdspDev->suppression.suppr_mode = 1;
	CdspDev->suppression.denoisen = ENABLE;
	CdspDev->suppression.lowyen = DISABLE;
	CdspDev->suppression.denoisethrl = 0x4;
	CdspDev->suppression.denoisethrwth= 0x6;
	CdspDev->suppression.yhtdiv= 0x2;
	CdspDev->edge_day = CdspDev->edge_night = 1;
	
	memset(CdspDev->in_que, 0xFF, C_BUFFER_MAX);
	memset(CdspDev->out_que, 0xFF, C_BUFFER_MAX);
	
	/* register irq */
	nRet = request_irq(IRQ_CDSP,
					  gp_cdsp_irq_handler,
					  IRQF_DISABLED,
					  "CDSP_IRQ",
					  CdspDev);
	if(nRet < 0) {
		RETURN(-1);
	}
	
	/* initialize */
	init_MUTEX(&CdspDev->sem);
	init_waitqueue_head(&CdspDev->cdsp_wait_queue);
	init_waitqueue_head(&CdspDev->ae_awb_wait_queue); // comi
	//CdspDev->cdsp_splock = SPIN_LOCK_UNLOCKED; // comi
	init_MUTEX(&CdspDev->aeawb_sem); // comi
	

	/* register char device */
	CdspDev->dev.name  = "cdsp";
	CdspDev->dev.minor = MISC_DYNAMIC_MINOR;
	CdspDev->dev.fops  = &cdsp_fops;
	nRet = misc_register(&CdspDev->dev);
	if(nRet < 0) {
		DERROR("cdsp device register fail\n");
		RETURN(-ENXIO);
	}

	/* register platform device/driver */
	platform_device_register(&gp_cdsp_device);
	platform_driver_register(&gp_cdsp_driver);
	
__return:
	if(nRet < 0) {
		DERROR(KERN_WARNING "CdspInitFail\n");
		/* free irq */
		free_irq(IRQ_CDSP, CdspDev);

		/* free char device */
		misc_deregister(&CdspDev->dev);

		/* platform unregister */
		platform_device_unregister(&gp_cdsp_device);
		platform_driver_unregister(&gp_cdsp_driver);


		/* stop kthread for AE AWB */
		kthread_stop(CdspDev->ae_awb_task);

		/* free memory */
		gp_chunk_free((void *)CdspDev->aeAddr[0]);
		kfree(CdspDev);

		
	}
	return nRet;
}

static void __exit
cdsp_exit_module(
	void
)
{
	DEBUG(KERN_WARNING "ModuleExit: cdsp \n");
	/* free irq */
	free_irq(IRQ_CDSP, CdspDev);

	/* free char device */
	misc_deregister(&CdspDev->dev);

	/* platform unregister */
	platform_device_unregister(&gp_cdsp_device);
	platform_driver_unregister(&gp_cdsp_driver);

	/* free memory */
	gp_chunk_free((void *)CdspDev->aeAddr[0]);
	kfree(CdspDev);
}

module_init(cdsp_init_module);
module_exit(cdsp_exit_module);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus CDSP Driver");
MODULE_LICENSE_GP;
MODULE_VERSION("1.0");



