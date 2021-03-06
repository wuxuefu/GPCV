/*
 * arch/arm/mach-spmp8000/include/mach/regs-scu.h
 *
 * Copyright (C) 2007 Sunplus MultiMedia
 *
 * Syetem Unit Control Unit
 *
 */



#define SCU_A_BASE			IO3_ADDRESS(0x7000)
#define SCU_B_BASE			IO0_ADDRESS(0x5000)
#define SCU_C_BASE			IO2_ADDRESS(0x5000)
#define SCU_D_BASE			IO0_ADDRESS(0x532800)


/*  SCU_A  Control Unit */
#define SCUA_A_PERI_RST			(*(volatile unsigned int*)(SCU_A_BASE+0x00))
#define SCUA_A_PERI_CLKEN		(*(volatile unsigned int*)(SCU_A_BASE+0x04))
#define SCUA_A_PERI_DGCLKEN		(*(volatile unsigned int*)(SCU_A_BASE+0x0C))
#define SCUA_LCD_TYPE_SEL		(*(volatile unsigned int*)(SCU_A_BASE+0x10))
#define SCUA_USBPHY_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0x3C))
#define SCUA_VDAC_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0x40))
#define SCUA_APLL_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0x44))
#define SCUA_CPU_DBG_CTRL_A		(*(volatile unsigned int*)(SCU_A_BASE+0x50))
#define SCUA_CPU_DBG_STAT_A		(*(volatile unsigned int*)(SCU_A_BASE+0x54))
#define SCUA_LCD_CLK_CFG		(*(volatile unsigned int*)(SCU_A_BASE+0x80))
#define SCUA_CSI_CLK_CFG		(*(volatile unsigned int*)(SCU_A_BASE+0x84))
#define SCUA_DUMMY2				(*(volatile unsigned int*)(SCU_A_BASE+0x88))
#define SCUA_DUMMY6				(*(volatile unsigned int*)(SCU_A_BASE+0x8C))
#define SCUA_I2S_BCK_CFG		(*(volatile unsigned int*)(SCU_A_BASE+0x90))
#define SCUA_UART_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0x94))
#define SCUA_DUMMY0				(*(volatile unsigned int*)(SCU_A_BASE+0xA0))
#define SCUA_DUMMY1				(*(volatile unsigned int*)(SCU_A_BASE+0xA4))
#define SCUA_CODEC_CFG			(*(volatile unsigned int*)(SCU_A_BASE+0xB0))
#define SCUA_DUMMY3				(*(volatile unsigned int*)(SCU_A_BASE+0xC0))
#define SCUA_DUMMY4				(*(volatile unsigned int*)(SCU_A_BASE+0xC4))
#define SCUA_DUMMY5				(*(volatile unsigned int*)(SCU_A_BASE+0xD0))
#define SCUA_SAR_GPIO_CTRL		(*(volatile unsigned int*)(SCU_A_BASE+0xE0))
#define SCUA_SAR_GPIO_OEN		(*(volatile unsigned int*)(SCU_A_BASE+0xE4))
#define SCUA_SAR_GPIO_O			(*(volatile unsigned int*)(SCU_A_BASE+0xE8))
#define SCUA_SAR_GPIO_I			(*(volatile unsigned int*)(SCU_A_BASE+0xEC))

#define SP_SCUB_WFI				(SCU_B_BASE+0x58)

