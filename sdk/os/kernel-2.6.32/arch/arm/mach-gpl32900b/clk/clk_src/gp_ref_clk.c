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
 * @file	gp_ref_clk.c
 * @brief	Clock tree of reference clock (all fraction divider).
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

#define UPDATE_TIMEOUT	HZ/10 	/* 100 ms */

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

#define to_refclk(_hw) container_of(_hw, struct clk_ref, hw)

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

struct frac_div_reg{
	unsigned num:20;		/* numerator */
	unsigned rsv0:12;
	unsigned den:20;		/* denominator */
	unsigned rsv1:12;
	unsigned integer:10;	/* integer */
	unsigned rsv2:22;
	unsigned update:1;
	unsigned ssc:1;
	unsigned rsv3:14;
	unsigned ack:1;
};

struct clk_ref {
	struct clk_hw	hw;
	u8 				id;
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

static unsigned long ref_clk_recalc_rate( struct clk_hw *hw, unsigned long parent_rate );
static long ref_clk_round_rate( struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate );
static int ref_clk_set_rate( struct clk_hw *hw, unsigned long rate, unsigned long parent_rate );
static u8 ref_clk_get_parent( struct clk_hw *hw );
static int ref_clk_set_parent( struct clk_hw *hw, u8 index );

/**************************************************************************
 *                         G L O B A L    D A T A                         *
**************************************************************************/

static struct clk_ops ref_clk_ops = {
	.recalc_rate	= ref_clk_recalc_rate,
	.round_rate		= ref_clk_round_rate,
	.set_rate		= ref_clk_set_rate,
	.set_parent		= ref_clk_set_parent,
	.get_parent		= ref_clk_get_parent,
};

static const char *ref_clk_parents_1[] = {
	"xtal",
	"pll0",
	"pll1",
	"pll2",
};

static const char *ref_clk_parents_2[] = {
	"xtal",
	"pll0",
	"pll1",
	"apll",
};

static const char *clk_name[] = {
	"clk_ref_sys",
	"clk_ref_arm",
	"clk_ref_encoder",
	"clk_ref_decoder",
	"clk_ref_audio",
	NULL,
	"clk_ref_i2s",
	"clk_ref_cdsp",
	"clk_ref_disp",
	"clk_ref_mipi",
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		Calculate clock rate from 3 parameter (integer, denominator and numerator).
* @param 		parent_rate[in]: Parent clock .
* @param 		frac_int[in]: Integer.
* @param 		frac_den[in]: Denominator.
* @param 		frac_num[in]: Numerator.
* @return		Clock rate.
*/
static unsigned long ref_clk_calc_rate(
	unsigned long parent_rate,
	unsigned long frac_int,
	unsigned long frac_den,
	unsigned long frac_num)
{
	unsigned long clk_rate;
	
	parent_rate = parent_rate / 1000; /* scaler down to avoid overflow */
	
	if( ( frac_int < 1 ) || ( frac_den < frac_num ) )
	{
		printk("Fractional divider error: int %lu, den %lu, num %lu\n", frac_int, frac_den, frac_num );
		clk_rate = parent_rate;
		goto end;
	}
	/* ----- Fractional divider ----- */ 
	if( frac_den == 0 )
	{
		clk_rate = parent_rate / frac_int;
	}
	else
	{
		if( frac_int == 1)
			clk_rate = parent_rate * ( frac_den - frac_num ) / frac_den;
		else
			clk_rate = parent_rate * frac_den / ( frac_int * frac_den + frac_num ) ;
	}
end:
	clk_rate = clk_rate * 1000;	
	return clk_rate;
}

/**
* @brief 		Calculate 3 parameter (integer, denominator and numerator) by input and output clock.
* @param 		parent_rate[in]: Parent clock .
* @param 		rate[in]: Output clock.
* @param 		frac_int[out]: Integer.
* @param 		frac_den[out]: Denominator.
* @param 		frac_num[out]: Numerator.
* @return		None.
*/
static void ref_clk_calc_para(
	unsigned long parent_rate,
	unsigned long rate,
	unsigned long *frac_int,
	unsigned long *frac_den,
	unsigned long *frac_num)
{
	unsigned long retry_cnt = 0;
	
	*frac_int = parent_rate / rate;
	if( *frac_int == 0 || (*frac_int >0x3ff) )
	{
		printk("[%s]: parent clock %lu, target clock %lu, int %lu error\n", __FUNCTION__, parent_rate, rate, *frac_int );
		*frac_int = 1;
		return;
	}	

retry:	
	*frac_den = 0;
	*frac_num = 0;
	*frac_num = parent_rate % rate;

	if( *frac_num )
	{
		unsigned long io_gcd;
		*frac_den = ( *frac_int >= 2 ) ? rate : parent_rate ;
		io_gcd = gcd( *frac_den, *frac_num );
		if( io_gcd > 1 )
		{
			*frac_den = *frac_den / io_gcd;
			*frac_num = *frac_num / io_gcd;
		}
	}
	
	if( *frac_den > 0xfffff || *frac_num > 0xfffff )
	{
		/* ----- Scaler down and retry again ----- */
		if( retry_cnt <= 3 )
		{
			parent_rate = parent_rate / 10;
			rate = rate / 10;
			retry_cnt ++;
			goto retry;
		}
		
		printk("[%s] calculate parameter error: parent clock %lu, target clock %lu. Result integer %lu, denominator %lu, numerator %lu.\n", __FUNCTION__, parent_rate, rate, *frac_int, *frac_den, *frac_num );	
		
		*frac_int = 1;
		*frac_den = 0;
		*frac_num = 0;
	}	
}

/**
* @brief 		Re-calculate reference clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Clock rate.
*/
static unsigned long ref_clk_recalc_rate(
	struct clk_hw *hw,
	unsigned long parent_rate)
{
	struct clk_ref *clk = to_refclk(hw);
	volatile struct frac_div_reg *reg = (volatile struct frac_div_reg *) ( &SCUB_CLK0_CFG0 + ( clk->id *4 ) );

	return ref_clk_calc_rate( parent_rate, reg->integer, reg->den, reg->num );
}

/**
* @brief 		Reference clock calculate most close clock rate form input rate.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		Most close clock rate form input rate.
*/
static long ref_clk_round_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long *parent_rate)
{
	unsigned long integer, num, den;
	
	ref_clk_calc_para( *parent_rate, rate, &integer, &den, &num );
	return ref_clk_calc_rate( *parent_rate, integer, den, num );
}

/**
* @brief 		Set reference clock rate function.
* @param 		hw[in]: Hardware-specific structure.
* @param 		rate[in]: Input rate.
* @param 		parent_rate[in]: Parent clock rate.
* @return		0 on success, -EIO otherwise.
*/
static int ref_clk_set_rate(
	struct clk_hw *hw,
	unsigned long rate,
	unsigned long parent_rate)
{
	struct clk_ref *clk = to_refclk(hw);
	volatile struct frac_div_reg *reg = (volatile struct frac_div_reg *) ( &SCUB_CLK0_CFG0 + ( clk->id *4 ) );
	unsigned long integer, num, den;
	unsigned long old_integer, old_num, old_den;
	unsigned long start;
	int ret = 0;
	
	ref_clk_calc_para( parent_rate, rate, &integer, &den, &num );
	
	if( reg->integer == integer && reg->den == den && reg->num == num )
		return 0;
	
	old_integer = reg->integer;
	old_den = reg->den;
	old_num = reg->num;
	
	reg->integer = integer ;
	reg->den = den;
	reg->num = num;
		
	reg->update = 1;
	start = jiffies;
	/* ----- Busy waiting timeout, set_rate function can't sleep ----- */
	while( reg->ack != 1)
	{
		/* ----- Timeout ----- */
		if((jiffies-start)>=UPDATE_TIMEOUT)
		{
			printk("[%s][%s] calculate parameter error: parent clock %lu, target clock %lu.\n"
				" Result integer %lu, denominator %lu, numerator %lu.\n",
					hw->clk->name, __FUNCTION__, parent_rate, rate, integer, den, num );
			
			reg->integer = old_integer ;
			reg->den = old_den;
			reg->num = old_num;
			ret = -EIO ;
			break;
		}		
	}
	reg->update = 0;
	return ret;
}

/**
* @brief 		Get shift value form reference clock id.
* @param 		id[in]: Reference clock id.
* @return		Shift value.
*/
static u8 id2shift(
	u8 id)
{
	u8 shift;
	if( id < 4)
	{
		shift = (id * 2);
	}
	else if( id == 4 )
	{
		shift = 28;		
	}
	else if( id == 6 )
	{
		shift = 30;		
	}
	else
	{
		shift = ( ( id - 2 ) * 2);	
	}
	return shift;
}

/**
* @brief 		Get reference clock's parent index.
* @param 		hw[in]: Hardware-specific structure.
* @return		Parent index.
*/
static u8 ref_clk_get_parent(
	struct clk_hw *hw)
{
	struct clk_ref *clk = to_refclk(hw);	
	u8 shift = id2shift(clk->id);
	
