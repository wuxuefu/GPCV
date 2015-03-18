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
/**
 * @file gp_display1.c
 * @brief Display1 interface, this driver is dedicated for TFT1
 * @author Anson Chuang
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/hal/hal_disp1.h>
#include <mach/hal/hal_ppu.h>
#include <mach/gp_board.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_display1.h>
#include <mach/hardware.h>
#include <mach/hal/regmap/reg_scu.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
enum display_mutex {
	DISP1_MUTEX_PRIMARY = 0,
	DISP1_MUTEX_OSD0,
	DISP1_MUTEX_OSD1,
	DISP1_MUTEX_OTHERS,
	DISP1_MUTEX_MAX,
};

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if 0
	#define DEBUG	printk
#else
	#define DEBUG(...)
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gp_disp1_buf_s {
	gp_disp_bufinfo_t info;	/*!< @brief Buffer info */
	void *ptr;				/*!< @brief Buffer address */
} gp_disp1_buf_t;

typedef struct gp_disp1_info_s
{
	struct miscdevice disp1_dev;
	int32_t state;
	struct semaphore sem[DISP1_MUTEX_MAX];

	/* Interrupt */
	int32_t intFlag;

	/* Panel */
	int32_t outputEnable;
	gp_size_t panelSize;
	gp_disp_output_t outputDev;

	/* Primary */
	uint32_t priEnable;
	gp_bitmap_t priBitmap;

	/* Dithering */
	uint32_t ditherEnable;
	uint32_t ditherType;
	gp_disp_ditherparam_t ditherParam;

	/* Gamma */
	uint32_t gammaEnable;
	uint8_t gammaTable[3][256];

	/* Buffer control */
	uint32_t mmap_enable;
	gp_disp1_buf_t dispBuf[GP_DISP_BUFFER_MAX];
} gp_disp1_info_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/



/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int32_t disp1_module_init(void);
static void disp1_module_exit(void);

static int32_t disp1_open(struct inode *inode, struct file *file);
static int32_t disp1_release(struct inode *inode, struct file *file);
static int32_t disp1_mmap(struct file *file, struct vm_area_struct *vma);
static long disp1_ioctl(struct file *file, uint32_t cmd, unsigned long arg);

static void disp1_panel_suspend(void);
static void disp1_panel_resume(void);


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_disp1_info_t *gpDisp1Info = NULL;
static DECLARE_WAIT_QUEUE_HEAD(disp1_fe_done);

/* Structure that declares the usual file */
/* access functions */
static struct file_operations disp1_fops = {
	open: disp1_open,
	release: disp1_release,
	mmap: disp1_mmap,
	unlocked_ioctl: disp1_ioctl
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
/**
 * \Send LCD SPI command
 */
int32_t
disp1_spi(
	uint32_t val,
	uint32_t bitsLen,
	uint32_t lsbFirst
)
{
	UINT32	i, j;
	UINT32	data;
	gp_board_panel_t *panel_spi;

	panel_spi = gp_board_get_config("panel", gp_board_panel_t);

#if	0
	printk("val=%04x, bitsLen=%d, lsbFirst=%d\n",val, bitsLen, lsbFirst);
#endif

	panel_spi->set_panel_spi_cs(1);
	panel_spi->set_panel_spi_scl(1);
	panel_spi->set_panel_spi_sda(1);

	if ( lsbFirst ) {
		data = val;
	}
	else {
		data = 0;
		for ( i = 0; i < 24; i++ ) {
			if ( val & 0x800000 ) {
				data |= (0x0001 << i);
			}
			val <<= 1;
		}
		data >>= (24 - bitsLen);
	}

	panel_spi->set_panel_spi_cs(0);
	for ( i = 0; i < bitsLen; i++ ) {
		panel_spi->set_panel_spi_scl(0);
		if ( data & 0x0001 ) {
			panel_spi->set_panel_spi_sda(1);
		}
		else {
			panel_spi->set_panel_spi_sda(0);
		}
		for ( j = 0; j < 3; j++ );
		panel_spi->set_panel_spi_scl(1);
		data >>= 1;
		for ( j = 0; j < 3; j++ );

	}
	panel_spi->set_panel_spi_cs(1);

	return 0;
}
EXPORT_SYMBOL(disp1_spi);

/**
 * \brief Muxtex lock in ioctl
 */
static uint32_t
disp1_mux_get_id(
	uint32_t cmd
)
{
	uint32_t id = cmd & 0x00f0;

	if (id == 0x10)
		return DISP1_MUTEX_PRIMARY;
	else if (id == 0x80)
		return DISP1_MUTEX_OSD0;
	else if (id == 0x90)
		return DISP1_MUTEX_OSD1;
	else
		return DISP1_MUTEX_OTHERS;
}

/**
 * \brief Muxtex lock in ioctl
 */
static uint32_t
disp1_mux_lock(
	uint32_t cmd
)
{
	uint32_t id = disp1_mux_get_id(cmd);

	if (down_interruptible(&gpDisp1Info->sem[id]) != 0) {
		return -ERESTARTSYS;
	}

	return 0;
}

/**
 * \brief Muxtex unlock in ioctl
 */
static void
disp1_mux_unlock(
	uint32_t cmd
)
{
	uint32_t id = disp1_mux_get_id(cmd);

	up(&gpDisp1Info->sem[id]);
}

/**
 * \brief Display device update parameter
 */
void
disp1_update(
	void
)
{
	/* FIX ME */
	gpHalDisp1UpdateParameter();
	//disp1_wait_frame_end();
}
EXPORT_SYMBOL(disp1_update);

/**
 * \brief Display wait frame end
 */
int32_t
disp1_wait_frame_end(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);