/*  SCU_B  Control Unit */
#define SCUB_B_PERI_RST			(*(volatile unsigned int*)(SCU_B_BASE+0x00))
#define SCUB_SPLL_CFG			(*(volatile unsigned int*)(SCU_B_BASE+0x04))
#define SCUB_B_INTR_STATUS		(*(volatile unsigned int*)(SCU_B_BASE+0x08))
#define SCUB_TIMER_ICE_EN		(*(volatile unsigned int*)(SCU_B_BASE+0x0C))
#define SCUB_TIMER_EXT_CTRL		(*(volatile unsigned int*)(SCU_B_BASE+0x10))
#define SCUB_OTP0				(*(volatile unsigned int*)(SCU_B_BASE+0x14))
#define SCUB_REV				(*(volatile unsigned int*)(SCU_B_BASE+0x18))
#define SCUB_RAND0				(*(volatile unsigned int*)(SCU_B_BASE+0x1C))
#define SCUB_B_PERI_CLKEN		(*(volatile unsigned int*)(SCU_B_BASE+0x20))
#define SCUB_B_PERI_DBGCLKEN	(*(volatile unsigned int*)(SCU_B_BASE+0x24))
#define SCUB_B_UPDATE_RATIO		(*(volatile unsigned int*)(SCU_B_BASE+0x28))
#define SCUB_CHIP_DBG_CTRL		(*(volatile unsigned int*)(SCU_B_BASE+0x38))
#define SCUB_CHIP_DBG_STAT		(*(volatile unsigned int*)(SCU_B_BASE+0x3C))
#define SCUB_PWRC_CFG			(*(volatile unsigned int*)(SCU_B_BASE+0x40))
#define SCUB_OTP1				(*(volatile unsigned int*)(SCU_B_BASE+0x44))
#define SCUB_RAND1				(*(volatile unsigned int*)(SCU_B_BASE+0x48))
#define SCUB_OTP2				(*(volatile unsigned int*)(SCU_B_BASE+0x4C))
#define SCUB_IO_TRAP			(*(volatile unsigned int*)(SCU_B_BASE+0x50))
#define SCUB_WFI				(*(volatile unsigned int*)(SCU_B_BASE+0x58))
#define SCUB_GPIO2_I			(*(volatile unsigned int*)(SCU_B_BASE+0x60))
#define SCUB_GPIO2_O			(*(volatile unsigned int*)(SCU_B_BASE+0x64))
#define SCUB_GPIO2_E			(*(volatile unsigned int*)(SCU_B_BASE+0x68))
#define SCUB_GPIO3_I			(*(volatile unsigned int*)(SCU_B_BASE+0x70))
#define SCUB_GPIO3_O			(*(volatile unsigned int*)(SCU_B_BASE+0x74))
#define SCUB_GPIO3_E			(*(volatile unsigned int*)(SCU_B_BASE+0x78))
#define SCUB_PGS0				(*(volatile unsigned int*)(SCU_B_BASE+0x80))
#define SCUB_PGS1				(*(volatile unsigned int*)(SCU_B_BASE+0x84))
#define SCUB_PGS2				(*(volatile unsigned int*)(SCU_B_BASE+0x88))
#define SCUB_PGS3				(*(volatile unsigned int*)(SCU_B_BASE+0x8C))
//#define SCUB_PGC0				(*(volatile unsigned int*)(SCU_B_BASE+0x90))
//#define SCUB_PGC1				(*(volatile unsigned int*)(SCU_B_BASE+0x94))
//#define SCUB_PGC2				(*(volatile unsigned int*)(SCU_B_BASE+0x98))
//#define SCUB_PGC3				(*(volatile unsigned int*)(SCU_B_BASE+0x9C))
#define SCUB_DUMMYREG0			(*(volatile unsigned int*)(SCU_B_BASE+0xA8))
#define SCUB_DUMMYREG1			(*(volatile unsigned int*)(SCU_B_BASE+0xAC))
#define SCUB_CPU_DBG_CTRL		(*(volatile unsigned int*)(SCU_B_BASE+0xB0))
#define SCUB_CPU_DBG_STAT		(*(volatile unsigned int*)(SCU_B_BASE+0xB4))
#define SCUB_DUMMYREG4			(*(volatile unsigned int*)(SCU_B_BASE+0xBC))
#define SCUB_ARM_RATIO			(*(volatile unsigned int*)(SCU_B_BASE+0xD0))
#define SCUB_ARM_AHB_RATIO		(*(volatile unsigned int*)(SCU_B_BASE+0xD4))
#define SCUB_ARM_APB_RATIO		(*(volatile unsigned int*)(SCU_B_BASE+0xD8))
#define SCUB_SYS_CNT_EN			(*(volatile unsigned int*)(SCU_B_BASE+0xDC))

