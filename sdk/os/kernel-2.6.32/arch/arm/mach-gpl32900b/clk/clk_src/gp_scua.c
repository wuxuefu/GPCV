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
 * @file	gp_scua.c
 * @brief	Clock tree of SCU_A.
 * @author	Dunker Chen
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <linux/gcd.h>
#include <asm/clkdev.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/clk/clk-private.h>
#include <mach/clk/gp_clock_gate.h>
#include <mach/hal/regmap/reg_scu.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

struct clk_scua {
	const char					*name;
	const char					*parent_name;
	unsigned long 				flag;
	struct clk_gp_gate_para		para;
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
**************************************************************************/

static struct clk_scua	gp_scua[] = {
	{
		.name 				= "SCU_A",
		.parent_name 		= "clk_sys",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 6,
		},
	},
	{
		.name 				= "FABRIC_A",
		.parent_name 		= "SCU_A",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 0,
		},
	},
	{
		.name 				= "SYS_A",				/* System A control register */
		.parent_name 		= "FABRIC_A",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 24,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 23,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "AAHBM212",
		.parent_name 		= "SYS_A",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 16,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 16,
			.flag			= CLK_INIT_EN,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "NAND_ABT",
		.parent_name 		= "AAHBM212",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 20,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 20,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "RTABT212",
		.parent_name 		= "AAHBM212",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 22,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 22,
			.flag			= CLK_INIT_EN,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "REALTIME_ABT",
		.parent_name 		= "RTABT212",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 21,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 21,
			.flag			= CLK_INIT_EN,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "LCD_CTRL",
		.parent_name 		= "REALTIME_ABT",
		.para	={
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 1,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "ENCODER_INTF",
		.parent_name 		= "clk_ref_encoder",
		.para	={
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 2,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "DECODER_INTF",
		.parent_name 		= "clk_ref_decoder",
		.para	={
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 9,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "USB_HOST0",
		.parent_name 		= "AAHBM212",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 3,
			.reset_reg		= (void __iomem *) &SCUA_A_PERI_RST,
			.reset_bit		= 3,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "USB_DEVICE",
		.parent_name 		= "AAHBM212",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 4,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 4,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "USB_HOST1",
		.parent_name 		= "AAHBM212",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 2,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 5,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "FD",
		.parent_name 		= "REALTIME_ABT",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 1,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 7,
			.flag			= CLK_INIT_EN,//CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "APBDMA_A",
		.parent_name 		= "REALTIME_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 9,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 8,
			.flag			= CLK_INIT_EN,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "CMOS_CTRL",
		.parent_name 		= "REALTIME_ABT",
		.para	={
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 10,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "NAND0",
		.parent_name 		= "NAND_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 11,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 11,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "BCH",
		.parent_name 		= "NAND_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 13,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 13,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "I2S",
		.parent_name 		= "APBDMA_A",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 17,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 17,
			.flag			= CLK_INIT_EN,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "I2SRX",
		.parent_name 		= "APBDMA_A",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 18,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 18,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SAACC",
		.parent_name 		= "APBDMA_A",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 19,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 19,
			.flag			= CLK_INIT_EN,// | CLK_RST_DIS,
	//		.flag			= CLK_INIT_DIS,// | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SPU",
		.parent_name 		= "APBDMA_A",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 25,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 25,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "LBP",
		.parent_name 		= "REALTIME_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 7,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 26,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "MIPI",
		.parent_name 		= "REALTIME_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 28,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 28,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SCALER0",
		.parent_name 		= "REALTIME_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 26,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 27,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "AES",
		.parent_name 		= "NAND_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN,
			.gate_bit		= 30,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 29,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	/* ----- new SCUA ----- */
	{
		.name 				= "PPU_SPR",			/* PPU internal RAM to sprite RAM */
		.parent_name 		= "PPU",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN2,
			.gate_bit		= 0,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "PPU",
		.parent_name 		= "PPU_REG",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN2,
			.gate_bit		= 24,
			.reset_reg		= (void __iomem *)&SCUA_A_PERI_RST,
			.reset_bit		= 24,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "PPU_REG",
		.parent_name 		= "PPU_FB",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN2,
			.gate_bit		= 25,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "PPU_TFT",
		.parent_name 		= "PPU_REG",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN2,
			.gate_bit		= 26,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "PPU_STN",
		.parent_name 		= "PPU_REG",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN2,
			.gate_bit		= 27,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "PPU_TV",
		.parent_name 		= "PPU_REG",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN2,
			.gate_bit		= 28,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "PPU_FB",
		.parent_name 		= "REALTIME_ABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUA_A_PERI_CLKEN2,
			.gate_bit		= 29,
			.flag			= CLK_INIT_DIS,
		},
	},
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		SCU_A clock initialization function.
* @return		None.
*/
void __init gp_scua_clk_init(void)
{
	struct clk *clk;
	unsigned long i;
	int ret;
	
	for ( i=0 ; i<ARRAY_SIZE(gp_scua); i++ )
	{
		clk = clk_register_gp_gate(NULL, gp_scua[i].name, gp_scua[i].parent_name, gp_scua[i].flag, &gp_scua[i].para  );
		if (IS_ERR(clk))
		{
			pr_err("%s not registered\n", gp_scua[i].name );
			break;
		}
		ret = clk_register_clkdev( clk, clk->name, NULL );
		if (ret<0)
		{
			pr_err("%s not registered clkdev\n", clk->name );	
			break;
		}	
		gp_clk_print( CLK_INFO, clk);
	}
}