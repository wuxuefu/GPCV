#ifndef _CPU_PWR_DOWN_H_
#define _CPU_PWR_DOWN_H_ 
extern void pwr_set_scanram_magic(void);
extern void pwr_down_reserved_mem_init(void);
extern void pwr_down_restore_mmu_tbl(void);
extern void pwr_down_save_mmu_tbl(void);
#endif

