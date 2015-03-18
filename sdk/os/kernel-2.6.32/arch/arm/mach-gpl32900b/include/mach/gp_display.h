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
 * @file display.h
 * @brief Display interface header file
 * @author Anson Chuang
 */

#ifndef _GP_DISPLAY_DEVICE_H_
#define _GP_DISPLAY_DEVICE_H_


#include <mach/typedef.h>
#ifdef __KERNEL__
#include <linux/list.h>
#endif
#include <linux/clk.h>


/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define DISPIO_TYPE 'd'

/* Common */
#define DISPIO_SET_INITIAL			_IO(DISPIO_TYPE, 0x01)
#define DISPIO_SET_UPDATE			_IO(DISPIO_TYPE, 0x02)
#define DISPIO_GET_PANEL_RESOLUTION	_IOR(DISPIO_TYPE, 0x03, gp_disp_res_t *)
#define DISPIO_GET_PANEL_SIZE		_IOR(DISPIO_TYPE, 0x04, gp_size_t *)
#define DISPIO_SET_OUTPUT			_IOW(DISPIO_TYPE, 0x05, gp_disp_output_t)
#define DISPIO_GET_OUTPUT			_IOR(DISPIO_TYPE, 0x05, gp_disp_output_t *)

#define DISPIO_WAIT_FRAME_END		_IO(DISPIO_TYPE, 0x07)
#define DISPIO_SET_SUSPEND			_IO(DISPIO_TYPE, 0x08)
#define DISPIO_SET_RESUME			_IO(DISPIO_TYPE, 0x09)
#define DISPIO_SET_BACKLIGHT		_IOW(DISPIO_TYPE, 0x0A, uint32_t)

#define DISPIO_SET_FLIP				_IO(DISPIO_TYPE, 0x0C)
#define DISPIO_SET_TV_MODE			_IOW(DISPIO_TYPE, 0x0D, uint32_t)
#define DISPIO_GET_TV_MODE			_IOR(DISPIO_TYPE, 0x0D, uint32_t *)

/* Primary layer */
#define DISPIO_SET_PRI_ENABLE			_IOW(DISPIO_TYPE, 0x10, uint32_t)
#define DISPIO_GET_PRI_ENABLE			_IOR(DISPIO_TYPE, 0x10, uint32_t)
#define DISPIO_SET_PRI_BITMAP			_IOW(DISPIO_TYPE, 0x11, gp_bitmap_t)
#define DISPIO_GET_PRI_BITMAP			_IOR(DISPIO_TYPE, 0x11, gp_bitmap_t *)
#define DISPIO_SET_PRI_SCALEINFO		_IOW(DISPIO_TYPE, 0x12, gp_disp_scale_t)
#define DISPIO_GET_PRI_SCALEINFO		_IOR(DISPIO_TYPE, 0x12, gp_disp_scale_t *)
#define DISPIO_CHANGE_PRI_BITMAP_BUF	_IOW(DISPIO_TYPE, 0x13, gp_bitmap_t)
#define DISPIO_PRIMARY_SET_FLIP			_IO(DISPIO_TYPE, 0x14)

/* Dithering */
#define DISPIO_SET_DITHER_ENABLE	_IOW(DISPIO_TYPE, 0x20, uint32_t)
#define DISPIO_GET_DITHER_ENABLE	_IOR(DISPIO_TYPE, 0x20, uint32_t)
#define DISPIO_SET_DITHER_TYPE		_IOW(DISPIO_TYPE, 0x21, uint32_t)
#define DISPIO_GET_DITHER_TYPE		_IOR(DISPIO_TYPE, 0x21, uint32_t *)
#define DISPIO_SET_DITHER_PARAM		_IOW(DISPIO_TYPE, 0x22, gp_disp_ditherparam_t)
#define DISPIO_GET_DITHER_PARAM		_IOR(DISPIO_TYPE, 0x22, gp_disp_ditherparam_t *)

/* Color matrix */
#define DISPIO_SET_CMATRIX_PARAM	_IOW(DISPIO_TYPE, 0x30, gp_disp_colormatrix_t)
#define DISPIO_GET_CMATRIX_PARAM	_IOR(DISPIO_TYPE, 0x30, gp_disp_colormatrix_t *)

