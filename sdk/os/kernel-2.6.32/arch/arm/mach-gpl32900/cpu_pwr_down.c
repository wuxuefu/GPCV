/*
 * Hibernation support specific for ARM
 *
 * Copyright (C) 2010 Nokia Corporation
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Copyright (C) 2006 Rafael J. Wysocki <rjw@xxxxxxx>
 *
 * Contact: Hiroshi DOYU <Hiroshi.DOYU@xxxxxxxxx>
 *
 * License terms: GNU General Public License (GPL) version 2
 */
#include <asm/hibernate.h>
#include <mach/kernel.h>
#include <linux/ioport.h>
#include <mach/memory.h>
#include <mach/hardware.h>
#include <mach/gp_suspend.h>
#include <mach/gp_power_cfg.h>
#include <mach/gp_suspend_def.h>


struct saved_context *pwr_saved_context = NULL;
unsigned long cpu_context_save_area = 0;
unsigned long ctx_backup_addr;

unsigned int g_ttb_phy_addr = 0;
unsigned int g_ttb_remap_addr = 0;
unsigned int g_ttb_sram_entry_val = 0;

extern void gpHalRtcDummyWrite(unsigned short value);

void init_pwr_saved_context(unsigned char *aAddr)
{
	printk("[%s] size=%d ctx addr = 0x%08x", __func__, sizeof(struct saved_context), (unsigned int)pwr_saved_context);
	pwr_saved_context = (struct saved_context *)aAddr;
}

static inline void __pwr_save_processor_state(struct saved_context *ctxt)
{
	asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(ctxt->cr));
	asm volatile ("mrc p15, 0, %0, c1, c0, 2" : "=r"(ctxt->cacr));
	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r"(ctxt->ttb0));
	asm volatile ("mrc p15, 0, %0, c2, c0, 1" : "=r"(ctxt->ttb1));
	asm volatile ("mrc p15, 0, %0, c2, c0, 2" : "=r"(ctxt->ttbcr));
	asm volatile ("mrc p15, 0, %0, c3, c0, 0" : "=r"(ctxt->dacr));
	asm volatile ("mrc p15, 0, %0, c5, c0, 0" : "=r"(ctxt->dfsr));
	asm volatile ("mrc p15, 0, %0, c5, c0, 1" : "=r"(ctxt->ifsr));
	asm volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r"(ctxt->dfar));
	asm volatile ("mrc p15, 0, %0, c6, c0, 1" : "=r"(ctxt->wfar));
	asm volatile ("mrc p15, 0, %0, c6, c0, 2" : "=r"(ctxt->ifar));
	asm volatile ("mrc p15, 0, %0, c9, c0, 0" : "=r"(ctxt->dclr));
	asm volatile ("mrc p15, 0, %0, c9, c0, 1" : "=r"(ctxt->iclr));
	asm volatile ("mrc p15, 0, %0, c9, c1, 0" : "=r"(ctxt->dtcmr));
	asm volatile ("mrc p15, 0, %0, c9, c1, 1" : "=r"(ctxt->itcmr));
	asm volatile ("mrc p15, 0, %0, c9, c2, 0" : "=r"(ctxt->tcmsel));
	asm volatile ("mrc p15, 0, %0, c9, c8, 0" : "=r"(ctxt->cbor));
	asm volatile ("mrc p15, 0, %0, c10, c0, 0" : "=r"(ctxt->tlblr));
	asm volatile ("mrc p15, 0, %0, c10, c2, 0" : "=r"(ctxt->prrr));
	asm volatile ("mrc p15, 0, %0, c10, c2, 1" : "=r"(ctxt->nrrr));
	asm volatile ("mrc p15, 0, %0, c12, c0, 0" : "=r"(ctxt->snsvbar));
	asm volatile ("mrc p15, 0, %0, c12, c0, 1" : "=r"(ctxt->mvbar));
	asm volatile ("mrc p15, 0, %0, c13, c0, 0" : "=r"(ctxt->fcse));
	asm volatile ("mrc p15, 0, %0, c13, c0, 1" : "=r"(ctxt->cid));
	asm volatile ("mrc p15, 0, %0, c13, c0, 2" : "=r"(ctxt->urwtpid));
	asm volatile ("mrc p15, 0, %0, c13, c0, 3" : "=r"(ctxt->urotpid));
	asm volatile ("mrc p15, 0, %0, c13, c0, 4" : "=r"(ctxt->potpid));
	asm volatile ("mrc p15, 0, %0, c15, c2, 4" : "=r"(ctxt->pmrr));
	asm volatile ("mrc p15, 0, %0, c15, c12, 0" : "=r"(ctxt->pmcr));
	asm volatile ("mrc p15, 0, %0, c15, c12, 1" : "=r"(ctxt->pmcc));
	asm volatile ("mrc p15, 0, %0, c15, c12, 2" : "=r"(ctxt->pmc0));
	asm volatile ("mrc p15, 0, %0, c15, c12, 3" : "=r"(ctxt->pmc1));
}