	if (gpHalDisp1GetEnable() == 0)
		return -1;

	/* Enable frame end interrupt */
	gpHalDisp1ClearIntFlag(HAL_DISP1_INT_TFT_VBLANK);
	gpDisp1Info->intFlag = 0;
	gpHalDisp1SetIntEnable(HAL_DISP1_INT_TFT_VBLANK);

   	wait_event_interruptible(disp1_fe_done, gpDisp1Info->intFlag != 0);

	return 0;
}
EXPORT_SYMBOL(disp1_wait_frame_end);

/**
 * \brief Display get format and type of primary layer
 */
void
disp1_set_pri_enable(
	uint32_t enable
)
{
	gpDisp1Info->priEnable = enable;
	gpHalDisp1SetEnable(enable);
}
EXPORT_SYMBOL(disp1_set_pri_enable);

/**
 * \brief Display get format and type of primary layer
 */
static uint32_t
disp1_set_pri_fmt_type(
	uint8_t srcType
)
{
	switch (srcType) {
		case SP_BITMAP_RGBA8888:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(2);
			break;

		case SP_BITMAP_RGB565:
			gpHalPPUSetFbFormat(0);
			gpHalPPUSetFbMono(0);
			break;

		case SP_BITMAP_YUYV:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(1);
			gpHalPPUSetYuvType(3);
			break;

		case SP_BITMAP_YVYU:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(1);
			gpHalPPUSetYuvType(1);
			break;

		case SP_BITMAP_UYVY:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(1);
			gpHalPPUSetYuvType(2);
			break;

		case SP_BITMAP_VYUY:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(1);
			gpHalPPUSetYuvType(0);
			break;

		case SP_BITMAP_BGRG:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(0);
			gpHalPPUSetYuvType(0);
			break;

		case SP_BITMAP_GBGR:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(0);
			gpHalPPUSetYuvType(1);
			break;

		case SP_BITMAP_RGBG:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(0);
			gpHalPPUSetYuvType(2);
			break;

		case SP_BITMAP_GRGB:
			gpHalPPUSetFbFormat(1);
			gpHalPPUSetFbMono(0);
			gpHalPPUSetYuvType(3);
			break;

		default:
			printk("[%s:%d] Error! Unkonwn type\n", __FUNCTION__, __LINE__);
			return -1;
			break;
	}

	return 0;
}

/**
 * \brief Display set primary bitmap
 */
int32_t
disp1_set_pri_bitmap(
	gp_bitmap_t *pbitmap
)
{
	memcpy(&gpDisp1Info->priBitmap, pbitmap, sizeof(gp_bitmap_t));
	DEBUG("[%s:%d], DISP1_IO_SET_PRI_BITMAP, width=%d, height=%d, bpl=%d, type%d, addr=0x%x\n",
		__FUNCTION__, __LINE__, pbitmap->width, pbitmap->height, pbitmap->bpl, pbitmap->type, (uint32_t) pbitmap->pData);

	if (disp1_set_pri_fmt_type(pbitmap->type) < 0)
		return -1;

	gpHalDisp1SetPriFrameAddr((uint32_t) pbitmap->pData); /* Set Frame Address */

	return 0;
}
EXPORT_SYMBOL(disp1_set_pri_bitmap);


