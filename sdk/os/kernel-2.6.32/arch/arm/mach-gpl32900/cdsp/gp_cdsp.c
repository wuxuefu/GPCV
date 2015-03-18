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

#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

#include <mach/gp_chunkmem.h>
#include <mach/gp_cache.h>
#include <mach/hal/hal_cdsp.h>
#include <mach/hal/hal_clock.h>
#include <mach/gp_cdsp.h>
#include <mach/gp_mipi.h>
#include <mach/gp_board.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/sensor_mgr.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define C_BUFFER_MAX	5
#define C_DMA_A			1
#define	NO_INPUT		0xFFFFFFFF
#define USBPHY_CLK		96000000
#define CDSP_CHK_MIPI_EN	0
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

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gpCdspDev_s
{
	struct miscdevice dev;
	struct semaphore sem;
	wait_queue_head_t cdsp_wait_queue;
	bool cdsp_feint_flag;

	unsigned int OpenCnt;	/* open count */
	unsigned int SyncFlag;	/* Cdsp Sync Flag */
	unsigned int CapCnt;	/* capture count */
	
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
}gpCdspDev_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
unsigned short g_lenscmp_table[] =
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

unsigned char g_gamma_table[512] =
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

unsigned char g_edge_table[256] =
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
static void
gpCdspClockEnable(
	unsigned int enable
)
{
	unsigned char div;
	int clock_out;
	struct clk *clock;

	clock = clk_get(NULL, "clk_sys_ahb");
	clock_out = clk_get_rate(clock);
	div = clock_out/100000000;
#ifdef CONFIG_PM
	if(enable) {
		/*For CDSP write*/
		gp_enable_clock((int*)"READLTIME_ABT", 1);
		/*For CDSP read*/
		gp_enable_clock((int*)"2DSCAABT", 1);
		gpHalScuClkEnable(SCU_A_PERI_CDSP, SCU_A, 1);
		gpHalCdspSetClk(C_CDSP_CLK_ENABLE, div);
		gpHalCdspSetModuleReset(1);
	} else {
		gpHalCdspSetClk(C_CDSP_CLK_DISABLE, 0);
		gpHalScuClkEnable(SCU_A_PERI_CDSP, SCU_A, 0);
		/*For CDSP write*/
		gp_enable_clock((int*)"READLTIME_ABT", 0);
		/*For CDSP read*/
		gp_enable_clock((int*)"2DSCAABT", 0);
	}
#else
	gpHalScuClkEnable(SCU_A_PERI_CDSP, SCU_A, enable);
	gpHalCdspSetClk(C_CDSP_CLK_ENABLE, div);
	gpHalCdspSetModuleReset(1);
#endif
}

static int
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
		return -1;
	}
	return 0;
}

static int
gpCdspSetFifoSize(
	unsigned short width
)
{
	while(1) {
		if(width <= 0x100) {
			break;
		}
		width >>= 1;
	}

	DEBUG(KERN_WARNING "fifo=0x%x.\n", width);
	gpHalCdspSetSRAM(ENABLE, width);
	gpHalCdspSetLineInterval(0x28);
	return width;
}

static void
gpCdspSetBadPixOb(
	gpCdspBadPixOB_t *argp,
	gpCdspWhtBal_t *wh_bal
)
{
	/* bad pixel */
	if(argp->badpixen) {
		gpHalCdspSetBadPixel(argp->bprthr, argp->bpgthr, argp->bpbthr);
		gpHalCdspSetBadPixelEn(argp->badpixen, 0x03); /* enable mirror */
	} else {
		gpHalCdspSetBadPixelEn(DISABLE, 0x03);
	}

	/* optical black */
	if(argp->manuoben || argp->autooben) {
		/* enable white balance offset when use ob */
		if(wh_bal->wboffseten == 0) {
			wh_bal->wboffseten = 1;
		}
		
		gpHalCdspSetWbOffset(wh_bal->wboffseten, wh_bal->roffset, wh_bal->groffset, wh_bal->boffset, wh_bal->gboffset);
		gpHalCdspSetManuOB(argp->manuoben, argp->manuob);
		gpHalCdspSetAutoOB(argp->autooben, argp->obtype, argp->obHOffset, argp->obVOffset);
	} else {
		gpHalCdspSetManuOB(DISABLE, argp->manuob);
		gpHalCdspSetAutoOB(DISABLE, argp->obtype, argp->obHOffset, argp->obVOffset);
	}
}

static void
gpCdspGetBadPixOb(
	gpCdspBadPixOB_t *argp
)
{
	/* bad pixel */
	gpHalCdspGetBadPixelEn(&argp->badpixen, &argp->reserved0);
	gpHalCdspGetBadPixel(&argp->bprthr, &argp->bpgthr, &argp->bpbthr);

	/* optical black */
	gpHalCdspGetManuOB(&argp->manuoben, &argp->manuob);
	gpHalCdspGetAutoOB(&argp->autooben, &argp->obtype, &argp->obHOffset, &argp->obVOffset);
	gpHalCdspGetAutoOBAvg(&argp->Ravg, &argp->GRavg, &argp->Bavg, &argp->GBavg);
}

static void
gpCdspSetLensCmp(
	unsigned char raw_flag,
	gpCdspLenCmp_t *argp
)
{
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

static void
gpCdspGetLensCmp(
	gpCdspLenCmp_t *argp
)
{
	gpHalCdspGetLensCmpPos(&argp->centx, &argp->centy, &argp->xmoffset, &argp->ymoffset);
	gpHalCdspGetLensCmp(&argp->stepfactor, &argp->xminc, &argp->ymoinc, &argp->ymeinc);
	argp->lcen = gpHalCdspGetLensCmpEn();
	argp->lenscmp_table = NULL;
}

static void
gpCdspSetWhiteBalance(
	gpCdspWhtBal_t *argp
)
{
	gpHalCdspSetWbOffset(argp->wboffseten,argp->roffset, argp->groffset, argp->boffset, argp->gboffset);
	gpHalCdspSetWbGain(argp->wbgainen, argp->rgain, argp->grgain, argp->bgain, argp->gbgain);
	if(argp->wbgainen) {
		gpHalCdspSetGlobalGain(argp->global_gain);
	}
}

static void
gpCdspGetWhiteBalance(
	gpCdspWhtBal_t *argp
)
{
	gpHalCdspGetWbOffset(&argp->wboffseten, &argp->roffset, &argp->groffset, &argp->boffset, &argp->gboffset);
	gpHalCdspGetWbGain(&argp->wbgainen, &argp->rgain, &argp->grgain, &argp->bgain, &argp->gbgain);
	argp->global_gain =	gpHalCdspGetGlobalGain();
}

static void
gpCdspSetLutGamma(
	gpCdspGamma_t *argp
)
{
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

static void
gpCdspGetLutGamma(
	gpCdspGamma_t *argp
)
{
	argp->lut_gamma_en = gpHalCdspGetLutGammaEn();
	argp->gamma_table = NULL;
}

static void
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

static void
gpCdspGetIntpl(
	gpCdspIntpl_t *argp
)
{
	gpHalCdspGetIntplThr(&argp->int_low_thr, &argp->int_hi_thr);
	argp->rawspecmode = gpHalCdspGetRawSpecMode();
}

static void
gpCdspSetEdge(
	unsigned char raw_flag,
	gpCdspEdge_t *argp
)
{
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
			gpHalCdspSetEdgeFilter(0, argp->lf00, argp->lf01, argp->lf02);
			gpHalCdspSetEdgeFilter(1, argp->lf10, argp->lf11, argp->lf12);
			gpHalCdspSetEdgeFilter(2, argp->lf20, argp->lf21, argp->lf22);
		}
	} else {
		gpHalCdspSetEdgeEn(DISABLE);
		gpHalCdspSetEdgeLutTableEn(DISABLE);
	}
}

