//----------------------------------------------------------------------------
#include <asm/hibernate.h>
#include <linux/ioport.h>

#include <mach/gp_suspend.h>
#include <mach/gp_power_cfg.h>
#include <mach/gp_suspend_def.h>
#include <mach/hardware.h>
#include <mach/kernel.h>
#include <mach/memory.h>

#include <mach/hal/regmap/reg_scu.h>

//----------------------------------------------------------------------------
static unsigned int scu_a_spll0_con, scu_a_spll1_con, scu_a_spll2_con;
static unsigned int scu_a_clksrc_mux_sel;
static unsigned int scuf_peripheral_sub_system_A_bus_control;
static struct {
    unsigned int udt;
    unsigned int _int;
    unsigned int den;
    unsigned int num;
} scu_a_clk_val[10];
static unsigned int scu_j_eth_drive_ref, scu_j_apbdma_int_en;
static unsigned sram_backup_addr = 0, sram_test_addr = 0, suspend_resume_mem = 0;

static unsigned int scu_a_pad_pull_ctrl[7];
static unsigned int scu_a_dsp_pwr_ctrl;

gp_suspend_helper *g_gp_suspend_info = NULL;
extern int cpu_power_down;
//----------------------------------------------------------------------------
GP_SUSPEND_TYPE soc_pwr_get_info_suspend_type(void)
{
	if(NULL == g_gp_suspend_info){
		return GP_SUSPEND_TYPE_MEM;
	}
	else{
		return g_gp_suspend_info->mType;
	}
}
//----------------------------------------------------------------------------
void soc_pwr_down_register_suspend_info(gp_suspend_helper *aInfo)
{
	dbg_printf("[%s] enter.\n", __func__);
	if(NULL != aInfo){
		g_gp_suspend_info = aInfo;
	}

	if(NULL == g_gp_suspend_info){
		cpu_power_down = 0;
	}
	else if(GP_SUSPEND_TYPE_MEM != g_gp_suspend_info->mType){
		cpu_power_down = 1;
	}
}
EXPORT_SYMBOL(soc_pwr_down_register_suspend_info);
//----------------------------------------------------------------------------
void soc_pwr_down_save_scu_regs(void)
{
    int i;
    unsigned int offs;

    scu_a_spll0_con = SCUB_SPLL0_CFG;
    scu_a_spll1_con = SCUB_SPLL1_CFG;
    scu_a_spll2_con = SCUB_SPLL2_CFG;
    scu_a_clksrc_mux_sel = SCUB_SPLL_SEL;
    // milton -- scuf_peripheral_sub_system_A_bus_control = SCUF_SUBA_PERI_BUS_CTRL;
    //dbg_printf("[%s][%d] spll0=%x, spll1=%x, spll2=%x, mux_sel=%x\n", __func__, __LINE__, scu_a_spll0_con, scu_a_spll1_con, scu_a_spll2_con, scu_a_clksrc_mux_sel);

    for (i = 0, offs = 0; i < 10; i++) {
        if (i == 3 || i == 16 || i == 17)
            continue;
        offs = (LOGI_ADDR_SCU_B_REG + 0x400 + (i * 0x10));
        scu_a_clk_val[i].udt = *(volatile unsigned int*)(offs + 0x00);
        scu_a_clk_val[i]._int = *(volatile unsigned int*)(offs + 0x04);
        scu_a_clk_val[i].den = *(volatile unsigned int*)(offs + 0x08);
        scu_a_clk_val[i].num = *(volatile unsigned int*)(offs + 0x0c);
        //dbg_printf("T-DEBUG: [%s][%d] clk%02d udt=%x, int=%x, den=%x, num=%x\n", __func__, __LINE__, i, scu_a_clk_val[i].udt, scu_a_clk_val[i]._int, scu_a_clk_val[i].den, scu_a_clk_val[i].num);
    }

#if 0 // milton 
    scu_j_eth_drive_ref = ETH_DRIVE_REF;
    scu_j_apbdma_int_en = APBDMA_INT_EN;


	scu_a_pad_pull_ctrl[0] = SCU_PAD_PULL_CTRL0;
	scu_a_pad_pull_ctrl[1] = SCU_PAD_PULL_CTRL1;
	scu_a_pad_pull_ctrl[2] = SCU_PAD_PULL_CTRL2;
	scu_a_pad_pull_ctrl[3] = SCU_PAD_PULL_CTRL3;
	scu_a_pad_pull_ctrl[4] = SCU_PAD_PULL_CTRL4;
	scu_a_pad_pull_ctrl[5] = SCU_PAD_PULL_CTRL5;
	scu_a_dsp_pwr_ctrl = PWR_DSP_CTRL;
	scu_a_pad_pull_ctrl[6] = PWR_DISP_CTRL;
#endif

}
//----------------------------------------------------------------------------
//milton -- extern int power_down_domain(PWR_DOMAIN aDomain, int aEnable);
#define MAX_COUNT 0xFFFF
void soc_pwr_down_restore_scu_regs(void)
{
    int i;
    unsigned int offs;
	unsigned int val = 0, val2 = 0, tmp = 0, count = 0;
    printk("[%s][%d] enter\n", __func__, __LINE__);

    for (i = 0, offs = 0; i < 10; i++) {
        // added by warren to avoid change setting for cpu
        if (i == 3 || i == 16 || i == 17 || i == 1 || i == 0)
            continue;
        offs = (LOGI_ADDR_SCU_B_REG + 0x400 + (i * 0x10));
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
        dbg_printf(KERN_WARNING "T-DEBUG: [%s][%d] timeout\n", __func__, __LINE__);
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
    }

	SCUB_SPLL_SEL = scu_a_clksrc_mux_sel;
	
#if 0	// milton
    ETH_DRIVE_REF = scu_j_eth_drive_ref;
    APBDMA_INT_EN = scu_j_apbdma_int_en;
    SCUF_SUBA_PERI_BUS_CTRL = scuf_peripheral_sub_system_A_bus_control;

	SCU_PAD_PULL_CTRL0 = scu_a_pad_pull_ctrl[0];
	SCU_PAD_PULL_CTRL1 = scu_a_pad_pull_ctrl[1];
	SCU_PAD_PULL_CTRL2 = scu_a_pad_pull_ctrl[2];
	SCU_PAD_PULL_CTRL3 = scu_a_pad_pull_ctrl[3];
	SCU_PAD_PULL_CTRL4 = scu_a_pad_pull_ctrl[4];
	SCU_PAD_PULL_CTRL5 = scu_a_pad_pull_ctrl[5];
	PWR_DISP_CTRL = scu_a_pad_pull_ctrl[6];

	if(0 != scu_a_dsp_pwr_ctrl){
		extern int gCpxPowerDownFlag;
		gCpxPowerDownFlag = 0xFF;
		power_down_domain(PD_CPX,1);
		scu_a_dsp_pwr_ctrl = 0;
	}
#endif

}
//----------------------------------------------------------------------------
int verify_suspend_resume_mem_checksum(void)
{
	int checksum = 0, maxval = 0, i = 0;
	int *ptr = suspend_resume_mem;
	maxval = 0x13000 >> 2;
	for(i = 0; i < maxval; i++){
		checksum += ptr[i];
	}

	if(ptr[maxval] == checksum){
		return 0;
	}
	else{
		return 1;
	}
}
//----------------------------------------------------------------------------
void calculate_suspend_resume_mem_checksum(void)
{
	int checksum = 0, maxval = 0, i = 0;
	int *ptr = suspend_resume_mem;
	maxval = 0x13000 >> 2;
	for(i = 0; i < maxval; i++){
		checksum += ptr[i];
	}
	ptr[maxval] = checksum;
}
//----------------------------------------------------------------------------
void reset_suspend_resume_mem_addr(void)
{
	if(0 != suspend_resume_mem){
		memset(suspend_resume_mem, 0, 0x14000);
	}
}
//----------------------------------------------------------------------------
void init_suspend_resume_mem_addr(unsigned int aAddr)
{
	suspend_resume_mem = aAddr;
}
//----------------------------------------------------------------------------
void init_sram_backup_addr(unsigned int aAddr, unsigned int  aAddr2)
{
	sram_backup_addr = aAddr;
	sram_test_addr = aAddr2;
}
//----------------------------------------------------------------------------
void soc_pwr_down_save_sram(void)
{
	void *ptr = NULL;
	dbg_printf("[%s] enter\n", __func__);

	if(0 == sram_backup_addr){
		printk("[%s] save area not initialized. fail return\n", __func__);
		return;
	}

	//ptr =  (void *) ioremap(0xA0000000, 0x8000);
	ptr = SRAM_BASE;

	if(NULL != ptr){
		memcpy((void *)sram_backup_addr, ptr, 0x8000);
		//iounmap(ptr);
	}
	else{
		printk("[%s] ioremap fail\n", __func__);
	}
}
//----------------------------------------------------------------------------
void test_mem(void *ptr, int aSize)
{
	int *tmp_src = NULL, *tmp_dst = NULL, *tmp_dst_bak = NULL;
	int i = 0, new_size = 0, pcondition= 0, pcount = 0;

	dbg_printf("[%s] enter\n", __func__);
	tmp_dst = (int *) sram_test_addr;
	dbg_printf("[%s] malloc mem\n", __func__);

	new_size = aSize / sizeof(int);
	tmp_src = (int *)ptr;
	pcondition = 1024 / sizeof(int);

	dbg_printf("[%s] copy from 0x%08x to 0x%08x\n", __func__, tmp_src, tmp_dst);
	for(i = 0; i < new_size; i++){
		if(0 == (i % pcondition)){
			dbg_printf("[%s] [%d]kbytes ok\n", __func__, pcount++);
		}
		*tmp_dst = *tmp_src;
		tmp_dst++;
		tmp_src++;
	}

	dbg_printf("[%s] verify done\n", __func__);
}
//----------------------------------------------------------------------------
void silly_memcpy(void *ptr_dst, void *ptr_src, int aSize)
{
	int int_size = aSize / sizeof(int), i = 0, pcondition= 0, pcount = 0;
	int *dst = NULL, *src = NULL;

	dst = (int *)ptr_dst;
	src = (int *)ptr_src;
	pcondition = 1024 / sizeof(int);

	dbg_printf("[%s] test src mem\n", __func__);
	test_mem(ptr_src, aSize);

	dbg_printf("[%s] test dst mem\n", __func__);
	test_mem(ptr_dst, aSize);

	for(i = 0; i < int_size; i++){
		if(0 == (i % pcondition)){
			dbg_printf("[%s] copy [%d]kbytes ok\n", __func__, pcount++);
		}
		*dst = *src;
		dst++;
		src++;
	}
	dbg_printf("[%s] copy done\n", __func__);
}
//----------------------------------------------------------------------------
void soc_pwr_down_clean_sram(void)
{
	void *ptr = NULL;
	dbg_printf("[%s] before ioremap\n", __func__);
	//ptr =  (void *) ioremap(0xA0000000, 0x8000);
	ptr = SRAM_BASE;
	dbg_printf("[%s] after ioremap\n", __func__);

	if(NULL != ptr){
		dbg_printf("[%s] before memset = 0x%08x\n", __func__, (int)ptr);
		memset(ptr, 0, 0x8000);
		dbg_printf("[%s] after memset\n", __func__);
		//iounmap(ptr);
		dbg_printf("[%s] iounmap done.\n", __func__);
	}
	else{
		printk("[%s] ioremap fail\n", __func__);
	}
}
//----------------------------------------------------------------------------
void soc_pwr_down_restore_sram(void)
{
	void *ptr = NULL;
	dbg_printf("[%s] enter sram_backup_addr = 0x%08x\n", __func__, sram_backup_addr);

	if(0 == sram_backup_addr){
		printk("[%s] save area not initialized. fail return\n", __func__);
		return;
	}

	dbg_printf("[%s] before ioremap\n", __func__);
	//ptr =  (void *) ioremap(0xA0000000, 0x8000);
	ptr = ptr = SRAM_BASE;
	dbg_printf("[%s] after ioremap\n", __func__);

	if(NULL != ptr){
		dbg_printf("[%s] before memcpy dst = 0x%08x src = 0x%08x\n", __func__, (int)ptr, (int)sram_backup_addr);
		silly_memcpy(ptr, (void *)sram_backup_addr, 0x8000);
		//memcpy(ptr, (void *)sram_backup_addr, 0x8000);
		dbg_printf("[%s] after memcpy\n", __func__);
		//iounmap(ptr);
		dbg_printf("[%s] iounmap done.\n", __func__);
	}
	else{
		printk("[%s] ioremap fail\n", __func__);
	}
}
//----------------------------------------------------------------------------
void soc_pwr_down_cache_flush_all(void)
{
	unsigned long irq;
	local_irq_save(irq);
	__cpuc_flush_kern_all();
	local_irq_restore(irq);
}
//----------------------------------------------------------------------------