/**
 * \brief Display get format and type of primary layer
 */
void
disp1_set_pri_frame_addr(
	uint32_t addr
)
{
	gpDisp1Info->priBitmap.pData = (uint8_t*)addr;
	gpHalDisp1SetPriFrameAddr((uint32_t) addr); /* Set Frame Address */
}
EXPORT_SYMBOL(disp1_set_pri_frame_addr);

/**
 * \brief Switch panel
 */
static int32_t
disp1_switch_panel(
	void
)
{
	int32_t ret;

	/* Is panel available? */
	if (gpDisp1Info->outputDev.ops == NULL ||
		gpDisp1Info->outputDev.ops->init == NULL) {

		DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
		return -EIO;
	}

	/* Turn on panel clock & set parameters */
	ret = (gpDisp1Info->outputDev.ops->init)();
	if (ret < 0) {
		return ret;
	}

	/* Set data path to TFT1 or TV1 */
	if (gpDisp1Info->outputDev.type == SP_DISP_OUTPUT_TV) {
		SCUA_SAR_GPIO_CTRL |= 0x21;
	}
	else {
		SCUA_SAR_GPIO_CTRL |= 0x02;
	}

	/* Init global variable */
	disp1_get_panel_res(&gpDisp1Info->panelSize);

	/* Disable primary  layer */
	disp1_set_pri_enable(0);

	disp1_update();

	disp1_wait_frame_end();

	gpDisp1Info->state = DISP_STATE_RESUME;
	return 0;
}

/**
 * \brief Display device initial
 */
static int32_t
disp1_init(
	void
)
{
	/* If current output index exist, then disable controller */
	if (gpDisp1Info->outputEnable) {
		/* Disable primary layer */
		disp1_set_pri_enable(0);
		disp1_update();
		return 0;
	}

	/* Initial output device */
	gpDisp1Info->outputEnable = 1;
	return disp1_switch_panel();
}


/**
 * \brief Display get format and type of primary layer
 */
void
disp1_get_panel_res(
	gp_size_t *res
)
{
	gpHalDisp1GetRes(&res->width, &res->height);
}
EXPORT_SYMBOL(disp1_get_panel_res);

/**
 * \brief Display device set dithering enable
 */
void
disp1_set_dither_enable(
	uint32_t enable
)
{
	gpDisp1Info->ditherEnable = enable;
	gpHalDisp1SetDitherEnable(enable);
}
EXPORT_SYMBOL(disp1_set_dither_enable);

/**
 * \brief Display device set dithering type
 */
void
disp1_set_dither_type(
	uint32_t type
)
{
	gpDisp1Info->ditherType = type;
	gpHalDisp1SetDitherType(type);
}
EXPORT_SYMBOL(disp1_set_dither_type);

/**
 * \brief Display device set dithering parameter
 */
void
disp1_set_dither_param(
	gp_disp_ditherparam_t *pDitherParam
)
{
	/* FIX ME */
	uint32_t map0, map1, map2, map3;
	memcpy(&gpDisp1Info->ditherParam, pDitherParam, sizeof(gp_disp_ditherparam_t));

	map0 = ((pDitherParam->d00 & 0xF) << 12 |
			(pDitherParam->d01 & 0xF) << 8 |
			(pDitherParam->d02 & 0xF) << 4 |
			(pDitherParam->d03 & 0xF));

	map1 = ((pDitherParam->d10 & 0xF) << 12 |
			(pDitherParam->d11 & 0xF) << 8 |
			(pDitherParam->d12 & 0xF) << 4 |
			(pDitherParam->d13 & 0xF));

	map2 = ((pDitherParam->d20 & 0xF) << 12 |
			(pDitherParam->d21 & 0xF) << 8 |
			(pDitherParam->d22 & 0xF) << 4 |
			(pDitherParam->d23 & 0xF));

	map3 = ((pDitherParam->d30 & 0xF) << 12 |
			(pDitherParam->d31 & 0xF) << 8 |
			(pDitherParam->d32 & 0xF) << 4 |
			(pDitherParam->d33 & 0xF));

	gpHalDisp1SetDitherMap(map0, map1, map2, map3);
}
EXPORT_SYMBOL(disp1_set_dither_param);

