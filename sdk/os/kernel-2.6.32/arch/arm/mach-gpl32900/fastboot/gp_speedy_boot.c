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
 * @file    gp_speedy_boot.c
 * @brief   
 * @author  Milton Jiang
 * @date    2013-08-30
 */

#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/syscalls.h>
#include <linux/console.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/buffer_head.h>
#include <linux/blkdev.h>
#include <asm/uaccess.h>

//@roger speedy boot
#include <asm/hibernate.h>
#include <mach/gp_suspend.h>
#include <mach/cpu_pwr_down.h>
#include <mach/hardware.h>
#include <mach/gp_storage_api.h>
#include <mach/gp_speedy_boot.h>
#include "../../../../kernel/power/power.h"

#ifdef CONFIG_MTD
#include <linux/mtd/mtd.h>
#endif

#include <linux/freezer.h>
#include <asm/suspend.h>

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
//@roger 3.0.31
extern int syscore_suspend(void);
extern void syscore_resume(void);
extern void enable_dram_ioremap(int aEnable);
extern struct saved_context *pwr_saved_context;
extern asmlinkage int gp_jump_exec(void);
extern void pwr_save_processor_state(void);
extern struct saved_context *pwr_saved_context;
/**************************************************************************
 *                          C O N S T A N T S                          *
 **************************************************************************/
#ifndef SPEEDY_WORK_SIZE
#define SPEEDY_WORK_SIZE          (768 * 1024)
#endif

#define ZONETBL_DEFAULT_NUM     8
//#define EXTTBL_DEFAULT_NUM      8
#define EXTTBL_DEFAULT_NUM      32

#define SHRINK_BITE             (0xc0000000 >> PAGE_SHIFT)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) && \
    LINUX_VERSION_CODE <  KERNEL_VERSION(2,6,33)
#ifndef SPEEDY_SHRINK_REPEAT
#define SPEEDY_SHRINK_REPEAT      1
#endif
#else
#ifndef SPEEDY_SHRINK_REPEAT
#define SPEEDY_SHRINK_REPEAT      10
#endif
#ifndef SPEEDY_SHRINK_REPEAT2
#define SPEEDY_SHRINK_REPEAT2     1
#endif
#ifndef SPEEDY_SHRINK_REPEAT3
#define SPEEDY_SHRINK_REPEAT3     2
#endif
#endif


#ifndef SPEEDY_SHRINK_REPEAT_P1
#define SPEEDY_SHRINK_REPEAT_P1   100
#endif

#ifndef SPEEDY_SHRINK_THRESHOLD
#define SPEEDY_SHRINK_THRESHOLD   1
#endif

#define pm_device_suspend(x)    dpm_suspend_start(x)
#define pm_device_resume(x)     dpm_resume_end(x)
#define pm_device_power_down(x) dpm_suspend_noirq(x)
#define pm_device_power_up(x)   dpm_resume_noirq(x)

#define STATE_FREEZE    PMSG_FREEZE

#define STATE_RESTORE   (!speedy_stat ? (ret ? PMSG_RECOVER : PMSG_THAW) : \
                         PMSG_RESTORE)

#ifdef CONFIG_SWAP
#define SPEEDY_SEPARATE_MAX       2
#else
#define SPEEDY_SEPARATE_MAX       0
#endif

#ifndef speedy_pfn_valid
#define speedy_pfn_valid(pfn)     pfn_valid(pfn)
#endif

#define SPEEDY_SAVEAREA_NUM       (sizeof(speedy_savearea) / \
                                 sizeof(struct speedy_savearea))

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
int hibernate_debug = 0;


static const struct speedy_savearea speedy_savearea[] = {
    SPEEDY_SAVEAREA
};

int pm_device_down;
int speedy_shrink;
int speedy_swapout_disable;
int speedy_separate_pass;
int speedy_canceled;
speedy_info_t speedy_param;

EXPORT_SYMBOL(pm_device_down);
EXPORT_SYMBOL(speedy_shrink);
EXPORT_SYMBOL(speedy_swapout_disable);
EXPORT_SYMBOL(speedy_separate_pass);
EXPORT_SYMBOL(speedy_canceled);
EXPORT_SYMBOL(speedy_param);

EXPORT_SYMBOL(hibdrv_snapshot);
EXPORT_SYMBOL(speedy_set_savearea);
EXPORT_SYMBOL(speedy_save_cancel);
EXPORT_SYMBOL(speedy_register_machine);
EXPORT_SYMBOL(speedy_unregister_machine);

static struct speedy_ops *speedy_ops;

static int speedy_stat;
static int speedy_error;
static int speedy_retry;
static int speedy_saveno;
static int speedy_separate;

static int speedy_save_pages;
//@todo:using chunk allocate memory at work_init
static char speedy_work[SPEEDY_WORK_SIZE];
static unsigned int speedy_nosave_area, speedy_nosave_size;
static unsigned int speedy_lowmem_nosave_area, speedy_lowmem_nosave_size;
static unsigned int zonetbl_max, exttbl_max, dramtbl_max;

//static unsigned int bootloader_checksum = 0;

unsigned char *speedy_hibdrv_buf;

static unsigned int mmu_0mb = 0, mmu_1mb = 0, mmu_b00mb = 0, mmu_b10mb = 0, mmu_b20mb = 0;
static unsigned int g_ttb_phy_addr = 0;
static unsigned int g_ttb_remap_addr = 0;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
void dump_speedy_param(speedy_info_t *speedy_param);
void dump_cpu_param(struct saved_context *ctxt);
void dumpbuf(unsigned char *buf,int size);

unsigned int gp_get_checksum(unsigned char *buf, int size)
{
	int i = 0;
	unsigned int checksum = 0;
	unsigned int *ptr = (unsigned int *)buf;
	int cnt = size >>2;
	//speedy_printf("[%s][%d]buf=0x%x,cnt=0x%x,checksum=0x%x\r\n", __FUNCTION__, __LINE__, buf, cnt,checksum);
	for (i = 0 ; i < cnt ; i ++) {
		checksum += (*ptr);
		//speedy_printf ("[%s][%d]ptr=0x%x,i=%d,checksum=0x%x\r\n", __FUNCTION__, __LINE__, ptr, i,checksum);
		ptr++;
	}
	speedy_printf("[%s][%d]buf=0x%x,cnt=0x%x,checksum=0x%x\r\n", __FUNCTION__, __LINE__, buf, cnt,checksum);
	//speedy_printf ("\n\n============================= Checksum = [0x%8x]\r\n", checksum);
	return checksum;
}
EXPORT_SYMBOL(gp_get_checksum);

#if CHECKSUM_VERIFY
/**
* @brief 	Calculate checksum by table
* @param[in] 	save_table: table list
* @param[in] 	save_table_num: table number
* @param[in] 	page_shift: start/end need shift
* @param[out] 	saved_offset: total saved sectors
* @return 		checksum
*/
unsigned int gp_dram_checksum(speedy_savetbl_t *save_table, int save_table_num, int page_shift)
{
	int i, saved_byte;
	unsigned int total_checksum = 0;
	unsigned char empty_buffer[SECTOR_SIZE];
	int remain_byte = 0;

	//speedy_printf ("[%s][%d] run\n",__FUNCTION__, __LINE__);
	for (i = 0 ; i < save_table_num ; i++) {
		speedy_printf("param->%s[%d] = start = 0x%x(0x%x) , end 0x%x(0x%x)\n","table", i,
		save_table[i].start,save_table[i].start<<page_shift, save_table[i].end, save_table[i].end<<page_shift);

		saved_byte = (save_table[i].end - save_table[i].start) << page_shift;
		//check sector alignment
		remain_byte = saved_byte & (SECTOR_SIZE-1);
		saved_byte &= ~(SECTOR_SIZE-1);

		total_checksum += gp_get_checksum(__va((save_table[i].start << page_shift)), saved_byte);
		//speedy_printf ("[%s][%d] total_checksum=0x%x\n",__FUNCTION__, __LINE__,total_checksum);

		if (remain_byte != 0) {
			speedy_printf ("[%s][%d] re-calculate dram address=0x%x\n",__FUNCTION__, __LINE__,(unsigned char *)((save_table[i].start << page_shift)+saved_byte) );
			memset(empty_buffer, 0, SECTOR_SIZE);

			//handle not sector slignment data
			memcpy(empty_buffer, __va((unsigned char *)(save_table[i].start << page_shift)+saved_byte), remain_byte);

			//re-calculate checksum when size smaller than 512 bytes
			//checksum_array[i] = get_checksum(empty_buffer, SECTOR_SIZE);
			total_checksum += gp_get_checksum(empty_buffer, SECTOR_SIZE);
			speedy_printf ("[%s][%d] re-calculate total_checksum=0x%x\n",__FUNCTION__, __LINE__,total_checksum);
		}

	}
	speedy_printf ("[%s][%d] total_checksum=0x%x\n",__FUNCTION__, __LINE__,total_checksum);

	return total_checksum;
}
#endif //CHECKSUM_VERIFY

/*
 * Save the current interrupt enable state.
 */
