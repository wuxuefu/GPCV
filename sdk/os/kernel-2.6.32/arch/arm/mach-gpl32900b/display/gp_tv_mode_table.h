#ifndef __GP_TV_MODE_TABLE_H__
#define __GP_TV_MODE_TABLE_H__

#define		VDAC_PDALL					(1 << 0)
#define		VDAC_TEST					(1 << 1)
#define		VDAC_UD						(1 << 2)
#define     VDAC_CRYSTAL_EN             (1 << 3)

// NTSC
#define NTSC_UNDERSCAN_WIDTH		720
#define NTSC_UNDERSCAN_HEIGHT		480
#define NTSC_OVERSCAN_WIDTH			664
#define NTSC_OVERSCAN_HEIGHT		440
// PAL
#define PAL_UNDERSCAN_WIDTH			720	//664
#define PAL_UNDERSCAN_HEIGHT		576	//528
#define PAL_OVERSCAN_WIDTH			664
#define PAL_OVERSCAN_HEIGHT			528

#define NTSC_WIDTH		NTSC_OVERSCAN_WIDTH
#define NTSC_HEIGHT		NTSC_OVERSCAN_HEIGHT

#define PAL_WIDTH		PAL_OVERSCAN_WIDTH
#define PAL_HEIGHT		PAL_OVERSCAN_HEIGHT

typedef struct
{
	const char *name;
	int underscanwidth;
	int underscanheight;
	int width;
	int height;
	int format;
	// int type;
	// int dataSeqEven;
	// int dataSeqOdd;
//		.type = HAL_DISP_OUTPUT_TYPE_SRGBM888,
//		.dataSeqEven = HAL_DISP_PRGB888_RGB,
//		.dataSeqOdd = HAL_DISP_PRGB888_RGB,
	
	int dmaType;
	int tvType;
	int pulse6;
	int scanSel;
	int fscType;
	int fix625;
	int lineSel;
	int cbWidth;
	int cbSel;
	gpHalDispTvAmpAdj_t ampAdj;
	gpHalDispTvPosAdj_t posAdj;
	const gp_disp_colormatrix_t *pColorMatrix;
} TV_MODE;

//	The inversed row 1 and 3 of GP8050 old table was fixed in GP8300 by SJ 20110222
static const gp_disp_colormatrix_t gNTSCColorMatrix = {
	.a00 = 0x27,	.a01 = 0x4C,	.a02 = 0xF,
	.a10 = 0xFFED,	.a11 = 0xFFDA,	.a12 = 0x39,
	.a20 = 0x50,	.a21 = 0xFFBD,	.a22 = 0xFFF3,
	.b0 = 0,		.b1 = 0,		.b2 = 0,
};

static const gp_disp_colormatrix_t gNTSCJColorMatrix = {
	.a00 = 0x2A,	.a01 = 0x52,	.a02 = 0x20,
	.a10 = 0xFFEC,	.a11 = 0xFFD7,	.a12 = 0x3D,
	.a20 = 0x56,	.a21 = 0xFFB8,	.a22 = 0xFFF2,
	.b0 = 0,		.b1 = 0,		.b2 = 0,
};

// The inversed row 1 and 3 of GP8050 old table was fixed in GP8300 by SJ 20110222
// PAL(BDGHINc)
static const gp_disp_colormatrix_t gPALColorMatrix = {
	.a00 = 0x29,	.a01 = 0x50,	.a02 = 0x20,
	.a10 = 0xFFEC,	.a11 = 0xFFD8,	.a12 = 0x3C,
	.a20 = 0x54,	.a21 = 0xFFBA,	.a22 = 0xFFF2,
	.b0 = 0,		.b1 = 0,		.b2 = 0,
};

