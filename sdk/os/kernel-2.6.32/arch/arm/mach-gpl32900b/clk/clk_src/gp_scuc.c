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
 * @file	gp_scuc.c
 * @brief	Clock tree of SCU_C.
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

struct clk_scuc {
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

static struct clk_scuc	gp_scuc[] = {
	{
		.name 				= "SCU_C",
		.parent_name 		= "clk_sys",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 5,
		},
	},
	{
		.name 				= "FABRIC_C",
		.parent_name 		= "SCU_C",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 0,
		},
	},
	{
		.name 				= "SDMA",
		.parent_name 		= "FABRIC_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 1,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 1,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "OVG",
		.parent_name 		= "FABRIC_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 3,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 29,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "DRAM_CTRL",
		.parent_name 		= "FABRIC_C",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 4,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 4,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "I2C_CFG",					/* DDR PHY config */
		.parent_name 		= "FABRIC_C",
		.para	={
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 6,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "APBDMA_C",					/* APBDMA 0 */
		.parent_name 		= "FABRIC_C",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 7,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 7,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "INT_MEM",					/* Internal RAM clock */
		.parent_name 		= "FABRIC_C",
		.flag				= CLK_IGNORE_UNUSED,
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 12,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 12,
			.flag			= CLK_INIT_EN,
		},
	},
	{
		.name 				= "UART_C0",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 13,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 13,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "UART_C2",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 15,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 15,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SPI0",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 16,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 16,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SPI1",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 17,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 17,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SD0",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 18,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 18,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SD1",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 19,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 19,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "I2C_C",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 20,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 20,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "TI2C",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 23,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 23,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "CIR",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 27,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 27,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "EFUSE",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 29,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 30,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SD2",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 31,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 31,
			.flag			= CLK_INIT_DIS,// | CLK_RST_DIS,
		},
	},
	{
		.name 				= "2DSCALEABT",
		.parent_name 		= "APBDMA_C",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 22,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 22,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "SCALER2",
		.parent_name 		= "2DSCALEABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 21,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 21,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
	{
		.name 				= "EMAC",
		.parent_name 		= "2DSCALEABT",
		.para	={
			.gate_reg		= (void __iomem *)&SCUC_C_PERI_CLKEN,
			.gate_bit		= 30,
			.reset_reg		= (void __iomem *)&SCUC_C_PERI_RST,
			.reset_bit		= 24,
			.flag			= CLK_INIT_DIS | CLK_RST_DIS,
		},
	},
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		SCU_C clock initialization function.
* @return		None.
*/
void __init gp_scuc_clk_init(void)
{
	struct clk *clk;
	unsigned long i;
	int ret;
	
	for ( i=0 ; i<ARRAY_SIZE(gp_scuc); i++ )
	{
		clk = clk_register_gp_gate(NULL, gp_scuc[i].name, gp_scuc[i].parent_name, gp_scuc[i].flag, &gp_scuc[i].para  );
		if (IS_ERR(clk))
		{
			pr_err("%s not registered\n", gp_scuc[i].name );
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