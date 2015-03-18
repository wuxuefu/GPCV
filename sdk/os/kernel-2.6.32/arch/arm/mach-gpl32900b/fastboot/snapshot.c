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
 * @file    snapshot.c
 * @brief   
 * @author  Milton Jiang
 * @date    2013-08-30
 */

#include <mach/kernel.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <mach/gp_speedy_boot.h>
#include <asm/cacheflush.h>
#include <mach/gp_cpx.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/hal/regmap/reg_vic.h>
#include <mach/hal/regmap/reg_timer.h>
#include <mach/hal/regmap/reg_wdt.h>
#include <mach/hal/regmap/reg_gpio.h>
#include <mach/regs-interrupt.h>
#include <mach/hardware.h>
#include <mach/gp_chunkmem.h>
#include <mach/gp_storage_api.h>
#include <mach/cdev.h>

#include <mach/gp_suspend.h>
#include <mach/cpu_pwr_down.h>
//#include <mach/gp_ceva.h>
#include <mach/hal/hal_clock.h>

//extern int gp_ceva_nop(void);
//extern void ceva_clock_enable(int enable);

//extern SINT32 nand_flush_user_partition(UINT8 which_kind, UINT16 which_partition);
extern int getBootDevID (void);
#if 0 // milton 
extern spmp8050_power_off(void);
extern int power_down_domain(PWR_DOMAIN aDomain, int aEnable);
#endif

static struct timeval tv;
static u8 check_part_gpfb = 0;

static struct savearea
{
    unsigned int bootflag_area;
    unsigned int bootflag_size;
    unsigned int snapshot_area;
    unsigned int snapshot_size;
} savearea[SPEEDY_SNAPSHOT_NUM];
static u8 savearea_num_check[SPEEDY_SNAPSHOT_NUM];

static void *ka=0;           /* kernel_addr */
static unsigned int pa=0;    /* phy_addr*/
static unsigned char *sram_data_ptr = NULL;
static unsigned char *va_sram;
static int reserved_buffer_size = 0;

#ifdef CONFIG_PM_SPEEDY_DEBUG
//#define GP_FUNCTION_TIME_CHECK
#undef GP_FUNCTION_TIME_CHECK
#else
#undef GP_FUNCTION_TIME_CHECK
#endif /* CONFIG_PM_SPEEDY_DEBUG */

extern int gp_fastbot_binary_read(int *buf, int size_sector);
extern int gp_fastbot_parameter_get(int index, int *boot_offset, int *image_offset, int *boot_size, int *image_size);
extern int gp_sdcard_app_read_sector(u32, u32, u16, u32);
//extern gpGenStorageApiAddr_t gpGenStorageApi;

#ifdef CONFIG_PM_SPEEDY_DEBUG
#include <linux/serial_reg.h>
#include <mach/hal/hal_uart.h>
#define UART_ID		0
#define BOTH_EMPTY	(UART_LSR_TEMT | UART_LSR_THRE)

static regs_uart_t* pUartPorts[]= {
	((regs_uart_t*)(UART2_BASE)),
	((regs_uart_t*)(UART0_BASE)),
};

static void speedy_putchar(char c)
{
	unsigned long status;

	if (c == '\n')
	 	speedy_putc('\r');

	/* Wait up to 2ms for the character(s)  to be sent. */
	do {
		status = pUartPorts[UART_ID]->regLSR;
	} while ((status & BOTH_EMPTY) != BOTH_EMPTY);

	pUartPorts[UART_ID]->regTHR = c;
}

#define AH_START_LBA	0
#define AH_SIZE			  (4096 / 512)	/* page alignment */
#define SD_NUM		    2 
static int app_header_read(void *buf)
{
	int ret = 0;

	speedy_printf("[%s:%d]\n", __func__, __LINE__);
	
	ret = gp_sdcard_app_read_sector(speedy_param.snapshot_dev, AH_START_LBA, AH_SIZE, (u32)buf);
	if (ret)
		return -EIO;
	
	speedy_printf("%s:%d\n", __func__, __LINE__);
	flush_cache_all();

	speedy_printf("[%s:%d] ret=%d\n", __func__, __LINE__, ret);

#if 0
	{	/* debug dump */
		int i;
		u8 *p = (u8*)buf;
		speedy_printf("---------- App Header dump ------------ \n");
		for (i = 0; i < 128; i++) {
			if (!(i % 8))
				printk("\n%08x:  ", i);
			printk("%02x ", p[i]);
		}
		speedy_printf("\n---------------------------------------\n");
	}
#endif

	return 0;
}

#define AH_PARTTYPE_KERNEL		0	/* kernel area */
#define AH_PARTTYPE_RESOURCE	1	/* resource */
#define AH_PARTTYPE_GPSB			2	/* reserved */
#define AH_PARTTYPE_SNAPIMG		3	/* snapshot image 1 */
#define AH_PARTTYPE_HIBIMG		4	/* snapshot image 2 */
#define AH_PARTTYPE_GPSBFLAG	5	/* reserved */
#define AH_PARTTYPE_2NDLOADER	6	/* 2nd bootloader */

