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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30077, Taiwan, R.O.C                                     *
 *                                                                        *
 **************************************************************************/
/**
 * @file    warp_snapshot.c
 * @brief   GPlus Speedy Boot header definition
 * @author  Roger hsu
 * @date    2012-10-24
 */

#include <mach/kernel.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/warp_param.h>
#include <asm/cacheflush.h>
#include <mach/gp_cpx.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/hal/regmap/reg_timer.h>
#include <mach/hal/regmap/reg_gpio.h>
#include <mach/regs-interrupt.h>
#include <mach/hardware.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_storage_api.h>
#include <mach/cdev.h>

//extern SINT32 nand_flush_user_partition(UINT8 which_kind, UINT16 which_partition);
extern int getBootDevID (void);
extern spmp8050_power_off(void);

static struct timeval tv;

static struct savearea
{
    unsigned long bootflag_area;
    unsigned long bootflag_size;
    unsigned long snapshot_area;
    unsigned long snapshot_size;
} savearea[WARP_SNAPSHOT_NUM];
static u8 savearea_num_check[WARP_SNAPSHOT_NUM];

static void *ka=0;           /* kernel_addr */
static unsigned int pa=0;    /* phy_addr*/
static unsigned char *sram_data_ptr = NULL;
static unsigned char *va_sram;

#ifdef CONFIG_PM_WARP_DEBUG
//#define GP_FUNCTION_TIME_CHECK
#undef GP_FUNCTION_TIME_CHECK
#else
#undef GP_FUNCTION_TIME_CHECK
#endif /* CONFIG_PM_WARP_DEBUG */

extern int gp_fastbot_binary_read(int *buf, int size_sector);
extern int gp_fastbot_parameter_get(int index, int *boot_offset, int *image_offset, int *boot_size, int *image_size);
//extern gpGenStorageApiAddr_t gpGenStorageApi;
extern void fb_get_buff_info(unsigned int *addr, unsigned int *size);

#ifdef CONFIG_PM_WARP_DEBUG
#include <linux/serial_reg.h>
#include <mach/hal/hal_uart.h> 
#define UART_ID		0
#define BOTH_EMPTY	(UART_LSR_TEMT | UART_LSR_THRE)

static void warp_putchar(char c)
{
	unsigned long status;

	if (c == '\n')
	 	warp_putc('\r');
   
	/* Wait up to 2ms for the character(s)  to be sent. */
	do {
		status = pUartPorts[UART_ID]->regLSR;
	} while ((status & BOTH_EMPTY) != BOTH_EMPTY);

	pUartPorts[UART_ID]->regTHR = c;
}

