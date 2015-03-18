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
 * @file    device.c
 * @brief   Implement of power manager
 * @author  Roger Hsu
 */
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/diag.h>
#include <mach/pm.h>
#include <mach/hardware.h>
#include <mach/regs-wdt.h>
#include <linux/usb/android_composite.h>
#include <mach/gp_sram.h>
#include <mach/gp_board.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/gp_fiq.h>
#include <mach/hal/hal_clock.h>
#include <mach/hal/hal_usb.h>
#include <mach/gp_usb.h>
#include <mach/clk/gp_clk_core.h>

struct spmpehci_platform_data {
	int (*init)(struct device *);
	void (*exit)(struct device *);
};

struct spmpohci_platform_data {
	int (*init)(struct device *);
	void (*exit)(struct device *);
};

struct spmpmci_platform_data {
	int(*cd_setup)(void *mmc_host, int on);
	int(*card_inserted)(void *mmc_host);
	int(*card_readonly)(void *mmc_host);
	void(*set_power)(void *mmc_host, int state);
};
/*****************************************************************
 * USB HOST OHCI Info
*****************************************************************/
static int spmp_usb_host_phy( unsigned int host_id, int enable )
{
    struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
    static int g_usb_host_cnt[2] = {0};
    //char *host_name = (host_id)? "USB_HOST1" : "USB_HOST0";
    int phy_config;
    
    if(host_id == 0 )
    {
    	phy_config = (pConfig->phy0_func_en_get() == PHY0_HOST)? 1 : 0;
    }
    else
    {
    	phy_config = (pConfig->phy1_func_sel_get() == PHY0_HOST)? 1 : 0;
    }
    
    if(enable)
    {
        if( g_usb_host_cnt[host_id] <= 0 )
        {
			printk("host id %d enable\n",host_id );
			g_usb_host_cnt[host_id] = 0;
			gpHalUsbHostEn(host_id, 1);
			pConfig->set_power(1);
			gpHalScuUsbPhyClkEnable(1);	
			gpHalUsbPhyPowerControlSet( host_id ^0x01, 0x0 );
			if(phy_config)
				gpHalUsbPhyFuncSet( host_id ^0x01, 1 );
			msleep(5);
		}
		g_usb_host_cnt[host_id] ++;
    }
    else
    {
        g_usb_host_cnt[host_id] --;
        if(g_usb_host_cnt[host_id] <= 0)
        {
        	g_usb_host_cnt[host_id] = 0;
			if(phy_config)
		    	gpHalUsbPhyFuncSet( host_id ^0x01, 0 );
        	gpHalUsbPhyPowerControlSet( host_id ^0x01, 0x1);
			pConfig->set_power(0);
			gpHalUsbHostEn(host_id, 0);
		}
	}
    
#if 0 
    int phy0_config = pConfig->phy0_func_en_get();
	/*0: phy0 host, 1: phy0 disable*/
	int phy1_config = pConfig->phy1_func_sel_get();
	/*0: phy1 host, 1: phy1 slave, 2: phy1 host/slave, 3 phy1 disable*/
    int host_phy = -1;
    
    if( phy0_config == PHY0_HOST ) 
    {
        host_phy = 0;
    }
	else if( phy1_config == PHY1_HOST || phy1_config == PHY1_HOST_SLAVE ) 
    {
        host_phy = 1;
	}
    else
        return -1;

    if(enable)
    {
        if( g_usb_host_cnt <= 0 )
        {
            printk("host id %d enable, host phy = %d\n",host_id,host_phy );
            g_usb_host_cnt = 0;
		    gpHalUsbHostEn(host_id, 1);
		    pConfig->set_power(1);
		    gpHalScuUsbPhyClkEnable(1);	
		    gpHalUsbPhyPowerControlSet(host_phy, 0x0);
		    msleep(5);
		}
		g_usb_host_cnt ++;
    }
    else
    {
        g_usb_host_cnt --;
        if(g_usb_host_cnt <= 0)
        {
        	g_usb_host_cnt = 0;
        	gpHalUsbPhyPowerControlSet(host_phy, 0x1);
			pConfig->set_power(0);
			gpHalUsbHostEn(host_id, 0);
		}
	}
#endif
    return 0;
}

static int spmp_ohci_init(struct device *dev)
{
	struct platform_device *ohci = container_of( dev, struct platform_device, dev );
	return spmp_usb_host_phy( ohci->id, 1 );
}

