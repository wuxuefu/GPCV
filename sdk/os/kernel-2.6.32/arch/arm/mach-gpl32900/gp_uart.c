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
 * @file gp_uart.c
 * @brief uart driver
 * @author zh.l
 */
 
#include <mach/module.h>
#include <mach/hal/hal_uart.h>
#include <mach/hal/hal_clock.h>
#include <linux/serial_core.h>
#include <linux/console.h>
#include <asm/irq.h>		/*for request_irq*/
#include <linux/delay.h>	/*for udelay*/

struct gp_uart_port {
	struct uart_port	port;
	unsigned char		ier;	/*interrupt enable register*/
	unsigned char		lcr;	/*line control register*/
	unsigned char		mcr;	/*modem	control	register*/
	unsigned int		lsr_break_flag;
	char			*name;
};

/**
 * @brief   Uart clock enable/disable
 * @param   enable [in] 0:disable, 1:enable
 * @return  None
 * @see
 */
static void uart_clock_enable(struct uart_port *port, int enable)
{
	unsigned int uclk = (port->line == 0)?SCU_C_PERI_UART_C2:SCU_C_PERI_UART_C0;
	gpHalScuClkEnable(uclk, SCU_C, enable);
}

#if 0
/* enable modem status interrupts */
static void gp_uart_enable_ms(struct uart_port *port)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;

	up->ier	|= UART_IER_EDSSI;
	gpHalUartSetIntEn(port->line, up->ier);
}
#endif

static void gp_uart_stop_tx(struct uart_port *port)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;

	if (up->ier & UART_IER_ETBEI) {
		up->ier	&= ~UART_IER_ETBEI;
		gpHalUartSetIntEn(port->line, up->ier);
	}
}

static void gp_uart_stop_rx(struct uart_port *port)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;

	up->ier	&= ~UART_IER_ELSI;
	up->port.read_status_mask &= ~UART_LSR_DR;
	gpHalUartSetIntEn(port->line, up->ier);
}

static void receive_chars(struct gp_uart_port *up, int *status)
{
	struct tty_struct *tty = up->port.state->port.tty;
	unsigned int ch, flag;
	int max_count = 256;	

	do {
		ch = gpHalUartGetChar(up->port.line);

		flag = TTY_NORMAL;
		up->port.icount.rx++;

		if (unlikely(*status & (UART_LSR_BI | UART_LSR_PE |
				       UART_LSR_FE | UART_LSR_OE))) {
			/*
			 * For statistics only
			 */
			if (*status & UART_LSR_BI) {
				*status	&= ~(UART_LSR_FE | UART_LSR_PE);
				up->port.icount.brk++;
				/*
				 * We do the SysRQ and SAK checking
				 * here	because	otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask.
				 */
				if (uart_handle_break(&up->port)) {
					goto ignore_char;
				}
			}
			else if	(*status & UART_LSR_PE)	{
				up->port.icount.parity++;
			}
			else if	(*status & UART_LSR_FE)	{
				up->port.icount.frame++;
			}

			if (*status & UART_LSR_OE) {
				up->port.icount.overrun++;
			}
			/*
			 * Mask	off conditions which should be ignored.
			 */
			*status	&= up->port.read_status_mask;

			if (up->port.line == up->port.cons->index) {
				/* Recover the break flag from console xmit */
				*status	|= up->lsr_break_flag;
				up->lsr_break_flag = 0;
			}

			if (*status & UART_LSR_BI) {
				flag = TTY_BREAK;
			}
			else if	(*status & UART_LSR_PE)	{
				flag = TTY_PARITY;
			}
			else if	(*status & UART_LSR_FE)	{
				flag = TTY_FRAME;
			}
		}

		if (uart_handle_sysrq_char(&up->port, ch)) {
			goto ignore_char;
		}

		uart_insert_char(&up->port, *status, UART_LSR_OE, ch, flag);

	ignore_char:
		*status	= gpHalUartGetLineStatus(up->port.line);
	} while	((*status & UART_LSR_DR) && (max_count-- > 0));
	tty_flip_buffer_push(tty);
}

static void transmit_chars(struct gp_uart_port *up)
{
	struct circ_buf	*xmit =	&up->port.state->xmit;
	int count;

	if (up->port.x_char) {
		gpHalUartPutChar(up->port.line, up->port.x_char);
		up->port.icount.tx++;
		up->port.x_char	= 0;
		return;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(&up->port)) {
		gp_uart_stop_tx(&up->port);
		return;
	}

	count =	up->port.fifosize / 2;
	do {
		gpHalUartPutChar(up->port.line,	xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) &	(UART_XMIT_SIZE	- 1);
		up->port.icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	} while	(--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS) {
		uart_write_wakeup(&up->port);
	}

	if (uart_circ_empty(xmit)) {
		gp_uart_stop_tx(&up->port);
	}
}