static void dbg_regdump(void)
{
#if 0
	int i;
	scuaReg_t *pScuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG;
	scubReg_t *pScubReg = (scubReg_t *)LOGI_ADDR_SCU_B_REG;
	scucReg_t *pScucReg = (scucReg_t *)LOGI_ADDR_SCU_C_REG;

	warp_printf("+++++ IRQ +++++\n");
	warp_printf("    VIC0 IRQ EnableSet=0x%08x\n", CYG_DEVICE_IRQ0_EnableSet);
	warp_printf("    VIC0 IRQ EnableClear=0x%08x\n", CYG_DEVICE_IRQ0_EnableClear);
	warp_printf("    VIC1 IRQ EnableSet=0x%08x\n", CYG_DEVICE_IRQ1_EnableSet);
	warp_printf("    VIC1 IRQ EnableClear=0x%08x\n", CYG_DEVICE_IRQ1_EnableClear);
	warp_printf("+++++ TIMER +++++\n");
	for (i = 0; i < 4; i++) {
		timerReg_t *ptimerReg =
			(timerReg_t *)(LOGI_ADDR_TIMER_REG + (i * LOGI_TIMER_OFFSET));
		warp_printf("    TIMER%d tmCtr=0x%08x\n", i, ptimerReg->tmCtr);
		warp_printf("    TIMER%d tmPsr=0x%08x\n", i, ptimerReg->tmPsr);
		warp_printf("    TIMER%d tmLdrVlr=0x%08x\n", i, ptimerReg->tmLdrVlr);
		warp_printf("    TIMER%d tmIsr=0x%08x\n", i, ptimerReg->tmIsr);
		warp_printf("    TIMER%d tmCmp=0x%08x\n", i, ptimerReg->tmCmp);
	}

	warp_printf("+++++ CLOCK +++++\n");
	warp_printf("    SCUA PeriClockEnable=0x%08x\n", pScuaReg->scuaPeriClkEn);
	warp_printf("    SCUB PeriClockEnable=0x%08x\n", pScubReg->scubPeriClkEn);
	warp_printf("    SCUC PeriClockEnable=0x%08x\n", pScucReg->scucPeriClkEn);
	warp_printf("    SCUA PeriClockEnable2=0x%08x\n", pScuaReg->scuaPeriClkEn2);
	warp_printf("    SCUB SysCntEn=0x%08x\n", pScubReg->scubSysCntEn);
	warp_printf("    SCUC CevaCntEn=0x%08x\n", pScucReg->scucCevaCntEn);
	warp_printf("+++++ GPIO +++++\n");
	warp_printf("    SCUB_GPIO0_IE=0x%08x\n", SCUB_GPIO0_IE);
	warp_printf("    SCUB_GPIO0_DS=0x%08x\n", SCUB_GPIO0_DS);
	warp_printf("    SCUB_GPIO0_PE=0x%08x\n", SCUB_GPIO0_PE);
	warp_printf("    SCUB_GPIO0_PS=0x%08x\n", SCUB_GPIO0_PS);
	warp_printf("    SCUB_GPIO1_IE=0x%08x\n", SCUB_GPIO1_IE);
	warp_printf("    SCUB_GPIO1_DS=0x%08x\n", SCUB_GPIO1_DS);
	warp_printf("    SCUB_GPIO1_PE=0x%08x\n", SCUB_GPIO1_PE);
	warp_printf("    SCUB_GPIO1_PS=0x%08x\n", SCUB_GPIO1_PS);
	warp_printf("    SCUB_GPIO2_IE=0x%08x\n", SCUB_GPIO2_IE);
	warp_printf("    SCUB_GPIO2_DS=0x%08x\n", SCUB_GPIO2_DS);
	warp_printf("    SCUB_GPIO2_PE=0x%08x\n", SCUB_GPIO2_PE);
	warp_printf("    SCUB_GPIO2_PS=0x%08x\n", SCUB_GPIO2_PS);
	warp_printf("    SCUB_GPIO3_IE=0x%08x\n", SCUB_GPIO3_IE);
	warp_printf("    SCUB_GPIO3_DS=0x%08x\n", SCUB_GPIO3_DS);
	warp_printf("    SCUB_GPIO3_PE=0x%08x\n", SCUB_GPIO3_PE);
	warp_printf("    SCUB_GPIO3_PS=0x%08x\n", SCUB_GPIO3_PS);
	warp_printf("    SCUB_PIN_MUX=0x%08x\n", SCUB_PIN_MUX);
#endif
}
#else
#define warp_putchar   NULL
#define dbg_regdump()
#endif

static int gp_snapshot_waitlock = 0;
static int gp_snapshot_wait_flag = 0;
static struct miscdevice misdev;     				

static ssize_t gp_snapshot_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	printk("[%s]\n", __FUNCTION__);
	gp_snapshot_waitlock = 0;

	return -1;
}

static ssize_t gp_snapshot_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	printk("[%s] In\n", __FUNCTION__);
	gp_snapshot_wait_flag = 1; 
	while (gp_snapshot_waitlock == 0)
		msleep(100);
       gp_snapshot_wait_flag = 0;  
	printk("[%s] Out\n", __FUNCTION__);	   
	return -1;
}

static struct file_operations gp_snapshot_fops = {
	.owner		= THIS_MODULE,
	.read		= gp_snapshot_read,
	.write		= gp_snapshot_write,
};


static int warp_drv_init(void)
{

	do_gettimeofday(&tv);
	va_sram = ioremap(SRAM_RESERVED_START, SRAM_RESERVED_SIZE);

	return 0;
}