/**
 * \brief Display device set gamma table
 */
void
disp1_set_gamma_enable(
	uint32_t enable
)
{
	gpDisp1Info->gammaEnable = enable;
	gpHalDisp1SetGammaEnable(enable);
}
EXPORT_SYMBOL(disp1_set_gamma_enable);

/**
 * \brief Display device set gamma table
 */
void
disp1_set_gamma_table(
	uint32_t id,
	uint8_t *pTable
)
{
	memcpy(&gpDisp1Info->gammaTable[id], pTable, sizeof(uint8_t) * 256);
	gpHalDisp1SetGammaTable(id, (uint8_t *) &gpDisp1Info->gammaTable[id]);
}
EXPORT_SYMBOL(disp1_set_gamma_table);

/**
 * \brief Display device allocate buffer
 */
void*
disp1_allocate_buffer(
	gp_disp_bufinfo_t info
)
{
	void *ptr;
	if ((info.id >= GP_DISP_BUFFER_MAX) || gpDisp1Info->dispBuf[info.id].ptr) {
		/* Occupied */
		printk("[%s:%d] Fail, id=%d\n", __FUNCTION__, __LINE__, info.id);
		return NULL;
	}
	else {
		/* Allocate buffer */
		ptr = gp_chunk_malloc(info.size);
		if (ptr == NULL) {
			return NULL;
		}
		gpDisp1Info->dispBuf[info.id].info = info;
		gpDisp1Info->dispBuf[info.id].ptr = ptr;
	}

	return ptr;
}
EXPORT_SYMBOL(disp1_allocate_buffer);

/**
 * \brief Display device free buffer
 */
int32_t
disp1_free_buffer(
	uint32_t id
)
{
	if ((id >= GP_DISP_BUFFER_MAX)) {
		printk("[%s:%d] Fail, id=%d\n", __FUNCTION__, __LINE__, id);
		return -1;
	}
	else {
		/* Free buffer */
		gp_chunk_free(gpDisp1Info->dispBuf[id].ptr);
		gpDisp1Info->dispBuf[id].ptr = NULL;
	}

	return 0;
}
EXPORT_SYMBOL(disp1_free_buffer);

/**
 * \brief Panel suspend
 */
static void
disp1_panel_suspend(
	void
)
{
	if (gpDisp1Info->outputDev.ops->suspend)
		(gpDisp1Info->outputDev.ops->suspend)();

	return;
}

/**
 * \brief Panel resume
 */
static void
disp1_panel_resume(
	void
)
{
	if (gpDisp1Info->outputDev.ops->resume)
		(gpDisp1Info->outputDev.ops->resume)();

	/* Restore display parameters */
	/* Primary */
	if (gpDisp1Info->priEnable) {
		disp1_set_pri_bitmap(&gpDisp1Info->priBitmap);
	}
	disp1_set_pri_enable(gpDisp1Info->priEnable);

	/* Dither */
	if (gpDisp1Info->ditherEnable) {
		disp1_set_dither_type(gpDisp1Info->ditherType);
		disp1_set_dither_enable(gpDisp1Info->ditherEnable);
	}

	/* Gamma */
	if (gpDisp1Info->gammaEnable) {
		disp1_set_gamma_table(SP_DISP_GAMMA_R, (uint8_t*) &gpDisp1Info->gammaTable[SP_DISP_GAMMA_R]);
		disp1_set_gamma_table(SP_DISP_GAMMA_G, (uint8_t*) &gpDisp1Info->gammaTable[SP_DISP_GAMMA_G]);
		disp1_set_gamma_table(SP_DISP_GAMMA_B, (uint8_t*) &gpDisp1Info->gammaTable[SP_DISP_GAMMA_B]);
		disp1_set_gamma_enable(gpDisp1Info->gammaEnable);
	}

	disp1_update();

	if (gpDisp1Info->outputDev.ops->resume_post)
		(gpDisp1Info->outputDev.ops->resume_post)();

	return;
}

/**
 * \brief Panel driver register
 */
