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
#include <mach/gp_display.h>
#include <mach/gp_chunkmem.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define	FBBUFFER_NUM 2

#define ERROR(fmt, arg...) printk(KERN_ERR "[%s:%d] Error! "fmt, __FUNCTION__, __LINE__, ##arg)
#define WARNING(fmt, arg...) printk(KERN_WARNING "[%s:%d] Warning! "fmt, __FUNCTION__, __LINE__, ##arg)
#define MSG(fmt, arg...) printk(KERN_DEBUG "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)
#define INFO(fmt, arg...) printk(KERN_INFO "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)

#define RETURN(x)		{ret = x; goto Return;}
#define CHECK_(x, msg, errid) if(!(x)) {ERROR("%s, %s\n", msg, #x); RETURN(errid);}
#define CHECK_PRG(x)	CHECK_(x, "Program Error", -EIO)
#define CHECK_VAL(x)	CHECK_(x, "Value Error", -1)
#define CHECK(x)		{int ret = (x); if(ret < 0) {printk("[%s:%d] Check Failed! %s\n", __FUNCTION__, __LINE__, #x); RETURN(ret);}}

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct {
	char ID[8];
	struct fb_info *fb;

	int ColorFmt;

	void *mainDisp;
	gp_size_t panelRes;
	gp_size_t UIRes;
	int fbsize;
	void __iomem *fbmem;
} GPFB_WORKMEM;

typedef struct {
	const char *name;
	uint32_t bits_per_pixel;
	uint32_t nonstd;
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
} GP_COLORMODE;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static char *main_disp = "";
static char *color_fmt = "RGB565";

static char driver_name[] = "gp-fb";
static struct fb_info *gFrameBufInfo = 0;

static const GP_COLORMODE ColorModeTab[] = {
	[SP_BITMAP_RGB565] = {
		.name = "RGB565",
	    .bits_per_pixel = 16,
	    .red    = { .length = 5, .offset = 11, .msb_right = 0 },
	    .green  = { .length = 6, .offset = 5, .msb_right = 0 },
	    .blue   = { .length = 5, .offset = 0, .msb_right = 0 },
	    .transp = { .length = 0, .offset = 0, .msb_right = 0 },
	},
	[SP_BITMAP_ARGB1555] = {
		.name = "ARGB1555",
		.bits_per_pixel = 16,
		.red	= { .length = 5, .offset = 10, .msb_right = 0 },
		.green	= { .length = 5, .offset = 5, .msb_right = 0 },
		.blue	= { .length = 5, .offset = 0, .msb_right = 0 },
		.transp	= { .length = 1, .offset = 0, .msb_right = 0 },
	},
	[SP_BITMAP_ABGR8888] = {
		.name = "ABGR8888",
		.bits_per_pixel = 32,
		.red	= { .length = 8, .offset = 0, .msb_right = 0 },
		.green	= { .length = 8, .offset = 8, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 16, .msb_right = 0 },
		.transp	= { .length = 8, .offset = 24, .msb_right = 0 },
	},
	[SP_BITMAP_ARGB8888] = {
		.name = "ARGB8888",
		.bits_per_pixel = 32,
		.red	= { .length = 8, .offset = 16, .msb_right = 0 },
		.green	= { .length = 8, .offset = 8, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 0, .msb_right = 0 },
		.transp	= { .length = 8, .offset = 24, .msb_right = 0 },
	},
	[SP_BITMAP_RGBA8888] = {
		.name = "RGBA8888",
		.bits_per_pixel = 32,
		.red	= { .length = 8, .offset = 24, .msb_right = 0 },
		.green	= { .length = 8, .offset = 16, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 8, .msb_right = 0 },
		.transp	= { .length = 8, .offset = 0, .msb_right = 0 },
	},
	[SP_BITMAP_BGRA8888] = {
		.name = "BGRA8888",
		.bits_per_pixel = 32,
		.red	= { .length = 8, .offset = 8, .msb_right = 0 },
		.green	= { .length = 8, .offset = 16, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 24, .msb_right = 0 },
		.transp	= { .length = 8, .offset = 0, .msb_right = 0 },
	},
};