static inline unsigned long dump_cpsr(void)
{
	unsigned long flags;
	asm volatile(
		"	mrs	%0, cpsr	@ local_save_flags"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

static inline void gp_jump_config(void)
{
	unsigned int value;
	/* Set to SVC mode */
	//asm volatile ("mcr p15, 2, %0, c0, c0, 0" : : "r"(ctxt->cssr));
	//asm volatile ("mcr p15, 5, %0, c15, c7, 2" : : "r"(ctxt->mtlbar));

	/*
	 * Go back to original SVC mode
	 */
	/*
	mrs     r3, cpsr
	bic     r3, #0x1F
	orr     r3, r3, #0x13
	msr     cpsr, r3
	*/

	asm volatile ("mrs %0, cpsr" : "=r"(value));
	value &= ~0x1F;
	value |= 0x13;
	asm volatile ("mov r3, %0" : : "r"(value));
	asm volatile ("msr cpsr_c, r3");

	//config DACR fir all memory area
	value = 0xFFFF;
	asm volatile ("mov r3, %0" : : "r"(value));
	asm volatile ("mcr p15, 0, r3, c3, c0, 0");
	//asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r"(0x1F));

	return;
}

int gp_verify_checksum(unsigned char *buf)
{
	speedy_info_t *param = (speedy_info_t *)(buf+PARAM_OFFSET);
	unsigned int current_checksumn = 0;
	//unsigned int saved_checksum = *(unsigned int *)(buf+CHECKSUM_OFFSET);
	unsigned int saved_checksum = speedy_param.checksum;
	speedy_printf("[%s][%d] Start, buf=0x%x\n",__func__, __LINE__,buf);
	speedy_printf("[%s][%d] Start, param=0x%x\n",__func__, __LINE__,param);
	speedy_printf("[%s][%d] Start, speedy_param=0x%x\n",__func__, __LINE__,saved_checksum);

	dump_speedy_param(&speedy_param);
	current_checksumn = gp_dram_checksum(speedy_param.dramtbl, speedy_param.dramtbl_num, speedy_param.page_shift);
	speedy_printf("[%s][%d] dram tabl,current_checksum=0x%x\n",__func__, __LINE__,current_checksumn);
	current_checksumn = gp_dram_checksum(speedy_param.exttbl, speedy_param.exttbl_num, 0);
	speedy_printf("[%s][%d] extt able,current_checksum=0x%x\n",__func__, __LINE__,current_checksumn);
	if (current_checksumn != saved_checksum) {
		speedy_printf ("[%s][%d] !!!!!!!!!!!!!!!!!!!! Different, current_checksumn 0x%x, saved_check = 0x%x\n",__FUNCTION__, __LINE__, current_checksumn, saved_checksum);
	}
	else {
		speedy_printf ("[%s][%d] ^^^^^^^^^^^^^^^^^^^^ Checksum match, current_checksumn 0x%x, saved_check = 0x%x\n",__FUNCTION__, __LINE__, current_checksumn, saved_checksum);
	}
	return 0;
}

int gp_mmu_hibernation_area_enable(int enable)
{
	struct saved_context *ctxt = pwr_saved_context;
	gpStorageApiAddress_t *storage_api = (gpStorageApiAddress_t *)(LAODER_DEV_START);
	int ret = 0;
	
	speedy_printf("[%s][%d] LAODER_DEV_BASE=0x%08x\n",__func__, __LINE__, LAODER_DEV_BASE);
	if(NULL == ctxt){
		speedy_printf("[%s] saved_context is NULL!!!!!!.\n", __func__);

		return -EPERM;
	}

	//clear context save area
	reset_suspend_resume_mem_addr();

	speedy_printf("[%s][%d] cpsr =0x%x\n",__func__, __LINE__, dump_cpsr());
	pwr_save_processor_state();
	dump_cpu_param(ctxt);
	//save arm context
	pwr_arch_suspend(cpu_context_save_area);

	speedy_printf("[%s][%d] ttb_0r=0x%x(0x%x)\n",__func__, __LINE__, pwr_saved_context->ttb0, (pwr_saved_context->ttb0) );
	speedy_printf("[%s][%d] ttb_0r=0x%x(0x%x)\n",__func__, __LINE__, ctxt->ttb0, (ctxt->ttb0));

	speedy_printf("[%s][%d] speedy_param.stat=%d\n",__func__, __LINE__, speedy_param.stat);
	speedy_printf("[%s][%d] g_ttb_phy_addr=0x%x, g_ttb_remap_addr=0x%x\n",__func__, __LINE__, g_ttb_phy_addr, g_ttb_remap_addr);
	//check current flow is fastboot or snapshot
	if ((speedy_param.restore != 0) || (speedy_param.stat == 1) ){
		speedy_printf("[%s][%d] cpsr =0x%x\n",__func__, __LINE__, dump_cpsr());

		speedy_printf("[%s][%d] FastBoot return\n",__func__, __LINE__);
		speedy_printf("[%s][%d] mmu_b00mb=0x%x, mmu_b10mb=0x%x, mmu_b20mb=0x%x\n", __func__, __LINE__, mmu_b00mb, mmu_b10mb, mmu_b20mb);
		
		speedy_printf("[%s][%d] mmu_0mb=0x%x, mmb_1mb=0x%x\n",__func__, __LINE__, mmu_0mb, mmu_1mb);

		*((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))) = mmu_0mb;
		*((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))) = mmu_1mb;
                *((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))) = mmu_b00mb;	// milton test
		*((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))) = mmu_b10mb;	// milton test
		*((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))) = mmu_b20mb;	// milton test
		
		speedy_printf("[%s][%d] Original ttb_0r + 0xb00(0x%x)=0x%x\n",__func__, __LINE__
		, ((volatile unsigned int *)(g_ttb_remap_addr + (0xb00<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xb00<<2))));
		speedy_printf("[%s][%d] Original ttb_0r + 0xb10(0x%x)=0x%x\n",__func__, __LINE__
		, ((volatile unsigned int *)(g_ttb_remap_addr + (0xb10<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xb10<<2))));
		speedy_printf("[%s][%d] Original ttb_0r + 0xb20(0x%x)=0x%x\n",__func__, __LINE__
		, ((volatile unsigned int *)(g_ttb_remap_addr + (0xb20<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xb20<<2))));

		//dumpbuf(g_ttb_remap_addr, 64);
		speedy_printf("[%s][%d] jiffies=0x%x\n",__func__, __LINE__, jiffies);
		speedy_printf("[%s][%d] buf=0x%x,__va(bug)=0x%x\n",__func__, __LINE__, (speedy_param.speedy_buf_addr), __va(speedy_param.speedy_buf_addr));
		//gp_verify_checksum(__va(speedy_param.speedy_buf_addr));

		speedy_printf("[%s][%d] speedy_param.stat=%d\n",__func__, __LINE__, speedy_param.stat);
		if (speedy_param.restore < 0) {
			//error or cancel happen, pass return value to speedy_param.stat
			ret = speedy_param.restore;
		}
		goto exit_point;
	}

	soc_pwr_down_cache_flush_all();
	dmb();
	dsb();

	speedy_printf("[%s][%d] cpsr =0x%x\n",__func__, __LINE__, dump_cpsr());
	enable_dram_ioremap(1);
	g_ttb_phy_addr = ctxt->ttb0 & 0xffffc000;;
	g_ttb_remap_addr = (int) ioremap(g_ttb_phy_addr, 0x4000);
	
	speedy_printf("[%s][%d] g_ttb_phy_addr=0x%x, g_ttb_remap_addr=0x%x\n",__func__, __LINE__, g_ttb_phy_addr, g_ttb_remap_addr);

#if 1	// milton test

	//save MMU table entry of 0x0-0x200000
	mmu_0mb = *((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2)));
	mmu_1mb = *((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2)));

	speedy_printf("[%s][%d] mmu_0mb=0x%x, mmb_1mb=0x%x\n",__func__, __LINE__, mmu_0mb, mmu_1mb);
	speedy_printf("[%s][%d] Original ttb_0r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))));
	speedy_printf("[%s][%d] Original ttb_0r + 0x1(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))));
	//dumpbuf(g_ttb_remap_addr, 512);

	//mapping 0x0 to physical 0x0
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))) &= 0x000FFFFF;
	//*((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))) |= 0x000000;

	//mapping 0x100000 to physical 0x100000
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))) &= 0x000FFFFF;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))) |= 0x100000;

	//change to section mmu table format at area 0x0-0x1fffff
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))) &= ~0x3;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))) |= 0x2;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))) &= ~0x3;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))) |= 0x2;

	*((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))) &= ~0xC00;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))) |= 0x400;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))) &= ~0xC00;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))) |= 0x400;
	
	//dumpbuf(pwr_saved_context->ttb_0r|0xc0000000, 512);
	//g_ttb_sram_entry_val = (*((volatile unsigned int *)(g_ttb_remap_addr + (0x907<<2))));
	speedy_printf("[%s][%d] change content ttb_0r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0x0<<2))));
	speedy_printf("[%s][%d] change content ttb_0r + 0x1(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))));

	//speedy_printf("[%s][%d] change to section ttb_0r + 0x1(0x%x)=0x%x\n",__func__, __LINE__
	//, ((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0x1<<2))));

	// milton test -----------------------------------------------------------------------------
	
	//save MMU table entry of 0xB0000000
	mmu_b00mb = *((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2)));
	
	speedy_printf("[%s][%d] mmu_b00mb=0x%x\n",__func__, __LINE__, mmu_b00mb);
	speedy_printf("[%s][%d] Original ttb_b00r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))));
	
	//mapping 0xB0000000 to physical 0xB0000000
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))) &= 0x000FFFFF;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))) |= 0xB0000432;

	//change to section mmu table format at area 0xB0000000
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))) &= ~0x3;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))) |= 0x2;
	
	speedy_printf("[%s][%d] change content ttb_b00r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xB00<<2))));
	
	// -----------------------------------------------------------------------------------------
	
	//save MMU table entry of 0xB1000000
	mmu_b10mb = *((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2)));
	
	speedy_printf("[%s][%d] mmu_b10mb=0x%x\n",__func__, __LINE__, mmu_b10mb);
	speedy_printf("[%s][%d] Original ttb_b10r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))));
	
	//mapping 0xB1000000 to physical 0xB1000000
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))) &= 0x000FFFFF;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))) |= 0xB1000432;

	//change to section mmu table format at area 0xB1000000
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))) &= ~0x3;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))) |= 0x2;
	
	speedy_printf("[%s][%d] change content ttb_b10r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xB10<<2))));
	
	// -----------------------------------------------------------------------------------------
	
	//save MMU table entry of 0xB2000000
	mmu_b20mb = *((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2)));
	
	speedy_printf("[%s][%d] mmu_b20mb=0x%x\n",__func__, __LINE__, mmu_b20mb);
	speedy_printf("[%s][%d] Original ttb_b20r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))));
	
	//mapping 0xB2000000 to physical 0xB2000000
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))) &= 0x000FFFFF;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))) |= 0xB2000432;

	//change to section mmu table format at area 0xB2000000
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))) &= ~0x3;
	*((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))) |= 0x2;
	
	speedy_printf("[%s][%d] change content ttb_b20r + 0x0(0x%x)=0x%x\n",__func__, __LINE__
	, ((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))), *((volatile unsigned int *)(g_ttb_remap_addr + (0xB20<<2))));
	
	// -----------------------------------------------------------------------------------------