#define AH_OFFS_HTAG_0				0	/* App Header Tag: 8bit access */
#define AH_OFFS_HTAG_1				1	/* App Header Tag: 8bit access */
#define AH_OFFS_HTAG_2				2	/* App Header Tag: 8bit access */
#define AH_OFFS_HTAG_3				3	/* App Header Tag: 8bit access */
#define AH_OFFS_PART_N				12 /* App Part Nums: 16bit access */
#define AH_OFFS_PART_START_SECT_ID(x)	(24 + ((x) * 16)) /* PartX Start Sect ID: 32bit access */
#define AH_OFFS_PART_IMAGE_SIZE(x)	(28 + ((x) * 16)) /* PartX Image Size: 32bit access */
#define AH_OFFS_PART_TYPE(x)		(32 + ((x) * 16)) /* PartX Image Size: 8bit access */

static int app_header_check(void *buf,
	u32 *hibdrv_area_sect, u32 *hibdrv_area_size)
{
	int partno = 0, i;
	u32 sect_id = 0, sect_size = 0;
	u32 area_total_size;

	check_part_gpfb = 0;

	if (buf == NULL) {
		printk("[%s][%d] no Memory\n", __func__, __LINE__);
		return -ENOMEM;
	}
	
	/* check Header Tag */
	if ((*(u8*)(buf + AH_OFFS_HTAG_0) != 'G') ||
	    (*(u8*)(buf + AH_OFFS_HTAG_1) != 'P') ||
	    (*(u8*)(buf + AH_OFFS_HTAG_2) != 'A') ||
	    (*(u8*)(buf + AH_OFFS_HTAG_3) != 'P')) {
		printk("[%s][%d] App Header bad format\n", __func__, __LINE__);
		return -EINVAL;
	}

	partno = *(u16*)(buf + AH_OFFS_PART_N);
	speedy_printf("[%s][%d] partno=%d\n", __func__, __LINE__, partno);

	for (i = 0; i < partno; i++) {
		u8 parttype = *(u8*)(buf + AH_OFFS_PART_TYPE(i));
		sect_id = *(u32*)(buf + AH_OFFS_PART_START_SECT_ID(i));
		sect_size = *(u32*)(buf + AH_OFFS_PART_IMAGE_SIZE(i));
		switch (parttype) {
			case AH_PARTTYPE_GPSB:
				/* hibdrv */
				break;
			case AH_PARTTYPE_SNAPIMG:
				/* snapshot image 1 */
				break;
			case AH_PARTTYPE_HIBIMG:
				/* snapshot image 2 */
				break;
			case AH_PARTTYPE_GPSBFLAG:
				/* reserved */
				break;
			case AH_PARTTYPE_2NDLOADER:
				*hibdrv_area_sect = sect_id;
				*hibdrv_area_size = sect_size;
				check_part_gpfb = 1;
				speedy_printf("[%s][%d] part 2nd bootloader: sect=%x, size=%x\n",
					__func__, __LINE__, sect_id, sect_size);
				break;	
			default:
				break;
		}
	}

	if ( check_part_gpfb != 1 ) {
		printk("[%s][%d] Partition not find.\n", __func__, __LINE__);
		return -EINVAL;
	}

	return 0;
}

static int hibdrv_copy_to_sram(void *buf, u32 start_sect, u32 work_size)
{
	int ret = 0;
	u32 hibdrv_size = 0;
	u32 sect_num = 0;

	//if (work_size < 512)
	//	return -ENOMEM;

	speedy_printf("[%s:%d]: start_sect=%x, size=%x, buf=%x\n", __func__, __LINE__, start_sect, work_size, buf);

	/* GPH header read */
	ret = gp_sdcard_app_read_sector(speedy_param.snapshot_dev, start_sect, work_size, (u32)buf);

	if (ret)
		return -EIO;

	return 0;
}



