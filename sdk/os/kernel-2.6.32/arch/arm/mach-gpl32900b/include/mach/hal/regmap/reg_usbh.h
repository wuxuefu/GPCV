/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Technology Co., Ltd.         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Technology Co., Ltd. All rights are reserved by Generalplus Technology*
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Technology Co., Ltd. reserves the right to modify this    *
 *  software without notice.                                              *
 *                                                                        *
 *  Generalplus Technology Co., Ltd.                                      *
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/
#ifndef _REG_USBH_H_
#define _REG_USBH_H_

#include <mach/hardware.h>                          
#include <mach/typedef.h>


/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

#define	LOGI_ADDR_USBH0_REG		IO3_ADDRESS(0x4000)
#define	LOGI_ADDR_USBH1_REG		IO3_ADDRESS(0x5000)

#define	LOGI_ADDR_OHCI0_REG		(LOGI_ADDR_USBH0_REG+0x0080)
#define	LOGI_ADDR_EHCI0_REG		(LOGI_ADDR_USBH0_REG+0x0100)

#define	LOGI_ADDR_OHCI1_REG		(LOGI_ADDR_USBH1_REG+0x0080)
#define	LOGI_ADDR_EHCI1_REG		(LOGI_ADDR_USBH1_REG+0x0100)

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

struct gp_ohci_regs {
	/* control and status registers (setion 7.1) */
	volatile UINT32	revision;
	volatile UINT32	control;
	volatile UINT32	cmdstatus;
	volatile UINT32	intrstatus;
	volatile UINT32	intrenable;
	volatile UINT32	intrdisable;

	/* memory pointers (section 7.2) */
	volatile UINT32	hcca;
	volatile UINT32	ed_periodcurrent;
	volatile UINT32	ed_controlhead;
	volatile UINT32	ed_controlcurrent;
	volatile UINT32	ed_bulkhead;
	volatile UINT32	ed_bulkcurrent;
	volatile UINT32	donehead;

	/* frame counters (section 7.3) */
	volatile UINT32	fminterval;
	volatile UINT32	fmremaining;
	volatile UINT32	fmnumber;
	volatile UINT32	periodicstart;
	volatile UINT32	lsthresh;

	/* Root hub ports (section 7.4) */
	struct	ohci_roothub_regs {
		volatile UINT32	a;
		volatile UINT32	b;
		volatile UINT32	status;
#define MAX_ROOT_PORTS	2	/* maximum OHCI root hub ports (RH_A_NDP) */
		volatile UINT32	portstatus [MAX_ROOT_PORTS];
	} roothub;
	
} __attribute__ ((aligned(32)));

/* Section 2.2 Host Controller Capability Registers */
struct gp_ehci_caps {
	volatile UINT32		hc_capbase;
	volatile UINT32		hcs_params;			/* HCSPARAMS - offset 0x4 */
	volatile UINT32		hcc_params;			/* HCCPARAMS - offset 0x8 */
	volatile UINT8		portroute [8];		/* nibbles for routing - offset 0xC */
} __attribute__ ((packed));

/* Section 2.3 Host Controller Operational Registers */
struct gp_ehci_regs {
	volatile UINT32		command;			/* USBCMD: offset 0x00 */
	volatile UINT32		status;				/* USBSTS: offset 0x04 */
	volatile UINT32		intr_enable;		/* USBINTR: offset 0x08 */
	volatile UINT32		frame_index;		/* FRINDEX: offset 0x0C, current microframe number */
	volatile UINT32		segment;			/* CTRLDSSEGMENT: offset 0x10, address bits 63:32 if needed */
	volatile UINT32		frame_list;			/* PERIODICLISTBASE: offset 0x14, points to periodic list */
	volatile UINT32		async_next;			/* ASYNCLISTADDR: offset 0x18, address of next async queue head */
	volatile UINT32		reserved [9];
	volatile UINT32		configured_flag;	/* CONFIGFLAG: offset 0x40 */
	volatile UINT32		port_status [2];	/* up to N_PORTS */
} __attribute__ ((packed));

typedef struct usbhReg_s {
	volatile UINT32 version;			/* offset 0x0000: USB HOST Version */
	volatile UINT32 ctrl;				/* offset 0x0004: USB HOST Control */
	volatile UINT32 rsv008[0x02];
	volatile UINT32 pwr_ctrl_p1;		/* offset 0x0010: PORT1 USB HOST POWER CTRL */
	volatile UINT32 pwr_ctrl_p2;		/* offset 0x0014: PORT2 USB HOST POWER CTRL */
} usbhReg_t;

#endif /* _REG_USBH_H_ */