#endif	

	//dumpbuf(cpu_context_save_area, 128);
	//dumpbuf(pwr_saved_context, 128);

	//dumpbuf(0x100000, 128);
	//dumpbuf(0xc00f0000, 128);
	//dumpbuf(0xc00f8000, 128);
	//speedy_printf("[%s][%d] storage_api=0x%x\n",__func__, __LINE__, storage_api);
	//dumpbuf(0xf8000, 128);
	//speedy_printf("[%s][%d] dump parameter info\n",__func__, __LINE__);
	//dumpbuf(&speedy_param, sizeof(speedy_param));
	speedy_printf("[%s][%d] gp speedy boot jump to bootloader : 0x%x, 0x%x\n",__func__, __LINE__, storage_api->gpSpeedyEntry, *(unsigned int*)(storage_api->gpSpeedyEntry));

	__cpuc_flush_kern_all();
	dmb();
	dsb();

	//gp_jump_to_save_image();
    //gp_jump_exec();
    gp_jump_config();    

  ret = storage_api->gpSpeedyEntry(&speedy_param);

	speedy_printf("[%s][%d] jump back from bootloader\n",__func__, __LINE__);

exit_point:
	soc_pwr_down_cache_flush_all();
	dmb();
	dsb();

	//disable dram remap
	enable_dram_ioremap(0);
	iounmap((void *)g_ttb_remap_addr);

	speedy_printf("[%s][%d] preempt enable.\n", __func__, __LINE__);
	//@todo : remove to out of hibdrv_snapshot() function
	printk("[%s][%d] preempt enable.\n", __func__,__LINE__);
	preempt_enable();
	return ret;

}

extern const void __nosave_begin, __nosave_end;

void swsusp_show_speed(struct timeval *start, struct timeval *stop,
			unsigned nr_pages, char *msg)
{

}

int pfn_is_nosave(unsigned long pfn)
{
    unsigned long nosave_begin_pfn, nosave_end_pfn;

    nosave_begin_pfn = __pa(&__nosave_begin) >> PAGE_SHIFT;
    nosave_end_pfn = PAGE_ALIGN(__pa(&__nosave_end)) >> PAGE_SHIFT;
    return (pfn >= nosave_begin_pfn) && (pfn < nosave_end_pfn);
}

bool system_entering_hibernation(void)
{
    return pm_device_down != SPEEDY_STATE_NORMAL;
}

EXPORT_SYMBOL(system_entering_hibernation);

#ifdef CONFIG_PM_SPEEDY_DEBUG

void speedy_putc(char c)
{
    if (speedy_ops->putc) {
        if (c == '\n')
            speedy_ops->putc('\r');
        speedy_ops->putc(c);
    }
}

int speedy_printf(const char *fmt, ...)
{
    int i, len;
    va_list args;
    char buf[256];

	///using silence to dynamic enable debug message
	if (speedy_param.silent == 0)
		return 0;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    for (i = 0; i < len; i++)
        speedy_putc(buf[i]);
    return len;
}

EXPORT_SYMBOL(speedy_putc);
EXPORT_SYMBOL(speedy_printf);

#endif

#if 0 //CONFIG_MTD

static int speedy_mtd(struct mtd_info *mtd, void *buf, size_t size)
{
    int ret;
    size_t read_size;
    loff_t offs = SPEEDY_HIBDRV_OFFSET;

    if (IS_ERR(mtd)) {
        printk("Can't find hibernation driver partition.\n");
        return PTR_ERR(mtd);
    }

#ifdef SPEEDY_HIBDRV_AREA_SIZE
    ret = -EIO;
    while (offs < SPEEDY_HIBDRV_OFFSET + SPEEDY_HIBDRV_AREA_SIZE) {
        if (!mtd->block_isbad || !mtd->block_isbad(mtd, offs)) {
            ret = mtd->read(mtd, offs, PAGE_SIZE, &read_size, buf);
            break;
        }
        offs += mtd->erasesize;
    }
#else
    ret = mtd->read(mtd, offs, PAGE_SIZE, &read_size, buf);
#endif
    if (ret >= 0) {
        if (size <= SPEEDY_DRV_COPY_SIZE) {
            ret = -ENOMEM;
        } else {
            if (SPEEDY_DRV_COPY_SIZE <= PAGE_SIZE ||
                (ret = mtd->read(mtd, offs + PAGE_SIZE,
                                 SPEEDY_DRV_COPY_SIZE - PAGE_SIZE, &read_size,
                                 buf + PAGE_SIZE)) >= 0) {
                flush_cache_all();
                put_mtd_device(mtd);
                return 0;
            }
        }
    }

    printk("Can't load hibernation driver.\n");
    put_mtd_device(mtd);
    return ret;
}

int speedy_mtd_load(int mtdno, void *buf, size_t size)
{
    return speedy_mtd(get_mtd_device(NULL, mtdno), buf, size);
}

EXPORT_SYMBOL(speedy_mtd_load);

int speedy_mtd_load_nm(const char *mtdname, void *buf, size_t size)
{
    return speedy_mtd(get_mtd_device_nm(mtdname), buf, size);
}

EXPORT_SYMBOL(speedy_mtd_load_nm);

#endif //CONFIG_MTD

#if 0
int speedy_dev_load(const char *dev, void *buf, size_t size)
{
    int ret = 0;
    struct file *f;

    f = filp_open(dev, O_RDONLY, 0777);
    if (IS_ERR(f)) {
        printk("Can't open hibernation driver device.\n");
        return PTR_ERR(f);
    }

    if ((ret = kernel_read(f, SPEEDY_HIBDRV_OFFSET, buf, PAGE_SIZE)) >= 0) {
        if (SPEEDY_ID != SPEEDY_ID_DRIVER) {
            printk("[%s][%d]Can't find hibernation driver.\n",__FUNCTION__, __LINE__);
            return -EIO;
        }
        if (size <= SPEEDY_DRV_COPY_SIZE) {
            ret = -ENOMEM;
        } else {
            if (SPEEDY_DRV_COPY_SIZE <= PAGE_SIZE ||
                (ret = kernel_read(f, SPEEDY_HIBDRV_OFFSET + PAGE_SIZE,
                                   buf + PAGE_SIZE,
                                   SPEEDY_DRV_COPY_SIZE - PAGE_SIZE)) >= 0) {
                filp_close(f, NULL);
                flush_cache_all();
                return 0;
            }
        }
    }

    filp_close(f, NULL);

    printk("Can't load hibernation driver.\n");
    return ret;
}

EXPORT_SYMBOL(speedy_dev_load);
#endif

static int speedy_work_init(void)
{
    int ret;
    int work_size, table_size;
    speedy_savetbl_t *savetbl;

    work_size = SPEEDY_WORK_SIZE;

    speedy_hibdrv_buf = speedy_work;
    BUG_ON(!speedy_ops->drv_load);
    if ((ret = speedy_ops->drv_load(speedy_work, work_size)) < 0) {
        return ret;
    }
    //@todo:check nothing use about SPEEDY_DRV_COPY_SIZE in savetbl
    savetbl = (speedy_savetbl_t *)(speedy_work);
    table_size = work_size;

    zonetbl_max = ZONETBL_DEFAULT_NUM;
    exttbl_max = EXTTBL_DEFAULT_NUM;
    dramtbl_max = table_size / sizeof(speedy_savetbl_t) - ZONETBL_DEFAULT_NUM - EXTTBL_DEFAULT_NUM;
    printk("[%s][%d] dramtbl_max=%d, exttbl_max=%d\n",__FUNCTION__, __LINE__,dramtbl_max, exttbl_max);

    speedy_param.zonetbl = savetbl + dramtbl_max + exttbl_max;
    speedy_param.exttbl = savetbl + dramtbl_max;
    speedy_param.dramtbl = savetbl;
    speedy_param.zonetbl_num = 0;
    speedy_param.exttbl_num = 0;
    speedy_param.dramtbl_num = 0;
    speedy_nosave_area = (unsigned long)-1;
    speedy_nosave_size = 0;
    speedy_lowmem_nosave_area = (unsigned long)-1;
    speedy_lowmem_nosave_size = 0;
    speedy_param.maxarea = 0;
    speedy_param.maxsize = 0;
    speedy_param.lowmem_maxarea = 0;
    speedy_param.lowmem_maxsize = 0;
    speedy_param.speedy_buf_addr = 0;
    speedy_param.speedy_buf_size = 0;
    speedy_param.start_us = 0;
    speedy_save_pages = 0;
    speedy_printf("[%s][%d] speedy_param->%s = 0x%x,0x%x\n",__FUNCTION__, __LINE__,"zonetbl", speedy_param.zonetbl,&speedy_param.zonetbl);
    speedy_printf("[%s][%d] speedy_param->%s = 0x%x,0x%x\n",__FUNCTION__, __LINE__,"exttbl ", speedy_param.exttbl ,&speedy_param.exttbl );
    speedy_printf("[%s][%d] speedy_param->%s = 0x%x,0x%x\n",__FUNCTION__, __LINE__,"dramtbl", speedy_param.dramtbl,&speedy_param.dramtbl);
    speedy_printf("[%s][%d] speedy_work = 0x%x\n",__FUNCTION__, __LINE__, speedy_work);
    
    speedy_param.total_saved_size = 0;
    speedy_param.compress_size = 0;
    speedy_param.stat = 0;
    speedy_param.restore = 0;
    
    dump_speedy_param(&speedy_param);

    return 0;
}

static void speedy_work_free(void)
{
    return;
}

static void speedy_set_tbl(unsigned long start, unsigned long end,
                         speedy_savetbl_t *tbl, int *num)
{
    if (*num > 0 && start == tbl[*num - 1].end) {
		//speedy_printf("[%s][%d] 1 start = 0x%x(0x%x),end = 0x%x(0x%x)\n", __FUNCTION__, __LINE__,
		//start, tbl[*num].start, end, tbl[*num-1].end);
        tbl[*num - 1].end = end;

    } else if (start < end) {
		//speedy_printf("[%s][%d] 2 start = 0x%x(0x%x),end = 0x%x(0x%x)\n", __FUNCTION__, __LINE__,
		//start, tbl[*num].start, end, tbl[*num].end);
        tbl[*num].start = start;
        tbl[*num].end = end;
        (*num)++;
    }
}

