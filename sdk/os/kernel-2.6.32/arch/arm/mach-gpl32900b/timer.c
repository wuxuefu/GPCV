/* arch/arm/mach-gpl32900/timer.c
**
** Copyright (C) 2007 Google, Inc.
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
*/

#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <mach/timer.h>
#include <asm/mach/time.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <mach/hal/hal_clock.h>
#include <mach/timex.h>
#include <mach/regs-wdt.h>
#include <mach/regs-interrupt.h>
#include <mach/regs-timer.h>
#include <linux/platform_device.h>
#include <mach/hal/regmap/reg_scu.h>
/*****************************************************
 
*****************************************************/
#define SCUB_TIMER0_CLKENABLE ((0x01<<9))
#define MSEC_10                 (1000 * 10)  // 10 ms
#define TIMER_USEC_SHIFT 16

static int g_apb_clk = 100*1000000; /*PLK = arm_apb*/
static int g_prescaler_usec = 99;
static int g_ticks_per_usec = 1;
static int g_time_interval = 10000;

static int TICKS2USECS(int x){
	return x / g_ticks_per_usec;
}


static irqreturn_t gpl32900b_timer_interrupt(int irq, void *dev_id)
{
#if 0
	if (TMISR_0 == 1) {
		timer_tick();
		TMISR_0 = 0;
		return IRQ_HANDLED;
	}
    return IRQ_NONE;
#else
    timer_tick();
    TMISR_0 = 0;
    return IRQ_HANDLED;
#endif
}


static struct irqaction gpl32900b_timer_irq = {
	.name		= "gpl32900 Timer",
	.flags		= IRQF_DISABLED| IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= gpl32900b_timer_interrupt,
//	.dev_id		= &gpl32900_clockevent,
};

static inline unsigned long timer_ticks_to_usec(unsigned long ticks)
{
	unsigned long res;

	res = TICKS2USECS(ticks);
	
	return res;
}

#if 0
#ifdef CONFIG_PM
static void gpl32900b_timer_suspend(void)
{
}

static void gpl32900b_timer_resume(void)
{
}
#else
#define gpl32900b_timer_suspend NULL
#define gpl32900b_timer_resume NULL
#endif
#endif

#if 0
static unsigned long gpl32900b_gettimeoffset (void)
{
	unsigned long t;
	unsigned long irqpend;
	unsigned long tval;
	/* work out how many ticks have gone since last timer interrupt */
	tval = g_time_interval - TMVLR_0;
	t   = tval;
    irqpend = VIC0_IRQSTATUS;
	
	if (irqpend & IRQ_TIMERINT0) {
		tval = g_time_interval - TMVLR_0;
		t = tval;
		if( t != 0 ) {
			t += g_time_interval;
		}
	}
	return timer_ticks_to_usec(t);
}
#else

static unsigned long gpl32900b_gettimeoffset(void)
{
    unsigned long value = g_time_interval - TMVLR_0;

	return ((tick_nsec / 1000) * value) / g_time_interval;
}

#endif

#ifndef CONFIG_FPGA_TEST

