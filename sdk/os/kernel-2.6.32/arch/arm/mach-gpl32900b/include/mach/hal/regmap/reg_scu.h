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
 * @file    reg_scu.h
 * @brief   Regmap of GPL32900 SCU
 * @author  qinjian
 * @since   2010-9-29
 * @date    2010-9-29
 */
#ifndef _REG_SCU_H_
#define _REG_SCU_H_

#include <mach/hardware.h>
#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define LOGI_ADDR_SCU_A_REG     IO3_ADDRESS(0x7000)
#define LOGI_ADDR_SCU_B_REG     IO0_ADDRESS(0x5000)
#define LOGI_ADDR_SCU_C_REG     IO2_ADDRESS(0x5000)

#define LOGI_ADDR_SCU_B_CLK_REG	(LOGI_ADDR_SCU_B_REG+0x400)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/* SCU_A Control Unit */
typedef struct scuaReg_s {
	volatile UINT32 scuaPeriRst;        /* 0x0000 ~ 0x0003 SCUA Peripheral Reset */
	volatile UINT32 scuaPeriClkEn;      /* 0x0004 ~ 0x0007 SCUA Peripheral Clock Enable */
	volatile UINT32 dummy0[1];          /* 0x0008 ~ 0x000B */
	volatile UINT32 scuaPeriDgClkEn;    /* 0x000C ~ 0x000F SCUA Dynamic Clock Gating Enable */

	volatile UINT32 scuaDispType;       /* 0x0010 ~ 0x0013 TFT0 Display Type Select */
	volatile UINT32 dummy1[1];          /* 0x0014 ~ 0x0017 */
	volatile UINT32 scuaPeriClkEn2;     /* 0x0018 ~ 0x001B SCUA Peripheral Clock Enable 2 */
	volatile UINT32 dummy2[7];          /* 0x001C ~ 0x0037 */
	volatile UINT32 scuaUsbPhyCfg0;     /* 0x0038 ~ 0x003B USBPHY Configure 0 */
	volatile UINT32 scuaUsbPhyCfg1;     /* 0x003C ~ 0x003F USBPHY Configure 1 */

	volatile UINT32 scuaVdacCfg;        /* 0x0040 ~ 0x0043 Video DAC Configure */
	volatile UINT32 scuaApllCfg;        /* 0x0044 ~ 0x0047 Audio PLL Configure */
	volatile UINT32 scuaExtCodec;       /* 0x0048 ~ 0x004B Ext codec set */
	volatile UINT32 scuaSpuReq;         /* 0x004C ~ 0x004F SPU request source */

	volatile UINT32 scuaCpuDbgCtrlA;    /* 0x0050 ~ 0x0053 For Chip Debug Only */
	volatile UINT32 scuaCpuDbgStatA;    /* 0x0054 ~ 0x0057 For Chip Debug Only */
	volatile UINT32 dummy4[10];         /* 0x0058 ~ 0x007F */

	volatile UINT32 scuaLcdClkCfg;      /* 0x0080 ~ 0x0083 LCD(TFT0) Clock Configure */
	volatile UINT32 scuaCsiClkCfg;      /* 0x0084 ~ 0x0087 CSI Clock Configure */
	volatile UINT32 dummy5[2];          /* 0x0088 ~ 0x008F */

	volatile UINT32 scuaI2sBckCfg;      /* 0x0090 ~ 0x0093 I2S BCK Configure */
	volatile UINT32 scuaUartCfg;        /* 0x0094 ~ 0x0097 UART Clock Configure */
	volatile UINT32 dummy6[6];          /* 0x0098 ~ 0x00AF */

	volatile UINT32 scuaCodecCfg;       /* 0x00B0 ~ 0x00B3 I2C Codec Configure */
	volatile UINT32 scuaB210Pri;        /* 0x00B4 ~ 0x00B7 B210 bridge high priority */
	volatile UINT32 dummy7[10];         /* 0x00B8 ~ 0x00DF */

	volatile UINT32 scuaSysSel;         /* 0x00E0 ~ 0x00E3 System Select */
	volatile UINT32 dummy8[1];          /* 0x00E4 ~ 0x00E7 */
	volatile UINT32 scuaCsi2ClkCfg;     /* 0x00E8 ~ 0x00EB CSI Clock Configure 2 */
	volatile UINT32 scuaCdspPclk;       /* 0x00EC ~ 0x00EF CDSP Clock Configure */
} scuaReg_t;