int speedy_set_savearea(unsigned long start, unsigned long end)
{
    speedy_savetbl_t *tbl;

    if (speedy_param.exttbl_num >= exttbl_max) {
        if (speedy_param.dramtbl_num + EXTTBL_DEFAULT_NUM > dramtbl_max) {
            printk("[speedy_set_savearea]speedy: save table overflow,speedy_param.exttbl_num=%d,dramtbl_max=%d\n",speedy_param.exttbl_num,dramtbl_max);
            return -ENOMEM;
        }
        tbl = speedy_param.exttbl - EXTTBL_DEFAULT_NUM;
        memmove(tbl, speedy_param.exttbl,
                exttbl_max * sizeof(speedy_savetbl_t));
        speedy_param.exttbl = tbl;
        exttbl_max += EXTTBL_DEFAULT_NUM;
    }
	//printk("[speedy_set_savearea]	: save table info, speedy_param.exttbl_num=%d,exttbl_max=%d\n",speedy_param.exttbl_num,exttbl_max);
	printk("[%s][%d] start = 0x%x(0x%x),end = 0x%x(0x%x)\n", __FUNCTION__, __LINE__, (unsigned int)start, (unsigned int)start, (unsigned int)end, (unsigned int)end);
	speedy_param.total_saved_size += (end - start);
    speedy_set_tbl(start, end, speedy_param.exttbl, &speedy_param.exttbl_num);
    return 0;
}

static int speedy_set_save_zone(unsigned long pfn)
{
    speedy_savetbl_t *tbl;

    if (speedy_param.zonetbl_num >= zonetbl_max) {
        if (speedy_param.dramtbl_num + EXTTBL_DEFAULT_NUM > dramtbl_max) {
            printk("[speedy_set_save_zone]speedy: save table overflow,speedy_param.exttbl_num=%d,dramtbl_max=%d\n",speedy_param.exttbl_num,dramtbl_max);
            return -ENOMEM;
        }
		speedy_printf("[%s][%d] tbl = 0x%x\n",__FUNCTION__, tbl);
		speedy_printf("[%s][%d] speedy_param.exttbl = 0x%x\n",__FUNCTION__, speedy_param.exttbl);
        tbl = speedy_param.exttbl - ZONETBL_DEFAULT_NUM;
        memmove(tbl, speedy_param.exttbl,
                (exttbl_max + zonetbl_max) * sizeof(speedy_savetbl_t));
        speedy_param.exttbl = tbl;
        speedy_param.zonetbl -= ZONETBL_DEFAULT_NUM;
        zonetbl_max += ZONETBL_DEFAULT_NUM;
		speedy_printf("[%s][%d] speedy_param->%s = 0x%x,0x%x\n",__FUNCTION__, __LINE__,"zonetbl", speedy_param.zonetbl,&speedy_param.zonetbl);
		speedy_printf("[%s][%d] speedy_param->%s = 0x%x,0x%x\n",__FUNCTION__, __LINE__,"exttbl ", speedy_param.exttbl ,&speedy_param.exttbl );
		speedy_printf("[%s][%d] speedy_param->%s = 0x%x,0x%x\n",__FUNCTION__, __LINE__,"dramtbl", speedy_param.dramtbl,&speedy_param.dramtbl);
    }

	//speedy_printf(" before speedy_param->%s[%d] = 0x%x(0x%x),end = 0x%x(0x%x)\n","zonetbl->start",speedy_param.zonetbl_num,
	//speedy_param.zonetbl[speedy_param.zonetbl_num].start,speedy_param.zonetbl[speedy_param.zonetbl_num].start<<PAGE_SHIFT,speedy_param.zonetbl[speedy_param.zonetbl_num].end,speedy_param.zonetbl[speedy_param.zonetbl_num].end<<PAGE_SHIFT);
	//speedy_printf("[%s][%d] start = 0x%x(0x%x),end = 0x%x(0x%x)\n", __FUNCTION__, __LINE__, pfn,  pfn + 1);
	//speedy_param.total_saved_size += (1 << PAGE_SHIFT);
    speedy_set_tbl(pfn, pfn + 1, speedy_param.zonetbl, &speedy_param.zonetbl_num);
    //if (speedy_param.zonetbl_num != 0)
	//	speedy_printf(" after  speedy_param->%s[%d] = 0x%x(0x%x),end = 0x%x(0x%x)\n","zonetbl->start",speedy_param.zonetbl_num,
	//	speedy_param.zonetbl[speedy_param.zonetbl_num-1].start,speedy_param.zonetbl[speedy_param.zonetbl_num-1].start<<PAGE_SHIFT,speedy_param.zonetbl[speedy_param.zonetbl_num-1].end,speedy_param.zonetbl[speedy_param.zonetbl_num-1].end<<PAGE_SHIFT);
	//else
	//	speedy_printf(" after  speedy_param->%s[%d] = 0x%x(0x%x),end = 0x%x(0x%x)\n","zonetbl->start",speedy_param.zonetbl_num,
	//	speedy_param.zonetbl[speedy_param.zonetbl_num].start,speedy_param.zonetbl[speedy_param.zonetbl_num].start<<PAGE_SHIFT,speedy_param.zonetbl[speedy_param.zonetbl_num].end,speedy_param.zonetbl[speedy_param.zonetbl_num].end<<PAGE_SHIFT);
    return 0;
}

static int speedy_set_save_dram(struct zone *zone, unsigned long pfn)
{
    if (speedy_param.dramtbl_num >= dramtbl_max) {
        printk("[speedy_set_save_dram]speedy: save table overflow,speedy_param.dramtbl_num=%d,dramtbl_max=%d\n",speedy_param.dramtbl_num,dramtbl_max);
        return -ENOMEM;
    }
	//speedy_printf(" before speedy_param->%s[%d] = 0x%x,end = 0x%x\n","dramtbl->start",speedy_param.dramtbl_num,speedy_param.dramtbl[speedy_param.dramtbl_num].start,speedy_param.dramtbl[speedy_param.dramtbl_num].end);
	//speedy_printf("[%s][%d] start = 0x%x(0x%x),end = 0x%x(0x%x)\n", __FUNCTION__, __LINE__, pfn,  pfn + 1);
	speedy_param.total_saved_size += (1 << PAGE_SHIFT);
    speedy_set_tbl(pfn, pfn + 1, speedy_param.dramtbl, &speedy_param.dramtbl_num);
	//speedy_printf(" after  speedy_param->%s[%d] = 0x%x,end = 0x%x\n","dramtbl->start",speedy_param.dramtbl_num,speedy_param.dramtbl[speedy_param.dramtbl_num].start,speedy_param.dramtbl[speedy_param.dramtbl_num].end);
    return 0;
}

static void speedy_set_nosave_dram(struct zone *zone, unsigned long pfn)
{
	//speedy_printf("???????? [%s][%d] pfn=%d\n",__FUNCTION__, __LINE__,pfn);
    if (pfn == speedy_nosave_area + speedy_nosave_size) {
        speedy_nosave_size++;
        if (speedy_param.maxsize < speedy_nosave_size) {
            speedy_param.maxarea = speedy_nosave_area;
            speedy_param.maxsize = speedy_nosave_size;
        }
    } else {
        speedy_nosave_area = pfn;
        speedy_nosave_size = 1;
    }
    if (!is_highmem(zone)) {
        if (pfn == speedy_lowmem_nosave_area + speedy_lowmem_nosave_size) {
            speedy_lowmem_nosave_size++;
            if (speedy_param.lowmem_maxsize < speedy_lowmem_nosave_size) {
                speedy_param.lowmem_maxarea = speedy_lowmem_nosave_area;
                speedy_param.lowmem_maxsize = speedy_lowmem_nosave_size;
            }
        } else {
            speedy_lowmem_nosave_area = pfn;
            speedy_lowmem_nosave_size = 1;
        }
    }
}

static int speedy_make_save_table(void)
{
    int ret;
    struct zone *zone;
    unsigned long pfn, end;

    for_each_zone (zone) {
        mark_free_pages(zone);
        end = zone->zone_start_pfn + zone->spanned_pages;
        for (pfn = zone->zone_start_pfn; pfn < end; pfn++) {
            if (!speedy_pfn_valid(pfn) ||
                swsusp_page_is_forbidden(pfn_to_page(pfn))
                )
                continue;
			//speedy_printf("[%s][%d] pfn = 0x%x, zone=0x%x\n",__FUNCTION__, __LINE__, pfn, zone);
            if ((ret = speedy_set_save_zone(pfn)) < 0)
                return ret;
            if (speedy_param.mode != 1 || swsusp_page_is_saveable(zone, pfn)) {
                speedy_save_pages++;
                if ((ret = speedy_set_save_dram(zone, pfn)) < 0)
                    return ret;
            } else {
                speedy_set_nosave_dram(zone, pfn);
            }
        }
    }
    return 0;
}

#if 1 /* memory debug function for debug */
#define DUMP_PRINT    speedy_printf
#define DUMPLIST    16
void dumpbuf(unsigned char *buf,int size)
{
    int i,j;
    int round;
    unsigned char *ptr = buf;

    DUMP_PRINT(" buf=%x,size=%d\n",(unsigned int)buf,size);
    if ((size&(DUMPLIST-1)) != 0)
        round = size/DUMPLIST+1;
    else
        round = size/DUMPLIST;

    /* DUMP_PRINT("ptr(%x)=%2x\n",ptr,*ptr++); */
    DUMP_PRINT("EXEC round=%d\n",round);
#if 1
    for (j=0;j < round;j++)
    {
        DUMP_PRINT(" %3x | ",j);
        for(i=0; i< DUMPLIST;i++)
            DUMP_PRINT("%02x;",*ptr++);
        DUMP_PRINT("\n");
    }
#endif
}
#endif
EXPORT_SYMBOL(dumpbuf);