static void gp_uart_start_tx(struct uart_port *port)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;

	if (!(up->ier &	UART_IER_ETBEI))	{
		up->ier	|= UART_IER_ETBEI;
		//transmit_chars((struct gp_uart_port *)port);
		gpHalUartSetIntEn(port->line, up->ier);
	}
}

#if 0
static void check_modem_status(struct gp_uart_port *up)
{
	int status;

	status = gpHalUartGetModemStatus(up->port.line);

	if ((status & 0x0f) == 0)
		return;

	if (status & UART_MSR_TERI)
		up->port.icount.rng++;
	if (status & UART_MSR_DDSR)
		up->port.icount.dsr++;
	if (status & UART_MSR_DDCD)
		uart_handle_dcd_change(&up->port, status & UART_MSR_DCD);
	if (status & UART_MSR_DCTS)
		uart_handle_cts_change(&up->port, status & UART_MSR_CTS);

	wake_up_interruptible(&up->port.state->port.delta_msr_wait);
}
#endif

/*
 * This	handles	the interrupt from one port.
 */
static irqreturn_t gp_uart_irq(int irq, void *dev_id)
{
	struct gp_uart_port *up	= dev_id;
	unsigned int iir, lsr;

	iir = gpHalUartGetIntFlags(up->port.line);

	if (iir & UART_IIR_NO_IRQ)
		return IRQ_NONE;

	lsr = gpHalUartGetLineStatus(up->port.line);
	if (lsr & UART_LSR_DR)
		receive_chars(up, &lsr);

#if 0
	check_modem_status(up);
#endif

	lsr = gpHalUartGetFifoStatus(up->port.line);
	if (lsr & UART_FSR_TFEMT)
		transmit_chars(up);

//	if (lsr	& UART_LSR_THRE)
//		transmit_chars(up);

	return IRQ_HANDLED;
}

static unsigned	int gp_uart_tx_empty(struct uart_port *port)
{
	//struct gp_uart_port *up	= (struct gp_uart_port *)port;

	return gpHalUartGetLineStatus(port->line) & UART_LSR_TEMT ? TIOCSER_TEMT : 0;
}

static unsigned	int gp_uart_get_mctrl(struct uart_port *port)
{

	unsigned char status;
	unsigned int ret;

	status = gpHalUartGetModemStatus(port->line);

	ret = 0;
	if (status & UART_MSR_DCD) {
		ret |= TIOCM_CAR;
	}

	if (status & UART_MSR_RI) {
		ret |= TIOCM_RNG;
	}

	if (status & UART_MSR_DSR) {
		ret |= TIOCM_DSR;
	}

	if (status & UART_MSR_CTS) {
		ret |= TIOCM_CTS;
	}
	return ret;
}

static void gp_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;
	unsigned char mcr = 0;

	if (mctrl & TIOCM_RTS) {
		mcr |= UART_MCR_RTS;
	}
	if (mctrl & TIOCM_DTR) {
		mcr |= UART_MCR_DTR;
	}
#if 0 /*our chip don't support*/
	if (mctrl & TIOCM_OUT1)	{
		mcr |= UART_MCR_OUT1;
	}
	if (mctrl & TIOCM_OUT2)	{
		mcr |= UART_MCR_OUT2;
	}
#endif
	if (mctrl & TIOCM_LOOP)	{
		mcr |= UART_MCR_LOOP;
	}
	mcr |= up->mcr;

	gpHalUartSetModemCtrl(port->line, mcr);
}
/*
 * Control the transmission of a break signal
 */
static void gp_uart_break_ctl(struct uart_port *port, int break_state)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;

	if (break_state	!= 0) {
		up->lcr	|= UART_LCR_BRK;	/*start break*/
	}
	else {
		up->lcr	&= ~UART_LCR_BRK;	/*stop break*/
	}
	gpHalUartSetLineCtrl(port->line, up->lcr);
}

/*
 * Perform initialization and enable port for reception
 */