/* SCU_B Control Unit */
typedef struct scubReg_s {
	volatile UINT32 scubPeriRst;        /* 0x0000 ~ 0x0003 SCUB Peripheral Reset */
	volatile UINT32 scubSpllCfg;        /* 0x0004 ~ 0x0007 SPLL Configure */
	volatile UINT32 scubIntrStatus;     /* 0x0008 ~ 0x000B Timer Interrupt Status */
	volatile UINT32 scubTimerIceEn;     /* 0x000C ~ 0x000F Timer Stop Enable When ICE Connect */

	volatile UINT32 scubTimerExtCtrl;   /* 0x0010 ~ 0x0013 Timer External Trigger Up/Down */
	volatile UINT32 scubOtp0;           /* 0x0014 ~ 0x0017 OTP0 */
	volatile UINT32 scubRev;            /* 0x0018 ~ 0x001B IC Version */
	volatile UINT32 scubRand0;          /* 0x001C ~ 0x001F Randomize Number Seed 0 */

	volatile UINT32 scubPeriClkEn;      /* 0x0020 ~ 0x0023 SCUB Peripheral Clock Enable */
	volatile UINT32 scubPeriDgClkEn;    /* 0x0024 ~ 0x0027 SCUB Dynamic Clock Gating Enable */
	volatile UINT32 scubUpdateRatio;    /* 0x0028 ~ 0x002B ARM Clock Ratio Update Control */
	volatile UINT32 dummy1[5];          /* 0x002C ~ 0x003F */

	volatile UINT32 scubPwrcCfg;        /* 0x0040 ~ 0x0043 PWRC Configure */
	volatile UINT32 scubOtp1;           /* 0x0044 ~ 0x0047 OTP1 */
	volatile UINT32 scubRand1;          /* 0x0048 ~ 0x004B Randomize Number Seed 1 */
	volatile UINT32 scubOtp2;           /* 0x004C ~ 0x004F OTP2 */

	volatile UINT32 scubIoIcCtrl;       /* 0x0050 ~ 0x0053 IC Control */
	volatile UINT32 dummy2[1];          /* 0x0054 ~ 0x0057 */
	volatile UINT32 scubDbgRqCtrl;      /* 0x0058 ~ 0x005B ARM Dynamic Gated Clock Control */
	volatile UINT32 scubXckCtrl0;       /* 0x005C ~ 0x005F XTAL generator control */
	volatile UINT32 scubXckCtrl1;       /* 0x0060 ~ 0x0063 XTAL generator control  */

	volatile UINT32 scubSpll0Cfg;       /* 0x0064 ~ 0x0067 SPLL 0 config */
	volatile UINT32 scubSpll1Cfg;       /* 0x0068 ~ 0x006B SPLL 1 config */
	volatile UINT32 scubSpll2Cfg;       /* 0x006C ~ 0x006F SPLL 2 config*/
	
	volatile UINT32 scubSpllSel;        /* 0x0070 ~ 0x0073 Clcok PLL selection */
	volatile UINT32 scubSpll2SscCfg0;   /* 0x0074 ~ 0x0078 SPLL2 spread trum config 0 */
	volatile UINT32 scubSpll2SscCfg1;   /* 0x0078 ~ 0x007B SPLL2 spread trum config 1 */
	volatile UINT32 scubSpll2SscCfg2;   /* 0x007C ~ 0x007F SPLL2 spread trum config 2 */

	volatile UINT32 scubPadGrpSel0;     /* 0x0080 ~ 0x0083 PAD Group Selection 0 */
	volatile UINT32 scubPadGrpSel1;     /* 0x0084 ~ 0x0087 PAD Group Selection 1 */
	volatile UINT32 scubPadGrpSel2;     /* 0x0088 ~ 0x008B PAD Group Selection 2 */
	volatile UINT32 scubPadGrpSel3;     /* 0x008C ~ 0x008F PAD Group Selection 3 */

	volatile UINT32 scubPadGrpCtrl0;    /* 0x0090 ~ 0x0093 PAD Group Control 0 */
	volatile UINT32 scubPadGrpCtrl1;    /* 0x0094 ~ 0x0097 PAD Group Control 1 */
	volatile UINT32 scubPadGrpCtrl2;    /* 0x0098 ~ 0x009B PAD Group Control 2 */
	volatile UINT32 scubPadGrpCtrl3;    /* 0x009C ~ 0x009F PAD Group Control 3 */

	volatile UINT32 dummy4[4];          /* 0x00A0 ~ 0x00AF */

	volatile UINT32 scubCpuDbgCtrl;     /* 0x00B0 ~ 0x00B3 For Chip Debug Only */
	volatile UINT32 scubCpuDbgStat;     /* 0x00B4 ~ 0x00B7 For Chip Debug Only */
	volatile UINT32 dummy5[1];          /* 0x00B8 ~ 0x00BB */
	volatile UINT32 scubPadCtrl0;       /* 0x00BC ~ 0x00BF PAD Control 0 */

	volatile UINT32 dummy6[4];          /* 0x00C0 ~ 0x00CF */

	volatile UINT32 scubArmRatio;       /* 0x00D0 ~ 0x00D3 ARM Clock Ratio */
	volatile UINT32 scubArmAhbRatio;    /* 0x00D4 ~ 0x00D7 ARM AHB Clock Ratio */
	volatile UINT32 scubArmApbRatio;    /* 0x00D8 ~ 0x00DB ARM APB Clock Ratio */
	volatile UINT32 scubSysCntEn;       /* 0x00DC ~ 0x00DF System Counter Enable */

	volatile UINT32 dummy7[4];          /* 0x00E0 ~ 0x00EF */
	
	volatile UINT32 scubGpio4PinEn;     /* 0x00F0 ~ 0x00F0 GPIO4 Input Enable */
	volatile UINT32 scubGpio4PinDs;     /* 0x00F4 ~ 0x00F7 GPIO4 Driving Strength */
	volatile UINT32 scubGpio4PinPe;     /* 0x00F8 ~ 0x00FB GPIO4 Pull Enable */
	volatile UINT32 scubGpio4PinPs;     /* 0x00FC ~ 0x00FF GPIO4 Pull Select */

	volatile UINT32 scubGpio0PinEn;     /* 0x0100 ~ 0x0103 GPIO0 Input Enable */
	volatile UINT32 scubGpio0PinDs;     /* 0x0104 ~ 0x0107 GPIO0 Driving Strength */
	volatile UINT32 scubGpio0PinPe;     /* 0x0108 ~ 0x010B GPIO0 Pull Enable */
	volatile UINT32 scubGpio0PinPs;     /* 0x010C ~ 0x010F GPIO0 Pull Select */

	volatile UINT32 scubGpio1PinEn;     /* 0x0110 ~ 0x0113 GPIO1 Input Enable */
	volatile UINT32 scubGpio1PinDs;     /* 0x0114 ~ 0x0117 GPIO1 Driving Strength */
	volatile UINT32 scubGpio1PinPe;     /* 0x0118 ~ 0x011B GPIO1 Pull Enable */
	volatile UINT32 scubGpio1PinPs;     /* 0x011C ~ 0x011F GPIO1 Pull Select */

	volatile UINT32 scubGpio2PinEn;     /* 0x0120 ~ 0x0123 GPIO2 Input Enable */
	volatile UINT32 scubGpio2PinDs;     /* 0x0124 ~ 0x0127 GPIO2 Driving Strength */
	volatile UINT32 scubGpio2PinPe;     /* 0x0128 ~ 0x012B GPIO2 Pull Enable */
	volatile UINT32 scubGpio2PinPs;     /* 0x012C ~ 0x012F GPIO2 Pull Select */

	volatile UINT32 scubGpio3PinEn;     /* 0x0130 ~ 0x0133 GPIO3 Input Enable */
	volatile UINT32 scubGpio3PinDs;     /* 0x0134 ~ 0x0137 GPIO3 Driving Strength */
	volatile UINT32 scubGpio3PinPe;     /* 0x0138 ~ 0x013B GPIO3 Pull Enable */
	volatile UINT32 scubGpio3PinPs;     /* 0x013C ~ 0x013F GPIO3 Pull Select */

	volatile UINT32 scubPwmCtrl;        /* 0x0140 ~ 0x0143 DC-DC PWM Control */
	volatile UINT32 scubPinMux;         /* 0x0144 ~ 0x0147 IO Pad Mux Configure */
	volatile UINT32 dummy8[1];          /* 0x0148 ~ 0x014B */
	volatile UINT32 scubDbgPinMux;      /* 0x014C ~ 0x014F Debug Probe Signal Pin Mux Configure */

	volatile UINT32 scubCp15sDisable;   /* 0x0150 ~ 0x0153 ARM CP15 Access Disable */
	volatile UINT32 scubArmJtag;        /* 0x0154 ~ 0x0157 ARM JTAG Information */
	volatile UINT32 scubTzCfg;          /* 0x0158 ~ 0x015B TrusZone Secure and Non-Secure Region Configure */
	volatile UINT32 scubBusArbSel;      /* 0x015C ~ 0x015F BUS Arbiter Configure */
	
	volatile UINT32 scubErr;      		/* 0x0160 ~ 0x0163 Error flag */
	volatile UINT32 dummy9[7];          /* 0x0164 ~ 0x017F */
	
	volatile UINT32 scubPZoneERst;      /* 0x0180 ~ 0x0183 P zone E reset */
	volatile UINT32 scubPZoneEClkEn;    /* 0x0184 ~ 0x0187 P zone E clock enable */
	volatile UINT32 scubMipiPinSel;     /* 0x0188 ~ 0x018B Mipi interface select */
	volatile UINT32 scubDispCdacSel;    /* 0x018C ~ 0x018F TCON/LCM/LCD/CVBS out select */
	volatile UINT32 scubCodecRst;       /* 0x0190 ~ 0x0193 Video encoder/decoder reset */
	volatile UINT32 dummy10[11];        /* 0x0194 ~ 0x01BF */
	
	volatile UINT32 scubDDRCfg0;        /* 0x01C0 ~ 0x01C3 DDR phy config 0 */
	volatile UINT32 scubDDRCfg1;        /* 0x01C0 ~ 0x01C3 DDR phy config 1 */
	volatile UINT32 scubDDRCfg2;        /* 0x01C0 ~ 0x01C3 DDR phy config 2 */
} scubReg_t;

