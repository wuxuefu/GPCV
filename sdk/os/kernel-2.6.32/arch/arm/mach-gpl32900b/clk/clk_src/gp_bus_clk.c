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
 * @file	gp_bus_clk.c
 * @brief	Clock tree of bus clock (include ARM and system bus).
 * @author	Dunker Chen
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <linux/gcd.h>
#include <asm/clkdev.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/clk/clk-private.h>
#include <mach/hal/regmap/reg_scu.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

#define to_clk_gp_bus(_hw) container_of(_hw, struct clk_gp_bus, hw)
#define to_clk_gp_para(_hw) (to_clk_gp_bus(_hw)->para)
#define div_mask(d)	((1 << ((d)->width)) - 1)

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

struct clk_gp_bus_para{
	const char		*name;
	const char		*parent_name;
	void __iomem	*ratio_reg;				/* Divider ratio register*/
	void __iomem	*up_reg;				/* Update divider ratio register*/
	void __iomem	*en_reg;				/* Clock enable register*/
	u8				shift;
	u8				width;
	u8				up_bit;
	u8				en_bit;
	u8				flags;
	u8				is_uart;
}; 

struct clk_gp_bus {
	struct clk_hw				hw;
	struct clk_gp_bus_para		*para;
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

static unsigned long clk_gp_bus_recalc_rate( struct clk_hw *hw, unsigned long parent_rate );
static long clk_gp_bus_round_rate( struct clk_hw *hw, unsigned long rate, unsigned long *parent );
static int clk_gp_bus_set_rate( struct clk_hw *hw, unsigned long rate, unsigned long parent_rate );
static int clk_gp_bus_enable( struct clk_hw *hw );
static void clk_gp_bus_disable( struct clk_hw *hw );
static int clk_gp_bus_is_enabled( struct clk_hw *hw );

static u8 uart_clk_get_parent( struct clk_hw *hw );
static int uart_clk_set_parent( struct clk_hw *hw, u8 index );
static void uart_clk_init( struct clk_hw *hw );

/**************************************************************************
 *                         G L O B A L    D A T A                         *
**************************************************************************/

const struct clk_ops clk_gp_bus_ops = {
	.recalc_rate = clk_gp_bus_recalc_rate,
	.round_rate = clk_gp_bus_round_rate,
	.set_rate = clk_gp_bus_set_rate,
};

const struct clk_ops clk_gp_bus_ops_gate = {
	.enable			= clk_gp_bus_enable,
	.disable		= clk_gp_bus_disable,
	.is_enabled		= clk_gp_bus_is_enabled,
	.recalc_rate	= clk_gp_bus_recalc_rate,
	.round_rate		= clk_gp_bus_round_rate,
	.get_parent		= uart_clk_get_parent,
};

const struct clk_ops clk_uart_ops = {
	.enable			= clk_gp_bus_enable,
	.disable		= clk_gp_bus_disable,
	.is_enabled		= clk_gp_bus_is_enabled,
	.recalc_rate	= clk_gp_bus_recalc_rate,
	.round_rate		= clk_gp_bus_round_rate,
	.set_rate		= clk_gp_bus_set_rate,
	.set_parent		= uart_clk_set_parent,
	.get_parent		= uart_clk_get_parent,
	.init			= uart_clk_init,
};

static const char *uart_clk_parents[] = {
	"clk_sys",
	"xtal",
};

const struct clk_gp_bus_para bus_init[] = {
	{
		.name = "clk_arm",
		.parent_name = "clk_ref_arm",
		.ratio_reg = (void __iomem *)&SCUB_ARM_RATIO,
		.up_reg = (void __iomem *)&SCUB_B_UPDATE_RATIO,
		.width = 6,
		.up_bit = 0,
	},
	{
		.name = "clk_arm_ahb",
		.parent_name = "clk_arm",
		.ratio_reg = (void __iomem *)&SCUB_ARM_AHB_RATIO,
		.up_reg = (void __iomem *)&SCUB_B_UPDATE_RATIO,
		.width = 6,
		.up_bit = 1,
	},
	{
		.name = "clk_arm_apb",
		.parent_name = "clk_arm_ahb",
		.ratio_reg = (void __iomem *)&SCUB_ARM_APB_RATIO,
		.up_reg = (void __iomem *)&SCUB_B_UPDATE_RATIO,
		.width = 6,
		.up_bit = 2,
	},
	{
		.name = "clk_sys",
		.parent_name = "clk_ref_sys",
		.ratio_reg = (void __iomem *)&SCUC_SYS_RATIO,
		.up_reg = (void __iomem *)&SCUC_SYS_RATIO_UPDATE,
		.width = 6,
		.up_bit = 0,
	},
	{
		.name = "clk_sys_ahb",
		.parent_name = "clk_sys",
		.ratio_reg = (void __iomem *)&SCUC_SYS_AHB_RATIO,
		.up_reg = (void __iomem *)&SCUC_SYS_RATIO_UPDATE,
		.width = 6,
		.up_bit = 2,
	},
	{
		.name = "clk_sys_apb",
		.parent_name = "clk_sys_ahb",
		.ratio_reg = (void __iomem *)&SCUC_SYS_APB_RATIO,
		.up_reg = (void __iomem *)&SCUC_SYS_RATIO_UPDATE,
		.width = 6,
		.up_bit = 3,
	},
	{
		.name = "clk_i2srx0_mclk",
		.parent_name = "clk_ref_audio",
		.ratio_reg = (void __iomem *)&SCUA_APLL_CFG,
		.width = 8,
		.shift = 16,
	},
	{
		.name = "clk_i2stx0_mclk",
		.parent_name = "clk_ref_audio",
		.ratio_reg = (void __iomem *)&SCUA_APLL_CFG,
		.width = 8,
		.shift = 24,
	},
	{
		.name = "clk_i2srx0_bck",
		.parent_name = "clk_i2stx0_mclk",
		.ratio_reg = (void __iomem *)&SCUA_I2S_BCK_CFG,
		.en_reg = (void __iomem *)&SCUA_I2S_BCK_CFG,
		.width = 8,
		.en_bit = 8,
	},
	{
		.name = "clk_csi",
		.parent_name = "clk_sys",
		.ratio_reg = (void __iomem *)&SCUA_CSI_CLK_CFG,
		.en_reg = (void __iomem *)&SCUA_CSI_CLK_CFG,
		.width = 8,
		.en_bit = 8,
	},
	{
		.name = "clk_mipi_rx0",
		.parent_name = "pll0",
		.ratio_reg = (void __iomem *)&SCUA_MIPI_RX0_CFG,
		.en_reg = (void __iomem *)&SCUA_MIPI_RX0_CFG,
		.width = 8,
		.en_bit = 8,
	},
	{
		.name = "clk_uart",
		.ratio_reg = (void __iomem *)&SCUA_UART_CFG,
		.en_reg = (void __iomem *)&SCUA_UART_CFG,
		.width = 8,
		.en_bit = 8,
		.is_uart = 1,
	},
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		Re-calculate bus clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Clock rate.
*/
static unsigned long clk_gp_bus_recalc_rate(
	struct clk_hw *hw,
	unsigned long parent_rate)
{
	struct clk_gp_bus_para *para = to_clk_gp_para(hw);
	unsigned int div;

	div = readl(para->ratio_reg) >> para->shift;
	div = (div & div_mask(para)) + 1;
	
	return parent_rate / div;
}

/**
* @brief 		Bus clock calculate most close clock rate form input rate.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Most close clock rate form input rate.
*/
static long clk_gp_bus_round_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long *parent)
{
	unsigned long div;
	struct clk_gp_bus_para *para = to_clk_gp_para(hw);
	unsigned long io_gcd = gcd( rate, *parent );
	unsigned long parent_clock = *parent / io_gcd;
	rate = rate / io_gcd;
	
	div = ( ( parent_clock * 10 / rate ) + 5 ) / 10 ;
	
	if ( div < 1)
		div = 1;	
	if( div > div_mask(para) )
		div = div_mask(para) ;

	return *parent / div;
}

/**
* @brief 		Set bus clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		0 on success, -EIO otherwise.
*/
static int clk_gp_bus_set_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long parent_rate)
{
	struct clk_gp_bus_para *para = to_clk_gp_para(hw);
	unsigned long div = parent_rate / rate;
	unsigned long cur_div;
	u32 val;
	
	cur_div = readl(para->ratio_reg) >> para->shift;
	cur_div = (cur_div & div_mask(para)) + 1;
	
	if( cur_div == div )
		return 0;
		
	val = readl(para->ratio_reg);
	val &= ~(div_mask(para) << para->shift);
	val |= div << para->shift;
	writel(val, para->ratio_reg);
	if(para->up_reg)
		writel(BIT(para->up_bit), para->up_reg);
	
	return 0;
}