#define SCUB_GPIO0_IE   (*(volatile unsigned int*)(SCU_B_BASE+0x100))
#define SCUB_GPIO0_DS   (*(volatile unsigned int*)(SCU_B_BASE+0x104))
#define SCUB_GPIO0_PE   (*(volatile unsigned int*)(SCU_B_BASE+0x108))
#define SCUB_GPIO0_PS   (*(volatile unsigned int*)(SCU_B_BASE+0x10C))

#define SCUB_GPIO1_IE   (*(volatile unsigned int*)(SCU_B_BASE+0x110))
#define SCUB_GPIO1_DS   (*(volatile unsigned int*)(SCU_B_BASE+0x114))
#define SCUB_GPIO1_PE   (*(volatile unsigned int*)(SCU_B_BASE+0x118))
#define SCUB_GPIO1_PS   (*(volatile unsigned int*)(SCU_B_BASE+0x11C))

#define SCUB_GPIO2_IE   (*(volatile unsigned int*)(SCU_B_BASE+0x120))
#define SCUB_GPIO2_DS   (*(volatile unsigned int*)(SCU_B_BASE+0x124))
#define SCUB_GPIO2_PE   (*(volatile unsigned int*)(SCU_B_BASE+0x128))
#define SCUB_GPIO2_PS   (*(volatile unsigned int*)(SCU_B_BASE+0x12C))

#define SCUB_GPIO3_IE   (*(volatile unsigned int*)(SCU_B_BASE+0x130))
#define SCUB_GPIO3_DS   (*(volatile unsigned int*)(SCU_B_BASE+0x134))
#define SCUB_GPIO3_PE   (*(volatile unsigned int*)(SCU_B_BASE+0x138))
#define SCUB_GPIO3_PS   (*(volatile unsigned int*)(SCU_B_BASE+0x13C))

/*  SCU_C  Control Unit */
#define SCUC_C_PERI_RST			(*(volatile unsigned int*)(SCU_C_BASE+0x00))
#define SCUC_C_PERI_CLKEN		(*(volatile unsigned int*)(SCU_C_BASE+0x04))
#define SCUC_C_PERI_DGCLKEN		(*(volatile unsigned int*)(SCU_C_BASE+0x08))
#define SCUC_GC_CFG0			(*(volatile unsigned int*)(SCU_C_BASE+0x10))
#define SCUC_DUMMY_C2			(*(volatile unsigned int*)(SCU_C_BASE+0x18))
#define SCUC_SYS_RATIO_UPDATE	(*(volatile unsigned int*)(SCU_C_BASE+0x28))
#define SCUC_ROM_ADDR0_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x2C))
#define SCUC_ROM_DATA0_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x30))
#define SCUC_ROM_DATA1_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x34))
#define SCUC_ROM_DATA2_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x38))
#define SCUC_ROM_DATA3_CYC		(*(volatile unsigned int*)(SCU_C_BASE+0x3C))
#define SCUC_DDRPHY_CTRL0		(*(volatile unsigned int*)(SCU_C_BASE+0x48))
#define SCUC_DDRPHY_CTRL1		(*(volatile unsigned int*)(SCU_C_BASE+0x4C))
#define SCUC_DUMMY_C0			(*(volatile unsigned int*)(SCU_C_BASE+0xA0))
#define SCUC_DUMMY_C1			(*(volatile unsigned int*)(SCU_C_BASE+0xA4))
#define SCUC_CPU_DBG_CTRL		(*(volatile unsigned int*)(SCU_C_BASE+0xA8))
#define SCUC_CPU_DBG_STAT		(*(volatile unsigned int*)(SCU_C_BASE+0xAC))
#define SCUC_NR_REG0			(*(volatile unsigned int*)(SCU_C_BASE+0xB0))
#define SCUC_NR_REG1			(*(volatile unsigned int*)(SCU_C_BASE+0xB4))
#define SCUC_NR_REG2			(*(volatile unsigned int*)(SCU_C_BASE+0xB8))
#define SCUC_NR_REG3			(*(volatile unsigned int*)(SCU_C_BASE+0xBC))
#define SCUC_TAS0				(*(volatile unsigned int*)(SCU_C_BASE+0xC0))
#define SCUC_TAS1				(*(volatile unsigned int*)(SCU_C_BASE+0xC4))
#define SCUC_TAS2				(*(volatile unsigned int*)(SCU_C_BASE+0xC8))
#define SCUC_TAS3				(*(volatile unsigned int*)(SCU_C_BASE+0xCC))
#define SCUC_TAS4				(*(volatile unsigned int*)(SCU_C_BASE+0xD0))
#define SCUC_TAS5				(*(volatile unsigned int*)(SCU_C_BASE+0xD4))
#define SCUC_TAS6				(*(volatile unsigned int*)(SCU_C_BASE+0xD8))
#define SCUC_TAS7				(*(volatile unsigned int*)(SCU_C_BASE+0xDC))
#define SCUC_SYS_RATIO			(*(volatile unsigned int*)(SCU_C_BASE+0x100))
#define SCUC_SYS_RT_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x104))
#define SCUC_SYS_AHB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x108))
#define SCUC_SYS_APB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x10C))
#define SCUC_CEVA_RATIO			(*(volatile unsigned int*)(SCU_C_BASE+0x110))
#define SCUC_CEVA_AHB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x114))
#define SCUC_CEVA_APB_RATIO		(*(volatile unsigned int*)(SCU_C_BASE+0x118))
#define SCUC_CEVA_CNT_EN		(*(volatile unsigned int*)(SCU_C_BASE+0x11C))

