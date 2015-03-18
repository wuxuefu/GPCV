/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2013 by Generalplus Inc.                         *
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
 * @file hdmi.c
 * @brief HDMI driver
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/hal/hal_disp.h>
#include <mach/hal/hal_hdmi.h>
#include <mach/gp_hdmi.h>
#include <mach/gp_ti2c_bus.h>
#include <mach/gp_i2c_bus.h>
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
enum {
	HDMI_STATE_REMOVED = 0,
	HDMI_STATE_PLUGED
};

enum {
	HDMI_AUDIO_INFO_FRAME_PKT = 0,
	HDMI_AVI_INFO_FRAME_PKT, 
	HDMI_VENDOR_INFO_FRAME_PKT,
	HDMI_SPD_INFO_FRAME_PKT,
	HDMI_MAX_PACKET, // = 200, // for testing
};

#define HW_I2C			0x01
#define HW_TI2C			0x02
#define I2C_MODE 		HW_I2C

//#define SUPPORT_AUDIO
#ifdef SUPPORT_AUDIO
extern int HDMI_AUDIO_On(int);
extern int HDMI_AUDIO_Off(int);
#define AUDIO_On(int)  HDMI_AUDIO_On(int)
#define AUDIO_Off(int)  HDMI_AUDIO_Off(int)
#else
#define AUDIO_On(int)
#define AUDIO_Off(int)
#endif