/**
* @brief 		Bus clock enable.
* @param 		hw[in]: Hardware-specific structure.
* @return		0.
*/
static int clk_gp_bus_enable(
	struct clk_hw *hw)
{
	struct clk_gp_bus_para *para = to_clk_gp_para(hw);
	u32 val = readl(para->en_reg);
	
	val |= BIT(para->en_bit);
	writel(val, para->en_reg);
	
	return 0;
}

/**
* @brief 		Bus clock disable.
* @param 		hw[in]: Hardware-specific structure.
* @return		None.
*/
static void clk_gp_bus_disable(
	struct clk_hw *hw)
{
	struct clk_gp_bus_para *para = to_clk_gp_para(hw);
	u32 val = readl(para->en_reg);
	
	val &= ~BIT(para->en_bit);
	writel(val, para->en_reg);
}

/**
* @brief 		Bus check clock enable.
* @param 		hw[in]: Hardware-specific structure.
* @return		1 for enable, 0 for disable.
*/
static int clk_gp_bus_is_enabled(
	struct clk_hw *hw)
{
	struct clk_gp_bus_para *para = to_clk_gp_para(hw);
	u32 val = readl(para->en_reg);
	
	return ( val & BIT(para->en_bit) ) ? 1 : 0;
}

/**
* @brief 		Get uart clock's parent index.
* @param 		hw[in]: Hardware-specific structure.
* @return		Parent index.
*/
static u8 uart_clk_get_parent(
	struct clk_hw *hw)
{
	return ( SCUA_UART_CFG >> 18 ) & 0x01 ;
}