static void spmp_ohci_exit(struct device *dev)
{
	struct platform_device *ohci = container_of( dev, struct platform_device, dev );
	spmp_usb_host_phy( ohci->id, 0 );
}

static struct spmpohci_platform_data spmp_ohci_platform_data = {
    .init = spmp_ohci_init,
    .exit = spmp_ohci_exit,
};

static struct resource spmp_resource_ohci0[] = {
	[0] = {
		.start  = 0x93004080,
		.end    = 0x930040FF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_USB_OHCI0,
		.end    = IRQ_USB_OHCI0,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource spmp_resource_ohci1[] = {
	[0] = {
		.start  = 0x93005080,
		.end    = 0x930050FF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_USB_OHCI1,
		.end    = IRQ_USB_OHCI1,
		.flags  = IORESOURCE_IRQ,
	},
};

static u64 spmp_ohci_dma_mask = DMA_BIT_MASK(32);

struct platform_device spmp_device_ohci0 = {
	.name		= "spmp-ohci",
	.id		= 0,
	.dev		= {
		.dma_mask = &spmp_ohci_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data		= &spmp_ohci_platform_data,
	},
	.num_resources  = ARRAY_SIZE(spmp_resource_ohci0),
	.resource       = spmp_resource_ohci0,
};

struct platform_device spmp_device_ohci1 = {
	.name		= "spmp-ohci",
	.id		= 1,
	.dev		= {
		.dma_mask = &spmp_ohci_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data		= &spmp_ohci_platform_data,
	},
	.num_resources  = ARRAY_SIZE(spmp_resource_ohci1),
	.resource       = spmp_resource_ohci1,
};

static struct platform_device *gp_ohci[] __initdata = {
	&spmp_device_ohci0,
	&spmp_device_ohci1,
};


static int __init spmp_regdev_ohci(void)
{
    int ret;
	ret = platform_add_devices(gp_ohci, ARRAY_SIZE(gp_ohci));
	if (ret)
		printk("GP OHCI: unable to register device: %d\n", ret);
	return ret;
}

/*****************************************************************
 * USB HOST EHCI Info
*****************************************************************/
static int spmp_ehci_init(struct device *dev)
{
	struct platform_device *ehci = container_of( dev, struct platform_device, dev );
	return spmp_usb_host_phy( ehci->id, 1 );
}

static void spmp_ehci_exit(struct device *dev)
{
	struct platform_device *ehci = container_of( dev, struct platform_device, dev );
	spmp_usb_host_phy( ehci->id, 0 );
}

static struct spmpehci_platform_data spmp_ehci_platform_data = {
    .init = spmp_ehci_init,
    .exit = spmp_ehci_exit,
};

static struct resource spmp_resource_ehci0[] = {
	[0] = {
		.start  = 0x93004100,
		.end    = 0x930041FF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_USB_EHCI0,
		.end    = IRQ_USB_EHCI0,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource spmp_resource_ehci1[] = {
	[0] = {
		.start  = 0x93005100,
		.end    = 0x930051FF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_USB_EHCI1,
		.end    = IRQ_USB_EHCI1,
		.flags  = IORESOURCE_IRQ,
	},
};

static u64 spmp_ehci_dma_mask = DMA_BIT_MASK(32);

struct platform_device spmp_device_ehci0 = {
	.name		= "spmp-ehci",
	.id		= 0,
	.dev		= {
		.dma_mask = &spmp_ehci_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data		= &spmp_ehci_platform_data,
	},
	.num_resources  = ARRAY_SIZE(spmp_resource_ehci0),
	.resource       = spmp_resource_ehci0,
};

struct platform_device spmp_device_ehci1 = {
	.name		= "spmp-ehci",
	.id		= 1,
	.dev		= {
		.dma_mask = &spmp_ehci_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data		= &spmp_ehci_platform_data,
	},
	.num_resources  = ARRAY_SIZE(spmp_resource_ehci1),
	.resource       = spmp_resource_ehci1,
};

static struct platform_device *gp_ehci[] __initdata = {
	&spmp_device_ehci0,
	&spmp_device_ehci1,
};

static int __init spmp_regdev_ehci(void)
{
	int ret;
	
	ret = platform_add_devices(gp_ehci, ARRAY_SIZE(gp_ehci));;
	
	if (ret)
		printk("GP EHCI: unable to register device: %d\n", ret);
	return ret;
}

static struct resource spmp_resource_udc[] = {
	[0] = {
		.start  = 0x93006000,
		.end   = 0x93006FFF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_USB_DEV,
		.end    = IRQ_USB_DEV,
		.flags  = IORESOURCE_IRQ,
	},
};

#define GP329XXB_UDC_NAME	"gp329xxb_udc"
#define SPMP_UDC_NAME		"spmp-udc"

struct platform_device spmp_device_udc = {
	.name		= SPMP_UDC_NAME,
	.id		= -1,
	.dev		= {
		.dma_mask = &spmp_ehci_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources  = ARRAY_SIZE(spmp_resource_udc),
	.resource       = spmp_resource_udc,
};

static int __init spmp_regdev_udc(void)
{
    int ret;
	ret = platform_device_register(&spmp_device_udc);
	if (ret)
		dev_err(&(spmp_device_udc.dev), "unable to register device: %d\n", ret);
	return ret;
}

/*****************************************************************
 * I2C / I2C Device Info
*****************************************************************/

static struct platform_device gpl32900b_i2c_device = {
	.name	= "gp32900b-i2c",
	.id	= -1,
	.dev	= {
	//	.release = gp_ti2c_device_release,
	}
};

static int __init spmp_regdev_i2c(void)
{
    int ret;
	ret = platform_device_register(&gpl32900b_i2c_device);
	if (ret)
		dev_err(&(gpl32900b_i2c_device.dev), "unable to register device: %d\n", ret);
	return ret;
}


/*****************************************************************
 * SD / SDIO Device Info
*****************************************************************/

#if 0
static struct sd_data_s spmpmci0_platdata = {
	.info = {
	         .device_id = 0,
             .p_addr = 0,
	         .v_addr = 0,
	         .dma_chan = -1,
	         .is_irq = 1,
	         .is_dmairq = 1,
	         .is_detectirq = 0,
	         .is_readonly = 0,
	         .clk_rate = 0,
	         .clk_div = 2,
	         .max_clkdiv = 256,
	         .real_rate = 0,
	         .dma_cb = NULL,
	         .detect_chan = -1,
	         .detect_delay = 20,
	         .detect_cb = NULL,
	},
	.ops = &spmpmci_ops0,
};

static struct resource spmp_resources_mci0[] = {
	[0] = {
		.start	= 0x92B0B000,
		.end	= 0x92B0CFFF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SD0,
		.end	= IRQ_SD0,
		.flags	= IORESOURCE_IRQ,
	},
};

#if 0
static struct resource spmp_resources_mci1[] = {
	[0] = {
		.start	= 0x92B0C000,
		.end	= 0x92B0CFFF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SD1,
		.end	= IRQ_SD1,
		.flags	= IORESOURCE_IRQ,
	},
};
#endif

static u64 spmp_mci0_dma_mask = DMA_BIT_MASK(32);
struct platform_device spmp_mci0_device = {
	.name	= "spmp-mci",
	.id		= 0,
	.dev	= {
		.platform_data		= &spmpmci0_platdata,
		.dma_mask = &spmp_mci0_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(spmp_resources_mci0),
	.resource	= spmp_resources_mci0,
};

#if 0
static u64 spmp_mci1_dma_mask = DMA_BIT_MASK(32);
struct platform_device spmp_mci1_device = {
	.name	= "spmp-mci",
	.id		= 1,
	.dev	= {
		.platform_data		= &spmpmci_platdata,
		.dma_mask = &spmp_mci1_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(spmp_resources_mci1),
	.resource	= spmp_resources_mci1,
};
#endif

static int __init spmp_regdev_sd0(void)
{
    int ret;
	ret = platform_device_register(&spmp_mci0_device);
	if (ret)
		dev_err(&(spmp_mci0_device.dev), "unable to register device: %d\n", ret);
	return ret;
}

#if 0
static int __init spmp_regdev_sd1(void)
{
    int ret;
	ret = platform_device_register(&spmp_mci1_device);
	if (ret)
		dev_err(&(spmp_mci1_device.dev), "unable to register device: %d\n", ret);
	return ret;
}
#endif
#endif
/*****************************************************************
 * APBDMAA APBDMAC Device Info
*****************************************************************/

/*****************************************************************
 * RTC Device Info
*****************************************************************/
#if 0
static struct resource spmp_rtc_resources[] = {
	[0] = {
		.start  = 0x9000B000,
		.end	= 0x9000B000 + 0xFFF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_REALTIME_CLOCK,
		.end    = IRQ_REALTIME_CLOCK,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device spmp_device_rtc = {
	.name		= "gp-rtc",
	.id		= -1,
	.num_resources  = ARRAY_SIZE(spmp_rtc_resources),
	.resource       = spmp_rtc_resources,
};

static int __init spmp_regdev_rtc(void)
{
    int ret;
	ret = platform_device_register(&spmp_device_rtc);
	if (ret)
		dev_err(&(spmp_device_rtc.dev), "unable to register device: %d\n", ret);
	return ret;
}
#endif

/*****************************************************************
 * SARADC Controller Device Info
*****************************************************************/
static struct resource spmp_saacc_resources[] = {
	[0] = {
		.start  = 0x9301F000,
		.end	= 0x9301F000 + 0xFFF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_SAACC,
		.end    = IRQ_SAACC,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device spmp_device_saacc = {
	.name		= "spmp-saacc",
	.id		= -1,
	.num_resources  = ARRAY_SIZE(spmp_saacc_resources),
	.resource       = spmp_saacc_resources,
};

static int __init spmp_regdev_saacc(void)
{
    int ret;
	ret = platform_device_register(&spmp_device_saacc);
	if (ret)
		dev_err(&(spmp_device_saacc.dev), "unable to register device: %d\n", ret);
	return ret;
}

/*****************************************************************
 * I2C Info
*****************************************************************/
struct s3c2410_platform_i2c {
	int		bus_num;	/* bus number to use */
	unsigned int	flags;
	unsigned int	slave_addr;	/* slave address for controller */
	unsigned long	bus_freq;	/* standard bus frequency */
	unsigned long	max_freq;	/* max frequency for the bus */
	unsigned long	min_freq;	/* min frequency for the bus */
	unsigned int	sda_delay;	/* pclks (s3c2440 only) */

	void	(*cfg_gpio)(struct platform_device *dev);
};

static struct s3c2410_platform_i2c  i2cB_info = {
	.flags		= 0,
	.slave_addr	= 0x60,
	.bus_freq	= 100*1000,
	.max_freq	= 130*1000,
};

static struct resource spmp_i2cB_resource[] = {
	[0] = {
		.start = 0x92B03000,
		.end   = 0x92B03000 + 0xFFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_I2C_C,
		.end   = IRQ_I2C_C,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device spmp_i2cB_device = {
	.name	= "spmp-i2cB",
	.id		= -1,
	.dev	= {
		.platform_data	= &i2cB_info,
	},
	.num_resources  = ARRAY_SIZE(spmp_i2cB_resource),
	.resource       = spmp_i2cB_resource,
};

/*****************************************************************
 * DISPLAY Module
*****************************************************************/
static u64 spmp_display_dma_mask = DMA_BIT_MASK(32);
#ifndef FB_USE_CHUNKMEM
static struct resource spmp_resource_display[] = {
	[0] = {
		.start  = FRAMEBUF_BASE,
		.end    = FRAMEBUF_BASE+FRAMEBUF_SIZE -1,
		.flags  = IORESOURCE_MEM,
	},
};
#endif

struct platform_device spmp_device_display = {
	.name		= "spmp-display",
	.id		= -1,
	.dev		= {
		.dma_mask = &spmp_display_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
#ifndef FB_USE_CHUNKMEM
	.num_resources  = ARRAY_SIZE(spmp_resource_display),
	.resource       = spmp_resource_display,
#endif
};

int __init spmp_regdev_display(void)
{
    int ret;
	ret = platform_device_register(&spmp_device_display);
	if (ret)
		dev_err(&(spmp_device_display.dev), "unable to register device: %d\n", ret);
	return ret;
}


/*****************************************************************
 * PM Module
*****************************************************************/
#ifdef CONFIG_PM

#if 0
/* configuration for the IRQ mask over sleep */
extern unsigned long gp_irqwake_int1mask;
extern unsigned long gp_irqwake_int2mask;

#define SAVE(x)		sleep_save[SLEEP_SAVE_##x] = x
#define RESTORE(x)	x = sleep_save[SLEEP_SAVE_##x]
#define MMP_IRQ_VIC0_BASE			IO0_ADDRESS(0x10000)
#define MMP_IRQ_VIC1_BASE			IO0_ADDRESS(0x20000)
#define MMP_IRQENABLE				0x10
#define MMP_IRQENABLECLEAR		0x14


#define CYG_DEVICE_IRQ0_EnableSet \
    (*(volatile unsigned int *) (MMP_IRQ_VIC0_BASE + MMP_IRQENABLE))
    // Enable (1's only), write only
#define CYG_DEVICE_IRQ0_EnableClear \
    (*(volatile unsigned int *) (MMP_IRQ_VIC0_BASE + MMP_IRQENABLECLEAR))
    // Disable (1's only), write only

#define CYG_DEVICE_IRQ1_EnableSet \
    (*(volatile unsigned int *) (MMP_IRQ_VIC1_BASE + MMP_IRQENABLE))
    // Enable (1's only), write only
#define CYG_DEVICE_IRQ1_EnableClear \
    (*(volatile unsigned int *) (MMP_IRQ_VIC1_BASE + MMP_IRQENABLECLEAR))
    // Disable (1's only), write only
/*
 * List of global PXA peripheral registers to preserve.
 * More ones like CP and general purpose register values are preserved
 * with the stack pointer in sleep.S.
 */
enum {
	SLEEP_SAVE_SCUA_A_PERI_CLKEN,
	SLEEP_SAVE_SCUB_B_PERI_CLKEN,
	SLEEP_SAVE_SCUC_C_PERI_CLKEN,
	SLEEP_SAVE_COUNT
};
//static void (*spmp_sram_suspend)(void) = NULL;
extern void (*gp_sram_suspend)(void);
static void gp_cpu_pm_suspend(void)
{
    extern void  gp_cpu_suspend(void);
    unsigned long gp_irqwakeup_int1mask;
    unsigned long gp_irqwakeup_int2mask;
	local_irq_disable();
	local_fiq_disable();
      gp_irqwakeup_int1mask = CYG_DEVICE_IRQ0_EnableSet;
      gp_irqwakeup_int2mask = CYG_DEVICE_IRQ1_EnableSet;
      CYG_DEVICE_IRQ0_EnableClear = CYG_DEVICE_IRQ0_EnableSet & gp_irqwake_int1mask;
      CYG_DEVICE_IRQ1_EnableClear = CYG_DEVICE_IRQ1_EnableSet & gp_irqwake_int2mask;
	flush_cache_all();
	gp_sram_suspend();
      CYG_DEVICE_IRQ0_EnableSet = gp_irqwakeup_int1mask;
      CYG_DEVICE_IRQ1_EnableSet = gp_irqwakeup_int2mask;
	local_irq_enable();
	local_fiq_enable();
	printk("suspend = %p\n",gp_sram_suspend);
}

static void gp_cpu_pm_save(unsigned long *sleep_save)
{
	SAVE(SCUA_A_PERI_CLKEN);
	SAVE(SCUB_B_PERI_CLKEN);
	SAVE(SCUC_C_PERI_CLKEN);
}

static void gp_cpu_pm_restore(unsigned long *sleep_save)
{
	RESTORE(SCUA_A_PERI_CLKEN);
	RESTORE(SCUB_B_PERI_CLKEN);
	RESTORE(SCUC_C_PERI_CLKEN);
}

static void gp_cpu_pm_enter(suspend_state_t state)
{
	switch (state) {
	case PM_SUSPEND_MEM:
	    gp_cpu_pm_suspend();
		printk("exit suspend \n");
		break;
	}
}

static int gp_cpu_pm_prepare(void)
{
	return 0;
}

static void gp_cpu_pm_finish(void)
{
}

static struct gp_cpu_pm_fns gpl32900_cpu_pm_fns = {
	.save_count	= SLEEP_SAVE_COUNT,
	.valid		= suspend_valid_only_mem,
	.save		= gp_cpu_pm_save,
	.restore	= gp_cpu_pm_restore,
	.enter		= gp_cpu_pm_enter,
	.prepare	= gp_cpu_pm_prepare,
	.finish		= gp_cpu_pm_finish,
};
#else
extern void (*gp_sram_suspend)(void);
#endif

#if 0
static void __init gp_init_pm(void)
{
	gp_sram_suspend = gp_sram_push(gp_cpu_suspend,
				   gp_cpu_suspend_sz);
	gp_cpu_pm_fns = &gpl32900_cpu_pm_fns;
}
#endif

#else
static inline void gp_init_pm(void) {}
#endif


extern void gp_init_pm(void);
/*****************************************************************
 * register mapping
*****************************************************************/
void __init gp_regdev_reregister_map(void)
{

	if (check_mem_region(LOGI_ADDR_REG, LOGI_ADDR_REG_RANGE)) {
	    printk("register_address: memory already in use\n");
    	return;
	}
	/* request all register address for hal asscess */
	if (!request_mem_region(LOGI_ADDR_REG, LOGI_ADDR_REG_RANGE, "register_address")) {
	    printk("!!!!!!!!!!!!!!!!!!request_mem_region: fail\n");
    	return;
	}

    //release_mem_region(mem_addr, mem_size);

}

/*****************************************************************
 *
*****************************************************************/
void __init gpl32900b_devinit(void)
{
	printk(KERN_INFO "[%s][%d] run\n",__FUNCTION__, __LINE__);
#ifdef CONFIG_PM
	gp_init_pm();
#endif
#ifdef CONFIG_FB_SPMP
	spmp_regdev_display();
#endif
	gp_fiq_init();
	gp_regdev_reregister_map();
	
	spmp_regdev_ehci();
	spmp_regdev_ohci();
	spmp_regdev_udc();
	spmp_regdev_saacc();
	spmp_regdev_i2c();
	
//   i2c_register_board_info(0, vo9655_i2c_devices, ARRAY_SIZE(vo9655_i2c_devices));
//   spmp_regdev_sd1();
}

void gpl32900b_power_off(void)
{
	gp_board_t *config;

	printk(KERN_INFO "powering system down...\n");

	config = gp_board_get_config("board", gp_board_t);
	if (config != NULL && config->power_off != NULL) {
		config->power_off();
	}

	SCUB_PWRC_CFG = 0;
}

void gpl32900_power_on(void)
{
	printk(KERN_INFO "powering system down...\n");

	SCUB_PWRC_CFG = 0;
}

#define SCUB_TIMER1_CLKENABLE ((0x01<<10))

#define MSEC_10                 (1000 * 10)  // 10 ms

#define SYSTEM_CLOCK            (27*1000000)   // Watchdog Clk = 27 Mhz
#define PRESCALER_USEC_1      	((SYSTEM_CLOCK / 1000000) - 1)
#define TICKS_PER_USEC          (SYSTEM_CLOCK / (PRESCALER_USEC_1+1) / 1000000)
#define TIMER_INTERVAL          ((TICKS_PER_USEC * MSEC_10) -1) //10ms

#define TICKS2USECS(x)          ( (x) / TICKS_PER_USEC)

#define TIMER_USEC_SHIFT 16
#define RESETWDT_BASE			(WDT_BASE + 0)

/* Watchdog Timer 0 register */
#define WDTCTR_R   (*(volatile unsigned int*)(RESETWDT_BASE+0x00))  //control Register
#define WDTPSR_R   (*(volatile unsigned int*)(RESETWDT_BASE+0x04))  //pre-scare Register
#define WDTLDR_R   (*(volatile unsigned int*)(RESETWDT_BASE+0x08))  //load value Register
#define WDTVLR_R   (*(volatile unsigned int*)(RESETWDT_BASE+0x0c))  //current counter value Register
#define WDTCMP_R   (*(volatile unsigned int*)(RESETWDT_BASE+0x10))  //compare Register
void gpl32900_power_reset(void)
{
	gp_board_t *config;
	printk(KERN_INFO "powering system reset...\n");

	config = gp_board_get_config("board", gp_board_t);
	if (config != NULL && config->power_reset != NULL) {
		config->power_reset();
	}
	else {
       SCUB_B_PERI_CLKEN |= SCUB_TIMER1_CLKENABLE;
		WDTCTR_R  = WDT_DISABLE;
		WDTPSR_R  = PRESCALER_USEC_1;
		WDTLDR_R  = TIMER_INTERVAL;
		WDTCMP_R  = 0;
		WDTCTR_R  = WDT_RE_ENABLE | WDT_IE_ENABLE | WDT_PWMON_WDT | WDT_ENABLE ;
	}
}