/* SCU_B Clock Control Unit */
typedef struct scubClkReg_s {
	volatile UINT32 scubClk0Cfg0;		/* 0x0400 ~ 0x0403 Clock 0 control 0 */
	volatile UINT32 scubClk0Cfg1;		/* 0x0404 ~ 0x0407 Clock 0 control 1 */
	volatile UINT32 scubClk0Cfg2;		/* 0x0408 ~ 0x040B Clock 0 control 2 */
	volatile UINT32 scubClk0Cfg3;		/* 0x040C ~ 0x040F Clock 0 control 3 */
	volatile UINT32 scubClk1Cfg0;		/* 0x0410 ~ 0x0413 Clock 1 control 0 */
	volatile UINT32 scubClk1Cfg1;		/* 0x0414 ~ 0x0417 Clock 1 control 1 */
	volatile UINT32 scubClk1Cfg2;		/* 0x0418 ~ 0x041B Clock 1 control 2 */
	volatile UINT32 scubClk1Cfg3;		/* 0x041C ~ 0x041F Clock 1 control 3 */
	volatile UINT32 scubClk2Cfg0;		/* 0x0420 ~ 0x0423 Clock 2 control 0 */
	volatile UINT32 scubClk2Cfg1;		/* 0x0424 ~ 0x0427 Clock 2 control 1 */
	volatile UINT32 scubClk2Cfg2;		/* 0x0428 ~ 0x042B Clock 2 control 2 */
	volatile UINT32 scubClk2Cfg3;		/* 0x042C ~ 0x042F Clock 2 control 3 */
	volatile UINT32 scubClk3Cfg0;		/* 0x0430 ~ 0x0433 Clock 3 control 0 */
	volatile UINT32 scubClk3Cfg1;		/* 0x0434 ~ 0x0437 Clock 3 control 1 */
	volatile UINT32 scubClk3Cfg2;		/* 0x0438 ~ 0x043B Clock 3 control 2 */
	volatile UINT32 scubClk3Cfg3;		/* 0x043C ~ 0x043F Clock 3 control 3 */
	volatile UINT32 scubClk4Cfg0;		/* 0x0440 ~ 0x0443 Clock 4 control 0 */
	volatile UINT32 scubClk4Cfg1;		/* 0x0444 ~ 0x0447 Clock 4 control 1 */
	volatile UINT32 scubClk4Cfg2;		/* 0x0448 ~ 0x044B Clock 4 control 2 */
	volatile UINT32 scubClk4Cfg3;		/* 0x044C ~ 0x044F Clock 4 control 3 */
	volatile UINT32 scubClk5Cfg0;		/* 0x0450 ~ 0x0453 Clock 5 control 0 */
	volatile UINT32 scubClk5Cfg1;		/* 0x0454 ~ 0x0457 Clock 5 control 1 */
	volatile UINT32 scubClk5Cfg2;		/* 0x0458 ~ 0x045B Clock 5 control 2 */
	volatile UINT32 scubClk5Cfg3;		/* 0x045C ~ 0x045F Clock 5 control 3 */
	volatile UINT32 scubClk6Cfg0;		/* 0x0460 ~ 0x0463 Clock 6 control 0 */
	volatile UINT32 scubClk6Cfg1;		/* 0x0464 ~ 0x0467 Clock 6 control 1 */
	volatile UINT32 scubClk6Cfg2;		/* 0x0468 ~ 0x046B Clock 6 control 2 */
	volatile UINT32 scubClk6Cfg3;		/* 0x046C ~ 0x046F Clock 6 control 3 */
	volatile UINT32 scubClk7Cfg0;		/* 0x0470 ~ 0x0473 Clock 7 control 0 */
	volatile UINT32 scubClk7Cfg1;		/* 0x0474 ~ 0x0477 Clock 7 control 1 */
	volatile UINT32 scubClk7Cfg2;		/* 0x0478 ~ 0x047B Clock 7 control 2 */
	volatile UINT32 scubClk7Cfg3;		/* 0x047C ~ 0x047F Clock 7 control 3 */
	volatile UINT32 scubClk8Cfg0;		/* 0x0480 ~ 0x0483 Clock 8 control 0 */
	volatile UINT32 scubClk8Cfg1;		/* 0x0484 ~ 0x0487 Clock 8 control 1 */
	volatile UINT32 scubClk8Cfg2;		/* 0x0488 ~ 0x048B Clock 8 control 2 */
	volatile UINT32 scubClk8Cfg3;		/* 0x048C ~ 0x048F Clock 8 control 3 */
	volatile UINT32 scubClk9Cfg0;		/* 0x0490 ~ 0x0493 Clock 9 control 0 */
	volatile UINT32 scubClk9Cfg1;		/* 0x0494 ~ 0x0497 Clock 9 control 1 */
	volatile UINT32 scubClk9Cfg2;		/* 0x0498 ~ 0x049B Clock 9 control 2 */
	volatile UINT32 scubClk9Cfg3;		/* 0x049C ~ 0x049F Clock 9 control 3 */
} scubClkReg_t;

