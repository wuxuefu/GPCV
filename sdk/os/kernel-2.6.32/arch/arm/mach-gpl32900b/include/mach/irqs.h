/* include/asm-arm/arch-spmp8000/irqs.h
**
**
*/

#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

//////////  define for the VIC0

#define IRQ_DMAC0_M410_CH0            	   	0
#define IRQ_DMAC0_M410_CH1            	   	1
#define IRQ_ON2_DECODE	            	   	2
#define IRQ_ON2_ENCODE	            	   	3
#define IRQ_2D_ENGINE						4
#define IRQ_EMAC		            	   	5
#define IRQ_CAN			            	   	6
#define IRQ_TIMERINT0						7
#define IRQ_TIMERINT1						8
#define IRQ_TIMERINTX						9
#define IRQ_WDT0							10
#define IRQ_WDT1							11
#define IRQ_WDTX							12
#define IRQ_I2STX							13
#define IRQ_I2SRX							14
#define IRQ_CDSP							15
#define IRQ_USB_DEV							16		// device
#define IRQ_USB_EHCI0                  		17		// host
#define IRQ_USB_OHCI0                  		18		// host
#define IRQ_PPU_SENSOR	                    19
#define IRQ_PPU		                    	20
#define IRQ_ARM_DBG	                    	21
#define IRQ_SCALE_ENGINE_1                 	22
#define IRQ_RESERVED0	 					23
#define IRQ_APBDMA_A_CH0	                24
#define IRQ_APBDMA_A_CH1	            	25
#define IRQ_SCALE_ENGINE_0	             	26
#define IRQ_DISPLAY			             	27
#define IRQ_MIPI_RX1						28
#define IRQ_I2STX1							29		// For HDMI
#define IRQ_RESERVED1						30
#define IRQ_FD								31		// Face detection

///////////  define for VIC1

#define IRQ_GPIO0							32
#define IRQ_GPIO1							33
#define IRQ_GPIO4							34
#define IRQ_I2C_B							35		// ARM subsystem
#define IRQ_I2C_C							36		// system
#define IRQ_REALTIME_CLOCK					37		//
#define IRQ_USB_EHCI1						38
#define IRQ_USB_OHCI1						39
#define IRQ_SD0								40
#define IRQ_SD1								41
#define IRQ_MIPI_RX0						42
#define IRQ_BCH								43
#define IRQ_OVG								44		// Open VG
#define IRQ_GPIO2                           45
#define IRQ_GPIO3                           46
#define IRQ_RESERVED2						47
#define IRQ_CIR								48		// REMOTE CONTROLLER
#define IRQ_LBP								49
#define IRQ_NAND0							50
#define IRQ_SPU								51
#define IRQ_SSP0							52
#define IRQ_SSP1							53
#define IRQ_SD2								54
#define IRQ_TI2C							55		// Turbo I2C
#define IRQ_UART_C0							56
#define IRQ_AES								57
#define IRQ_UART_C2							58
#define IRQ_SAACC							59
#define IRQ_APBDMA_C_CH0					60
#define IRQ_APBDMA_C_CH1					61
#define IRQ_APBDMA_C_CH2					62
#define IRQ_APBDMA_C_CH3					63

#define MIN_IRQ_NUM							0
#define MAX_IRQ_NUM							63
#define NR_IRQS								64

#endif