static int warp_drv_load (void *buf, size_t size)
{
	gpGenStorageApiAddr_t *pGpGenStorageApi;
#ifdef WARP_HIBDRV_DEV_LOAD
	int ret = 0, i;
#if 0
	/* Load to file. for debug */
	ret = warp_dev_load("/media/sdcardb1/hibdrv.bin", buf, size);
#else
    for (i = 0; i < WARP_SNAPSHOT_NUM; i++)
        savearea_num_check[i] = 0;

    ret = gp_fastbot_binary_read(buf, size / 512);
    if (ret != 0) {
        printk("[%s][%d] gp_fastbot_binary_read error (return: %d)\n",
            __func__, __LINE__, ret);
        return ret;
    }

    for (i = 0; i < WARP_SNAPSHOT_NUM; i++) {
        int bootoffs, snapoffs, bootsize, snapsize;
        /* Get parameter: snapshot image x */
        ret =gp_fastbot_parameter_get(i, &bootoffs, &snapoffs, &bootsize, &snapsize);
        if (ret != 0)
            continue;
        savearea_num_check[i] = 1;
	    savearea[i].bootflag_area = bootoffs * 512;
	    savearea[i].bootflag_size = bootsize * 512;
	    savearea[i].snapshot_area = snapoffs * 512;
	    savearea[i].snapshot_size = snapsize * 512;
        warp_printf("T-DEBUG: [%s][%d] %d: bootoffs=%d, bootsize=%d, snapoffs=%d, snapsize=%d\n",
            __func__, __LINE__, i, bootoffs, bootsize, snapoffs, snapsize);
    }
#endif
#endif

	//get memory for bootloader using
	ka = gp_chunk_malloc(0, FASBOOT_BUFFER_SIZE);
	//printk("gp_chunk_malloc return ka=%08x\n", (unsigned int)ka);
	if (ka == NULL) {
		warp_printf("CHUNK_MEM_ALLOC: out of memory! (size:%08X)\n", FASBOOT_BUFFER_SIZE);
		ret = -ENOMEM;
		//@todo : force normal power whenout of memory
	}
	pa = gp_chunk_pa(ka);
	printk("gp_chunk_malloc for fastboot pa=%08x\n", pa);

	//save buffer address 
	gp_fastbot_buffer_address_write(pa, FASBOOT_BUFFER_SIZE);

#if RELOAD_BOOTLOADER
	//reload bootloader
	ret = gp_fastbot_load_bootlaoder();
	if( ret != 0 ) {
		printk("[%s][%d] gp_fastbot_load_bootlaoder fail[%x]\n", __FUNCTION__, __LINE__, ret);
	}
#endif /* RELOAD_BOOTLOADER */

	//Todo: Judge nand
	if(getBootDevID() != DEVICE_SD0 && getBootDevID() != DEVICE_SD1){
		printk("\nFastboot - Flush NAND\n\n\n");
		pGpGenStorageApi = (gpGenStorageApiAddr_t *)gp_fastboot_storage_api_get();
		if( pGpGenStorageApi != NULL ) {
			pGpGenStorageApi->gpStorageFlush(0, 0);
		}
		else{
			printk("\nFastboot - Flush NAND NGNGNG. Not function register\n\n");
		}
		//nand_flush_user_partition(0, 0);
	}

	gp_snapshot_waitlock = 1;
	if (gp_snapshot_wait_flag == 1) {
		/*Wait for UI response*/
		while (gp_snapshot_waitlock != 0)
			msleep(100);
	}
	else {
		gp_snapshot_waitlock = 0;
	}
	
	return ret;
}


static void warp_drv_uninit (void)
{
	iounmap(va_sram);

	return;
}

static int warp_device_suspend(void)
{
 	return 0;
}

static void warp_device_resume(void)
{
}

#if 1
static void warp_chunk_save_mem(unsigned long addr, unsigned long size)
{
	unsigned long start, end;

	start = addr & ~3;
    end = (addr + size + 3) & ~3;

	//warp_printf("T-DEBUG [%s][%d] addr=0x%x, size=0x%x area=0x%x - 0x%x\n", __func__, __LINE__, addr, size, start,end);
	printk("[%s][%d] addr=0x%x, size=0x%x area=0x%x - 0x%x\n", __func__, __LINE__, addr, size, start,end);
	warp_set_savearea(start, end);
}
#endif

/*
 * SCU
 */

static unsigned int scu_a_spll0_con, scu_a_spll1_con, scu_a_spll2_con;
static unsigned int scu_a_clksrc_mux_sel;
static struct {
    unsigned int udt;
    unsigned int _int;
    unsigned int den;
    unsigned int num;
} scu_a_clk_val[19];
static unsigned int scu_j_eth_drive_ref, scu_j_apbdma_int_en;

