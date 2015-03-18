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
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file hal_uart.c
 * @brief uart driver HAL layer
 * @author zh.l
 */
 
#include <linux/module.h>
#include <linux/clk.h>
#include <mach/hal/hal_uart.h>	/*for function declare*/

static regs_uart_t* pUartPorts[]= {
	((regs_uart_t*)(UART2_BASE)),
	((regs_uart_t*)(UART0_BASE)),
};
#define UART_PORT_NUM	(sizeof(pUartPorts)/sizeof(pUartPorts[0]))
/**
* @brief UART init function.
* @param device_id[in] : Index of uart device
* @param baud[in] : baudrate value
*/
void gpHalUartInit(UINT32 device_id, uart_cfg_t *puartCfg)
{
	if(device_id >= UART_PORT_NUM) 
		return;
	gpHalUartSetWordLen(device_id,puartCfg->bits_per_word);
	gpHalUartSetBaud(device_id, puartCfg->baudrate);
	gpHalUartSetAutoFlowCtrl(device_id, puartCfg->auto_flow_ctrl);
	gpHalUartSetParity(device_id,puartCfg->parity);
	gpHalUartSetStopBit(device_id,puartCfg->stop_bits);
}
EXPORT_SYMBOL(gpHalUartInit);


/**
* @brief UART write data function.
* @param device_id[in] : Index of uart device
* @param buffer[in] : write data buffer
* @param len[in] : write data len
*/
void gpHalUartWrite(UINT32 device_id,char *buffer, UINT32 len)
{
	if(device_id >= UART_PORT_NUM) 
		return;
	/*if FIFO not full,write to FIFO. else wait...*/
	while( len-- ) {
		while( pUartPorts[device_id]->regFSR & UART_FSR_TFFUL )
			;
		pUartPorts[device_id]->regTHR = *buffer++;
	}
}
EXPORT_SYMBOL(gpHalUartWrite);

/**
* @brief UART read data function.
* @param device_id[in] : Index of uart device
* @param buffer[in] : read data buffer
* @param len[in] : read data len
* @return data bytes actually read
*/
int gpHalUartRead(UINT32 device_id,char *buffer, UINT32 len)
{
	int length = 0;
	if ((device_id >= UART_PORT_NUM) ||(NULL==buffer)) 
		return 0;
	while( !(pUartPorts[device_id]->regFSR & UART_FSR_RFEMT) && length<len ) {
		*buffer++ = (UINT8)pUartPorts[device_id]->regRBR;
		length ++;
	}
	return length;
}
EXPORT_SYMBOL(gpHalUartRead);

/**
* @brief UART set freq function.
* @param device_id[in] : Index of uart device
* @param freq[in] : freq value(?? what frequency??)
*/
void gpHalUartSetBaud(UINT32 device_id,UINT32 baud)
{
	struct clk *uart_clk;
	unsigned long uart_clk_rate;
	unsigned char bt;							// bit time length
	unsigned char sp;							// sample point
	unsigned short div;							// divisor
	unsigned long tolerance = baud * 3 /100;	// 3 % tolerance
	
	if(device_id >= UART_PORT_NUM || 0==baud) 
		return;
	uart_clk = clk_get(NULL, "clk_uart");
	uart_clk_rate = clk_get_rate(uart_clk);
	clk_put(uart_clk);
	/* ----- scale down to avoid overflow ----- */
	uart_clk_rate = uart_clk_rate/100;
	baud = baud/100;

	for( bt=15 ; bt>=3 ; bt-=2)
	{
		unsigned long out_baud;
		unsigned long diff;
		div	= ( ( uart_clk_rate * 10 / bt / baud ) + 5 ) / 10  ;		/* round it to the nearest tenth */
		out_baud = uart_clk_rate / bt / div ;
		diff = ( out_baud > baud ) ? ( out_baud - baud ) : ( baud - out_baud ) ;
		if( diff < tolerance )
		{
			sp = (bt+1)/2;
			pUartPorts[device_id]->regLCR |= UART_LCR_DLAB;	/* set DLAB */
			pUartPorts[device_id]->regDLL = (div-1) & 0xff;		/* LS of divisor */
			pUartPorts[device_id]->regDLM = (div-1) >> 8;	/* MS of divisor */
			pUartPorts[device_id]->regLCR &= ~UART_LCR_DLAB;	/* reset DLAB */
			pUartPorts[device_id]->regSPR = ((sp-1)<<4) | (bt-1);
			return ;
		}
	}
}
EXPORT_SYMBOL(gpHalUartSetBaud);