/* SCU_C Control Unit */
typedef struct scucReg_s {
	volatile UINT32 scucPeriRst;        /* 0x0000 ~ 0x0003 SCUC Peripheral Reset */
	volatile UINT32 scucPeriClkEn;      /* 0x0004 ~ 0x0007 SCUC Peripheral Clock Enable */
	volatile UINT32 scucPeriDgClkEn;    /* 0x0008 ~ 0x000B SCUC Dynamic Gating Clock Enable */
	volatile UINT32 dummy1[1];          /* 0x000C ~ 0x000F */

	volatile UINT32 scucGcCfg0;         /* 0x0010 ~ 0x0013 DDRPHY Configure 0 */
	volatile UINT32 dummy2[5];          /* 0x0014 ~ 0x0027 */
	volatile UINT32 scucSysRatioUpdate; /* 0x0028 ~ 0x002B System Clock Ratio Update Control */
	volatile UINT32 dummy3[5];    	    /* 0x002C ~ 0x003F */

	volatile UINT32 scucDdrPhyCtrl2;    /* 0x0040 ~ 0x0043 DDRPHY Control 2 */
	volatile UINT32 dummy4[3];    	    /* 0x0044 ~ 0x004F */
	volatile UINT32 scucDramSts;    	/* 0x0050 ~ 0x0053 DRAM controller request port idle status */

	volatile UINT32 dummy5[21];         /* 0x0050 ~ 0x00A7 */
	volatile UINT32 scucCpuDbgCtrl;     /* 0x00A8 ~ 0x00AB For Chip Debug Only */
	volatile UINT32 scucCpuDbgStat;     /* 0x00AC ~ 0x00AF For Chip Debug Only */

	volatile UINT32 scucNrReg0;         /* 0x00B0 ~ 0x00B3 NO_RST_REG0, this value will keep during reset */
	volatile UINT32 scucNrReg1;         /* 0x00B4 ~ 0x00B7 NO_RST_REG1, this value will keep during reset */
	volatile UINT32 scucNrReg2;         /* 0x00B8 ~ 0x00BB NO_RST_REG2, this value will keep during reset */
	volatile UINT32 scucNrReg3;         /* 0x00BC ~ 0x00BF NO_RST_REG3, this value will keep during reset */

	volatile UINT32 scucTas0;           /* 0x00C0 ~ 0x00C3 TEST_AND_SET[0], the value will be 0 for winner only */
	volatile UINT32 scucTas1;           /* 0x00C4 ~ 0x00C7 TEST_AND_SET[1], the value will be 0 for winner only */
	volatile UINT32 scucTas2;           /* 0x00C8 ~ 0x00CB TEST_AND_SET[2], the value will be 0 for winner only */
	volatile UINT32 scucTas3;           /* 0x00CC ~ 0x00CF TEST_AND_SET[3], the value will be 0 for winner only */
	volatile UINT32 scucTas4;           /* 0x00D0 ~ 0x00D3 TEST_AND_SET[4], the value will be 0 for winner only */
	volatile UINT32 scucTas5;           /* 0x00D4 ~ 0x00D7 TEST_AND_SET[5], the value will be 0 for winner only */
	volatile UINT32 scucTas6;           /* 0x00D8 ~ 0x00DB TEST_AND_SET[6], the value will be 0 for winner only */
	volatile UINT32 scucTas7;           /* 0x00DC ~ 0x00DF TEST_AND_SET[7], the value will be 0 for winner only */

	volatile UINT32 dummy6[8];          /* 0x00E0 ~ 0x00FF */

	volatile UINT32 scucSysRatio;       /* 0x0100 ~ 0x0103 System Clock Ratio */
	volatile UINT32 scucSysRtRatio;     /* 0x0104 ~ 0x0107 System RealTime Clock Ratio */
	volatile UINT32 scucSysAhbRatio;    /* 0x0108 ~ 0x010B System AHB Clock Ratio */
	volatile UINT32 scucSysApbRatio;    /* 0x010C ~ 0x010F System APB Clock Ratio */

	volatile UINT32 scucCevaRatio;       /* 0x0110 ~ 0x0113 CEVA Clock Ratio */
	volatile UINT32 scucCevaAhbRatio;    /* 0x0114 ~ 0x0117 CEVA AHB Clock Ratio */
	volatile UINT32 scucCevaApbRatio;    /* 0x0118 ~ 0x011B CEVA APB Clock Ratio */
	volatile UINT32 scucCevaCntEn;       /* 0x011C ~ 0x011F CEVA Counter Enable */

	volatile UINT32 dummy7[8];          /* 0x0120 ~ 0x013F */

	volatile UINT32 scucApolloCfg;      /* 0x0140 ~ 0x0143 APOLLO HPORT Configure */
	volatile UINT32 scucApolloSel;      /* 0x0144 ~ 0x0147 APOLLO HPORT Select */
	volatile UINT32 scucApolloAxiCfg;   /* 0x0148 ~ 0x014B APOLLO AXI Configure */
} scucReg_t;