int32_t
register_paneldev1(
	uint32_t panelType,
	char *name,
	gp_disp_panel_ops_t *panelOps
)
{
	printk("[%s:%d], type=%d, name=%s\n", __FUNCTION__, __LINE__, panelType, name);

	if ((panelType != SP_DISP_OUTPUT_LCD) && (panelType != SP_DISP_OUTPUT_LCM)) {
		printk("[%s:%d], error, disp1 only support LCD or LCM\n", __FUNCTION__, __LINE__);
		return -1;
	}

	gpDisp1Info->outputDev.type = panelType;

	memset(gpDisp1Info->outputDev.name, 0, sizeof(gpDisp1Info->outputDev.name));
	strncpy(gpDisp1Info->outputDev.name, name, strlen(name));

	gpDisp1Info->outputDev.ops = panelOps;

	return 0;
}
EXPORT_SYMBOL(register_paneldev1);

/**
 * \brief Panel driver unregister
 */
int32_t
unregister_paneldev1(
	uint32_t panelType,
	char *name
)
{
	printk("[%s:%d], type=%d, name=%s\n", __FUNCTION__, __LINE__, panelType, name);

	if ((panelType != SP_DISP_OUTPUT_LCD) || (panelType != SP_DISP_OUTPUT_LCM)) {
		printk("[%s:%d], error, disp1 only support LCD or LCM\n", __FUNCTION__, __LINE__);
		return -1;
	}

	gpDisp1Info->outputDev.ops = NULL;

	return 0;
}
EXPORT_SYMBOL(unregister_paneldev1);


static irqreturn_t
disp1_irq(
	int32_t irq,
	void *dev_id
)
{
	if ((gpHalDisp1GetIntStatus() & HAL_DISP1_INT_TFT_VBLANK) == 0) {
		return IRQ_NONE;
	}

	/* Disable frame end interrupt & clear flag */
	gpHalDisp1SetIntDisable(HAL_DISP1_INT_TFT_VBLANK);
	gpHalDisp1ClearIntFlag(HAL_DISP1_INT_TFT_VBLANK);

	if (gpDisp1Info->intFlag == 0) {
		gpDisp1Info->intFlag = 1;
		wake_up_interruptible(&disp1_fe_done);
	}
	return IRQ_HANDLED;
}

/**
 * \brief Open display device
 */
static int32_t
disp1_open(
	struct inode *inode,
	struct file *file
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
  SCUA_SAR_GPIO_CTRL |= 0x2;
  
	/* Success */
	return 0;
}

/**
 * \brief Release display device
 */
static int32_t
disp1_release(
	struct inode *inode,
	struct file *file
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
  SCUA_SAR_GPIO_CTRL &= ~0x2;
	/* Success */
	return 0;
}

/**
 * \brief mmap of display device
 */
static int32_t
disp1_mmap(
	struct file *file,
	struct vm_area_struct *vma
)
{
	int ret;

	if (!gpDisp1Info->mmap_enable) {
		ret = -EPERM; /* disable calling mmap from user AP */
		goto out;
	}

	/* This is an IO map - tell maydump to skip this VMA */
	vma->vm_flags |= VM_IO | VM_RESERVED;
	ret = io_remap_pfn_range(vma,
							 vma->vm_start,
							 vma->vm_pgoff,
							 vma->vm_end - vma->vm_start,
							 vma->vm_page_prot);
	if (ret != 0) {
		ret = -EAGAIN;
	}
out:
	return ret;
}

/**
 * \brief Ioctl of display1 device
 */