/**
* @brief UART set word length(5/6/7/8 bits per word).
* @param device_id[in] : Index of uart device
* @param len[in] : word length value
*/
void gpHalUartSetWordLen(UINT32 device_id,UINT32 len)
{
	UINT32 temp;
	if ((device_id >= UART_PORT_NUM) || (len>8 || len<5) )
		return;

	temp = pUartPorts[device_id]->regLCR;
	temp &= ~0x03;
	temp |= len-5;
	pUartPorts[device_id]->regLCR = temp;
}
EXPORT_SYMBOL(gpHalUartSetWordLen);

/**
* @brief UART set parity function.
* @param device_id[in] : Index of uart device
* @param parity [in] : parity method
	should be one of [set to 1 | clear to 0 | even | odd | none ]
*/
void gpHalUartSetParity (UINT32 device_id,UINT32 parity)
{
	UINT32 temp;
	if (device_id >= UART_PORT_NUM)
		return;

	temp = pUartPorts[device_id]->regLCR;
	temp &= ~(UART_LCR_PEN | UART_LCR_EPS | UART_LCR_SPS);
	switch (parity) {
	case PARITY_ONE:
		temp |= UART_LCR_PEN | UART_LCR_SPS;
		break;
	case PARITY_ZERO:
		temp |= UART_LCR_PEN | UART_LCR_EPS | UART_LCR_SPS;
		break;
	case PARITY_EVEN:
		temp |= UART_LCR_PEN | UART_LCR_EPS;
		break;
	case PARITY_ODD:
		temp |= UART_LCR_PEN;
		break;
	case PARITY_NONE:
		/*falling down,default no parity*/
	default:
		break;
	};
	pUartPorts[device_id]->regLCR = temp;
}
EXPORT_SYMBOL(gpHalUartSetParity);

/**
* @brief UART set stop bits function.
* @param device_id[in] : Index of uart device
* @param stop_bit [in] : stop bits 1/1.5/2 stop bits
*/
void gpHalUartSetStopBit(UINT32 device_id,UINT32 stop_bit)
{
	UINT32 temp;
	if (device_id >= UART_PORT_NUM)
		return;

	temp = pUartPorts[device_id]->regLCR;
	temp &= ~UART_LCR_STB;
	if( (ONEHALF_STOP_BITS == stop_bit) || (TWO_STOP_BITS == stop_bit) )
		temp |= UART_LCR_STB;
	pUartPorts[device_id]->regLCR = temp;
}
EXPORT_SYMBOL(gpHalUartSetStopBit);

/**
* @brief UART set auto flow control function.
* @param device_id[in] : Index of uart device
* @param mode[in] : auto hardware flow control mode value
*/
void gpHalUartSetAutoFlowCtrl(UINT32 device_id,UINT32 mode)
{
	UINT32 temp;
	if (device_id >= UART_PORT_NUM)
		return;

	temp = pUartPorts[device_id]->regMCR;
	temp &= ~UART_MCR_AFC;
	if( 0!=mode )
		temp |= UART_MCR_AFC;
	pUartPorts[device_id]->regMCR = temp;
}
EXPORT_SYMBOL(gpHalUartSetAutoFlowCtrl);
/**
* @brief UART set modem function.
* @param device_id[in] : Index of uart device
* @param modem [in] : modem value
*/
void gpHalUartSetModemCtrl(UINT32 device_id,UINT32 modem)
{
	//UINT32 temp;
	if (likely(device_id < UART_PORT_NUM))
		pUartPorts[device_id]->regMCR = modem;
}

/**
* @brief UART get modem status register
* @param device_id[in] : Index of uart device
* @param modem [out] : modem status
*/
UINT32 gpHalUartGetModemStatus(UINT32 device_id)
{

	if (likely(device_id < UART_PORT_NUM))
		return pUartPorts[device_id]->regMSR;
	else
		return 0;
}
EXPORT_SYMBOL(gpHalUartGetModemStatus);