static inline void __pwr_restore_processor_state(struct saved_context *ctxt)
{
	asm volatile ("mcr p15, 0, %0, c1, c0, 0" : : "r"(ctxt->cr));
	asm volatile ("mcr p15, 0, %0, c1, c0, 2" : : "r"(ctxt->cacr));
	asm volatile ("mcr p15, 0, %0, c2, c0, 0" : : "r"(ctxt->ttb0));
	asm volatile ("mcr p15, 0, %0, c2, c0, 1" : : "r"(ctxt->ttb1));
	asm volatile ("mcr p15, 0, %0, c2, c0, 2" : : "r"(ctxt->ttbcr));
	asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r"(ctxt->dacr));
	asm volatile ("mcr p15, 0, %0, c5, c0, 0" : : "r"(ctxt->dfsr));
	asm volatile ("mcr p15, 0, %0, c5, c0, 1" : : "r"(ctxt->ifsr));
	asm volatile ("mcr p15, 0, %0, c6, c0, 0" : : "r"(ctxt->dfar));
	asm volatile ("mcr p15, 0, %0, c6, c0, 1" : : "r"(ctxt->wfar));
	asm volatile ("mcr p15, 0, %0, c6, c0, 2" : : "r"(ctxt->ifar));
	asm volatile ("mcr p15, 0, %0, c9, c0, 0" : : "r"(ctxt->dclr));
	asm volatile ("mcr p15, 0, %0, c9, c0, 1" : : "r"(ctxt->iclr));
	asm volatile ("mcr p15, 0, %0, c9, c1, 0" : : "r"(ctxt->dtcmr));
	asm volatile ("mcr p15, 0, %0, c9, c1, 1" : : "r"(ctxt->itcmr));
	asm volatile ("mcr p15, 0, %0, c9, c2, 0" : : "r"(ctxt->tcmsel));
	asm volatile ("mcr p15, 0, %0, c9, c8, 0" : : "r"(ctxt->cbor));
	asm volatile ("mcr p15, 0, %0, c10, c0, 0" : : "r"(ctxt->tlblr));
	asm volatile ("mcr p15, 0, %0, c10, c2, 0" : : "r"(ctxt->prrr));
	asm volatile ("mcr p15, 0, %0, c10, c2, 1" : : "r"(ctxt->nrrr));
	asm volatile ("mcr p15, 0, %0, c12, c0, 0" : : "r"(ctxt->snsvbar));
	asm volatile ("mcr p15, 0, %0, c12, c0, 1" : : "r"(ctxt->mvbar));
	asm volatile ("mcr p15, 0, %0, c13, c0, 0" : : "r"(ctxt->fcse));
	asm volatile ("mcr p15, 0, %0, c13, c0, 1" : : "r"(ctxt->cid));
	asm volatile ("mcr p15, 0, %0, c13, c0, 2" : : "r"(ctxt->urwtpid));
	asm volatile ("mcr p15, 0, %0, c13, c0, 3" : : "r"(ctxt->urotpid));
	asm volatile ("mcr p15, 0, %0, c13, c0, 4" : : "r"(ctxt->potpid));
	asm volatile ("mcr p15, 0, %0, c15, c2, 4" : : "r"(ctxt->pmrr));
	asm volatile ("mcr p15, 0, %0, c15, c12, 0" : : "r"(ctxt->pmcr));
	asm volatile ("mcr p15, 0, %0, c15, c12, 1" : : "r"(ctxt->pmcc));
	asm volatile ("mcr p15, 0, %0, c15, c12, 2" : : "r"(ctxt->pmc0));
	asm volatile ("mcr p15, 0, %0, c15, c12, 3" : : "r"(ctxt->pmc1));	
}

void pwr_save_processor_state(void)
{
	dbg_printf("[%s] enter.\n", __func__);
	dbg_printf("[%s] size=%d ctx addr = 0x%08x\n", __func__, sizeof(struct saved_context), (unsigned int)pwr_saved_context);
	preempt_disable();
	__pwr_save_processor_state(pwr_saved_context);
}

void pwr_restore_processor_state(void)
{
	__pwr_restore_processor_state(pwr_saved_context);
	preempt_enable();
	dbg_printf("[%s] exit.\n", __func__);
}

void dump_mem(unsigned int aAddr, unsigned int aSize)
{
	int size = aSize >> 2;
	int i = 0;
	unsigned int *ptr = (unsigned int *)aAddr;
	for(i = 0; i < size; i++){
		printk("[0x%08x]0x%08x", (unsigned int)ptr, *ptr);
		if(i%2) printk("\n");
		ptr++;
	}
	printk("\n");
}