#define SCU_A_BASE			IO3_ADDRESS(0x7000)
#define SCU_B_BASE			IO0_ADDRESS(0x5000)
#define SCU_C_BASE			IO2_ADDRESS(0x5000)
#define SCU_B_CLK_BASE		(SCU_B_BASE+0x400)

/*  SCU_A  Control Unit */
#define SCUA_A_PERI_RST			(*(volatile unsigned int*)(SCU_A_BASE+0x00))
#define SCUA_A_PERI_CLKEN		(*(volatile unsigned int*)(SCU_A_BASE+0x04))
#define SCUA_A_PERI_DGCLKEN		(*(volatile unsigned int*)(SCU_A_BASE+0x0C))
#define SCUA_LCD_TYPE_SEL		(*(volatile unsigned int*)(SCU_A_BASE+0x10))
#define SCUA_A_PERI_CLKEN2		(*(volatile unsigned int*)(SCU_A_BASE+0x18))
#define SCUA_USBPHY_CFG0		(*(volatile unsigned int*)(SCU_A_BASE+0x38))
#define SCUA_USBPHY_CFG1		(*(volatile unsigned int*)(SCU_A_BASE+0x3C))
#define SCUA_VDAC_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0x40))
#define SCUA_APLL_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0x44))
#define SCUA_EXT_CODEC			(*(volatile unsigned int*)(SCU_A_BASE+0x48))
#define SCUA_SPU_REQ			(*(volatile unsigned int*)(SCU_A_BASE+0x4C))
#define SCUA_CPU_DBG_CTRL_A		(*(volatile unsigned int*)(SCU_A_BASE+0x50))
#define SCUA_CPU_DBG_STAT_A		(*(volatile unsigned int*)(SCU_A_BASE+0x54))
#define SCUA_LCD_CLK_CFG		(*(volatile unsigned int*)(SCU_A_BASE+0x80))
#define SCUA_CSI_CLK_CFG		(*(volatile unsigned int*)(SCU_A_BASE+0x84))
#define SCUA_DUMMY2				(*(volatile unsigned int*)(SCU_A_BASE+0x88))
#define SCUA_DUMMY6				(*(volatile unsigned int*)(SCU_A_BASE+0x8C))
#define SCUA_I2S_BCK_CFG		(*(volatile unsigned int*)(SCU_A_BASE+0x90))
#define SCUA_UART_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0x94))
#define SCUA_DUMMY0				(*(volatile unsigned int*)(SCU_A_BASE+0xA0))
#define SCUA_DUMMY1				(*(volatile unsigned int*)(SCU_A_BASE+0xA4))
#define SCUA_CODEC_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0xB0))
#define SCUA_B210_PRI			(*(volatile unsigned int*)(SCU_A_BASE+0xB4))
#define SCUA_DUMMY3				(*(volatile unsigned int*)(SCU_A_BASE+0xC0))
#define SCUA_DUMMY4				(*(volatile unsigned int*)(SCU_A_BASE+0xC4))
#define SCUA_DUMMY5				(*(volatile unsigned int*)(SCU_A_BASE+0xD0))
#define SCUA_SAR_GPIO_CTRL		(*(volatile unsigned int*)(SCU_A_BASE+0xE0))
#define SCUA_SAR_GPIO_OEN		(*(volatile unsigned int*)(SCU_A_BASE+0xE4))
#define SCUA_SAR_GPIO_O			(*(volatile unsigned int*)(SCU_A_BASE+0xE8))
#define SCUA_SAR_GPIO_I			(*(volatile unsigned int*)(SCU_A_BASE+0xEC))
#define SCUA_MIPI_RX0_CFG		(*(volatile unsigned int*)(SCU_A_BASE+0xF0))