/**
* @brief 		Set uart clock's parent.
* @param 		hw[in]: Hardware-specific structure.
* @param 		index[in]: Parent index.
* @return		0 on success, -EIO otherwise.
*/
static int uart_clk_set_parent(
	struct clk_hw *hw,
	u8 index)
{
	if(index)
		SCUA_UART_CFG |= ( BIT(16)|BIT(18) ) ;
	else
		SCUA_UART_CFG &= ~( BIT(16)|BIT(18) ) ;
	
	return 0;
}

/**
* @brief 		UART initial function.
* @param 		hw[in]: Hardware-specific structure.
* @return		None.
*/
static void uart_clk_init(
	struct clk_hw *hw)
{
	/* ----- Set UART 0 and UART 1 with the same parent clock ----- */
	if( SCUA_UART_CFG & BIT(18) )
		SCUA_UART_CFG |= ( BIT(16)|BIT(18) ) ;
	else
		SCUA_UART_CFG &= ~( BIT(16)|BIT(18) ) ;
}

/**
* @brief 		Register GP bus clock.
* @param 		dev[in]: Device node.
* @param 		bus[in]: GP bus clock parameter.
* @return		Clock struct pointer.
*/
static struct clk *register_gp_bus(
	struct device *dev,
	struct clk_gp_bus_para* para)
{
	struct clk_gp_bus *div;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate the divider */
	div = kzalloc(sizeof(struct clk_gp_bus), GFP_KERNEL);
	if (!div) 
	{
		pr_err("%s: could not allocate %s clk\n", __func__, para->name);
		return ERR_PTR(-ENOMEM);
	}

	init.name = para->name;
	init.flags = para->flags | CLK_IS_BASIC;
	if( para->is_uart )
	{
		init.ops = &clk_uart_ops;
		init.parent_names = uart_clk_parents;
		init.num_parents = ARRAY_SIZE( uart_clk_parents );
	}
	else
	{
		init.ops = (para->en_reg) ? &clk_gp_bus_ops_gate : &clk_gp_bus_ops;	
		init.parent_names = (para->parent_name ? &para->parent_name: NULL);
		init.num_parents = (para->parent_name ? 1 : 0);
	}
	/* struct clk_gp_bus assignments */
	div->para = para;
	div->hw.init = &init;
	
	/* register the clock */
	clk = clk_register(dev, &div->hw);

	if (IS_ERR(clk))
		kfree(div);

	return clk;
}

/**
* @brief 		Bus clock initialization function.
* @return		None.
*/
void __init gp_bus_clk_init(void)
{
	struct clk *clk;
	unsigned long i;
	int ret;
	
	for ( i=0 ; i<ARRAY_SIZE(bus_init); i++ )
	{
		clk = register_gp_bus(NULL, (struct clk_gp_bus_para*) &bus_init[i]);
		if (IS_ERR(clk))
		{
			pr_err("%s not registered\n", bus_init[i].name);
			break;
		}
		ret = clk_register_clkdev( clk, clk->name, NULL );
		if (ret<0)
		{
			pr_err("%s not registered clkdev\n", clk->name );	
			break;
		}	
		gp_clk_print( CLK_DEBUG, clk);
	}
}