static long
disp1_ioctl(
	struct file *file,
	uint32_t cmd,
	unsigned long arg
)
{
	long err = 0;

	DEBUG("[%s:%d], cmd = 0x%x\n", __FUNCTION__, __LINE__, cmd);

	/* mutex lock (primary, osd0, osd1, others) */
	if (disp1_mux_lock(cmd)) {
		return -ERESTARTSYS;
	}

	switch (cmd) {
		case DISPIO_SET_INITIAL:
			err = disp1_init();
			break;

		case DISPIO_SET_UPDATE:
			disp1_update();
			break;

		case DISPIO_GET_PANEL_RESOLUTION:
			copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->panelSize, sizeof(gp_disp_res_t));
			break;

		case DISPIO_GET_PANEL_SIZE:
			{
				gp_size_t size;
				if (gpDisp1Info->outputDev.ops->get_size) {
					(gpDisp1Info->outputDev.ops->get_size)(&size);
					copy_to_user ((void __user *) arg, (const void *) &size, sizeof(gp_size_t));
				}
				else {
					err = -EIO;
				}
			}
			break;

		case DISPIO_SET_SUSPEND:
			gpDisp1Info->state = DISP_STATE_SUSPEND;
			disp1_panel_suspend();
			break;

		case DISPIO_SET_RESUME:
			gpDisp1Info->state = DISP_STATE_RESUME;
			disp1_panel_resume();
			break;

		case DISPIO_SET_BACKLIGHT:
			{
				gp_board_panel_t *panel_config;
				panel_config = gp_board_get_config("panel", gp_board_panel_t);
				if (panel_config == NULL || panel_config->set_backlight == NULL) {
					printk("Panel backlight not found!\n");
				}
				else {
					panel_config->set_backlight((uint32_t) arg);
				}
			}
			break;

		case DISPIO_WAIT_FRAME_END:
			disp1_wait_frame_end();
			break;

		/* Primary layer */
		case DISPIO_SET_PRI_ENABLE:
			DEBUG("[%s:%d], DISPIO_SET_PRI_ENABLE = %d\n", __FUNCTION__, __LINE__, (uint32_t) arg);
			gpDisp1Info->priEnable = (uint32_t) arg;
			disp1_set_pri_enable((uint32_t) arg);
			break;

		case DISPIO_GET_PRI_ENABLE:
			DEBUG("[%s:%d], DISPIO_GET_PRI_ENABLE = %d\n", __FUNCTION__, __LINE__, gpDisp1Info->priEnable);
			copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->priEnable, sizeof(uint32_t));
			break;

		case DISPIO_SET_PRI_BITMAP:
			{
				gp_bitmap_t bitmap;

				copy_from_user((void*) &bitmap, (const void __user *) arg, sizeof(gp_bitmap_t));
				bitmap.pData = (uint8_t*) gp_user_va_to_pa(bitmap.pData);
				if (disp1_set_pri_bitmap(&bitmap) < 0) {
					err = -EIO;
				}
			}
			break;

		//sz modify , for double buffer switch
		case DISPIO_CHANGE_PRI_BITMAP_BUF:
			{
				gp_bitmap_t bitmap;

				copy_from_user((void*) &bitmap, (const void __user *) arg, sizeof(gp_bitmap_t));
				bitmap.pData = (uint8_t*) gp_user_va_to_pa(bitmap.pData);
				disp1_set_pri_frame_addr((uint32_t)bitmap.pData);
			}
			break;
		//-----------------------------------

		case DISPIO_GET_PRI_BITMAP:
			copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->priBitmap, sizeof(gp_bitmap_t));
			break;

		/* Dithering */
		case DISPIO_SET_DITHER_ENABLE:
			disp1_set_dither_enable((uint32_t) arg);
			break;

		case DISPIO_GET_DITHER_ENABLE:
			copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->ditherEnable, sizeof(uint32_t));
			break;

		case DISPIO_SET_DITHER_TYPE:
			disp1_set_dither_type((uint32_t) arg);
			break;

		case DISPIO_GET_DITHER_TYPE:
			copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->ditherType, sizeof(uint32_t));
			break;

		case DISPIO_SET_DITHER_PARAM:
			{
				gp_disp_ditherparam_t ditherParam;
				copy_from_user((void*) &ditherParam, (const void __user *) arg, sizeof(gp_disp_ditherparam_t));
				disp1_set_dither_param((gp_disp_ditherparam_t *) &ditherParam);
			}
			break;

		case DISPIO_GET_DITHER_PARAM:
			copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->ditherParam, sizeof(gp_disp_ditherparam_t));
			break;

		/* Gamma table */
		case DISPIO_SET_GAMMA_ENABLE:
			disp1_set_gamma_enable((uint32_t) arg);
			break;

		case DISPIO_GET_GAMMA_ENABLE:
			copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->gammaEnable, sizeof(uint32_t));
			break;

		case DISPIO_SET_GAMMA_PARAM:
			{
				gp_disp_gammatable_t *pGammaTable;
				pGammaTable = kmalloc(sizeof(gp_disp_gammatable_t), GFP_KERNEL);
				if (!pGammaTable) {
					err = -ENOMEM;
					break;
				}
				copy_from_user((void*) pGammaTable, (const void __user *) arg, sizeof(gp_disp_gammatable_t));
				DEBUG("Set gamma parameter, id=%d\n", pGammaTable->id);
				disp1_set_gamma_table(pGammaTable->id, (uint8_t*) &pGammaTable->table);
				kfree(pGammaTable);
			}
			break;

		case DISPIO_GET_GAMMA_PARAM:
			{
				gp_disp_gammatable_t *pGammaTable = (gp_disp_gammatable_t *)arg;
				uint32_t id;

				copy_from_user((void*) &id, (const void __user *) &pGammaTable->id, sizeof(uint32_t));
				DEBUG("Get gamma parameter, id=%d\n", id);
				copy_to_user ((void __user *) &pGammaTable->table, (const void *) gpDisp1Info->gammaTable[id], sizeof(uint8_t) * 256);
			}
			break;


		/* Buffer control */
		case DISPIO_BUF_ALLOC:
			{
				gp_disp_bufinfo_t info;

				copy_from_user((void*) &info, (const void __user *) arg, sizeof(gp_disp_bufinfo_t));
				DEBUG("[%s:%d], DISP1_IO_BUF_ALLOC, id=%d, width=%d, height=%d, bpp=%d, size=%d\n",
					__FUNCTION__, __LINE__, info.id, info.width, info.height, info.bpp, info.size);

				if (disp1_allocate_buffer(info) == NULL) {
					err = -EIO;
					break;
				}
			}
			break;

		case DISPIO_BUF_FREE:
			{
				gp_disp_bufinfo_t info;

				copy_from_user((void*) &info, (const void __user *) arg, sizeof(gp_disp_bufinfo_t));

				if (disp1_free_buffer(info.id) < 0) {
					printk("DISPIO_BUF_FREE Fail, id=%d\n", info.id);
					err = -EIO;
					break;
				}
			}
			break;

		case DISPIO_BUF_MMAP:
			{
				unsigned long va;
				gp_disp_bufaddr_t bufaddr;
				uint32_t size;
				uint32_t pa;

				copy_from_user((void*) &bufaddr, (const void __user *) arg, sizeof(gp_disp_bufaddr_t));
				size = gpDisp1Info->dispBuf[bufaddr.id].info.size;
				pa = gp_chunk_pa(gpDisp1Info->dispBuf[bufaddr.id].ptr);

				down_write(&current->mm->mmap_sem);
				gpDisp1Info->mmap_enable = 1; /* enable mmap in DISP1_IO_BUF_MMAP */
				va = do_mmap_pgoff(
					file, 0, size,
					PROT_READ|PROT_WRITE,
					MAP_SHARED,
					pa >> PAGE_SHIFT);
				gpDisp1Info->mmap_enable = 0; /* disable it */
				up_write(&current->mm->mmap_sem);

				bufaddr.ptr = (void *)va;
				copy_to_user ((void __user *) arg, (const void *) &bufaddr, sizeof(gp_disp_bufaddr_t));
			}
			break;

		case DISPIO_BUF_MUNMAP:
			{
				gp_disp_bufaddr_t bufaddr;
				uint32_t size;

				copy_from_user((void*) &bufaddr, (const void __user *) arg, sizeof(gp_disp_bufaddr_t));
				size = gpDisp1Info->dispBuf[bufaddr.id].info.size;

				down_write(&current->mm->mmap_sem);
				do_munmap(current->mm, (unsigned int)bufaddr.ptr, size);
				up_write(&current->mm->mmap_sem);
			}
			break;

		case DISPIO_BUF_GETINFO:
			{
				gp_disp_bufinfo_t info;

				copy_from_user((void*) &info, (const void __user *) arg, sizeof(gp_disp_bufinfo_t));

				if ((info.id >= GP_DISP_BUFFER_MAX) || (gpDisp1Info->dispBuf[info.id].ptr == NULL)) {
					err = -EIO;
					break;
				}
				else {
					copy_to_user ((void __user *) arg, (const void *) &gpDisp1Info->dispBuf[info.id].info, sizeof(gp_disp_bufinfo_t));
				}
			}
			break;

		default:
			DEBUG("[%s:%d], unknow cmd\n", __FUNCTION__, __LINE__);
			break;
	}

	/* mutex unlock */
	disp1_mux_unlock(cmd);
	return err;
}