gp_disp_osdalpha_t mainDispAlpha = {
	.consta = SP_DISP_ALPHA_PERPIXEL,
	.ppamd  = SP_DISP_ALPHA_PERPIXEL_ONLY,
	.alpha  = 100,
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
inline static void dss_mode_to_fb_mode(int dssmode, struct fb_var_screeninfo *var)
{
	const GP_COLORMODE *colormode = ColorModeTab + dssmode;
	MSG("color mode = <%s>\n", colormode->name);
	var->bits_per_pixel	= colormode->bits_per_pixel;
	var->nonstd			= colormode->nonstd;
	var->red			= colormode->red;
	var->green			= colormode->green;
	var->blue			= colormode->blue;
	var->transp			= colormode->transp;
}

static void mainDisp_Release(struct fb_info *fbinfo)
{
	GPFB_WORKMEM *WorkMem = fbinfo->par;
	MSG("\n");
	disp_close(WorkMem->mainDisp, 1);
	WorkMem->mainDisp = 0;
}

static void mainDisp_Init(struct fb_info *fbinfo)
{
	gp_bitmap_t Bitmap;
	gp_disp_scale_t Scale;

	GPFB_WORKMEM *WorkMem = fbinfo->par;

	dss_mode_to_fb_mode(WorkMem->ColorFmt, &(fbinfo->var));

	MSG("w=%d, h=%d, bpl=%d, fmt=%d\n", fbinfo->var.xres, fbinfo->var.yres, fbinfo->fix.line_length, WorkMem->ColorFmt);

	memset(&Bitmap, 0, sizeof(gp_bitmap_t));

	Bitmap.width  = WorkMem->UIRes.width;
	Bitmap.height = WorkMem->UIRes.height;
	Bitmap.bpl    = (WorkMem->UIRes.width * fbinfo->var.bits_per_pixel) / 8;
	Bitmap.type   = WorkMem->ColorFmt;
	Bitmap.pData  = (uint8_t*)fbinfo->fix.smem_start;

	// Set scaler
	Scale.x      = 0;
	Scale.y      = 0;
	Scale.width  = WorkMem->panelRes.width;
	Scale.height = WorkMem->panelRes.height;

	/* Enable disp */
	if (WorkMem->ColorFmt == SP_BITMAP_ARGB8888)
		mainDispAlpha.ppamd = SP_DISP_ALPHA_PERPIXEL_ONLY;  // for ARGB8888 format
	else
		mainDispAlpha.ppamd = SP_DISP_ALPHA_COLORKEY_ONLY;  // for RGB565 format

	disp_set_osd_bitmap(WorkMem->mainDisp, 0, &Bitmap);
	disp_set_osd_scale(WorkMem->mainDisp, 0, &Scale);
	disp_set_osd_alpha(WorkMem->mainDisp, 0, &mainDispAlpha);
	disp_set_osd_enable(WorkMem->mainDisp, 0, 1);

	disp_update(WorkMem->mainDisp);
}

static int32_t
gp_fb_open(
	struct fb_info *fbinfo,
	int32_t user
)
{
	MSG("\n");
	return 0;
}

static int32_t
gp_fb_release(
	struct fb_info *fbinfo,
	int32_t user
)
{
	MSG("\n");
	return 0;
}

static int32_t
gp_fb_check_var(
	struct fb_var_screeninfo *var,
	struct fb_info *fbinfo
)
{
	MSG("\n");

	if ((var->bits_per_pixel != 16) && (var->bits_per_pixel != 32)) {
	    ERROR("Frame buffer check var failed. bpp=%d\n", var->bits_per_pixel);
		return -EINVAL;
	}

	return 0;
}

static int32_t
gp_fb_set_par(
	struct fb_info *fbinfo
)
{
	ERROR("Not implement yet.\n");
	return -EINVAL;
}

static int32_t
gp_fb_pan_display(
	struct fb_var_screeninfo *var,
	struct fb_info *fbinfo
)
{
	void *addr;
	GPFB_WORKMEM *WorkMem = fbinfo->par;

	fbinfo->var.xoffset = var->xoffset;
	fbinfo->var.yoffset = var->yoffset;

	addr = (void*)(fbinfo->fix.smem_start + var->yoffset * fbinfo->fix.line_length);
	disp_set_osd_frame_addr(WorkMem->mainDisp, 0, (uint8_t*) addr);
	disp_set_osd_enable(WorkMem->mainDisp, 0, 1);

	disp_update(WorkMem->mainDisp);

	return 0;
}

static struct fb_ops gp_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_open        = gp_fb_open,
	.fb_release     = gp_fb_release,
	.fb_check_var	= gp_fb_check_var,
	.fb_set_par	    = gp_fb_set_par,
	.fb_pan_display = gp_fb_pan_display,
};

