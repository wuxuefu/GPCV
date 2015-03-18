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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30077, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file display.c
 * @brief Display interface
 * @author Anson Chuang
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/hal/hal_disp.h>
#include <mach/gp_board.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_display.h>
#include <mach/gp_panel.h>
#include <linux/clk.h>
#include <mach/hal/sysregs.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/hal/hal_hdmi.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define MAX_DISP_DEVICE_NUM	1

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define ERROR(fmt, arg...) printk(KERN_ERR "[%s:%d] Error! "fmt, __FUNCTION__, __LINE__, ##arg)
#define WARNING(fmt, arg...) printk(KERN_WARNING "[%s:%d] Warning! "fmt, __FUNCTION__, __LINE__, ##arg)
#define MSG(fmt, arg...) printk(KERN_DEBUG "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)
#define INFO(fmt, arg...) printk(KERN_INFO "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)

#define RETURN(x)		{ret = x; goto Return;}
#define CHECK_(x, msg, errid) if(!(x)) {ERROR("%s, %s\n", msg, #x); RETURN(errid);}
#define CHECK_PRG(x)	CHECK_(x, "Program Error", -EIO)
#define CHECK_VAL(x)	CHECK_(x, "Value Error", -1)

#define DISPNUM(WorkMem)	((WorkMem == gDispWorkMem) ? 0 : 1)

typedef enum {
	DISP_STATE_CLOSE = 0,
	DISP_STATE_OPEN,
	DISP_STATE_PRE_CLOSE,
} DISP_STATE;

#define DISPLAY_DRIVER_NAME "gp-display"

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct {
	struct list_head list;
	const gp_disp_drv_op *DrvOp;
} gp_disp_drv_t;

typedef struct {
	struct platform_device plat_dev;
	struct miscdevice misc;
	struct list_head list;
	struct clk *hClk;

	unsigned long ClkRate;
	int flag_is_clk_enable;

	DISP_STATE state;

	// RegisterSet
	void *hDev;
	void *RegBase;

	// ACKs
	int DispOffAck;
	int UpdateAck;
	// used only in Suspend
	int DispOffAck_Suspend;
	int UpdateAck_Suspend;

	// Panel
	const gp_disp_drv_op *DrvOp;
	gp_size_t panelSize;
	gp_disp_pixelsize_t pixelSize;
	
	// Current Output Device Type & Mode
	int curOutputType;
	int DispMode;

	// Primary
	int priEnable;
	gp_bitmap_t priBitmap;
	gp_disp_scale_t priScaleInfo;

	// Osd
	int osdEnable[HAL_DISP_OSD_MAX];
	gp_bitmap_t osdBitmap[HAL_DISP_OSD_MAX];
	gp_disp_scale_t osdScaleInfo[HAL_DISP_OSD_MAX];
	gp_disp_osdpalette_t osdPalette[HAL_DISP_OSD_MAX];
	int osdPaletteOffset[HAL_DISP_OSD_MAX];
	gp_disp_osdalpha_t osdAlpha[HAL_DISP_OSD_MAX];
	int osdColorKey[HAL_DISP_OSD_MAX];

	// Gamma
	uint8_t gammaTable[3][1024];
    int gammaTableInit;
    int gammaEnable;

	// Register Backup
	unsigned long BackupReg[0x400 / 4];

	unsigned long ic_REG258;
	unsigned long ic_REG278;
	unsigned long ic_REG298;
	unsigned long ic_REG324;

	// back-door
	char file_buf[256];

} GP_DISP_WORKMEM;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int disp_fops_open(struct inode *inode, struct file *pfile);
static int disp_fops_release(struct inode *inode, struct file *pfile);
static long disp_fops_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg);
static int disp_get_osd_fmt(int srcType, int *format, int *bpp);
static ssize_t disp_fops_write(struct file *, const char __user *, size_t, loff_t *);

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
// Event Q
static DECLARE_WAIT_QUEUE_HEAD(gDispWaitQ);

// Work memroy and its list
static LIST_HEAD(gDispWorkMemList);
static GP_DISP_WORKMEM gDispWorkMem[MAX_DISP_DEVICE_NUM];
static int g_disp_init = 0;
// Structure that declares the usual file
// access functions
static const struct file_operations disp_fops =
{
	.owner			= THIS_MODULE,
	.open			= disp_fops_open,
	.release		= disp_fops_release,
	.unlocked_ioctl	= disp_fops_ioctl,
	.write			= disp_fops_write,
};

// i-node
static const char *DispDevNodeName[MAX_DISP_DEVICE_NUM]	= {"disp0"};
static const char *DispClockName[MAX_DISP_DEVICE_NUM]	= {"DISP0_PCLK"};

// registration
static struct semaphore disp_registration_lock;
static struct semaphore disp_sema;
static LIST_HEAD(gDispOutputDevList);

// IRQ
static const int IrqID[MAX_DISP_DEVICE_NUM] = {IRQ_DISPLAY};
static const char *IrqName[MAX_DISP_DEVICE_NUM] = {"DISPLAY_CTRL"};

static char *pll_sel = "PLL0";
static int pllsel = 0;

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void disp_dump_reg(void *hDisp, char* msg)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	unsigned int* addr = (unsigned int *)0xfc800000;
	int i;

	MSG("Enter\n");
	disp_wait_for_updated(WorkMem);
	mdelay(2000);

	if (msg)
		printk("%s\n", msg);

	for (i=0; i<0xff; i+= 16) {
		printk("0x%08x: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", (UINT32)addr, *(addr), *(addr+1), *(addr+2), *(addr+3));
		addr += 4;
	}
	printk("\n");
}

void disp_update(void *hDisp)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	unsigned long tmp;
	MSG("Enter\n");
	local_irq_save(tmp);
	WorkMem->UpdateAck = 0;
	gpHalDispUpdateParameter(WorkMem->RegBase);
	local_irq_restore(tmp);
}
EXPORT_SYMBOL(disp_update);

static int disp_set_pri_enable(void *hDisp, int enable)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	MSG("Enter\n");
	WorkMem->priEnable = enable ? 1 : 0;

	if (enable) {
		/* Disable color bar */
		gpHalDispSetColorBarEnable(WorkMem->RegBase, 0);
	}
	else {
		/* Enable color bar */
		gpHalDispBlankInfo_t blank;

		blank.top = 0;
		blank.bottom = 0;
		blank.left = 0;
		blank.right = 0;
		blank.pattern = 0x000000;
		gpHalDispSetPriBlank(WorkMem->RegBase, &blank);
		gpHalDispSetPriRes(WorkMem->RegBase, WorkMem->panelSize.width, WorkMem->panelSize.height);
		gpHalDispSetPriInputInfo(WorkMem->RegBase, HAL_DISP_INPUT_FMT_RGB, HAL_DISP_INPUT_TYPE_RGB565);
		gpHalDispSetPriSclEnable(WorkMem->RegBase, 0, 0);
		gpHalDispSetColorBar(WorkMem->RegBase, 0, 0xff, 0x00);
		gpHalDispSetColorBarEnable(WorkMem->RegBase, 1);
	}
Return:
	return ret;
}

static int disp_get_pri_enable(void *hDisp)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	ret = WorkMem->priEnable;
Return:
	return ret;
}

static int disp_get_pri_fmt_type(int srcType, int *format, int *type, int *bpp)
{
	*format = 0;
	*type = 0;
	*bpp = 0;
	MSG("Enter\n");
	switch (srcType)
	{
	case SP_BITMAP_ARGB8888:	*format = HAL_DISP_INPUT_FMT_RGB;		*type = HAL_DISP_INPUT_TYPE_ARGB8888;	*bpp = 32;	break;
	case SP_BITMAP_RGB888:		*format = HAL_DISP_INPUT_FMT_RGB;		*type = HAL_DISP_INPUT_TYPE_RGB888;		*bpp = 24;	break;
	case SP_BITMAP_RGB565:		*format = HAL_DISP_INPUT_FMT_RGB;		*type = HAL_DISP_INPUT_TYPE_RGB565;		*bpp = 16;	break;
	case SP_BITMAP_RGB555:		*format = HAL_DISP_INPUT_FMT_RGB;		*type = HAL_DISP_INPUT_TYPE_RGB555;		*bpp = 16;	break;
	case SP_BITMAP_YCbYCr:
	case SP_BITMAP_YCbCr422:	*format = HAL_DISP_INPUT_FMT_YCbCr;		*type = HAL_DISP_INPUT_TYPE_YCbYCr422;	*bpp = 16;	break;
	case SP_BITMAP_YCbCr:
	case SP_BITMAP_YCbCr444:	*format = HAL_DISP_INPUT_FMT_YCbCr;		*type = HAL_DISP_INPUT_TYPE_YCbYCr444;	*bpp = 24;	break;
	case SP_BITMAP_SEMI400:		*format = HAL_DISP_INPUT_FMT_SEMI;		*type = HAL_DISP_INPUT_TYPE_SEMI400;	*bpp = 8;	break;
   	case SP_BITMAP_SEMI420:		*format = HAL_DISP_INPUT_FMT_SEMI;		*type = HAL_DISP_INPUT_TYPE_SEMI420;	*bpp = 8;	break;
	case SP_BITMAP_SEMI422:		*format = HAL_DISP_INPUT_FMT_SEMI;		*type = HAL_DISP_INPUT_TYPE_SEMI422;	*bpp = 8;	break;
	case SP_BITMAP_SEMI444:		*format = HAL_DISP_INPUT_FMT_SEMI;		*type = HAL_DISP_INPUT_TYPE_SEMI444;	*bpp = 8;	break;
	default:
		ERROR("Unsupported type %d\n", srcType);
		return -1;
	}
	return 0;	/* SP_OK */
}

static void __disp_set_pri_scale(GP_DISP_WORKMEM *WorkMem)
{
	gpHalDispBlankInfo_t blankInfo;
	int hScale = 0, vScale = 0;
	gpHalDispSclInfo_t scale;
	const gp_bitmap_t *pbitmap;
	gp_disp_scale_t *pscale;

	pbitmap = &WorkMem->priBitmap;
	pscale = &WorkMem->priScaleInfo;

	MSG("x=%d, y=%d, width=%d, height=%d, blankcolor=%d\n", pscale->x, pscale->y, pscale->width, pscale->height, pscale->blankcolor);
	MSG("panelSize.width =%d, panelSize.height =%d\n",		WorkMem->panelSize.width, WorkMem->panelSize.height);
	MSG("priBitmap.width =%d, priBitmap.height =%d\n",		pbitmap->width, pbitmap->height);

	// Set blank according to scale info
	blankInfo.top		= pscale->y;
	blankInfo.bottom	= WorkMem->panelSize.height - pscale->height - pscale->y;
	blankInfo.left		= pscale->x;
	blankInfo.right		= WorkMem->panelSize.width - pscale->width - pscale->x;
	blankInfo.pattern	= pscale->blankcolor;
	gpHalDispSetPriBlank(WorkMem->RegBase, &blankInfo);
	if (pscale->width != pbitmap->width || pscale->height != pbitmap->height) {
		// Scale enable
		scale.srcWidth	= pbitmap->width;
		scale.srcHeight	= pbitmap->height;
		scale.dstWidth	= pscale->width;
		scale.dstHeight	= pscale->height;
		scale.hInit	= 0;
		scale.vInit0 = 0;
		scale.vInit1 = 0;
		hScale = pscale->width  != pbitmap->width  ? 1 : 0;
		vScale = pscale->height != pbitmap->height ? 1 : 0;

		if (pscale->height < pbitmap->height) {
			MSG("It doesn't support vertical scaling down. [%d][%d]\n", pscale->height, pbitmap->height);
			if ((pscale->y + pscale->height) > WorkMem->panelSize.height) {
				scale.srcHeight = WorkMem->panelSize.height - pscale->y;

				blankInfo.bottom = 0;
				gpHalDispSetPriBlank(WorkMem->RegBase, &blankInfo);
			}
			else {
				scale.srcHeight = pscale->height;
			}
			scale.dstHeight = scale.srcHeight;
			vScale = 0;

			gpHalDispSetPriRes(WorkMem->RegBase, pbitmap->width, scale.srcHeight);
		}
	}

	if (hScale || vScale) {
		gpHalDispSetPriSclInfo(WorkMem->RegBase, scale);
		gpHalDispSetPriSclEnable(WorkMem->RegBase, hScale, vScale);
	}
	else {
		gpHalDispSetPriSclEnable(WorkMem->RegBase, 0, 0);
	}
}