static void dbg_regdump(void)
{

	int i;
	scubReg_t *pScubReg = (scuaReg_t *)LOGI_ADDR_SCU_B_REG;
	//scubReg_t *pScubReg = (scubReg_t *)LOGI_ADDR_SCU_B_REG;
	scucReg_t *pScucReg = (scucReg_t *)LOGI_ADDR_SCU_C_REG;

#if 0 // milton
	speedy_printf("+++++ IRQ +++++\n");
	speedy_printf("    VIC0 IRQ EnableSet=0x%08x\n", CYG_DEVICE_IRQ0_EnableSet);
	speedy_printf("    VIC0 IRQ EnableClear=0x%08x\n", CYG_DEVICE_IRQ0_EnableClear);
	speedy_printf("    VIC1 IRQ EnableSet=0x%08x\n", CYG_DEVICE_IRQ1_EnableSet);
	speedy_printf("    VIC1 IRQ EnableClear=0x%08x\n", CYG_DEVICE_IRQ1_EnableClear);
#endif	
	
	speedy_printf("+++++ TIMER +++++\n");
	for (i = 0; i < 4; i++) {
		timerReg_t *ptimerReg =
			(timerReg_t *)(LOGI_ADDR_TIMER_REG + (i * LOGI_TIMER_OFFSET));
		speedy_printf("    TIMER%d tmCtr=0x%08x\n", i, ptimerReg->tmCtr);
		speedy_printf("    TIMER%d tmPsr=0x%08x\n", i, ptimerReg->tmPsr);
		speedy_printf("    TIMER%d tmLdrVlr=0x%08x\n", i, ptimerReg->tmLdrVlr);
		speedy_printf("    TIMER%d tmIsr=0x%08x\n", i, ptimerReg->tmIsr);
		speedy_printf("    TIMER%d tmCmp=0x%08x\n", i, ptimerReg->tmCmp);
	}

#if 0 // milton
	speedy_printf("+++++ WDT +++++\n");
	for (i = 0; i < 4; i++) {
		wdtReg_t *pwdtReg =
			(wdtReg_t *)(LOGI_ADDR_WDT_REG + (i * LOGI_WDT_OFFSET));
		speedy_printf("    WDT%d wdtCtr=0x%08x\n",		i, pwdtReg->wdtCtr);
		speedy_printf("    WDT%d wdtPsr=0x%08x\n",		i, pwdtReg->wdtPsr);
		speedy_printf("    WDT%d wdtLdr=0x%08x\n",		i, pwdtReg->wdtLdr);
		speedy_printf("    WDT%d wdtVlr=0x%08x\n",		i, pwdtReg->wdtVlr);
		speedy_printf("    WDT%d wdtCmp=0x%08x\n",		i, pwdtReg->wdtCmp);
	}
#endif

	speedy_printf("+++++ CLOCK +++++\n");
	speedy_printf("    SCUA PeriClockEnable=0x%08x\n", pScubReg->scubPeriClkEn);
	//speedy_printf("    SCUB PeriClockEnable=0x%08x\n", pScubReg->scubPeriClkEn);
	// milton -- speedy_printf("    SCUC PeriClockEnable=0x%08x\n", pScubReg->scucPeriClkEn);
	//speedy_printf("    SCUA PeriClockEnable2=0x%08x\n", pScuaReg->scuaPeriClkEn2);
	//speedy_printf("    SCUB SysCntEn=0x%08x\n", pScubReg->scubSysCntEn);
	// milton -- speedy_printf("    SCUC CevaCntEn=0x%08x\n", pScubReg->scucCevaCntEn);
	speedy_printf("+++++ GPIO +++++\n");
	//speedy_printf("    SCUB_GPIO0_IE=0x%08x\n", SCUB_GPIO0_IE);
	//speedy_printf("    SCUB_GPIO0_DS=0x%08x\n", SCUB_GPIO0_DS);
	//speedy_printf("    SCUB_GPIO0_PE=0x%08x\n", SCUB_GPIO0_PE);
	//speedy_printf("    SCUB_GPIO0_PS=0x%08x\n", SCUB_GPIO0_PS);
	//speedy_printf("    SCUB_GPIO1_IE=0x%08x\n", SCUB_GPIO1_IE);
	//speedy_printf("    SCUB_GPIO1_DS=0x%08x\n", SCUB_GPIO1_DS);
	//speedy_printf("    SCUB_GPIO1_PE=0x%08x\n", SCUB_GPIO1_PE);
	//speedy_printf("    SCUB_GPIO1_PS=0x%08x\n", SCUB_GPIO1_PS);
	//speedy_printf("    SCUB_GPIO2_IE=0x%08x\n", SCUB_GPIO2_IE);
	//speedy_printf("    SCUB_GPIO2_DS=0x%08x\n", SCUB_GPIO2_DS);
	//speedy_printf("    SCUB_GPIO2_PE=0x%08x\n", SCUB_GPIO2_PE);
	//speedy_printf("    SCUB_GPIO2_PS=0x%08x\n", SCUB_GPIO2_PS);
	//speedy_printf("    SCUB_GPIO3_IE=0x%08x\n", SCUB_GPIO3_IE);
	//speedy_printf("    SCUB_GPIO3_DS=0x%08x\n", SCUB_GPIO3_DS);
	//speedy_printf("    SCUB_GPIO3_PE=0x%08x\n", SCUB_GPIO3_PE);
	//speedy_printf("    SCUB_GPIO3_PS=0x%08x\n", SCUB_GPIO3_PS);
	//speedy_printf("    SCUB_PIN_MUX=0x%08x\n", SCUB_PIN_MUX);
}
#else
#define speedy_putchar   NULL
#define dbg_regdump()
#endif

static int gp_snapshot_waitlock = 0;
static int gp_snapshot_wait_flag = 0;
static struct miscdevice misdev;

static void speedy_process(int val)
{
	char *state_string[11] = {
		"SPEEDY_PROGRESS_INIT",
		"SPEEDY_PROGRESS_SYNC",
		"SPEEDY_PROGRESS_FREEZE",
		"SPEEDY_PROGRESS_SHRINK",
		"SPEEDY_PROGRESS_SUSPEND",
		"SPEEDY_PROGRESS_SAVE",
		"SPEEDY_PROGRESS_SAVEEND",
		"SPEEDY_PROGRESS_RESUME",
		"SPEEDY_PROGRESS_THAW",
		"SPEEDY_PROGRESS_EXIT",
		"SPEEDY_PROGRESS_CANCEL"
	};
	speedy_printf(" =============== State ===============\n");
	if (val > 11)
		speedy_printf("[%s] state\n","Unknown");
	else
		speedy_printf("[%s] state\n",state_string[val]);
	speedy_printf(" =============== State ===============\n");

}

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
	.read			= gp_snapshot_read,
	.write		= gp_snapshot_write,
};