static inline void save_scu_regs(void)
{
    int i;
    unsigned int offs;

    scu_a_spll0_con = SPLL0_CON_REG;
    scu_a_spll1_con = SPLL1_CON_REG;
    scu_a_spll2_con = SPLL2_CON_REG;
    scu_a_clksrc_mux_sel = CLKSRC_MUX_SEL;
    warp_printf("T-DEBUG: [%s][%d] spll0=%x, spll1=%x, spll2=%x, mux_sel=%x\n", __func__, __LINE__, scu_a_spll0_con, scu_a_spll1_con, scu_a_spll2_con, scu_a_clksrc_mux_sel);

    for (i = 0, offs = 0; i < 19; i++) {
        if (i == 3 || i == 16 || i == 17)
            continue;
        offs = (LOGI_ADDR_SCU_A_REG + 0x400 + (i * 0x10));
        scu_a_clk_val[i].udt = *(volatile unsigned int*)(offs + 0x00);
        scu_a_clk_val[i]._int = *(volatile unsigned int*)(offs + 0x04);
        scu_a_clk_val[i].den = *(volatile unsigned int*)(offs + 0x08);
        scu_a_clk_val[i].num = *(volatile unsigned int*)(offs + 0x0c);
        warp_printf("T-DEBUG: [%s][%d] clk%02d udt=%x, int=%x, den=%x, num=%x\n", __func__, __LINE__, i, scu_a_clk_val[i].udt, scu_a_clk_val[i]._int, scu_a_clk_val[i].den, scu_a_clk_val[i].num);
    }

    scu_j_eth_drive_ref = ETH_DRIVE_REF;
    scu_j_apbdma_int_en = APBDMA_INT_EN;
}

#define MAX_COUNT 0xFFFF
static inline void restore_scu_regs(void)
{
    int i;
    unsigned int offs;
	unsigned int val = 0, val2 = 0, tmp = 0, count = 0;
    printk(KERN_WARNING "T-DEBUG: [%s][%d] enter\n", __func__, __LINE__);
    #if 0
        SPLL0_CON_REG = scu_a_spll0_con;
        SPLL1_CON_REG = scu_a_spll1_con;
        SPLL2_CON_REG = scu_a_spll2_con;
    #endif
    for (i = 0, offs = 0; i < 19; i++) {
        // added by warren to avoid change setting for cpu
        if (i == 3 || i == 16 || i == 17 || i == 1 || i == 0)
            continue;
        offs = (LOGI_ADDR_SCU_A_REG + 0x400 + (i * 0x10));
#if 0
        *(volatile unsigned int*)(offs + 0x00) = scu_a_clk_val[i].udt;
        *(volatile unsigned int*)(offs + 0x04) = scu_a_clk_val[i]._int;
        *(volatile unsigned int*)(offs + 0x08) = scu_a_clk_val[i].den;
        *(volatile unsigned int*)(offs + 0x0c) = scu_a_clk_val[i].num;
#else
		*(volatile unsigned int*)(offs + 0x00) &= 0xFFFFFFFE;
#if 0
		// temporary work around for on2 dec
		if(i == 6){
			*(volatile unsigned int*)(offs + 0x04) = 1;
		}
		else{
			*(volatile unsigned int*)(offs + 0x04) = scu_a_clk_val[i]._int;
		}
#else
		*(volatile unsigned int*)(offs + 0x04) = scu_a_clk_val[i]._int;
#endif
        *(volatile unsigned int*)(offs + 0x08) = scu_a_clk_val[i].den;
        *(volatile unsigned int*)(offs + 0x0c) = scu_a_clk_val[i].num;
		// issue update request
		val = *(volatile unsigned int*)(offs + 0x00);
		*(volatile unsigned int*)(offs + 0x00) = (val | 0x1);
		val = 0;
		count = 0;
        val2 = 0;
		// waiting for ack
		while(0 == val2){
			val = *(volatile unsigned int*)(offs + 0x00);
			val2 = val & 0x10000;
			count++;
			if(count > MAX_COUNT){
        printk(KERN_WARNING "T-DEBUG: [%s][%d] timeout\n", __func__, __LINE__);
				break;
			}
		}
		
		// check d2 update bit value
		tmp = val & 0x20;
		
		//clear request
		val = scu_a_clk_val[i].udt;
		val &= 0xFFFFFFFE;
		
		if(tmp){
			val &= (~0x20);
		}
		else{
			val |= 0x20;
		}
		
        //*(volatile unsigned int*)(offs + 0x00) = 0xFFFFFFFE;
		*(volatile unsigned int*)(offs + 0x00) = val;
#endif
    }

	CLKSRC_MUX_SEL = scu_a_clksrc_mux_sel;
    ETH_DRIVE_REF = scu_j_eth_drive_ref;
    APBDMA_INT_EN = scu_j_apbdma_int_en;
}


/*
 * GPIO
 */

static gpioReg_t gpio_regs;