int disp_set_pri_scale(void *hDisp, const gp_disp_scale_t *_pscale)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;

	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	memcpy(&WorkMem->priScaleInfo, _pscale, sizeof(gp_disp_scale_t));

	__disp_set_pri_scale(WorkMem);

Return:
	return ret;
}
EXPORT_SYMBOL(disp_set_pri_scale);

static void __disp_set_pri_bitmap(GP_DISP_WORKMEM *WorkMem)
{
	int format, type, bpp;
	gp_bitmap_t *pbitmap;

	MSG("Enter\n");
    pbitmap = &WorkMem->priBitmap;
    disp_get_pri_fmt_type(pbitmap->type, &format, &type, &bpp);

	gpHalDispSetPriRes(WorkMem->RegBase, pbitmap->width,pbitmap->height); // Set Resolution
	gpHalDispSetPriFrameAddr(WorkMem->RegBase, pbitmap->pData); // Set Frame Address
	gpHalDispSetPriUVAddr(WorkMem->RegBase, pbitmap->pDataU);   // Set Semi UV Address
	gpHalDispSetPriInputInfo(WorkMem->RegBase, format, type);	// Set format (RGB/YCbCr/YCbCr Semi-Planar) & type
	gpHalDispSetPriPitch(WorkMem->RegBase, pbitmap->bpl, pbitmap->width * (bpp / 8));	// Set pitch (src & active)
	gpHalDispSetPriBurstNum(WorkMem->RegBase, pbitmap->width * (bpp / 8) / 32 + 1);

	__disp_set_pri_scale(WorkMem);
}

int disp_set_pri_bitmap(void *hDisp, const gp_bitmap_t *_pbitmap)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	gp_bitmap_t *pbitmap;

	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	pbitmap = &WorkMem->priBitmap;
	memcpy(pbitmap, _pbitmap, sizeof(gp_bitmap_t));

	MSG("panelSize.width =%d, panelSize.height =%d\n", WorkMem->panelSize.width, WorkMem->panelSize.height);
	MSG("priBitmap.width =%d, priBitmap.height =%d\n", pbitmap->width, pbitmap->height);

	__disp_set_pri_bitmap(WorkMem);
Return:
	return ret;
}
EXPORT_SYMBOL(disp_set_pri_bitmap);

int disp_wait_for_updated(void *hDisp)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret;
	MSG("Enter\n");

	ret = wait_event_interruptible_timeout(gDispWaitQ, WorkMem->UpdateAck != 0, HZ / 20);
	if(ret <= 0) {
		WARNING("wait timeout\n");
	}

	return ret;
}
EXPORT_SYMBOL(disp_wait_for_updated);

int disp_set_pri_frame_addr(void *hDisp, const void *addr, const void *addrUV)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	gp_bitmap_t *pbitmap;

	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	pbitmap = &WorkMem->priBitmap;
	pbitmap->pData = (uint8_t*)addr;
	pbitmap->pDataU = (uint8_t*)addrUV;

	gpHalDispSetPriFrameAddr(WorkMem->RegBase, pbitmap->pData); // Set Frame Address
	gpHalDispSetPriUVAddr(WorkMem->RegBase, pbitmap->pDataU);   // Set Semi UV Address
Return:
	return ret;
}
EXPORT_SYMBOL(disp_set_pri_frame_addr);

int disp_set_osd_enable(void *hDisp, int layerNum, int enable)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	WorkMem->osdEnable[layerNum] = enable ? 1 : 0;
	gpHalDispSetOsdEnable(WorkMem->RegBase, layerNum, enable);
Return:
	return ret;
}
EXPORT_SYMBOL(disp_set_osd_enable);

int disp_get_osd_enable(void *hDisp, int layerNum)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	MSG("Enter\n");
	return WorkMem->osdEnable[layerNum];
}
EXPORT_SYMBOL(disp_get_osd_enable);

static int disp_get_osd_fmt(int srcType, int *format, int *bpp)
{
	MSG("Enter\n");
	*format = 0;
	*bpp = 0;
	switch (srcType)
	{
	case SP_BITMAP_RGB565:		*format = HAL_DISP_OSD_FMT_RGB565;		*bpp = 16;		break;
	case SP_BITMAP_RGAB5515:	*format = HAL_DISP_OSD_FMT_RGB5515;		*bpp = 16;		break;
	case SP_BITMAP_ARGB1555:	*format = HAL_DISP_OSD_FMT_RGB1555;		*bpp = 16;		break;
	case SP_BITMAP_ARGB8888:	*format = HAL_DISP_OSD_FMT_ARGB8888;	*bpp = 32;		break;
	case SP_BITMAP_ABGR8888:	*format = HAL_DISP_OSD_FMT_ABGR8888;	*bpp = 32;		break;
	case SP_BITMAP_RGBA8888:	*format = HAL_DISP_OSD_FMT_RGBA8888;	*bpp = 32;		break;
	case SP_BITMAP_BGRA8888:	*format = HAL_DISP_OSD_FMT_BGRA8888;	*bpp = 32;		break;
	case SP_BITMAP_1BPP:		*format = HAL_DISP_OSD_FMT_ALPHA;		*bpp = 1;		break;
	case SP_BITMAP_4BPP:		*format = HAL_DISP_OSD_FMT_ALPHA;		*bpp = 4;		break;
	case SP_BITMAP_8BPP:		*format = HAL_DISP_OSD_FMT_ALPHA;		*bpp = 8;		break;
	default:
		ERROR("Unsupported type %d\n", srcType);
		return -1;
	}
	return 0;
}

static void __disp_set_osd_scale(GP_DISP_WORKMEM *WorkMem, int layerNum)
{
	int hScale = 0, vScale = 0;
	gpHalDispSclInfo_t scale;
	const gp_bitmap_t *pbitmap;
	gp_disp_scale_t *pscale;

	pbitmap = &WorkMem->osdBitmap[layerNum];
	pscale = &WorkMem->osdScaleInfo[layerNum];

	gpHalDispSetOsdXY(WorkMem->RegBase, layerNum, pscale->x, pscale->y);
	if (pscale->width != pbitmap->width || pscale->height != pbitmap->height) {
		scale.srcWidth = pbitmap->width;
		scale.srcHeight = pbitmap->height;
		scale.dstWidth = pscale->width;
		scale.dstHeight = pscale->height;
		scale.hInit = 0;
		scale.vInit0 = 0;
		scale.vInit1 = 0;
		hScale = pscale->width  != pbitmap->width  ? 1 : 0;
		vScale = pscale->height != pbitmap->height ? 1 : 0;

		if (layerNum == 0 && pscale->height < pbitmap->height) {
			MSG("Osd0 doesn't support vertical scaling down. [%d][%d]\n", pscale->height, pbitmap->height);
			if ((pscale->y + pscale->height) > WorkMem->panelSize.height) {
				scale.srcHeight = WorkMem->panelSize.height - pscale->y;
			}
			else {
				scale.srcHeight = pscale->height;
			}
			scale.dstHeight = scale.srcHeight;
			vScale = 0;

			gpHalDispSetOsdRes(WorkMem->RegBase, layerNum, pbitmap->width, scale.srcHeight);
		}

		gpHalDispSetOsdSclInfo(WorkMem->RegBase, layerNum, scale);
		gpHalDispSetOsdSclEnable(WorkMem->RegBase, layerNum, hScale, vScale);
	}
	else {
		gpHalDispSetOsdSclEnable(WorkMem->RegBase, layerNum, 0, 0);
	}
}

int disp_set_osd_scale(void *hDisp, int layerNum, const gp_disp_scale_t *pscale)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;

	MSG("Enter\n");

	memcpy(&WorkMem->osdScaleInfo[layerNum], pscale, sizeof(gp_disp_scale_t));
	MSG("disp%d osd%d, x=%d, y=%d, width=%d, height=%d, blankcolor=0x%x\n",
		DISPNUM(WorkMem), layerNum, pscale->x, pscale->y, pscale->width, pscale->height, pscale->blankcolor);

	__disp_set_osd_scale(WorkMem, layerNum);

	return ret;
}
EXPORT_SYMBOL(disp_set_osd_scale);

static void __disp_set_osd_bitmap(GP_DISP_WORKMEM *WorkMem, int layerNum)
{
	int format, bpp;
	gp_bitmap_t *pbitmap;
	const gp_disp_scale_t *osdScale;

	MSG("Enter\n");
	pbitmap = &WorkMem->osdBitmap[layerNum];
	osdScale = &WorkMem->osdScaleInfo[layerNum];
	disp_get_osd_fmt(pbitmap->type, &format, &bpp);

	gpHalDispSetOsdRes(WorkMem->RegBase, layerNum, pbitmap->width,pbitmap->height);	// Set Resolution
	gpHalDispSetOsdFrameAddr(WorkMem->RegBase, layerNum, pbitmap->pData); // Set Frame Address
	gpHalDispSetOsdInputFmt(WorkMem->RegBase, layerNum, format);
	gpHalDispSetOsdPitch(WorkMem->RegBase, layerNum, pbitmap->bpl, pbitmap->width * bpp / 8);	// Set pitch (src & active)
	gpHalDispSetOsdBurstNum(WorkMem->RegBase, layerNum, pbitmap->width * bpp / 8 / 32 + 1);

	__disp_set_osd_scale(WorkMem, layerNum);
}

int disp_set_osd_bitmap(void *hDisp, int layerNum, const gp_bitmap_t *_pbitmap)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	gp_bitmap_t *pbitmap;
	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	pbitmap = &WorkMem->osdBitmap[layerNum];
	memcpy(pbitmap, _pbitmap, sizeof(gp_bitmap_t));
	__disp_set_osd_bitmap(WorkMem, layerNum);

Return:
	return ret;
}
EXPORT_SYMBOL(disp_set_osd_bitmap);

int disp_set_mode(void *hDisp, int mode)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;

	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->DrvOp != 0);
	CHECK_PRG(WorkMem->hDev != 0);
	CHECK_PRG(WorkMem->RegBase != 0);
	CHECK_PRG((WorkMem->DispMode = WorkMem->DrvOp->SetTiming(WorkMem->hDev, WorkMem->hClk, mode)) >= 0);
	MSG("disp%d mode = 0x%x\n", DISPNUM(WorkMem), WorkMem->DispMode);
	gpHalDispGetRes(WorkMem->RegBase, (unsigned short*)&WorkMem->panelSize.width, (unsigned short*)&WorkMem->panelSize.height);

Return:
	return ret;
}
EXPORT_SYMBOL(disp_set_mode);

int disp_get_mode(void *hDisp)
{
	int ret = 0;
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);

	MSG("disp%d\n", DISPNUM(WorkMem));
	ret = WorkMem->DispMode;
Return:
	return ret;
}
EXPORT_SYMBOL(disp_get_mode);