void dump_speedy_param(speedy_info_t *speedy_param)
{
	speedy_printf(" ==================== Dump speedy_param ====================\n");
	speedy_printf(" speedy_param->%s = %d\n","start_us      ",speedy_param->start_us      );
	speedy_printf(" speedy_param->%s = %d\n","mode          ",speedy_param->mode          );
	speedy_printf(" speedy_param->%s = %d\n","compress      ",speedy_param->compress      );
	speedy_printf(" speedy_param->%s = %d\n","oneshot       ",speedy_param->oneshot       );
	speedy_printf(" speedy_param->%s = %d\n","halt          ",speedy_param->halt          );
	speedy_printf(" speedy_param->%s = %d\n","bootflag_dev  ",speedy_param->bootflag_dev  );
	speedy_printf(" speedy_param->%s = %d\n","bootflag_area ",speedy_param->bootflag_area );
	speedy_printf(" speedy_param->%s = %d\n","bootflag_size ",speedy_param->bootflag_size );
	speedy_printf(" speedy_param->%s = %d\n","snapshot_dev  ",speedy_param->snapshot_dev  );
	speedy_printf(" speedy_param->%s = %d\n","snapshot_area ",speedy_param->snapshot_area );
	speedy_printf(" speedy_param->%s = %d\n","snapshot_size ",speedy_param->snapshot_size );
	speedy_printf(" speedy_param->%s = %d\n","lowmem_size   ",speedy_param->lowmem_size   );
	speedy_printf(" speedy_param->%s = %d\n","page_shift    ",speedy_param->page_shift    );
	speedy_printf(" speedy_param->%s = %d\n","zonetbl_num   ",speedy_param->zonetbl_num   );
	speedy_printf(" speedy_param->%s = %d\n","exttbl_num    ",speedy_param->exttbl_num    );
	speedy_printf(" speedy_param->%s = %d\n","dramtbl_num   ",speedy_param->dramtbl_num   );
	speedy_printf(" speedy_param->%s = %d\n","preload_exttbl",speedy_param->preload_exttbl);
	speedy_printf(" speedy_param->%s = %d\n","maxarea       ",speedy_param->maxarea       );
	speedy_printf(" speedy_param->%s = %d\n","maxsize       ",speedy_param->maxsize       );
	speedy_printf(" speedy_param->%s = %d\n","lowmem_maxarea",speedy_param->lowmem_maxarea);
	speedy_printf(" speedy_param->%s = %d\n","lowmem_maxsize",speedy_param->lowmem_maxsize);
	speedy_printf(" speedy_param->%s = %d\n","silent        ",speedy_param->silent        );
	speedy_printf(" speedy_param->%s = %d\n","interface_addr",speedy_param->interface_addr);
	speedy_printf(" speedy_param->%s = %d\n","checksum      ",speedy_param->checksum      );
	speedy_printf(" speedy_param->%s = %d\n","stat          ",speedy_param->stat          );
	speedy_printf(" speedy_param->%s = %d\n","retry         ",speedy_param->retry         );
	speedy_printf(" speedy_param->%s = %d\n","erase_sig     ",speedy_param->erase_sig     );
	speedy_printf(" speedy_param->%s = 0x%x\n","v2p_offset    ",speedy_param->v2p_offset    );
	speedy_printf(" speedy_param->%s = 0x%x\n","zonetbl->start",speedy_param->zonetbl->start);
	speedy_printf(" speedy_param->%s = 0x%x\n","zonetbl->end",speedy_param->zonetbl->end);
	speedy_printf(" speedy_param->%s = 0x%x\n","exttbl->start ",speedy_param->exttbl->start);
	speedy_printf(" speedy_param->%s = 0x%x\n","exttbl->end ",speedy_param->exttbl->end);
	speedy_printf(" speedy_param->%s = 0x%x\n","dramtbl->start",speedy_param->dramtbl->start);
	speedy_printf(" speedy_param->%s = 0x%x\n","dramtbl->end",speedy_param->dramtbl->end);
	speedy_printf(" speedy_param->%s = 0x%x\n","speedy_buf_addr",speedy_param->speedy_buf_addr);
	speedy_printf(" speedy_param->%s = 0x%x\n","speedy_buf_size",speedy_param->speedy_buf_size);
	speedy_printf(" ==================== Dump speedy_param ====================\n");
}

void dump_cpu_param(struct saved_context *ctxt)
{
	speedy_printf(" ==================== Dump ctxt ====================\n");
	speedy_printf(" ctxt->%s = 0x%08x\n",  "cr        ",ctxt->cr            );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "cacr      ",ctxt->cacr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "ttb0      ",ctxt->ttb0          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "ttb1      ",ctxt->ttb1          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "ttbcr     ",ctxt->ttbcr         );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "dacr      ",ctxt->dacr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "dfsr      ",ctxt->dfsr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "ifsr      ",ctxt->ifsr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "dfar      ",ctxt->dfar          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "wfar      ",ctxt->wfar          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "ifar      ",ctxt->ifar          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "dclr      ",ctxt->dclr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "iclr      ",ctxt->iclr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "dtcmr     ",ctxt->dtcmr         );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "itcmr     ",ctxt->itcmr         );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "tcmsel    ",ctxt->tcmsel        );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "cbor      ",ctxt->cbor          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "tlblr     ",ctxt->tlblr         );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "prrr      ",ctxt->prrr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "nrrr      ",ctxt->nrrr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "snsvbar   ",ctxt->snsvbar       );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "mvbar     ",ctxt->mvbar         );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "fcse      ",ctxt->fcse          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "cid       ",ctxt->cid           );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "urwtpid   ",ctxt->urwtpid       );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "urotpid   ",ctxt->urotpid       );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "potpid    ",ctxt->potpid        );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "pmrr      ",ctxt->pmrr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "pmcr      ",ctxt->pmcr          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "pmcc      ",ctxt->pmcc          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "pmc0      ",ctxt->pmc0          );
	speedy_printf(" ctxt->%s = 0x%08x\n",  "pmc1      ",ctxt->pmc1          );
	speedy_printf(" ==================== Dump ctxt ====================\n");
}

//@roger 3.0.31
#define SPEEDY_PRINT_MEMINFO
#ifdef SPEEDY_PRINT_MEMINFO

#define K(x) ((x) << (PAGE_SHIFT - 10))

static void speedy_print_meminfo(void)
{
    struct sysinfo si;
    unsigned long buffers, cached, dirty, mapped;

    dirty = global_page_state(NR_FILE_DIRTY);
    mapped = global_page_state(NR_FILE_MAPPED);
    cached = global_page_state(NR_FILE_PAGES);

    buffers = nr_blockdev_pages();
    cached -= total_swapcache_pages + buffers;
    si_swapinfo(&si);

    printk("Buffers        :%8lu KB\n"
           "Cached         :%8lu KB\n"
           "SwapCached     :%8lu KB\n"
           "SwapUsed       :%8lu KB\n",
           K(buffers),
           K(cached),
           K(total_swapcache_pages),
           K(si.totalswap - si.freeswap));
    printk("Active(anon)   :%8lu KB\n"
           "Inactive(anon) :%8lu KB\n"
           "Active(file)   :%8lu KB\n"
           "Inactive(file) :%8lu KB\n",
           K(global_page_state(NR_ACTIVE_ANON)),
           K(global_page_state(NR_INACTIVE_ANON)),
           K(global_page_state(NR_ACTIVE_FILE)),
           K(global_page_state(NR_INACTIVE_FILE)));
    printk("AnonPages      :%8lu KB\n",
           K(global_page_state(NR_ANON_PAGES)));
    printk("Dirty          :%8lu KB\n"
           "Mapped         :%8lu KB\n",
           K(dirty),
           K(mapped));
    printk("SReclaimable   :%8lu kB\n"
           "SUnreclaim     :%8lu kB\n"
           "PageTables     :%8lu kB\n",
           K(global_page_state(NR_SLAB_RECLAIMABLE)),
           K(global_page_state(NR_SLAB_UNRECLAIMABLE)),
           K(global_page_state(NR_PAGETABLE)));
}

#else

#define speedy_print_meminfo()

#endif

static int speedy_shrink_memory(void)
{
    int i, pages;
    int shrink_sav = speedy_shrink;
    int repeat = SPEEDY_SHRINK_REPEAT;

#ifdef CONFIG_SWAP
    if (!speedy_swapout_disable) {
        speedy_shrink = SPEEDY_SHRINK_ALL;
        repeat = SPEEDY_SHRINK_REPEAT_P1;
    }
#endif
	printk("[%s][%d] speedy_shrink=%d\n", __FUNCTION__, __LINE__, speedy_shrink);
    if (speedy_shrink != SPEEDY_SHRINK_NONE) {
        speedy_print_meminfo();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
        if (speedy_shrink == SPEEDY_SHRINK_LIMIT1)
            repeat = SPEEDY_SHRINK_REPEAT2;
        else if (speedy_shrink == SPEEDY_SHRINK_LIMIT2)
            repeat = SPEEDY_SHRINK_REPEAT3;
#endif
        for (i = 0; i < repeat; i++) {
            printk("Shrinking memory...  ");
            pages = shrink_all_memory(SHRINK_BITE);
            printk("\bdone (%d pages freed)\n", pages);
            if (pages <= SPEEDY_SHRINK_THRESHOLD)
                break;
        }
    }
    speedy_print_meminfo();

    speedy_shrink = shrink_sav;
    return 0;
}

int hibdrv_snapshot(void)
{
    int ret;
    //unsigned char *temp;
    //unsigned char *gpfb_signature = "GPFB";
    //unsigned int current_checksum;
    //unsigned int *pMem =0 ;

	speedy_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);

    drain_local_pages(NULL);

	speedy_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);
    
    if ((ret = speedy_make_save_table()) < 0)
        return ret;
      
	speedy_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);

#if 0
    printk("dram save %d pages\n", speedy_save_pages);
    printk("maxarea 0x%08lx(0x%08lx)  lowmem_maxarea 0x%08lx(0x%08lx)\n",
           speedy_param.maxarea << PAGE_SHIFT,
           speedy_param.maxsize << PAGE_SHIFT,
           speedy_param.lowmem_maxarea << PAGE_SHIFT,
           speedy_param.lowmem_maxsize << PAGE_SHIFT);
    printk("zonetbl %d  exttbl %d  dramtbl %d\n", speedy_param.zonetbl_num,
           speedy_param.exttbl_num, speedy_param.dramtbl_num);