/*  SCU_D  Control Unit */
#define SCUD_DRAM_MAP			(*(volatile unsigned int*)(SCU_D_BASE+0x00))
#define SCUD_SB0_RGN			(*(volatile unsigned int*)(SCU_D_BASE+0x04))
#define SCUD_SB1_RGN			(*(volatile unsigned int*)(SCU_D_BASE+0x08))
#define SCUD_PRIM_ST			(*(volatile unsigned int*)(SCU_D_BASE+0x10))
#define SCUD_DMIM_ST			(*(volatile unsigned int*)(SCU_D_BASE+0x18))
#define SCUD_CXS_CTRL			(*(volatile unsigned int*)(SCU_D_BASE+0x28))
#define SCUD_CXP_ADDR			(*(volatile unsigned int*)(SCU_D_BASE+0x2C))
#define SCUD_DUMMY_D0			(*(volatile unsigned int*)(SCU_D_BASE+0x30))
#define SCUD_DUMMY_D1			(*(volatile unsigned int*)(SCU_D_BASE+0x34))
#define SCUD_GCLKEN				(*(volatile unsigned int*)(SCU_D_BASE+0x38))
#define SCUD_DGCLKEN			(*(volatile unsigned int*)(SCU_D_BASE+0x3C))
#define SCUD_CPU_DBG_CTRL		(*(volatile unsigned int*)(SCU_D_BASE+0x48))
#define SCUD_CPU_DBG_STAT		(*(volatile unsigned int*)(SCU_D_BASE+0x4C))