int disp_set_osd_alpha(void *hDisp, int layerNum, const gp_disp_osdalpha_t *palpha)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;

	MSG("Enter\n");
	memcpy(&WorkMem->osdAlpha[layerNum], palpha, sizeof(gp_disp_osdalpha_t));
	gpHalDispSetOsdAlpha(WorkMem->RegBase, layerNum, palpha->consta, palpha->ppamd, palpha->alpha, palpha->alphasel);

	return ret;
}
EXPORT_SYMBOL(disp_set_osd_alpha);

int disp_set_osd_frame_addr(void *hDisp, int layerNum, const void *addr)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	gp_bitmap_t *pbitmap;

	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	pbitmap = &WorkMem->osdBitmap[layerNum];
	pbitmap->pData = (uint8_t*)addr;

	gpHalDispSetOsdFrameAddr(WorkMem->RegBase, layerNum, pbitmap->pData); // Set Frame Address
Return:
	return ret;
}
EXPORT_SYMBOL(disp_set_osd_frame_addr);

static int disp_init(void *hDisp, int timing)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int i, j;
	int ret = 0;
	unsigned long tmp;
	gp_bitmap_t *pbitmap;
	int intFlag;

	MSG("\n");

	clk_enable(WorkMem->hClk);
	WorkMem->flag_is_clk_enable = 1;

	CHECK_PRG((WorkMem->hDev = WorkMem->DrvOp->Open(WorkMem->RegBase)) != 0);

	pbitmap = &WorkMem->osdBitmap[0];

	CHECK_PRG(disp_set_mode(WorkMem, timing) >= 0);

	WorkMem->priScaleInfo.x = 0;
	WorkMem->priScaleInfo.y = 0;
	WorkMem->priScaleInfo.width = WorkMem->panelSize.width;
	WorkMem->priScaleInfo.height = WorkMem->panelSize.height;
	WorkMem->priScaleInfo.blankcolor = 0;

	pbitmap->width = WorkMem->panelSize.width;
	pbitmap->height = WorkMem->panelSize.height;

	for (i=0; i<HAL_DISP_OSD_MAX; i++) {
		WorkMem->osdScaleInfo[i].x		= 0;
		WorkMem->osdScaleInfo[i].y		= 0;
		WorkMem->osdScaleInfo[i].width	= WorkMem->panelSize.width;
		WorkMem->osdScaleInfo[i].height	= WorkMem->panelSize.height;
	}

	gpHalDispSetPriBurstNum(WorkMem->RegBase, 0x20);
	gpHalDispSetOsdBurstNum(WorkMem->RegBase, 0, 0x20);
	gpHalDispSetOsdBurstNum(WorkMem->RegBase, 1, 0x22);

	// Turn on color bar to ban primary layer output
	disp_set_pri_enable(WorkMem, 0);

	// Disable OSD layer
	for(i=0; i<HAL_DISP_OSD_MAX; i++) {
		disp_set_osd_enable(WorkMem, i, 0);
	}
	if(WorkMem->DrvOp->OnEnable)
		CHECK_PRG(WorkMem->DrvOp->OnEnable(WorkMem->hDev) >= 0);
#if 1
	// interrupt
	intFlag = HAL_DISP_INT_FRAME_END | HAL_DISP_INT_DISPLAY_OFF | HAL_DISP_INT_UPDATE_FAIL;
	if(WorkMem->DrvOp->OnInterrupt) intFlag |= WorkMem->DrvOp->InterruptMask;
	gpHalDispSetIntEnable(WorkMem->RegBase, intFlag);
#endif
	// initial gamma table
	for(i=0;i<3;i++) {
		unsigned long *dst = (unsigned long*)WorkMem->gammaTable[i];

        if(WorkMem->gammaTableInit != 1) { // gammaTableInit 0: gamma table not been set; 1: gamma table set already
    		for(j=0;j<256;j++)
    			*dst++ = (j << 24) | (j << 16) | (j << 8) | j;
        }
		gpHalDispSetGammaTable(WorkMem->RegBase, i, WorkMem->gammaTable[i]);
	}

    // Record VDec default off
    WorkMem->ic_REG324 = 0x3;

    gpHalDispSetGammaEnable(WorkMem->RegBase, WorkMem->gammaEnable);

	local_irq_save(tmp);
	WorkMem->state = DISP_STATE_OPEN;
	gpHalDispUpdateParameter(WorkMem->RegBase);
	local_irq_restore(tmp);

Return:
	if(ret < 0) {
		if(WorkMem->flag_is_clk_enable) {
			if(WorkMem->hDev) {
				WorkMem->DrvOp->Close(WorkMem->hDev);
				WorkMem->hDev = 0;
			}
			WorkMem->flag_is_clk_enable = 0;
			clk_disable(WorkMem->hClk);
		}
	}
	return ret;
}

static int disp_get_panel_pixelsize(void *hDisp, gp_disp_pixelsize_t *size)
{
	int ret = 0;
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;

	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->DrvOp != 0);
	CHECK_PRG(WorkMem->hDev != 0);
	CHECK_PRG(WorkMem->RegBase != 0);
	CHECK_PRG(WorkMem->DrvOp->GetPixelSize != 0);
	
	WorkMem->DrvOp->GetPixelSize(WorkMem->hDev, size);

Return:
	return ret;
}

void disp_get_panel_res(void *hDisp, gp_size_t *res)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	MSG("disp%d (%d x %d)\n", DISPNUM(WorkMem), WorkMem->panelSize.width, WorkMem->panelSize.height);
	res->width = WorkMem->panelSize.width;
	res->height = WorkMem->panelSize.height;
}
EXPORT_SYMBOL(disp_get_panel_res);

int disp_set_dither_enable(void *hDisp, int enable)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("enable = %d\n", enable);

 	gpHalDispSetDitherEnable(WorkMem->RegBase, enable);

	return ret;
}
EXPORT_SYMBOL(disp_set_dither_enable);

int disp_set_dither_type(void *hDisp, int type)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

 	gpHalDispSetDitherType(WorkMem->RegBase, type);

	return ret;
}
EXPORT_SYMBOL(disp_set_dither_type);

int disp_set_flip_function(void *hDisp, int value)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	/* Primary layer */
	if (value & 0x1) {
		gpHalDispSetPriFlip(WorkMem->RegBase, SP_DISP_FLIP_V);
	}
	else {
		gpHalDispSetPriFlip(WorkMem->RegBase, SP_DISP_FLIP_NONE);
	}

	/* Osd layer */
	if (value & 0x0A) {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 0, SP_DISP_FLIP_V);
	}
	else {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 0, SP_DISP_FLIP_NONE);
	}
	if (value & 0x14) {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 1, SP_DISP_FLIP_V);
	}
	else {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 1, SP_DISP_FLIP_NONE);
	}


	return ret;
}
EXPORT_SYMBOL(disp_set_flip_function);

int disp_primary_set_flip_function(void *hDisp, int value)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	if (value & 0x1) {
		gpHalDispSetPriFlip(WorkMem->RegBase, SP_DISP_FLIP_V);
	}
	else {
		gpHalDispSetPriFlip(WorkMem->RegBase, SP_DISP_FLIP_NONE);
	}

	return ret;
}
EXPORT_SYMBOL(disp_primary_set_flip_function);

int disp_OSD0_set_flip_function(void *hDisp, int value)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	/* Osd layer */
	if (value & 0x0A) {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 0, SP_DISP_FLIP_V);
	}
	else {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 0, SP_DISP_FLIP_NONE);
	}

	return ret;

}
EXPORT_SYMBOL(disp_OSD0_set_flip_function);

int disp_OSD1_set_flip_function(void *hDisp, int value)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	/* Osd layer */
	if (value & 0x14) {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 1, SP_DISP_FLIP_V);
	}
	else {
		gpHalDispSetOsdFlip(WorkMem->RegBase, 1, SP_DISP_FLIP_NONE);
	}

	return ret;
}
EXPORT_SYMBOL(disp_OSD1_set_flip_function);

int disp_set_dither_param(void *hDisp, const gp_disp_ditherparam_t *pDitherParam)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	unsigned long ditherParam[2];
	int ret = 0;
	MSG("\n");

	ditherParam[0] = ((pDitherParam->d00 & 0x3) << 28 |
			(pDitherParam->d01 & 0x3) << 24 |
			(pDitherParam->d02 & 0x3) << 20 |
			(pDitherParam->d03 & 0x3) << 16 |
			(pDitherParam->d10 & 0x3) << 12 |
			(pDitherParam->d11 & 0x3) << 8 |
			(pDitherParam->d12 & 0x3) << 4 |
			(pDitherParam->d13 & 0x3));

	ditherParam[1] = ((pDitherParam->d20 & 0x3) << 28 |
			(pDitherParam->d21 & 0x3) << 24 |
			(pDitherParam->d22 & 0x3) << 20 |
			(pDitherParam->d23 & 0x3) << 16 |
			(pDitherParam->d30 & 0x3) << 12 |
			(pDitherParam->d31 & 0x3) << 8 |
			(pDitherParam->d32 & 0x3) << 4 |
			(pDitherParam->d33 & 0x3));

	gpHalDispSetDitherMap(WorkMem->RegBase, ditherParam[0], ditherParam[1]);

	return ret;
}
EXPORT_SYMBOL(disp_set_dither_param);

int disp_get_dither_param(void *hDisp, gp_disp_ditherparam_t *pDitherParam)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	unsigned int ditherParam[2];
	int ret = 0;
	MSG("\n");

	gpHalDispGetDitherMap(WorkMem->RegBase, &ditherParam[0], &ditherParam[1]);

	pDitherParam->d00 = (ditherParam[0] >> 28) & 0x3;
	pDitherParam->d01 = (ditherParam[0] >> 24) & 0x3;
	pDitherParam->d02 = (ditherParam[0] >> 20) & 0x3;
	pDitherParam->d03 = (ditherParam[0] >> 16) & 0x3;
	pDitherParam->d10 = (ditherParam[0] >> 12) & 0x3;
	pDitherParam->d11 = (ditherParam[0] >>  8) & 0x3;
	pDitherParam->d12 = (ditherParam[0] >>  4) & 0x3;
	pDitherParam->d13 = (ditherParam[0]      ) & 0x3;

	pDitherParam->d20 = (ditherParam[1] >> 28) & 0x3;
	pDitherParam->d21 = (ditherParam[1] >> 24) & 0x3;
	pDitherParam->d22 = (ditherParam[1] >> 20) & 0x3;
	pDitherParam->d23 = (ditherParam[1] >> 16) & 0x3;
	pDitherParam->d30 = (ditherParam[1] >> 12) & 0x3;
	pDitherParam->d31 = (ditherParam[1] >>  8) & 0x3;
	pDitherParam->d32 = (ditherParam[1] >>  4) & 0x3;
	pDitherParam->d33 = (ditherParam[1]      ) & 0x3;

	return ret;
}
EXPORT_SYMBOL(disp_get_dither_param);

int disp_set_color_matrix(void *hDisp, const gp_disp_colormatrix_t *pColorMatrix)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	gpHalDispSetColorMatrix(WorkMem->RegBase, (const UINT16 *)pColorMatrix);

	return ret;
}
EXPORT_SYMBOL(disp_set_color_matrix);

int disp_set_dyn_matrix_info(void *hDisp, const gp_disp_dyncmtxinfo_t *cmtxinfo)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");
	gpHalDispSetDynCMtxInfo(WorkMem->RegBase, (cmtxinfo->satlimit << 20) | (cmtxinfo->lolimit << 9) | cmtxinfo->hilimit);

	return ret;
}
EXPORT_SYMBOL(disp_set_dyn_matrix_info);

