/*
 * Hibernation support specific for ARM
 *
 * Copyright (C) 2010 Nokia Corporation
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Copyright (C) 2006 Rafael J. Wysocki <rjw@xxxxxxx>
 *
 * Contact: Hiroshi DOYU <Hiroshi.DOYU@xxxxxxxxx>
 *
 * License terms: GNU General Public License (GPL) version 2
 */

#include <linux/linkage.h>
	.arm
	.text
ENTRY(pwr_arch_suspend)
	/*
	 * pwr_arch_suspend(save_area)
	 * Save current program status register
	 */
	ldr     r3, .Lctx_backup_addr
	str		r0, [r3]
	 
	/* ldr     r3, .Lsaved_cpsr */
	mov		r3, r0
	mrs     r0, cpsr
	str     r0, [r3]
	add		r3, r3, #0x4
	
	/* add signature 0x4*/
	mov		r1, #0x11
	str     r1, [r3]
	add		r3, r3, #0x4

	/*
	 * Change to system(user) mode
	 */
	mov     r1, r0
	orr     r1, r1, #0x1f
	msr     cpsr_c, r1

	/*
	 * Save User context
	 * 0x8
	 */
	/* ldr     r3, .Lsaved_context_r0 */
	stmia   r3, {r0-r14}
	add		r3, r3, #0x3C

	/*
	 * Go back to original SVC mode
	 */
	msr     cpsr_c, r0

	/* add signature 0x44*/
	mov		r1, #0x22
	str     r1, [r3]
	add		r3, r3, #0x4

	/*
	 * Save SVC context 0x48
	 */
	/* ldr     r3, .Lsaved_context_r13_svc */
	stmia   r3, {r13-r14}
	add		r3, r3, #0x8
	
	/* add signature 0x50 */
	mov		r1, #0x33
	str     r1, [r3]
	add		r3, r3, #0x4
	
	/* ldr     r3, .Lsaved_spsr_svc */
	/* 0x54 */
	mrs     r1, spsr
	str     r1, [r3]
	add		r3, r3, #0x4

	/* add signature */
	/* 0x58 */
	mov		r1, #0x44
	str     r1, [r3]
	add		r3, r3, #0x4

	/* bl      swsusp_save */

/* 20120522	*/
	ldr     r3, .Lctx_backup_addr
	ldr		r3, [r3]
	add		r3, r3, #0x5C

	/*
	 * Change to IRQ mode
	 */
	mrs     r1, cpsr
	bic		r1,	r1, #0x1f
	orr     r1, r1, #0x12
	msr     cpsr_c, r1

	/* 0x5C - 0x60 */
	stmia   r3, {r13-r14}
	add		r3, r3, #0x8

	/*
	 * Change to ABT mode
	 */
	mrs     r1, cpsr
	bic		r1,	r1, #0x1f
	orr     r1, r1, #0x17
	msr     cpsr_c, r1

	/* 0x64 - 0x68 */
	stmia   r3, {r13-r14}
	add		r3, r3, #0x8

	/*
	 * Change to Undefined mode
	 */
	mrs     r1, cpsr
	bic			r1,	r1, #0x1f
	orr     r1, r1, #0x1b
	msr     cpsr_c, r1

	/* 0x6C - 0x70 */
	stmia   r3, {r13-r14}
	add		r3, r3, #0x8

	/*
	 * Change to FIQ mode
	 */
	mrs     r1, cpsr
	bic		r1,	r1, #0x1f
	orr     r1, r1, #0x11
	msr     cpsr_c, r1

	/* 0x74 - 0x8C */
	stmia   r3, {r8-lr}
	add		r3, r3, #0x1C

	/*
	 * Go back to original SVC mode
	 */
	msr     cpsr_c, r0

/* 20120522	end*/

	/*
	 * Restore return address
	 */
	/* ldr     r3, .Lsaved_context_r14_svc */
	ldr     r3, .Lctx_backup_addr
	ldr		r3, [r3]
	add		r3, r3, #0x4C
	ldr     lr, [r3]
	mov     pc, lr
ENDPROC(pwr_arch_suspend)

ENTRY(pwr_arch_resume)
	/*
	 * Restore SVC context
	 * pwr_arch_resume(save_area)
	 */
	ldr		r3, .Lctx_backup_addr
	str		r0, [r3]
	mov		r3, r0
	 
	/* ldr     r3, .Lsaved_context_r13_svc */
	add		r3, r3, #0x48
	ldmia   r3, {r13-r14}
	
	/*ldr     r3, .Lsaved_spsr_svc */
	mov		r3, r0
	add		r3, r3, #0x54
	ldr     r1, [r3]
	msr     spsr_cxsf, r1

	mrs     r0, cpsr	/* Save current mode into r0 */

	/*
	 * Change to system(user) mode
	 */
	mov     r1, r0
	orr     r1, r1, #0x1f
	msr     cpsr_c, r1

	/*
	 * Restore User context
	 */
	/* ldr     r3, .Lsaved_context_r0 */
	ldr		r3, .Lctx_backup_addr
	ldr		r3, [r3]
	add		r3, r3, #0x8
	ldmia   r3, {r0-r14}
	
	/* ldr     r3, .Lsaved_cpsr */
	ldr		r3, .Lctx_backup_addr
	ldr		r3, [r3]	
	ldr     r1, [r3]
	msr     cpsr_cxsf, r1

	msr     cpsr_c, r0	/* Restore original mode from r0 */

	/*
	 * Flush TLB (Invalidate unified TLB unlocked entries)
	 */
	mov     r1, #0
	mcr     p15, 0, r1, c8, c7, 0

	/* Set the return value */
	mov	r0, #0

	/* Restore return address */
	/* ldr     r3, .Lsaved_context_r14_svc */
	ldr		r3, .Lctx_backup_addr
	ldr		r3, [r3]	
	add		r3, r3, #0x4C
	ldr     lr, [r3]
	mov     pc, lr
ENDPROC(pwr_arch_resume)

ENTRY(gp_jump_exec)
	/*
	 * Go back to original SVC mode
	 */
	mrs     r3, cpsr
	bic     r3, #0x1F
	orr     r3, r3, #0x13
	msr     cpsr, r3

#if 0
	mov	r2, r0
	mov	r0, #0
	/* disabling the caches: */
	MCR p15, 0, r0, c7, c5, 0  /*; Invalidate entire instruction cache             */
	ISB

	MRC p15, 0, R1, c1, c0, 0  /*; Read System Control Register configuration data */
	BIC R1, R1, #0x1 <<12      /*; instruction cache disable                       */
	BIC R1, R1, #0x1 <<2       /*; data cache disable                              */
	DSB
	MCR p15, 0, R1, c1, c0, 0  /*; disabled cache RAMs                             */
	ISB
#endif

	mrc	p15, 0, r3, c3, c0, 0
	mov r3, #0x1F
	mcr	p15, 0, r3, c3, c0, 0	/*;write to@cp15 DACR, r3=0x0000001F */
	/* milton ISB */
	
	/* mov		pc, r1 */
	mov     pc, lr

ENDPROC(gp_jump_exec)
	/* .align	4 */
.Lctx_backup_addr:		.long	ctx_backup_addr