static void save_gpio_regs(void)
{
    gpioReg_t *regs = (gpioReg_t *)LOGI_ADDR_GPIO_REG;

    gpio_regs.gpioEnable0 = regs->gpioEnable0;
    gpio_regs.gpioEnable1 = regs->gpioEnable1;
    gpio_regs.gpioEnable2 = regs->gpioEnable2;
    gpio_regs.gpioEnable3 = regs->gpioEnable3;
    gpio_regs.gpioEnable4 = regs->gpioEnable4;
    gpio_regs.gpioEnable5 = regs->gpioEnable5;
#if 0
    gpio_regs.gpioOutData0 = regs->gpioOutData0;
    gpio_regs.gpioOutData1 = regs->gpioOutData1;
    gpio_regs.gpioOutData2 = regs->gpioOutData2;
    gpio_regs.gpioOutData3 = regs->gpioOutData3;
    gpio_regs.gpioOutData4 = regs->gpioOutData4;
    gpio_regs.gpioOutData5 = regs->gpioOutData5;
#endif
    gpio_regs.gpioDirection0 = regs->gpioDirection0;
    gpio_regs.gpioDirection1 = regs->gpioDirection1;
    gpio_regs.gpioDirection2 = regs->gpioDirection2;
    gpio_regs.gpioDirection3 = regs->gpioDirection3;
    gpio_regs.gpioDirection4 = regs->gpioDirection4;
    gpio_regs.gpioDirection5 = regs->gpioDirection5;
    gpio_regs.gpioPolarity0 = regs->gpioPolarity0;
    gpio_regs.gpioPolarity1 = regs->gpioPolarity1;
    gpio_regs.gpioPolarity2 = regs->gpioPolarity2;
    gpio_regs.gpioPolarity3 = regs->gpioPolarity3;
    gpio_regs.gpioPolarity4 = regs->gpioPolarity4;
    gpio_regs.gpioPolarity5 = regs->gpioPolarity5;
    gpio_regs.gpioSticky0 = regs->gpioSticky0;
    gpio_regs.gpioSticky1 = regs->gpioSticky1;
    gpio_regs.gpioSticky2 = regs->gpioSticky2;
    gpio_regs.gpioSticky3 = regs->gpioSticky3;
    gpio_regs.gpioSticky4 = regs->gpioSticky4;
    gpio_regs.gpioSticky5 = regs->gpioSticky5;
    gpio_regs.gpioIntEn0 = regs->gpioIntEn0;
    gpio_regs.gpioIntEn1 = regs->gpioIntEn1;
    gpio_regs.gpioIntEn2 = regs->gpioIntEn2;
    gpio_regs.gpioIntEn3 = regs->gpioIntEn3;
    gpio_regs.gpioIntEn4 = regs->gpioIntEn4;
    gpio_regs.gpioIntEn5 = regs->gpioIntEn5;
#if 0
    gpio_regs.gpioIntPending0 = regs->gpioIntPending0;
    gpio_regs.gpioIntPending1 = regs->gpioIntPending1;
    gpio_regs.gpioIntPending2 = regs->gpioIntPending2;
    gpio_regs.gpioIntPending3 = regs->gpioIntPending3;
    gpio_regs.gpioIntPending4 = regs->gpioIntPending4;
    gpio_regs.gpioIntPending5 = regs->gpioIntPending5;
#endif
    gpio_regs.gpioDebounceReg0 = regs->gpioDebounceReg0;
    gpio_regs.gpioDebounceReg1 = regs->gpioDebounceReg1;
    gpio_regs.gpioDebounceReg2 = regs->gpioDebounceReg2;
    gpio_regs.gpioDebounceReg3 = regs->gpioDebounceReg3;
    gpio_regs.gpioDebounceReg4 = regs->gpioDebounceReg4;
    gpio_regs.gpioDebounceReg5 = regs->gpioDebounceReg5;
    gpio_regs.gpioDebounceEn0 = regs->gpioDebounceEn0;
    gpio_regs.gpioDebounceEn1 = regs->gpioDebounceEn1;
    gpio_regs.gpioDebounceEn2 = regs->gpioDebounceEn2;
    gpio_regs.gpioDebounceEn3 = regs->gpioDebounceEn3;
    gpio_regs.gpioDebounceEn4 = regs->gpioDebounceEn4;
    gpio_regs.gpioDebounceEn5 = regs->gpioDebounceEn5;
    gpio_regs.gpioWakeupEn0 = regs->gpioWakeupEn0;
    gpio_regs.gpioWakeupEn1 = regs->gpioWakeupEn1;
    gpio_regs.gpioWakeupEn2 = regs->gpioWakeupEn2;
    gpio_regs.gpioWakeupEn3 = regs->gpioWakeupEn3;
    gpio_regs.gpioWakeupEn4 = regs->gpioWakeupEn4;
    gpio_regs.gpioWakeupEn5 = regs->gpioWakeupEn5;
}