/* Gamma table */
#define DISPIO_SET_GAMMA_ENABLE	_IOW(DISPIO_TYPE, 0x40, uint32_t)
#define DISPIO_GET_GAMMA_ENABLE	_IOR(DISPIO_TYPE, 0x40, uint32_t)
#define DISPIO_SET_GAMMA_PARAM	_IOW(DISPIO_TYPE, 0x41, gp_disp_gammatable_t)
#define DISPIO_GET_GAMMA_PARAM	_IOR(DISPIO_TYPE, 0x41, gp_disp_gammatable_t *)

/* Color bar */
#define DISPIO_SET_CBAR_ENABLE	_IOW(DISPIO_TYPE, 0x50, uint32_t)
#define DISPIO_GET_CBAR_ENABLE	_IOR(DISPIO_TYPE, 0x50, uint32_t)
#define DISPIO_SET_CBARINFO		_IOW(DISPIO_TYPE, 0x51, gp_disp_colorbar_t *)

/* Buffer control */
#define DISPIO_BUF_ALLOC	_IOW(DISPIO_TYPE, 0x60, gp_disp_bufinfo_t *)
#define DISPIO_BUF_FREE		_IOW(DISPIO_TYPE, 0x61, uint32_t)
#define DISPIO_BUF_MMAP		_IOW(DISPIO_TYPE, 0x62, gp_disp_bufaddr_t *)
#define DISPIO_BUF_MUNMAP	_IOW(DISPIO_TYPE, 0x63, gp_disp_bufaddr_t *)
#define DISPIO_BUF_GETINFO	_IOR(DISPIO_TYPE, 0x64, gp_disp_bufinfo_t *)

#define	DISPIO_GET_PANEL_PIXELSIZE	_IOR(DISPIO_TYPE, 0x66, gp_disp_pixelsize_t *)

/* OSD layer */
#define DISPIO_OSD_BASE		0x80

#define DISPIO_GET_OSD_TOTALNUM		_IOR(DISPIO_TYPE, 0x70, uint32_t *)

#define DISPIO_SET_OSD_ENABLE(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x0 + (osdIndex) * 0x10, uint32_t)
#define DISPIO_GET_OSD_ENABLE(osdIndex)	_IOR(DISPIO_TYPE, DISPIO_OSD_BASE + 0x0 + (osdIndex) * 0x10, uint32_t *)
#define DISPIO_SET_OSD_BITMAP(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x1 + (osdIndex) * 0x10, gp_bitmap_t)
#define DISPIO_GET_OSD_BITMAP(osdIndex)	_IOR(DISPIO_TYPE, DISPIO_OSD_BASE + 0x1 + (osdIndex) * 0x10, gp_bitmap_t *)
#define DISPIO_SET_OSD_SCALEINFO(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x2 + (osdIndex) * 0x10, gp_disp_scale_t)
#define DISPIO_GET_OSD_SCALEINFO(osdIndex)	_IOR(DISPIO_TYPE, DISPIO_OSD_BASE + 0x2 + (osdIndex) * 0x10, gp_disp_scale_t *)
#define DISPIO_SET_OSD_PALETTE(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x3 + (osdIndex) * 0x10, gp_disp_osdpalette_t)
#define DISPIO_GET_OSD_PALETTE(osdIndex)	_IOR(DISPIO_TYPE, DISPIO_OSD_BASE + 0x3 + (osdIndex) * 0x10, gp_disp_osdpalette_t *)
#define DISPIO_SET_OSD_PALETTEOFFSET(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x4 + (osdIndex) * 0x10, uint32_t)
#define DISPIO_GET_OSD_PALETTEOFFSET(osdIndex)	_IOR(DISPIO_TYPE, DISPIO_OSD_BASE + 0x4 + (osdIndex) * 0x10, uint32_t *)
#define DISPIO_SET_OSD_ALPHA(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x5 + (osdIndex) * 0x10, uint32_t)
#define DISPIO_GET_OSD_ALPHA(osdIndex)	_IOR(DISPIO_TYPE, DISPIO_OSD_BASE + 0x5 + (osdIndex) * 0x10, uint32_t *)
#define DISPIO_SET_OSD_KEY(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x6 + (osdIndex) * 0x10, uint32_t)
#define DISPIO_GET_OSD_KEY(osdIndex)	_IOR(DISPIO_TYPE, DISPIO_OSD_BASE + 0x6 + (osdIndex) * 0x10, uint32_t *)
#define DISPIO_OSD0_SET_FLIP     _IO(DISPIO_TYPE, 0x88)
#define DISPIO_OSD1_SET_FLIP     _IO(DISPIO_TYPE, 0x89)

