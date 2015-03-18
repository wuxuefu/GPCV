#include <linux/kernel.h>       /* printk() */
#include <linux/device.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/hal/hal_clock.h>
#include <mach/hal/regmap/reg_apbdma1.h>

#define APBDMA_M2P					0x00000000
#define APBDMA_P2M					0x00000001
#define APBDMA_AUTO					0x00000000
#define APBDMA_REQ					0x00000002
#define APBDMA_CON					0x00000000
#define APBDMA_FIX					0x00000004
#define APBDMA_SINGLE_BUF			0x00000000
#define APBDMA_DOUBLE_BUF			0x00000008
#define APBDMA_8BIT					0x00000000
#define APBDMA_16BIT				0x00000010
#define APBDMA_32BIT				0x00000020
#define APBDMA_32BIT_BURST			0x00000030
#define APBDMA_IRQOFF				0x00000000
#define APBDMA_IRQON				0x00000040
#define APBDMA_OFF					0x00000000
#define APBDMA_ON					0x00000080

DEFINE_SPINLOCK(apbdma_lock);

static apbdma1Reg_t *g_apb = (apbdma1Reg_t *)LOGI_ADDR_APBDMA1_REG;

void gpHalApbdma1ClkEn(int enable)
{
	spin_lock(&apbdma_lock);
	gpHalScuClkEnable(SCU_A_PERI_APBDMA_A, SCU_A, enable);
	spin_unlock(&apbdma_lock);
}
EXPORT_SYMBOL(gpHalApbdma1ClkEn);

void gpHalApbdma1En(int ch)
{
	apbdma1chReg_t *apbch = (apbdma1chReg_t *)(LOGI_ADDR_APBDMA1_CH0_REG + (ch<<2));
	if(ch==0) {
		apbch->apbchSA=0x93012004;
		apbch->apbchCtrl = DMA_M2P | DMA_REQ | DMA_FIX |
					  DMA_DOUBLE_BUF | DMA_32BIT_BURST |
					  DMA_IRQON | DMA_ON;
	} else {
		apbch->apbchSA=0x9301D004;
		apbch->apbchCtrl = DMA_P2M | DMA_REQ | DMA_FIX | DMA_DOUBLE_BUF | DMA_32BIT | DMA_IRQON | DMA_ON;
	}
}
EXPORT_SYMBOL(gpHalApbdma1En);

void gpHalApbdma1Disable(int ch)
{
	apbdma1chReg_t *apbch = (apbdma1chReg_t *)(LOGI_ADDR_APBDMA1_CH0_REG + (ch<<2));
	if(ch==0) {
		apbch->apbchCtrl = DMA_OFF;
	} else {
		apbch->apbchCtrl = DMA_OFF;
	}
}
EXPORT_SYMBOL(gpHalApbdma1Disable);

void gpHalApbdma1Rst(int ch)
{
	int setbit = 1<<ch;
	apbdma1chReg_t *apbch = (apbdma1chReg_t *)(LOGI_ADDR_APBDMA1_CH0_REG + (ch<<2));
	/* ----- Disable channel ----- */
	apbch->apbchCtrl = 0;
	/* ----- Set channel reset ----- */
	g_apb->apbRST |= setbit;
	/* ----- Wait reset end ----- */ 
	while(g_apb->apbRST&setbit);
}
EXPORT_SYMBOL(gpHalApbdma1Rst);

/**
* @brief	Clear channel IRQ status.
* @param	ch[in]: Channel number.
* @return: 	None.
*/
void gpHalApbdma1ClearIRQFlag(int ch)
{
	int clrbit = 1<<ch;
	g_apb->apbINT = clrbit;
}
EXPORT_SYMBOL(gpHalApbdma1ClearIRQFlag);

/**
* @brief	Set apbdma0 buffer.
* @param	ch[in]: Channel number.
* @param	buf_num[in]: Buffer number 0 or 1.
* @param	addr[in]: Buffer start address.
* @param	ln[in]: Buffer size.
* @return: 	None.
*/
void gpHalApbdma1SetBuf(int ch, int buf_num, char* addr, int ln)
{
	apbdma1chReg_t *apbch = (apbdma1chReg_t *)(LOGI_ADDR_APBDMA1_CH0_REG + (ch<<2));
	char data_width = 1<<((apbch->apbchCtrl&0x30)>>2);
	
//	if(data_width>4)
		data_width =4;		//force data_width to 4
	if(buf_num)	{
		apbch->apbchSAB = (unsigned int)addr;
		apbch->apbchEAB = (unsigned int)(addr+ln-data_width);
	} else {
		apbch->apbchSAA = (unsigned int)addr;
		apbch->apbchEAA = (unsigned int)(addr+ln-data_width);	
	}
}
EXPORT_SYMBOL(gpHalApbdma1SetBuf);

/**
* @brief	Check which apbdma1 buffer in use.
* @param	ch[in]: Channel number.
* @return: 	0 = buffer 0, 1 = buffer 1.
*/
int gpHalApbdma1BufStatus (int ch)
{
	int ckbit = 0x100<<ch;
	return 	(g_apb->apbStatus&ckbit)?1:0;		
}
EXPORT_SYMBOL(gpHalApbdma1BufStatus);