int disp_set_dyn_matrix_index(void *hDisp, const gp_disp_dyncmtxindex_t *cmtxindex)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	int i;
	unsigned long dispCMatxThre[4];
	MSG("\n");

	for(i=0;i<4;i++)
		dispCMatxThre[i] = (cmtxindex->index[i*3+2]<<18) | (cmtxindex->index[i*3+1]<<9) | cmtxindex->index[i*3];
	gpHalDispSetDynCMtxIndex(WorkMem->RegBase, (UINT32*)dispCMatxThre);

	return ret;
}
EXPORT_SYMBOL(disp_set_dyn_matrix_index);

int disp_set_dyn_matrix_para(void *hDisp, const gp_disp_dyncmtxpara_t *cmtxpara)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	gpHalDispSetDynCMtxTable(WorkMem->RegBase, cmtxpara);
	WorkMem->ic_REG258 = ((UINT32*)cmtxpara)[30];
	WorkMem->ic_REG278 = ((UINT32*)cmtxpara)[38];
	WorkMem->ic_REG298 = ((UINT32*)cmtxpara)[46];

	return ret;
}
EXPORT_SYMBOL(disp_set_dyn_matrix_para);

int disp_set_gamma_enable(void *hDisp, int enable)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	WorkMem->gammaEnable = enable;
	gpHalDispSetGammaEnable(WorkMem->RegBase, enable);

	return ret;
}
EXPORT_SYMBOL(disp_set_gamma_enable);

int disp_set_gamma_table(void *hDisp, int id, const unsigned char *pTable)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	memcpy(&WorkMem->gammaTable[id], pTable, sizeof(uint8_t) * 1024);
    WorkMem->gammaTableInit = 1;
   	gpHalDispSetGammaTable(WorkMem->RegBase, id, (uint8_t *) &WorkMem->gammaTable[id]);

	return ret;
}
EXPORT_SYMBOL(disp_set_gamma_table);

int disp_set_dyn_cmtx_enable(void *hDisp, int enable)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");
 	gpHalDispSetHueAdj(WorkMem->RegBase, enable);

	return ret;
}
EXPORT_SYMBOL(disp_set_dyn_cmtx_enable);

int disp_set_edge_gain(void *hDisp, int edgetype, int edgegain, int cortype, int cliptype)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int ret = 0;
	MSG("\n");

	gpHalDispSetEdgeType(WorkMem->RegBase, edgetype);
	gpHalDispSetEdgeGain(WorkMem->RegBase, edgegain);
	gpHalDispSetCortype(WorkMem->RegBase, cortype);
	gpHalDispSetCliptype(WorkMem->RegBase, cliptype);

	return ret;
}
EXPORT_SYMBOL(disp_set_edge_gain);

int
register_display(
	const gp_disp_drv_op *dispDrvOp
)
{
	gp_disp_drv_t *DispDrv = 0;
	int ret = 0;

	MSG("Enter\n");
	CHECK_VAL(dispDrvOp != 0);
	CHECK_VAL(dispDrvOp->Owner != 0);
	CHECK_VAL(dispDrvOp->Name != 0);
	CHECK_VAL(*dispDrvOp->Name != 0);
	CHECK_VAL(dispDrvOp->Open != 0);
	CHECK_VAL(dispDrvOp->Close != 0);
	CHECK_VAL(dispDrvOp->SetTiming != 0);

	MSG("name = %s\n", dispDrvOp->Name);

	CHECK_PRG(down_interruptible(&disp_registration_lock) >= 0);
	DispDrv = kmalloc(sizeof(DispDrv), GFP_KERNEL);
	if (!DispDrv) {
		ERROR("kmalloc memory fail\n");
		up(&disp_registration_lock);
		RETURN(-1);
	}
	DispDrv->DrvOp = dispDrvOp;
	list_add_tail(&DispDrv->list, &gDispOutputDevList);

	up(&disp_registration_lock);

Return:
	return ret;
}
EXPORT_SYMBOL(register_display);

int unregister_display(
	const gp_disp_drv_op *dispDrvOp
)
{
	int ret = 0;
	int found = 0;
	gp_disp_drv_t *DispDrv = 0;
	MSG("Enter\n");
	CHECK_PRG(dispDrvOp != 0);

	CHECK_PRG(down_interruptible(&disp_registration_lock) >= 0);
	list_for_each_entry(DispDrv, &gDispOutputDevList, list) {
		if(DispDrv->DrvOp == dispDrvOp) {
			found = 1;
			break;
		}
	}
	if(!found) {
		ERROR("display not found\n");
		up(&disp_registration_lock);
		RETURN(-1);
	}

	MSG("name = %s\n", dispDrvOp->Name);
	
	list_del(&DispDrv->list);
	kfree(DispDrv);
	up(&disp_registration_lock);

Return:
	return ret;
}
EXPORT_SYMBOL(unregister_display);

static const gp_disp_panel_ops_t *gPanelOps = 0;
static struct semaphore LcdResourceCnt; // resource count
typedef struct {
	void *RegBase;
} GP_LCD_WORKMEM;

GP_LCD_WORKMEM gLcdWorkMem;

static void *LCDDisp_Open(void *RegBase)
{
	MSG("\n");
#if 0
	if (down_trylock(&LcdResourceCnt) != 0) {
		ERROR("LCD Busy\n");
		return 0;
	}
#endif
	gLcdWorkMem.RegBase = RegBase;

	/* Set lcd output path */
	gpHalDispPathSelect(DISP_MODE_LCD, DISP_PATH_DISP0);

	return &gLcdWorkMem;
}

static void LCDDisp_Close(void *Inst)
{
	GP_LCD_WORKMEM *WorkMem = (GP_LCD_WORKMEM*)Inst;
	MSG("\n");
	WorkMem->RegBase = 0;
#if 0
	up(&LcdResourceCnt);
#endif
}

static int LCDDisp_OnEnable(void *Inst)
{
	GP_LCD_WORKMEM *WorkMem = (GP_LCD_WORKMEM*)Inst;
	MSG("\n");
	if(gPanelOps->init) gPanelOps->init();
	gpHalDispSetEnable(WorkMem->RegBase, HAL_DISP_DEV_LCD, 1);
	return 0;
}

static int LCDDisp_SetTiming(void *Inst, struct clk *pClk, int mode)
{
	GP_LCD_WORKMEM *WorkMem = (GP_LCD_WORKMEM*)Inst;
	int ret = 0;
	const panel_lcdInfo_t *panelInfo = 0;

	MSG("\n");
	CHECK_PRG(gPanelOps != 0);
	CHECK_PRG(gPanelOps->get_param != 0);
	panelInfo = (const panel_lcdInfo_t*)gPanelOps->get_param();
	CHECK_PRG(panelInfo != 0);

	//clk_set_rate(pClk, panelInfo->workFreq);
	gp_enable_clock( (int*)"DISP0", 1 );
	gpHalDispSetClock(panelInfo->workFreq, pllsel);

	gpHalDispSetRes(WorkMem->RegBase, panelInfo->resolution.width, panelInfo->resolution.height);
	gpHalDispSetLcdVsync(WorkMem->RegBase, panelInfo->vsync);
	gpHalDispSetLcdHsync(WorkMem->RegBase, panelInfo->hsync);
	gpHalDispSetLcdTiming(WorkMem->RegBase, 0, 0, panelInfo->vsync.fPorch + panelInfo->vsync.bPorch + panelInfo->resolution.height);
	gpHalDispSetPanelFormat(WorkMem->RegBase, panelInfo->format, panelInfo->type, panelInfo->dataSeqEven, panelInfo->dataSeqOdd);
	gpHalDispSetClkPolarity(WorkMem->RegBase, panelInfo->clkPolatiry);
	gpHalDispSetPriDmaType(WorkMem->RegBase, HAL_DISP_TV_DMA_PROGRESSIVE);
	gpHalDispSetOsdDmaType(WorkMem->RegBase, 0, HAL_DISP_TV_DMA_PROGRESSIVE);
	gpHalDispSetOsdDmaType(WorkMem->RegBase, 1, HAL_DISP_TV_DMA_PROGRESSIVE);
	gpHalDispSetColorMatrix(WorkMem->RegBase, (const UINT16 *) panelInfo->pColorMatrix);

	if (panelInfo->tcon_info.tcon_en != 0) {
		gpHalDispSetTconSTHarea(WorkMem->RegBase, panelInfo->tcon_stvh );
		gpHalDispSetTconSTVH(WorkMem->RegBase, panelInfo->tcon_stvh);
		gpHalDispSetTconOEH(WorkMem->RegBase, panelInfo->oeh );
		gpHalDispSetTconOEV(WorkMem->RegBase, panelInfo->oev );
		gpHalDispSetTconCKV(WorkMem->RegBase, panelInfo->ckv);
		gpHalDispSetTconPOL(WorkMem->RegBase, panelInfo->tcon_info);
		gpHalDispSetTconConfig(WorkMem->RegBase, panelInfo->tcon_info );
	}

	if (panelInfo->LVDSconfig.Enable != 0) {
		gpHalDispSetLVDSConfig(WorkMem->RegBase, panelInfo->LVDSconfig );
	}

Return:
	return ret;
}

static int LCDDisp_SetWorkFreq(gp_disp_panel_ops_t *PanelOps)
{
	int ret = 0;
	const panel_lcdInfo_t *panelInfo = 0;

	CHECK_PRG(PanelOps != 0);
	CHECK_PRG(PanelOps->get_param != 0);
	panelInfo = (const panel_lcdInfo_t*)PanelOps->get_param();
	CHECK_PRG(panelInfo != 0);
	
	printk("<%s> set disp0 work frequence %d\n",__FUNCTION__,panelInfo->workFreq);
	
	gpHalDispSetClock(panelInfo->workFreq, pllsel);
	
Return:
	return ret;
}
static int LCDDisp_OnPreSuspend(void *Inst)
{
	GP_LCD_WORKMEM *WorkMem = (GP_LCD_WORKMEM*)Inst;
	MSG("\n");

	// show full-screen white color-bar
	gpHalDispSetPriInputInfo(WorkMem->RegBase, HAL_DISP_INPUT_FMT_RGB, HAL_DISP_INPUT_TYPE_RGB888);
	gpHalDispSetColorBar(WorkMem->RegBase, 0, 0xFF, 0x00FFFFFF);
	return 0;
}

static int LCDDisp_OnPostSuspend(void *Inst)
{
	GP_LCD_WORKMEM *WorkMem = (GP_LCD_WORKMEM*)Inst;
	LVDSconfig_t LVDSConfig;
	MSG("\n");

	LVDSConfig.Enable		= 0;
	LVDSConfig.HFME			= 1;
	LVDSConfig.CV_R_LVDS	= 4;
	LVDSConfig.CPI_LVDS		= 1;
	LVDSConfig.PD_CH3_LVDS	= 1;
	LVDSConfig.PD_LVDS		= 1;
	gpHalDispSetLVDSConfig(WorkMem->RegBase, LVDSConfig);

	// pin group
	if(gPanelOps && gPanelOps->suspend) gPanelOps->suspend();
	return 0;
}

static int LCDDisp_OnResume(void *Inst)
{
	int ret = 0;

	MSG("Call pannel private resume\n");
	if(gPanelOps && gPanelOps->resume) gPanelOps->resume();
	return ret;
}

static int LCDDisp_GetPixelSize(void *Inst, gp_disp_pixelsize_t *size)
{
	int ret = 0;
	MSG("\n");
	const panel_lcdInfo_t *panelInfo = 0;

	CHECK_PRG(gPanelOps != 0);
	CHECK_PRG(gPanelOps->get_param != 0);
	panelInfo = (const panel_lcdInfo_t*)gPanelOps->get_param();
	
	size->width = panelInfo->pixelPitch.width;
	size->height = panelInfo->pixelPitch.height;
	
Return:
	return ret;
}