/**
* @brief UART set interrupt enable register
* @param device_id[in] : Index of uart device
* @param int_state[out] : interrupt enable state
*/
void gpHalUartSetIntEn(unsigned device_id, unsigned int_state)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		pUartPorts[device_id]->regIER =  int_state;
	}
}
EXPORT_SYMBOL(gpHalUartSetIntEn);

/**
* @brief UART get interrupt enable register
* @param device_id[in] : Index of uart device
* @return interrupt enable state
*/
unsigned gpHalUartGetIntEn(unsigned device_id)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		return pUartPorts[device_id]->regIER;
	} else
		return 0;
}
EXPORT_SYMBOL(gpHalUartGetIntEn);

unsigned gpHalUartGetIntFlags(unsigned device_id)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		return pUartPorts[device_id]->regIIR;
	} else
		return 0;
}
EXPORT_SYMBOL(gpHalUartGetIntFlags);

int gpHalUartGetChar(unsigned device_id)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		return pUartPorts[device_id]->regRBR;
	} else
		return -1;
}
EXPORT_SYMBOL(gpHalUartGetChar);

void gpHalUartPutChar(unsigned device_id, unsigned ch)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		pUartPorts[device_id]->regTHR = ch;
	}
}
EXPORT_SYMBOL(gpHalUartPutChar);

unsigned gpHalUartGetLineStatus( unsigned device_id)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		return pUartPorts[device_id]->regLSR;
	} else
		return 0;
}
EXPORT_SYMBOL(gpHalUartGetLineStatus);

void gpHalUartSetLineCtrl( unsigned device_id, unsigned ctrl)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		pUartPorts[device_id]->regLCR = ctrl;
	}
}
EXPORT_SYMBOL(gpHalUartSetLineCtrl);

unsigned gpHalUartGetFifoStatus(unsigned device_id)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		return pUartPorts[device_id]->regFSR;
	} else
		return 0;
}
EXPORT_SYMBOL(gpHalUartGetFifoStatus);

void gpHalUartSetFifoCtrl(unsigned device_id, unsigned ctrl)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		pUartPorts[device_id]->regFCR = ctrl;
	}
}
EXPORT_SYMBOL(gpHalUartSetFifoCtrl);

unsigned gpHalUartGetLineCtrl(unsigned device_id)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		return pUartPorts[device_id]->regLCR;
	} else
		return 0;
}
EXPORT_SYMBOL(gpHalUartGetLineCtrl);

void gpHalUartClrIntFlags(unsigned device_id)
{
	if( likely(device_id < UART_PORT_NUM) ) {
		(void)  pUartPorts[device_id]->regLSR;
		(void)  pUartPorts[device_id]->regRBR;
		(void)  pUartPorts[device_id]->regIIR;
		(void)  pUartPorts[device_id]->regMSR;
	}
}
EXPORT_SYMBOL(gpHalUartClrIntFlags);

/**
* @brief UART get clock rate
* @param device_id[in] : Index of uart device
* @return clock rate of specified uart port
*/
unsigned int gpHalUartGetClkRate(unsigned device_id)
{
	struct clk *uart_clk;
	int clk_rate = 27000000ul;	/*default 27MHz*/
	unsigned char bt;							// bit time length
	unsigned short div;							// divisor
	
	uart_clk = clk_get(NULL, "clk_uart");
	if (uart_clk) 
	{
		clk_rate = clk_get_rate(uart_clk);
		clk_put(uart_clk);
	}
	pUartPorts[device_id]->regLCR |= UART_LCR_DLAB;	/* set DLAB */
	div = pUartPorts[device_id]->regDLM ;			/* LS of divisor */
	div = (div<<8) | pUartPorts[device_id]->regDLL;	/* MS of divisor */
	pUartPorts[device_id]->regLCR &= ~UART_LCR_DLAB;	/* reset DLAB */
	div = div + 1 ;
	bt = (pUartPorts[device_id]->regSPR & 0xf) + 1;
	return clk_rate / bt / div ;
}
EXPORT_SYMBOL(gpHalUartGetClkRate);