static TV_MODE TvModeTable[] = {
	[SP_DISP0_TV_MODE_NTSC] = 
	{
		.name = "NTSC",
		.underscanwidth	= NTSC_UNDERSCAN_WIDTH,
		.underscanheight = NTSC_UNDERSCAN_HEIGHT,
		.width = NTSC_WIDTH, //720,
		.height = NTSC_HEIGHT, //480,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_NTSC,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_NTSCMJ,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_262_525,
		.cbWidth = HAL_DISP_TV_CBWIDTH_252,
		.cbSel = HAL_DISP_TV_CBSEL_NTSCMJ,
		.ampAdj = {
			.luminance = 240,
			.blank = 42,
			.burst = 112,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 32,
		},
		.pColorMatrix = &gNTSCColorMatrix,
	},
	[SP_DISP0_TV_MODE_PAL] =
	{
		.name = "PAL",
		.underscanwidth = PAL_UNDERSCAN_WIDTH,
		.underscanheight = PAL_UNDERSCAN_HEIGHT,
		.width = PAL_WIDTH,
		.height = PAL_HEIGHT,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_PAL,
		.pulse6 = HAL_DISP_TV_PULSE6_5PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_PALBDGHIN,
		.fix625 = 1,
		.lineSel = HAL_DISP_TV_LINESEL_312_625,
		.cbWidth = HAL_DISP_TV_CBWIDTH_225,
		.cbSel = HAL_DISP_TV_CBSEL_PALBDGHINNC,
		.ampAdj = {
			.luminance = 252,
			.blank = 0,
			.burst = 117,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 52,
		},
		.pColorMatrix = &gPALColorMatrix,
	},
	[SP_DISP0_TV_MODE_NTSCJ] =
	{
		.name = "NTSCJ",
		.underscanwidth = NTSC_UNDERSCAN_WIDTH,
		.underscanheight = NTSC_UNDERSCAN_HEIGHT,
		.width = NTSC_WIDTH, //720,
		.height = NTSC_HEIGHT, //480,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_NTSC,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_NTSCMJ,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_262_525,
		.cbWidth = HAL_DISP_TV_CBWIDTH_252,
		.cbSel = HAL_DISP_TV_CBSEL_NTSCMJ,
		.ampAdj = {
			.luminance = 240,
			.blank = 0,
			.burst = 112,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 32,
		},
		.pColorMatrix = &gNTSCJColorMatrix,
	},
	[SP_DISP0_TV_MODE_NTSC443] =
	{
		.name = "NTSC443",
		.underscanwidth = NTSC_UNDERSCAN_WIDTH,
		.underscanheight = NTSC_UNDERSCAN_HEIGHT,
		.width = NTSC_WIDTH, //720,
		.height = NTSC_HEIGHT, //480,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_NTSC,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_NTSC443_PAL60,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_262_525,
		.cbWidth = HAL_DISP_TV_CBWIDTH_225,
		.cbSel = HAL_DISP_TV_CBSEL_NTSCMJ,
		.ampAdj = {
			.luminance = 240,
			.blank = 42,
			.burst = 112,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 32,
		},
		.pColorMatrix = &gNTSCColorMatrix,
	},
	[SP_DISP0_TV_MODE_PALM] =
	{
		.name = "PALM",
		.underscanwidth = NTSC_UNDERSCAN_WIDTH,
		.underscanheight = NTSC_UNDERSCAN_HEIGHT,
		.width = NTSC_WIDTH, //720,
		.height = NTSC_HEIGHT, //480,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_PAL,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_PALM,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_262_525,
		.cbWidth = HAL_DISP_TV_CBWIDTH_252,
		.cbSel = HAL_DISP_TV_CBSEL_PALM,
		.ampAdj = {
			.luminance = 240,
			.blank = 42,
			.burst = 117,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 32,
		},
		.pColorMatrix = &gNTSCColorMatrix,
	},
	[SP_DISP0_TV_MODE_PAL60] =
	{
		.name = "PAL60",
		.underscanwidth = NTSC_UNDERSCAN_WIDTH,
		.underscanheight = NTSC_UNDERSCAN_HEIGHT,
		.width = NTSC_WIDTH, //720,
		.height = NTSC_HEIGHT, //480,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_PAL,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_NTSC443_PAL60,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_262_525,
		.cbWidth = HAL_DISP_TV_CBWIDTH_225,
		.cbSel = HAL_DISP_TV_CBSEL_NTSCMJ,
		.ampAdj = {
			.luminance = 252,
			.blank = 0,
			.burst = 117,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 32,
		},
		.pColorMatrix = &gNTSCColorMatrix,
	},
	[SP_DISP0_TV_MODE_PALN] =
	{
		.name = "PALN",
		.underscanwidth = PAL_UNDERSCAN_WIDTH,
		.underscanheight = PAL_UNDERSCAN_HEIGHT,
		.width = PAL_WIDTH,
		.height = PAL_HEIGHT,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_PAL,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_PALBDGHIN,
		.fix625 = 1,
		.lineSel = HAL_DISP_TV_LINESEL_312_625,
		.cbWidth = HAL_DISP_TV_CBWIDTH_225,
		.cbSel = HAL_DISP_TV_CBSEL_PALBDGHINNC,
		.ampAdj = {
			.luminance = 240,
			.blank = 42,
			.burst = 117,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 52,
		},
		.pColorMatrix = &gNTSCColorMatrix,
	},
	[SP_DISP0_TV_MODE_PALNc] =
	{
		.name = "PALNc",
		.underscanwidth = PAL_UNDERSCAN_WIDTH,
		.underscanheight = PAL_UNDERSCAN_HEIGHT,
		.width = PAL_WIDTH,
		.height = PAL_HEIGHT,
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_INTERLACED,
		.tvType = HAL_DISP_TV_TYPE_PAL,
		.pulse6 = HAL_DISP_TV_PULSE6_5PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_INTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_PALNC,
		.fix625 = 1,
		.lineSel = HAL_DISP_TV_LINESEL_312_625,
		.cbWidth = HAL_DISP_TV_CBWIDTH_252,
		.cbSel = HAL_DISP_TV_CBSEL_PALBDGHINNC,
		.ampAdj = {
			.luminance = 252,
			.blank = 0,
			.burst = 117,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 26,
			.hAct = 52,
		},
		.pColorMatrix = &gPALColorMatrix,
	},
	[SP_DISP0_TV_MODE_NTSC_NONINTERLACED] =
	{
		.name = "NTSC Non-Interlaced",
		.underscanwidth = NTSC_UNDERSCAN_WIDTH,
		.underscanheight = (NTSC_UNDERSCAN_HEIGHT >> 1),
		.width = NTSC_WIDTH,
		.height = (NTSC_HEIGHT >> 1),
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_PROGRESSIVE,
		.tvType = HAL_DISP_TV_TYPE_NTSC,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_NONINTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_NTSCMJ,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_262_525,
		.cbWidth = HAL_DISP_TV_CBWIDTH_252,
		.cbSel = HAL_DISP_TV_CBSEL_NTSCMJ,
		.ampAdj = {
			.luminance = 240,
			.blank = 42,
			.burst = 112,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 0,
			.hAct = 32,
		},
		.pColorMatrix = &gNTSCColorMatrix,
	},
	[SP_DISP0_TV_MODE_PALM_NONINTERLACED] =
	{
		.name = "PALM Non-Interlaced",
		.underscanwidth = NTSC_UNDERSCAN_WIDTH,
		.underscanheight = (NTSC_UNDERSCAN_HEIGHT >> 1),
		.width = NTSC_WIDTH,
		.height = (NTSC_HEIGHT >> 1),
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_PROGRESSIVE,
		.tvType = HAL_DISP_TV_TYPE_PAL,
		.pulse6 = HAL_DISP_TV_PULSE6_6PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_NONINTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_PALM,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_262_525,
		.cbWidth = HAL_DISP_TV_CBWIDTH_252,
		.cbSel = HAL_DISP_TV_CBSEL_PALM,
		.ampAdj = {
			.luminance = 240,
			.blank = 42,
			.burst = 117,
		},
		.posAdj = {
			.vAct0 = 26,
			.vAct1 = 0,
			.hAct = 32,
		},
		.pColorMatrix = &gNTSCColorMatrix,
	},
	[SP_DISP0_TV_MODE_PAL_NONINTERLACED] =
	{
		.name = "PAL Non-Interlaced",
		.underscanwidth = PAL_UNDERSCAN_WIDTH,
		.underscanheight = (PAL_UNDERSCAN_HEIGHT >> 1),
		.width = PAL_WIDTH,
		.height = (PAL_HEIGHT >> 1),
		.format = HAL_DISP_OUTPUT_FMT_RGB,
		.dmaType = HAL_DISP_TV_DMA_PROGRESSIVE,
		.tvType = HAL_DISP_TV_TYPE_PAL,
		.pulse6 = HAL_DISP_TV_PULSE6_5PULSE,
		.scanSel = HAL_DISP_TV_SCANSEL_NONINTERLACED,
		.fscType = HAL_DISP_TV_FSCTYPE_PALBDGHIN,
		.fix625 = 0,
		.lineSel = HAL_DISP_TV_LINESEL_312_625,
		.cbWidth = HAL_DISP_TV_CBWIDTH_225,
		.cbSel = HAL_DISP_TV_CBSEL_PALBDGHINNC,
		.ampAdj = {
			.luminance = 252,
			.blank = 0,
			.burst = 117,
		},
		.posAdj = {
			.vAct0 = 32,
			.vAct1 = 0,
			.hAct = 52,
		},
		.pColorMatrix = &gPALColorMatrix,
	},
};

#endif // __GP_TV_MODE_TABLE_H__