static void restore_gpio_regs(void)
{
    gpioReg_t *regs = (gpioReg_t *)LOGI_ADDR_GPIO_REG;

     regs->gpioEnable0 = gpio_regs.gpioEnable0;
     regs->gpioEnable1 = gpio_regs.gpioEnable1;
     regs->gpioEnable2 = gpio_regs.gpioEnable2;
     regs->gpioEnable3 = gpio_regs.gpioEnable3;
     regs->gpioEnable4 = gpio_regs.gpioEnable4;
     regs->gpioEnable5 = gpio_regs.gpioEnable5;
#if 0
     regs->gpioOutData0 = gpio_regs.gpioOutData0;
     regs->gpioOutData1 = gpio_regs.gpioOutData1;
     regs->gpioOutData2 = gpio_regs.gpioOutData2;
     regs->gpioOutData3 = gpio_regs.gpioOutData3;
     regs->gpioOutData4 = gpio_regs.gpioOutData4;
     regs->gpioOutData5 = gpio_regs.gpioOutData5;
#endif
     regs->gpioDirection0 = gpio_regs.gpioDirection0;
     regs->gpioDirection1 = gpio_regs.gpioDirection1;
     regs->gpioDirection2 = gpio_regs.gpioDirection2;
     regs->gpioDirection3 = gpio_regs.gpioDirection3;
     regs->gpioDirection4 = gpio_regs.gpioDirection4;
     regs->gpioDirection5 = gpio_regs.gpioDirection5;
     regs->gpioPolarity0 = gpio_regs.gpioPolarity0;
     regs->gpioPolarity1 = gpio_regs.gpioPolarity1;
     regs->gpioPolarity2 = gpio_regs.gpioPolarity2;
     regs->gpioPolarity3 = gpio_regs.gpioPolarity3;
     regs->gpioPolarity4 = gpio_regs.gpioPolarity4;
     regs->gpioPolarity5 = gpio_regs.gpioPolarity5;
     regs->gpioSticky0 = gpio_regs.gpioSticky0;
     regs->gpioSticky1 = gpio_regs.gpioSticky1;
     regs->gpioSticky2 = gpio_regs.gpioSticky2;
     regs->gpioSticky3 = gpio_regs.gpioSticky3;
     regs->gpioSticky4 = gpio_regs.gpioSticky4;
     regs->gpioSticky5 = gpio_regs.gpioSticky5;
     regs->gpioIntEn0 = gpio_regs.gpioIntEn0;
     regs->gpioIntEn1 = gpio_regs.gpioIntEn1;
     regs->gpioIntEn2 = gpio_regs.gpioIntEn2;
     regs->gpioIntEn3 = gpio_regs.gpioIntEn3;
     regs->gpioIntEn4 = gpio_regs.gpioIntEn4;
     regs->gpioIntEn5 = gpio_regs.gpioIntEn5;
#if 0
     regs->gpioIntPending0 = gpio_regs.gpioIntPending0;
     regs->gpioIntPending1 = gpio_regs.gpioIntPending1;
     regs->gpioIntPending2 = gpio_regs.gpioIntPending2;
     regs->gpioIntPending3 = gpio_regs.gpioIntPending3;
     regs->gpioIntPending4 = gpio_regs.gpioIntPending4;
     regs->gpioIntPending5 = gpio_regs.gpioIntPending5;
#endif
     regs->gpioDebounceReg0 = gpio_regs.gpioDebounceReg0;
     regs->gpioDebounceReg1 = gpio_regs.gpioDebounceReg1;
     regs->gpioDebounceReg2 = gpio_regs.gpioDebounceReg2;
     regs->gpioDebounceReg3 = gpio_regs.gpioDebounceReg3;
     regs->gpioDebounceReg4 = gpio_regs.gpioDebounceReg4;
     regs->gpioDebounceReg5 = gpio_regs.gpioDebounceReg5;
     regs->gpioDebounceEn0 = gpio_regs.gpioDebounceEn0;
     regs->gpioDebounceEn1 = gpio_regs.gpioDebounceEn1;
     regs->gpioDebounceEn2 = gpio_regs.gpioDebounceEn2;
     regs->gpioDebounceEn3 = gpio_regs.gpioDebounceEn3;
     regs->gpioDebounceEn4 = gpio_regs.gpioDebounceEn4;
     regs->gpioDebounceEn5 = gpio_regs.gpioDebounceEn5;
     regs->gpioWakeupEn0 = gpio_regs.gpioWakeupEn0;
     regs->gpioWakeupEn1 = gpio_regs.gpioWakeupEn1;
     regs->gpioWakeupEn2 = gpio_regs.gpioWakeupEn2;
     regs->gpioWakeupEn3 = gpio_regs.gpioWakeupEn3;
     regs->gpioWakeupEn4 = gpio_regs.gpioWakeupEn4;
     regs->gpioWakeupEn5 = gpio_regs.gpioWakeupEn5;
}