static int gp_uart_startup(struct uart_port *port)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;
	int retval;
	
	/*
	 * Enable UART clock
	 */
	uart_clock_enable( port, 1 );
	
	/*
	 * Ensure that no interrupts are enabled otherwise when
	 * request_irq() is called we could get stuck trying to
	 * handle an unexpected interrupt
	 */
	gpHalUartSetIntEn(port->line, 0);

	/*
	 * Allocate the	IRQ
	 */
	retval = request_irq(up->port.irq, gp_uart_irq,	0, up->name, up);
	if (retval){
		printk("[%s][%d]\n", __FUNCTION__, __LINE__);
		return retval;
	}

	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reenabled in set_termios())
	 */
	gpHalUartSetFifoCtrl(port->line, UART_FCR_FFE);
	gpHalUartSetFifoCtrl(port->line, UART_FCR_FFE |
			UART_FCR_RFRST | UART_FCR_TFRST);
	gpHalUartSetFifoCtrl(port->line, 0);

	/*
	 * Clear the interrupt registers.
	 */
	(void) gpHalUartClrIntFlags(port->line);

	/*
	 * Now,	initialize the UART
	 */
	gpHalUartSetLineCtrl(port->line, UART_LCR_WLS8);

	gp_uart_set_mctrl(&up->port, up->port.mctrl);

	/*
	 * Finally, enable interrupts.	Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	up->ier	= UART_IER_ELSI	| UART_IER_ERBFI;
	gpHalUartSetIntEn(port->line, up->ier);
	/*
	 * And clear the interrupt registers again for luck.
	 */
	(void) gpHalUartClrIntFlags(port->line);
	return 0;
}

static void gp_uart_shutdown(struct uart_port *port)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;

	/*
	 * Disable interrupts from this	port
	 */
	up->ier	= 0;
	gpHalUartSetIntEn(port->line, up->ier);
	free_irq(up->port.irq, up);

	up->port.mctrl &= ~TIOCM_OUT2;
	gp_uart_set_mctrl(&up->port, up->port.mctrl);

	/*
	 * Disable break condition and FIFOs
	 */
	gpHalUartSetLineCtrl(port->line, gpHalUartGetLineCtrl(port->line) & ~UART_LCR_BRK);
	gpHalUartSetFifoCtrl(port->line, UART_FCR_FFE | UART_FCR_RFRST | UART_FCR_TFRST);
	gpHalUartSetFifoCtrl(port->line, 0);
	uart_clock_enable( port, 0 );
}

static void
gp_uart_set_termios(struct uart_port *port, struct ktermios *termios,
		       struct ktermios *old)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;
	unsigned char /*ier,*/ cval, fcr	= 0;
	unsigned int baud;

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		cval = UART_LCR_WLS5;
		break;
	case CS6:
		cval = UART_LCR_WLS6;
		break;
	case CS7:
		cval = UART_LCR_WLS7;
		break;
	case CS8:
		/*falling down,default CS8*/
	default:	
		cval = UART_LCR_WLS8;
		break;
	}
	
	/* stop bits */
	if (termios->c_cflag & CSTOPB)
		cval |=	UART_LCR_STB;
	
	/*parity*/
	if (termios->c_cflag & PARENB) {
		/*Mark or Space parity*/
		if (termios->c_cflag & CMSPAR) {
			if (termios->c_cflag & PARODD) {
				cval |= UART_LCR_SPS; /*mark parity*/
			} else {
				cval |= UART_LCR_SPS | UART_LCR_EPS; /*space parity*/
			}
		} else if (!(termios->c_cflag & PARODD)) {
			cval |= UART_LCR_EPS;	/*even parity*/
		}
		cval |=	UART_LCR_PEN;
	}
	
	/*
	 * Ask the core	to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port,	termios, old, 0, port->uartclk);

	fcr = UART_FCR_FFE | UART_FCR_RFTRG_8;
	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */

	gpHalUartSetIntEn(port->line, 0); /* disable UART interrupt */
	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	up->port.read_status_mask = UART_LSR_OE	| UART_LSR_THRE	| UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		up->port.read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (BRKINT | PARMRK))
		up->port.read_status_mask |= UART_LSR_BI;

	/*
	 * Characters to ignore
	 */
	up->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		up->port.ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		up->port.ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and	break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			up->port.ignore_status_mask |= UART_LSR_OE;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag &	CREAD) == 0)
		up->port.ignore_status_mask |= UART_LSR_DR;

