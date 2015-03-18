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
 * @file	gp_scub.c
 * @brief	Clock tree of SCU_B and SCU_E.
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

struct clk_scub {
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

static struct clk_scub	gp_scub[] = {
	{
		.name 				= "TCM_BIST",
		.parent_name 		= "clk_arm",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 0,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "TCM_CTRL",
		.parent_name 		= "clk_arm",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 1,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "AHB2AHB",
		.parent_name 		= "clk_arm_ahb",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 2,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 2,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "AXI1AHB",
		.parent_name 		= "clk_arm",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 3,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "VIC0",
		.parent_name 		= "AHB2AHB",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 4,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 4,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "VIC1",
		.parent_name 		= "AHB2AHB",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 5,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 5,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "TIMER",
		.parent_name 		= "AHB2AHB",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 9,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 9,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "WDT",						/* WDT */
		.parent_name 		= "AHB2AHB",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 10,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 10,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "ARM_I2C",
		.parent_name 		= "AHB2AHB",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 12,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 12,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "RAND",
		.parent_name 		= "AHB2AHB",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 13,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 13,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "GPIO",
		.parent_name 		= "AHB2AHB",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 14,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 14,
			.flag			= CLK_INIT_EN,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,//CLK_INIT_EN,
		},
	},
	{
		.name 				= "RTC",
		.parent_name 		= "AHB2AHB",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_B_PERI_CLKEN,
			.gate_bit		= 15,
			.reset_reg		= (void __iomem *)&SCUB_B_PERI_RST,
			.reset_bit		= 15,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	/* ----- SCUE ----- */
	{
		.name 				= "HERMES",
		.parent_name 		= "clk_sys",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 0,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 0,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "AHBSW",
		.parent_name 		= "HERMES",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 1,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 1,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "SCU_E",
		.parent_name 		= "AHBSW",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 3,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 3,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "DISP0",
		.parent_name 		= "SCU_E",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 2,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 2,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "I2S_TX1",
		.parent_name 		= "SCU_E",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 4,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 4,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "CDSP",
		.parent_name 		= "SCU_E",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 6,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 6,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "MIPI_RX1",
		.parent_name 		= "SCU_E",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 7,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 7,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "CDSP_PCLK",
		.parent_name 		= "CDSP",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 16,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "CDSP_PCLK_D2",
		.parent_name 		= "CDSP",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 17,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "MIPI_PHY",
		.parent_name 		= "SCU_E",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 18,
			.flag			= CLK_INIT_DIS,
		},
	},
	{
		.name 				= "DISP0_PCLK",
		.parent_name 		= "DISP0",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 19,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 19,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "DISP0_HDMI",
		.parent_name 		= "DISP0",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 20,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 20,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "I2S_TX1_CLK",
		.parent_name 		= "I2S_TX1",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 21,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 21,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "DISP0_LVDS",
		.parent_name 		= "DISP0",
		.para	={
			.gate_reg		= (void __iomem *)&SCUB_PZONEE_CLKEN,
			.gate_bit		= 24,
			.reset_reg		= (void __iomem *)&SCUB_PZONEE_RST,
			.reset_bit		= 24,
		//	.flag			= CLK_INIT_DIS | CLK_RST_DIS,
			.flag			= CLK_INIT_EN,
		},
	},
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		SCU_B clock initialization function.
* @return		None.
*/
void __init gp_scub_clk_init(void)
{
	struct clk *clk;
	unsigned long i;
	int ret;
	
	for ( i=0 ; i<ARRAY_SIZE(gp_scub); i++ )
	{
		clk = clk_register_gp_gate(NULL, gp_scub[i].name, gp_scub[i].parent_name, gp_scub[i].flag, &gp_scub[i].para  );
		if (IS_ERR(clk))
		{
			pr_err("%s not registered\n", gp_scub[i].name );
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