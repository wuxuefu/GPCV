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

/*!
 * @file gp_tv.c
 * @brief The tv driver
 */

#include <mach/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/clk.h>

#include <mach/hal/hal_disp.h>
#include <mach/gp_display.h>
#include <linux/earlysuspend.h>

#include "gp_tv_mode_table.h"

MODULE_LICENSE("GPL");

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#if 0
	#define DEBUG(fmt, arg...)	printk("[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)
#else
	#define DEBUG(...)
#endif

#define ERROR(fmt, arg...)		printk("[%s:%d] Error! "fmt, __FUNCTION__, __LINE__, ##arg)
#define WARNING(fmt, arg...)	printk("[%s:%d] Warning! "fmt, __FUNCTION__, __LINE__, ##arg)
#define MSG(fmt, arg...)		printk("[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)

#define RETURN(x)		{ret = x; goto Return;}
#define CHECK_(x, msg, errid) if(!(x)) {ERROR("%s, %s\n", msg, #x); RETURN(errid);}
#define CHECK_PRG(x)	CHECK_((x), "Program Error", -EIO)
#define CHECK_VAL(x)	CHECK_((x), "Value Error", -1)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
typedef struct
{
	void *RegBase;
} gp_tv_workMem;

static gp_tv_workMem gTvWorkMem;
static struct semaphore refCnt_Tv;

static void *TV_Open(void *RegBase)
{
	if(down_trylock(&refCnt_Tv) != 0) return 0;
	MSG("\n");
	gTvWorkMem.RegBase = RegBase;
	//gpHalDispSetEnable(RegBase, HAL_DISP_DEV_TV, 1);

	/* Set tv output path */
	gpHalDispPathSelect(DISP_MODE_CVBS, DISP_PATH_DISP0);

	return &gTvWorkMem;
}

static void TV_Close(void *Inst)
{
	gp_tv_workMem *WorkMem = (gp_tv_workMem*)Inst;
	MSG("\n");
	WorkMem->RegBase = 0;
	up(&refCnt_Tv);
}