static gp_disp_drv_op DispLcdDrv =
{
	.Owner			= THIS_MODULE,
	.Open			= LCDDisp_Open,
	.Close			= LCDDisp_Close,
	.OnEnable		= LCDDisp_OnEnable,
	.SetTiming		= LCDDisp_SetTiming,
	.OnPreSuspend	= LCDDisp_OnPreSuspend,
	.OnPostSuspend	= LCDDisp_OnPostSuspend,
	.OnResume		= LCDDisp_OnResume,
	.GetPixelSize	= LCDDisp_GetPixelSize,
};

int
register_paneldev(
	int panelType,
	const char *name,
	const gp_disp_panel_ops_t *_panelOps
)
{
	int ret = 0;
	CHECK_VAL(name != 0);
	CHECK_VAL(*name != 0);
	CHECK_VAL(_panelOps != 0);
	CHECK_VAL(gPanelOps == 0);

	MSG("name=%s\n", name);

	DispLcdDrv.Name = name;
	DispLcdDrv.Type = panelType;
	gPanelOps = _panelOps;
	ret = register_display(&DispLcdDrv);
Return:
	return ret;
}
EXPORT_SYMBOL(register_paneldev);

int unregister_paneldev(int panelType, const char *name)
{
	int ret;
	MSG("name=%s\n", name);
	ret = unregister_display(&DispLcdDrv);
	gPanelOps = 0;
	return ret;
}
EXPORT_SYMBOL(unregister_paneldev);

static irqreturn_t disp_irq(int irq, void *param)
{
	int wake_flag = 0;
	int show_flag = 0;
	int intFlag;
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)param;

	//MSG("Enter\n");
	intFlag = gpHalDispGetIntFlag(WorkMem->RegBase);
	intFlag &= ~2; // clear update flag
	gpHalDispClearIntFlag(WorkMem->RegBase, intFlag);

	if (intFlag & (HAL_DISP_INT_FRAME_END | HAL_DISP_INT_UPDATE_FAIL | HAL_DISP_INT_DISPLAY_OFF)) {
		if(WorkMem->DispOffAck == 0) show_flag = 1;
		if(WorkMem->DispOffAck_Suspend == 0) show_flag = 1;
		if(WorkMem->UpdateAck_Suspend == 0) show_flag = 1;
	}

	if(intFlag & HAL_DISP_INT_FRAME_END)
	{
		if(WorkMem->UpdateAck == 0) {
			WorkMem->UpdateAck = 1;
			wake_flag = 1;
		}

		if(WorkMem->UpdateAck_Suspend == 0) WorkMem->UpdateAck_Suspend = 1;
	}

	if(intFlag & HAL_DISP_INT_UPDATE_FAIL) {
		WARNING("UPDATE FAIL\n");
		show_flag = 1;
	}

	if(intFlag & HAL_DISP_INT_DISPLAY_OFF)
	{
		INFO("DISPLAY %d OFF\n", DISPNUM(WorkMem));
		if(WorkMem->DispOffAck == 0)
		{
			WorkMem->DispOffAck = 1;
			WorkMem->state = DISP_STATE_CLOSE;
			MSG("wake up gDispWaitQ\n");
			wake_flag = 1;
		}
		if(WorkMem->DispOffAck_Suspend == 0) WorkMem->DispOffAck_Suspend = 1;
		show_flag = 1;
	}

	if(WorkMem->DrvOp->OnInterrupt && (intFlag & WorkMem->DrvOp->InterruptMask)) {
		WorkMem->DrvOp->OnInterrupt(WorkMem->hDev, (intFlag & WorkMem->DrvOp->InterruptMask));
	}

	if(wake_flag) wake_up_interruptible(&gDispWaitQ);
	if(show_flag) MSG("disp%d INT FLAG = %08X\n", DISPNUM(WorkMem), intFlag);

	return IRQ_HANDLED;
}

static const gp_disp_drv_op *
disp_load_driver(
	const char *dev_name
)
{
	int found = 0;
	int ret = 0;
	gp_disp_drv_t *DispDrv = 0;

	MSG("\n");

	if(dev_name == 0) {
		ERROR("dev_name can not be NULL\n");
		RETURN(-1);
	}

	list_for_each_entry(DispDrv, &gDispOutputDevList, list) {
		if (strncmp(DispDrv->DrvOp->Name, dev_name, GP_DISP_DEV_NAME_MAX_SIZE) == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		ERROR("display <%s> not found\n", dev_name);
		RETURN(-1);
	}

	if (!try_module_get(DispDrv->DrvOp->Owner)) {
		ERROR("module lock fail\n");
		RETURN(-1);
	}

Return:
	if(ret < 0) return NULL;
	else return DispDrv->DrvOp;
}

static void
disp_unload_driver(
	const gp_disp_drv_op *DrvOp
)
{
	module_put(DrvOp->Owner);
}

void *disp_open(
	const char *dev_name,
	int timing,
	int ch
)
{
	GP_DISP_WORKMEM *WorkMem = gDispWorkMem + ch;
	MSG("%s\n", dev_name);
	WorkMem->DrvOp = NULL;
	if (down_interruptible(&disp_registration_lock) >= 0) {
		WorkMem->DrvOp = disp_load_driver(dev_name);
		up(&disp_registration_lock);
	}

	if(!WorkMem->DrvOp) goto fail_load;
	WorkMem->hDev = 0;
	if(disp_init(WorkMem, timing) < 0) {
		ERROR("display init fail\n");
		goto fail_init;
	}

	MSG("open <%s> successed\n", dev_name);
	return WorkMem;

fail_init:
	disp_unload_driver(WorkMem->DrvOp);
	WorkMem->DrvOp = 0;
fail_load:
	return 0;
}
EXPORT_SYMBOL(disp_open);

static gp_disp_drv_t*
disp_get_output_driver(
	int32_t type
)
{
	gp_disp_drv_t *DispDrv = 0;

	MSG("type=%d\n", type);

	if (list_empty(&gDispOutputDevList)) {
		WARNING("no output device installed\n");
		return NULL;
	}

	if (type < 0) {
		/* FIXME : default output device should be configured */
		/* Current implementation, find the fist installed device */
		list_for_each_entry(DispDrv, &gDispOutputDevList, list) {
			return DispDrv;
		}
	}

	list_for_each_entry(DispDrv, &gDispOutputDevList, list) {
		if (DispDrv->DrvOp->Type == type)
			return DispDrv;
	}

	ERROR("can not find output device, type = %d\n", type);

	return NULL;
}

static int
disp_set_output(
	void *hDisp,
	gp_disp_output_t dispOutputDev
)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	gp_disp_drv_t *DispDrv = 0;
	int ret = 0;

	MSG("type=%d, mode=%d\n", dispOutputDev.type, dispOutputDev.mode);

	/* Get new disp device */
	DispDrv = disp_get_output_driver(dispOutputDev.type);
	CHECK_PRG(DispDrv != 0);

	/* Close previous disp device */
	disp_close(WorkMem, 1);

	/* Switch to new disp device */
	WorkMem->curOutputType = DispDrv->DrvOp->Type;
	disp_open(DispDrv->DrvOp->Name, dispOutputDev.mode, 0);

Return:
	return ret;
}

static int
disp_get_output(
	void *hDisp,
	gp_disp_output_t *pDispOutputDev
)
{

	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	gp_disp_drv_t *DispDrv = 0;
	int ret = 0;

	MSG("type=%d\n", pDispOutputDev->type);

	/* if type < 0, then get current output type */
	if (pDispOutputDev->type < 0) {
		CHECK_VAL(WorkMem->curOutputType >= 0);

		pDispOutputDev->type = WorkMem->curOutputType;
		pDispOutputDev->mode = WorkMem->DispMode;
		strncpy(pDispOutputDev->name, WorkMem->DrvOp->Name, 32);
		return 0;
	}

	/* Get disp device */
	DispDrv = disp_get_output_driver(pDispOutputDev->type);
	CHECK_PRG(DispDrv != 0);

	pDispOutputDev->mode = 0;
	if (DispDrv->DrvOp->GetSupportMode)
		ret = DispDrv->DrvOp->GetSupportMode(&pDispOutputDev->mode);
	strncpy(pDispOutputDev->name, DispDrv->DrvOp->Name, 32);
	MSG("mode=0x%x, name=%s\n", pDispOutputDev->mode, pDispOutputDev->name);
Return:
	return ret;
}

static int
disp_set_initial(
	void *hDisp
)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	gp_disp_drv_t *DispDrv = 0;
	int ret = 0;

	/* If current output index exist, then display black screen */
	if (WorkMem->curOutputType >= 0) {
		int i;
		/* Disable primary/osd layer */
		disp_set_pri_enable(WorkMem, 0);
		for (i=0; i<HAL_DISP_OSD_MAX; i++) {
			disp_set_osd_enable(WorkMem, i, 0);
		}
		disp_update(WorkMem);
		return 0;
	}

	DispDrv = disp_get_output_driver(-1);
	CHECK_PRG(DispDrv != 0);

	MSG("[%d][%s]\n", DispDrv->DrvOp->Type, DispDrv->DrvOp->Name);
	WorkMem->curOutputType = DispDrv->DrvOp->Type;
	/* FIXME : default display mode can be configured */
	disp_open(DispDrv->DrvOp->Name, -1, 0);

Return:
	return ret;
}

static void disp_pre_suspend(GP_DISP_WORKMEM *WorkMem)
{
	MSG("Enter\n");
	if(WorkMem->DrvOp->OnPreSuspend)
		WorkMem->DrvOp->OnPreSuspend(WorkMem->hDev);
	gpHalDispSetColorBarEnable(WorkMem->RegBase, 1);
	gpHalDispSetOsdEnable(WorkMem->RegBase, 0, 0);
	gpHalDispSetOsdEnable(WorkMem->RegBase, 1, 0);
}

void disp_close(void *hDisp, int ClkOff)
{
	int ret = 0;
	unsigned long tmp;
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)hDisp;
	int intFlag;

	MSG("Enter\n");
	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->DrvOp != 0);
	CHECK_PRG(WorkMem->hDev != 0);
	CHECK_PRG(WorkMem->RegBase != 0);
	CHECK_PRG(WorkMem->state == DISP_STATE_OPEN);

	MSG("close <%s>\n", WorkMem->DrvOp->Name);

	// pre_suspend
	disp_pre_suspend(WorkMem);
	disp_update(WorkMem);

	// disp_set_off_and_then_update
	local_irq_save(tmp);
	WorkMem->DispOffAck = 0;
	WorkMem->state = DISP_STATE_PRE_CLOSE;
	gpHalDispSetEnable(WorkMem->RegBase, 0, 0);
	gpHalDispUpdateParameter(WorkMem->RegBase);
	local_irq_restore(tmp);

	MSG("wait gDispWaitQ\n");
	ret = wait_event_interruptible_timeout(gDispWaitQ, WorkMem->DispOffAck != 0, HZ / 20);
	if(ret==0) {
		WorkMem->DispOffAck = 1;
		WARNING("Wait display-close TIMEOUT!!\n");
	}

	// stop interrupt
	intFlag = HAL_DISP_INT_FRAME_END | HAL_DISP_INT_DISPLAY_OFF | HAL_DISP_INT_UPDATE_FAIL;
	if(WorkMem->DrvOp->OnInterrupt) intFlag |= WorkMem->DrvOp->InterruptMask;
	gpHalDispSetIntDisable(WorkMem->RegBase, intFlag);

	if(WorkMem->DrvOp->OnPostSuspend)
		WorkMem->DrvOp->OnPostSuspend(WorkMem->hDev);

	WorkMem->DrvOp->Close(WorkMem->hDev);
	WorkMem->hDev = 0;
	disp_unload_driver(WorkMem->DrvOp);
	WorkMem->DrvOp = 0;

	WorkMem->flag_is_clk_enable = 0;

    if(ClkOff)
	    clk_disable(WorkMem->hClk);

