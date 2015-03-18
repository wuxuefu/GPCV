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
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/
#ifndef _REG_CDSP_H_
#define _REG_CDSP_H_
	
#include <mach/hardware.h>
#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define	LOGI_ADDR_CDSP_REG		IO2_ADDRESS(0x14000)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct cdspReg0_s               /* 0x92014000 ~ 0x920140FF */
{
	volatile UINT8 cdspMacroSel;          /* LOGI_ADDR_CDSP_REG+0x000 */
	volatile UINT8 cdspMacroPagSel;       /* LOGI_ADDR_CDSP_REG+0x001 */
	volatile UINT8 cdspCPUSramEn;	      /* LOGI_ADDR_CDSP_REG+0x002 */
	volatile UINT8 reserved003;	      /* LOGI_ADDR_CDSP_REG+0x003 */
	volatile UINT8 cdspSwitchClk;         /* LOGI_ADDR_CDSP_REG+0x004 */
	volatile UINT8 reserved005[2];        /* LOGI_ADDR_CDSP_REG+0x005 */
	volatile UINT8 cdspBadPixMir;         /* LOGI_ADDR_CDSP_REG+0x007 */
	volatile UINT8 cdspBadPixRThr;        /* LOGI_ADDR_CDSP_REG+0x008 */
	volatile UINT8 cdspBadPixGThr;        /* LOGI_ADDR_CDSP_REG+0x009 */
	volatile UINT8 cdspBadPixBThr;        /* LOGI_ADDR_CDSP_REG+0x00A */
	volatile UINT8 cdspBadPixEn;          /* LOGI_ADDR_CDSP_REG+0x00B */
	volatile UINT8 reserved00C;			      /* LOGI_ADDR_CDSP_REG+0x00C */
	volatile UINT8 cdspYuvSpEfYOff;	      /* LOGI_ADDR_CDSP_REG+0x00D */
	volatile UINT8 cdspYuvSpEfUOff;	      /* LOGI_ADDR_CDSP_REG+0x00E */
	volatile UINT8 cdspYuvSpEfVOff;	      /* LOGI_ADDR_CDSP_REG+0x00F */

	volatile UINT8 cdspImgType;		        /* LOGI_ADDR_CDSP_REG+0x010 */
	volatile UINT8 cdspYuvSpEfYScl;	      /* LOGI_ADDR_CDSP_REG+0x011 */
	volatile UINT8 cdspYuvSpEfUScl;	      /* LOGI_ADDR_CDSP_REG+0x012 */
	volatile UINT8 cdspYuvSpEfVScl;	      /* LOGI_ADDR_CDSP_REG+0x013 */
	volatile UINT8 cdspManuObVal[2];      /* LOGI_ADDR_CDSP_REG+0x014 */
	volatile UINT8 cdspManuObEn;		      /* LOGI_ADDR_CDSP_REG+0x016 */
	volatile UINT8 cdspYuvSpEfBiThr;  	  /* LOGI_ADDR_CDSP_REG+0x017 */
	volatile UINT8 cdspAutoObCtrl;        /* LOGI_ADDR_CDSP_REG+0x018 */
	volatile UINT8 reserved019[3];        /* LOGI_ADDR_CDSP_REG+0x019 */
	volatile UINT8 cdspAutoObHOff[2];     /* LOGI_ADDR_CDSP_REG+0x01C */
	volatile UINT8 cdspAutoObVOff[2];     /* LOGI_ADDR_CDSP_REG+0x01E */

	volatile UINT8 cdspAutoObRAvg[2];     /* LOGI_ADDR_CDSP_REG+0x020 */
	volatile UINT8 cdspDummy0;			      /* LOGI_ADDR_CDSP_REG+0x022 */
	volatile UINT8 cdspDummy1;			      /* LOGI_ADDR_CDSP_REG+0x023 */
	volatile UINT8 cdspAutoObGrAvg[2];    /* LOGI_ADDR_CDSP_REG+0x024 */
	volatile UINT8 cdspDummy2;			      /* LOGI_ADDR_CDSP_REG+0x026 */
	volatile UINT8 cdspDummy3;			      /* LOGI_ADDR_CDSP_REG+0x027 */
	volatile UINT8 cdspAutoObBAvg[2];     /* LOGI_ADDR_CDSP_REG+0x028 */
	volatile UINT8 cdspDummy4;			      /* LOGI_ADDR_CDSP_REG+0x02A */
	volatile UINT8 cdspDummy5;			      /* LOGI_ADDR_CDSP_REG+0x02B */
	volatile UINT8 cdspAutoObGbAvg[2];    /* LOGI_ADDR_CDSP_REG+0x02C */
	volatile UINT8 cdspDummy6;			      /* LOGI_ADDR_CDSP_REG+0x02E */
	volatile UINT8 cdspDummy7;			      /* LOGI_ADDR_CDSP_REG+0x02F */

	volatile UINT8 cdspLensCmpCtrl;	      /* LOGI_ADDR_CDSP_REG+0x030 */
	volatile UINT8 cdspVValid;			      /* LOGI_ADDR_CDSP_REG+0x031 */
	volatile UINT8 cdspLensCmpXmOff[2];   /* LOGI_ADDR_CDSP_REG+0x032 */
	volatile UINT8 cdspLensCmpXmInc;	    /* LOGI_ADDR_CDSP_REG+0x034 */
	volatile UINT8 reserved035;           /* LOGI_ADDR_CDSP_REG+0x035 */
	volatile UINT8 cdspLensCmpYmOff[2];	  /* LOGI_ADDR_CDSP_REG+0x036 */
	volatile UINT8 reserved038;           /* LOGI_ADDR_CDSP_REG+0x038 */
	volatile UINT8 cdspLensCmpYmInc;	    /* LOGI_ADDR_CDSP_REG+0x039 */
	volatile UINT8 cdspRawHSclIniEve[2]; 	/* LOGI_ADDR_CDSP_REG+0x03A */
	volatile UINT8 cdspLensCmpXCent[2];   /* LOGI_ADDR_CDSP_REG+0x03C */
	volatile UINT8 cdspLensCmpYCent[2];   /* LOGI_ADDR_CDSP_REG+0x03E */

	volatile UINT8 cdspRawHSclFactor[2];  /* LOGI_ADDR_CDSP_REG+0x040 */
	volatile UINT8 cdspRawHSclIniOdd[2]; 	/* LOGI_ADDR_CDSP_REG+0x042 */
	volatile UINT8 cdspYuvVSclFactor[2];  /* LOGI_ADDR_CDSP_REG+0x044 */
	volatile UINT8 cdspYuvVSclIni[2];     /* LOGI_ADDR_CDSP_REG+0x046 */
	volatile UINT8 cdspYuvHSclFactor[2];  /* LOGI_ADDR_CDSP_REG+0x048 */
	volatile UINT8 cdspYuvHSclIni[2];     /* LOGI_ADDR_CDSP_REG+0x04A */
	volatile UINT8 cdspRawHSclCtrl;	      /* LOGI_ADDR_CDSP_REG+0x04C */
	volatile UINT8 cdspYuvVSclCtrl;	      /* LOGI_ADDR_CDSP_REG+0x04D */
	volatile UINT8 cdspYuvHSclCtrl;	      /* LOGI_ADDR_CDSP_REG+0x04E */
	volatile UINT8 cdspSclUpdateMode;	    /* LOGI_ADDR_CDSP_REG+0x04F */

	volatile UINT8 cdspWbRGain[2];        /* LOGI_ADDR_CDSP_REG+0x050 */
	volatile UINT8 cdspWbROff;            /* LOGI_ADDR_CDSP_REG+0x052 */
	volatile UINT8 reserved053;           /* LOGI_ADDR_CDSP_REG+0x053 */
	volatile UINT8 cdspWbGrGain[2];       /* LOGI_ADDR_CDSP_REG+0x054 */
	volatile UINT8 cdspWbGrOff;           /* LOGI_ADDR_CDSP_REG+0x056 */
	volatile UINT8 reserved057;           /* LOGI_ADDR_CDSP_REG+0x057 */
	volatile UINT8 cdspWbBGain[2];        /* LOGI_ADDR_CDSP_REG+0x058 */
	volatile UINT8 cdspWbBOff;            /* LOGI_ADDR_CDSP_REG+0x05A */
	volatile UINT8 reserved05B;           /* LOGI_ADDR_CDSP_REG+0x05B */
	volatile UINT8 cdspWbGbGain[2];       /* LOGI_ADDR_CDSP_REG+0x05C */
	volatile UINT8 cdspWbGbOff;           /* LOGI_ADDR_CDSP_REG+0x05E */
	volatile UINT8 cdspYuvSpeEfMode;      /* LOGI_ADDR_CDSP_REG+0x05F */

	volatile UINT8 cdspGlobalGain;        /* LOGI_ADDR_CDSP_REG+0x060 */
	volatile UINT8 cdspWbCtrl;            /* LOGI_ADDR_CDSP_REG+0x061 */
	volatile UINT8 cdspGammaEn;           /* LOGI_ADDR_CDSP_REG+0x062 */
	volatile UINT8 cdspImgCropCtrl;       /* LOGI_ADDR_CDSP_REG+0x063 */
	volatile UINT8 cdspImgCropHOff[2];    /* LOGI_ADDR_CDSP_REG+0x064 */
	volatile UINT8 cdspImgCropHSize[2];   /* LOGI_ADDR_CDSP_REG+0x066 */
	volatile UINT8 cdspImgCropVOff[2];    /* LOGI_ADDR_CDSP_REG+0x068 */
	volatile UINT8 cdspImgCropVSize[2];   /* LOGI_ADDR_CDSP_REG+0x06A */
	volatile UINT8 cdspInpDnLowThr;	      /* LOGI_ADDR_CDSP_REG+0x06C */
	volatile UINT8 cdspInpDnHighThr;	    /* LOGI_ADDR_CDSP_REG+0x06D */
	volatile UINT8 cdspInpMirCtrl;        /* LOGI_ADDR_CDSP_REG+0x06E */
	volatile UINT8 cdspRgbSpeEfMode;      /* LOGI_ADDR_CDSP_REG+0x06F */

	volatile UINT8 cdspNdnHpfL00L01;      /* LOGI_ADDR_CDSP_REG+0x070 */
	volatile UINT8 cdspNdnHpfL02L10;      /* LOGI_ADDR_CDSP_REG+0x071 */
	volatile UINT8 cdspNdnHpfL11L12;      /* LOGI_ADDR_CDSP_REG+0x072 */
	volatile UINT8 cdspNdnHpfL20L21;      /* LOGI_ADDR_CDSP_REG+0x073 */
	volatile UINT8 cdspNdnHpfL22;         /* LOGI_ADDR_CDSP_REG+0x074 */
	volatile UINT8 cdspNdnLHDiv;          /* LOGI_ADDR_CDSP_REG+0x075 */
	volatile UINT8 cdspNdnLHCor;          /* LOGI_ADDR_CDSP_REG+0x076 */
	volatile UINT8 cdspNdnLHMode;         /* LOGI_ADDR_CDSP_REG+0x077 */
	volatile UINT8 cdspNdnAmpga;          /* LOGI_ADDR_CDSP_REG+0x078 */
	volatile UINT8 cdspNdnEdgeCtrl;       /* LOGI_ADDR_CDSP_REG+0x079 */
	volatile UINT8 reserved07A[6];        /* LOGI_ADDR_CDSP_REG+0x07A */

	volatile UINT8 cdspInpHpfL00L01;      /* LOGI_ADDR_CDSP_REG+0x080 */
	volatile UINT8 cdspInpHpfL02L10;      /* LOGI_ADDR_CDSP_REG+0x081 */
	volatile UINT8 cdspInpHpfL11L12;      /* LOGI_ADDR_CDSP_REG+0x082 */
	volatile UINT8 cdspInpHpfL20L21;      /* LOGI_ADDR_CDSP_REG+0x083 */
	volatile UINT8 cdspInpHpfL22;         /* LOGI_ADDR_CDSP_REG+0x084 */
	volatile UINT8 cdspInpLHDiv;          /* LOGI_ADDR_CDSP_REG+0x085 */
	volatile UINT8 cdspInpLHCor;          /* LOGI_ADDR_CDSP_REG+0x086 */
	volatile UINT8 cdspInpLHMode;         /* LOGI_ADDR_CDSP_REG+0x087 */
	volatile UINT8 cdspInpAmpga;          /* LOGI_ADDR_CDSP_REG+0x088 */
	volatile UINT8 cdspInpEdgeCtrl;       /* LOGI_ADDR_CDSP_REG+0x089 */
	volatile UINT8 cdspInpQThr;           /* LOGI_ADDR_CDSP_REG+0x08A */
	volatile UINT8 reserved08B;           /* LOGI_ADDR_CDSP_REG+0x08B */
	volatile UINT8 cdspInpPreRbClamp;     /* LOGI_ADDR_CDSP_REG+0x08C */
	volatile UINT8 cdspInpQCnt[3];        /* LOGI_ADDR_CDSP_REG+0x08D */

	volatile UINT8 cdspCcCof00[2];        /* LOGI_ADDR_CDSP_REG+0x090 */
	volatile UINT8 cdspCcCof01[2];        /* LOGI_ADDR_CDSP_REG+0x092 */
	volatile UINT8 cdspCcCof02[2];        /* LOGI_ADDR_CDSP_REG+0x094 */
	volatile UINT8 cdspCcCof10[2];        /* LOGI_ADDR_CDSP_REG+0x096 */
	volatile UINT8 cdspCcCof11[2];        /* LOGI_ADDR_CDSP_REG+0x098 */
	volatile UINT8 cdspCcCof12[2];        /* LOGI_ADDR_CDSP_REG+0x09A */
	volatile UINT8 cdspCcCof20[2];        /* LOGI_ADDR_CDSP_REG+0x09C */
	volatile UINT8 cdspCcCof21[2];        /* LOGI_ADDR_CDSP_REG+0x09E */

	volatile UINT8 cdspCcCof22[2];        /* LOGI_ADDR_CDSP_REG+0x0A0 */
	volatile UINT8 cdspCcEn;              /* LOGI_ADDR_CDSP_REG+0x0A2 */
	volatile UINT8 cdspDisGatingClk1;     /* LOGI_ADDR_CDSP_REG+0x0A3 */
	volatile UINT8 cdspRbClampVal;        /* LOGI_ADDR_CDSP_REG+0x0A4 */
	volatile UINT8 cdspRbClampEn;         /* LOGI_ADDR_CDSP_REG+0x0A5 */
	volatile UINT8 reserved0A6[2];        /* LOGI_ADDR_CDSP_REG+0x0A6 */
	volatile UINT8 cdspUvSclDiv68;        /* LOGI_ADDR_CDSP_REG+0x0A8 */
	volatile UINT8 cdspUvSclDiv58;        /* LOGI_ADDR_CDSP_REG+0x0A9 */
	volatile UINT8 cdspUvSclDiv48;        /* LOGI_ADDR_CDSP_REG+0x0AA */
	volatile UINT8 cdspUvSclDiv38;        /* LOGI_ADDR_CDSP_REG+0x0AB */
	volatile UINT8 cdspUvSclDiv28;        /* LOGI_ADDR_CDSP_REG+0x0AC */
	volatile UINT8 cdspUvSclDiv18;        /* LOGI_ADDR_CDSP_REG+0x0AD */
	volatile UINT8 cdspUvSclYuvInsertEn;  /* LOGI_ADDR_CDSP_REG+0x0AE */
	volatile UINT8 reserved0AF;           /* LOGI_ADDR_CDSP_REG+0x0AF */

	volatile UINT8 cdspSuppEn;            /* LOGI_ADDR_CDSP_REG+0x0B0 */
	volatile UINT8 cdspBistEn;            /* LOGI_ADDR_CDSP_REG+0x0B1 */
	volatile UINT8 cdspSramBistFail;      /* LOGI_ADDR_CDSP_REG+0x0B2 */
	volatile UINT8 reserved0B3;           /* LOGI_ADDR_CDSP_REG+0x0B3 */
	volatile UINT8 cdspDnLowThr;          /* LOGI_ADDR_CDSP_REG+0x0B4 */
	volatile UINT8 cdspDnThrWth;          /* LOGI_ADDR_CDSP_REG+0x0B5 */
	volatile UINT8 cdspDnYhtDiv;          /* LOGI_ADDR_CDSP_REG+0x0B6 */
	volatile UINT8 reserved0B7[3];        /* LOGI_ADDR_CDSP_REG+0x0B7 */
	volatile UINT8 cdspHueSinData1;       /* LOGI_ADDR_CDSP_REG+0x0BA */
	volatile UINT8 cdspHueCosData1;       /* LOGI_ADDR_CDSP_REG+0x0BB */
	volatile UINT8 cdspHueSinData2;       /* LOGI_ADDR_CDSP_REG+0x0BC */
	volatile UINT8 cdspHueCosData2;       /* LOGI_ADDR_CDSP_REG+0x0BD */
	volatile UINT8 cdspYbYcEn;            /* LOGI_ADDR_CDSP_REG+0x0BE */
	volatile UINT8 cdspYuvRange;          /* LOGI_ADDR_CDSP_REG+0x0BF */

	volatile UINT8 reserved0C0[0x010];    /* LOGI_ADDR_CDSP_REG+0x0C0~0x0CF */

	volatile UINT8 cdspSuppMirCtrl;       /* LOGI_ADDR_CDSP_REG+0x0D0 */
	volatile UINT8 cdspNdnMirCtrl;        /* LOGI_ADDR_CDSP_REG+0x0D1 */
	volatile UINT8 reserved0D2[3];        /* LOGI_ADDR_CDSP_REG+0x0D2 */
	volatile UINT8 cdspRgbEn;             /* LOGI_ADDR_CDSP_REG+0x0D5 */
	volatile UINT8 reserved0D6[5];        /* LOGI_ADDR_CDSP_REG+0x0D6 */
	volatile UINT8 cdspIntEn;             /* LOGI_ADDR_CDSP_REG+0x0DB */
	volatile UINT8 cdspDisGateDmClk;      /* LOGI_ADDR_CDSP_REG+0x0DC */
	volatile UINT8 cdspDisGatingClk2;     /* LOGI_ADDR_CDSP_REG+0x0DD */
	volatile UINT8 cdspDisGatingClk3;     /* LOGI_ADDR_CDSP_REG+0x0DE */
	volatile UINT8 cdspDisGatingClk4;     /* LOGI_ADDR_CDSP_REG+0x0DF */

	volatile UINT8 cdspYCoring;           /* LOGI_ADDR_CDSP_REG+0x0E0 */
	volatile UINT8 cdspUCoring;           /* LOGI_ADDR_CDSP_REG+0x0E1 */
	volatile UINT8 cdspVCoring;           /* LOGI_ADDR_CDSP_REG+0x0E2 */
	volatile UINT8 cdspBistFinish;        /* LOGI_ADDR_CDSP_REG+0x0E3 */
	volatile UINT8 cdspInt;               /* LOGI_ADDR_CDSP_REG+0x0E4 */
	volatile UINT8 cdspBistFail;          /* LOGI_ADDR_CDSP_REG+0x0E5 */
	volatile UINT8 cdspProbeMode;         /* LOGI_ADDR_CDSP_REG+0x0E6 */
	volatile UINT8 cdspProbeSel;          /* LOGI_ADDR_CDSP_REG+0x0E7 */
	volatile UINT8 cdspProbeTestMode;     /* LOGI_ADDR_CDSP_REG+0x0E8 */
	volatile UINT8 cdspExtBlankSize[2];   /* LOGI_ADDR_CDSP_REG+0x0E9 */
	volatile UINT8 cdspYuvHAvgMirEn;      /* LOGI_ADDR_CDSP_REG+0x0EB */	
	volatile UINT8 cdspYuvHAvgLpfType;    /* LOGI_ADDR_CDSP_REG+0x0EC */
	volatile UINT8 cdspExtLineSize[2];    /* LOGI_ADDR_CDSP_REG+0x0ED */
	volatile UINT8 cdspReset;             /* LOGI_ADDR_CDSP_REG+0x0EF */
}cdspReg0_t;