static int speedy_drv_init(void)
{
	do_gettimeofday(&tv);
	
	printk("[%s:%d]\r\n",__func__, __LINE__);
	
	va_sram = ioremap(SRAM_RESERVED_START, SRAM_RESERVED_SIZE);

	return 0;
}

static int speedy_drv_load (void *buf, size_t size)
{
	gpGenStorageApiAddr_t *pGpGenStorageApi;
	pnandprofileheader_t *bootHeader;
	int ret = 0, i;
	int nand_block_size = 0;
	u32 hibdrv_area_size = 0, hibdrv_area_sect_id = 0;
	//void *tmpbuf = ioremap(0x00000100, 0x80000);
	void *tmpbuf = (void *)SND_LOADER_BASE;
	
#if 0 // milton 

#if 0
	/* Load to file. for debug */
	ret = speedy_dev_load("/media/sdcardb1/hibdrv.bin", buf, size);
#else
    for (i = 0; i < SPEEDY_SNAPSHOT_NUM; i++)
        savearea_num_check[i] = 0;

	printk("[%s][%d] jiffies=0x%x\n",__func__, __LINE__, jiffies);
    ret = gp_fastbot_binary_read(buf, size / 512);
    if (ret != 0) {
        printk("[%s][%d] gp_fastbot_binary_read error (return: %d)\n",
            __func__, __LINE__, ret);
        return ret;
    }

	printk("[%s][%d] jiffies=0x%x\n",__func__, __LINE__, jiffies);
    for (i = 0; i < SPEEDY_SNAPSHOT_NUM; i++) {
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
        speedy_printf("T-DEBUG: [%s][%d] %d: bootoffs=%d, bootsize=%d, snapoffs=%d, snapsize=%d\n",
            __func__, __LINE__, i, bootoffs, bootsize, snapoffs, snapsize);
    }
	printk("[%s][%d] jiffies=0x%x\n",__func__, __LINE__, jiffies);
#endif

	//get memory for bootloader using
	//get nand block size from sram(bootheader)
	bootHeader =  (pnandprofileheader_t *) ioremap(SRAM_BOOT_HEADER_START, sizeof(pnandprofileheader_t));
	nand_block_size = bootHeader->PagePerBlock*bootHeader->PageSize;
	iounmap(bootHeader);

	//buffer size = nand block size X 2 X 6 + BOOTLOADER_RESERVED_SIZE + NAND_BLOCK SIZE(for nand buffer using)
repeat_alloc:
	reserved_buffer_size = nand_block_size*2*6 + BOOTLOADER_RESERVED_SIZE + nand_block_size;
	ka = gp_chunk_malloc(0, reserved_buffer_size);
	//printk("gp_chunk_malloc return ka=%08x\n", (unsigned int)ka);
	if (ka == NULL) {
		printk("CHUNK_MEM_ALLOC: out of memory!(blocksize =0x%x, size:0x%08x), retry for half size\n", nand_block_size, reserved_buffer_size);
		nand_block_size >>= 1;
		if (nand_block_size < 65536) {
			printk("CHUNK_MEM_ALLOC: out of memory! hibernation fail(blocksize =0x%x, size:0x%08x)\n", nand_block_size, reserved_buffer_size);
			ret = -ENOMEM;
		}
		//@todo : force normal power when out of memory
	}
	pa = gp_chunk_pa(ka);
	printk("gp_chunk_malloc for fastboot pa=%08x, size=0x%x\n", pa, reserved_buffer_size);

	//save buffer address
	//@todo : not using in GP Speedy Boot
	//gp_fastbot_buffer_address_write(pa, reserved_buffer_size);

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

#else // milton
	
	/* header read */
	ret = app_header_read(tmpbuf);
	if (ret != 0)
	{
		printk("[%s:%d]error:app_header_read", __FUNCTION__, __LINE__);
		return ret;
	}

	/* header format check & setting */
	ret = app_header_check(tmpbuf, &hibdrv_area_sect_id, &hibdrv_area_size);
	if (ret != 0)
	{
		printk("[%s:%d]error:app_header_check", __FUNCTION__, __LINE__);
		return ret;
	}
	
	/* hibdrv copy to SRAM */
	ret = hibdrv_copy_to_sram(tmpbuf, hibdrv_area_sect_id, hibdrv_area_size);
	if (ret != 0)
	{
		printk("[%s:%d]error:hibdrv_copy_to_sram", __FUNCTION__, __LINE__);
		return ret;
	}

	printk("snd bootloader addr=0x%08x\r\n", tmpbuf);
	
	reserved_buffer_size = 8*1024*1024;
	ka = gp_chunk_malloc(reserved_buffer_size);
	//printk("gp_chunk_malloc return ka=%08x\n", (unsigned int)ka);
	if (ka == NULL) {
		printk("CHUNK_MEM_ALLOC: out of memory!(size:0x%08x), retry for half size\n", reserved_buffer_size);
		//@todo : force normal power when out of memory
	}
	pa = gp_chunk_pa(ka);
	printk("gp_chunk_malloc for fastboot pa=%08x, size=0x%x\n", pa, reserved_buffer_size);

#endif // milton 

	return ret;
}

