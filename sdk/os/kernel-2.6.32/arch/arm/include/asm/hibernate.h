//----------------------------------------------------------------------------
#ifndef _HIBERNATE_H_
#define _HIBERNATE_H_
//----------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/mm.h>
//----------------------------------------------------------------------------
/*
 * Image of the saved processor state
 *
 * coprocessor 15 registers(RW)
 */
struct saved_context {
	u32 cr;
	u32 cacr;
	u32 ttb0;
	u32 ttb1;
	u32 ttbcr;
	u32 dacr;
	u32 dfsr;
	u32 ifsr;
	u32 dfar;
	u32 wfar;
	u32 ifar;
	u32 dclr;
	u32 iclr;
	u32 dtcmr;
	u32 itcmr;
	u32 tcmsel; 
	u32 cbor;
	u32 tlblr;
	u32 prrr;
	u32 nrrr;
	u32 snsvbar;
	u32 mvbar;
	u32 fcse;
	u32 cid;
	u32 urwtpid;
	u32 urotpid;
	u32 potpid;
	u32 pmrr;
	u32 pmcr;
	u32 pmcc;
	u32 pmc0;
	u32 pmc1;
} __attribute__((packed));

/* Used in hibernate_asm.S */
#define USER_CONTEXT_SIZE (15 * sizeof(u32))
extern unsigned long saved_context_r0[USER_CONTEXT_SIZE];
extern unsigned long saved_cpsr;
extern unsigned long saved_context_r13_svc;
extern unsigned long saved_context_r14_svc;
extern unsigned long saved_spsr_svc;

extern struct saved_context saved_context;
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