typedef struct cdspReg1_s               /* 0x92014100 ~ 0x920141FF */
{
	volatile UINT8 cdspRawHSubCtrl;       /* LOGI_ADDR_CDSP_REG+0x100 */
	volatile UINT8 cdspRawVSubCtrl;       /* LOGI_ADDR_CDSP_REG+0x101 */
	volatile UINT8 reserved102[2];        /* LOGI_ADDR_CDSP_REG+0x102 */
	volatile UINT8 cdspRawHMirCtrl;       /* LOGI_ADDR_CDSP_REG+0x104 */
	volatile UINT8 reserved105[3];        /* LOGI_ADDR_CDSP_REG+0x105 */
	volatile UINT8 cdspRawVMirCtrl;       /* LOGI_ADDR_CDSP_REG+0x108 */
	volatile UINT8 reserved109[3];        /* LOGI_ADDR_CDSP_REG+0x109 */
	volatile UINT8 cdspClampHSize[2];     /* LOGI_ADDR_CDSP_REG+0x10C */
	volatile UINT8 cdspClampHSizeEn;      /* LOGI_ADDR_CDSP_REG+0x10E */
	volatile UINT8 reserved10F;           /* LOGI_ADDR_CDSP_REG+0x10F */
	
	volatile UINT8 cdspRbHSize[2];        /* LOGI_ADDR_CDSP_REG+0x110 */
	volatile UINT8 cdspRbHOffset[2];      /* LOGI_ADDR_CDSP_REG+0x112 */
	volatile UINT8 cdspRbVSize[2];        /* LOGI_ADDR_CDSP_REG+0x114 */
	volatile UINT8 cdspRbVOffset[2];      /* LOGI_ADDR_CDSP_REG+0x116 */
	volatile UINT8 cdspWdramHOffset[2];   /* LOGI_ADDR_CDSP_REG+0x118 */
	volatile UINT8 cdspWdramVOffset[2];   /* LOGI_ADDR_CDSP_REG+0x11A */
	volatile UINT8 reserved11C[4];        /* LOGI_ADDR_CDSP_REG+0x11C */
	
	volatile UINT8 cdspImgSrc;            /* LOGI_ADDR_CDSP_REG+0x120 */
	volatile UINT8 reserved121[3];        /* LOGI_ADDR_CDSP_REG+0x121 */
	volatile UINT8 cdspLineInterval[2];   /* LOGI_ADDR_CDSP_REG+0x124 */
	volatile UINT8 reserved126[2];        /* LOGI_ADDR_CDSP_REG+0x126 */
	volatile UINT8 cdspDo;                /* LOGI_ADDR_CDSP_REG+0x128 */
	volatile UINT8 reserved129[3];        /* LOGI_ADDR_CDSP_REG+0x129 */
	volatile UINT8 cdspSkipPix;           /* LOGI_ADDR_CDSP_REG+0x12C */
	volatile UINT8 reserved12D[3];        /* LOGI_ADDR_CDSP_REG+0x12D */
	
	volatile UINT8 cdspTvMode;            /* LOGI_ADDR_CDSP_REG+0x130 */
	volatile UINT8 cdspSramThr[2];        /* LOGI_ADDR_CDSP_REG+0x131 */
	volatile UINT8 reserved133[0x00D];    /* LOGI_ADDR_CDSP_REG+0x133~0x1EF */
	
	volatile UINT8 reserved140[0x0A0];    /* LOGI_ADDR_CDSP_REG+0x140~0x1DF */

	volatile UINT8 cdspRawCapYuvMode;     /* LOGI_ADDR_CDSP_REG+0x1E0 */
	volatile UINT8 cdspGatVld;            /* LOGI_ADDR_CDSP_REG+0x1E1 */
	volatile UINT8 cdspRawSubEn;          /* LOGI_ADDR_CDSP_REG+0x1E2 */
	volatile UINT8 reserved1E3[0x00D];    /* LOGI_ADDR_CDSP_REG+0x1E3~0x1EF */

	volatile UINT8 cdspTsrxCtrl;          /* LOGI_ADDR_CDSP_REG+0x1F0 */
	volatile UINT8 reserved1F1[0x00E];    /* LOGI_ADDR_CDSP_REG+0x1F1 */
	volatile UINT8 cdspGInt;              /* LOGI_ADDR_CDSP_REG+0x1FF */
}cdspReg1_t;