static void
gpCdspGetEdge(
	gpCdspEdge_t *argp
)
{
	argp->eluten = gpHalCdspGetEdgeLutTableEn();
	argp->edge_table = NULL;
	argp->edgeen = gpHalCdspGetEdgeEn();
	gpHalCdspGetEdgeLCoring(&argp->lhdiv, &argp->lhtdiv, &argp->lhcoring, &argp->lhmode);
	argp->ampga = gpHalCdspGetEdgeAmpga();
	argp->edgedomain = gpHalCdspGetEdgeDomain();
	argp->Qthr = gpHalCdspGetEdgeQCnt();

	gpHalCdspGetEdgeFilter(0, &argp->lf00, &argp->lf01, &argp->lf02);
	gpHalCdspGetEdgeFilter(1, &argp->lf10, &argp->lf11, &argp->lf12);
	gpHalCdspGetEdgeFilter(2, &argp->lf20, &argp->lf21, &argp->lf22);
}

static void
gpCdspSetColorMatrix(
	gpCdspCorMatrix_t *argp
)
{
	if(argp->colcorren) {
		gpHalCdspSetColorMatrix(0, argp->a11, argp->a12, argp->a13);
		gpHalCdspSetColorMatrix(1, argp->a21, argp->a22, argp->a23);
		gpHalCdspSetColorMatrix(2, argp->a31, argp->a32, argp->a33);
		gpHalCdspSetColorMatrixEn(argp->colcorren);
	} else {
		gpHalCdspSetColorMatrixEn(DISABLE);
	}
}

static void
gpCdspGetColorMatrix(
	gpCdspCorMatrix_t *argp
)
{
	gpHalCdspGetColorMatrix(0, &argp->a11, &argp->a12, &argp->a13);
	gpHalCdspGetColorMatrix(1, &argp->a21, &argp->a22, &argp->a23);
	gpHalCdspGetColorMatrix(2, &argp->a31, &argp->a32, &argp->a33);
	argp->colcorren = gpHalCdspGetColorMatrixEn();
}

static void
gpCdspSetRgbToYuv(
	gpCdspRgb2Yuv_t *argp
)
{
	gpHalCdspSetPreRBClamp(argp->pre_rbclamp);
	gpHalCdspSetRBClamp(argp->rbclampen, argp->rbclamp);
	if(argp->uvdiven) {
		gpHalCdspSetUvDivideEn(argp->uvdiven);
		gpHalCdspSetUvDivide(argp->Yvalue_T1, argp->Yvalue_T2, argp->Yvalue_T3,
							argp->Yvalue_T4, argp->Yvalue_T5, argp->Yvalue_T6);
	} else {
		gpHalCdspSetUvDivideEn(DISABLE);
	}
}

static void
gpCdspGetRgbToYuv(
	gpCdspRgb2Yuv_t *argp
)
{
	argp->pre_rbclamp = gpHalCdspGetPreRBClamp();
	gpHalCdspGetRBClamp(&argp->rbclampen, (unsigned char *)&argp->rbclamp);
	argp->uvdiven = gpHalCdspGetUvDivideEn();
	gpHalCdspGetUvDivide(&argp->Yvalue_T1, &argp->Yvalue_T2, &argp->Yvalue_T3,
						&argp->Yvalue_T4, &argp->Yvalue_T5, &argp->Yvalue_T6);
}

static void
gpCdspSetYuv444Insert(
	gpCdspYuvInsert_t *argp
)
{
	unsigned char y_value, u_value, v_value;

	y_value = ((argp->y_corval & 0x0F) << 4)|(argp->y_coring & 0xF);
	u_value = ((argp->u_corval & 0x0F) << 4)|(argp->u_coring & 0xF);
	v_value = ((argp->v_corval & 0x0F) << 4)|(argp->v_coring & 0xF);
	gpHalCdspSetYuv444InsertEn(argp->yuv444_insert);
	gpHalCdspSetYuvCoring(y_value, u_value, v_value);
}

static void
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

static void
gpCdspSetYuvHavg(
	gpCdspYuvHAvg_t *argp
)
{
	gpHalCdspSetYuvHAvg(0x03, argp->ytype, argp->utype, argp->vtype);
}

static void
gpCdspGetYuvHavg(
	gpCdspYuvHAvg_t *argp
)
{
	gpHalCdspGetYuvHAvg(&argp->reserved, &argp->ytype, &argp->utype, &argp->vtype);
}

static void
gpCdspSetSpecialMode(
	gpCdspSpecMod_t *argp
)
{
	gpHalCdspSetYuvSpecModeBinThr(argp->binarthr);
	gpHalCdspSetYuvSpecMode(argp->yuvspecmode);
}

static void
gpCdspGetSpecialMode(
	gpCdspSpecMod_t *argp
)
{
	argp->binarthr = gpHalCdspGetYuvSpecModeBinThr();
	argp->yuvspecmode = gpHalCdspGetYuvSpecMode();
}

static void
gpCdspSetSatHue(
	gpCdspSatHue_t *argp
)
{
	gpHalCdspSetYuvSPHue(argp->u_huesindata, argp->u_huecosdata, argp->v_huesindata, argp->v_huecosdata);
	gpHalCdspSetYuvSPEffOffset(argp->y_offset, argp->u_offset, argp->v_offset);
	gpHalCdspSetYuvSPEffScale(argp->y_scale, argp->u_scale, argp->v_scale);
	gpHalCdspSetBriContEn(argp->YbYcEn);
}

static void
gpCdspGetSatHue(
	gpCdspSatHue_t *argp
)
{
	gpHalCdspGetYuvSPHue(&argp->u_huesindata, &argp->u_huecosdata, &argp->v_huesindata, &argp->v_huecosdata);
	gpHalCdspGetYuvSPEffOffset(&argp->y_offset, &argp->u_offset, &argp->v_offset);
	gpHalCdspGetYuvSPEffScale(&argp->y_scale, &argp->u_scale, &argp->v_scale);
	argp->YbYcEn = gpHalCdspGetBriContEn();
}