/* 
 * IRQ
 */

static struct {
    unsigned int intselect;
    unsigned int intenable;
    unsigned int softint;
    unsigned int protection;
    unsigned int prioritymask;
} irq_regs[2];

static void save_irq_regs(void)
{
    irq_regs[0].intselect = VIC0_INTSELECT;
    irq_regs[0].intenable = VIC0_INTENABLE;
    irq_regs[0].softint = VIC0_SOFTINT;
    irq_regs[0].protection = VIC0_PROTECTION;
    irq_regs[0].prioritymask = VIC0_PRIORITYMASK;
    irq_regs[1].intselect = VIC1_INTSELECT;
    irq_regs[1].intenable = VIC1_INTENABLE;
    irq_regs[1].softint = VIC1_SOFTINT;
    irq_regs[1].protection = VIC1_PROTECTION;
    irq_regs[1].prioritymask = VIC1_PRIORITYMASK;
}

static void restore_irq_regs(void)
{
    VIC0_INTENCLEAR = 0xffffffff;
    VIC0_SOFTINTCLEAR = 0xffffffff;

    VIC0_INTSELECT = irq_regs[0].intselect;
    VIC0_PROTECTION = irq_regs[0].protection;
    VIC0_PRIORITYMASK = irq_regs[0].prioritymask;
    VIC1_INTSELECT = irq_regs[1].intselect;
    VIC1_PROTECTION = irq_regs[1].protection;
    VIC1_PRIORITYMASK = irq_regs[1].prioritymask;
    VIC0_SOFTINT = irq_regs[0].softint;
    VIC1_SOFTINT = irq_regs[1].softint;
    VIC0_INTENABLE = irq_regs[0].intenable;
    VIC1_INTENABLE = irq_regs[1].intenable;
}


/*
 * TIMER
 */

static timerReg_t timer_regs[3];

static void save_timer_regs(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        timerReg_t *regs = (timerReg_t *)(LOGI_ADDR_TIMER_REG +
                                i * LOGI_TIMER_OFFSET);
        timer_regs[i].tmCtr = regs->tmCtr;
        timer_regs[i].tmPsr = regs->tmPsr;
        timer_regs[i].tmLdrVlr = regs->tmLdrVlr;
        timer_regs[i].tmIsr = regs->tmIsr;
        timer_regs[i].tmCmp = regs->tmCmp;
    }
}

/*
 * SRAM
 */
static void save_sram(void)
{
	memcpy(sram_data_ptr, va_sram, SRAM_RESERVED_SIZE);
}

static void restore_sram(void)
{
	memcpy(va_sram, sram_data_ptr, SRAM_RESERVED_SIZE);
}

static void restore_timer_regs(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        timerReg_t *regs = (timerReg_t *)(LOGI_ADDR_TIMER_REG +
                                i * LOGI_TIMER_OFFSET);
        regs->tmCtr = timer_regs[i].tmCtr;
        regs->tmPsr = timer_regs[i].tmPsr;
        regs->tmLdrVlr = timer_regs[i].tmLdrVlr;
        regs->tmIsr = timer_regs[i].tmIsr;
        regs->tmCmp = timer_regs[i].tmCmp;
    }
}