#if 0
	/*
	 * CTS flow control flag and modem status interrupts
	 */
	up->ier	&= ~UART_IER_EDSSI;
	if (UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier	|= UART_IER_EDSSI;

	gpHalUartSetIntEn(port->line, up->ier);
#endif

	gpHalUartSetLineCtrl( port->line, cval );
	gpHalUartSetBaud( port->line, baud);
	up->lcr = gpHalUartGetLineCtrl(port->line);
	gp_uart_set_mctrl(&up->port, up->port.mctrl);
	gpHalUartSetFifoCtrl( port->line, fcr);
	gpHalUartSetIntEn(port->line, up->ier); /* enable UART interrupt */
}

static void
gp_uart_pm(struct uart_port *port, unsigned int	state, unsigned	int oldstate)
{
	if (!state){
		uart_clock_enable( port, 1 );
	} else{
		uart_clock_enable( port, 0 );
	}
}

/*
 * Release the memory region(s) being used by 'port'.
 * we don't claim any resources, so nothing to do
 */
static void gp_uart_release_port(struct	uart_port *port)
{

}
/*
 * Request the memory region(s) being used by 'port'.
 * we don't claim any resources, so nothing to do
 */
static int gp_uart_request_port(struct uart_port *port)
{
	return 0;
}

/*
 * Configure/autoconfigure the port.
 */
static void gp_uart_config_port(struct uart_port *port,	int flags)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;
	up->port.type =	PORT_SPMP8000;
}

/*
 * Verify the new serial_struct (for TIOCSSERIAL).
 */
static int
gp_uart_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	/* we don't want the core code to modify any port params */
	return -EINVAL;
}

static const char *
gp_uart_type(struct uart_port *port)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;
	return up->name;
}


static struct gp_uart_port gp_uart_ports[];
static struct uart_driver gp_uart_reg;

#define	BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

/*
 *	Wait for transmitter & holding register	to empty
 */
static int wait_for_xmitr(struct gp_uart_port *up)
{
	unsigned int status, tmout = 2000;

	/* Wait	up to 2ms for the character(s)	to be sent. */
	do {
		status = gpHalUartGetLineStatus(up->port.line);
		if (status & UART_LSR_BI)
			up->lsr_break_flag = UART_LSR_BI;

		if (--tmout == 0)
			break;
		udelay(1);
	} while	((status & BOTH_EMPTY) != BOTH_EMPTY);

#if 0
	/* Wait	up to 1s for flow control if necessary */
	if (up->port.flags & UPF_CONS_FLOW) {
		tmout =	1000000;
		while (--tmout &&
		       ((gpHalUartGetModemStatus(up->port.line) & UART_MSR_CTS) == 0))
			udelay(1);
	}
#endif

	return 0;
}

static void gp_uart_console_putchar(struct uart_port *port, int	ch)
{
	struct gp_uart_port *up	= (struct gp_uart_port *)port;

	wait_for_xmitr(up);
	gpHalUartPutChar( port->line,  ch);
}

/*
 * Print a string to the serial	port trying not	to disturb
 * any possible	real use of the	port...
 *
 *	The console_lock must be held when we get here.
 */
static void
gp_uart_console_write(struct console *co, const	char *s, unsigned int count)
{
	struct gp_uart_port *up	= &gp_uart_ports[co->index];
	unsigned ier;
	/*
	 *First save the IER then disable the interrupts
	 */
	ier = gpHalUartGetIntEn(up->port.line);
	gpHalUartSetIntEn(up->port.line,  0);
	uart_console_write(&up->port, s, count,	gp_uart_console_putchar);

	/*
	 *Finally, wait for transmitter to become empty
	 *and restore the IER
	 */
	wait_for_xmitr(up);
	gpHalUartSetIntEn(up->port.line, ier);
}

static int __init
gp_uart_console_setup(struct console *co, char *options)
{
	struct gp_uart_port *up;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	if (co->index == -1 || co->index >= gp_uart_reg.nr)
		co->index = 0;
	up = &gp_uart_ports[co->index];
	
	uart_clock_enable( &up->port, 1 );
	up->port.uartclk = gpHalUartGetClkRate(up->port.line);
	
	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	return uart_set_options(&up->port, co, baud, parity, bits, flow);
}

static struct console gp_uart_console =	{
	.name		= "ttyS",
	.write		= gp_uart_console_write,
	.device		= uart_console_device,
	.setup		= gp_uart_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &gp_uart_reg,
};