typedef struct cdspReg2_s 
{
	volatile UINT8 cdspAwbWinRGain2[2];   /* LOGI_ADDR_CDSP_REG+0x200 */
	volatile UINT8 cdspAwbWinGGain2[2];   /* LOGI_ADDR_CDSP_REG+0x202 */
	volatile UINT8 cdspAeHisAwbWinHold;   /* LOGI_ADDR_CDSP_REG+0x204 */
	volatile UINT8 cdspAeAwbWinEn;        /* LOGI_ADDR_CDSP_REG+0x205 */
	volatile UINT8 cdspAeActiveWin;       /* LOGI_ADDR_CDSP_REG+0x206 */
	volatile UINT8 cdspRawAwbWinCtrl;     /* LOGI_ADDR_CDSP_REG+0x207 */
	volatile UINT8 cdspAeAccFator;        /* LOGI_ADDR_CDSP_REG+0x208 */
	volatile UINT8 cdspCtrlSvdEn;         /* LOGI_ADDR_CDSP_REG+0x209 */
	volatile UINT8 cdspAwbWinBGain2[2];   /* LOGI_ADDR_CDSP_REG+0x20A */
	volatile UINT8 reserved20C[3];        /* LOGI_ADDR_CDSP_REG+0x20C */
	volatile UINT8 cdspAwbWinGain2En;     /* LOGI_ADDR_CDSP_REG+0x20F */

	volatile UINT8 cdspRgbWinHSize[2];    /* LOGI_ADDR_CDSP_REG+0x210 */
	volatile UINT8 cdspRgbWinHOff[2];     /* LOGI_ADDR_CDSP_REG+0x212 */
	volatile UINT8 cdspRgbWinVSize[2];    /* LOGI_ADDR_CDSP_REG+0x214 */
	volatile UINT8 cdspRgbWinVOff[2];     /* LOGI_ADDR_CDSP_REG+0x216 */
	volatile UINT8 cdspAeWinBufAddrA[4];  /* LOGI_ADDR_CDSP_REG+0x218 */
	volatile UINT8 cdspAeWinBufAddrB[4];  /* LOGI_ADDR_CDSP_REG+0x21C */

	volatile UINT8 cdspHisLowThr;		      /* LOGI_ADDR_CDSP_REG+0x220 */
	volatile UINT8 cdspHisHiThr;          /* LOGI_ADDR_CDSP_REG+0x221 */
	volatile UINT8 cdspHisEn;             /* LOGI_ADDR_CDSP_REG+0x222 */
	volatile UINT8 reserved223;           /* LOGI_ADDR_CDSP_REG+0x223 */
	volatile UINT8 cdspHisLowCnt[3];		  /* LOGI_ADDR_CDSP_REG+0x224 */
	volatile UINT8 reserved227;		        /* LOGI_ADDR_CDSP_REG+0x227 */
	volatile UINT8 cdspHisHiCnt[3];       /* LOGI_ADDR_CDSP_REG+0x228 */
	volatile UINT8 reserved22B[5];        /* LOGI_ADDR_CDSP_REG+0x22B~0x22F */

	volatile UINT8 cdspAfWin1HOff[2];     /* LOGI_ADDR_CDSP_REG+0x230 */
	volatile UINT8 cdspAfWin1VOff[2];     /* LOGI_ADDR_CDSP_REG+0x232 */
	volatile UINT8 cdspAfWin1HSize[2];    /* LOGI_ADDR_CDSP_REG+0x234 */
	volatile UINT8 cdspAfWin1VSize[2];    /* LOGI_ADDR_CDSP_REG+0x236 */
	volatile UINT8 cdspAfWin23SizeL;      /* LOGI_ADDR_CDSP_REG+0x238 */
	volatile UINT8 cdspAfWinHoldEn;       /* LOGI_ADDR_CDSP_REG+0x239 */
	volatile UINT8 cdspAfWin2HOff[2];     /* LOGI_ADDR_CDSP_REG+0x23A */
	volatile UINT8 cdspAfWin2VOff[2];     /* LOGI_ADDR_CDSP_REG+0x23C */
	volatile UINT8 cdspAfWin3HOff[2];     /* LOGI_ADDR_CDSP_REG+0x23E */

	volatile UINT8 cdspAfWin3VOff[2];     /* LOGI_ADDR_CDSP_REG+0x240 */
	volatile UINT8 cdspAfWin1HVal[5];     /* LOGI_ADDR_CDSP_REG+0x242 */
	volatile UINT8 cdspAfWin1VVal[5];     /* LOGI_ADDR_CDSP_REG+0x247 */
	volatile UINT8 cdspAfWin2HValL[4];    /* LOGI_ADDR_CDSP_REG+0x24C */

	volatile SINT8 cdspAwbSinData;        /* LOGI_ADDR_CDSP_REG+0x250 */
	volatile SINT8 cdspAwbCosData;        /* LOGI_ADDR_CDSP_REG+0x251 */
	volatile UINT8 cdspAwbWinThr;         /* LOGI_ADDR_CDSP_REG+0x252 */
	volatile UINT8 cdspAwbClampEn;        /* LOGI_ADDR_CDSP_REG+0x253 */
	volatile UINT8 cdspAwbSpWinYThr0;     /* LOGI_ADDR_CDSP_REG+0x254 */
	volatile UINT8 cdspAwbSpWinYThr1;     /* LOGI_ADDR_CDSP_REG+0x255 */
	volatile UINT8 cdspAwbSpWinYThr2;     /* LOGI_ADDR_CDSP_REG+0x256 */
	volatile UINT8 cdspAwbSpWinYThr3;     /* LOGI_ADDR_CDSP_REG+0x257 */
	volatile UINT8 reserved258[8];        /* LOGI_ADDR_CDSP_REG+0x258~0x25F */

	volatile SINT8 cdspAwbSpWinULowThr1;  /* LOGI_ADDR_CDSP_REG+0x260 */
	volatile SINT8 cdspAwbSpWinUHiThr1;   /* LOGI_ADDR_CDSP_REG+0x261 */
	volatile SINT8 cdspAwbSpWinVLowThr1;  /* LOGI_ADDR_CDSP_REG+0x262 */
	volatile SINT8 cdspAwbSpWinVHiThr1;   /* LOGI_ADDR_CDSP_REG+0x263 */
	volatile SINT8 cdspAwbSpWinULowThr2;  /* LOGI_ADDR_CDSP_REG+0x264 */
	volatile SINT8 cdspAwbSpWinUHiThr2;   /* LOGI_ADDR_CDSP_REG+0x265 */
	volatile SINT8 cdspAwbSpWinVLowThr2;  /* LOGI_ADDR_CDSP_REG+0x266 */
	volatile SINT8 cdspAwbSpWinVHiThr2;   /* LOGI_ADDR_CDSP_REG+0x267 */
	volatile SINT8 cdspAwbSpWinULowThr3;  /* LOGI_ADDR_CDSP_REG+0x268 */
	volatile SINT8 cdspAwbSpWinUHiThr3;   /* LOGI_ADDR_CDSP_REG+0x269 */
	volatile SINT8 cdspAwbSpWinVLowThr3;  /* LOGI_ADDR_CDSP_REG+0x26A */
	volatile SINT8 cdspAwbSpWinVHiThr3;   /* LOGI_ADDR_CDSP_REG+0x26B */
	volatile UINT8 reserved26C[0x014];    /* LOGI_ADDR_CDSP_REG+0x26C~0x27F */

	volatile UINT8 cdspAwbSumCnt1[4];     /* LOGI_ADDR_CDSP_REG+0x280 */
	volatile UINT8 cdspAwbSumG1L[4];      /* LOGI_ADDR_CDSP_REG+0x284 */
	volatile UINT8 cdspAwbSumRg1L[4];     /* LOGI_ADDR_CDSP_REG+0x288 */
	volatile UINT8 cdspAwbSumBg1L[4];     /* LOGI_ADDR_CDSP_REG+0x28C */

	volatile UINT8 cdspAwbSumCnt2[4];     /* LOGI_ADDR_CDSP_REG+0x290 */
	volatile UINT8 cdspAwbSumG2L[4];      /* LOGI_ADDR_CDSP_REG+0x294 */
	volatile UINT8 cdspAwbSumRg2L[4];     /* LOGI_ADDR_CDSP_REG+0x298 */
	volatile UINT8 cdspAwbSumBg2L[4];     /* LOGI_ADDR_CDSP_REG+0x29C */

	volatile UINT8 cdspAwbSumCnt3[4];     /* LOGI_ADDR_CDSP_REG+0x2A0 */
	volatile UINT8 cdspAwbSumG3L[4];      /* LOGI_ADDR_CDSP_REG+0x2A4 */
	volatile UINT8 cdspAwbSumRg3L[4];     /* LOGI_ADDR_CDSP_REG+0x2A8 */
	volatile UINT8 cdspAwbSumBg3L[4];     /* LOGI_ADDR_CDSP_REG+0x2AC */

	volatile UINT8 reserved2B0[0x019];    /* LOGI_ADDR_CDSP_REG+0x2B0~0x2C9 */

	volatile UINT8 cdspAfWin2VValL[4];    /* LOGI_ADDR_CDSP_REG+0x2CA */
	volatile UINT8 cdspAfWin3HValL[4];    /* LOGI_ADDR_CDSP_REG+0x2CE */
	volatile UINT8 cdspAfWin3VValL[4];    /* LOGI_ADDR_CDSP_REG+0x2D2 */	
	volatile UINT8 cdspAfWin2HValH;       /* LOGI_ADDR_CDSP_REG+0x2D6 */
	volatile UINT8 cdspAfWin2VValH;       /* LOGI_ADDR_CDSP_REG+0x2D7 */
	volatile UINT8 cdspAfWin3HValH;       /* LOGI_ADDR_CDSP_REG+0x2D8 */
	volatile UINT8 cdspAfWin3VValH;       /* LOGI_ADDR_CDSP_REG+0x2D9 */
	volatile UINT8 reserved2DA[6];        /* LOGI_ADDR_CDSP_REG+0x2DA~0x2DF */
	
	volatile UINT8 cdspAeAfWinTest;       /* LOGI_ADDR_CDSP_REG+0x2E0 */
	volatile UINT8 cdspAfWin23SizeH;      /* LOGI_ADDR_CDSP_REG+0x2E1 */
	volatile UINT8 cdspAwbSumG1H;         /* LOGI_ADDR_CDSP_REG+0x2E2 */
	volatile SINT8 cdspAwbSumRg1H;        /* LOGI_ADDR_CDSP_REG+0x2E3 */
	volatile SINT8 cdspAwbSumBg1H;        /* LOGI_ADDR_CDSP_REG+0x2E4 */
	volatile UINT8 cdspAwbSumG2H;         /* LOGI_ADDR_CDSP_REG+0x2E5 */
	volatile SINT8 cdspAwbSumRg2H;        /* LOGI_ADDR_CDSP_REG+0x2E6 */
	volatile SINT8 cdspAwbSumBg2H;        /* LOGI_ADDR_CDSP_REG+0x2E7 */
	volatile UINT8 cdspAwbSumG3H;         /* LOGI_ADDR_CDSP_REG+0x2E8 */
	volatile SINT8 cdspAwbSumRg3H;        /* LOGI_ADDR_CDSP_REG+0x2E9 */
	volatile SINT8 cdspAwbSumBg3H;        /* LOGI_ADDR_CDSP_REG+0x2EA */
}cdspReg2_t;	