#define SP_SCUB_WFI				(SCU_B_BASE+0x58)

/*  SCU_B  Control Unit */
#define SCUB_B_PERI_RST			(*(volatile unsigned int*)(SCU_B_BASE+0x00))
#define SCUB_SPLL_CFG0			(*(volatile unsigned int*)(SCU_B_BASE+0x04))
#define SCUB_B_INTR_STATUS		(*(volatile unsigned int*)(SCU_B_BASE+0x08))
#define SCUB_TIMER_ICE_EN		(*(volatile unsigned int*)(SCU_B_BASE+0x0C))
#define SCUB_TIMER_EXT_CTRL		(*(volatile unsigned int*)(SCU_B_BASE+0x10))
#define SCUB_OTP0				(*(volatile unsigned int*)(SCU_B_BASE+0x14))
#define SCUB_REV				(*(volatile unsigned int*)(SCU_B_BASE+0x18))
#define SCUB_RAND0				(*(volatile unsigned int*)(SCU_B_BASE+0x1C))
#define SCUB_B_PERI_CLKEN		(*(volatile unsigned int*)(SCU_B_BASE+0x20))
#define SCUB_B_PERI_DBGCLKEN	(*(volatile unsigned int*)(SCU_B_BASE+0x24))
#define SCUB_B_UPDATE_RATIO		(*(volatile unsigned int*)(SCU_B_BASE+0x28))
#define SCUB_CHIP_DBG_CTRL		(*(volatile unsigned int*)(SCU_B_BASE+0x38))
#define SCUB_CHIP_DBG_STAT		(*(volatile unsigned int*)(SCU_B_BASE+0x3C))
#define SCUB_PWRC_CFG			(*(volatile unsigned int*)(SCU_B_BASE+0x40))
#define SCUB_OTP1				(*(volatile unsigned int*)(SCU_B_BASE+0x44))
#define SCUB_RAND1				(*(volatile unsigned int*)(SCU_B_BASE+0x48))
#define SCUB_OTP2				(*(volatile unsigned int*)(SCU_B_BASE+0x4C))
#define SCUB_IO_TRAP			(*(volatile unsigned int*)(SCU_B_BASE+0x50))
#define SCUB_WFI				(*(volatile unsigned int*)(SCU_B_BASE+0x58))

#define SCUB_XCK_CTRL0			(*(volatile unsigned int*)(SCU_B_BASE+0x5C))
#define SCUB_XCK_CTRL1			(*(volatile unsigned int*)(SCU_B_BASE+0x60))
#define SCUB_SPLL0_CFG			(*(volatile unsigned int*)(SCU_B_BASE+0x64))
#define SCUB_SPLL1_CFG			(*(volatile unsigned int*)(SCU_B_BASE+0x68))
#define SCUB_SPLL2_CFG			(*(volatile unsigned int*)(SCU_B_BASE+0x6C))
#define SCUB_SPLL_SEL			(*(volatile unsigned int*)(SCU_B_BASE+0x70))
#define SCUB_SPLL2_SSC_CFG0		(*(volatile unsigned int*)(SCU_B_BASE+0x74))
#define SCUB_SPLL2_SSC_CFG1		(*(volatile unsigned int*)(SCU_B_BASE+0x78))
#define SCUB_SPLL2_SSC_CFG2		(*(volatile unsigned int*)(SCU_B_BASE+0x7C))