static void speedy_drv_uninit (void)
{
	printk("[%s:%d]\r\n",__func__, __LINE__);
	
	iounmap(va_sram);
	speedy_printf("T-DEBUG [%s][%d] \n", __func__, __LINE__);
	return;
}

static int speedy_device_suspend(void)
{
 	return 0;
}

static void speedy_device_resume(void)
{
	speedy_printf("T-DEBUG [%s][%d] \n", __func__, __LINE__);
}

#if 1
static void speedy_chunk_save_mem(unsigned long addr, unsigned long size)
{
	unsigned long start, end;

	start = addr & ~3;
    end = (addr + size + 3) & ~3;

	speedy_printf("T-DEBUG [%s][%d] addr=0x%x, size=0x%x area=0x%x - 0x%x\n", __func__, __LINE__, addr, size, start,end);
	//printk("[%s][%d] addr=0x%x, size=0x%x area=0x%x - 0x%x\n", __func__, __LINE__, addr, size, start,end);
	if (size < 16) {
		size = 32;
		end = start + 32;
		speedy_printf("T-DEBUG [%s][%d] Extend size to 16 for porting, addr0x%x, size=0x%x area=0x%x - 0x%x\n", __func__, __LINE__, addr, size, start,end);
	}
	speedy_set_savearea(start, end);
}
#endif

/*
 * SCU
 */

scuaReg_t *pScuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG;
scubReg_t *pScubReg = (scubReg_t *)LOGI_ADDR_SCU_B_REG;
scucReg_t *pScucReg = (scucReg_t *)LOGI_ADDR_SCU_C_REG;

scuaReg_t saveScuaReg;
scubReg_t saveScubReg;
scucReg_t saveScucReg;

static inline void save_scu_regs(void)
{

	/* clock */
	saveScuaReg.scuaPeriClkEn = pScuaReg->scuaPeriClkEn;
	saveScuaReg.scuaPeriClkEn2 = pScuaReg->scuaPeriClkEn2;
	saveScubReg.scubPeriClkEn = pScubReg->scubPeriClkEn;
	saveScubReg.scubSysCntEn = pScubReg->scubSysCntEn;
	saveScucReg.scucPeriClkEn = pScucReg->scucPeriClkEn;
}

static inline void restore_scu_regs(void)
{
		/* clock */
	pScuaReg->scuaPeriClkEn = saveScuaReg.scuaPeriClkEn;
	//milton test--if((pScuaReg->scuaPeriClkEn2) & SCU_A_PERI_PPU_TFT)//if en disp1 TFT clock
	if (0) // milton test --
	{
		 while(1)
		 {
		 		if ((*((volatile unsigned int *) IO3_ADDRESS(0x2018C))) & 0x2000)//R_PPU_IRQ_STATUS
		 		{
		 			(*((volatile unsigned int *) IO3_ADDRESS(0x2018C))) |= 0x2000;//clear IRQ status
		 			pScuaReg->scuaPeriClkEn2 = saveScuaReg.scuaPeriClkEn2;
		 			break;
		 		}
		 		speedy_printf(" wait vblank end... R_PPU_IRQ_STATUS = 0x%08x\n",(*((volatile unsigned int *) IO3_ADDRESS(0x2018C))));
		 	}
	}
	else 		 	
		pScuaReg->scuaPeriClkEn2 = saveScuaReg.scuaPeriClkEn2;
		pScubReg->scubPeriClkEn = saveScubReg.scubPeriClkEn;
		pScubReg->scubSysCntEn = saveScubReg.scubSysCntEn;
		pScucReg->scucPeriClkEn = saveScucReg.scucPeriClkEn;

}


/*
 * GPIO
 */