/**
 * @brief display device release
 */
static void
disp1_device_release(
	struct device *dev
)
{
	printk("remove display1 device ok\n");
}

static struct platform_device disp1_device = {
	.name	= "gp-disp1",
	.id		= 0,
	.dev	= {
		.release = disp1_device_release,
	},
};

#ifdef CONFIG_PM
static int
disp1_suspend(
	struct platform_device *pdev,
	pm_message_t state
)
{
	/* Panel suspend */
	gpDisp1Info->state = DISP_STATE_SUSPEND;
	disp1_panel_suspend();

	return 0;
}

static int
disp1_resume(
	struct platform_device *pdev
)
{
	/* Panel Resume */
	gpDisp1Info->state = DISP_STATE_RESUME;
	disp1_panel_resume();

	return 0;
}
#else
#define disp1_suspend NULL
#define disp1_resume NULL
#endif

/**
 * @brief display driver define
 */
static struct platform_driver disp1_driver = {
	.suspend = disp1_suspend,
	.resume = disp1_resume,
	.driver = {
		.owner = THIS_MODULE,
		.name = "gp-disp1"
	}
};

/**
 * \brief Initialize display device
 */
static int32_t __init
disp1_module_init(
	void
)
{
	int32_t retval;
	int32_t i;

	DEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	/* Initial display hal */
	if (gpHalDisp1Init() != 0) {
		DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
		retval = -EIO;
		goto fail_init;
	}

	/* malloc */
	gpDisp1Info = kmalloc(sizeof(gp_disp1_info_t), GFP_KERNEL);
	if (!gpDisp1Info) {
		printk("[%s:%d], Error\n", __FUNCTION__, __LINE__);
		retval = -ENOMEM;
		goto fail_malloc;
	}
	memset(gpDisp1Info, 0, sizeof(gp_disp1_info_t));
	gpDisp1Info->state = DISP_STATE_SUSPEND;

	/* irq request */
	retval = request_irq(IRQ_PPU, disp1_irq, IRQF_SHARED, "PPU_IRQ", (void *) gpDisp1Info);
	if (retval) {
		printk("[%s:%d], request_irq error %d\n", __FUNCTION__, __LINE__, retval);
		goto fail_irq;
	}

	/* init mutex */
	for (i=0; i<DISP1_MUTEX_MAX; i++) {
		init_MUTEX(&gpDisp1Info->sem[i]);
	}

	/* Registering device */
	gpDisp1Info->disp1_dev.minor = MISC_DYNAMIC_MINOR;
	gpDisp1Info->disp1_dev.name = "disp1";
	gpDisp1Info->disp1_dev.fops = &disp1_fops;
	retval = misc_register(&gpDisp1Info->disp1_dev);
	if (retval) {
		goto fail_register;
	}

	printk("disp1 dev minor : %i\n", gpDisp1Info->disp1_dev.minor);

	platform_device_register(&disp1_device);
	return platform_driver_register(&disp1_driver);

fail_register:
	free_irq(IRQ_PPU, (void *)gpDisp1Info);
fail_irq:
	kfree(gpDisp1Info);
	gpDisp1Info = NULL;
fail_malloc:
fail_init:
	return retval;
}

/**
 * \brief Exit display device
 */
static void __exit
disp1_module_exit(
	void
)
{
	DEBUG("%s:%d\n", __FUNCTION__, __LINE__);

	if (gpDisp1Info->state == DISP_STATE_RESUME) {
		gpDisp1Info->state = DISP_STATE_SUSPEND;
		disp1_panel_suspend();
	}

	/* Freeing the major number */
	misc_deregister(&gpDisp1Info->disp1_dev);
	free_irq(IRQ_PPU, (void *)gpDisp1Info);
	kfree(gpDisp1Info);
	gpDisp1Info = NULL;

	platform_device_unregister(&disp1_device);
	platform_driver_unregister(&disp1_driver);

	DEBUG("Removing disp1 module\n");
}


/* Declaration of the init and exit functions */
module_init(disp1_module_init);
module_exit(disp1_module_exit);

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP Display1 Driver");
MODULE_LICENSE_GP;