static int warp_snapshot(void)
{
	int ret = 0;
	int warp_saveno = warp_param.bootflag_area;
	unsigned int frame_buf_addr = 0, frame_buf_size = 0;

    if (warp_saveno >= WARP_SNAPSHOT_NUM ||
        savearea_num_check[warp_saveno] == 0)
        return -EINVAL;

	//update warp interface
	*(unsigned int *)(__va(0xF0000 + 0x28)) = pa;
	*(unsigned int *)(__va(0xF0000 + 0x2C)) = FASBOOT_BUFFER_SIZE;

	/* free memory */
	if (ka)	gp_chunk_free(ka);

	//get frame buffer address and size, if size != 0, means need reserved frame buffer
	fb_get_buff_info(&frame_buf_addr, &frame_buf_size);
	if (frame_buf_size != 0) {
		warp_chunk_save_mem(frame_buf_addr, frame_buf_size);
	}

	/* Save chunk memory */
	gp_chunk_suspend(warp_chunk_save_mem);

	/* save srame */
	save_sram();

	/* save regs */
    save_irq_regs();
    save_timer_regs();
    save_gpio_regs();
    save_scu_regs();

	/* set param */
	warp_param.bootflag_area = savearea[warp_saveno].bootflag_area;
	warp_param.bootflag_size = savearea[warp_saveno].bootflag_size;
	warp_param.snapshot_area = savearea[warp_saveno].snapshot_area;
	warp_param.snapshot_size = savearea[warp_saveno].snapshot_size;

	//warp_param.snapshot_ver = tv.tv_usec;

#if 0
	warp_param.private = UART0_BASE; //T-DEBUG debug uart : for hibdrv
#else
	warp_param.private = 0x000f0000; //header address
#endif

	dbg_regdump();

	warp_printf("[%s][%d] hibdrv_snapshot() start\n",
			__func__, __LINE__);
	ret = hibdrv_snapshot();
	warp_printf("[%s][%d] hibdrv_snapshot() end(return=%d)\n",
		__func__, __LINE__, ret);

	warp_printf(" ===============[Start power off] (ret=%d, warp_param.stat=%d) ===============\n",
		ret, warp_param.stat);
	if ( (ret >= 0) || (ret == -ENOMEM) ) {
	    if (warp_param.stat == 0) {
	        /* return from saving snapshot */
	        spmp8050_power_off();
	        //*(volatile unsigned int*)(LOGI_ADDR_SCU_A_REG + 0xF80) = 0;
	        while(1) {
				warp_printf("[%s][%d] : wait power off\n",
					__func__, __LINE__);
				}
	        	
	    } else {
	        /* Warp!! fast boot */
	    }
	}

	warp_printf("[%s][%d] hibernation canceled flag=%d\n",__func__, __LINE__, warp_canceled);
    if (warp_canceled == 1) {
    	//if hibernation was canceled, clear flag
		warp_canceled = 0;
	}

	/* restore regs */
	warp_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);
    restore_scu_regs();
	warp_printf("[%s][%d] : restore_gpio_regs\n",
		__func__, __LINE__);
    restore_gpio_regs();
	warp_printf("[%s][%d] : restore_irq_regs\n",
		__func__, __LINE__);
    restore_irq_regs();
	warp_printf("[%s][%d] : restore_timer_regs\n",
		__func__, __LINE__);
    restore_timer_regs();
	warp_printf("[%s][%d] : restore_regs done\n",
		__func__, __LINE__);

	/* restore srame */
	restore_sram();

	dbg_regdump();

	return ret;
}

static struct warp_ops warp_machine_ops = {
	.drv_load		= warp_drv_load,
	.drv_init       = warp_drv_init,
	.device_suspend = warp_device_suspend,
	.device_resume  = warp_device_resume,
	.snapshot       = warp_snapshot,
	.drv_uninit     = warp_drv_uninit,
	.putc           = warp_putchar,
};

static int __init warp_machine_init(void)
{
	//allocate memory for sram reserved

	sram_data_ptr = kmalloc( SRAM_RESERVED_SIZE, GFP_ATOMIC);
	if (sram_data_ptr == NULL) {
		printk("kmalloc fail : out of memory! (size:%08X)\n", SRAM_RESERVED_SIZE);
		return -ENOMEM;
	}
	
	/* register misc device */
	misdev.name = "snapshot";
	misdev.minor = MISC_DYNAMIC_MINOR;
	misdev.fops  = &gp_snapshot_fops;
	misc_register(&misdev);

	return warp_register_machine(&warp_machine_ops);
}

static void __exit warp_machine_exit(void)
{
	warp_unregister_machine(&warp_machine_ops);

	//free memory for sram reserved data
	if (sram_data_ptr) {
		kfree(sram_data_ptr);
		sram_data_ptr = NULL;
	}
}

module_init(warp_machine_init);
module_exit(warp_machine_exit);
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Snapshot Driver");
MODULE_LICENSE_GP;