/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define ERROR(fmt, arg...) printk(KERN_ERR "[%s:%d] Error! "fmt, __FUNCTION__, __LINE__, ##arg)
#define WARNING(fmt, arg...) printk(KERN_WARNING "[%s:%d] Warning! "fmt, __FUNCTION__, __LINE__, ##arg)
#define MSG(fmt, arg...) printk(KERN_DEBUG "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)
#define INFO(fmt, arg...) printk(KERN_INFO "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)

#define RETURN(x)		{ret = x; goto Return;}
#define CHECK_(x, msg, errid) if(!(x)) {ERROR("%s, %s\n", msg, #x); RETURN(errid);}
#define CHECK(x)		CHECK_(x, "Check failed", -1)
#define CHECK_PRG(x)	CHECK_(x, "Program Error", -EIO)
#define CHECK_VAL(x)	CHECK_(x, "Value Error", -1)

#define HDMI_PACKET_CH 3
#define HDMI_PACKET_CH_INT_MASK (HAL_DISP_INT_PKT0_SEND << HDMI_PACKET_CH)


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct {
	unsigned char type;
	unsigned char version;
	unsigned char length;
	unsigned char flag;
	unsigned char CheckSum;
	unsigned char Body[27];
} gp_hdmi_packet_t;

typedef struct {
	void *RegBase;
	int timing;
	int EnPacketSend;
	int PacketCnt;
	gp_hdmi_packet_t Packet[HDMI_MAX_PACKET];

	int ShowPktUnderRun;
	int PktUnderRunCnt;

	/* Edid */
#if (I2C_MODE == HW_I2C)
	int i2c_ddc;
	int i2c_segment;
#elif (I2C_MODE == HW_TI2C)
	ti2c_set_value_t i2c_ddc;
	ti2c_set_value_t i2c_segment;
#endif
	
	unsigned char Edid[512];

} gp_hdmi_workMem_t;

typedef struct {
	/*	Progressive: 0, Interlaced: 1	*/
	/*	Negative: 0, Positive: 1	*/
	short	PixelClock,	HFreq,	VFreq,	HRes,	VRes,	IsInterlaced;
	short	HTotal, HBlank, HFPorch, HSWidth, HBPorch, HPol;
	short 	VTotal, VBlank, VFPorch, VSWidth, VBPorch, VPol;
	short	Oversampled, VedioID, IsPC;
	unsigned long Phy1, Phy2;
} gp_hdmi_timing_table_t;

//HDMI EDID check ----------------------------------------------------------------------------
typedef struct {
    unsigned short PixelClock;                                     // in 10khz
    unsigned char HorizontalAddressableVideo;                      // in pixels
    unsigned char HorizontalBlanking;                              // in pixels
    unsigned char HorizontalBlanking_upper              : 4;
    unsigned char HorizontalAddressableVideo_upper      : 4;
    unsigned char VerticalAddressableVideo;
    unsigned char VerticalBlanking;
    unsigned char VerticalBlanking_upper                : 4;
    unsigned char VerticalAddressableVideo_upper        : 4;
    unsigned char HorizontalFrontPorch;                            //  in pixels
    unsigned char HorizontalSyncPulseWidth;             
    unsigned char VerticalSyncPulseWidth                    : 4;
    unsigned char VerticalFrontPorch                        : 4;
    unsigned char VerticalSyncPulseWidth_upper              : 2;
    unsigned char VerticalFrontPorch_upper                  : 2;
    unsigned char HorizontalSyncPulseWidth_upper            : 2;
    unsigned char HorizontalFrontPorch_upper                : 2;
    unsigned char HorizontalAddressableVideoImageSize;              // in mm
    unsigned char VerticalAddressableVideoImageSize;                // in mm
    unsigned char VerticalAddressableVideoImageSize_upper   : 4;    // in mm
    unsigned char HorizontalAddressableVideoImageSize_upper : 4;    // in mm
    unsigned char RightHorizontalBorderOrLeftHorizontalBorder;      // in pixels
    unsigned char TopVerticalBorderOrBottomVerticalBorder;          // in Lines
    unsigned char StereoViewingSupport_2                    : 1;
    unsigned char SyncSignalDefinitions                     : 4;
    unsigned char StereoViewingSupport                      : 2;
    unsigned char Interlaced                                : 1;
} vesa_e_edid_timing_mode_t;

typedef struct {
    unsigned long Indicator;
    unsigned char DisplayRangeLimitsOffsets;
    unsigned char MinimumVerticalRate;
    unsigned char MaximumVerticalRate;
    unsigned char MinimumHorizontalRate;
    unsigned char MaximumHorizontalRate;
    unsigned char MaximumPixelClock;
    unsigned char VideoTimingSupportFlags;
    unsigned char VideoTimingData[7];
} vesa_e_edid_display_range_limits_t;

typedef struct {
    int PixelClock;
    int hSize, hBlanking, hFrontPorch, hPluseWidth;
    int vSize, vBlanking, vFrontPorch, vPluseWidth;
} frame_timing_t;

typedef struct {
    int maxPixelClock;          // MHz
    int hMaxRate, hMinRate;     // kHz
    int vMaxRate, vMinRate;     // Hz
} timing_limits_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/


/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/



/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_hdmi_workMem_t gHdmiWorkMem;
static struct semaphore HdmiResourceCnt;
static int HDMI_PixelClk;
static char *pll_sel = "PLL0";
static int pllsel;

static const gp_hdmi_timing_table_t HdmiTiming[HDMI_VIDEO_TIMING_NUM] = {
	//	PxlClk	HFreq	VFreq	HRes	VRes	Intrlac	HTotal	HBlank	HFPorch	HSWidth	HBPorch	HPol	VTotal	VBlank	VFPorch	VSWidth	VBPorch	VPol	Oversmp	ID	IsPC	PHY1		PHY2
	{	2475,	315,	60,		640,	480,	0,		800,	160,	16,		96,		48,		0,		525,	45,		10,		2,		33,		0,		0,		1,	1,		0x10670200,	0x00000010}, 	// 640 x 480 x60
	{	2700,	315,	60,		720,	480,	0,		858,	138,	16,		62,		60,		0,		525,	45,		9,		6,		30,		0,		0,		2,	0,		0x312f0200,	0x00000030},	// 480  p60
	{	7425,	450,	60,		1280,	720,	0,		1650,	370,	110,	40,		220,	1,		750,	30,		5,		5,		20,		1,		0,		4,	0,		0x162C0200,	0x00000010},	// 720  p60
	{	7425,	338,	60,		1920,	1080,	1,		2200,	280,	88,		44,		148,	1,		1125,	22,		2,		5,		15,		1,		0,		5,	0,		0x162C0200,	0x00000010},	// 1080 i60
	{	2700,	157,	60,		1440,	480,	1,		1716,	276,	38,		124,	114,	0,		525,	22,		4,		3,		15,		0,		1,		6,	0,		0x312f0200,	0x00000030},	// 480  i60 (Oversampled)
	{	14850,	675,	60,		1920,	1080,	0,		2200,	280,	88,		44,		148,	1,		1125,	45,		4,		5,		36,		1,		0,		16,	0,		0x12600200,	0x00000010},	// 1080 p60
	{	2700,	313,	50,		720,	576,	0,		864,	144,	12,		64,		68,		0,		625,	49,		5,		5,		39,		0,		0,		17,	0,		0x312f0200,	0x00000030},	// 576  p50
	{	7425,	375,	50,		1280,	720,	0,		1980,	700,	440,	40,		220,	1,		750,	30,		5,		5,		20,		1,		0,		19,	0,		0x162C0200,	0x00000010},	// 720  p50
	{	7425,	281,	50,		1920,	1080,	1,		2640,	720,	528,	44,		148,	1,		1125,	22,		2,		5,		15,		1,		0,		20,	0,		0x162C0200,	0x00000010},	// 1080 i50
	{	2700,	157,	50,		1440,	576,	1,		1728,	288,	24,		126,	138,	0,		625,	24,		2,		3,		19,		0,		1,		21,	0,		0x312f0200,	0x00000030},	// 576  i50 (Oversampled)
	{	14850,	563,	50,		1920,	1080,	0,		2640,	720,	528,	44,		148,	1,		1125,	45,		4,		5,		36,		1,		0,		31,	0,		0x12600200,	0x00000010},	// 1080 p50
};

static const unsigned short RGBColorMatrix[] = {
	/*	RGB color space		*/
	0x100,	0,		0,
	0,		0x100,	0,
	0,		0,		0x100,
	0,		0,		0,
};

static const unsigned short YCbCrColorMatrix[] = {
	/*	RGB to YCbCr color space, SDTV		*/
	0x4C,	0x96,	0x1d,
	0xffd5,	0xffac,	0x7f,
	0x7f,	0xff95,	0xffec,
	0x1000,	0x7FC0,	0x7FC0,
};

static const gp_hdmi_packet_t HdmiPackets[HDMI_MAX_PACKET] = {
	[HDMI_AUDIO_INFO_FRAME_PKT] = {
		.length		= 10,
		.version	= 1,
		.type		= HDMI_AUDIOINFOFRAME_PACKET,
		.Body = {
			0x00,	0x00,	0x00,	0x1F,
			0x00,	0x00,	0x00,	0x00,
			0x00,	0x00
		},
	},
	[HDMI_AVI_INFO_FRAME_PKT] = {
		.length		= 13,
		.version	= 2,
		.type		= HDMI_AVIINFOFRAME_PACKET,
		.Body = {
			(0 /*ColorSpace*/ << 5) | (HDMI_ACTIVEFORMAT_PRESENT << 4) | HDMI_BARNOTPRESENT << 2 | HDMI_UNDERSCANNED, // 
			HDMI_ASPECTRATIO_SAME,
			0x00,
			0x00 /*VideoIdCode*/,
			0x00 /*PixelRep*/,
			0x00,	0x00,	0x00,	0x00,
			0x00,	0x00,	0x00,	0x00
		},
	},
	[HDMI_VENDOR_INFO_FRAME_PKT] = {
		.length		= 4,
		.version	= 1,
		.type		= HDMI_VENDORINFOFRAME_PACKET,
		.Body = {
			0x03,	0x0C,	0x00, 	0x00
		},
	},
	[HDMI_SPD_INFO_FRAME_PKT] = {
		.length		= 25,
		.version	= 1,
		.type		= HDMI_SPDINFOFRAME_PACKET,
		.Body = {
			0x47,	0x50,	0x6c,	0x75,
			0x73,	0x00,	0x00,	0x00,
			0x47,	0x50,	0x4d,	0x50,
			0x38,	0x33,	0x30,	0x30,
			0x20,	0x50,	0x4d,	0x50,
			0x00,	0x00,	0x00,	0x00,
			0x0d
		},
	},
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
static void hdmi_timing_init(void *RegBase, int timing)
{
	int bcolor_format;		// 0: RGB, 1: YCbCr422, 2: YCbCr444
	int h_filed_offset;
	int hblank, vback;

	MSG("timing = %d\n", timing);
	
	bcolor_format = 0;		// 0: RGB, 1: YCbCr422, 2: YCbCr444

	// This flag MUST be set to be 1 to make enough time slot for audio Tx
	// contact L.C Hsu if you need more infomation
	gpHalDispHDMISetAudioTimingSlot(RegBase, 1);

	gpHalDispSetDeflickerInfo(RegBase, 0, 0, 0, 0);
	if (HdmiTiming[timing].IsInterlaced) {
		gpHalDispSetPriDmaType(RegBase, HAL_DISP_TV_DMA_INTERLACED);
		gpHalDispSetOsdDmaType(RegBase, 0, HAL_DISP_TV_DMA_INTERLACED);
		gpHalDispSetOsdDmaType(RegBase, 1, HAL_DISP_TV_DMA_INTERLACED);
		gpHalDispSetTvScan(RegBase, HAL_DISP_TV_SCANSEL_INTERLACED);	
	}
	else {
		gpHalDispSetPriDmaType(RegBase, HAL_DISP_TV_DMA_PROGRESSIVE);
		gpHalDispSetOsdDmaType(RegBase, 0, HAL_DISP_TV_DMA_PROGRESSIVE);
		gpHalDispSetOsdDmaType(RegBase, 1, HAL_DISP_TV_DMA_PROGRESSIVE);
		gpHalDispSetTvScan(RegBase, HAL_DISP_TV_SCANSEL_PROGRESSIVE);
	}

	gpHalDispSetRes(RegBase, HdmiTiming[timing].HRes, HdmiTiming[timing].VRes);
	
	if (HdmiTiming[timing].IsPC) {
		gpHalDispSetPanelFormat(RegBase, HAL_DISP_OUTPUT_FMT_RGB, HAL_DISP_OUTPUT_TYPE_PRGB888, 0, 0);
		gpHalDispSetColorMatrix(RegBase, RGBColorMatrix);
	}
	else {
		switch (bcolor_format)
		{
			case 0:
				gpHalDispSetPanelFormat(RegBase, HAL_DISP_OUTPUT_FMT_RGB, HAL_DISP_OUTPUT_TYPE_PRGB888, 0, 0);
				gpHalDispSetColorMatrix(RegBase, RGBColorMatrix);
				break;
			case 1:
				gpHalDispSetPanelFormat(RegBase, HAL_DISP_OUTPUT_FMT_YCbCr, HAL_DISP_OUTPUT_TYPE_YCbCr16, 0, 0);
				gpHalDispSetColorMatrix(RegBase, YCbCrColorMatrix);
				break;
			case 2:
			default:
				gpHalDispSetPanelFormat(RegBase, HAL_DISP_OUTPUT_FMT_YCbCr, HAL_DISP_OUTPUT_TYPE_YCbCr24, 0, 0);
				gpHalDispSetColorMatrix(RegBase, YCbCrColorMatrix);
				break;
		}
	}

	gpHalDispHDMIPHYConfig(RegBase, HdmiTiming[timing].Phy1, HdmiTiming[timing].Phy2);

	gpHalDispSetBlankingIntervalTo0(RegBase, 1);	

	// H-Sync timing
	{
		lcdTiming_t hsync = 
		{
			.bPorch		= HdmiTiming[timing].HBPorch + HdmiTiming[timing].HSWidth,
			.fPorch		= HdmiTiming[timing].HFPorch,
			.polarity	= HdmiTiming[timing].HPol,
			.width		= HdmiTiming[timing].HSWidth,
		};
		//if(HdmiTiming[timing].Oversampled) gpHalHDMISetPixelRepetition(RegBase, HAL_HDMI_PIXEL_REP_2TIMES);
		//else gpHalHDMISetPixelRepetition(RegBase, HAL_HDMI_PIXEL_REP_1TIMES);
		gpHalHDMISetPixelRepetition(RegBase, HAL_HDMI_PIXEL_REP_1TIMES);
		
		gpHalDispSetLcdHsync(RegBase, hsync);
		gpHalDispHDMISetHPolarity(RegBase, hsync.polarity);
	}

	// V-Sync timing
	{
		lcdTiming_t vsync = {
			.bPorch		= HdmiTiming[timing].VBPorch + HdmiTiming[timing].VSWidth,
			.fPorch		= HdmiTiming[timing].VFPorch,
			.polarity	= HdmiTiming[timing].VPol,
			.width		= HdmiTiming[timing].VSWidth,
		};
		gpHalDispSetLcdVsync(RegBase, vsync);
		gpHalDispHDMISetVPolarity(RegBase, vsync.polarity);
	}

	// interlaced or not
	if (HdmiTiming[timing].IsInterlaced) {
		//set H-Sync offset, if interlaced mode
		// if(HdmiTiming[timing].Oversampled) h_filed_offset = HdmiTiming[timing].HTotal;
		// else 
		h_filed_offset = HdmiTiming[timing].HTotal >> 1;
	}
	else {
		h_filed_offset = 0;
	}
	gpHalDispSetLcdTiming(RegBase, HdmiTiming[timing].IsInterlaced, h_filed_offset, HdmiTiming[timing].VTotal);

	// Set TimeCycle Register
	vback = HdmiTiming[timing].HTotal * (HdmiTiming[timing].VBlank - 1);
	hblank = HdmiTiming[timing].HBlank;
	gpHalHDMISetTimeCycle(RegBase, vback, hblank);
}

static int hdmi_get_edid(void)
{
	gp_hdmi_workMem_t *WorkMem = &gHdmiWorkMem;
	int ret = 0;
	int i, j;
	int check_sum;
	int segment, addr;
	int SegmentPointSupport = 1;
	const unsigned char *ptr;
	unsigned char Edid1[512];

	memset(Edid1, 0, sizeof(Edid1));

	// set segment to 0x0
	segment = 0;
#if(I2C_MODE == HW_TI2C)	
	WorkMem->i2c_segment.transmitMode = TI2C_NORMAL_WRITE_MODE;
	WorkMem->i2c_segment.pBuf = (void*)&segment;
	WorkMem->i2c_segment.dataCnt = 1;
	if(gp_ti2c_bus_xfer(&WorkMem->i2c_segment) < 0) {
		SegmentPointSupport = 0;
	}
#elif(I2C_MODE == HW_I2C)
	if(gp_i2c_bus_write(WorkMem->i2c_segment, (void*)&segment, 1) < 0){
		SegmentPointSupport = 0;
	}	
#endif
	// reset address
	addr = 0;
#if(I2C_MODE == HW_TI2C)
	WorkMem->i2c_ddc.transmitMode = TI2C_NORMAL_WRITE_MODE;
	WorkMem->i2c_ddc.pBuf = (void*)&addr;
	WorkMem->i2c_ddc.dataCnt = 1;
	CHECK(gp_ti2c_bus_xfer(&WorkMem->i2c_ddc) >= 0);

	//Read Edid
	WorkMem->i2c_ddc.transmitMode = TI2C_NORMAL_READ_MODE;
	WorkMem->i2c_ddc.pBuf = Edid1;
	WorkMem->i2c_ddc.dataCnt = 256;
	CHECK(gp_ti2c_bus_xfer(&WorkMem->i2c_ddc) >= 0);
#elif(I2C_MODE == HW_I2C)
	CHECK(gp_i2c_bus_write(WorkMem->i2c_ddc, (void*)&addr, 1)>=0);
	CHECK(gp_i2c_bus_read(WorkMem->i2c_ddc,Edid1,256)>=0);
#endif

	if (SegmentPointSupport && Edid1[128] == 0xF0) {
		ptr = Edid1 + 128;
		for (i=1; i<127; i++) {
			if (ptr[i] == 0x02) {
				// set segment
				segment = (i + 1) >> 1;
#if(I2C_MODE == HW_TI2C)			
				WorkMem->i2c_segment.transmitMode = TI2C_NORMAL_WRITE_MODE;
				WorkMem->i2c_segment.pBuf = (void*)&segment;
				WorkMem->i2c_segment.dataCnt = 1;
				CHECK(gp_ti2c_bus_xfer(&WorkMem->i2c_segment) >= 0);
#elif(I2C_MODE == HW_I2C)
				CHECK(gp_i2c_bus_write(WorkMem->i2c_segment, (void*)&segment, 1)>=0);
#endif
				// reset address
				addr = ((i + 1) & 1) << 7;
#if(I2C_MODE == HW_TI2C)
				WorkMem->i2c_ddc.transmitMode = TI2C_NORMAL_WRITE_MODE;
				WorkMem->i2c_ddc.pBuf = (void*)&addr;
				WorkMem->i2c_ddc.dataCnt = 1;
				CHECK(gp_ti2c_bus_xfer(&WorkMem->i2c_ddc) >= 0);

				//Read Edid
				WorkMem->i2c_ddc.transmitMode = TI2C_NORMAL_READ_MODE;
				WorkMem->i2c_ddc.pBuf = Edid1 + 128;
				WorkMem->i2c_ddc.dataCnt = 128;
				CHECK(gp_ti2c_bus_xfer(&WorkMem->i2c_ddc) >= 0);
#elif(I2C_MODE == HW_I2C)
				CHECK(gp_i2c_bus_write(WorkMem->i2c_ddc, (void*)&addr, 1)>=0);
				CHECK(gp_i2c_bus_read(WorkMem->i2c_ddc,Edid1+128,128)>=0);
#endif
				break;
			}
		}
	}

	memcpy(WorkMem->Edid, Edid1, sizeof(Edid1));

	ptr = Edid1;
	for (j=0; j<2; j++) {
		check_sum = 0;
		for (i=0; i<128; i++) check_sum += *ptr++;
		check_sum &= 0xFF;
		MSG("Block[%d] check_sum = %d\n", j, check_sum);
		if(check_sum) RETURN(-EIO);
	}

Return:
	return ret;
}

int hdmi_get_supported(const unsigned char *edid, unsigned int *flag_supported)
{
    int i;
    const unsigned char *ptr;
    timing_limits_t limits = {0};
    int flag_timing_limits = 0;

    MSG("EDID Version = %d.%d\n", edid[0x12], edid[0x13]);
    ptr = edid + 0x36;

	for(i=0;i<4;i++) {
		if (ptr[0] != 0 || ptr[1] != 0) {	// check if timing descriptor
			const vesa_e_edid_timing_mode_t *VesaTiming = (const vesa_e_edid_timing_mode_t*)ptr;
			frame_timing_t frame_timing;

			frame_timing.PixelClock   = VesaTiming->PixelClock;

			frame_timing.hSize        = VesaTiming->HorizontalAddressableVideo | (VesaTiming->HorizontalAddressableVideo_upper << 8);
			frame_timing.hBlanking    = VesaTiming->HorizontalBlanking | (VesaTiming->HorizontalBlanking_upper << 8);
			frame_timing.hFrontPorch  = VesaTiming->HorizontalFrontPorch | (VesaTiming->HorizontalFrontPorch_upper << 8);
			frame_timing.hPluseWidth  = VesaTiming->HorizontalSyncPulseWidth | (VesaTiming->HorizontalSyncPulseWidth_upper << 8);

			frame_timing.vSize        = VesaTiming->VerticalAddressableVideo | (VesaTiming->VerticalAddressableVideo_upper << 8);
			frame_timing.vBlanking    = VesaTiming->VerticalBlanking | (VesaTiming->VerticalBlanking_upper << 8);
			frame_timing.vFrontPorch  = VesaTiming->VerticalFrontPorch | (VesaTiming->VerticalFrontPorch_upper << 4);
			frame_timing.vPluseWidth  = VesaTiming->VerticalSyncPulseWidth | (VesaTiming->VerticalSyncPulseWidth_upper << 4);

			if(i == 0) MSG("========= Preffered frame_timing =========\n");
			else       MSG("============ frame_timing #%d =============\n", i);
			MSG("Pixel clock = %3.2lf MHz\n", frame_timing.PixelClock * 0.01);
			MSG("           Addressable Blanking FrontPorch PulseWidth \n");
			MSG("Horizontal    %4d        %4d     %4d      %4d    (pixels)\n",
			    frame_timing.hSize,  frame_timing.hBlanking,   frame_timing.hFrontPorch, frame_timing.hPluseWidth);
			MSG("Vertical      %4d        %4d     %4d      %4d    (lines)\n",
			    frame_timing.vSize,  frame_timing.vBlanking,   frame_timing.vFrontPorch, frame_timing.vPluseWidth);
		}
		else if (ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 0 && ptr[3] == 0xFD) {	// Display Range Limits
			const vesa_e_edid_display_range_limits_t *VesaLimits = (const vesa_e_edid_display_range_limits_t*)ptr;
			int hMaxOffset = 0, hMinOffset = 0;
			int vMaxOffset = 0, vMinOffset = 0;
			if(flag_timing_limits == 0) {
				if(VesaLimits->DisplayRangeLimitsOffsets & 1) vMinOffset = 255;
				if(VesaLimits->DisplayRangeLimitsOffsets & 2) vMaxOffset = 255;
				if(VesaLimits->DisplayRangeLimitsOffsets & 4) hMinOffset = 255;
				if(VesaLimits->DisplayRangeLimitsOffsets & 8) hMaxOffset = 255;

				limits.maxPixelClock    = VesaLimits->MaximumPixelClock * 10;
				limits.hMaxRate         = VesaLimits->MaximumHorizontalRate + hMaxOffset;
				limits.hMinRate         = VesaLimits->MinimumHorizontalRate + hMinOffset;
				limits.vMaxRate         = VesaLimits->MaximumVerticalRate + vMaxOffset;
				limits.vMinRate         = VesaLimits->MinimumVerticalRate + vMinOffset;

				MSG("========= Timing Limits =========\n");
				MSG("Vertical Rate       = %3d ~ %3d Hz\n", limits.vMinRate, limits.vMaxRate);
				MSG("Horizontal Rate     = %3d ~ %3d kHz\n", limits.hMinRate, limits.hMaxRate);
				MSG("Maximum Pixel Clock = %3d MHz\n", limits.maxPixelClock);
				flag_timing_limits = 1;
			}
		}
		ptr += 18;
	}

	if (flag_timing_limits) {
		int val = 0;
		if(flag_supported) {
			for(i=0;i<sizeof(HdmiTiming)/sizeof(HdmiTiming[0]);i++) {
				if( HdmiTiming[i].PixelClock <= limits.maxPixelClock * 100 &&
					HdmiTiming[i].HFreq <= limits.hMaxRate * 10 &&
					HdmiTiming[i].HFreq >= limits.hMinRate * 10 &&
					HdmiTiming[i].VFreq <= limits.vMaxRate &&
					HdmiTiming[i].VFreq >= limits.vMinRate)
					val |= 1 << i;
			}
			*flag_supported = val;
		}

		return 0;
	}
	else {
		return -1;
	}
}

void hdmi_set_audio_clk(void *RegBase, int AudioClk)
{
	int PixelClk = HDMI_PixelClk;
	int N, CTS;

	MSG("PixelClk = %d AudioClk = %d\n", PixelClk, AudioClk);

	CTS = PixelClk / 1000;

	switch(AudioClk)
	{
	case 6000:	N = 768; break;
	case 8000:	N = 1024; break;
	case 11025:	N = 1568; CTS = PixelClk / 900; break;
	case 12000:	N = 1536; break;
	case 16000:	N = 2048; break;
	case 22050:	N = 3136; CTS = PixelClk / 900; break;
	case 24000:	N = 3072; break;
	case 32000:	N = 4096; break;
	case 44100:	N = 6272; CTS = PixelClk / 900; break;
	case 48000:	N = 6144; break;
	default:
		ERROR("AudioClk (%d) not found\n", AudioClk);
		return;	
	}

	MSG("N = %d CTS = %d\n", N, CTS);
	gpHalHDMISetACRPacket(RegBase, N, CTS);
}
EXPORT_SYMBOL(hdmi_set_audio_clk);

static void hdmi_config_packet(void *hHdmi, int ColorSpace, int PixelRep, int VideoIdCode)
{
	int i;
	gp_hdmi_workMem_t *WorkMem = (gp_hdmi_workMem_t*)hHdmi;
	gp_hdmi_packet_t *Pkt = WorkMem->Packet;
	MSG("\n");

	memcpy(WorkMem->Packet, HdmiPackets, sizeof(HdmiPackets));
	WorkMem->Packet[HDMI_AVI_INFO_FRAME_PKT].Body[0] |= ColorSpace << 5;
	WorkMem->Packet[HDMI_AVI_INFO_FRAME_PKT].Body[3] |= VideoIdCode;
	WorkMem->Packet[HDMI_AVI_INFO_FRAME_PKT].Body[4] |= PixelRep;
	for(i=0;i<HDMI_MAX_PACKET;i++) {
		int CheckSum = 0, j;
		const unsigned char *body;

		body = Pkt->Body;
		CheckSum -= Pkt->type + Pkt->version + Pkt->length;
		for(j=0;j<Pkt->length;j++)
			CheckSum -= *body++;
		Pkt->CheckSum = (unsigned char)(CheckSum & 0xFF);
		Pkt++;
	}	
	gpHalHDMISetAudioSamplePacket(WorkMem->RegBase, 2);
	hdmi_set_audio_clk(WorkMem->RegBase, 44100);
}

static void *HDMI_Open(void *RegBase)
{
	MSG("\n");

	if(down_trylock(&HdmiResourceCnt) != 0)
		return 0;

	gHdmiWorkMem.RegBase=RegBase;
	gHdmiWorkMem.PktUnderRunCnt = 0;
	gpHalDispSetEnable(RegBase, HAL_DISP_DEV_HDMI, 1);

	AUDIO_On(0);

	return &gHdmiWorkMem;
}

static void HDMI_Close(void *hHdmi)
{
	gp_hdmi_workMem_t *WorkMem = (gp_hdmi_workMem_t*)hHdmi;

	AUDIO_Off(0);
    gpHalDispHDMIPHYConfig(WorkMem->RegBase, HdmiTiming[WorkMem->timing].Phy1 | 1, HdmiTiming[WorkMem->timing].Phy2);

	WorkMem->RegBase = 0;
	up(&HdmiResourceCnt);
}

static int HDMI_SetTiming(void *hHdmi, struct clk *pClk, int timing)
{
	gp_hdmi_workMem_t *WorkMem = (gp_hdmi_workMem_t*)hHdmi;

	MSG("timing = %d\n", timing);

	if(timing < 0 || timing >= HDMI_VIDEO_TIMING_NUM)
		timing = HDMI_720X480P60;

    // Set on clock
	//clk_set_rate(pClk, (unsigned long) 10000 * HdmiTiming[timing].PixelClock);
	gpHalDispSetClock((unsigned int) 10000 * HdmiTiming[timing].PixelClock, pllsel);
	HDMI_PixelClk = clk_get_rate(pClk);
	MSG("hdmi_clk_rate = %d\n", HDMI_PixelClk);

	hdmi_timing_init(WorkMem->RegBase, timing);

	WorkMem->timing = timing;
	hdmi_config_packet(WorkMem, 0, HdmiTiming[timing].Oversampled, HdmiTiming[timing].VedioID);

	WorkMem->PacketCnt = 0;
	WorkMem->EnPacketSend = 0;

	return timing;
}

static int HDMI_OnResume(void *hHdmi)
{
	MSG("\n");
	gp_hdmi_workMem_t *WorkMem = (gp_hdmi_workMem_t*)hHdmi;

	gpHalDispSetEnable(WorkMem->RegBase, HAL_DISP_DEV_HDMI, 1);

	AUDIO_On(0);
	return 0;
}

static void HDMI_OnInterrupt(void *hHdmi, int intFlag)
{
 	gp_hdmi_workMem_t *WorkMem = (gp_hdmi_workMem_t*)hHdmi;

	if(intFlag & HAL_DISP_INT_FRAME_END) {
		// Stop Sending Packet
		WorkMem->EnPacketSend = 0;
	}

	if(intFlag & HDMI_PACKET_CH_INT_MASK) {
		if(WorkMem->EnPacketSend) {
			if(WorkMem->PacketCnt < HDMI_MAX_PACKET) {
				gpHalHDMISendPacket(WorkMem->RegBase,
					HDMI_PACKET_CH,
					&WorkMem->Packet[WorkMem->PacketCnt++],
					HAL_HDMI_PKT_HVLD,
					HAL_HDMI_PKT_SEND);
			}
		}
		else {
			if(WorkMem->ShowPktUnderRun) WARNING("Oops! HDMI Packet Tx underrun\n");
			else MSG("Oops! HDMI Packet Tx underrun\n");
			WorkMem->PktUnderRunCnt++;
		}
	}

	// ACR is the 1st or 2nd packet that be sent after v-sync
	// use ACR packet to trigger other packets sending
	// that can make sure packets send after v-sync
	if(intFlag & HAL_DISP_INT_ACR_SEND) {
		WorkMem->PacketCnt = 0;
		WorkMem->EnPacketSend = 1;
		gpHalHDMISendPacket(WorkMem->RegBase,
			HDMI_PACKET_CH,
			&WorkMem->Packet[WorkMem->PacketCnt++],
			HAL_HDMI_PKT_HVLD,
			HAL_HDMI_PKT_SEND);
	}	
}

static int HDMI_Backdoor(void *hHdmi, const char *backdoor)
{
	int ret = 0;
	gp_hdmi_workMem_t *WorkMem = (gp_hdmi_workMem_t*)hHdmi;
	CHECK_VAL(WorkMem != 0);
	CHECK_VAL(backdoor != 0);

	if (strncmp(backdoor, "ShowPktUnderRun=0", 18) == 0) {
		WorkMem->ShowPktUnderRun = 0;
	}
	else if(strncmp(backdoor, "ShowPktUnderRun=1", 18) == 0) {
		WorkMem->ShowPktUnderRun = 1;
	}
	else if(strncmp(backdoor, "Status", 18) == 0) {
		INFO("PktUnderRunCnt = %d\n", WorkMem->PktUnderRunCnt);
	}
Return:
	return ret;
}

static int HDMI_GetSupportMode(unsigned int *mode)
{
	int ret = 0;

	/* Get EDID */
	CHECK(hdmi_get_edid() >= 0);

	/* Get support mode */
	CHECK(hdmi_get_supported(gHdmiWorkMem.Edid, mode) >= 0);

	MSG("supportMode=0x%x\n", *mode);

Return:
	return ret;
}

static int HDMI_GetPixelSize(void *Inst, gp_disp_pixelsize_t *size)
{
	size->width = 16;
	size->height = 9;
}

static const gp_disp_drv_op HDMI_DrvOp = 
{
	.Owner			= THIS_MODULE,
	.Name			= "HDMI",
	.Type			= SP_DISP_OUTPUT_HDMI,
	.Open			= HDMI_Open,
	.Close			= HDMI_Close,
	.SetTiming		= HDMI_SetTiming,
	.OnResume		= HDMI_OnResume,
	.InterruptMask	= HAL_DISP_INT_FRAME_END | HAL_DISP_INT_ACR_SEND | HDMI_PACKET_CH_INT_MASK,
	.OnInterrupt	= HDMI_OnInterrupt,
	.Backdoor		= HDMI_Backdoor,
	.GetSupportMode	= HDMI_GetSupportMode,
	.GetPixelSize	= HDMI_GetPixelSize,
};

//-----------------------------------------------------------------------------
static int __init module_init_hdmi(void)
{
	int ret = 0;
	MSG("\n");
	
	if(strncmp(pll_sel, "PLL2", 5) == 0)
		pllsel = 2;
	else
		pllsel = 0;
	
#ifdef CONFIG_FPGA_TEST
	{
		volatile unsigned int *reg = (volatile unsigned int *) 0xfc00518c;
		unsigned int regVal = *reg;
		regVal |= 0x02;
		*reg = regVal;

		// lcd_type_sel
		reg = (volatile unsigned int *) 0xfc807010;
		*reg = 0x00000000;
	}
#endif
	memset(&gHdmiWorkMem, 0, sizeof(gp_hdmi_workMem_t));
	sema_init(&HdmiResourceCnt, 1);

	/* i2c bus request */
#if (I2C_MODE == HW_TI2C)	
	gHdmiWorkMem.i2c_ddc.pDeviceString = "HDMI DDC";
	gHdmiWorkMem.i2c_ddc.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;
	gHdmiWorkMem.i2c_ddc.slaveAddr = 0xa0;
	gHdmiWorkMem.i2c_ddc.clockRate = 80;
	if (gp_ti2c_bus_request(&gHdmiWorkMem.i2c_ddc) != 0) {
		ERROR("DDC channel open failed\n");
		RETURN(-EIO);
	}
#elif(I2C_MODE == HW_I2C)	
	gHdmiWorkMem.i2c_ddc = gp_i2c_bus_request(0xa0,80);
	if((gHdmiWorkMem.i2c_ddc == 0) ||(gHdmiWorkMem.i2c_ddc == -ENOMEM)) {
		ERROR("DDC channel open failed\n");
		RETURN(-EIO);		
	}
#endif

#if (I2C_MODE == HW_TI2C)	
	gHdmiWorkMem.i2c_segment.pDeviceString = "HDMI DDC Segment";
	gHdmiWorkMem.i2c_segment.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;
	gHdmiWorkMem.i2c_segment.slaveAddr = 0x60;
	gHdmiWorkMem.i2c_segment.clockRate = 80;
	if (gp_ti2c_bus_request(&gHdmiWorkMem.i2c_segment) != 0) {
		ERROR("DDC Segment channel open failed\n");
		gp_ti2c_bus_release(&gHdmiWorkMem.i2c_ddc);
		RETURN(-EIO);
	}
#elif(I2C_MODE == HW_I2C)	
	gHdmiWorkMem.i2c_segment = gp_i2c_bus_request(0x60,80);
	if((gHdmiWorkMem.i2c_segment == 0) ||(gHdmiWorkMem.i2c_segment == -ENOMEM)) {
		ERROR("DDC Segment channel open failed\n");
		gp_i2c_bus_release(gHdmiWorkMem.i2c_ddc);
		RETURN(-EIO);		
	}
#endif

	CHECK_PRG(register_display(&HDMI_DrvOp) >= 0);

Return:
	return ret;
}

static void __exit module_exit_hdmi(void)
{
	MSG("\n");
	unregister_display(&HDMI_DrvOp);
}

module_param(pll_sel, charp, S_IRUGO);

/* Declaration of the init and exit functions */
module_init(module_init_hdmi);
module_exit(module_exit_hdmi);

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP HDMI Driver");
MODULE_LICENSE_GP;