// SCU A  Peripheral Clock bit
#define		SCU_A_PERI_SYSA				(1 << 0)
#define		SCU_A_PERI_LCD_CTRL			(1 << 1)
#define		SCU_A_PERI_DRM				(1 << 2)
#define		SCU_A_PERI_USB0				(1 << 3)
#define		SCU_A_PERI_USB1				(1 << 4)
#define		SCU_A_PERI_LINEBUFFER				(1 << 5)
#define		SCU_A_PERI_SCUA				(1 << 6)
#define		SCU_A_PERI_TVOUT			(1 << 7)
#define		SCU_A_PERI_APBDMA_A			(1 << 9)
#define		SCU_A_PERI_CMOS_CTRL		(1 << 10)
#define		SCU_A_PERI_NAND0			(1 << 11)
#define		SCU_A_PERI_NAND1			(1 << 12)
#define		SCU_A_PERI_BCH				(1 << 13)
#define		SCU_A_PERI_APLL				(1 << 14)
#define		SCU_A_PERI_UART_CNCT		(1 << 15)
#define		SCU_A_PERI_AAHBM_SLICE		(1 << 16)
#define		SCU_A_PERI_I2S				(1 << 17)
#define		SCU_A_PERI_I2SRX			(1 << 18)
#define		SCU_A_PERI_SAACC			(1 << 19)
#define		SCU_A_PERI_NAND_ABT			(1 << 20)
#define		SCU_A_PERI_REALTIME_ABT		(1 << 21)
#define		SCU_A_PERI_RTABT212			(1 << 22)
#define		SCU_A_PERI_CAHBM212			(1 << 23)

// SCU B  Peripheral Clock bit
#define		SCU_B_PERI_TCM_BIST			(1 << 0)
#define		SCU_B_PERI_TCM_CTRL			(1 << 1)
#define		SCU_B_PERI_AHB2AHB			(1 << 2)
#define		SCU_B_PERI_AHB_SW			(1 << 3)
#define		SCU_B_PERI_VIC0				(1 << 4)
#define		SCU_B_PERI_VIC1				(1 << 5)
#define		SCU_B_PERI_DPM				(1 << 6)
#define		SCU_B_PERI_APB_BRG			(1 << 7)
#define		SCU_B_PERI_ARM926           (1 << 8)
#define		SCU_B_PERI_TIMER0			(1 << 9)
#define		SCU_B_PERI_TIMER1			(1 << 10)								
#define		SCU_B_PERI_UART				(1 << 11)								
#define		SCU_B_PERI_I2C				(1 << 12)								
#define		SCU_B_PERI_RAND				(1 << 13)								
#define		SCU_B_PERI_GPIO				(1 << 14)
#define		SCU_B_PERI_RTC				(1 << 15)

// SCU C  Peripheral Clock bit
#define		SCU_C_PERI_FABRIC_C			(1 << 0)
#define		SCU_C_PERI_DMAC0			(1 << 1)
#define		SCU_C_PERI_DMAC1			(1 << 2)
#define		SCU_C_PERI_MEM_CTRL			(1 << 3)
#define		SCU_C_PERI_DRAM_CTRL		(1 << 4)
#define		SCU_C_PERI_SCU_C			(1 << 5)
#define		SCU_C_PERI_I2C_CFG			(1 << 6)
#define		SCU_C_PERI_APBDMA			(1 << 7)								
#define		SCU_C_PERI_2D_ENGIN			(1 << 8)							
#define		SCU_C_PERI_NOR				(1 << 9)	
#define		SCU_C_PERI_CF				(1 << 10)	
#define		SCU_C_PERI_MS				(1 << 11)	
#define		SCU_C_PERI_RAM				(1 << 12)	
#define		SCU_C_PERI_UART_C0			(1 << 13)	
#define		SCU_C_PERI_UART_C1			(1 << 14)
#define		SCU_C_PERI_UART_C2			(1 << 15)	
#define		SCU_C_PERI_SSP0				(1 << 16)
#define		SCU_C_PERI_SSP1				(1 << 17)	
#define		SCU_C_PERI_SD0				(1 << 18)
#define		SCU_C_PERI_SD1				(1 << 19)
#define		SCU_C_PERI_I2C				(1 << 20)
#define		SCU_C_PERI_SCALING			(1 << 21)
#define 	SCU_C_PERI_2DSCALEABT		(1 << 22)
#define 	SCU_C_PERI_TI2C 			(1 << 23)
#define		SCU_C_PERI_FRBRIC_A			(1 << 24)
#define		SCU_C_PERI_CXMP_SL			(1 << 25)
#define		SCU_C_PERI_CXMD_SL			(1 << 26)
#define		SCU_C_PERI_CIR			(1 << 27)
#define		SCU_C_PERI_ROTATOR			(1 << 28)