static unsigned long gp32900_timer_plk_rate_get( void )
{
	unsigned int arm_ref_sel = (SCUB_SPLL_SEL & 0xC)>>2;
	unsigned int arm_ratio = (SCUB_ARM_RATIO & 0x3F) + 1 ;
	unsigned int arm_apb_ratio = ( SCUB_ARM_APB_RATIO & 0x3F ) + 1 ;
	unsigned int arm_ahb_ration = ( SCUB_ARM_AHB_RATIO & 0x3F ) + 1 ;
	unsigned int frac_num = SCUB_CLK1_CFG0 & 0xfffff;
	unsigned int frac_den = SCUB_CLK1_CFG1 & 0xfffff;
	unsigned int frac_int = SCUB_CLK1_CFG2 & 0x3ff;
	unsigned int arm_clk = XTAL_RATE ;

	switch(arm_ref_sel)
	{
		case 1:
		case 2:
			{
				volatile unsigned int* pll_reg = ( arm_ref_sel==1) ? &SCUB_SPLL0_CFG : &SCUB_SPLL1_CFG;
				unsigned int BP = *pll_reg & 0x20000 ;
				unsigned int NF = ( *pll_reg & 0x7F) + 1 ;
				unsigned int NR = (( *pll_reg & 0x1F00)>>8) + 1 ;
				unsigned int NO = (( *pll_reg & 0xC000)>>14) + 1 ;
				arm_clk = (BP)? XTAL_RATE : (XTAL_RATE * NF / (NR*NO));
			}
			break;
		case 3:
			{
				unsigned int BP = SCUB_SPLL2_CFG & 0x100 ;
				unsigned int NF = SCUB_SPLL2_SSC_CFG2 & 0x3F ;
				unsigned int NR = ((SCUB_SPLL2_CFG & 0x1F000000)>>24) + 1 ;
				unsigned int NO = 1<<((SCUB_SPLL2_CFG & 0xC0000)>>18) ;	
				arm_clk = (BP)? XTAL_RATE : ( XTAL_RATE * NF / (NR*NO));
			}
			break;
	}
	arm_clk = arm_clk / 1000;
	if( ( frac_int < 1 ) || ( frac_den < frac_num ) )
	{
		printk("Fractional divider error: int %d, den %d, num %d\n", frac_int, frac_den, frac_num );
		goto end;
	}
	/* ----- Fractional divider ----- */ 
	if( frac_den == 0 )
	{
		arm_clk = arm_clk/frac_int;
	}
	else
	{
		if( frac_int == 1)
			arm_clk = arm_clk * ( frac_den - frac_num ) / frac_den;
		else
			arm_clk = arm_clk * frac_den / ( frac_int * frac_den + frac_num ) ;
	}
end:	
	return arm_clk/arm_ratio/arm_ahb_ration/arm_apb_ratio * 1000 ;
}
#endif

static void gpl32900b_timer_setup (void)
{
#if 0
#ifdef CONFIG_FPGA_TEST
	g_apb_clk = 30000000;
#else
	g_apb_clk = gp32900_timer_plk_rate_get();
#endif
	
	g_prescaler_usec = ((g_apb_clk / 1000000) - 1);
	g_ticks_per_usec = (g_apb_clk / (g_prescaler_usec+1) / 1000000);
	g_time_interval =  (g_ticks_per_usec * MSEC_10); // 10ms

	printk("GPL32900b system timer init, PCLK(arm_apb)[%d]Prescaler[%d]LDR[%d]\n", g_apb_clk, g_prescaler_usec, g_time_interval);
	SCUB_B_PERI_CLKEN |= SCUB_TIMER0_CLKENABLE;
	TMCTR_0 = 0; 
	TMPSR_0 = g_prescaler_usec; // 1MHz tick
	TMLDR_0 = g_time_interval;  // 10000 ticks = 10 ms
	TMCTR_0 = TMR_ENABLE | TMR_IE_ENABLE | TMR_OM_PULSE | TMR_UD_DOWN | TMR_M_PERIOD_TIMER;
#else
	g_prescaler_usec = 0;
	g_ticks_per_usec = 1;
	g_time_interval = 0x41EA;
	SCUB_B_PERI_CLKEN |= SCUB_TIMER0_CLKENABLE;
	TMCTR_0 = 0; 
	TMPSR_0 = g_prescaler_usec; // 1MHz tick
	TMLDR_0 = g_time_interval;  // 10000 ticks = 10 ms
	TMCTR_0 = 0x8000 | TMR_ENABLE | TMR_IE_ENABLE | TMR_OM_PULSE | TMR_UD_DOWN | TMR_M_PERIOD_TIMER;
#endif
}


static void __init gpl32900b_timer_init(void)
{
	int res;
	
	gpl32900b_timer_setup();
	res = setup_irq(IRQ_TIMERINT0, &gpl32900b_timer_irq);
	if (res)
		printk(KERN_ERR "gpl32900b_timer_init: setup_irq failed\n");
}

struct sys_timer gpl32900b_timer = {
	.init		= gpl32900b_timer_init,
	.offset		= gpl32900b_gettimeoffset,
	.resume		= gpl32900b_timer_setup
};