#else   //T-DEBUG
    speedy_printf("dram save %d pages\n", speedy_save_pages);
	speedy_printf("maxarea 0x%08lx(0x%08lx)  lowmem_maxarea 0x%08lx(0x%08lx)\n",
           speedy_param.maxarea << PAGE_SHIFT,
           speedy_param.maxsize << PAGE_SHIFT,
           speedy_param.lowmem_maxarea << PAGE_SHIFT,
           speedy_param.lowmem_maxsize << PAGE_SHIFT);
    speedy_printf("zonetbl %d  exttbl %d  dramtbl %d\n", speedy_param.zonetbl_num,
           speedy_param.exttbl_num, speedy_param.dramtbl_num);
    printk("dram save %d pages\n", speedy_save_pages);
	printk("maxarea 0x%x(0x%x)  lowmem_maxarea 0x%x(0x%x)\n",
           speedy_param.maxarea << PAGE_SHIFT,
           speedy_param.maxsize << PAGE_SHIFT,
           speedy_param.lowmem_maxarea << PAGE_SHIFT,
           speedy_param.lowmem_maxsize << PAGE_SHIFT);
    printk("zonetbl %d  exttbl %d  dramtbl %d\n", speedy_param.zonetbl_num,
           speedy_param.exttbl_num, speedy_param.dramtbl_num);
#endif

	speedy_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);
    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_SAVE);

#if 0 //dump memory info
	speedy_printf("[%s][%d] : Dump speedy_param before save\n",
		__func__, __LINE__);
	dump_speedy_param(&speedy_param);
    if (speedy_param.zonetbl_num == 0) {
		speedy_printf("speedy_param->%s[%d] = start = 0x%x , end 0x%x\n","zonetbl", 0,
			speedy_param.zonetbl[0].start, speedy_param.zonetbl[0].end);
    }
    else {
    	int i;
    	for (i = 0 ; i < speedy_param.zonetbl_num ; i++) {
			speedy_printf("speedy_param->%s[%d] = start = 0x%x(0x%x) , end 0x%x(0x%x)\n","zonetbl", i,
				speedy_param.zonetbl[i].start,speedy_param.zonetbl[i].start<<PAGE_SHIFT, speedy_param.zonetbl[i].end, speedy_param.zonetbl[i].end<<PAGE_SHIFT);
#if 0
			//add GPFB signature for confirm
			temp = (speedy_param.zonetbl[i].start << PAGE_SHIFT) + 0xc0000000 + 512-16;
			memcpy(temp, gpfb_signature, 4);
			memcpy(&temp[4], "ZONE", 4);
			*(int *)(&temp[8]) = i;
			//test code
			//speedy_param.zonetbl[i].start = 0, speedy_param.zonetbl[i].end = 0;
#endif
		}
#if 0
		{
		    int work_size, table_size;
		    speedy_savetbl_t *savetbl;

			work_size = SPEEDY_WORK_SIZE;
		    savetbl = (speedy_savetbl_t *)speedy_work;
		    table_size = work_size;
		    zonetbl_max = ZONETBL_DEFAULT_NUM;
		    exttbl_max = EXTTBL_DEFAULT_NUM;
		    dramtbl_max = table_size / sizeof(speedy_savetbl_t) -
		        ZONETBL_DEFAULT_NUM - EXTTBL_DEFAULT_NUM;

		    speedy_param.zonetbl_num = 0;
	    	speedy_param.zonetbl = savetbl + dramtbl_max + exttbl_max;
	    }
#endif
    }

    if (speedy_param.exttbl_num == 0) {
		speedy_printf("speedy_param->%s[%d] = start = 0x%x , end 0x%x\n","exttbl", 0,
			speedy_param.exttbl[0].start, speedy_param.exttbl[0].end);
    }
    else {
    	int i;
    	for (i = 0 ; i < speedy_param.exttbl_num ; i++) {
			speedy_printf("speedy_param->%s[%d] = start = 0x%x(0x%x) , end 0x%x(0x%x)\n","exttbl", i,
				speedy_param.exttbl[i].start,speedy_param.exttbl[i].start, speedy_param.exttbl[i].end, speedy_param.exttbl[i].end);
#if 0
			if (speedy_param.exttbl[i].start < 0x17000000) {
				//add GPFB signature for confirm
				if ( (speedy_param.exttbl[i].end - speedy_param.exttbl[i].start) >= 512 ) {
					temp = (speedy_param.exttbl[i].start) + 0xc0000000 + 512-16;
					memcpy(temp, gpfb_signature, 4);
					memcpy(&temp[4], "EXTT", 4);
					*(int *)(&temp[8]) = i;
				}
				else {
					speedy_printf("[%s][%d] ext table[%d] size = %d \n",__func__, __LINE__, i, (speedy_param.exttbl[i].end - speedy_param.exttbl[i].start));
					temp = (speedy_param.exttbl[i].start) + 0xc0000000 + 16;
					memcpy(temp, "GPFBEXTT", 8);
					*(int *)(&temp[8]) = i;
				}
			}
			else {
				speedy_printf("[%s][%d] ext table[%d] not write signature(0x%x)\n",__func__, __LINE__, i, speedy_param.exttbl[i].start);
			}
#endif
		}
    }

    if (speedy_param.dramtbl_num == 0) {
		//speedy_printf("speedy_param->%s[%d] = start = 0x%x , end 0x%x\n","dramtbl", 0,
		//	speedy_param.dramtbl[0].start, speedy_param.dramtbl[0].end);
    }
    else {
    	int i;
    	for (i = 0 ; i < speedy_param.dramtbl_num ; i++) {
			speedy_printf("speedy_param->%s[%d] = start = 0x%x(0x%x) , end 0x%x(0x%x)\n","dramtbl", i,
				speedy_param.dramtbl[i].start,speedy_param.dramtbl[i].start<<PAGE_SHIFT, speedy_param.dramtbl[i].end,speedy_param.dramtbl[i].end<<PAGE_SHIFT);
#if 0
			//add GPFB signature for confirm
			temp = (speedy_param.dramtbl[i].start << PAGE_SHIFT) + 0xc0000000 + 512-16;
			memcpy(temp, gpfb_signature, 4);
			memcpy(&temp[4], "DRAM", 4);
			*(int *)(&temp[8]) = i;
#endif
		}
    }
#endif	//dump memory info

#if 0
	speedy_printf("[%s][%d] dump 3th dram address:0x%x \n",__func__, __LINE__,speedy_param.dramtbl[3].end<<PAGE_SHIFT);
	dumpbuf((speedy_param.dramtbl[3].end<<PAGE_SHIFT|0xc0000000) - 512,512+128);
	speedy_printf("[%s][%d] dump 4th dram address \n",__func__, __LINE__);
	dumpbuf((speedy_param.dramtbl[4].start<<PAGE_SHIFT|0xc0000000),512);
	dumpbuf((speedy_param.dramtbl[4].end<<PAGE_SHIFT|0xc0000000) - 512,512+128);
	speedy_printf("[%s][%d] dump 5th dram address \n",__func__, __LINE__);
	dumpbuf((speedy_param.dramtbl[5].start<<PAGE_SHIFT|0xc0000000),512);
	dumpbuf(0xc06db000,512);
#endif

	dump_speedy_param(&speedy_param);

	//*pMem = gp_dram_checksum(speedy_param.dramtbl, speedy_param.dramtbl_num, speedy_param.page_shift);
	//speedy_printf("[%s][%d] Before hibernation,current_checksum=0x%x\n",__func__, __LINE__,*pMem);
	//*pMem += gp_dram_checksum(speedy_param.exttbl, speedy_param.exttbl_num, 0);
	//speedy_printf("[%s][%d] Before hibernation,current_checksum=0x%x\n",__func__, __LINE__,*pMem);

#if 0//@roger gp speedy
	//move to driver level snapshot.c
/*
	current_checksum = gp_get_checksum(__va(0x100000), 0x10000);
	if (bootloader_checksum != current_checksum) {
		speedy_printf("[%s][%d]ERROR, bootloader checksum fail, original=0x%x, current=0x%x\n", __FUNCTION__, __LINE__, bootloader_checksum, current_checksum);
		return -ENOMEM;
	}
*/
    if (speedy_canceled) {
        ret = -ECANCELED;
    } else {
        if ((ret = SPEEDY_DRV_SNAPSHOT(&speedy_param)) == -ECANCELED)
            speedy_canceled = 1;
    }

	hibernate_debug = 1;
	speedy_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);
    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_SAVEEND);
	speedy_printf("[%s][%d] : restore_scu_regs\n",
		__func__, __LINE__);

#else
//@roger gp speedyboot
	//gp speedy boot
	speedy_printf("[%s][%d] gp speedy boot \n",__func__, __LINE__);
	//hibernate_debug = 1;
    if (speedy_canceled) {
        ret = -ECANCELED;
    } else {
    	ret = gp_mmu_hibernation_area_enable(1);
        if (ret == -ECANCELED)
            speedy_canceled = 1;
    }

	speedy_printf("[%s][%d] get cancel return for speedy boot test\n",__func__, __LINE__);
	//hibernate_debug = 0;
    //ret = -ECANCELED;
    //speedy_canceled = 1;

#endif
	//*pMem = gp_dram_checksum(speedy_param.dramtbl, speedy_param.dramtbl_num, speedy_param.page_shift);
	//speedy_printf("[%s][%d] After hibernation,current_checksum=0x%x\n",__func__, __LINE__,*pMem);
	//*pMem += gp_dram_checksum(speedy_param.exttbl, speedy_param.exttbl_num, 0);
	//speedy_printf("[%s][%d] Before hibernation,current_checksum=0x%x\n",__func__, __LINE__,*pMem);

    return ret;
}

static int speedy_save_snapshot(void)
{
    int ret = 0;
		speedy_printf("[%s][%d] : \n", __func__, __LINE__);
		
		// milton -- ?????
    speedy_param.bootflag_dev  = speedy_savearea[speedy_saveno].bootflag_dev;
    speedy_param.bootflag_area = speedy_savearea[speedy_saveno].bootflag_area;
    speedy_param.bootflag_size = speedy_savearea[speedy_saveno].bootflag_size;
    speedy_param.snapshot_dev  = speedy_savearea[speedy_saveno].snapshot_dev;
    speedy_param.snapshot_area = speedy_savearea[speedy_saveno].snapshot_area;
    speedy_param.snapshot_size = speedy_savearea[speedy_saveno].snapshot_size;

#ifdef CONFIG_SWAP
    if (!speedy_param.oneshot && total_swap_pages > nr_swap_pages)
        speedy_swapout_disable = 1;
    else
        speedy_swapout_disable = 0;
#endif
    if ((ret = speedy_ops->snapshot()) == 0) {
        speedy_stat = speedy_param.stat;
        speedy_retry = speedy_param.retry;
    }

    return ret;
}