Return:
	ret = 0;
}
EXPORT_SYMBOL(disp_close);

static int disp_fops_open(struct inode *inode, struct file *pfile)
{
	int ret = 0;
	int found = 0;
	int minor = iminor(inode);
	GP_DISP_WORKMEM *WorkMem = 0;

	MSG("\n");
	// TODO get FILE lock

	list_for_each_entry(WorkMem, &gDispWorkMemList, list) {
		if(minor == WorkMem->misc.minor) {
			found = 1;
			break;
		}
	}
	if (!found) {
		ERROR("invalid inode\n");
		pfile->private_data = NULL;
		ret = -EBUSY;
	}
	else {
		MSG("disp%d\n", DISPNUM(WorkMem));
		pfile->private_data = WorkMem;
	}
	return ret;
}

static int disp_fops_release(struct inode *inode, struct file *pfile)
{
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)pfile->private_data;

	MSG("disp%d\n", DISPNUM(WorkMem));
	return 0;
}

static long disp_fops_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)pfile->private_data;
	int enable;
	int mode;
	MSG("Enter, 0x%x\n", cmd);
	CHECK_PRG(WorkMem != 0);
	
	if(down_interruptible(&disp_sema) != 0) {
		ERROR("disp_sema error!\n");
		return -ERESTARTSYS;
	}
	
	switch (cmd)
	{
	case DISPIO_SET_INITIAL:
		if(g_disp_init)//have initial
		{
			printk("disp0 have inital,not need reinitial!\r\n");
			ret = 0;
			goto Return;
		}		
		else
		{
			ret = disp_set_initial(WorkMem);
			if(ret == 0) {
				g_disp_init = 1;
			}
		}
		break;
		
	case DISPIO_SET_UPDATE:
		disp_update(WorkMem);
		break;

	case DISPIO_GET_PANEL_RESOLUTION:
        if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->panelSize, sizeof(gp_disp_res_t))) {
			ret = -EIO;
		}
		break;
		
	case DISPIO_GET_PANEL_PIXELSIZE:
		disp_get_panel_pixelsize(WorkMem, &WorkMem->pixelSize);
        if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->pixelSize, sizeof(gp_disp_res_t))) {
			ret = -EIO;
		}
		break;

	case DISPIO_WAIT_FRAME_END:
		disp_wait_for_updated(WorkMem);
		break;

	case DISPIO_SET_BACKLIGHT:
		{
#if 0
			gp_board_panel_t *panel_config;
			panel_config = gp_board_get_config("panel", gp_board_panel_t);
			if (panel_config && panel_config->set_backlight) {
				panel_config->set_backlight((uint32_t) arg);
			}
			else {
				ERROR("Panel backlight not found!\n");
			}
#endif
		}
		break;

	case DISPIO_SET_OUTPUT:
		{
			gp_disp_output_t dispOutputDev;

			if (copy_from_user((void*) &dispOutputDev, (const void __user *) arg, sizeof(gp_disp_output_t))) {
				ret = -EIO;
				break;
			}
			ret = disp_set_output(WorkMem, dispOutputDev);
		}
		break;

	case DISPIO_GET_OUTPUT:
		{
			gp_disp_output_t dispOutputDev;

			if (copy_from_user((void*) &dispOutputDev, (const void __user *) arg, sizeof(gp_disp_output_t))) {
				ret = -EIO;
				break;
			}
			ret = disp_get_output(WorkMem, &dispOutputDev);
			if (copy_to_user ((void __user *) arg, (const void *) &dispOutputDev, sizeof(gp_disp_output_t))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_SET_MODE:
		CHECK_PRG(copy_from_user((void *)&mode, (const void __user *) arg, sizeof(int)))
		CHECK_PRG(disp_set_mode(WorkMem, mode) >= 0);
		break;

	case DISPIO_GET_MODE:
		mode = disp_get_mode(WorkMem);
		CHECK_PRG(copy_to_user ((void __user *) arg, (const void*) &mode, sizeof(int)));
		break;

	// Primary layer
	case DISPIO_SET_PRI_ENABLE:
		disp_set_pri_enable(WorkMem, arg);
		break;

	case DISPIO_GET_PRI_ENABLE:
		enable = disp_get_pri_enable(WorkMem);
		if (copy_to_user ((void __user *) arg, (const void *) &enable, sizeof(int))) {
			ret = -EIO;
		}
		break;

	case DISPIO_SET_PRI_BITMAP:
		{
			gp_bitmap_t bitmap;

			if (copy_from_user((void*) &bitmap, (const void __user *) arg, sizeof(gp_bitmap_t))) {
				ret = -EIO;
				break;
			}
			bitmap.pData = (uint8_t*) gp_user_va_to_pa(bitmap.pData);
			if(bitmap.pDataU) bitmap.pDataU = (uint8_t*) gp_user_va_to_pa(bitmap.pDataU);
			if(bitmap.pDataV) bitmap.pDataV = (uint8_t*) gp_user_va_to_pa(bitmap.pDataV);
			disp_set_pri_bitmap(WorkMem, &bitmap);
		}
		break;

	case DISPIO_GET_PRI_BITMAP:
		if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->priBitmap, sizeof(gp_bitmap_t))) {
			ret = -EIO;
		}
		break;

	case DISPIO_SET_PRI_SCALEINFO:
		{
			gp_disp_scale_t scale;

			if (copy_from_user((void*) &scale, (const void __user *) arg, sizeof(gp_disp_scale_t))) {
				ret = -EIO;
					break;
			}
			disp_set_pri_scale(WorkMem, &scale);
		}
		break;

	case DISPIO_GET_PRI_SCALEINFO:
		if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->priScaleInfo, sizeof(gp_disp_scale_t))) {
			ret = -EIO;
		}
		break;

	case DISPIO_CHANGE_PRI_BITMAP_BUF:
		{
			gp_bitmap_t bitmap;

			if (copy_from_user((void*) &bitmap, (const void __user *) arg, sizeof(gp_bitmap_t))) {
				ret = -EIO;
			break;
			}
			if(bitmap.pData) bitmap.pData = (void*)gp_user_va_to_pa(bitmap.pData);
			if(bitmap.pDataU) bitmap.pDataU = (uint8_t*)gp_user_va_to_pa(bitmap.pDataU);

			disp_set_pri_frame_addr(WorkMem, bitmap.pData, bitmap.pDataU);
        }
		break;

    case DISPIO_SET_PRI_ADDRESS:
		{
			gp_disp_pri_addr_t pri_addr;

			if (copy_from_user((void*) &pri_addr, (const void __user *) arg, sizeof(gp_disp_pri_addr_t))) {
				ret = -EIO;
				break;
			}
			if(pri_addr.addr) pri_addr.addr = (void*)gp_user_va_to_pa(pri_addr.addr);
			if(pri_addr.addrU) pri_addr.addrU = (uint8_t*)gp_user_va_to_pa(pri_addr.addrU);

			disp_set_pri_frame_addr(WorkMem, pri_addr.addr, pri_addr.addrU);
		}
		break;

	// Dithering
	case DISPIO_SET_DITHER_ENABLE:
		disp_set_dither_enable(WorkMem, arg);
		break;

	case DISPIO_GET_DITHER_ENABLE:
		{
			uint32_t enable;
			enable = gpHalDispGetDitherEnable(WorkMem->RegBase);
			if (copy_to_user ((void __user *) arg, (const void *) &enable, sizeof(uint32_t))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_SET_DITHER_TYPE:
		disp_set_dither_type(WorkMem, arg);
		break;

	case DISPIO_GET_DITHER_TYPE:
		{
			uint32_t type;
			type = gpHalDispGetDitherType(WorkMem->RegBase);
			if (copy_to_user ((void __user *) arg, (const void *) &type, sizeof(uint32_t))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_SET_DITHER_PARAM:
		{
			gp_disp_ditherparam_t ditherParam;
			if (copy_from_user((void*) &ditherParam, (const void __user *) arg, sizeof(gp_disp_ditherparam_t))) {
				ret = -EIO;
				break;
			}
			disp_set_dither_param(WorkMem, (gp_disp_ditherparam_t *) &ditherParam);
		}
		break;

	case DISPIO_GET_DITHER_PARAM:
		{
			gp_disp_ditherparam_t ditherParam;
			disp_get_dither_param(WorkMem, (gp_disp_ditherparam_t *) &ditherParam);
			if (copy_to_user ((void __user *) arg, (const void *) &ditherParam, sizeof(gp_disp_ditherparam_t))) {
				ret = -EIO;
			}
		}
		break;

	// Color matrix
	case DISPIO_SET_CMATRIX_PARAM:
		{
			gp_disp_colormatrix_t colorMatrix;
			if (copy_from_user((void*) &colorMatrix, (const void __user *) arg, sizeof(gp_disp_colormatrix_t))) {
				ret = -EIO;
				break;
			}
			CHECK_PRG(disp_set_color_matrix(WorkMem, &colorMatrix) >= 0);
		}
		break;

	case DISPIO_GET_CMATRIX_PARAM:
		{
			gp_disp_colormatrix_t colorMatrix;
			gpHalDispGetColorMatrix(WorkMem, (unsigned short *) &colorMatrix);
			if (copy_to_user ((void __user *) arg, (const void *) &colorMatrix, sizeof(gp_disp_colormatrix_t))) {
				ret = -EIO;
				break;
			}
		}
		break;

	case DISPIO_SET_DYN_CMTX_INFO:
		{
			gp_disp_dyncmtxinfo_t cmtxinfo;
			if (copy_from_user((void*) &cmtxinfo, (const void __user *) arg, sizeof(gp_disp_dyncmtxinfo_t))) {
				ret = -EIO;
				break;
			}
 			CHECK_PRG(disp_set_dyn_matrix_info(WorkMem, &cmtxinfo) >= 0);
		}
		break;

	case DISPIO_SET_DYN_CMTX_INDEX:
		{
			gp_disp_dyncmtxindex_t cmtxindex;
			if (copy_from_user((void*) &cmtxindex, (const void __user *) arg, sizeof(gp_disp_dyncmtxindex_t))) {
				ret = -EIO;
				break;
			}
			CHECK_PRG(disp_set_dyn_matrix_index(WorkMem, &cmtxindex) >= 0);
		}
		break;

	case DISPIO_SET_DYN_CMTX_PARA:
		{
			gp_disp_dyncmtxpara_t cmtxpara;
			if (copy_from_user((void*) &cmtxpara, (const void __user *) arg, sizeof(gp_disp_dyncmtxpara_t))) {
				ret = -EIO;
				break;
			}
			CHECK_PRG(disp_set_dyn_matrix_para(WorkMem, &cmtxpara) >= 0);
		}
		break;

	case DISPIO_SET_DYN_CMTX_ENABLE:
		disp_set_dyn_cmtx_enable(WorkMem, arg);
		break;

	// Gamma table
	case DISPIO_SET_GAMMA_ENABLE:
		disp_set_gamma_enable(WorkMem, arg);
		break;

	case DISPIO_GET_GAMMA_ENABLE:
		if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->gammaEnable, sizeof(uint32_t))) {
			ret = -EIO;
		}
		break;

	case DISPIO_SET_GAMMA_PARAM:
		{
			gp_disp_gammatable_t *pGammaTable;
			pGammaTable = kmalloc(sizeof(gp_disp_gammatable_t), GFP_KERNEL);
			if (!pGammaTable) {
				ret = -ENOMEM;
				break;
			}
			if (copy_from_user((void*) pGammaTable, (const void __user *) arg, sizeof(gp_disp_gammatable_t))) {
				kfree(pGammaTable);
				ret = -EIO;
				break;
			}
			disp_set_gamma_table(WorkMem, pGammaTable->id, (uint8_t*) &pGammaTable->table);
			kfree(pGammaTable);
		}
		break;

	case DISPIO_GET_GAMMA_PARAM:
		{
			gp_disp_gammatable_t *pGammaTable = (gp_disp_gammatable_t *)arg;
			uint32_t id;

			if (copy_from_user((void*) &id, (const void __user *) &pGammaTable->id, sizeof(uint32_t))) {
				ret = -EIO;
				break;
			}
			MSG("Get gamma parameter, id=%d\n", id);
			if (copy_to_user ((void __user *) &pGammaTable->table, (const void *) WorkMem->gammaTable[id], sizeof(uint8_t) * 256)) {
				ret = -EIO;
			}
		}
		break;

	// Color bar
	case DISPIO_SET_CBAR_ENABLE:
		gpHalDispSetColorBarEnable(WorkMem->RegBase, (uint32_t) arg);
		break;

	case DISPIO_GET_CBAR_ENABLE:
		{
			uint32_t enable;
			enable = gpHalDispGetColorBarEnable(WorkMem->RegBase);
			MSG("DISPIO_GET_CBAR_ENABLE = %d\n", enable);
			if (copy_to_user ((void __user *) arg, (const void *) &enable, sizeof(uint32_t))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_SET_CBARINFO:
		{
			gp_disp_colorbar_t colorBar;

			if (copy_from_user((void*) &colorBar, (const void __user *) arg, sizeof(gp_disp_colorbar_t))) {
				ret = -EIO;
				break;
			}
			MSG("DISPIO_SET_CBARINFO, type=%d, size=%d, color=%d\n",
				colorBar.type, colorBar.size, colorBar.color);

			gpHalDispSetColorBar(WorkMem->RegBase, colorBar.type, colorBar.size, colorBar.color);
		}
		break;

	// Osd layer
	case DISPIO_GET_OSD_TOTALNUM:
		{
			uint32_t num = HAL_DISP_OSD_MAX;
			if (copy_to_user ((void __user *) arg, (const void *) &num, sizeof(uint32_t))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_SET_OSD_ENABLE(0):
	case DISPIO_SET_OSD_ENABLE(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			disp_set_osd_enable(WorkMem, layerNum, arg);
		}
		break;

	case DISPIO_GET_OSD_ENABLE(0):
	case DISPIO_GET_OSD_ENABLE(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->osdEnable[layerNum], sizeof(int))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_SET_OSD_BITMAP(0):
	case DISPIO_SET_OSD_BITMAP(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			gp_bitmap_t bitmap;

			if (copy_from_user((void*) &bitmap, (const void __user *) arg, sizeof(gp_bitmap_t))) {
				ret = -EIO;
				break;
			}
			bitmap.pData = (uint8_t*) gp_user_va_to_pa(bitmap.pData);
			disp_set_osd_bitmap(WorkMem, layerNum, &bitmap);
		}
		break;

	case DISPIO_GET_OSD_BITMAP(0):
	case DISPIO_GET_OSD_BITMAP(1):
		{
			uint32_t layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->osdBitmap[layerNum], sizeof(gp_bitmap_t))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_CHANGE_OSD_BITMAP_BUF(0):
	case DISPIO_CHANGE_OSD_BITMAP_BUF(1):
		{
            int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			gp_bitmap_t bitmap;

            if (copy_from_user((void*) &bitmap, (const void __user *) arg, sizeof(gp_bitmap_t))) {
                ret = -EIO;
                break;
            }
            if(bitmap.pData) bitmap.pData = (void*)gp_user_va_to_pa(bitmap.pData);
            disp_set_osd_frame_addr(WorkMem, layerNum, bitmap.pData);
		}
		break;

	case DISPIO_SET_OSD_ADDRESS(0):
	case DISPIO_SET_OSD_ADDRESS(1):
		{
            int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
            gp_disp_pri_addr_t pri_addr;

            if (copy_from_user((void*) &pri_addr, (const void __user *) arg, sizeof(gp_disp_pri_addr_t))) {
                ret = -EIO;
                break;
            }
            if(pri_addr.addr) pri_addr.addr = (void*)gp_user_va_to_pa(pri_addr.addr);
            disp_set_osd_frame_addr(WorkMem, layerNum, pri_addr.addr);
		}
		break;

	case DISPIO_SET_OSD_SCALEINFO(0):
	case DISPIO_SET_OSD_SCALEINFO(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			gp_disp_scale_t scale;

			if (copy_from_user((void*) &scale, (const void __user *) arg, sizeof(gp_disp_scale_t))) {
				ret = -EIO;
				break;
			}
			disp_set_osd_scale(WorkMem, layerNum, &scale);
		}
		break;

	case DISPIO_GET_OSD_SCALEINFO(0):
	case DISPIO_GET_OSD_SCALEINFO(1):
		{
			uint32_t layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->osdScaleInfo[layerNum], sizeof(gp_disp_scale_t))) {
				ret = -EIO;
			}
		}
		break;

	case DISPIO_SET_OSD_PALETTE(0):
	case DISPIO_SET_OSD_PALETTE(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			gp_disp_osdpalette_t *pOsdPalette;

			pOsdPalette = kmalloc(sizeof(gp_disp_osdpalette_t), GFP_KERNEL);
			if (copy_from_user((void*) pOsdPalette, (const void __user *) arg, sizeof(gp_disp_osdpalette_t))) {
				kfree(pOsdPalette);
				ret = -EIO;
				break;
			}
			WorkMem->osdPalette[layerNum].type = pOsdPalette->type;
			memcpy(&WorkMem->osdPalette[layerNum].table[pOsdPalette->startIndex], &pOsdPalette->table[0], sizeof(int) * pOsdPalette->count);
			gpHalDispSetOsdInputType(WorkMem->RegBase, layerNum, WorkMem->osdPalette[layerNum].type);
			gpHalDispSetOsdPalette(WorkMem->RegBase, layerNum, pOsdPalette->startIndex, pOsdPalette->count, (int*) &pOsdPalette->table[0]);
			kfree(pOsdPalette);
		}
		break;

	case DISPIO_GET_OSD_PALETTE(0):
	case DISPIO_GET_OSD_PALETTE(1):
		{
			uint32_t layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			gp_disp_osdpalette_t *pOsdPalette = (gp_disp_osdpalette_t *) arg;
			uint32_t startIndex;
			uint32_t count;

			if (copy_from_user((void*) &startIndex, (const void __user *) &pOsdPalette->startIndex, sizeof(uint32_t))) {
				ret = -EIO;
				break;
			}
			if (copy_from_user((void*) &count, (const void __user *) &pOsdPalette->count, sizeof(uint32_t))) {
				ret = -EIO;
				break;
			}
			MSG("get palette, index=%d, count=%d\n", startIndex, count);
			if (copy_to_user ((void __user *) &pOsdPalette->type, (const void *) &WorkMem->osdPalette[layerNum].type, sizeof(uint32_t))) {
				ret = -EIO;
				break;
			}
			if (copy_to_user ((void __user *) &pOsdPalette->table[0], (const void *) &WorkMem->osdPalette[layerNum].table[startIndex], sizeof(uint32_t) * count)) {
				ret = -EIO;
				break;
			}

		}
		break;

	case DISPIO_SET_OSD_PALETTEOFFSET(0):
	case DISPIO_SET_OSD_PALETTEOFFSET(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			WorkMem->osdPaletteOffset[layerNum] = arg;
			gpHalDispSetOsdPaletteOffset(WorkMem->RegBase, layerNum, arg);
		}
		break;

	case DISPIO_GET_OSD_PALETTEOFFSET(0):
	case DISPIO_GET_OSD_PALETTEOFFSET(1):
		{
			uint32_t layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->osdPaletteOffset[layerNum], sizeof(uint32_t))) {
				ret = -EIO;
				break;
			}
		}
		break;

	case DISPIO_SET_OSD_ALPHA(0):
	case DISPIO_SET_OSD_ALPHA(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			gp_disp_osdalpha_t alpha;

			if (copy_from_user((void*) &alpha, (const void __user *) arg, sizeof(gp_disp_osdalpha_t))) {
				ret = -EIO;
				break;
			}
			disp_set_osd_alpha(WorkMem, layerNum, &alpha);
		}
		break;

	case DISPIO_GET_OSD_ALPHA(0):
	case DISPIO_GET_OSD_ALPHA(1):
		{
			uint32_t layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->osdAlpha[layerNum], sizeof(gp_disp_osdalpha_t))) {
				ret = -EIO;
				break;
			}
		}
		break;

	case DISPIO_SET_OSD_KEY(0):
	case DISPIO_SET_OSD_KEY(1):
		{
			int layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			WorkMem->osdColorKey[layerNum] = arg;
			gpHalDispSetOsdColorKey(WorkMem->RegBase, layerNum, arg);
		}
		break;

	case DISPIO_GET_OSD_KEY(0):
	case DISPIO_GET_OSD_KEY(1):
		{
			uint32_t layerNum = ((cmd & 0xff) - DISPIO_OSD_BASE) >> 4;
			if (copy_to_user ((void __user *) arg, (const void *) &WorkMem->osdColorKey[layerNum], sizeof(uint32_t))) {
				ret = -EIO;
				break;
			}
		}
		break;

	case DISPIO_SET_EDGE_GAIN:
		{
			gp_disp_edge_gain_t edge_gain;
			if (copy_from_user((void*) &edge_gain, (const void __user *) arg, sizeof(gp_disp_edge_gain_t))) {
				ret = -EIO;
				break;
			}
			disp_set_edge_gain(WorkMem, edge_gain.edgetype, edge_gain.edgegain, edge_gain.cortype, edge_gain.cliptype);
		}
		break;

	/* Flip function : for compatible with GPL320xxA */
	case DISPIO_SET_FLIP:
		disp_set_flip_function(WorkMem, (uint32_t) arg);
		break;
	case DISPIO_PRIMARY_SET_FLIP:
		disp_primary_set_flip_function(WorkMem, (uint32_t) arg);
		break;
	case DISPIO_OSD0_SET_FLIP:
		disp_OSD0_set_flip_function(WorkMem, (uint32_t) arg);
		break;
	case DISPIO_OSD1_SET_FLIP:
		disp_OSD1_set_flip_function(WorkMem, (uint32_t) arg);
		break;
	default:
		WARNING("unknow cmd, 0x%x\n", cmd);
		break;
	}
	MSG("Exit, 0x%x\n", cmd);