void pwr_dump_arch_state(void)
{
	
	dbg_printf("[%s] enter.\n", __func__);
	dump_mem(cpu_context_save_area, 0x58);
}

void pwr_dump_processor_state(void)
{
	int size = sizeof(struct saved_context);
	printk("[%s] enter.\n", __func__);

	printk("[%s] ttb0 = 0x%08x\n", __func__, pwr_saved_context->ttb0);
	printk("[%s] ttb1 = 0x%08x\n", __func__, pwr_saved_context->ttb1);
	printk("[%s] ttb2 = 0x%08x\n", __func__, pwr_saved_context->ttbcr);
	dump_mem((unsigned int)pwr_saved_context, size);	
}

extern void soc_pwr_down_register_suspend_info(gp_suspend_helper *aInfo);

void pwr_set_scanram_magic(void)
{
	unsigned long addr = cpu_context_save_area;
	unsigned int *ptr_francis = NULL;

	if(0 == cpu_context_save_area){
		return;
	}
	
	addr += 0x13FF0;
	ptr_francis = addr;
	*ptr_francis = 0xA55A5AA5;
	ptr_francis++;
	*ptr_francis = 0xA55A5AA5;
	ptr_francis++;
	*ptr_francis = 0xA55A5AA5;
	ptr_francis++;
	*ptr_francis = 0xA55A5AA5;
}

void pwr_down_reserved_mem_init(void)
{
	unsigned char *ptr = NULL;
#if USE_DEFAULT_SUSPEND_INFO
	gp_suspend_helper *info_ptr = NULL;
#endif
	dbg_printf("[%s] enter.\n", __func__);

	
	if(0 == cpu_context_save_area)
	{
		ptr = (unsigned char *)CPU_REG_BASE;

		if(NULL == ptr){
			printk("[%s] ioremap for context save area failed.\n", __func__);
			return;
		}
		else{
			init_suspend_resume_mem_addr((unsigned int)ptr);
			//reset_suspend_resume_mem_addr();
			cpu_context_save_area = (unsigned long)ptr;
			init_pwr_saved_context(ptr + 0x1000);
			init_sram_backup_addr((unsigned int)ptr + 0x2000, (unsigned int)ptr + 0xA000);
		}
	}

#if USE_DEFAULT_SUSPEND_INFO
	info_ptr = (gp_suspend_helper *)kmalloc( sizeof(gp_suspend_helper), GFP_KERNEL);
	info_ptr->mType = GP_SUSPEND_TYPE_SOC_PWR_DOWN_I2C;
	info_ptr->mI2C_slave_Addr = 0x90;
	info_ptr->mI2C_clock = 1556;
	info_ptr->mCommandDelay = 2000;
	info_ptr->mNumCommand = 4;
	info_ptr->mCommandArray[0] = 0x02;
	info_ptr->mCommandArray[1] = 0xA0;
	info_ptr->mCommandArray[2] = 0x0d;
	info_ptr->mCommandArray[3] = 0x9f;
	info_ptr->mCommandArray[4] = 0x0e;
	info_ptr->mCommandArray[5] = 0x35;
	info_ptr->mCommandArray[6] = 0x0d;
	info_ptr->mCommandArray[7] = 0x9f;

	soc_pwr_down_register_suspend_info(info_ptr);
#endif

	//Set RTC Dummy1 value to 0xA5 move to devices-8k.c
	//gpHalRtcDummyWrite((unsigned short)0xA500);

}

// recover mmu tbl entry adapted in scan ram code
void pwr_down_restore_mmu_tbl(void)
{
	struct saved_context *ctxt = pwr_saved_context;

	if((NULL == ctxt) || (0 == g_ttb_phy_addr)){
		dbg_printf("[%s] pwr_saved_context is NULL or phy addr zero!!!!!!.\n", __func__);
		return;
	}
	(*((volatile unsigned int *)(g_ttb_remap_addr + (0x907<<2)))) = g_ttb_sram_entry_val;
}

extern void enable_dram_ioremap(int aEnable);

void pwr_down_save_mmu_tbl(void)
{
	struct saved_context *ctxt = pwr_saved_context;
	unsigned int val = 0;

	if(NULL == ctxt){
		dbg_printf("[%s] pwr_saved_context is NULL!!!!!!.\n", __func__);
		return;
	}

	val = ctxt->ttb0 & 0xffffc000;
	if(g_ttb_phy_addr != val){
		enable_dram_ioremap(1);
		g_ttb_phy_addr = val;
		if(0 != g_ttb_remap_addr){
			iounmap((void *)g_ttb_remap_addr);
		}
		g_ttb_remap_addr = (int) ioremap(g_ttb_phy_addr, 0x4000);
		enable_dram_ioremap(0);
	}

	g_ttb_sram_entry_val = (*((volatile unsigned int *)(g_ttb_remap_addr + (0x907<<2))));
}


