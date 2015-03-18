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
 * @file	gp_pll.c
 * @brief	Clock tree of PLL (all clock source).
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

#define XTAL_RATE 27000000  //27 MHz
#define	SCU_A_PERI_APLL				BIT(14)

#define KHZ     	1000
#define MHZ     	(KHZ * KHZ)

#define APLL_CLK1	73728000
#define APLL_CLK2	67736000

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

#define to_pllclk(_hw) container_of(_hw, struct clk_pll, hw)

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

struct clk_pll {
	struct clk_hw	hw;
	u8 				id;
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

static unsigned long pll_clk_recalc_rate( struct clk_hw *hw, unsigned long parent_rate );
static long pll_clk_round_rate( struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate );
static int pll_clk_set_rate( struct clk_hw *hw, unsigned long rate, unsigned long parent_rate );
static unsigned long apll_clk_recalc_rate( struct clk_hw *hw, unsigned long parent_rate );
static long apll_clk_round_rate( struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate );
static int apll_clk_set_rate( struct clk_hw *hw, unsigned long rate, unsigned long parent_rate );
static int apll_clk_enable( struct clk_hw *hw );
static void apll_clk_disable( struct clk_hw *hw );
static int apll_clk_is_enabled( struct clk_hw *hw );

/**************************************************************************
 *                         G L O B A L    D A T A                         *
**************************************************************************/

static struct clk_ops std_pll_ops = {
	.recalc_rate	= pll_clk_recalc_rate,
	.round_rate		= pll_clk_round_rate,
	.set_rate		= pll_clk_set_rate,
};

static struct clk_ops apll_ops = {
	.enable			= apll_clk_enable,
	.disable		= apll_clk_disable,
	.is_enabled		= apll_clk_is_enabled,
	.recalc_rate	= apll_clk_recalc_rate,
	.round_rate		= apll_clk_round_rate,
	.set_rate		= apll_clk_set_rate,
};

static const char *pll_clk_parents[] = {
	"xtal",
};

static struct clk_init_data clk_pll0_init = {
	.name			= "pll0",
	.ops			= &std_pll_ops,
	.parent_names	= pll_clk_parents,
	.num_parents	= ARRAY_SIZE(pll_clk_parents),
	.flags			= CLK_GET_RATE_NOCACHE,
};

static struct clk_init_data clk_pll1_init = {
	.name			= "pll1",
	.ops			= &std_pll_ops,
	.parent_names	= pll_clk_parents,
	.num_parents	= ARRAY_SIZE(pll_clk_parents),
	.flags			= CLK_GET_RATE_NOCACHE,
};

static struct clk_init_data clk_pll2_init = {
	.name			= "pll2",
	.ops			= &std_pll_ops,
	.parent_names	= pll_clk_parents,
	.num_parents	= ARRAY_SIZE(pll_clk_parents),
	.flags			= CLK_GET_RATE_NOCACHE,
};

static struct clk_init_data clk_apll_init = {
	.name			= "apll",
	.ops			= &apll_ops,
	.parent_names	= NULL,
	.num_parents	= 0,
	.flags			= CLK_IS_ROOT|CLK_GET_RATE_NOCACHE,
};

static struct clk_pll clk_pll0 = {
	.id = '0',
	.hw = {
		.init = &clk_pll0_init,
	},
};

static struct clk_pll clk_pll1 = {
	.id = '1',
	.hw = {
		.init = &clk_pll1_init,
	},
};

static struct clk_pll clk_pll2 = {
	.id = '2',
	.hw = {
		.init = &clk_pll2_init,
	},
};

static struct clk_pll clk_apll = {
	.id = 'a',
	.hw = {
		.init = &clk_apll_init,
	},
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		PLL re-calculate clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Clock rate.
*/
static unsigned long pll_clk_recalc_rate(
	struct clk_hw *hw,
	unsigned long parent_rate)
{
	struct clk_pll *clk = to_pllclk(hw);
	unsigned int BP = 0, NF = 1, NR = 1, NO = 1;
	
	switch(clk->id)
	{
		case '0':
		case '1':
			{
				volatile unsigned int* pll_reg = ( clk->id == '0' ) ? &SCUB_SPLL0_CFG : &SCUB_SPLL1_CFG;
				BP = *pll_reg & 0x20000 ;
				NF = ( *pll_reg & 0x7F) + 1 ;
				NR = (( *pll_reg & 0x1F00)>>8) + 1 ;
				NO = (( *pll_reg & 0xC000)>>14) + 1 ;
			}
			break;
		case '2':
			BP = SCUB_SPLL2_CFG & 0x100 ;
			NF = SCUB_SPLL2_SSC_CFG2 & 0x3F ;
			NR = ((SCUB_SPLL2_CFG & 0x1F000000)>>24) + 1 ;
			NO = 1<<((SCUB_SPLL2_CFG & 0xC0000)>>18) ;
		break;
	}
	
	return (BP) ? parent_rate : parent_rate * NF / ( NR * NO );
}

/**
* @brief 		PLL calculate most close clock rate form input rate.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Most close clock rate form input rate.
*/
static long pll_clk_round_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long *parent_rate)
{
	struct clk_pll *clk = to_pllclk(hw);
	unsigned long fin, NF, NR, NO;
	unsigned long io_gcd;
	unsigned long max_NF = (clk->id == '2') ? 6 : 7;

	/*
	 * fout = fin * NF / (NR * NO);
	 * set NO = 1, NR = fin/MHz, so fout = NF * MHz
	 */
	rate = rate / MHZ;
	fin = *parent_rate / MHZ;
	io_gcd = gcd( rate, fin );

	NF = rate / io_gcd;
	if (NF > BIT(max_NF))
		NF = BIT(max_NF);
	if (NF < 1)
		NF = 1;

	NR = fin / io_gcd;
	if (NR > BIT(5))
		NR = BIT(5);
	NO = 1;

	return fin * NF / (NR * NO);
}

/**
* @brief 		PLL set clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		0 on success, -EERROR otherwise.
*/
static int pll_clk_set_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long parent_rate)
{
	/* ----- Not ready ----- */

	return 0;
}

