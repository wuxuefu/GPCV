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
 * @file	gp_clk_gate.c
 * @brief	GP gated clock implement.
 * @author	Dunker Chen
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <linux/gcd.h>
#include <asm/clkdev.h>
#include <mach/clk/clk-private.h>
#include <mach/clk/gp_clock_gate.h>
#include <mach/hal/regmap/reg_scu.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

#define to_clk_gate(_hw) container_of(_hw, struct clk_gp_gate, hw)
#define to_clk_gp_para(_hw) (&(to_clk_gate(_hw)->para))

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

struct clk_gp_gate {
	struct clk_hw				hw;
	struct clk_gp_gate_para		para;
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

static int clk_gp_gate_enable( struct clk_hw *hw );
static void clk_gp_gate_disable( struct clk_hw *hw );
static int clk_gp_gate_is_enabled( struct clk_hw *hw );
static void clk_gp_gate_init( struct clk_hw *hw );

/**************************************************************************
 *                         G L O B A L    D A T A                         *
**************************************************************************/

const struct clk_ops clk_gp_gate_ops = {
	.enable			= clk_gp_gate_enable,
	.disable		= clk_gp_gate_disable,
	.is_enabled		= clk_gp_gate_is_enabled,
	.init			= clk_gp_gate_init,
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		GP clock reset enable/disable.
* @param 		hw[in]: Hardware-specific structure.
* @param 		enable[in]: Enable(1)/disable(0).
* @return		None.
*/
static void clk_gp_reset_endisable(
	struct clk_hw *hw,
	int enable)
{
	struct clk_gp_gate_para *para = to_clk_gp_para(hw);
	u32 val;
	
	if( para->reset_reg == NULL )
		return;
	
	val = readl(para->reset_reg);
	
	if (enable)
		val |= BIT(para->reset_bit);
	else
		val &= ~BIT(para->reset_bit);
		
	writel(val, para->reset_reg);	
}

/**
* @brief 		GP clock gate enable/disable.
* @param 		hw[in]: Hardware-specific structure.
* @param 		enable[in]: Enable(1)/disable(0).
* @return		None.
*/
static void clk_gp_gate_endisable(
	struct clk_hw *hw,
	int enable)
{
	struct clk_gp_gate_para *para = to_clk_gp_para(hw);
	u32 val;
	
	if( para->gate_reg == NULL )
		return;
	
	val = readl(para->gate_reg);
	
	if (enable)
		val |= BIT(para->gate_bit);
	else
		val &= ~BIT(para->gate_bit);
		
	writel(val, para->gate_reg);	
}

/**
* @brief 		GP clock gate enable.
* @param 		hw[in]: Hardware-specific structure.
* @return		0.
*/
static int clk_gp_gate_enable(
	struct clk_hw *hw)
{
	struct clk_gp_gate_para *para = to_clk_gp_para(hw);
	
	if( para->flag & CLK_RST_EN )
		clk_gp_reset_endisable( hw, 1 );
	
	clk_gp_reset_endisable( hw, 0 );
	
	clk_gp_gate_endisable( hw, 1 );
	
	return 0;
}

/**
* @brief 		GP clock gate disable.
* @param 		hw[in]: Hardware-specific structure.
* @return		None.
*/
static void clk_gp_gate_disable(
	struct clk_hw *hw)
{
	struct clk_gp_gate_para *para = to_clk_gp_para(hw);
	
	if( para->flag & CLK_INIT_DIS )
		clk_gp_gate_endisable( hw, 0 );
	
	if( para->flag & CLK_RST_DIS )
		clk_gp_reset_endisable( hw, 1 );
}

/**
* @brief 		GP clock gate check enable.
* @param 		hw[in]: Hardware-specific structure.
* @return		1 for enable, 0 for disable.
*/
static int clk_gp_gate_is_enabled(
	struct clk_hw *hw)
{
	struct clk_gp_gate_para *para = to_clk_gp_para(hw);
	u32 val;
	
	if( para->gate_reg == NULL )
		return 1;
	
	val = readl(para->gate_reg);
	
	return ( val & BIT(para->gate_bit) ) ? 1 : 0;
}

/**
* @brief 		GP clock gate initial function.
* @param 		hw[in]: Hardware-specific structure.
* @return		None.
*/
static void clk_gp_gate_init(
	struct clk_hw *hw)
{
	struct clk_gp_gate_para *para = to_clk_gp_para(hw);
	
	if( para->flag & CLK_INIT_EN )
		clk_gp_gate_enable( hw );
		
	if(  para->flag & CLK_INIT_DIS )
		clk_gp_gate_disable( hw );
}

/**
* @brief 		GP clock gate register.
* @param 		dev[in]: Device that is registering this clock.
* @param 		name[in]: Name of this clock.
* @param 		parent_name[in]: Name of this clock's parent.
* @param 		flags[in]: Framework-specific flags for this clock.
* @param 		para[in]: GP clock gate parameter.
* @return		Clock struct pointer.
*/
struct clk *clk_register_gp_gate(
	struct device *dev,
	const char *name,
	const char *parent_name,
	unsigned long flags,
	struct clk_gp_gate_para *para)
{
	struct clk_gp_gate *gate;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate the gate */
	gate = kzalloc(sizeof(struct clk_gp_gate), GFP_KERNEL);
	if (!gate) {
		pr_err("%s: could not allocate gated clk\n", name);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.ops = &clk_gp_gate_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = (parent_name ? &parent_name: NULL);
	init.num_parents = (parent_name ? 1 : 0);

	/* struct clk_gate assignments */
	memcpy( &gate->para, para, sizeof(struct clk_gp_gate_para) );
	
	gate->hw.init = &init;

	clk = clk_register(dev, &gate->hw);

	if (IS_ERR(clk))
		kfree(gate);

	return clk;
}