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
 * @file gp_fb2.c
 * @brief FrameBuffer interface
 * @author Anson Chuang
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include <mach/module.h>
#include <mach/typedef.h>
#include <mach/gp_display2.h>
#include <mach/gp_chunkmem.h>
#include <mach/hal/hal_disp2.h>
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
struct gp_fb_info {
	gp_size_t panelRes;
	uint32_t fbsize;
	void __iomem *fbmem;
};

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static struct fb_info *fbinfo;
static char driver_name[] = "gp-fb2";

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
static int32_t
gp_fb_open(
	struct fb_info *info,
	int32_t user
)
{
	return 0;
}

static int32_t
gp_fb_release(
	struct fb_info *info,
	int32_t user
)
{
	return 0;
}

static int32_t
gp_fb_check_var(
	struct fb_var_screeninfo *var,
	struct fb_info *info
)
{
	if (var->bits_per_pixel != 32) {
		return -EINVAL;
	}
	return 0;
}

static int32_t
gp_fb_set_par(
	struct fb_info *info
)
{
	return 0;
}

#if 0
static int32_t
gp_fb_setcolreg(
	unsigned regno,
	unsigned red,
	unsigned green,
	unsigned blue,
	unsigned transp,
	struct fb_info *info
)
{
	printk("gp_fb_setcolreg(%d, %d, %d, %d, %d\n",
		regno, red, green, blue, transp);

	return 0;
}
#endif

static int32_t
gp_fb_pan_display(
	struct fb_var_screeninfo *var,
	struct fb_info *info
)
{
	//struct gp_fb_info *fbi = info->par;
	uint32_t addr;

	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;

	addr = fbinfo->fix.smem_start + var->yoffset * fbinfo->fix.line_length;
	/* Set frame address */
	disp2_set_pri_frame_addr(addr);

	disp2_update();
	disp2_wait_frame_end();
	return 0;
}

static void
gp_fb_activate(
	struct fb_info *info
)
{
	gp_bitmap_t priBitmap;

	/* Set primary bitmap */
	priBitmap.width = info->var.xres;
	priBitmap.height = info->var.yres;
	priBitmap.bpl = info->fix.line_length;
	priBitmap.type = SP_BITMAP_RGBA8888;
	priBitmap.pData = (uint8_t*) info->fix.smem_start;
	disp2_set_pri_bitmap(&priBitmap);

	/* Enable primary */
	disp2_set_pri_enable(1);

	disp2_update();
}

static struct fb_ops gp_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_open        = gp_fb_open,
	.fb_release     = gp_fb_release,
	.fb_check_var	= gp_fb_check_var,
	.fb_set_par	    = gp_fb_set_par,
#if 0
	.fb_setcolreg	= gp_fb_setcolreg,
#endif
	.fb_pan_display = gp_fb_pan_display,
};

static int32_t __init
gp_fb_init(
	void
)
{
	int32_t ret = 0;
	struct gp_fb_info *info;

	fbinfo = framebuffer_alloc(sizeof(struct gp_fb_info), NULL);
	if (!fbinfo)
		return -ENOMEM;

	info = fbinfo->par;

	/*Get panel resolution & allocate frame buffer (double buffer) */
	disp2_get_panel_res(&info->panelRes);
	info->fbsize = info->panelRes.width * info->panelRes.height * 4 * 2;
	info->fbmem = gp_chunk_malloc(info->fbsize);
	if (info->fbmem == NULL) {
		ret = -ENXIO;
		goto dealloc_fb;
	}

	memset(info->fbmem, 0x00, info->fbsize);

	fbinfo->fbops = &gp_fb_ops;
	fbinfo->flags = FBINFO_FLAG_DEFAULT;
	fbinfo->pseudo_palette = NULL;
	fbinfo->screen_base = info->fbmem;
	fbinfo->screen_size = 0;

	/* Resolution */
	fbinfo->var.xres = info->panelRes.width;
	fbinfo->var.yres = info->panelRes.height;
	fbinfo->var.xres_virtual = fbinfo->var.xres;
	fbinfo->var.yres_virtual = fbinfo->var.yres * 2;

	/* Timing */
	fbinfo->var.left_margin		= 0;
	fbinfo->var.right_margin	= 0;
	fbinfo->var.upper_margin	= 0;
	fbinfo->var.lower_margin	= 0;
	fbinfo->var.hsync_len		= 0;
	fbinfo->var.vsync_len		= 0;

	/* Color RGBA8888 */
	fbinfo->var.bits_per_pixel	= 32;
	fbinfo->var.grayscale		= 0;
	fbinfo->var.transp.offset	= 0;
	fbinfo->var.red.offset		= 24;
	fbinfo->var.green.offset	= 16;
	fbinfo->var.blue.offset		= 8;
	fbinfo->var.transp.length	= 8;
	fbinfo->var.red.length		= 8;
	fbinfo->var.green.length	= 8;
	fbinfo->var.blue.length		= 8;
    fbinfo->var.nonstd			= 0;

	fbinfo->var.activate = FB_ACTIVATE_FORCE;
	fbinfo->var.accel_flags = 0;
	fbinfo->var.vmode = FB_VMODE_NONINTERLACED;

	/* fixed info */
	memset(&fbinfo->fix.id, 0, sizeof(fbinfo->fix.id));
	strncpy(fbinfo->fix.id, driver_name, sizeof(driver_name));
	fbinfo->fix.mmio_start  = 0x93000000;
	fbinfo->fix.mmio_len    = 0x1000;
	fbinfo->fix.type        = FB_TYPE_PACKED_PIXELS;
	fbinfo->fix.type_aux	= 0;
	fbinfo->fix.visual      = FB_VISUAL_TRUECOLOR;
	fbinfo->fix.xpanstep	= 0;
	fbinfo->fix.ypanstep	= 1;
	fbinfo->fix.ywrapstep	= 0;
	fbinfo->fix.accel	    = FB_ACCEL_NONE;
	fbinfo->fix.smem_start  = gp_chunk_pa(info->fbmem);
	fbinfo->fix.smem_len    = info->fbsize;
	fbinfo->fix.line_length = (fbinfo->var.xres_virtual * fbinfo->var.bits_per_pixel) / 8;

	gp_fb_activate(fbinfo);

	ret = register_framebuffer(fbinfo);
	if (ret < 0) {
		printk(KERN_ERR "Failed to register framebuffer device: %d\n", ret);
		goto release_fbmem;
	}
	printk(KERN_INFO "fb%d: %s frame buffer device\n",
		fbinfo->node, fbinfo->fix.id);

	return 0;

release_fbmem:
	gp_chunk_free(info->fbmem);

dealloc_fb:
	framebuffer_release(fbinfo);

	return ret;
}

/*
 *  Cleanup
 */
static void __exit
gp_fb_exit(
	void
)
{
	struct gp_fb_info *info = fbinfo->par;

	gp_chunk_free(info->fbmem);
	unregister_framebuffer(fbinfo);
	framebuffer_release(fbinfo);
	fbinfo = NULL;

	return;
}

module_init(gp_fb_init);
module_exit(gp_fb_exit);

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP frame buffer 2 driver");
MODULE_LICENSE_GP;