#define SCUB_PGS0				(*(volatile unsigned int*)(SCU_B_BASE+0x80))
#define SCUB_PGS1				(*(volatile unsigned int*)(SCU_B_BASE+0x84))
#define SCUB_PGS2				(*(volatile unsigned int*)(SCU_B_BASE+0x88))
#define SCUB_PGS3				(*(volatile unsigned int*)(SCU_B_BASE+0x8C))
//#define SCUB_PGC0				(*(volatile unsigned int*)(SCU_B_BASE+0x90))
//#define SCUB_PGC1				(*(volatile unsigned int*)(SCU_B_BASE+0x94))
//#define SCUB_PGC2				(*(volatile unsigned int*)(SCU_B_BASE+0x98))
//#define SCUB_PGC3				(*(volatile unsigned int*)(SCU_B_BASE+0x9C))
#define SCUB_DUMMYREG0			(*(volatile unsigned int*)(SCU_B_BASE+0xA8))
#define SCUB_DUMMYREG1			(*(volatile unsigned int*)(SCU_B_BASE+0xAC))
#define SCUB_CPU_DBG_CTRL		(*(volatile unsigned int*)(SCU_B_BASE+0xB0))
#define SCUB_CPU_DBG_STAT		(*(volatile unsigned int*)(SCU_B_BASE+0xB4))
#define SCUB_DUMMYREG4			(*(volatile unsigned int*)(SCU_B_BASE+0xBC))
#define SCUB_ARM_RATIO			(*(volatile unsigned int*)(SCU_B_BASE+0xD0))
#define SCUB_ARM_AHB_RATIO		(*(volatile unsigned int*)(SCU_B_BASE+0xD4))
#define SCUB_ARM_APB_RATIO		(*(volatile unsigned int*)(SCU_B_BASE+0xD8))
#define SCUB_SYS_CNT_EN			(*(volatile unsigned int*)(SCU_B_BASE+0xDC))

#define SCUB_GPIO4_IE   		(*(volatile unsigned int*)(SCU_B_BASE+0xF0))
#define SCUB_GPIO4_DS   		(*(volatile unsigned int*)(SCU_B_BASE+0xF4))
#define SCUB_GPIO4_PE   		(*(volatile unsigned int*)(SCU_B_BASE+0xF8))
#define SCUB_GPIO4_PS   		(*(volatile unsigned int*)(SCU_B_BASE+0xFC))

#define SCUB_GPIO0_IE   		(*(volatile unsigned int*)(SCU_B_BASE+0x100))
#define SCUB_GPIO0_DS   		(*(volatile unsigned int*)(SCU_B_BASE+0x104))
#define SCUB_GPIO0_PE   		(*(volatile unsigned int*)(SCU_B_BASE+0x108))
#define SCUB_GPIO0_PS   		(*(volatile unsigned int*)(SCU_B_BASE+0x10C))

#define SCUB_GPIO1_IE   		(*(volatile unsigned int*)(SCU_B_BASE+0x110))
#define SCUB_GPIO1_DS   		(*(volatile unsigned int*)(SCU_B_BASE+0x114))
#define SCUB_GPIO1_PE   		(*(volatile unsigned int*)(SCU_B_BASE+0x118))
#define SCUB_GPIO1_PS   		(*(volatile unsigned int*)(SCU_B_BASE+0x11C))

#define SCUB_GPIO2_IE   		(*(volatile unsigned int*)(SCU_B_BASE+0x120))
#define SCUB_GPIO2_DS   		(*(volatile unsigned int*)(SCU_B_BASE+0x124))
#define SCUB_GPIO2_PE   		(*(volatile unsigned int*)(SCU_B_BASE+0x128))
#define SCUB_GPIO2_PS   		(*(volatile unsigned int*)(SCU_B_BASE+0x12C))

#define SCUB_GPIO3_IE   		(*(volatile unsigned int*)(SCU_B_BASE+0x130))
#define SCUB_GPIO3_DS   		(*(volatile unsigned int*)(SCU_B_BASE+0x134))
#define SCUB_GPIO3_PE   		(*(volatile unsigned int*)(SCU_B_BASE+0x138))
#define SCUB_GPIO3_PS   		(*(volatile unsigned int*)(SCU_B_BASE+0x13C))

#define SCUB_PIN_MUX    		(*(volatile unsigned int*)(SCU_B_BASE+0x144))

#define SCUB_PZONEE_RST    		(*(volatile unsigned int*)(SCU_B_BASE+0x180))
#define SCUB_PZONEE_CLKEN    	(*(volatile unsigned int*)(SCU_B_BASE+0x184))
#define SCUB_MIPI_PIN_SEL    	(*(volatile unsigned int*)(SCU_B_BASE+0x188))
#define SCUB_DISP_VDAC_SEL    	(*(volatile unsigned int*)(SCU_B_BASE+0x18C))
#define SCUB_VIDEO_CODEC_RST    (*(volatile unsigned int*)(SCU_B_BASE+0x190))

#define SCUB_CLK0_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x400))
#define SCUB_CLK0_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x404))
#define SCUB_CLK0_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x408))
#define SCUB_CLK0_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x40C))

#define SCUB_CLK1_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x410))
#define SCUB_CLK1_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x414))
#define SCUB_CLK1_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x418))
#define SCUB_CLK1_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x41C))