/* GPL32900 use only */
#define DISPIO_DUMP_REGISTER		_IO(DISPIO_TYPE, 0x0B)
#define DISPIO_SET_FRAMERATE		_IOW(DISPIO_TYPE, 0x0E, uint32_t)
#define DISPIO_GET_FRAMERATE		_IOR(DISPIO_TYPE, 0x0E, uint32_t *)

/* GPL32900B use only */
#define DISPIO_SET_MODE				_IOW(DISPIO_TYPE, 0x0F, int)
#define DISPIO_GET_MODE				_IOR(DISPIO_TYPE, 0x0F, int *)
#define DISPIO_SET_EDGE_GAIN		_IOW(DISPIO_TYPE, 0x15, gp_disp_edge_gain_t)
#define DISPIO_SET_PRI_ADDRESS		_IOW(DISPIO_TYPE, 0x17, gp_disp_pri_addr_t*)
#define DISPIO_SET_DYN_CMTX_INFO    _IOW(DISPIO_TYPE, 0x31, gp_disp_dyncmtxinfo_t)
#define DISPIO_SET_DYN_CMTX_PARA    _IOW(DISPIO_TYPE, 0x32, gp_disp_dyncmtxpara_t)
#define DISPIO_SET_DYN_CMTX_ENABLE	_IOW(DISPIO_TYPE, 0x33, int)
#define DISPIO_SET_DYN_CMTX_INDEX   _IOW(DISPIO_TYPE, 0x34, gp_disp_dyncmtxindex_t)
#define DISPIO_CHANGE_OSD_BITMAP_BUF(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x7 + (osdIndex) * 0x10, gp_bitmap_t)
#define DISPIO_SET_OSD_ADDRESS(osdIndex)	_IOW(DISPIO_TYPE, DISPIO_OSD_BASE + 0x7 + (osdIndex) * 0x10, uint32_t *)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define GP_DISP_DEV_NAME_MAX_SIZE	33
#define GP_DISP_BUFFER_MAX 8	/* max buffer count */

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
/** @brief A structure of output device operations */
typedef struct gp_disp_panel_ops_s {
	int32_t (*init) (void);
	int32_t (*suspend) (void);
	int32_t (*resume) (void);
	int32_t (*resume_post) (void);
	int32_t (*get_size) (gp_size_t *size);
	int32_t (*set_param) (void *data);
	
	/* GPL32900B use only */
	void* (*get_param)(void);
} gp_disp_panel_ops_t;

typedef enum {
	SP_DISP_OUTPUT_LCD = 0,
	SP_DISP_OUTPUT_LCM,
	SP_DISP_OUTPUT_TV,
	SP_DISP_OUTPUT_MAX,
	/* GPL32900B use only */
	SP_DISP_OUTPUT_HDMI,
} SP_DISP_OUTPUT_TYPE;

/* for display 0 */
typedef enum {
	SP_DISP0_TV_MODE_NTSC = 0,
	SP_DISP0_TV_MODE_PAL,		// PALBDGHI
	SP_DISP0_TV_MODE_NTSCJ,
	SP_DISP0_TV_MODE_NTSC443,
	SP_DISP0_TV_MODE_PALM,
	SP_DISP0_TV_MODE_PAL60,
	SP_DISP0_TV_MODE_PALN,
	SP_DISP0_TV_MODE_PALNc,
	SP_DISP0_TV_MODE_NTSC_NONINTERLACED,
	SP_DISP0_TV_MODE_PALM_NONINTERLACED,
	SP_DISP0_TV_MODE_PAL_NONINTERLACED,
	SP_DISP0_TV_MODE_MAX,
} SP_DISP0_TV_MODE;

