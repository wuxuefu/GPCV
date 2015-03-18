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

/*!
 * @file lcd_cpt_CLAP070LF01CW.c
 * @brief The lcd driver of CPT CLAP070LF01CW
 */
#include <linux/init.h>
#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */

#include <mach/panel_cfg.h>
#include <mach/hardware.h>
//#include <mach/regs-scu.h>
#include <mach/hal/hal_gpio.h>
#include <mach/hal/hal_clock.h>
#include <mach/hal/hal_disp.h>
#include <mach/gp_display.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/gp_board.h>
#include <mach/gp_panel.h>
#include <mach/module.h>

MODULE_LICENSE_GP;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int32_t lcd_init(void);
static int32_t lcd_suspend(void);
static int32_t lcd_resume(void);
static void* lcd_get_param(void);

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if 1
	#define DEBUG	printk
#else
	#define DEBUG(...)
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/



/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static const gp_disp_colormatrix_t gColorMatrix = {
	.a00 = 0x0100,
	.a01 = 0,
	.a02 = 0,
	.a10 = 0,
	.a11 = 0x0100,
	.a12 = 0,
	.a20 = 0,
	.a21 = 0,
	.a22 = 0x0100,
	.b0 = 0,
	.b1 = 0,
	.b2 = 0,
};

static panel_lcdInfo_t gPanelInfo = {
	.name = "panel_lcd_CLAP070NJ02CW", // MUST equal module name
	.workFreq    = 51000000, //37000000
	.clkPolatiry = 1,
	.resolution = {
		.width  = 1024,
		.height = 600,
	},
	.pixelPitch = {
		.width  = 1920,
		.height = 1080,
	},
	.format      = PANEL_FMT_RGB,
	.type        = PANEL_TYPE_PRGB888,
	.dataSeqEven = PANEL_SEQUENCE_PRGB888_RGB,
	.dataSeqOdd  = PANEL_SEQUENCE_PRGB888_RGB,
	.vsync = {
		.polarity = 0,
		.fPorch   = 12,
		.bPorch   = 20+3,
		.width    = 3,
	},
	.hsync = {
		.polarity = 0,
		.fPorch   = 160,
		.bPorch   = 140+20,
		.width    = 20,
	},
	.pColorMatrix = (gp_disp_colormatrix_t *) &gColorMatrix,
	.pGammaTable = {
		NULL,
		NULL,
		NULL,
	}
};

/* access functions */
static gp_disp_panel_ops_t lcd_fops = {
	.init = lcd_init,
	.suspend = lcd_suspend,
	.resume = lcd_resume,
	.get_param = lcd_get_param,
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern int32_t panel_common_init(uint32_t mode);
extern int32_t panel_common_suspend(uint32_t mode);
extern int32_t panel_common_resume(uint32_t mode);
extern int32_t panel_pin_drivingCurrent_setting(uint32_t drivingCurrent);
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int32_t
lcd_init(
	void
)
{
	int32_t ret;
	gp_board_panel_t *lcd_power;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	lcd_power = gp_board_get_config("panel", gp_board_panel_t);

	if (lcd_power != NULL && lcd_power->set_panelpowerOn0 != NULL)
		lcd_power->set_panelpowerOn0(1);		/* Power on VCC, VGL, AVDD, VGH */

	if (lcd_power != NULL && lcd_power->set_panelpowerOn1 != NULL)
		lcd_power->set_panelpowerOn1(1);		/* Set LCD RESET to high */	

	ret = panel_common_init(GID_PRGB888);
    panel_pin_drivingCurrent_setting(1); // using gpio 8mA output
	if (ret != 0) {
		return ret;
	}
	
	return ret;
}

static int32_t
lcd_suspend(
	void
)
{
	int32_t ret;
	gp_board_panel_t *lcd_power;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	lcd_power = gp_board_get_config("panel", gp_board_panel_t);

	ret = panel_common_suspend(GID_PRGB888);

	if (lcd_power != NULL && lcd_power->set_panelpowerOn1 != NULL)
		lcd_power->set_panelpowerOn1(0);		//LCD_RST to Low	

	if (lcd_power != NULL && lcd_power->set_panelpowerOn0 != NULL)
		lcd_power->set_panelpowerOn0(0);		//VGH, AVDD, VGL, VCC to low
	
	return ret;
}

static int32_t
lcd_resume(
	void
)
{
	int32_t ret;
	gp_board_panel_t *lcd_power;

	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	lcd_power = gp_board_get_config("panel", gp_board_panel_t);
	
	if (lcd_power != NULL && lcd_power->set_panelpowerOn0 != NULL)
		lcd_power->set_panelpowerOn0(1);		/* Power on VCC, VGL, AVDD, VGH */

	if (lcd_power != NULL && lcd_power->set_panelpowerOn1 != NULL)
		lcd_power->set_panelpowerOn1(1);		/* Set LCD RESET to high */	

	ret = panel_common_resume(GID_PRGB888);
    gpHalGpioSetPadGrp((0<<16)|(47<<8));

	return ret;
}

static void*
lcd_get_param(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return (void *)&gPanelInfo;
}

static int32_t
panel_init(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	register_paneldev(SP_DISP_OUTPUT_LCD, gPanelInfo.name, &lcd_fops);
	return 0;
}

static void
panel_exit(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	unregister_paneldev(SP_DISP_OUTPUT_LCD, gPanelInfo.name);
}

module_init(panel_init);
module_exit(panel_exit);