#define SCUB_CLK2_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x420))
#define SCUB_CLK2_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x424))
#define SCUB_CLK2_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x428))
#define SCUB_CLK2_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x42C))

#define SCUB_CLK3_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x430))
#define SCUB_CLK3_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x434))
#define SCUB_CLK3_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x438))
#define SCUB_CLK3_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x43C))

#define SCUB_CLK4_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x440))
#define SCUB_CLK4_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x444))
#define SCUB_CLK4_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x448))
#define SCUB_CLK4_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x44C))

#define SCUB_CLK5_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x450))
#define SCUB_CLK5_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x454))
#define SCUB_CLK5_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x458))
#define SCUB_CLK5_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x45C))

#define SCUB_CLK6_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x460))
#define SCUB_CLK6_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x464))
#define SCUB_CLK6_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x468))
#define SCUB_CLK6_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x46C))

#define SCUB_CLK7_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x470))
#define SCUB_CLK7_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x474))
#define SCUB_CLK7_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x478))
#define SCUB_CLK7_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x47C))

#define SCUB_CLK8_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x480))
#define SCUB_CLK8_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x484))
#define SCUB_CLK8_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x488))
#define SCUB_CLK8_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x48C))

#define SCUB_CLK9_CFG0   		(*(volatile unsigned int*)(SCU_B_BASE+0x490))
#define SCUB_CLK9_CFG1   		(*(volatile unsigned int*)(SCU_B_BASE+0x494))
#define SCUB_CLK9_CFG2   		(*(volatile unsigned int*)(SCU_B_BASE+0x498))
#define SCUB_CLK9_CFG3   		(*(volatile unsigned int*)(SCU_B_BASE+0x49C))

/*  SCU_C  Control Unit */
#define SCUC_C_PERI_RST			(*(volatile unsigned int*)(SCU_C_BASE+0x00))
#define SCUC_C_PERI_CLKEN		(*(volatile unsigned int*)(SCU_C_BASE+0x04))
#define SCUC_C_PERI_DGCLKEN		(*(volatile unsigned int*)(SCU_C_BASE+0x08))
#define SCUC_GC_CFG0			(*(volatile unsigned int*)(SCU_C_BASE+0x10))
#define SCUC_DUMMY_C2			(*(volatile unsigned int*)(SCU_C_BASE+0x18))
#define SCUC_SYS_RATIO_UPDATE	(*(volatile unsigned int*)(SCU_C_BASE+0x28))
#define SCUC_ROM_ADDR0_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x2C))
#define SCUC_ROM_DATA0_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x30))
#define SCUC_ROM_DATA1_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x34))
#define SCUC_ROM_DATA2_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x38))
#define SCUC_ROM_DATA3_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x3C))
#define SCUC_DDRPHY_CTRL0		(*(volatile unsigned int*)(SCU_C_BASE+0x48))
#define SCUC_DDRPHY_CTRL1		(*(volatile unsigned int*)(SCU_C_BASE+0x4C))
#define SCUC_DUMMY_C0			(*(volatile unsigned int*)(SCU_C_BASE+0xA0))
#define SCUC_DUMMY_C1			(*(volatile unsigned int*)(SCU_C_BASE+0xA4))
#define SCUC_CPU_DBG_CTRL		(*(volatile unsigned int*)(SCU_C_BASE+0xA8))
#define SCUC_CPU_DBG_STAT		(*(volatile unsigned int*)(SCU_C_BASE+0xAC))
#define SCUC_NR_REG0			(*(volatile unsigned int*)(SCU_C_BASE+0xB0))
#define SCUC_NR_REG1			(*(volatile unsigned int*)(SCU_C_BASE+0xB4))
#define SCUC_NR_REG2			(*(volatile unsigned int*)(SCU_C_BASE+0xB8))
#define SCUC_NR_REG3			(*(volatile unsigned int*)(SCU_C_BASE+0xBC))
#define SCUC_TAS0				(*(volatile unsigned int*)(SCU_C_BASE+0xC0))
#define SCUC_TAS1				(*(volatile unsigned int*)(SCU_C_BASE+0xC4))
#define SCUC_TAS2				(*(volatile unsigned int*)(SCU_C_BASE+0xC8))
#define SCUC_TAS3				(*(volatile unsigned int*)(SCU_C_BASE+0xCC))
#define SCUC_TAS4				(*(volatile unsigned int*)(SCU_C_BASE+0xD0))
#define SCUC_TAS5				(*(volatile unsigned int*)(SCU_C_BASE+0xD4))
#define SCUC_TAS6				(*(volatile unsigned int*)(SCU_C_BASE+0xD8))
#define SCUC_TAS7				(*(volatile unsigned int*)(SCU_C_BASE+0xDC))
#define SCUC_SYS_RATIO			(*(volatile unsigned int*)(SCU_C_BASE+0x100))
#define SCUC_SYS_RT_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x104))
#define SCUC_SYS_AHB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x108))
#define SCUC_SYS_APB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x10C))
#define SCUC_CEVA_RATIO			(*(volatile unsigned int*)(SCU_C_BASE+0x110))
#define SCUC_CEVA_AHB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x114))
#define SCUC_CEVA_APB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x118))
#define SCUC_CEVA_CNT_EN		(*(volatile unsigned int*)(SCU_C_BASE+0x11C))

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif /* _REG_SCU_H_ */