Return:
	up(&disp_sema);
	return ret;
}

static int display_probe(struct platform_device *pdev)
{
	GP_DISP_WORKMEM *WorkMem = gDispWorkMem + pdev->id;
	int retval;

	MSG("Enter\n");

	WorkMem->misc.minor	= MISC_DYNAMIC_MINOR;
	WorkMem->misc.name	= DispDevNodeName[pdev->id];
	WorkMem->misc.fops	= &disp_fops;

	INIT_LIST_HEAD(&WorkMem->list);
	list_add(&WorkMem->list, &gDispWorkMemList);

	retval = request_irq(IrqID[pdev->id], disp_irq, IRQF_DISABLED, IrqName[pdev->id], (void*)WorkMem);
	if (retval) {
		ERROR("request_irq error %d\n", retval);
		goto fail_irq;
	}

	// Registering device
	retval = misc_register(&WorkMem->misc);
	if (retval) {
		goto fail_register;
	}

	MSG("disp%d name = %s, minor = %d\n", DISPNUM(WorkMem), WorkMem->misc.name, WorkMem->misc.minor);

	return 0;

fail_register:
	free_irq(IrqID[pdev->id], (void*)WorkMem);
fail_irq:
	list_del(&WorkMem->list);
	return retval;
}

static int display_remove(struct platform_device *pdev)
{
	GP_DISP_WORKMEM *WorkMem = gDispWorkMem + pdev->id;

	MSG("disp%d\n", DISPNUM(WorkMem));

	// Freeing the major number
	misc_deregister(&WorkMem->misc);
	free_irq(IrqID[pdev->id], (void *)WorkMem);
	list_del(&WorkMem->list);

	return 0;
}

