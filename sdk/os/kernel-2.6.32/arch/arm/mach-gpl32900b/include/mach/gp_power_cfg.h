#ifndef _GP_AUTO_TEST_H_
#define _GP_AUTO_TEST_H_

// deprecated, do not use this define
#define AUTO_POWER_KEY 0 


#define DUMP_POWER_CFG 1
#define CLK_DRAM_OFF 1 

#define OPEN_SELF_REFRESH 1 
#define CPU_ENTER_WFI   1	
#define CORE_POWER_CHANGE 0 
#define SLOW_CPU_AT_WFI 0
#define WDT_AUTO_RESET 1
#define PLL_POWER_DOWN 1 

#define CLK_CHANGE_MODE 0 

#define CORE_POWER_CHANGE_NEW	0
#define MOVE_STACK_TO_SRAM 1


#define AUTO_SUSPEND_RESUME_TEST 0

#define SUSPEND_DGB_PRINT_ENABLE 0

#define USE_DEFAULT_SUSPEND_INFO 0

#define	SPEED_DOWN_COUNT 	100		// 40 seconds, each count for 400ms


#if SUSPEND_DGB_PRINT_ENABLE
#define dbg_printf printk
#else
#define dbg_printf(...)
#endif

#if (AUTO_SUSPEND_RESUME_TEST)
	#define AUTO_VIRTUAL_POWER_KEY 1
#else
#define AUTO_VIRTUAL_POWER_KEY 0
#endif

#endif