static void
gpCdspSetSuppression(
	unsigned short width,
	gpCdspSuppression_t *argp,
	gpCdspEdge_t *edge
)
{
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

static void
gpCdspGetSuppression(
	gpCdspSuppression_t *argp
)
{
	gpHalCdspGetYDenoise(&argp->denoisethrl, &argp->denoisethrwth, &argp->yhtdiv);
	argp->denoisen = gpHalCdspGetYDenoiseEn();
	argp->lowyen = gpHalCdspGetYLPFEn();
}

static void
gpCdspSetRawWin(
	unsigned short width,
	unsigned short height,
	gpCdspRawWin_t *argp
)
{
	unsigned int x, y;

	width -= 8;
	height -= 8;
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

	DEBUG(KERN_WARNING "RawWinOffset[%d,%d]\n", argp->hwdoffset, argp->vwdoffset);
	DEBUG(KERN_WARNING "RawWinCellSize[%d,%d]\n", argp->hwdsize, argp->vwdsize);

	gpHalCdspSetAeAwbSrc(argp->aeawb_src);
	gpHalCdspSetAeAwbSubSample(argp->subample);
	gpHalCdspSet3ATestWinEn(argp->AeWinTest, argp->AfWinTest);
	gpHalCdspSetRGBWin(argp->hwdoffset, argp->vwdoffset, argp->hwdsize, argp->vwdsize);
}

static void
gpCdspGetRawWin(
	gpCdspRawWin_t *argp
)
{
	argp->aeawb_src = gpHalCdspGetAeAwbSrc();
	argp->subample = gpHalCdspGetAeAwbSubSample();
	gpHalCdspGet3ATestWinEn(&argp->AeWinTest, &argp->AfWinTest);
	gpHalCdspGetRGBWin(&argp->hwdoffset, &argp->vwdoffset, &argp->hwdsize, &argp->vwdsize);
}

static void
gpCdspSetAF(
	unsigned short width,
	unsigned short height,
	gpCdspAF_t *argp
)
{
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

static void
gpCdspGetAF(
	gpCdspAF_t *argp
)
{
	gpHalCdspGetAfWin1(&argp->af1_hoffset, &argp->af1_voffset, &argp->af1_hsize, &argp->af1_hsize);
	gpHalCdspGetAfWin2(&argp->af2_hoffset, &argp->af2_voffset, &argp->af2_hsize, &argp->af2_hsize);
	gpHalCdspGetAfWin3(&argp->af3_hoffset, &argp->af3_voffset, &argp->af3_hsize, &argp->af3_hsize);
	gpHalCdspGetAFEn(&argp->af_win_en, &argp->af_win_hold);
}

static void
gpCdspSetAE(
	unsigned char phaccfactor,
	unsigned char pvaccfactor,
	gpCdspAE_t *argp
)
{
	if(argp->ae_win_en) {
		unsigned int ae0, ae1;
		
		ae0 = (unsigned int)gp_chunk_pa(CdspDev->aeAddr[0]);
		ae1 = (unsigned int)gp_chunk_pa(CdspDev->aeAddr[1]);
		DEBUG(KERN_WARNING "AeWinAddr0 = 0x%x.\n", ae0);
		DEBUG(KERN_WARNING "AeWinAddr1 = 0x%x.\n", ae1);

		gpHalCdspSetAEBuffAddr(ae0, ae1);
		gpHalCdspSetAEEn(argp->ae_win_en, argp->ae_win_hold);
		gpHalCdspSetAEWin(phaccfactor, pvaccfactor);
		gpHalCdspSetIntEn(ENABLE, CDSP_AEWIN_SEND);
	} else {
		gpHalCdspSetAEEn(DISABLE, DISABLE);
		gpHalCdspSetIntEn(DISABLE, CDSP_AEWIN_SEND);
	}
}

static void
gpCdspGetAE(
	gpCdspAE_t *argp
)
{
	gpHalCdspGetAEEn(&argp->ae_win_en, &argp->ae_win_hold);
}

static void
gpCdspSetAWB(
	gpCdspAWB_t *argp
)
{
	if(argp->awb_win_en) {
		gpHalCdspSetAWB(argp->awbclamp_en, argp->sindata, argp->cosdata, argp->awbwinthr);
		gpHalCdspSetAwbYThr(argp->Ythr0, argp->Ythr1, argp->Ythr2, argp->Ythr3);
		gpHalCdspSetAwbUVThr(1,	argp->UL1N1, argp->UL1P1, argp->VL1N1, argp->VL1P1);
		gpHalCdspSetAwbUVThr(2,	argp->UL1N2, argp->UL1P2, argp->VL1N2, argp->VL1P2);
		gpHalCdspSetAwbUVThr(3,	argp->UL1N3, argp->UL1P3, argp->VL1N3, argp->VL1P3);
		gpHalCdspSetAWBEn(argp->awb_win_en, argp->awb_win_hold);
		gpHalCdspSetIntEn(ENABLE, CDSP_AWBWIN_UPDATE);
	} else {
		gpHalCdspSetAWBEn(DISABLE, DISABLE);
		gpHalCdspSetIntEn(DISABLE, CDSP_AWBWIN_UPDATE);
	}
}

static void
gpCdspGetAWB(
	gpCdspAWB_t *argp
)
{
	gpHalCdspGetAWB(&argp->awbclamp_en, &argp->sindata, &argp->cosdata, &argp->awbwinthr);
	gpHalCdspGetAwbYThr(&argp->Ythr0, &argp->Ythr1, &argp->Ythr2, &argp->Ythr3);
	gpHalCdspGetAwbUVThr(1,	&argp->UL1N1, &argp->UL1P1, &argp->VL1N1, &argp->VL1P1);
	gpHalCdspGetAwbUVThr(2,	&argp->UL1N2, &argp->UL1P2, &argp->VL1N2, &argp->VL1P2);
	gpHalCdspGetAwbUVThr(3,	&argp->UL1N3, &argp->UL1P3, &argp->VL1N3, &argp->VL1P3);
	gpHalCdspGetAWBEn(&argp->awb_win_en, &argp->awb_win_hold);
}

static void
gpCdspSetWBGain2(
	gpCdspWbGain2_t *argp
)
{
	if(argp->wbgain2en) {
		gpHalCdspSetWbGain2(argp->rgain2, argp->ggain2, argp->bgain2);
		gpHalCdspSetWbGain2En(ENABLE);
	} else {
		gpHalCdspSetWbGain2En(DISABLE);
	}
}

static void
gpCdspGetWBGain2(
	gpCdspWbGain2_t *argp
)
{
	gpHalCdspGetWbGain2(&argp->rgain2, &argp->ggain2, &argp->bgain2);
	argp->wbgain2en = gpHalCdspGetWbGain2En();
}

static void
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

static void
gpCdspGetHistgm(
	gpCdspHistgm_t *argp
)
{
	gpHalCdspGetHistgm(&argp->hislowthr, &argp->hishighthr);
	gpHalCdspGetHistgmEn(&argp->his_en, &argp->his_hold_en);
}

static int
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
		DEBUG(KERN_WARNING "YuvHScaleEn\n");
		if(argp->yuv_dst_hsize > argp->img_rb_h_size) {
			argp->yuv_dst_hsize = argp->img_rb_h_size;
		}
		
		argp->yuv_dst_hsize &= ~(0x1); 	/* 2 align */
		clamphsizeen = 1;
		clamphsize = argp->yuv_dst_hsize;
		temp = (argp->yuv_dst_hsize<<16)/src_width + 1;
		gpHalCdspSetYuvHScale(temp, temp);
		gpHalCdspSetYuvHScaleEn(argp->yuvhscale_en, argp->yuvhscale_mode);
	} else if(src_width > argp->img_rb_h_size) {
		DEBUG(KERN_WARNING "YuvHScaleEn1\n");
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
		DEBUG(KERN_WARNING "YuvVScaleEn\n");
		if(argp->yuv_dst_vsize >  argp->img_rb_v_size) {
			argp->yuv_dst_vsize = argp->img_rb_v_size;
		}
		
		argp->yuv_dst_vsize &= ~(0x1); 	/* 2 align */
		temp = (argp->yuv_dst_vsize<<16)/src_height + 1;
		gpHalCdspSetYuvVScale(temp, temp);
		gpHalCdspSetYuvVScaleEn(argp->yuvvscale_en, argp->yuvvscale_mode);
	} else if(src_height > argp->img_rb_v_size) {
		DEBUG(KERN_WARNING "YuvVScaleEn1\n");
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
	CdspDev->imgRbWidth = CdspDev->imgWidth = argp->img_rb_h_size;
	CdspDev->imgRbHeight = CdspDev->imgHeight = argp->img_rb_v_size;
	DEBUG(KERN_WARNING "Clampen = %d\n", clamphsizeen);
	DEBUG(KERN_WARNING "ClampSize = %d\n", clamphsize);
	DEBUG(KERN_WARNING "rbWidth  = %d\n", CdspDev->imgRbWidth);
	DEBUG(KERN_WARNING "rbHeight = %d\n", CdspDev->imgRbHeight);
	return 0;
}

static void
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

static void
gpCdspSetScaleAeAf(
	gpCdspScalePara_t *argp
)
{
	unsigned short w, h;
	gpCdspRawWin_t raw_win;
	gpCdspAE_t ae;
	gpCdspAF_t af;

	if(argp->crop_en) {
		w = argp->crop_hsize;
		h = argp->crop_vsize;
	} else {
		w = CdspDev->imgWidth;
		h = CdspDev->imgHeight;
	}

	gpCdspGetRawWin(&raw_win);
	gpCdspSetRawWin(w, h, &raw_win);

	gpCdspGetAF(&af);
	gpCdspSetAF(w, h, &af);

	gpCdspGetAE(&ae);
	gpCdspSetAE(raw_win.hwdsize, raw_win.vwdsize, &ae);
}

static int
check_rqbuf_type(
	void
)
{
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

static int
gpCdspPostProcess(
	gpCdspPostProcess_t	*PostPress
)
{
	unsigned int cdsp_inFmt;
	unsigned int rawFlag;
	unsigned int rawFmt;
	unsigned int addr, size;
	gpCdspYuvHAvg_t yuv_havg;	
	gpCdspLenCmp_t lens_cmp;
	gpCdspScalePara_t scale;

	DEBUG("%s\n", __FUNCTION__);
	if(CdspDev->imgSrc != C_CDSP_SDRAM) {
		DEBUG(KERN_WARNING "imag source is not SDRAM\n");
		return -1;
	}
	
	switch(PostPress->inFmt)	
	{	
	case V4L2_PIX_FMT_VYUY:
		cdsp_inFmt = C_SDRAM_FMT_VY1UY0;				
		rawFlag = 0;
		rawFmt = 0;
		break;

	case V4L2_PIX_FMT_SBGGR8:
		cdsp_inFmt = C_SDRAM_FMT_RAW8;
		rawFlag = 1;
		rawFmt = 2;
		break;

	case V4L2_PIX_FMT_SGBRG8:
		cdsp_inFmt = C_SDRAM_FMT_RAW8;
		rawFlag = 1;
		rawFmt = 3;
		break;
		
	case V4L2_PIX_FMT_SGRBG8:
		cdsp_inFmt = C_SDRAM_FMT_RAW8;
		rawFlag = 1;
		rawFmt = 0;
		break;

	case V4L2_PIX_FMT_SGRBG10:	
		cdsp_inFmt = C_SDRAM_FMT_RAW10;
		rawFlag = 1;
		rawFmt = 1;
		break;

	default:
		DEBUG(KERN_WARNING "input Format ERROR\n");
		return -1;	
	}

	/* cdsp clock */
	gpCdspSetClkPath(CdspDev->imgSrc, CdspDev->rawFmtFlag);

	/* cdsp reset */
	gpHalCdspReset();
	gpHalCdspFrontReset();
	gpHalCdspDataSource(C_CDSP_SDRAM);

	/* set module */	
	if(CdspDev->rawFmtFlag) {		
		gpCdspIntpl_t intpl;		
		gpCdspEdge_t edge;
		
		gpCdspGetIntpl(&intpl);		
		gpCdspSetIntpl(CdspDev->imgWidth, &intpl);		
		gpCdspGetEdge(&edge);		
		gpCdspSetEdge(rawFlag, &edge);	
	} else {				
		gpCdspEdge_t edge;
		gpCdspSuppression_t suppression;
		
		gpCdspGetEdge(&edge);		
		gpCdspSetSuppression(PostPress->width, &suppression, &edge);	
	}	

	gpCdspGetLensCmp(&lens_cmp);	
	gpCdspSetLensCmp(rawFlag, &lens_cmp);	
	gpCdspGetYuvHavg(&yuv_havg);	
	gpCdspSetYuvHavg(&yuv_havg);	

	/* set scale & crop */	
	gpCdspSetScaleCrop(&scale);	
	gpCdspSetScaleAeAf(&scale);	
	
	/* set fifo */	
	gpCdspSetFifoSize(PostPress->width);
	
	if(rawFlag) {
		DEBUG(KERN_WARNING "SDRAMRawFmt\n");
		gpHalCdspSetYuvRange(0x00);
		gpHalCdspSetRawDataFormat(rawFmt);
		gpHalCdspFrontSetInputFormat(cdsp_inFmt);

		addr = (unsigned int)gp_user_va_to_pa((void *)PostPress->inAddr);
		gpHalCdspSetRawBuff(PostPress->width, PostPress->height, 0x00,addr);
		
		addr = (unsigned int)gp_user_va_to_pa((void *)PostPress->outAddr);
		gpHalCdspSetYuvBuffA(PostPress->width, PostPress->height, addr);
		gpHalCdspSetDmaBuff(RD_A_WR_A);

		size = PostPress->width*PostPress->height;
		gpHalCdspSetReadBackSize(0x00, 0x00, PostPress->width, PostPress->height);
	} else {
		DEBUG(KERN_WARNING "SDRAMYuvFmt\n");
		gpHalCdspSetYuvRange(PostPress->yuvRange);
		gpHalCdspSetRawDataFormat(0x00);
		gpHalCdspFrontSetInputFormat(cdsp_inFmt);

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
	int i, ret, div, idx;
	unsigned int cdsp_inFmt;
	struct clk *clock;
	gpCdspYuvHAvg_t yuv_havg;	
	gpCdspLenCmp_t lens_cmp;

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

	/* cdsp clock */
	gpCdspSetClkPath(CdspDev->imgSrc, CdspDev->rawFmtFlag);
	
	/* cdsp reset */
	gpHalCdspReset();
	gpHalCdspFrontReset();
	gpHalCdspDataSource(C_CDSP_SDRAM);

	/* set module */	
	if(CdspDev->rawFmtFlag) {		
		gpCdspIntpl_t intpl;		
		gpCdspEdge_t edge;
		
		gpCdspGetIntpl(&intpl);		
		gpCdspSetIntpl(CdspDev->imgWidth, &intpl);		
		gpCdspGetEdge(&edge);		
		gpCdspSetEdge(CdspDev->rawFmtFlag, &edge);	
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
			DEBUG(KERN_WARNING "FrontRawFmt\n");
			gpHalCdspSetYuvRange(0x00);
			gpHalCdspSetRawDataFormat(CdspDev->rawFmt);
		} else {
			DEBUG(KERN_WARNING "FrontYuvFmt\n");
			gpHalCdspSetYuvRange(0x00);
			gpHalCdspSetRawDataFormat(0x00);
		}

		gpHalCdspFrontSetInputFormat(cdsp_inFmt);
		//gpHalCdspFrontSetInterlace(FIELD_ODDL, sensor->sensor_interlace_mode);
		gpHalCdspFrontSetInterface((sensor->sensor_timing_mode == CCIR656) ? 1 : 0,
								sensor->sensor_hsync_mode ? 0 : 1,
								sensor->sensor_vsync_mode ? 0 : 1,
								CdspDev->SyncFlag);
		if((sensor->sensor_timing_mode == HREF) || (sensor->fmt[idx].voffset == 0)) {
			ret = sensor->fmt[idx].vline - 1;
		} else {
			ret = sensor->fmt[idx].vline;
		}
		gpHalCdspFrontSetFrameSize(sensor->fmt[idx].hoffset, sensor->fmt[idx].voffset,
								sensor->fmt[idx].hpixel, ret);
		break;

	case C_CDSP_MIPI:
		if(CdspDev->rawFmtFlag) 
		{		
			DEBUG(KERN_WARNING "MipiRawFmt\n");		
			gpHalCdspSetYuvRange(0x00);		
			gpHalCdspSetRawDataFormat(CdspDev->rawFmt);	
		} else {		
			DEBUG(KERN_WARNING "MipiYuvFmt\n");		
			gpHalCdspSetYuvRange(0x00);		
			gpHalCdspSetRawDataFormat(0x00);	
		}	

		gpHalCdspFrontSetInputFormat(cdsp_inFmt);	
		gpHalCdspFrontSetMipiFrameSize(sensor->fmt[idx].hoffset, sensor->fmt[idx].voffset,
									sensor->fmt[idx].hpixel, sensor->fmt[idx].vline);
		break;
		
	default:
		DEBUG(KERN_WARNING "ImgSrcErr!\n");
		return -1;
	}

	/* set csi output clock again */
	if(sensor->fmt[idx].mclk_src == CSI_CLK_SPLL) {
		clock = clk_get(NULL, "clk_ref_ceva");
		ret = clk_get_rate(clock);
	} else {
		ret = USBPHY_CLK;
	}

	div = ret/sensor->fmt[idx].mclk;			
	if((ret % sensor->fmt[idx].mclk) == 0) {
		div--;			
	}
	
	DEBUG("mclk = %d\n", ret/(div + 1));
	gpHalCdspSetMclk(sensor->fmt[idx].mclk_src, div, 0, 0);	
	return 0;
}

static int
gp_cdsp_g_ctrl(
	struct v4l2_control *ctrl
)
{
	int nRet = 0;

	DEBUG("%s\n", __FUNCTION__);
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

		case MSG_CDSP_SAT_HUE:
		{
			gpCdspSatHue_t sat_hue;

			gpCdspGetSatHue(&sat_hue);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&sat_hue, sizeof(sat_hue));
			break;
		}

		case MSG_CDSP_SUPPRESSION:
		{
			gpCdspGetSuppression(&CdspDev->suppression);
			nRet = copy_to_user((void __user*)ctrl->value, (void*)&CdspDev->suppression, sizeof(CdspDev->suppression));
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
			struct v4l2_control csi_ctrl;
			
			nRet = copy_from_user((void*)&csi_ctrl, (void __user*)ctrl, sizeof(csi_ctrl));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			sd = CdspDev->sd;
			nRet = sd->ops->core->g_ctrl(sd, &csi_ctrl);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = copy_to_user((void __user*)ctrl, (void*)&csi_ctrl, sizeof(csi_ctrl));
			break;
		}
	}

__return:
	return nRet;
}