typedef struct cdspReg7_s               /* 0x92014700 ~ 0x920147FF */
{
	volatile UINT8 cdspYuvAfbSAddr[4];    /* LOGI_ADDR_CDSP_REG+0x700 */
	volatile UINT8 cdspYuvAfbHSize[2];    /* LOGI_ADDR_CDSP_REG+0x704 */
	volatile UINT8 cdspYuvAfbVSize[2];    /* LOGI_ADDR_CDSP_REG+0x706 */
	volatile UINT8 cdspYuvBfbSAddr[4];    /* LOGI_ADDR_CDSP_REG+0x708 */
	volatile UINT8 cdspYuvBfbHSize[2];    /* LOGI_ADDR_CDSP_REG+0x70C */
	volatile UINT8 cdspYuvBfbVSize[2];    /* LOGI_ADDR_CDSP_REG+0x70E */

	volatile UINT8 cdspRawfbSAddr[4];     /* LOGI_ADDR_CDSP_REG+0x710 */
	volatile UINT8 cdspRawfbHSize[2];     /* LOGI_ADDR_CDSP_REG+0x714 */
	volatile UINT8 cdspRawfbVSize[2];     /* LOGI_ADDR_CDSP_REG+0x716 */
	volatile UINT8 cdspRawfbHOff[2];      /* LOGI_ADDR_CDSP_REG+0x718 */
	volatile UINT8 cdspDarkHOff[2];       /* LOGI_ADDR_CDSP_REG+0x71A */
	volatile UINT8 cdspDarkSAddr[4];      /* LOGI_ADDR_CDSP_REG+0x71C */

	volatile UINT8 cdspYuvABfbWrIdx;      /* LOGI_ADDR_CDSP_REG+0x720 */
}cdspReg7_t;

typedef struct cdspLensCmpLut_s 
{
	volatile UINT8 LensCmpTable[512]; /* 256x16 */
}cdspLensCmpLut_t;

typedef struct cdspGammaLut_s 
{
	volatile UINT8 GammaTable[512]; /* 128x22 */
}cdspGammaLut_t;

typedef struct cdspEdgeLut_s 
{
	volatile UINT8 EdgeTable[256]; /* 256x8 */
}cdspEdgeLut_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
 
#endif /* _REG_CDSP_H_ */