typedef enum {
	SP_DISP_TV_MODE_NTSC = 0,
	SP_DISP_TV_MODE_PAL,
	SP_DISP_TV_MODE_NTSC_J,
	SP_DISP_TV_MODE_NTSC_N,
	SP_DISP_TV_MODE_PAL_B,
	SP_DISP_TV_MODE_PAL_N,
	SP_DISP_TV_MODE_PAL_NC,	
	SP_DISP_TV_MODE_NTSC_NON_INTERLACE,
	SP_DISP_TV_MODE_PAL_NON_INTERLACE,
	SP_DISP_TV_MODE_VGA,
	SP_DISP_TV_MODE_QVGA,
	SP_DISP_TV_MODE_V_576,		
	SP_DISP_TV_MODE_480I,
	SP_DISP_TV_MODE_720P
} SP_DISP_TV_MODE;

/** @brief A structure of output device */
typedef struct gp_disp_output_s {
	int32_t type;		/*!< @brief Output device type */
	int32_t mode;		/*!< @brief Output device support mode */
	uint8_t name[32];	/*!< @brief Output device name */
	gp_disp_panel_ops_t *ops;	/*!< @brief Output device operations */
} gp_disp_output_t;

/** @brief A structure of display resolution */
typedef struct gp_disp_res_s {
	uint16_t width;	/*!< @brief Width */
	uint16_t height;	/*!< @brief Height */
} gp_disp_res_t;

typedef struct gp_disp_pixelsize_s {
	uint16_t width;	/*!< @brief Width */
	uint16_t height;	/*!< @brief Height */
} gp_disp_pixelsize_t;

/** @brief A structure of display scale setting */
typedef struct gp_disp_scale_s {
	int16_t x;			/*!< @brief X position */
	int16_t y;			/*!< @brief Y position */
	uint16_t width;		/*!< @brief Width */
	uint16_t height;		/*!< @brief Height */
	uint32_t blankcolor;	/*!< @brief Blank color RGB/CrCbY */
} gp_disp_scale_t;

/** @brief A structure of dithering map */
typedef struct gp_disp_ditherparam_s {
	uint32_t d00;
	uint32_t d01;
	uint32_t d02;
	uint32_t d03;
	uint32_t d10;
	uint32_t d11;
	uint32_t d12;
	uint32_t d13;
	uint32_t d20;
	uint32_t d21;
	uint32_t d22;
	uint32_t d23;
	uint32_t d30;
	uint32_t d31;
	uint32_t d32;
	uint32_t d33;
} gp_disp_ditherparam_t;

/** @brief A structure of color matrix parameters */
typedef struct gp_disp_colormatrix_s {
	uint16_t a00;
	uint16_t a01;
	uint16_t a02;
	uint16_t a10;
	uint16_t a11;
	uint16_t a12;
	uint16_t a20;
	uint16_t a21;
	uint16_t a22;
	uint16_t b0;
	uint16_t b1;
	uint16_t b2;
} gp_disp_colormatrix_t;

enum {
	SP_DISP_GAMMA_R = 0,
	SP_DISP_GAMMA_G = 1,
	SP_DISP_GAMMA_B = 2
};

/** @brief A structure of gamma table */
typedef struct gp_disp_gammatable_s {
	uint32_t id;		/*!< @brief The id of gamma table */
	uint8_t table[1024];	/*!< @brief The gamma table */
} gp_disp_gammatable_t;

enum {
	SP_DISP_OSD_TYPE_16BPP = 0,
	SP_DISP_OSD_TYPE_8BPP = 1,
	SP_DISP_OSD_TYPE_4BPP = 2,
	SP_DISP_OSD_TYPE_1BPP = 3,
};

/** @brief A structure of osd palette */
typedef struct gp_disp_osdpalette_s {
	uint32_t type;		/*!< @brief The type of osd layer  */
	uint32_t startIndex;	/*!< @brief The offset index of palette table in sram */
	uint32_t count;		/*!< @brief The number of palette color */
	uint32_t table[256];	/*!< @brief The source palette table */
} gp_disp_osdpalette_t;

typedef enum {
	SP_DISP_ALPHA_PERPIXEL = 0,
	SP_DISP_ALPHA_CONSTANT = 1,
} gp_disp_alpha_type1;	/* consta */

typedef enum {
	SP_DISP_ALPHA_PERPIXEL_ONLY = 0,
	SP_DISP_ALPHA_COLORKEY_ONLY = 1,
	SP_DISP_ALPHA_BOTH = 2,
} gp_disp_alpha_type2;	/* ppamd */