#define disp_busy_wait(cond) \
{\
	int i;\
	for(i=0;i<50;i++)\
	{\
		if(cond) break;\
		mdelay(1);\
	}\
	if(i != 50)\
		MSG("Busy wait for <%s> in %d msec\n", #cond, i);\
	else\
		WARNING("Busy wait for <%s> TIMEOUT!!\n", #cond);\
}

static int display_suspend(struct platform_device *pdev, pm_message_t state)
{
	GP_DISP_WORKMEM *WorkMem = gDispWorkMem + pdev->id;
	int ret = 0;
	int i;
	const unsigned long *src;
	unsigned long *dst;

	CHECK_PRG(WorkMem != 0);
	CHECK_PRG(WorkMem->RegBase != 0);

	MSG("disp%d\n", DISPNUM(WorkMem));

	// if device is enabled.
	switch(WorkMem->state)
	{
	case DISP_STATE_CLOSE:
		break;

	case DISP_STATE_OPEN:
		CHECK_PRG(WorkMem->DrvOp != 0);
		CHECK_PRG(WorkMem->hDev != 0);

		MSG("Backup registers\n");
		src = (const unsigned long*)WorkMem->RegBase;
		dst = (unsigned long*)WorkMem->BackupReg;
		for(i=0; i<sizeof(WorkMem->BackupReg)/sizeof(unsigned long); i++)
			*dst++ = *src++;

		WorkMem->BackupReg[0x258/4] = WorkMem->ic_REG258;
		WorkMem->BackupReg[0x278/4] = WorkMem->ic_REG278;
		WorkMem->BackupReg[0x298/4] = WorkMem->ic_REG298;
		WorkMem->BackupReg[0x324/4] = WorkMem->ic_REG324;

		// clear update flag
		// this register should NOT be stored back
		WorkMem->BackupReg[0xF4/4] = 0;

		// Busy Wait for UpdateACK
		disp_busy_wait((volatile int)WorkMem->UpdateAck != 0);
		// pre_suspend
		disp_pre_suspend(WorkMem);
		WorkMem->UpdateAck_Suspend = 0;
		gpHalDispUpdateParameter(WorkMem->RegBase);
		disp_busy_wait((volatile int)WorkMem->UpdateAck_Suspend != 0);

		MSG("Close display\n");
		WorkMem->DispOffAck_Suspend = 0;
		gpHalDispSetEnable(WorkMem->RegBase, 0, 0);
		gpHalDispUpdateParameter(WorkMem->RegBase);
		disp_busy_wait((volatile int)WorkMem->DispOffAck_Suspend!= 0);

		// post-suspend
		// 1. pin-group
		if(WorkMem->DrvOp->OnPostSuspend)
			WorkMem->DrvOp->OnPostSuspend(WorkMem->hDev);

		WorkMem->ClkRate = clk_get_rate(WorkMem->hClk);
		clk_disable(WorkMem->hClk);
		MSG("Backup clk = %lu Hz\n", WorkMem->ClkRate);
		break;

	case DISP_STATE_PRE_CLOSE:
		disp_busy_wait((volatile int)WorkMem->DispOffAck!= 0);
		WorkMem->ClkRate = clk_get_rate(WorkMem->hClk);
		clk_disable(WorkMem->hClk);
		break;
	}

Return:
	return ret;
}

static int display_resume(struct platform_device *pdev)
{
	int i = 0;
	GP_DISP_WORKMEM *WorkMem = gDispWorkMem + pdev->id;
	const unsigned long *src;
	unsigned long *dst;

	MSG("disp%d\n", DISPNUM(WorkMem));

	if(WorkMem->state == DISP_STATE_CLOSE) {
        return 0;
    }
	else if(WorkMem->state == DISP_STATE_PRE_CLOSE) {
		clk_enable(WorkMem->hClk);
		clk_set_rate(WorkMem->hClk, WorkMem->ClkRate);
		return 0;
	}

	MSG("Restore clk = %lu Hz\n", WorkMem->ClkRate);
	clk_enable(WorkMem->hClk);
	clk_set_rate(WorkMem->hClk, WorkMem->ClkRate);

	/* Set lcd output path */
	gpHalDispPathSelect(DISP_MODE_LCD, DISP_PATH_DISP0);
	/* Set lcd work frequence */
	LCDDisp_SetWorkFreq(gPanelOps);
	if(WorkMem->DrvOp && WorkMem->DrvOp->OnResume)
		WorkMem->DrvOp->OnResume(WorkMem->hDev);

	MSG("Restore registers\n");
	src = (const unsigned long*)WorkMem->BackupReg;
	dst = (unsigned long*)WorkMem->RegBase;
	for(i=0; i<sizeof(WorkMem->BackupReg)/sizeof(unsigned long); i++)
	{
		if(i<0x63)
			*dst++ = *src++;
//		*dst++ = *src++;
	}

	MSG("Restore gamma\n");
	for(i=0;i<3;i++)
		gpHalDispSetGammaTable(WorkMem->RegBase, i, WorkMem->gammaTable[i]);

	MSG("Restore palette\n");
	for(i=0;i<HAL_DISP_OSD_MAX;i++) {
		gpHalDispSetOsdPalette(
			WorkMem->RegBase,
			i,
			WorkMem->osdPalette[i].startIndex,
			WorkMem->osdPalette[i].count,
			WorkMem->osdPalette[i].table);
	}

	// if device is enabled
	if(WorkMem->state == DISP_STATE_OPEN) {
		gpHalDispUpdateParameter(WorkMem->RegBase);
		MSG("Open display\n");
	}

    if (DISPNUM(WorkMem) == 1) {
        const gp_bitmap_t *pbitmap;

        pbitmap = &WorkMem->priBitmap;
        WorkMem->DrvOp->SetTiming(WorkMem->hDev, WorkMem->hClk, WorkMem->DispMode);
        gpHalDispSetPriRes(WorkMem->RegBase, pbitmap->width,pbitmap->height);
	}

	return 0;
}

static void plat_dev_release(struct device * dev)
{
	MSG("Enter\n");
}

static struct platform_driver gp_display_driver = {
	.probe		= display_probe,
	.remove		= display_remove,
	.suspend	= display_suspend,
	.resume		= display_resume,
	.driver		=
	{
		.name	= DISPLAY_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init gp_display_init(void)
{
	int i;
	int ret;

	MSG("Enter\n");

	memset(gDispWorkMem, 0, sizeof(gDispWorkMem));

	for(i=0; i<MAX_DISP_DEVICE_NUM; i++) {
		GP_DISP_WORKMEM *WorkMem = gDispWorkMem + i;
		MSG("register disp%d\n", i);
		WorkMem->UpdateAck			= 1;
		WorkMem->DispOffAck			= 1;
		WorkMem->UpdateAck_Suspend	= 1;
		WorkMem->DispOffAck_Suspend	= 1;
		WorkMem->RegBase	        = gpHalDispGetRegBase(i);
		WorkMem->hClk		        = clk_get(NULL, DispClockName[i]);
		WorkMem->hDev		        = 0;
		WorkMem->DrvOp		        = 0;
		WorkMem->plat_dev.name      = DISPLAY_DRIVER_NAME;
		WorkMem->plat_dev.id        = i;
		WorkMem->plat_dev.dev.release = plat_dev_release;
		WorkMem->gammaEnable        = 0;
		WorkMem->gammaTableInit     = 0;
		WorkMem->curOutputType      = -1;
		
		if(strncmp(pll_sel, "PLL2", 5) == 0)
			pllsel = 2;
		else if(strncmp(pll_sel, "PLL0", 5) == 0)
			pllsel = 0;
		
		gp_enable_clock( (int*)"DISP0_PCLK", 1 );		// Just work around, FIXED ME latter
		gp_enable_clock( (int*)"DISP0_HDMI", 1 );
		
		gpHalDispInit(WorkMem->RegBase);
		gpHalDispSetVDACPowerDown(WorkMem->RegBase, 1);
		gpHalDispHDMIPHYConfig(WorkMem->RegBase, 1, 0);
		
		ret = platform_device_register(&gDispWorkMem[i].plat_dev);
		if (ret) {
			ERROR("disp%d device register failed, ret = %d\n", i, ret);
			goto device_register_fail;
		}
	}

	ret = platform_driver_register(&gp_display_driver);
	if (ret < 0) {
		ERROR("driver register failed, ret = %d\n", ret);
		goto driver_register_fail;
	}

	sema_init(&LcdResourceCnt, 1);
	sema_init(&disp_registration_lock, 1);
	sema_init(&disp_sema, 1);
	return ret;

driver_register_fail:
	i = MAX_DISP_DEVICE_NUM;

device_register_fail:
	i--;
	for(;i>=0;i--) {
		MSG("unregister disp%d\n", i);
		platform_device_unregister(&gDispWorkMem[i].plat_dev);
	}
	MSG("\n");
	return ret;
}

static void __exit gp_display_exit(void)
{
	int i;
	MSG("\n");

	platform_driver_unregister(&gp_display_driver);
	for(i = MAX_DISP_DEVICE_NUM - 1;i>=0;i--) {
		platform_device_unregister(&gDispWorkMem[i].plat_dev);
	}
	g_disp_init = 0;
	MSG("done\n");
}

static ssize_t disp_fops_write(struct file *pfile, const char __user *buf, size_t len, loff_t *pos)
{
	int ret = len;
	int i;
	pm_message_t state = {0};
	GP_DISP_WORKMEM *WorkMem = (GP_DISP_WORKMEM*)pfile->private_data;
	MSG("Enter\n");
	if(len > 256) len = 256;
	strncpy_from_user(WorkMem->file_buf, buf, len);

	for(i=0;i<len;i++) {
		char c = WorkMem->file_buf[i];
		if(c==' ' || c=='\n' ||c=='\t' ||c=='\r') {
			WorkMem->file_buf[i] = 0;
			break;
		}
	}
	if(strncmp(WorkMem->file_buf, "suspend", 8) == 0)
		display_suspend(&WorkMem->plat_dev, state);
	else if(strncmp(WorkMem->file_buf, "resume", 7) == 0)
		display_resume(&WorkMem->plat_dev);
	else if(strncmp(WorkMem->file_buf, "suspend-resume", 15) == 0) {
		//unsigned long tmp;
		//local_irq_save(tmp);
		display_suspend(&WorkMem->plat_dev, state);
		display_resume(&WorkMem->plat_dev);
		//local_irq_restore(tmp);
	}
	else if(strncmp(WorkMem->file_buf, "ShowBkReg", 10) == 0) {
		unsigned int *ptr = (unsigned int*)WorkMem->BackupReg;
		for(i=0;i<sizeof(WorkMem->BackupReg)/sizeof(long)/4;i++) {
			MSG("%08x %08x-%08x %08x\n", ptr[0], ptr[1], ptr[2], ptr[3]);
			ptr += 4;
		}
	}
	else if(WorkMem->DrvOp && WorkMem->DrvOp->Backdoor && WorkMem->hDev) {
		WorkMem->DrvOp->Backdoor(WorkMem->hDev, WorkMem->file_buf);
	}

	ret = len;

	return ret;
}

module_param(pll_sel, charp, S_IRUGO);

/* Declaration of the init and exit functions */
module_init(gp_display_init);
module_exit(gp_display_exit);

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP Display Driver");
MODULE_LICENSE_GP;
