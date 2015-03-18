/* include/asm-arm/arch-goldfish/uncompress.h
**
** Copyright (C) 2007 Google, Inc.
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
*/

#ifndef __ASM_ARCH_UNCOMPRESS_H
#define __ASM_ARCH_UNCOMPRESS_H

/*
 * This does not append a newline
 */
#define HAL_READ_UINT32( _register_, _value_ ) \
        ((_value_) = *((volatile unsigned int *)(_register_)))
#define HAL_WRITE_UINT32( _register_, _value_ ) \
        (*((volatile unsigned int *)(_register_)) = (_value_))
       
//#define UART0Base 		(0x92b04000)
#define UART0Base 		(0x92b06000)	//UART2
#define	INTEGRATOR_UART0_BASE	UART0Base


#define UART_RBR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x00)	//Receiver Buffer Register
#define UART_THR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x00)	//Transmitter Holding Register
#define UART_IER	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x04)	//Interrupt Enable Register
#define UART_IIR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x08)	//Interrupt Indicative Register
#define UART_FCR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x08)	//Fifo Control Register
#define UART_LCR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x0C)	//Length Control Register
#define UART_MCR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x10)	//Mode Control Register
#define UART_LSR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x14)	//Status Register
#define UART_MSR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x18)	//Modem Status Register
#define UART_FSR	(volatile unsigned int *)(INTEGRATOR_UART0_BASE+0x20)	//Fifo Status Register

/*-----MSR----------------*/
#define		DCD			0x80
#define		RI			0x40
#define		DSR			0x20
#define		CTS			0x10
#define		DDCD		0x08
#define		TERI		0x04
#define		DDSR		0x02
#define		DCTS		0x01
/*-----FSR----------------*/
#define		RFFUL		0x40
#define		RFHLF		0x20
#define		RFEMT		0x10
#define		TFFUL		0x04
#define		TFHLF		0x02
#define		TFEMT		0x01

static long UART_OutByte(unsigned char data)
{
	int counter = 0;
	int reg = 0;
	int reg2=0;
	

	HAL_READ_UINT32(UART_LSR, reg);
	if (!(reg&0x1e)) // error check (FE error maybe wrong)
	{
		do
		{
			HAL_READ_UINT32(UART_MSR, reg);
			HAL_READ_UINT32(UART_FSR, reg2);
			counter ++;
		}while( (!(reg & CTS) | (reg2 & TFFUL)) && counter < 4000 );
					
		if (counter < 4000)
		{
			HAL_WRITE_UINT32(UART_THR, data);
			return 1;
		}	
		else 
		{
			return -1;
		}
	}
	
	return -1;
}
 
static void putc(int c)
{
	UART_OutByte(c);
}

static inline void flush(void)
{
}

/*
 * nothing to do
 */
#define arch_decomp_setup()

#define arch_decomp_wdog()

#endif