static int TV_SetMode(void *Inst, struct clk *pClk, int mode)
{
	gp_tv_workMem *WorkMem = (gp_tv_workMem*)Inst;
	const TV_MODE *TvMode;
	MSG("\n");
	
	if(mode < 0 || mode >= SP_DISP0_TV_MODE_MAX)
		mode = SP_DISP0_TV_MODE_NTSC;
	TvMode = TvModeTable + mode;
	MSG("TV mode = <%s>\n", TvMode->name);

    // Set clock
	//clk_set_rate(pClk, 27000000);
	gpHalDispSetClock(27000000, 0);
	MSG("TV Clock = %lu\n", clk_get_rate(pClk));

	gpHalDispSetEnable(WorkMem->RegBase, HAL_DISP_DEV_TV, 0);

	gpHalDispSetRes(WorkMem->RegBase, TvMode->underscanwidth, TvMode->underscanheight);
	gpHalDispSetPriRes(WorkMem->RegBase, TvMode->width, TvMode->height);
	gpHalDispSetTvType(WorkMem->RegBase, TvMode->tvType);
	gpHalDispSetTvPulse(WorkMem->RegBase, TvMode->pulse6);
	gpHalDispSetTvScan(WorkMem->RegBase, TvMode->scanSel);
	gpHalDispSetTvFscType(WorkMem->RegBase, TvMode->fscType);
	gpHalDispSetTvFix625(WorkMem->RegBase, TvMode->fix625);
	gpHalDispSetTvLine(WorkMem->RegBase, TvMode->lineSel);
	gpHalDispSetTvColorBurstWidth(WorkMem->RegBase, TvMode->cbWidth);
	gpHalDispSetTvColorBurstSel(WorkMem->RegBase, TvMode->cbSel);
	gpHalDispSetTvAmpAdj(WorkMem->RegBase, TvMode->ampAdj);
	gpHalDispSetTvPosAdj(WorkMem->RegBase, TvMode->posAdj);

	MSG("check point 0\n");
#if 1 //Gavin modify to correct TV output. Deflicker can't turn on with display scaler
    if(TvMode->scanSel == HAL_DISP_TV_SCANSEL_INTERLACED)
		gpHalDispSetDeflickerInfo(WorkMem->RegBase, 0, 0, 0, 0);
    else
		gpHalDispSetDeflickerInfo(WorkMem->RegBase, 0, 0, 0, 1);

    gpHalDispSetPriDmaType(WorkMem->RegBase, TvMode->dmaType);
    gpHalDispSetOsdDmaType(WorkMem->RegBase, 0, TvMode->dmaType);
    gpHalDispSetOsdDmaType(WorkMem->RegBase, 1, TvMode->dmaType);
#else    
    if(TvMode->scanSel == HAL_DISP_TV_SCANSEL_INTERLACED)
	{
		gpHalDispSetDeflickerInfo(WorkMem->RegBase, 1, 1, 1, 1);
		gpHalDispSetPriDmaType(WorkMem->RegBase, HAL_DISP_TV_DMA_PROGRESSIVE);
		gpHalDispSetOsdDmaType(WorkMem->RegBase, 0, HAL_DISP_TV_DMA_PROGRESSIVE);
		gpHalDispSetOsdDmaType(WorkMem->RegBase, 1, HAL_DISP_TV_DMA_PROGRESSIVE);
	}
	else
	{
		gpHalDispSetDeflickerInfo(WorkMem->RegBase, 0, 0, 0, 1);
		gpHalDispSetPriDmaType(WorkMem->RegBase, TvMode->dmaType);
		gpHalDispSetOsdDmaType(WorkMem->RegBase, 0, TvMode->dmaType);
		gpHalDispSetOsdDmaType(WorkMem->RegBase, 1, TvMode->dmaType);
	}
#endif
	MSG("check point 1\n");
	gpHalDispSetVDACPowerDown(WorkMem->RegBase, 1);  //Gavin add to reset VDEC

	gpHalDispSetVDACPowerDown(WorkMem->RegBase, 0);  //Gavin add to reset VDEC

	gpHalDispSetPanelFormat(WorkMem->RegBase, HAL_DISP_OUTPUT_FMT_RGB, HAL_DISP_OUTPUT_TYPE_SRGBM888, HAL_DISP_PRGB888_RGB, HAL_DISP_PRGB888_RGB);
	gpHalDispSetBlankingIntervalTo0(WorkMem->RegBase, 1);

	gpHalDispSetColorMatrix(WorkMem->RegBase, (const UINT16*)TvMode->pColorMatrix);

	gpHalDispSetEnable(WorkMem->RegBase, HAL_DISP_DEV_TV, 1);
	MSG("check point 2\n");
	return mode;
}

static int TV_OnPreSuspend(void *Inst)
{
	gp_tv_workMem *WorkMem = (gp_tv_workMem*)&gTvWorkMem;
	MSG("\n");

	if(WorkMem && WorkMem->RegBase)
		gpHalDispSetVDACPowerDown(WorkMem->RegBase, 1);

	return 0;
}

static int TV_OnResume(void *Inst)
{
	gp_tv_workMem *WorkMem = (gp_tv_workMem*)&gTvWorkMem;
	MSG("\n");
	if(WorkMem && WorkMem->RegBase) {
		gpHalDispSetVDACPowerDown(WorkMem->RegBase, 1);
		msleep(100);
		gpHalDispSetVDACPowerDown(WorkMem->RegBase, 0);
	}

	return 0;
}

static int TV_GetPixelSize(void *Inst, gp_disp_pixelsize_t *size)
{
	size->width = 4;
	size->height = 3;
}

static const gp_disp_drv_op TV_DrvOp = 
{
	.Owner			= THIS_MODULE,
	.Name			= "TV",
	.Type			= SP_DISP_OUTPUT_TV,
	.Open			= TV_Open,
	.Close			= TV_Close,
	.SetTiming		= TV_SetMode,
	.OnPreSuspend	= TV_OnPreSuspend,
	.OnResume		= TV_OnResume,
	.GetPixelSize	= TV_GetPixelSize,
};

static int __init tvPanel_init(void)
{
	MSG("\n");
	sema_init(&refCnt_Tv, 1);

	register_display(&TV_DrvOp);
	return 0;
}

static void __exit tvPanel_exit(void)
{
	MSG("\n");
	unregister_display(&TV_DrvOp);
}

module_init(tvPanel_init);
module_exit(tvPanel_exit);

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP TV Driver");