static gpioReg_t gpio_regs;
static gpio_pg[4];
static void save_gpio_regs(void)
{
    gpioReg_t *regs = (gpioReg_t *)LOGI_ADDR_GPIO_REG;

    gpio_regs.gpioEnable0 = regs->gpioEnable0;
    gpio_regs.gpioEnable1 = regs->gpioEnable1;
    gpio_regs.gpioEnable2 = regs->gpioEnable2;
    gpio_regs.gpioEnable3 = regs->gpioEnable3;
    gpio_regs.gpioEnable4 = regs->gpioEnable4;
                               
    gpio_regs.gpioDirection0 = regs->gpioDirection0;
    gpio_regs.gpioDirection1 = regs->gpioDirection1;
    gpio_regs.gpioDirection2 = regs->gpioDirection2;
    gpio_regs.gpioDirection3 = regs->gpioDirection3;
    gpio_regs.gpioDirection4 = regs->gpioDirection4;
                               
    gpio_regs.gpioPolarity0 = regs->gpioPolarity0;
    gpio_regs.gpioPolarity1 = regs->gpioPolarity1;
    gpio_regs.gpioPolarity2 = regs->gpioPolarity2;
    gpio_regs.gpioPolarity3 = regs->gpioPolarity3;
    gpio_regs.gpioPolarity4 = regs->gpioPolarity4;
                            
    gpio_regs.gpioSticky0 = regs->gpioSticky0;
    gpio_regs.gpioSticky1 = regs->gpioSticky1;
    gpio_regs.gpioSticky2 = regs->gpioSticky2;
    gpio_regs.gpioSticky3 = regs->gpioSticky3;
    gpio_regs.gpioSticky4 = regs->gpioSticky4;
    
    gpio_regs.gpioIntEn0 = regs->gpioIntEn0;
    gpio_regs.gpioIntEn1 = regs->gpioIntEn1;
    gpio_regs.gpioIntEn2 = regs->gpioIntEn2;
    gpio_regs.gpioIntEn3 = regs->gpioIntEn3;
    gpio_regs.gpioIntEn4 = regs->gpioIntEn4;

    gpio_regs.gpioDebounceReg0 = regs->gpioDebounceReg0;
    gpio_regs.gpioDebounceReg1 = regs->gpioDebounceReg1;
    gpio_regs.gpioDebounceReg2 = regs->gpioDebounceReg2;
    gpio_regs.gpioDebounceReg3 = regs->gpioDebounceReg3;
    gpio_regs.gpioDebounceReg4 = regs->gpioDebounceReg4;
    
    gpio_regs.gpioDebounceEn0 = regs->gpioDebounceEn0;
    gpio_regs.gpioDebounceEn1 = regs->gpioDebounceEn1;
    gpio_regs.gpioDebounceEn2 = regs->gpioDebounceEn2;
    gpio_regs.gpioDebounceEn3 = regs->gpioDebounceEn3;
    gpio_regs.gpioDebounceEn4 = regs->gpioDebounceEn4;
    
    gpio_regs.gpioWakeupEn0 = regs->gpioWakeupEn0;
    gpio_regs.gpioWakeupEn1 = regs->gpioWakeupEn1;
    gpio_regs.gpioWakeupEn2 = regs->gpioWakeupEn2;
    gpio_regs.gpioWakeupEn3 = regs->gpioWakeupEn3;
    gpio_regs.gpioWakeupEn4 = regs->gpioWakeupEn4;
    
    gpio_pg[0] = SCUB_PGS0;
    gpio_pg[1] = SCUB_PGS1;
    gpio_pg[2] = SCUB_PGS2;
    gpio_pg[3] = SCUB_PGS3;

}

static void restore_gpio_regs(void)
{
    gpioReg_t *regs = (gpioReg_t *)LOGI_ADDR_GPIO_REG;

    regs->gpioEnable0 = gpio_regs.gpioEnable0;
    regs->gpioEnable1 = gpio_regs.gpioEnable1;
    regs->gpioEnable2 = gpio_regs.gpioEnable2;
    regs->gpioEnable3 = gpio_regs.gpioEnable3;
    regs->gpioEnable4 = gpio_regs.gpioEnable4;
    
    regs->gpioDirection0 = gpio_regs.gpioDirection0;
    regs->gpioDirection1 = gpio_regs.gpioDirection1;
    regs->gpioDirection2 = gpio_regs.gpioDirection2;
    regs->gpioDirection3 = gpio_regs.gpioDirection3;
    regs->gpioDirection4 = gpio_regs.gpioDirection4;
    
    regs->gpioPolarity0 = gpio_regs.gpioPolarity0;
    regs->gpioPolarity1 = gpio_regs.gpioPolarity1;
    regs->gpioPolarity2 = gpio_regs.gpioPolarity2;
    regs->gpioPolarity3 = gpio_regs.gpioPolarity3;
    regs->gpioPolarity4 = gpio_regs.gpioPolarity4;
    
    regs->gpioSticky0 = gpio_regs.gpioSticky0;
    regs->gpioSticky1 = gpio_regs.gpioSticky1;
    regs->gpioSticky2 = gpio_regs.gpioSticky2;
    regs->gpioSticky3 = gpio_regs.gpioSticky3;
    regs->gpioSticky4 = gpio_regs.gpioSticky4;
    
    regs->gpioIntEn0 = gpio_regs.gpioIntEn0;
    regs->gpioIntEn1 = gpio_regs.gpioIntEn1;
    regs->gpioIntEn2 = gpio_regs.gpioIntEn2;
    regs->gpioIntEn3 = gpio_regs.gpioIntEn3;
    regs->gpioIntEn4 = gpio_regs.gpioIntEn4;

    regs->gpioDebounceReg0 = gpio_regs.gpioDebounceReg0;
    regs->gpioDebounceReg1 = gpio_regs.gpioDebounceReg1;
    regs->gpioDebounceReg2 = gpio_regs.gpioDebounceReg2;
    regs->gpioDebounceReg3 = gpio_regs.gpioDebounceReg3;
    regs->gpioDebounceReg4 = gpio_regs.gpioDebounceReg4;
    
    regs->gpioDebounceEn0 = gpio_regs.gpioDebounceEn0;
    regs->gpioDebounceEn1 = gpio_regs.gpioDebounceEn1;
    regs->gpioDebounceEn2 = gpio_regs.gpioDebounceEn2;
    regs->gpioDebounceEn3 = gpio_regs.gpioDebounceEn3;
    regs->gpioDebounceEn4 = gpio_regs.gpioDebounceEn4;
     
    regs->gpioWakeupEn0 = gpio_regs.gpioWakeupEn0;
    regs->gpioWakeupEn1 = gpio_regs.gpioWakeupEn1;
    regs->gpioWakeupEn2 = gpio_regs.gpioWakeupEn2;
    regs->gpioWakeupEn3 = gpio_regs.gpioWakeupEn3;
    regs->gpioWakeupEn4 = gpio_regs.gpioWakeupEn4;
     
    SCUB_PGS0 = gpio_pg[0];
    SCUB_PGS1 = gpio_pg[1];
    SCUB_PGS2 = gpio_pg[2];
    SCUB_PGS3 = gpio_pg[3];
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
    irq_regs[0].intselect    = VIC0_INTSELECT;
    irq_regs[0].intenable    = VIC0_INTENABLE;
    irq_regs[0].softint      = VIC0_SOFTINT;
    irq_regs[0].protection   = VIC0_PROTECTION;
    irq_regs[0].prioritymask = VIC0_PRIORITYMASK;
    irq_regs[1].intselect    = VIC1_INTSELECT;
    irq_regs[1].intenable    = VIC1_INTENABLE;
    irq_regs[1].softint      = VIC1_SOFTINT;
    irq_regs[1].protection   = VIC1_PROTECTION;
    irq_regs[1].prioritymask = VIC1_PRIORITYMASK;
}