static int
gp_cdsp_s_ctrl(
	struct v4l2_control *ctrl
)
{
	unsigned short w, h;
	int nRet = 0;

	DEBUG("%s\n", __FUNCTION__);
	switch(ctrl->id)
	{
		case MSG_CDSP_POSTPROCESS:
		{
			gpCdspPostProcess_t post;

			nRet = copy_from_user((void*)&post, (void __user*)ctrl->value, sizeof(post));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->imgSrc != C_CDSP_SDRAM) {
				RETURN(-EINVAL);
			}
			
			CdspDev->imgSrc = C_CDSP_SDRAM;
			CdspDev->inFmt = post.inFmt;
			CdspDev->imgWidth = CdspDev->imgRbWidth = post.width;
			CdspDev->imgHeight = CdspDev->imgRbHeight = post.height;
			CdspDev->scale.img_rb_h_size = post.width;
			CdspDev->scale.img_rb_v_size = post.height;
			CdspDev->suppression.suppressen = 1;
			if(CdspDev->inFmt == V4L2_PIX_FMT_VYUY){
				CdspDev->rawFmtFlag = 0;
			} else if(CdspDev->inFmt == V4L2_PIX_FMT_SBGGR8){
				CdspDev->rawFmtFlag = 0;
			} else if(CdspDev->inFmt == V4L2_PIX_FMT_SGBRG8){
				CdspDev->rawFmtFlag = 0;
			} else if(CdspDev->inFmt == V4L2_PIX_FMT_SGRBG8){
				CdspDev->rawFmtFlag = 0;
			} else {
				RETURN(-EINVAL);
			}
			
			nRet = gpCdspPostProcess(&post);
			break;
		}
		
		case MSG_CDSP_SCALE_CROP:
		{
			nRet = copy_from_user((void*)&CdspDev->scale, (void __user*)ctrl->value, sizeof(CdspDev->scale));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			nRet = gpCdspSetScaleCrop(&CdspDev->scale);
			gpCdspSetScaleAeAf(&CdspDev->scale);
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
				gpCdspWhtBal_t wht_bal;
				
				gpCdspGetWhiteBalance(&wht_bal);
				gpCdspSetBadPixOb(&bad_pixel, &wht_bal);
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
			break;
		}

		case MSG_CDSP_LUT_GAMMA:
		{
			gpCdspGamma_t lut_gamma;

			nRet = copy_from_user((void*)&lut_gamma, (void __user*)ctrl->value, sizeof(lut_gamma));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			if(CdspDev->rawFmtFlag) {
				gpCdspSetLutGamma(&lut_gamma);
			}
			break;
		}

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
			break;
		}

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

		case MSG_CDSP_COLOR_MATRIX:
		{
			gpCdspCorMatrix_t matrix;
			
			nRet = copy_from_user((void*)&matrix, (void __user*)ctrl->value, sizeof(matrix));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(CdspDev->rawFmtFlag) {
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
			break;
		}

		case MSG_CDSP_AE_WIN:
		{
			gpCdspAE_t ae;
			gpCdspRawWin_t raw_win;

			nRet = copy_from_user((void*)&ae, (void __user*)ctrl->value, sizeof(ae));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			gpCdspGetRawWin(&raw_win);
			gpCdspSetAE(raw_win.hwdsize, raw_win.vwdsize, &ae);
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

		default:
		{
			struct v4l2_subdev *sd;
			struct v4l2_control csi_ctrl;

			nRet = copy_from_user((void*)&csi_ctrl, (void __user*)ctrl, sizeof(csi_ctrl));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			sd = CdspDev->sd;
			nRet = sd->ops->core->s_ctrl(sd, &csi_ctrl);
			break;
		}
	}

__return:
	return nRet;
}

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
#if C_DMA_A == 1
	gpHalCdspSetYuvBuffA(pCap->width, pCap->height, addr);
	gpHalCdspSetDmaBuff(RD_A_WR_A);