static int32_t __init
gp_fb_init(
	void
)
{
	int ret = 0;
	int BPP;
	struct fb_info *fbinfo = 0;
	GPFB_WORKMEM *WorkMem = 0;
	const GP_COLORMODE *colormode;

	MSG("\n");

	fbinfo = framebuffer_alloc(sizeof(GPFB_WORKMEM), NULL);
	CHECK_VAL(fbinfo != 0);

	gFrameBufInfo = fbinfo;

	WorkMem = fbinfo->par;
	memset(WorkMem, 0, sizeof(GPFB_WORKMEM));
	WorkMem->fb = fbinfo;
	strncpy(WorkMem->ID, "gp_fb", 6);

	if (strncmp(color_fmt, "ARGB8888", 9) == 0) WorkMem->ColorFmt = SP_BITMAP_ARGB8888;
	else if (strncmp(color_fmt, "RGB565", 7) == 0) WorkMem->ColorFmt = SP_BITMAP_RGB565;
	else WorkMem->ColorFmt = SP_BITMAP_ARGB8888;

	colormode = &ColorModeTab[WorkMem->ColorFmt];

	printk("Color format = %s\n", colormode->name);
	printk("Main display = %s\n", main_disp);

	WorkMem->mainDisp = disp_open(main_disp, -1, 0);
	if (WorkMem->mainDisp == 0) {
		ERROR("Main display <%s> open failed\n", main_disp);
		RETURN(-1);
	}

	disp_get_panel_res(WorkMem->mainDisp, &WorkMem->panelRes);

	WorkMem->UIRes.width  = WorkMem->panelRes.width;
	WorkMem->UIRes.height = WorkMem->panelRes.height;

	BPP = colormode->bits_per_pixel / 8;

	WorkMem->fbsize = (WorkMem->UIRes.width * WorkMem->UIRes.height * BPP) * FBBUFFER_NUM;

    printk("UIRes.width %d, UIRes.height %d, BPP %d\n", WorkMem->UIRes.width, WorkMem->UIRes.height, BPP);

	WorkMem->fbmem = gp_chunk_malloc(WorkMem->fbsize);
	CHECK_VAL(WorkMem->fbmem != NULL);

	memset(WorkMem->fbmem, 0, WorkMem->fbsize);

	fbinfo->fbops = &gp_fb_ops;
	fbinfo->flags = FBINFO_FLAG_DEFAULT;
	fbinfo->pseudo_palette = NULL;
	fbinfo->screen_base = WorkMem->fbmem;
	fbinfo->screen_size = WorkMem->fbsize;

	// Resolution
	fbinfo->var.xres = WorkMem->UIRes.width;
	fbinfo->var.yres = WorkMem->UIRes.height;
	fbinfo->var.xres_virtual = fbinfo->var.xres;
 	fbinfo->var.yres_virtual = fbinfo->var.yres * FBBUFFER_NUM;

	// Timing
	fbinfo->var.left_margin     = 0;
	fbinfo->var.right_margin    = 0;
	fbinfo->var.upper_margin    = 0;
	fbinfo->var.lower_margin    = 0;
	fbinfo->var.hsync_len       = 0;
	fbinfo->var.vsync_len       = 0;

	fbinfo->var.activate        = FB_ACTIVATE_FORCE;
	fbinfo->var.accel_flags     = 0;
	fbinfo->var.vmode           = FB_VMODE_NONINTERLACED;

	// fixed info
	strcpy(fbinfo->fix.id, driver_name);
	fbinfo->fix.mmio_start  = 0x92010000;
	fbinfo->fix.mmio_len    = 0x1000;
	fbinfo->fix.type        = FB_TYPE_PACKED_PIXELS;
	fbinfo->fix.type_aux	= 0;
	fbinfo->fix.visual      = FB_VISUAL_TRUECOLOR;
	fbinfo->fix.xpanstep	= 0;
	fbinfo->fix.ypanstep	= 1;
	fbinfo->fix.ywrapstep	= 0;
	fbinfo->fix.accel	    = FB_ACCEL_NONE;
	fbinfo->fix.smem_start  = gp_chunk_pa(WorkMem->fbmem);
	fbinfo->fix.smem_len    = WorkMem->fbsize;
	fbinfo->fix.line_length = (fbinfo->var.xres_virtual * colormode->bits_per_pixel) / 8;

	mainDisp_Init(fbinfo);

	CHECK(register_framebuffer(fbinfo));

Return:
	if(ret < 0) {
		if(fbinfo) {
			if(WorkMem->mainDisp) disp_close(WorkMem->mainDisp, 1);
			if(WorkMem->fbmem) gp_chunk_free(WorkMem->fbmem);
			framebuffer_release(fbinfo);
			gFrameBufInfo = 0;
		}
		ERROR("frame buffer initialization failed!!\n");
		msleep(5000);
	}
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
	struct fb_info *fbinfo = gFrameBufInfo;
	if (fbinfo) {
		GPFB_WORKMEM *WorkMem = fbinfo->par;
		unregister_framebuffer(fbinfo);
		if (WorkMem->mainDisp) mainDisp_Release(fbinfo);
		if (WorkMem->fbmem) {
			gp_chunk_free(WorkMem->fbmem);
		}
		framebuffer_release(fbinfo);
	}
	MSG("frame-buffer module remove done\n");

	return;
}

module_param(main_disp, charp, S_IRUGO);
module_param(color_fmt, charp, S_IRUGO);

module_init(gp_fb_init);
module_exit(gp_fb_exit);

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP frame buffer driver");
MODULE_LICENSE_GP;