static void restore_irq_regs(void)
{
    VIC0_INTENCLEAR = 0xffffffff;
    VIC0_SOFTINTCLEAR = 0xffffffff;

    VIC0_INTSELECT    = irq_regs[0].intselect;
    VIC0_PROTECTION   = irq_regs[0].protection;
    VIC0_PRIORITYMASK = irq_regs[0].prioritymask;
    VIC0_SOFTINT      = irq_regs[0].softint;
    VIC0_INTENABLE    = irq_regs[0].intenable;
    
    VIC1_INTSELECT    = irq_regs[1].intselect;
    VIC1_PROTECTION   = irq_regs[1].protection;
    VIC1_PRIORITYMASK = irq_regs[1].prioritymask;
    VIC1_SOFTINT      = irq_regs[1].softint;
    VIC1_INTENABLE    = irq_regs[1].intenable;
}


/*
 * TIMER
 */

static timerReg_t timer_regs[5];

static void save_timer_regs(void)
{
	int i;
	
	for (i = 0; i < 3; i++) {
		timerReg_t *regs = (timerReg_t *)(LOGI_ADDR_TIMER_REG + i * LOGI_TIMER_OFFSET);
		timer_regs[i].tmCtr = regs->tmCtr;
		timer_regs[i].tmPsr = regs->tmPsr;
		timer_regs[i].tmLdrVlr = regs->tmLdrVlr;
		timer_regs[i].tmIsr = regs->tmIsr;
		timer_regs[i].tmCmp = regs->tmCmp;
	}
}

static void restore_timer_regs(void)
{
	int i;
	for (i = 0; i < 3; i++) {
		timerReg_t *regs = (timerReg_t *)(LOGI_ADDR_TIMER_REG + i * LOGI_TIMER_OFFSET);
		regs->tmCtr = timer_regs[i].tmCtr;
		regs->tmPsr = timer_regs[i].tmPsr;
		regs->tmLdrVlr = timer_regs[i].tmLdrVlr;
		regs->tmIsr = timer_regs[i].tmIsr;
		regs->tmCmp = timer_regs[i].tmCmp;
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

/*
 * WDT
 */

static wdtReg_t wdt_regs[3];

static void save_wdt_regs(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        wdtReg_t *regs = (wdtReg_t *)(LOGI_ADDR_WDT_REG +
                                i * 0x20);
        wdt_regs[i].wdtCtr = regs->wdtCtr;
        wdt_regs[i].wdtPsr = regs->wdtPsr;
        wdt_regs[i].wdtLdr = regs->wdtLdr;
        wdt_regs[i].wdtVlr = regs->wdtVlr;
        wdt_regs[i].wdtCmp = regs->wdtCmp;
    }
}

static void restore_wdt_regs(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        wdtReg_t *regs = (wdtReg_t *)(LOGI_ADDR_WDT_REG +
                                i * 0x20);
        regs->wdtCtr = wdt_regs[i].wdtCtr;
        regs->wdtPsr = wdt_regs[i].wdtPsr;
        regs->wdtLdr = wdt_regs[i].wdtLdr;
        regs->wdtVlr = wdt_regs[i].wdtVlr;
        regs->wdtCmp = wdt_regs[i].wdtCmp;
    }
}