void speedy_save_cancel(void)
{
    if (pm_device_down == SPEEDY_STATE_SUSPEND || speedy_separate_pass == 1) {
        speedy_canceled = 1;
        if (speedy_ops->progress)
            speedy_ops->progress(SPEEDY_PROGRESS_CANCEL);
    }
}

int hibernate(void)
{
    int ret;

    if (!speedy_ops) {
        printk("Snapshot driver not found.\n");
        return -EIO;
    }

    BUG_ON(!speedy_ops->snapshot);

    speedy_stat = 0;
    speedy_retry = 0;
    pm_device_down = SPEEDY_STATE_SUSPEND;

    if (speedy_separate == 0) {
        speedy_separate_pass = 0;
        speedy_swapout_disable = 1;
    } else if (speedy_separate == 1) {
        speedy_separate_pass = 0;
        speedy_swapout_disable = 0;
    } else if (speedy_separate == 2) {
        if (speedy_separate_pass != 1) {
            speedy_separate_pass = 1;
            speedy_swapout_disable = 0;
        } else {
            speedy_separate_pass = 2;
            speedy_swapout_disable = 1;
        }
    }

    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_INIT);

    if (speedy_separate_pass == 2 && speedy_canceled) {
        speedy_separate_pass = 0;
        ret = -ECANCELED;
        goto speedy_work_init_err;
    }

    if ((ret = speedy_work_init()) < 0) {
        speedy_separate_pass = 0;
        goto speedy_work_init_err;
    }

    if (speedy_ops->drv_init && (ret = speedy_ops->drv_init()) < 0) {
        speedy_separate_pass = 0;
        goto speedy_drv_init_err;
    }

    system_state = SYSTEM_SUSPEND_DISK;

    mutex_lock(&pm_mutex);

    if ((ret = pm_notifier_call_chain(PM_HIBERNATION_PREPARE))) {
        speedy_separate_pass = 0;
        pm_device_down = SPEEDY_STATE_RESUME;
        goto pm_notifier_call_chain_err;
    }

    if ((ret = usermodehelper_disable())) {
        speedy_separate_pass = 0;
        pm_device_down = SPEEDY_STATE_RESUME;
        goto usermodehelper_disable_err;
    }

    /* Allocate memory management structures */
    if ((ret = create_basic_memory_bitmaps())) {
        speedy_separate_pass = 0;
        pm_device_down = SPEEDY_STATE_RESUME;
        goto create_basic_memory_bitmaps_err;
    }

    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_SYNC);

    printk("Syncing filesystems ... ");
    sys_sync();
    printk("done.\n");

    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_FREEZE);

    if (freeze_processes()) {
        ret = -EBUSY;
        speedy_separate_pass = 0;
        pm_device_down = SPEEDY_STATE_RESUME;
        goto freeze_processes_err;
    }

    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_SHRINK);

    /* Free memory before shutting down devices. */
    if ((ret = speedy_shrink_memory()) || speedy_separate_pass == 1 ||
        speedy_canceled) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto speedy_shrink_memory_err;
    }

    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_SUSPEND);

    suspend_console();

		// milton -- no use now -----------------------------------
    if (speedy_ops->device_suspend_early &&
        (ret = speedy_ops->device_suspend_early())) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto speedy_device_suspend_early_err;
    }
		// --------------------------------------------------------
	
    if ((ret = pm_device_suspend(STATE_FREEZE))) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto pm_device_suspend_err;
    }

		// milton -- no use now -----------------------------------
    if (speedy_ops->device_suspend && (ret = speedy_ops->device_suspend())) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto speedy_device_suspend_err;
    }
		// --------------------------------------------------------
		
    if ((ret = arch_prepare_suspend())) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto arch_prepare_suspend_err;
    }

    if ((ret = pm_device_power_down(STATE_FREEZE))) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto pm_device_power_down_err;
    }

		// milton -- no use now -----------------------------------
    if (speedy_ops->pre_snapshot && (ret = speedy_ops->pre_snapshot())) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto speedy_pre_snapshot_err;
    }
		// --------------------------------------------------------
		
    if ((ret = disable_nonboot_cpus())) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto disable_nonboot_cpus_err;
    }

    local_irq_disable();

#if 1
    if ((ret = sysdev_suspend(STATE_FREEZE))) {
        pm_device_down = SPEEDY_STATE_RESUME;
        goto sysdev_suspend_err;
    }
#else
	speedy_printf("[%s][%d] run syscore_suspend \n", __func__, __LINE__);
	//@roger 3.0.31
    ret = syscore_suspend();
    if (ret != 0) {
    	speedy_printf("T-DEBUG [%s][%d] enter SPEEDY_STATE_RESUME, syscore_suspend=%d \n", __func__, __LINE__,ret);
        pm_device_down = SPEEDY_STATE_RESUME;
        goto sysdev_suspend_err;
    }
#endif

    //mark by gp speedy boot
    //save_processor_state();

    /* Snapshot save */
    ret = speedy_save_snapshot();

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    pm_device_down = SPEEDY_STATE_RESUME;
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    //mark by gp speedy boot
    //restore_processor_state();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_RESUME);
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

	speedy_printf("[%s][%d] run syscore_resume \n", __func__, __LINE__);
#if 1
    sysdev_resume();
#else
	//@roger 3.0.31
    syscore_resume();
#endif
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
sysdev_suspend_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
	speedy_printf("[%s][%d] cpsr =0x%x\n",__func__, __LINE__, dump_cpsr());
    local_irq_enable();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
	speedy_printf("[%s][%d] cpsr =0x%x\n",__func__, __LINE__, dump_cpsr());

disable_nonboot_cpus_err:
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    enable_nonboot_cpus();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (speedy_ops->post_snapshot)
        speedy_ops->post_snapshot();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
speedy_pre_snapshot_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    pm_device_power_up(STATE_RESTORE);
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
pm_device_power_down_err:

arch_prepare_suspend_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (speedy_ops->device_resume)
        speedy_ops->device_resume();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
speedy_device_suspend_err:

pm_device_suspend_err:
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    pm_device_resume(STATE_RESTORE);
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (speedy_ops->device_resume_late)
        speedy_ops->device_resume_late();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
speedy_device_suspend_early_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    resume_console();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

speedy_shrink_memory_err:

freeze_processes_err:
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_THAW);
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    thaw_processes();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    free_basic_memory_bitmaps();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
create_basic_memory_bitmaps_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    usermodehelper_enable();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
usermodehelper_disable_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    pm_notifier_call_chain(PM_POST_HIBERNATION);
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
pm_notifier_call_chain_err:

    mutex_unlock(&pm_mutex);

    system_state = SYSTEM_RUNNING;

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (speedy_ops->drv_uninit)
        speedy_ops->drv_uninit();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
speedy_drv_init_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    speedy_work_free();
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
speedy_work_init_err:

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (speedy_separate_pass == 2)
        speedy_separate_pass = 0;
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

    pm_device_down = SPEEDY_STATE_NORMAL;

	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (speedy_ops->progress)
        speedy_ops->progress(SPEEDY_PROGRESS_EXIT);
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);

    if (speedy_canceled)
        ret = -ECANCELED;
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    speedy_error = ret;
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    if (ret < 0)
        printk(KERN_ERR "GPlus FastBoot!! error %d\n", ret);
	speedy_printf("[%s][%d] : \n", __func__, __LINE__);
    return ret;
}

#ifdef CONFIG_PROC_FS

static struct proc_dir_entry *proc_speedy;
static struct proc_dir_entry *proc_speedy_stat;
static struct proc_dir_entry *proc_speedy_error;
static struct proc_dir_entry *proc_speedy_retry;
static struct proc_dir_entry *proc_speedy_canceled;
static struct proc_dir_entry *proc_speedy_saveno;
static struct proc_dir_entry *proc_speedy_mode;
static struct proc_dir_entry *proc_speedy_compress;
static struct proc_dir_entry *proc_speedy_shrink;
static struct proc_dir_entry *proc_speedy_separate;
static struct proc_dir_entry *proc_speedy_oneshot;
static struct proc_dir_entry *proc_speedy_halt;
static struct proc_dir_entry *proc_speedy_silent;
static struct proc_dir_entry *proc_speedy_erase_sig;

static int write_proc_speedy(const char *buffer, unsigned long count,
                           int max, const char *str)
{
    int val;
    char buf[16];

    if (current_uid() != 0)
        return -EACCES;
    if (count == 0 || count >= 16)
        return -EINVAL;

    if (copy_from_user(buf, buffer, count))
        return -EFAULT;
    buf[count] = '\0';

    sscanf(buf, "%d", &val);

    if (val > max) {
        printk("speedy: %s too large !!\n", str);
        return -EINVAL;
    }
    return val;
}

static int read_proc_speedy_stat(char *page, char **start, off_t offset,
                               int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_stat);
    *eof = 1;
    return len;
}

static int write_proc_speedy_stat(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 1, "stat")) < 0)
        return val;
    speedy_stat = val;
    return count;
}

static int read_proc_speedy_error(char *page, char **start, off_t offset,
                                int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_error);
    *eof = 1;
    return len;
}

static int read_proc_speedy_retry(char *page, char **start, off_t offset,
                                int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_retry);
    *eof = 1;
    return len;
}

static int read_proc_speedy_canceled(char *page, char **start, off_t offset,
                                   int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_canceled);
    *eof = 1;
    return len;
}

static int write_proc_speedy_canceled(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 1, "canceled")) < 0)
        return val;
    speedy_canceled = val;
    return count;
}

static int read_proc_speedy_saveno(char *page, char **start, off_t offset,
                                 int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_saveno);
    *eof = 1;
    return len;
}

static int write_proc_speedy_saveno(struct file *file, const char *buffer,
                                  unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, SPEEDY_SAVEAREA_NUM - 1,
                               "saveno")) < 0)
        return val;
    speedy_saveno = val;
    return count;
}