/** @brief A structure of osd alpha */
typedef struct gp_disp_osdalpha_s {
	uint32_t consta; 	/*!< @brief The type1 of alpha blending. */
	uint32_t ppamd; 	/*!< @brief The type2 of alpha blending. */
	uint16_t alpha;	/*!< @brief The value of alpha blending from 0~100. */
	uint16_t alphasel;
} gp_disp_osdalpha_t;

/** @brief A structure of color bar */
typedef struct gp_disp_colorbar_s {
	uint32_t type;		/*!< @brief The type of color bar. */
	uint32_t size;		/*!< @brief The number of column per bar. */
	uint32_t color;	/*!< @brief The color value of color bar. [23:16] = r, [15:8] = g, [7:0] = b */
} gp_disp_colorbar_t;


/** @brief A structure of buffer control */
typedef struct gp_disp_bufinfo_s {
	uint32_t id;		/*!< @brief Buffer id */
	uint16_t width;		/*!< @brief Width */
	uint16_t height;	/*!< @brief Height */
	uint32_t bpp;		/*!< @brief Bits per pixel */
	uint32_t size;		/*!< @brief Buffer size in bytes */
} gp_disp_bufinfo_t;

/** @brief A structure of buffer address */
typedef struct gp_disp_bufaddr_s {
	uint32_t id;	/*!< @brief Buffer id */
	void *ptr;		/*!< @brief Buffer address */
} gp_disp_bufaddr_t;

typedef enum display_state {
	DISP_STATE_SUSPEND,
	DISP_STATE_RESUME,
} gp_disp_state;


/* GPL32900 use only */
/** @brief A structure of lcd info */
typedef struct gp_disp_lcdparam_s {
	uint16_t vPolarity;	/*!< @brief Vsync polarity */
	uint16_t vFront;		/*!< @brief Vsync front porch */
	uint16_t vBack;		/*!< @brief Vsync back porch */
	uint16_t vWidth;		/*!< @brief Vsync width */
	uint16_t hPolarity;	/*!< @brief Hsync polarity */
	uint16_t hFront;		/*!< @brief Hsync front porch */
	uint16_t hBack;		/*!< @brief Hsync back porch */
	uint16_t hWidth;		/*!< @brief Hsync width */
} gp_disp_lcdparam_t;

/* GPL32900B use only */
typedef struct gp_disp_dyncmtxinfo_s {
	uint32_t satlimit;
	uint32_t lolimit;
	uint32_t hilimit;    
} gp_disp_dyncmtxinfo_t;

typedef struct gp_disp_dyncmtxindex_s {
	uint32_t index[12];
} gp_disp_dyncmtxindex_t;

typedef struct gp_disp_dyncmtxpara_s {
    gp_disp_colormatrix_t dyn_CMtx[12];
} gp_disp_dyncmtxpara_t;

typedef struct gp_disp_edge_gain_s {
 	uint32_t edgetype;
	uint32_t edgegain;
	uint32_t cortype;
	uint32_t cliptype;
} gp_disp_edge_gain_t;

typedef struct {
	struct module *Owner;
	const char *Name;
	int Type;
	void *(*Open)(void *RegBase);
	void (*Close)(void *Inst);
	int (*OnEnable)(void *Inst);
	int (*SetTiming)(void *Inst, struct clk *pClk, int mode);
	int (*OnPreSuspend)(void *Inst);
	int (*OnPostSuspend)(void *Inst);
	int (*OnResume)(void *Inst);
	int (*GetPixelSize)(void *Inst, gp_disp_pixelsize_t *size);
	unsigned long InterruptMask;
	void (*OnInterrupt)(void *Inst, int intFlag);
	int (*Backdoor)(void *Inst, const char *backdoor);
	int (*GetSupportMode)(unsigned int *mode);
} gp_disp_drv_op;

typedef struct gp_disp_pri_addr_s {
    uint8_t* addr;
    uint8_t* addrU;
} gp_disp_pri_addr_t;

enum {
	SP_DISP_FLIP_NONE = 0,
	SP_DISP_FLIP_H = 1,
	SP_DISP_FLIP_V = 2,
	SP_DISP_FLIP_HV = 3,
};

#include <mach/gp_display_func.h>

#endif //endif _GP_DISPLAY_DEVICE_H_