static int speedy_snapshot(void)
{
	int ret = 0;
	int speedy_saveno = speedy_param.bootflag_area;
	unsigned int frame_buf_addr = 0, frame_buf_size = 0;
	gpStorageApiAddress_t *storage_api = (gpStorageApiAddress_t *)(LAODER_DEV_BASE);

#if 0 // milton
    if (speedy_saveno >= SPEEDY_SNAPSHOT_NUM ||
        savearea_num_check[speedy_saveno] == 0)
        return -EINVAL;
#endif

	//update speedy interface
	//@todo : not using in GP speedy boot
	*(unsigned int *)(LAODER_DEV_BASE + 0x28) = pa;	//gpStorageApiAddress_t .bufferAddr
	*(unsigned int *)(LAODER_DEV_BASE + 0x2C) = reserved_buffer_size;


	//save buffer address
	speedy_param.speedy_buf_addr = pa;
	speedy_param.speedy_buf_size = reserved_buffer_size;


	/* free memory */
	if (ka)	gp_chunk_free(ka);

	//get frame buffer address and size, if size != 0, means need reserved frame buffer
#if 0	// milton -- need implementation
	fb_get_buff_info(&frame_buf_addr, &frame_buf_size);
	if (frame_buf_size != 0) {
		speedy_chunk_save_mem(frame_buf_addr, frame_buf_size);
	}
#endif
	/* Save chunk memory */
	gp_chunk_suspend(speedy_chunk_save_mem);

	/* save srame */
	save_sram();

	/* save regs */
  save_irq_regs();
  save_timer_regs();
  //save_wdt_regs();
  save_gpio_regs();
  save_scu_regs();

#if 0
	/* set param */	
	speedy_param.bootflag_area = savearea[speedy_saveno].bootflag_area;
	speedy_param.bootflag_size = savearea[speedy_saveno].bootflag_size;
	speedy_param.snapshot_area = savearea[speedy_saveno].snapshot_area;
	speedy_param.snapshot_size = savearea[speedy_saveno].snapshot_size;
#endif

	//@todo : using for count hibernation time and boot time
	//speedy_param.snapshot_ver = tv.tv_usec;

#if 0
	speedy_param.interface_addr = UART0_BASE; //T-DEBUG debug uart : for hibdrv
#else
	speedy_param.interface_addr = LAODER_DEV_BASE; //header address
#endif

	dbg_regdump();

	speedy_printf("[%s][%d] hibdrv_snapshot() start\n",
			__func__, __LINE__);
	ret = hibdrv_snapshot();
	speedy_printf("[%s][%d] hibdrv_snapshot() end(return=%d)\n",
		__func__, __LINE__, ret);

	speedy_printf(" ===============[Start power off] (ret=%d, speedy_param.stat=%d) ===============\n",
		ret, speedy_param.stat);
	if (ret >= 0) {
	    if ( (speedy_param.stat == 0) && (speedy_param.oneshot == 1) ){
	        /* return from saving snapshot */
	        #if 0 // milton
	        spmp8050_power_off();
	        
	        //*(volatile unsigned int*)(LOGI_ADDR_SCU_A_REG + 0xF80) = 0;
	        while(1) {
						speedy_printf("[%s][%d] : wait power off\n",
						__func__, __LINE__);
					}
					
					#endif // milton       
					
	    } else {
	        /* return fast boot */
	        speedy_param.stat = 1;
	    }
	}

	speedy_printf("[%s][%d] hibernation canceled flag=%d\n",__func__, __LINE__, speedy_canceled);
    if (speedy_canceled == 1) {
    	//if hibernation was canceled, clear flag
		speedy_canceled = 0;
	}

	/* restore regs */
	speedy_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);
    restore_scu_regs();
	speedy_printf("[%s][%d] : restore_gpio_regs\n",
		__func__, __LINE__);
    restore_gpio_regs();
	speedy_printf("[%s][%d] : restore_irq_regs\n",
		__func__, __LINE__);
    restore_irq_regs();
	speedy_printf("[%s][%d] : restore_timer_regs\n",
		__func__, __LINE__);
    //restore_wdt_regs();
	speedy_printf("[%s][%d] : restore_regs done\n",
		__func__, __LINE__);

    restore_timer_regs();
	speedy_printf("[%s][%d] : restore_regs done\n",
		__func__, __LINE__);

	/* restore srame */
	restore_sram();

	dbg_regdump();

	return ret;
}

static struct speedy_ops speedy_machine_ops = {
	.drv_load       = speedy_drv_load,
	.drv_init       = speedy_drv_init,
	.device_suspend = speedy_device_suspend,
	.device_resume  = speedy_device_resume,
	.snapshot       = speedy_snapshot,
	.drv_uninit     = speedy_drv_uninit,
	//.putc           = speedy_putchar,
	.putc           = NULL,
	.progress       = speedy_process,
	//milton -- .erase_signature= gp_fastbot_signature_erase,
};

static int __init speedy_machine_init(void)
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

	return speedy_register_machine(&speedy_machine_ops);
}

static void __exit speedy_machine_exit(void)
{
	speedy_unregister_machine(&speedy_machine_ops);

	//free memory for sram reserved data
	if (sram_data_ptr) {
		kfree(sram_data_ptr);
		sram_data_ptr = NULL;
	}
}

module_init(speedy_machine_init);
module_exit(speedy_machine_exit);
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Snapshot Driver");
MODULE_LICENSE_GP;