#else
	gpHalCdspSetYuvBuffB(pCap->width, pCap->height, addr);
	gpHalCdspSetDmaBuff(RD_B_WR_B);
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
	
#if C_DMA_A == 1
	gpHalCdspSetYuvBuffA(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
	gpHalCdspSetDmaBuff(RD_A_WR_A);
#else
	gpHalCdspSetYuvBuffB(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
	gpHalCdspSetDmaBuff(RD_B_WR_B);
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
	struct v4l2_input *in;
	struct v4l2_queryctrl *qc;
	struct v4l2_fmtdesc *fmtd;
	struct v4l2_format *fmt;
	struct v4l2_buffer *bf;
	struct v4l2_control *ctrl;
	struct v4l2_streamparm *param;
	struct v4l2_subdev *sd;
	struct v4l2_cropcap *cc;
	struct v4l2_crop *crop;
	struct clk *clock;
	gpCsiMclk_t mclk;
	unsigned char div, idx;
	unsigned short *addr;
	int i, nRet = 0;
	
	if(down_interruptible(&CdspDev->sem) != 0)
		return -ERESTARTSYS;

	switch(cmd)
	{
		case VIDIOC_QUERYCAP:
			nRet = copy_to_user((void __user*)arg, (void*)&g_cdsp_cap, sizeof(g_cdsp_cap));
			if(nRet < 0) { 
				RETURN(-EINVAL);
			}
			break;
		
		case VIDIOC_ENUMINPUT:
		{
			callbackfunc_t	*cb;
			char			*port;
			
			in = (struct v4l2_input*)arg;
			nRet = gp_get_sensorinfo(in->index, (int*)&sd, (int*)&cb, (int*)&port, (int*)&sensor);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			DEBUG("Device[%d]:%s\n", in->index, sd->name);
			in->type = V4L2_INPUT_TYPE_CAMERA;
			strcpy(in->name, sd->name);
			break;
		}
		
		case VIDIOC_S_INPUT:
			if(arg == CdspDev->sdidx) {
				DEBUG("The Same Device\n");
				RETURN(0);
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

			if(CdspDev->cb_func->powerctl != NULL) {
				CdspDev->cb_func->powerctl(1);
			}
			if(CdspDev->cb_func->standby != NULL) {
				CdspDev->cb_func->standby(0); 
			}
			if(CdspDev->cb_func->set_port != NULL) {
				CdspDev->cb_func->set_port(CdspDev->port);
			}

			/* open mclk, some sensor need mclk befor init */
			if(sensor->fmt[0].mclk_src == CSI_CLK_SPLL) {
				clock = clk_get(NULL, "clk_ref_ceva");
				nRet = clk_get_rate(clock);
			} else {
				nRet = USBPHY_CLK;
			}
			
			div = nRet/sensor->fmt[0].mclk;
			if((nRet % sensor->fmt[0].mclk) == 0) { 
				div--;
			}
			gpHalCdspSetMclk(sensor->fmt[0].mclk_src, div, 0, 0);
			DEBUG("mclk = %d\n", nRet/(div + 1));

			sd = CdspDev->sd;
			nRet = sd->ops->core->reset(sd, 0);
			nRet = sd->ops->core->init(sd, 0);
			if(nRet < 0) {
				DEBUG("sensor init fail\n");
				RETURN(-EINVAL);
			}
			break;
		
		case VIDIOC_G_INPUT:
			RETURN(CdspDev->sdidx);
			break;
		
		case VIDIOC_S_FMT:
			fmt = (struct v4l2_format*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			nRet = gp_cdsp_s_fmt(fmt);
			if(nRet < 0) {
				RETURN(nRet);
			}

			sd = CdspDev->sd;
			nRet = sd->ops->video->s_fmt(sd, fmt);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			break;
		
		case VIDIOC_G_FMT:
			fmt = (struct v4l2_format*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->g_fmt(sd, fmt);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			break;
		
		case VIDIOC_TRY_FMT:
			fmt = (struct v4l2_format*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->try_fmt(sd, fmt);
			break;
		
		case VIDIOC_ENUM_FMT:
			fmtd = (struct v4l2_fmtdesc*)arg;	
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->enum_fmt(sd, fmtd);
			break;
		
		case VIDIOC_QUERYCTRL:
			qc = (struct v4l2_queryctrl*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->core->queryctrl(sd, qc);
			if(nRet < 0) {
				RETURN(-EINVAL);
			}		
			break;
		
		case VIDIOC_G_CTRL:
			ctrl = (struct v4l2_control*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			nRet = gp_cdsp_g_ctrl(ctrl);
			break;
		
		case VIDIOC_S_CTRL:
			ctrl = (struct v4l2_control*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = gp_cdsp_s_ctrl(ctrl);
			break;
		
		case VIDIOC_S_INTERFACE:
		{
			struct v4l2_interface Interface;
		
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			nRet = copy_from_user((void*)&Interface, (void __user*)arg, sizeof(Interface));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

			//gpHalCdspFrontSetInterlace(interface->Field, interface->Interlace);	
			gpHalCdspFrontSetInterface((Interface.Interface == CCIR656) ? 1 : 0,
										Interface.HsyncAct ? 0 : 1,
										Interface.VsyncAct ? 0 : 1,	
										CdspDev->SyncFlag);	
			if(Interface.FmtOut != YUVOUT) {
				DERROR("Only Support YUVOUT\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->ext->s_interface(sd, &Interface);
			break;
		}
		
		case VIDIOC_G_INTERFACE:
		{
			unsigned char sync_en;
			struct v4l2_interface Interface;
			
			gpHalCdspFrontGetInterface(&Interface.Interface,
										&Interface.HsyncAct,
										&Interface.VsyncAct,	
										&sync_en);	
			Interface.SampleEdge = MODE_POSITIVE_EDGE;
			Interface.Interlace = MODE_NONE_INTERLACE;
			Interface.Field = FIELD_ODDL;
			CdspDev->SyncFlag = sync_en;
			nRet = copy_to_user((void __user*)arg, (void*)&Interface, sizeof(Interface));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			break;
		}
		
		case VIDIOC_S_MCLK:	
			nRet = copy_from_user((void*)&mclk, (void __user*)arg, sizeof(mclk));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			if(mclk.mclk_out == 0) {
				mclk.mclk_sel = div = 0;
				mclk.pclk_dly = 0;
				mclk.pclk_revb = 0;
				DEBUG("mclk = 0\n");
			} else {
				if(mclk.mclk_sel == CSI_CLK_SPLL) {
					clock = clk_get(NULL, "clk_ref_ceva");
					nRet = clk_get_rate(clock);
				} else {
					nRet = USBPHY_CLK;
				}
				div = nRet / mclk.mclk_out;
				if((nRet % mclk.mclk_out) == 0) {
					div--;
				}
				DEBUG("mclk = %d\n", nRet/(div + 1));
			}
			CdspDev->SyncFlag = mclk.prvi;
			gpHalCdspSetMclk(mclk.mclk_sel, div, mclk.pclk_dly, mclk.pclk_revb);
			break;
		
		case VIDIOC_G_MCLK:
			gpHalCdspGetMclk(&mclk.mclk_sel, &div, &mclk.pclk_dly, &mclk.pclk_revb);
			if(mclk.mclk_sel == CSI_CLK_SPLL) {
				clock = clk_get(NULL, "clk_ref_ceva");
				nRet = clk_get_rate(clock);
			} else {
				nRet = USBPHY_CLK;
			}

			mclk.mclk_out = nRet/(div + 1);
			mclk.prvi = CdspDev->SyncFlag;
			nRet = copy_to_user((void __user*)arg, (void*)&mclk, sizeof(mclk));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			break;
		
		case VIDIOC_CROPCAP:
			cc = (struct v4l2_cropcap*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			sd = CdspDev->sd;
			nRet = sd->ops->video->cropcap(sd, cc);
			break;
		
		case VIDIOC_G_CROP:
			crop = (struct v4l2_crop*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->g_crop(sd, crop);
			break;

		case VIDIOC_S_CROP:
			crop = (struct v4l2_crop*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}

			sd = CdspDev->sd;
			nRet = sd->ops->video->s_crop(sd, crop);
			break;

		case VIDIOC_G_PARM:
			param = (struct v4l2_streamparm*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->g_parm(sd, param);
			break;
		
		case VIDIOC_S_PARM:
			param = (struct v4l2_streamparm*)arg;
			if(CdspDev->sdidx == NO_INPUT) {
				DEBUG("please set input first\n");
				RETURN(-EINVAL);
			}
			
			sd = CdspDev->sd;
			nRet = sd->ops->video->s_parm(sd, param);
			break;
		
		case VIDIOC_REQBUFS:
			nRet = copy_from_user(&(CdspDev->rbuf),(struct v4l2_requestbuffers*)arg, sizeof(struct v4l2_requestbuffers));
			nRet = check_rqbuf_type();
			break;
		
		case VIDIOC_STREAMON:
			if(arg == (unsigned long)NULL) {
				if(CdspDev->in_que[0] == 0xFF) {
					DEBUG("No buffer in Que\n");
					RETURN(-EINVAL);
				}

			#if C_DMA_A == 1
				gpHalCdspSetYuvBuffA(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
				gpHalCdspSetDmaBuff(RD_A_WR_A);
			#else
				gpHalCdspSetYuvBuffB(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->in_que[0]]);
				gpHalCdspSetDmaBuff(RD_B_WR_B);
			#endif

				gpHalCdspClrIntStatus(CDSP_INT_ALL);
				gpHalCdspSetIntEn(ENABLE, CDSP_OVERFOLW|CDSP_EOF);
				gpHalCdspDataSource(CdspDev->imgSrc);
				DEBUG("CdspStart.\n");
			} else {
				DEBUG("cdsp start fail\n");
				RETURN(-EINVAL);
			}
			break;
	
		case VIDIOC_STREAMOFF:
			gpHalCdspSetIntEn(DISABLE, CDSP_INT_ALL);
			gpHalCdspClrIntStatus(CDSP_INT_ALL);
			gpHalCdspDataSource(C_CDSP_SDRAM);
			gpHalCdspRedoTriger(DISABLE);
			if(CdspDev->imgSrc != C_CDSP_SDRAM) {
				gpHalCdspSetMclk(0, 0, 0, 0);
			}

			CdspDev->sd->ops->ext->suspend(CdspDev->sd);
			if(CdspDev->cb_func->standby) {
				CdspDev->cb_func->standby(1);
			}

			if(CdspDev->cb_func->powerctl) {
				CdspDev->cb_func->powerctl(0);
			}

			CdspDev->cdsp_feint_flag= 0;
			CdspDev->CapCnt = 0;
			memset(CdspDev->bfaddr, 0, C_BUFFER_MAX);
			memset(CdspDev->bf, 0, C_BUFFER_MAX);
			memset(CdspDev->in_que, 0xFF, C_BUFFER_MAX);
			memset(CdspDev->out_que, 0xFF, C_BUFFER_MAX);
			DEBUG("CdspStop.\n");
			break;

		case VIDIOC_QBUF:
			bf = (struct v4l2_buffer*)arg;
			if(bf->type != CdspDev->rbuf.type) {
				DEBUG("QBuf Type error\n");
				RETURN(-EINVAL);
			}

			if(bf->index >= CdspDev->rbuf.count ) {
				DEBUG("QBuf index out of bound\n");
				RETURN(-EINVAL);
			}

			for(i=0; i<C_BUFFER_MAX; i++) {
				if(CdspDev->in_que[i] == 0xFF)	{
					CdspDev->in_que[i] = bf->index;
					break;
				}
				
				if(i == (C_BUFFER_MAX - 1)) {
					DEBUG("QBuf index out of bound!\n");
					RETURN(-EINVAL);
				}
			}

			idx = bf->index;
			nRet = copy_from_user(&(CdspDev->bf[idx]), (struct v4l2_buffer*)arg, sizeof(struct v4l2_buffer));
			CdspDev->bf[idx].flags = V4L2_BUF_FLAG_QUEUED;
			addr = (unsigned short *)gp_user_va_to_pa((unsigned short *)bf->m.userptr);
			CdspDev->bfaddr[idx] = (int)addr;
			break;
	
		case VIDIOC_DQBUF:
			bf = (struct v4l2_buffer*)arg;
			if(bf->type != CdspDev->rbuf.type) {
				RETURN(-EINVAL);
			}

			if(CdspDev->out_que[0] == 0xFF) {
				DIAG_ERROR("no buffer ready\n");
				RETURN(-EINVAL);
			}

			nRet = copy_to_user((struct v4l2_buffer*)arg, &(CdspDev->bf[CdspDev->out_que[0]]), sizeof(struct v4l2_buffer));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}

            /* shift the out_queue buffer */
			for(i=0; i<(C_BUFFER_MAX-1); i++) {
				CdspDev->out_que[i] = CdspDev->out_que[i+1];
			}
			CdspDev->out_que[C_BUFFER_MAX-1]=0xFF;
			break;

		case VIDIOC_QUERYSTD:
		{
			v4l2_std_id *std = (v4l2_std_id*)arg;

			if(CdspDev->sdidx == NO_INPUT){
				DIAG_ERROR("please set input first\n");
				RETURN(-EINVAL);	
			}

			sd = CdspDev->sd;
			if(sd->ops->video->querystd) {
				nRet = sd->ops->video->querystd(sd, std);
			} else {
				nRet = ENOIOCTLCMD;
			}
			break;
		}
		
		case VIDIOC_CAPTURE:
		{
			gpCsiCapture_t capture;
			nRet = copy_from_user((void*)&capture, (void __user*)arg, sizeof(capture));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			nRet = gp_cdsp_s_capture(&capture);
			break;
		}
	
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
	unsigned int temphl, temphh, tempvl, tempvh;
	gpCdsp3aResult_t *argp = &CdspDev->result;
	
	gpHalCdspGetAFWinVlaue(1, &temphl, &temphh, &tempvl, &tempvh);
	argp->af1_h_value = temphh;
	argp->af1_h_value <<= 16;
	argp->af1_h_value |= temphl;
	argp->af1_v_value = tempvh;
	argp->af1_v_value <<= 16;
	argp->af1_v_value |= tempvl;

	gpHalCdspGetAFWinVlaue(2, &temphl, &temphh, &tempvl, &tempvh);
	argp->af2_h_value = temphh;
	argp->af2_h_value <<= 16;
	argp->af2_h_value |= temphl;
	argp->af2_v_value = tempvh;
	argp->af2_v_value <<= 16;
	argp->af2_v_value |= tempvl;

	gpHalCdspGetAFWinVlaue(3, &temphl, &temphh, &tempvl, &tempvh);
	argp->af3_h_value = temphh;
	argp->af3_h_value <<= 16;
	argp->af3_h_value |= temphl;
	argp->af3_v_value = tempvh;
	argp->af3_v_value <<= 16;
	argp->af3_v_value |= tempvl;

	return 0;
}

static int
gp_cdsp_handle_awb(
	void *dev_id
)
{
	signed int tempsh, tempsl;
	unsigned int cnt, temph, templ;
	gpCdsp3aResult_t *argp = &CdspDev->result;
	
	gpHalCdspGetAwbSumCnt(1, &cnt);
	argp->sumcnt1 = cnt;
	gpHalCdspGetAwbSumCnt(2, &cnt);
	argp->sumcnt2 = cnt;
	gpHalCdspGetAwbSumCnt(3, &cnt);
	argp->sumcnt3 = cnt;

	gpHalCdspGetAwbSumG(1, &templ, &temph);
	argp->sumg1 = temph;
	argp->sumg1 <<= 16;
	argp->sumg1 |= templ;
	gpHalCdspGetAwbSumG(2, &templ, &temph);
	argp->sumg2 = temph;
	argp->sumg2 <<= 16;
	argp->sumg2 |= templ;
	gpHalCdspGetAwbSumG(3, &templ, &temph);
	argp->sumg3 = temph;
	argp->sumg3 <<= 16;
	argp->sumg3 |= templ;

	gpHalCdspGetAwbSumRG(1, &tempsl, &tempsh);
	argp->sumrg1 = tempsh;
	argp->sumrg1 <<= 16;
	argp->sumrg1 |= tempsl;
	gpHalCdspGetAwbSumRG(2, &tempsl, &tempsh);
	argp->sumrg2 = tempsh;
	argp->sumrg2 <<= 16;
	argp->sumrg2 |= tempsl;
	gpHalCdspGetAwbSumRG(3, &tempsl, &tempsh);
	argp->sumrg3 = tempsh;
	argp->sumrg3 <<= 16;
	argp->sumrg3 |= tempsl;

	gpHalCdspGetAwbSumBG(1, &tempsl, &tempsh);
	argp->sumbg1 = tempsh;
	argp->sumbg1 <<= 16;
	argp->sumbg1 |= tempsl;
	gpHalCdspGetAwbSumBG(2, &tempsl, &tempsh);
	argp->sumbg1 = tempsh;
	argp->sumbg1 <<= 16;
	argp->sumbg1 |= tempsl;
	gpHalCdspGetAwbSumBG(3, &tempsl, &tempsh);
	argp->sumbg1 = tempsh;
	argp->sumbg1 <<= 16;
	argp->sumbg1 |= tempsl;

	return 0;
}

static int
gp_cdsp_handle_ae(
	void *dev_id
)
{
	gpCdsp3aResult_t *argp = &CdspDev->result;
		
	if(gpHalCdspGetAEActBuff()) {
		memcpy(argp->ae_win, (void*)CdspDev->aeAddr[0], 64);
	} else {
		memcpy(argp->ae_win, (void*)CdspDev->aeAddr[1], 64);
	}
	return 0;
}

static int
gp_cdsp_handle_of(
	void *dev_id
)
{
	DERROR("overflow\n");
	return 0;
}

static int
gp_cdsp_handle_facwr(
	void *dev_id
)
{
	DERROR("facwr\n");
	return 0;
}

static int 
gp_cdsp_handle_postprocess(
	void *dev_id
)
{
	if(CdspDev->cdsp_feint_flag == 0) {
		CdspDev->cdsp_feint_flag = 1;
		wake_up_interruptible(&CdspDev->cdsp_wait_queue);
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
	DEBUG("%d\n", CdspDev->CapCnt);
	CdspDev->CapCnt--;
	if(CdspDev->CapCnt == 0) {
		if(CdspDev->cdsp_feint_flag == 0) {
			CdspDev->cdsp_feint_flag = 1;
			wake_up_interruptible(&CdspDev->cdsp_wait_queue);
		} else {
			CdspDev->CapCnt++;
		}
	}
	return 0;
}

static int
gp_cdsp_handle_eof(
	void *dev_id

)
{
	int i;
	
	/* find empty frame */
	CdspDev->bfidx = CdspDev->in_que[1];
	if(CdspDev->bfidx == 0xFF) {
		return 0; 
	}
	
	/* get/set empty buffer to h/w */
#if C_DMA_A == 1
	gpHalCdspSetYuvBuffA(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->bfidx]);
	gpHalCdspSetDmaBuff(RD_A_WR_A);
#else
	gpHalCdspSetYuvBuffB(CdspDev->imgRbWidth, CdspDev->imgRbHeight, CdspDev->bfaddr[CdspDev->bfidx]);
	gpHalCdspSetDmaBuff(RD_B_WR_B);
#endif

	/* get/set ready buffer */
	CdspDev->bfidx = CdspDev->in_que[0];
	CdspDev->bf[CdspDev->bfidx].flags = V4L2_BUF_FLAG_DONE;
	
	/* shift the in_que buffer */
	for(i=0; i<(C_BUFFER_MAX-1); i++) {
		CdspDev->in_que[i] = CdspDev->in_que[i+1];
	}
	
	CdspDev->in_que[C_BUFFER_MAX-1] = 0xFF;
	
	/* push the ready index into out_que buffer */
	for(i=0; i<C_BUFFER_MAX; i++) {
		if(CdspDev->out_que[i] == 0xFF) {
			CdspDev->out_que[i] = CdspDev->bfidx;
			break;
		}
	}
	
	if(i == C_BUFFER_MAX) {
		return 0;
	}
	
	if(CdspDev->cdsp_feint_flag == 0) {
		CdspDev->cdsp_feint_flag = 1;
		wake_up_interruptible(&CdspDev->cdsp_wait_queue);
	}
	
	return 0;
}


static irqreturn_t
gp_cdsp_irq_handler(
	int irq,
	void *dev_id
)
{
	int status, ret = -1;;
 
	status = gpHalCdspGetGlbIntStatus();
	if(status & CDSP_INT_BIT) {
		
		status = gpHalCdspGetIntStatus();
		if(status & CDSP_EOF) { 
			if(CdspDev->imgSrc == C_CDSP_SDRAM) {
				ret = gp_cdsp_handle_postprocess(dev_id);
			} else {
			#if CDSP_CHK_MIPI_EN == 1
				if((strcmp("MIPI", CdspDev->port) == 0) && 
					(gp_mipi_get_curframe_status(0) == 0)){
					DEBUG("MIPIFrameFail\n");
					ret = 0;
			#else 
				if(0) {
			#endif
				} else {
					if(CdspDev->CapCnt > 0) {
						ret = gp_cdsp_handle_capture(dev_id);
					} else {
						ret = gp_cdsp_handle_eof(dev_id);
						/* histgm */
						gpHalCdspGetHistgmCount(&CdspDev->result.hislowcnt, &CdspDev->result.hishicnt);
					}
				}
			}
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
		status = gpHalCdspGetFrontVdIntStatus();
		ret = 0;
	} else if(status & FRONT_INT_BIT) {
		status = gpHalCdspGetFrontIntStatus();
		ret = 0;
	} 

	if(ret >= 0) {
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static unsigned int
gp_cdsp_poll(
	struct file *filp,
	struct poll_table_struct *poll
)
{
	unsigned int mask = 0;

#if 0
	wait_event_interruptible(CdspDev->cdsp_wait_queue, (CdspDev->cdsp_feint_flag != 0));
#else
	poll_wait(filp, &CdspDev->cdsp_wait_queue, poll);
#endif

	if(CdspDev->cdsp_feint_flag == 1) {
		CdspDev->cdsp_feint_flag = 0;
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
	if(CdspDev->OpenCnt == 0) {
		gpCdspClockEnable(1);
		gpHalCdspSetModuleReset(1);

		gpHalCdspReset();
		gpHalCdspFrontReset();
		gpHalCdspDataSource(C_CDSP_SDRAM);

		gpHalCdspSetIntEn(DISABLE, CDSP_INT_ALL);
		gpHalCdspClrIntStatus(CDSP_INT_ALL);

		CdspDev->OpenCnt = 1;
		DEBUG(KERN_WARNING "CdspOpen.\n");
	} else {
		DERROR(KERN_WARNING "CdspOpenFail!\n");
		return -1;
	}
	
	return 0;
}

static int
gp_cdsp_release(
	struct inode *inode,
	struct file *filp
)
{	
	if(CdspDev->OpenCnt == 1) {
		gpHalCdspSetIntEn(DISABLE, CDSP_INT_ALL);	
		gpHalCdspClrIntStatus(CDSP_INT_ALL);	
		gpHalCdspDataSource(C_CDSP_SDRAM);	
		gpHalCdspRedoTriger(DISABLE);
		gpCdspClockEnable(0);

		if(CdspDev->imgSrc != C_CDSP_SDRAM) {
			gpHalCdspSetMclk(0, 0, 0, 0);
		}
		
		CdspDev->cdsp_feint_flag = 0;
		CdspDev->OpenCnt = 0;
		CdspDev->CapCnt = 0;
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
		DEBUG(KERN_WARNING "CdspClose.\n");
	} else {
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
		gpCdspClockEnable(0);
		if(CdspDev->cb_func->standby) {
			CdspDev->cb_func->standby(1);
		}
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
		gpHalCdspFrontReset();
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

	CdspDev->sdidx = NO_INPUT;
	CdspDev->suppression.suppressen = ENABLE;
	memset(CdspDev->in_que, 0xFF, C_BUFFER_MAX);
	memset(CdspDev->out_que, 0xFF, C_BUFFER_MAX);
	
	/* register irq */
	nRet = request_irq(IRQ_AC97,
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
		free_irq(IRQ_AC97, CdspDev);

		/* free char device */
		misc_deregister(&CdspDev->dev);

		/* platform unregister */
		platform_device_unregister(&gp_cdsp_device);
		platform_driver_unregister(&gp_cdsp_driver);

		/* free memory */
		kfree(CdspDev);
		gp_chunk_free((void *)CdspDev->aeAddr[0]);
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
	free_irq(IRQ_AC97, CdspDev);

	/* free char device */
	misc_deregister(&CdspDev->dev);

	/* platform unregister */
	platform_device_unregister(&gp_cdsp_device);
	platform_driver_unregister(&gp_cdsp_driver);

	/* free memory */
	kfree(CdspDev);
	gp_chunk_free((void *)CdspDev->aeAddr[0]);
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