	return ( SCUB_SPLL_SEL >> shift ) & 0x03 ;
}

/**
* @brief 		Set reference clock's parent.
* @param 		hw[in]: Hardware-specific structure.
* @param 		index[in]: Parent index.
* @return		0 on success, -EIO otherwise.
*/
static int ref_clk_set_parent(
	struct clk_hw *hw,
	u8 index)
{
	struct clk_ref *clk = to_refclk(hw);
	u32 shift = id2shift(clk->id);
	u32 mask = 0x03 << shift;
	u32 reg = SCUB_SPLL_SEL;
	
	if( index > 3 )
		return -EIO;
	
	reg &= ~mask;
	reg |= ( ((u32) index) << shift ) ;
	SCUB_SPLL_SEL = reg;
	
	return 0;
}

/**
* @brief 		Reference clock initialization function.
* @return		None.
*/
void __init gp_ref_clk_init(void)
{
	struct clk_ref *ref;
	struct clk *clk;
	struct clk_init_data init;	
	unsigned long i;
	int ret;
	
	for ( i=0 ; i<ARRAY_SIZE(clk_name); i++ )
	{
		if( clk_name[i] == NULL )
			continue;
		ref = kzalloc(sizeof(struct clk_ref), GFP_KERNEL);
		if (!ref) 
		{
			pr_err("%s, %s : could not allocate divider clk\n", __func__, (char*)clk_name );
			break;
		}
		/* ----- Set init data ----- */
		init.name = clk_name[i];
		init.ops = &ref_clk_ops;
		init.flags = CLK_IS_BASIC | CLK_GET_RATE_NOCACHE ;
		if( i == 4 || i == 6)
		{
			init.parent_names = ref_clk_parents_2;
			init.num_parents = ARRAY_SIZE(ref_clk_parents_2);
		}
		else
		{
			init.parent_names = ref_clk_parents_1;
			init.num_parents = ARRAY_SIZE(ref_clk_parents_1);
		}
		ref->id = i;
		ref->hw.init = &init;
		
		clk = clk_register( NULL, &ref->hw);
		if (!clk) {
			pr_err("%s not registered\n", (char*)clk_name[i]);
			break;
		}
		ret = clk_register_clkdev(clk, clk->name, NULL );
		if (ret<0)
		{
			pr_err("%s not registered clkdev\n", clk->name );
			break;
		}
		gp_clk_print( CLK_DEBUG, clk);
	}
	
}