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
 * @file    reg_gpio.h
 * @brief   Regmap of SPMP8050 GPIO
 * @author  qinjian
 * @since   2010-9-27
 * @date    2010-9-27
 */
#ifndef _REG_GPIO_H_
#define _REG_GPIO_H_

#include <mach/hardware.h>
#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

#define LOGI_ADDR_GPIO_REG		IO0_ADDRESS(0xA000)
//#define LOGI_ADDR_PIMUX_REG	IO0_ADDRESS(0x5144)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gpioReg_s {
	volatile UINT32 gpioEnable0;        /* 0x0000 ~ 0x0003 GPIO Enable */
	volatile UINT32 gpioEnable1;        /* 0x0004 ~ 0x0007 0: Normal mode */
	volatile UINT32 gpioEnable2;        /* 0x0008 ~ 0x000B 1: GPIO mode */
	volatile UINT32 gpioEnable3;        /* 0x000C ~ 0x000F */
	volatile UINT32 gpioEnable4;        /* 0x0010 ~ 0x0013 */

	volatile UINT32 nc0[3];//0x0014~0x001F
	
	volatile UINT32 gpioOutData0;       /* 0x0020 ~ 0x0023 Output data in GPIO mode */
	volatile UINT32 gpioOutData1;       /* 0x0024 ~ 0x0027 */
	volatile UINT32 gpioOutData2;       /* 0x0028 ~ 0x002B */
	volatile UINT32 gpioOutData3;       /* 0x002C ~ 0x002F */
	volatile UINT32 gpioOutData4;       /* 0x0030 ~ 0x0033 */

	volatile UINT32 nc1[3];//0x0034~0x003F
	
	volatile UINT32 gpioDirection0;     /* 0x0040 ~ 0x0043 GPIO Direction */
	volatile UINT32 gpioDirection1;     /* 0x0044 ~ 0x0047 0: Output mode */
	volatile UINT32 gpioDirection2;     /* 0x0048 ~ 0x004B 1: Input mode */
	volatile UINT32 gpioDirection3;     /* 0x004C ~ 0x004F */
	volatile UINT32 gpioDirection4;     /* 0x0050 ~ 0x0053 */

	volatile UINT32 nc2[3];//0x0054~0x005F
	
	volatile UINT32 gpioPolarity0;      /* 0x0060 ~ 0x0063 GPIO Polarity */
	volatile UINT32 gpioPolarity1;      /* 0x0064 ~ 0x0067 0: Active Low */
	volatile UINT32 gpioPolarity2;      /* 0x0068 ~ 0x006B 1: Actice High */
	volatile UINT32 gpioPolarity3;      /* 0x006C ~ 0x006F */
	volatile UINT32 gpioPolarity4;      /* 0x0070 ~ 0x0073 */

	volatile UINT32 nc3[3];//0x0074~0x007F
	
	volatile UINT32 gpioSticky0;        /* 0x0080 ~ 0x0083 GPIO Sticky */
	volatile UINT32 gpioSticky1;        /* 0x0084 ~ 0x0087 0: Direct output */
	volatile UINT32 gpioSticky2;        /* 0x0088 ~ 0x008B 1: Clocked rising edge after polarity change */
	volatile UINT32 gpioSticky3;        /* 0x008C ~ 0x008F */
	volatile UINT32 gpioSticky4;        /* 0x0090 ~ 0x0093 */

	volatile UINT32 nc4[3];//0x0094~0x009F

	volatile UINT32 gpioIntEn0;         /* 0x00A0 ~ 0x00A3 GPIO Interrupt Enable */
	volatile UINT32 gpioIntEn1;         /* 0x00A4 ~ 0x00A7 0: Disable */
	volatile UINT32 gpioIntEn2;         /* 0x00A8 ~ 0x00AB 1: Enable Interrupt */
	volatile UINT32 gpioIntEn3;         /* 0x00AC ~ 0x00AF */
	volatile UINT32 gpioIntEn4;         /* 0x00B0 ~ 0x00B3 */

	volatile UINT32 nc5[3];//0x00B4~0x00BF

	volatile UINT32 gpioIntPending0;    /* 0x00C0 ~ 0x00C3 GPIO Interrupt Pending Signals */
	volatile UINT32 gpioIntPending1;    /* 0x00C4 ~ 0x00C7 */
	volatile UINT32 gpioIntPending2;    /* 0x00C8 ~ 0x00CB */
	volatile UINT32 gpioIntPending3;    /* 0x00CC ~ 0x00CF */
	volatile UINT32 gpioIntPending4;    /* 0x00D0 ~ 0x00D3 */

	volatile UINT32 nc6[3];//0x00D4~0x00DF

	volatile UINT32 gpioDebounceReg0;   /* 0x00E0 ~ 0x00F3 DeBounce counter comparsion register */
	volatile UINT32 gpioDebounceReg1;   /* 0x00E4 ~ 0x00F7 */
	volatile UINT32 gpioDebounceReg2;   /* 0x00E8 ~ 0x00FB */
	volatile UINT32 gpioDebounceReg3;   /* 0x00EC ~ 0x00FF */
	volatile UINT32 gpioDebounceReg4;   /* 0x00F0 ~ 0x00F3 */

	volatile UINT32 nc7[3];//0x00F4~0x00FF

	volatile UINT32 gpioDebounceEn0;    /* 0x0100 ~ 0x0103 GPIO DeBounce Enable */
	volatile UINT32 gpioDebounceEn1;    /* 0x0104 ~ 0x0107 0: Disable */
	volatile UINT32 gpioDebounceEn2;    /* 0x0108 ~ 0x010B 1: Enable DeBounce */
	volatile UINT32 gpioDebounceEn3;    /* 0x010C ~ 0x010F */
	volatile UINT32 gpioDebounceEn4;    /* 0x0110 ~ 0x0113 */

	volatile UINT32 nc8[3];//0x0114~0x011F

	volatile UINT32 gpioWakeupEn0;      /* 0x0120 ~ 0x0123 GPIO Wake-up Enable */
	volatile UINT32 gpioWakeupEn1;      /* 0x0124 ~ 0x0127 0: Disable */
	volatile UINT32 gpioWakeupEn2;      /* 0x0128 ~ 0x012B 1: Enable Wake-up */
	volatile UINT32 gpioWakeupEn3;      /* 0x012C ~ 0x012F */
	volatile UINT32 gpioWakeupEn4;      /* 0x0130 ~ 0x0133 */

	volatile UINT32 nc9[3];//0x0134~0x013F

	volatile UINT32 gpioStatus0;        /* 0x0140 ~ 0x0143 GPIO Status (Input data) */
	volatile UINT32 gpioStatus1;        /* 0x0144 ~ 0x0147 */
	volatile UINT32 gpioStatus2;        /* 0x0148 ~ 0x014B */
	volatile UINT32 gpioStatus3;        /* 0x014C ~ 0x014F */
	volatile UINT32 gpioStatus4;        /* 0x0150 ~ 0x0153 */
} gpioReg_t;

typedef struct pinmuxReg_s {
	volatile UINT32 pinmux1;
	volatile UINT32 rsv004[1];
	volatile UINT32 pinmux2;
} pinmuxReg_t;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif /* _REG_GPIO_H_ */