struct uart_ops	gp_uart_pops = {
	.tx_empty	= gp_uart_tx_empty,
	.set_mctrl	= gp_uart_set_mctrl,
	.get_mctrl	= gp_uart_get_mctrl,
	.stop_tx	= gp_uart_stop_tx,
	.start_tx	= gp_uart_start_tx,
	.stop_rx	= gp_uart_stop_rx,
#if 0
	.enable_ms	= gp_uart_enable_ms,
#endif
	.break_ctl	= gp_uart_break_ctl,
	.startup	= gp_uart_startup,
	.shutdown	= gp_uart_shutdown,
	.set_termios	= gp_uart_set_termios,
	.pm		= gp_uart_pm,
	.type		= gp_uart_type,
	.release_port	= gp_uart_release_port,
	.request_port	= gp_uart_request_port,
	.config_port	= gp_uart_config_port,
	.verify_port	= gp_uart_verify_port,
};

static struct gp_uart_port gp_uart_ports[] = {
	{
		.name	= "UARTC2",
		.port	= {
			.type		= PORT_SPMP8000,
			.iotype		= UPIO_MEM,
			.membase	= (void	*)UART2_BASE,
			.irq		= IRQ_UART_C2,
			.uartclk	= 0,
			.fifosize	= 16,
			.ops		= &gp_uart_pops,
			.line		= 0,
		},
	},
	{
		.name	= "UARTC0",
		.port	= {
			.type		= PORT_SPMP8000,
			.iotype		= UPIO_MEM,
			.membase	= (void	*)UART0_BASE,
			.irq		= IRQ_UART_C0,
			.uartclk	= 0,
			.fifosize	= 16,
			.ops		= &gp_uart_pops,
			.line		= 1,
		},
	}
};

static struct uart_driver gp_uart_reg =	{
	.owner		 = THIS_MODULE,
	.driver_name	= "gp serial",
	.dev_name	= "ttyS",
	.major		= TTY_MAJOR,
	.minor		= 64,
	.nr		= ARRAY_SIZE(gp_uart_ports),
	.cons		=  &gp_uart_console,
};

static int gp_uart_suspend(struct platform_device *dev,	pm_message_t state)
{
	struct gp_uart_port *sport = platform_get_drvdata(dev);

	if (sport)
		uart_suspend_port(&gp_uart_reg,	&sport->port);

	return 0;
}

static int gp_uart_resume(struct platform_device *dev)
{
	struct gp_uart_port *sport = platform_get_drvdata(dev);

	if (sport) {
		uart_resume_port(&gp_uart_reg, &sport->port);
	}

	return 0;
}

static int gp_uart_probe(struct	platform_device	*dev)
{
	gp_uart_ports[dev->id].port.dev	= &dev->dev;
	gp_uart_ports[dev->id].port.uartclk = gpHalUartGetClkRate(dev->id);

	uart_add_one_port(&gp_uart_reg, &gp_uart_ports[dev->id].port);
	platform_set_drvdata(dev, &gp_uart_ports[dev->id]);
	return 0;
}

static int gp_uart_remove(struct platform_device *dev)
{
	struct gp_uart_port *sport = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	if (sport) {
		uart_remove_one_port(&gp_uart_reg, &sport->port);
	}

	return 0;
}


/**
 * @brief   uart device release
 */
static void gp_uart_device_release(struct device *dev)
{
	printk("remove uart device ok");
}

static struct platform_driver gp_uart_driver = {
	.probe		= gp_uart_probe,
	.remove		= gp_uart_remove,

	.suspend	= gp_uart_suspend,
	.resume		= gp_uart_resume,
	.driver		= {
		.name	= "gp-uart",
		.owner	= THIS_MODULE,
	},
};

static struct platform_device uart0_device = {
	.name		= "gp-uart",
	.id		= 0,
	.dev	= {
		.release = gp_uart_device_release
	}
};

static struct platform_device uart1_device = {
	.name		= "gp-uart",
	.id		= 1,
	.dev	= {
		.release = gp_uart_device_release
	}
};


static struct platform_device *gp_uart_devices[] __initdata = {
	&uart0_device,
	&uart1_device,
};

int __init gp_uart_module_init(void)
{
	int ret;

	ret = uart_register_driver(&gp_uart_reg);
	if (ret	!= 0)
		return ret;
	platform_add_devices(gp_uart_devices, ARRAY_SIZE(gp_uart_devices));
	ret = platform_driver_register(&gp_uart_driver);
	if (ret	!= 0) {
		uart_unregister_driver(&gp_uart_reg);
	}

	return ret;
}

void __exit gp_uart_module_exit(void)
{
	platform_driver_unregister(&gp_uart_driver);
	uart_unregister_driver(&gp_uart_reg);
}

module_init(gp_uart_module_init);
module_exit(gp_uart_module_exit);

MODULE_LICENSE_GP;