static int read_proc_speedy_mode(char *page, char **start, off_t offset,
                               int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_param.mode);
    *eof = 1;
    return len;
}

static int write_proc_speedy_mode(struct file *file, const char *buffer,
                                unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 2, "mode")) < 0)
        return val;
    speedy_param.mode = val;
    return count;
}

static int read_proc_speedy_compress(char *page, char **start, off_t offset,
                                   int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_param.compress);
    *eof = 1;
    return len;
}

static int write_proc_speedy_compress(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 2, "compress")) < 0)
        return val;
    speedy_param.compress = val;
    return count;
}

static int read_proc_speedy_shrink(char *page, char **start, off_t offset,
                                 int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_shrink);
    *eof = 1;
    return len;
}

static int write_proc_speedy_shrink(struct file *file, const char *buffer,
                                  unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 3, "shrink")) < 0)
        return val;
    speedy_shrink = val;
    return count;
}

static int read_proc_speedy_separate(char *page, char **start, off_t offset,
                               int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_separate);
    *eof = 1;
    return len;
}

static int write_proc_speedy_separate(struct file *file, const char *buffer,
                                unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, SPEEDY_SEPARATE_MAX,
                               "separate")) < 0)
        return val;
    speedy_separate = val;
    speedy_separate_pass = 0;
    return count;
}

static int read_proc_speedy_oneshot(char *page, char **start, off_t offset,
                                  int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_param.oneshot);
    *eof = 1;
    return len;
}

static int write_proc_speedy_oneshot(struct file *file, const char *buffer,
                                   unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 1, "oneshot")) < 0)
        return val;
    speedy_param.oneshot = val;
    return count;
}


static int read_proc_speedy_halt(char *page, char **start, off_t offset,
                               int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_param.halt);
    *eof = 1;
    return len;
}

static int write_proc_speedy_halt(struct file *file, const char *buffer,
                                unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 1, "halt")) < 0)
        return val;
    speedy_param.halt = val;
    return count;
}

static int read_proc_speedy_silent(char *page, char **start, off_t offset,
                                 int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_param.silent);
    *eof = 1;
    return len;
}

static int write_proc_speedy_silent(struct file *file, const char *buffer,
                                  unsigned long count, void *data)
{
    int val;

    if ((val = write_proc_speedy(buffer, count, 3, "silent")) < 0)
        return val;
    speedy_param.silent = val;
    return count;
}

static int read_proc_speedy_erase_sig(char *page, char **start, off_t offset,
                                 int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "%d\n", speedy_param.erase_sig);
    *eof = 1;
    return len;
}

static int write_proc_speedy_erase_sig(struct file *file, const char *buffer,
                                  unsigned long count, void *data)
{
    int val;
    int ret = 0;

    if ((val = write_proc_speedy(buffer, count, 1, "erase_sig")) < 0)
        return val;
    if (speedy_ops->erase_signature)
        ret = speedy_ops->erase_signature(0);
    if (ret < 0)
    	return -EIO;
    speedy_param.erase_sig = val;
    return count;
}

#endif

int speedy_register_machine(struct speedy_ops *ops)
{
    speedy_ops = ops;
    return 0;
}

int speedy_unregister_machine(struct speedy_ops *ops)
{
    speedy_ops = NULL;
    return 0;
}

static int __init speedy_init(void)
{
	printk(KERN_INFO "speedy_init start\n");

#ifdef CONFIG_PROC_FS
    if ((proc_speedy = proc_mkdir("speedy", NULL))) {
#if 0
        proc_speedy_stat =
            create_proc_read_entry("stat",
                                   S_IRUGO ,
                                   proc_speedy,
                                   read_proc_speedy_stat,
                                   NULL);
#else
        if ((proc_speedy_stat =
             create_proc_entry("stat",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_stat->read_proc = read_proc_speedy_stat;
            proc_speedy_stat->write_proc = write_proc_speedy_stat;
        }
#endif
        proc_speedy_error =
            create_proc_read_entry("error",
                                   S_IRUGO,
                                   proc_speedy,
                                   read_proc_speedy_error,
                                   NULL);
        proc_speedy_retry =
            create_proc_read_entry("retry",
                                   S_IRUGO,
                                   proc_speedy,
                                   read_proc_speedy_retry,
                                   NULL);
        if ((proc_speedy_canceled =
             create_proc_entry("canceled",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_canceled->read_proc = read_proc_speedy_canceled;
            proc_speedy_canceled->write_proc = write_proc_speedy_canceled;
        }
        if ((proc_speedy_saveno =
             create_proc_entry("saveno",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_saveno->read_proc = read_proc_speedy_saveno;
            proc_speedy_saveno->write_proc = write_proc_speedy_saveno;
        }
        if ((proc_speedy_mode =
             create_proc_entry("mode",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_mode->read_proc = read_proc_speedy_mode;
            proc_speedy_mode->write_proc = write_proc_speedy_mode;
        }
        if ((proc_speedy_compress =
             create_proc_entry("compress",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_compress->read_proc = read_proc_speedy_compress;
            proc_speedy_compress->write_proc = write_proc_speedy_compress;
        }
        if ((proc_speedy_shrink =
             create_proc_entry("shrink",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_shrink->read_proc = read_proc_speedy_shrink;
            proc_speedy_shrink->write_proc = write_proc_speedy_shrink;
        }
        if ((proc_speedy_separate =
             create_proc_entry("separate",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_separate->read_proc = read_proc_speedy_separate;
            proc_speedy_separate->write_proc = write_proc_speedy_separate;
        }
        if ((proc_speedy_oneshot =
             create_proc_entry("oneshot",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_oneshot->read_proc = read_proc_speedy_oneshot;
            proc_speedy_oneshot->write_proc = write_proc_speedy_oneshot;
        }
        if ((proc_speedy_halt =
             create_proc_entry("halt",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_halt->read_proc = read_proc_speedy_halt;
            proc_speedy_halt->write_proc = write_proc_speedy_halt;
        }
        if ((proc_speedy_silent =
             create_proc_entry("silent",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_silent->read_proc = read_proc_speedy_silent;
            proc_speedy_silent->write_proc = write_proc_speedy_silent;
        }
        if ((proc_speedy_erase_sig =
             create_proc_entry("erase_sig",
                               S_IRUGO | S_IWUSR,
                               proc_speedy))) {
            proc_speedy_erase_sig->read_proc = read_proc_speedy_erase_sig;
            proc_speedy_erase_sig->write_proc = write_proc_speedy_erase_sig;
        }
    }
#endif

    speedy_saveno = CONFIG_PM_SPEEDY_SAVENO;
    speedy_param.mode = CONFIG_PM_SPEEDY_MODE;
    speedy_param.compress = CONFIG_PM_SPEEDY_COMPRESS;
    speedy_param.silent = CONFIG_PM_SPEEDY_SILENT;
	speedy_param.erase_sig = 0;
    speedy_shrink = CONFIG_PM_SPEEDY_SHRINK;
#ifdef CONFIG_SWAP
    //speedy_separate = CONFIG_PM_SPEEDY_SEPARATE;
#else
    speedy_separate = 0;
#endif
#ifdef CONFIG_PM_SPEEDY_ONESHOT
    speedy_param.oneshot = 1;
#else
    speedy_param.oneshot = 0;
#endif
#ifdef CONFIG_PM_SPEEDY_HALT
    speedy_param.halt = 1;
#else
    speedy_param.halt = 0;
#endif

#ifdef SPEEDY_PRELOAD_EXTTBL
    speedy_param.preload_exttbl = 1;
#else
    speedy_param.preload_exttbl = 0;
#endif

    speedy_param.v2p_offset = PAGE_OFFSET - __pa(PAGE_OFFSET);
    speedy_param.lowmem_size = (unsigned long)high_memory - PAGE_OFFSET;
    speedy_param.page_shift = PAGE_SHIFT;

    speedy_swapout_disable = 0;
    speedy_separate_pass = 0;
    speedy_canceled = 0;

    printk(KERN_INFO "GPlus FastBoot!! module loaded\n");

    return 0;
}

static void __exit speedy_exit(void)
{
#ifdef CONFIG_PROC_FS
    if (proc_speedy_stat) {
        remove_proc_entry("stat", proc_speedy_stat);
        proc_speedy_stat = NULL;
    }
    if (proc_speedy_error) {
        remove_proc_entry("error", proc_speedy_error);
        proc_speedy_error = NULL;
    }
    if (proc_speedy_retry) {
        remove_proc_entry("retry", proc_speedy_retry);
        proc_speedy_retry = NULL;
    }
    if (proc_speedy_canceled) {
        remove_proc_entry("canceled", proc_speedy_canceled);
        proc_speedy_canceled = NULL;
    }
    if (proc_speedy_saveno) {
        remove_proc_entry("saveno", proc_speedy_saveno);
        proc_speedy_saveno = NULL;
    }
    if (proc_speedy_mode) {
        remove_proc_entry("mode", proc_speedy_mode);
        proc_speedy_mode = NULL;
    }
    if (proc_speedy_compress) {
        remove_proc_entry("compress", proc_speedy_compress);
        proc_speedy_compress = NULL;
    }
    if (proc_speedy_shrink) {
        remove_proc_entry("shrink", proc_speedy_shrink);
        proc_speedy_shrink = NULL;
    }
    if (proc_speedy_separate) {
        remove_proc_entry("separate", proc_speedy_separate);
        proc_speedy_separate = NULL;
    }
    if (proc_speedy_oneshot) {
        remove_proc_entry("oneshot", proc_speedy_oneshot);
        proc_speedy_oneshot = NULL;
    }
    if (proc_speedy_halt) {
        remove_proc_entry("halt", proc_speedy_halt);
        proc_speedy_halt = NULL;
    }
    if (proc_speedy_silent) {
        remove_proc_entry("silent", proc_speedy_silent);
        proc_speedy_silent = NULL;
    }
    if (proc_speedy_erase_sig) {
        remove_proc_entry("erase_sig", proc_speedy_erase_sig);
        proc_speedy_erase_sig = NULL;
    }
    if (proc_speedy) {
        remove_proc_entry("speedy", proc_speedy);
        proc_speedy = NULL;
    }
#endif
}

module_init(speedy_init);
module_exit(speedy_exit);
MODULE_LICENSE_GP;