/**
* @brief 		APLL re-calculate clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Clock rate.
*/
static unsigned long apll_clk_recalc_rate(
	struct clk_hw *hw,
	unsigned long parent_rate)
{
	u8 apll_sel = SCUA_APLL_CFG & BIT(1);
	return (apll_sel) ? APLL_CLK2 : APLL_CLK1;
}

/**
* @brief 		APLL calculate most close clock rate form input rate.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Most close clock rate form input rate.
*/
static long apll_clk_round_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long *parent_rate)
{
	u32 diff_clk1 = abs( APLL_CLK1 - rate );
	u32 diff_clk2 = abs( APLL_CLK2 - rate );

	return ( diff_clk1>diff_clk2 ) ? APLL_CLK2 : APLL_CLK1;
}

/**
* @brief 		APLL set clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		0 on success, -EERROR otherwise.
*/
static int apll_clk_set_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long parent_rate)
{
	u32 diff_clk1 = abs( APLL_CLK1 - rate );
	u32 diff_clk2 = abs( APLL_CLK2 - rate );
	
	if( diff_clk1>diff_clk2 )
		SCUA_APLL_CFG &=  ~BIT(1);
	else
		SCUA_APLL_CFG |= BIT(1);
		
	return 0;
}

/**
* @brief 		APLL clock enable.
* @param 		hw[in]: Hardware-specific structure.
* @return		0.
*/
static int apll_clk_enable(
	struct clk_hw *hw)
{
	SCUA_A_PERI_CLKEN |= SCU_A_PERI_APLL;
	SCUA_APLL_CFG |= BIT(0);
	
	return 0;
}

/**
* @brief 		APLL clock disable.
* @param 		hw[in]: Hardware-specific structure.
* @return		None.
*/
static void apll_clk_disable(
	struct clk_hw *hw)
{
	SCUA_APLL_CFG &= ~BIT(0);
	SCUA_A_PERI_CLKEN &= ~SCU_A_PERI_APLL;
}

/**
* @brief 		APLL check clock enable.
* @param 		hw[in]: Hardware-specific structure.
* @return		1 for enable, 0 for disable.
*/
static int apll_clk_is_enabled(
	struct clk_hw *hw)
{
	return ( (SCUA_APLL_CFG & BIT(0)) && ( SCUA_A_PERI_CLKEN &  SCU_A_PERI_APLL ) ) ? 1 : 0;
}

/**
* @brief 		PLL initialization function.
* @param 		xtal[in]: xtal clock.
* @return		None.
*/
void __init gp_pll_init(
	unsigned long xtal)
{
	struct clk *clk;
	int ret;

	clk = clk_register_fixed_rate(NULL, "xtal", NULL, CLK_IS_ROOT, (xtal) ? xtal : XTAL_RATE);
	if (IS_ERR(clk))
		pr_err("XTAL not registered\n");
	ret = clk_register_clkdev(clk, clk->name, NULL );
	if (ret<0)
		pr_err("XTAL not registered clkdev\n");
	gp_clk_print( CLK_DEBUG, clk);
		
	clk = clk_register(NULL, &clk_pll0.hw);
	if (IS_ERR(clk))
		pr_err("PLL0 not registered\n");
	ret = clk_register_clkdev(clk, clk->name, NULL );
	if (ret<0)
		pr_err("PLL0 not registered clkdev\n");	
	gp_clk_print( CLK_DEBUG, clk);
		
	clk = clk_register(NULL, &clk_pll1.hw);
	if (IS_ERR(clk))
		pr_err("PLL1 not registered\n");
	ret = clk_register_clkdev(clk, clk->name, NULL );
	if (ret<0)
		pr_err("PLL1 not registered clkdev\n");	
	gp_clk_print( CLK_DEBUG, clk);
	
	clk = clk_register(NULL, &clk_pll2.hw);
	if (IS_ERR(clk))
		pr_err("PLL2 not registered\n");
	ret = clk_register_clkdev(clk, clk->name, NULL );
	if (ret<0)
		pr_err("PLL2 not registered clkdev\n");	
	gp_clk_print( CLK_DEBUG, clk);
		
	clk = clk_register(NULL, &clk_apll.hw);
	if (IS_ERR(clk))
		pr_err("APLL not registered\n");
	ret = clk_register_clkdev(clk, clk->name, NULL );
	if (ret<0)
		pr_err("APLL not registered clkdev\n");	
	gp_clk_print( CLK_DEBUG, clk);
	
}