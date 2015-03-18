//----------------------------------------------------------------------------
#ifndef _GP_SUSPEND_H_
#define _GP_SUSPEND_H_
//----------------------------------------------------------------------------
extern int cpu_power_down;
extern volatile int in_suspend_flow;

extern void pwr_dump_arch_state(void);
extern void pwr_dump_processor_state(void);
extern void pwr_restore_processor_state(void);
extern asmlinkage int swsusp_arch_suspend(void);
extern asmlinkage int swsusp_arch_resume(void);
extern asmlinkage int swsusp_arch_resume_special(void);
extern asmlinkage int pwr_arch_suspend(unsigned long aAddr);
extern asmlinkage int pwr_arch_resume(unsigned long aAddr);


extern void pwr_save_processor_state(void);
extern unsigned long cpu_context_save_area;



extern void soc_pwr_down_save_scu_regs(void);
extern void soc_pwr_down_restore_scu_regs(void);
extern void soc_pwr_down_save_sram(void);
extern void soc_pwr_down_restore_sram(void);
extern void soc_pwr_down_clean_sram(void);
extern void init_sram_backup_addr(unsigned int aAddr, unsigned int aAddr2);
extern void soc_pwr_down_cache_flush_all(void);
extern void init_suspend_resume_mem_addr(unsigned int aAddr);
extern void reset_suspend_resume_mem_addr(void);
extern void calculate_suspend_resume_mem_checksum(void);
extern int verify_suspend_resume_mem_checksum(void);

extern void gpHalRtcDummyWrite(unsigned short value);
extern unsigned short gpHalRtcDummyRead(void);

extern void set_kern_nand_reinit_var(int aIn);
extern int get_kern_nand_reinit_var(void);
extern void reset_kern_nand_reinit_var(void);

//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
