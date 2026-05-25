/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include <r52_help.h>

/* Keep the same with portASM.S use the ARM-MODE Code */
.text
.syntax unified
.cpu cortex-r52
.arm

.set ARCH_IRQ_MODE,	0x12
.set ARCH_SVC_MODE,	0x13
.set ARCH_ABT_MODE,	0x17
.set ARCH_UDF_MODE,	0x1B
.set ARCH_SYS_MODE,	0x1F

.global el2_vector
.global	el1_vectors

.extern main
.extern arch52_system_init

/* undef/abort/prefetch exception check */
.extern undef_exception_addr
.extern abort_exception_addr
.extern prfch_exception_addr
.extern arch52_undef_handler
.extern arch52_abort_handler
.extern arch52_prfch_handler
/* irq and swi exception use freertos's code */
.extern FreeRTOS_IRQ_Handler
.extern FreeRTOS_SWI_Handler

/* mpu setup */
.macro setup_mpu_el2 n, bar, lar
	mov	r0, #\n				// Load the region number into r1
	mcr	p15, 4, r0, c6, c2, 1		// HPRSELR - select region for EL2
	ldr	r0, =\bar			// Load the base address of the region into r1
	mcr	p15, 4, r0, c6, c3, 0		// HPRBAR - set base address for EL2 region
	ldr	r0, =\lar			// Load the limit address for the region into r1
	mcr	p15, 4, r0, c6, c3, 1		// HPRLAR - set limit address for EL2 region
.endm

.macro setup_mpu_el1 n, bar, lar
	mov	r0, #\n				// Load the region number into r1
	mcr	p15, 0, r0, c6, c2, 1		// PRSELR - select region for EL1
	ldr	r0, =\bar			// Load the base address of the region into r1
	mcr	p15, 0, r0, c6, c3, 0		// PRBAR - set base address for EL1 region
	ldr	r0, =\lar			// Load the limit address for the region into r1
	mcr	p15, 0, r0, c6, c3, 1		// PRLAR - set limit address for EL1 region
.endm

/***************************EL2-VECTORS**************************************/
/* place it at 32bytes align address */
.section .vectors.el2, "ax"
el2_vectors:
	ldr pc, el2_reset_addr
	ldr pc, el2_undefined_addr
	ldr pc, el2_hvc_addr
	ldr pc, el2_prefetch_addr
	ldr pc, el2_abort_addr
	ldr pc, el2_hype_addr
	ldr pc, el2_irq_addr
	ldr pc, el2_fiq_addr

el2_reset_addr:		.word	el2_reset_handler
el2_undefined_addr:	.word	el2_undefined_handler
el2_hvc_addr:		.word	el2_hvc_handler
el2_prefetch_addr:	.word	el2_prefetch_handler
el2_abort_addr:		.word	el2_abort_handler
el2_hype_addr:		.word	el2_hype_handler
el2_irq_addr:		.word	el2_irq_handler
el2_fiq_addr:		.word	el2_fiq_handler

/* insert some parameters pass from outside
 * syncnt = clkfreq_rate / 12
*/
.org 64, 0
.global param_syscnt_rate
param_syscnt_rate:	.word	0x00000000
.global param_ahb_rate
param_ahb_rate:		.word	0x00000000

.section .text.el2
.align 3
.type	el2_reset_handler, "function"
el2_reset_handler:
	/*
	 * IMP_BPCTLR:
	 * Disable branch predictor when running at EL2
	 * Note: this is must have, otherwise may hang
	 * due to incorrect branch prediction to
	 * fetch instruction from non-TCM region.
	 */
	mrc	p15, 1, r0, c9, c1, 1
	orr	r0, r0, #4
	mcr	p15, 1, r0, c9, c1, 1
	isb

	/* Trun on LLPP */
	mrc	p15, 0, r0, c15, c0, 0
	orr	r0, r0, #3
	mcr	p15, 0, r0, c15, c0, 0

	/* check current EL
	 * EL2_mode : 0x1A
	 * USR_mode : 0x10 ->EL0
	 * SVC_mode : 0x13 ->EL1->RTOS/OS_KERNEL, kernel_service SVC_MODE and task system-mode
	 * IRQ_mode : 0x12 ->EL1
	 * FIQ_mode : 0x11 ->EL1
	 * Abt_mode : 0x17 ->EL1
	 * udf_mode : 0x1B ->EL1
	*/
	mrs	r0, cpsr
	and	r0, r0, #0x1f
	cmp	r0, #0x1a
	bne	.	/* die here if not EL2 */

	/* init redistributor[GIC_REDIST_WKUP: 0x100014] of gic in r52-core0 */
	mrc	p15, 1, r0, c15, c3, 0	/* get the cbar: GIC_DISTRIBUTE_BASE */
	ldr	r1, =GIC_REDIST_WKUP	/* GIC_REDISTRIBUTE_WAKEUP_OFFSET */
	add	r0, r0, r1
	ldr	r1, [r0]
	bic	r1, r1, #2		/* Clear ProcessorSleep, BIT[1] */
	str	r1, [r0]
	dsb
	isb
3:	ldr	r1, [r0]		/* Wait ChildrenAsleep BIT[2] == 0 */
	tst	r1, #4
	bne	3b

	/* Change EL2 exception base address */
	ldr	r0, =el2_vectors
	mcr	p15, 4, r0, c12, c0, 0	/* write to hvbar */
	isb

	/* Init HSCTLR: disable MPU,Cache/Aligment/BR(background mpu)
	 * code can't run at 2GB above since this.(MPU disabled)
	*/
	ldr	r0, =0x30c50818
	mcr	p15, 4, r0, c1, c0, 0	/* write to hsctlr */
	isb

	/* Enable EL1 access to all IMP DEF registers */
	ldr	r0, =0x3781
	dsb
	mcr	p15, 4, r0, c1, c0, 1	/* write to hactlr */
	isb

	/* Change EL1 exception base address */
	ldr	r0, =el1_vectors
	mcr	p15, 0, r0, c12, c0, 0	/* write to vbar */
	isb

#if 0
	/* Change Counter-timer Frequency register
	 * gclk_r52 = 432M, syscnt=432/12 => 36M [EL2 only]
	 */
	ldr	r0, =36000000
	mcr	p15, 0, r0, c14, c0, 0
	dsb
	isb
#else
	ldr	r0, =param_syscnt_rate
	ldr	r0, [r0]
	mcr	p15, 0, r0, c14, c0, 0
	dsb
	isb
#endif

	/* setup the MPU for EL1, EL1 need to run at ATCM(above 2GB) address space
	 * device memory or address above 0x8000_0000 execute never.
	 * ATCM need to run code, use MPU to treate as normal memory.
	 * ONLY: SI_MEMORY, TCM_MEMORY, DDR_MEMOY can run code.
	*/
	MRC	p15, 0, r0, c10, c2, 0		/* MAIR0 */
	mov	r1, #MEM_ATTR_NORMAL
	BFI	r0, r1, #0, #8			/* Attr0 -> normal memory cached */
	mov	r1, #MEM_ATTR_NORMAL_NC
	BFI	r0, r1, #8, #8			/* Attr1 -> normal memory nocache(write-though) */
	mov	r1, #MEM_ATTR_DEVICE
	BFI	r0, r1, #16, #8			/* Attr2 -> device memory */
	MCR	p15, 0, r0, c10, c2, 0

	/* SI_MEMORY as normal memory, no cache, size: 3000KB: can execute code */
	setup_mpu_el1 0, XPRBAR(0x7fd00000, SH_NONE, AP_RW, 0), XPRLAR(0x7ffeffff, ATTR_IDX_NORMAL_NC)
	/* TCM memory as normal memory, no cache, size: 128KB: can execute code*/
	setup_mpu_el1 1, XPRBAR(0xf6000000, SH_NONE, AP_RW, 0), XPRLAR(0xf601ffff, ATTR_IDX_NORMAL_NC)
	/* ddr memory as normal memory, no cache: can execute code */
	setup_mpu_el1 2, XPRBAR(0x00000000, SH_NONE, AP_RW, 0), XPRLAR(0x7fcfffff, ATTR_IDX_NORMAL_NC)

	/* enable SCTRL for MPU at EL1 */
	ldr	r1, =MPU_CTRL_ENABLE
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, r1
	mcr	p15, 0, r0, c1, c0, 0
	isb

#if 0
	/* romcode already enable ABC_TCM for EL2/EL1/EL0 */
	mrc	p15, 0, r0, c9, c1, 0
	mov	r1, #ATCM_BASE_ADDR
	orr	r0, r0, r1
	orr	r0, r0, #3
	mcr	p15, 0, r0, c9, c1, 0
	add	r0, r0, #ATCM_SIZE	/* BTCM_ADDR = ATCM_ADDR + ATCM_SIZE */
	mcr	p15, 0, r0, c9, c1, 1
	add	r0, r0, #BTCM_SIZE	/* CTCM_ADDR = BTCM_ADDR + BTCM_SIZE */
	mcr	p15, 0, r0, c9, c1, 2
	mov	r0, #3
	mcr	p15, 0, r0, c11, c0, 0
#endif

	/* relocate EL1 code to ATCM */
	ldr	r0, = __relocate_start__
	ldr	r1, = __relocate_end__
	ldr	r2, = __snd_text_start__
loop:
	ldr	r3, [r0], #4
	str	r3, [r2], #4
	cmp	r0, r1
	blo	loop
	dsb
	isb

	/* switch to SVC mode for freertos kernel */
	mrs	r0, cpsr
	mov	r1, #0x13
	bfi	r0, r1, #0, #5
	bic	r0, r0, #(0x1 << 5)	/* clear T bit to make sure ARM-Mode */
	msr	spsr, r0
	ldr	r0, =el1_reset_handler
	msr	elr_hyp, r0
	dsb
	isb
	eret

/***************************EL2-other-handlers********************************/
.type	el2_undefined_handler, "function"
el2_undefined_handler:
	b	el2_undefined_handler

.type	el2_hvc_handler, "function"
el2_hvc_handler:
	b	el2_hvc_handler

.type	el2_prefetch_handler, "function"
el2_prefetch_handler:
	b	el2_prefetch_handler

.type	el2_abort_handler, "function"
el2_abort_handler:
	b	el2_abort_handler

.type	el2_hype_handler, "function"
el2_hype_handler:
	b	el2_hype_handler

.type	el2_irq_handler, "function"
el2_irq_handler:
	b	el2_irq_handler

.type	el2_fiq_handler, "function"
el2_fiq_handler:
	b	el2_fiq_handler
/****************************************************************************/

/***************************EL1-VECTORS**************************************/
.align 5	/* 32 bytes align for el1_vectors */
.section .text.atcm, "ax"
el1_vectors:
	b	el1_reset_handler
	b	el1_undefined_handler
	b	FreeRTOS_SWI_Handler
	b	el1_prefetch_handler
	b	el1_abort_handler
	b	.
	b	FreeRTOS_IRQ_Handler
	b	el1_fiq_handler
/***************************EL1-handlers**************************************/
.type	el1_reset_handler, "function"
el1_reset_handler:
	isb
	/* cache invalidation, icache && dcache from AMBA-threadx */
	dsb
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0	// invalidate entire instruction cache
	mrc	p15, 1, r0, c0, c0, 1	// read clidr
	ands	r3, r0, #0x07000000	// extract coherency level
	mov	r3, r3, lsr #23		// total cache levels << 1
	beq	finished		// if 0, no need to clean

	mov	r10, #0			// r10 holds current cache level << 1
loop1:
	add	r2, r10, r10, lsr #1	// r2 holds cache "set" position
	mov	r1, r0, lsr r2		// bottom 3 bits are the cache-type for this level
	and	r1, r1, #7		// isolate those lower 3 bits
	cmp	r1, #2
	blt	skip			// no cache or only instruction cache at this level

	mcr	p15, 2, r10, c0, c0, 0	// write the cache size selection register
	isb				// isb to sync the change to the cachesizeid reg
	mrc	p15, 1, r1, c0, c0, 0	// reads current cache size id register
	and	r2, r1, #7		// extract the line length field
	add	r2, r2, #4		// add 4 for the line length offset (log2 16 bytes)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3	// r4 is the max number on the way size (right aligned)
	clz	r5, r4			// r5 is the bit position of the way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13	// r7 is the max number of the index size (right aligned)

loop2:
	mov	r9, r4			// r9 working copy of the max way size (right aligned)

loop3:
	orr	r11, r10, r9, lsl r5	// factor in the way number and cache number into r11
	orr	r11, r11, r7, lsl r2	// factor in the set number
	mcr	p15, 0, r11, c7, c6, 2	// invalidate by set/way
	subs	r9, r9, #1		// decrement the way number
	bge	loop3
	subs	r7, r7, #1		// decrement the set number
	bge	loop2
skip:
	add	r10, r10, #2		// increment the cache number
	cmp	r3, r10
	bgt	loop1

finished:
	mrc	p15, 0, r0, c1, c0, 2	/* read cpacr */
	orr	r0, r0, #0xf00000	/* cpacr[23:22] = full access cp11, cpacr[21:20] = full access cp10 */
	mcr	p15, 0, r0, c1, c0, 2
	isb
/******************************cache done **************************************/

/* clear bss and make sure 8bytes aligned in the linkscript file */
	ldr	r0, =__bss_start__
	ldr	r1, =__bss_end__
	mov	r2, #0
	mov	r3, #0
bss_zero:
	stmia	r0!, {r2-r3}
	cmp	r0, r1
	blt	bss_zero

/*
 * Initialize Stacks for irq_mode, system_mode, svc_mode
 * irq_mode    : irq handler only(not fiq)
 * undef/abort mode  : undef/abort exectpion for debug used
 * system_mode : tasks_app (check the portASM.S)
 * svc_mode    : freertos kernel-service like task_scheduler
*/
	cps	#ARCH_IRQ_MODE
	ldr	r0, =__irq_stack_top__
	mov	sp, r0		/* sp_irq */
	dsb

	cps	#ARCH_UDF_MODE
	ldr	r0, =__udef_stack_top__
	mov	sp, r0		/* sp_undef */
	dsb

	cps	#ARCH_ABT_MODE
	ldr	r0, =__abort_stack_top__
	mov	sp, r0		/* sp_abt */
	dsb

	ldr	r0, =__sys_stack_top__
	cps	#ARCH_SYS_MODE
	mov	sp, r0		/* sp_usr || sp_sys */
	dsb

	/* must switch to SVC mode for freertos kernel */
	ldr	r0, =__svc_stack_top__
	cps	#ARCH_SVC_MODE
	mov	sp, r0		/* sp_svc */
	dsb

/******************************************************************************/
	bl	arch52_system_init
	bl	main
	b	.
/******************************************************************************/

.type	el1_undefined_handler, "function"
el1_undefined_handler:
	stmdb	sp!, {r0-r3, r12, lr}
	ldr	r0, =undef_exception_addr
	sub	r1, lr, #4
	str	r1, [r0]
	bl	arch52_undef_handler	/* should never return */
	ldmia	sp!, {r0-r3, r12, lr}
	movs	pc, lr

.type	el1_prefetch_handler, "function"
el1_prefetch_handler:
	stmdb	sp!,{r0-r3, r12, lr}
	ldr	r0, =prfch_exception_addr
	sub	r1, lr, #4
	str	r1, [r0]
	bl	arch52_prfch_handler	/* should never return */
	ldmia	sp!, {r0-r3, r12, lr}
	subs	pc, lr, #4

.type	el1_abort_handler, "function"	/* data abort */
el1_abort_handler:
	stmdb	sp!, {r0-r3, r12, lr}
	ldr	r0, =abort_exception_addr
	sub	r1, lr, #8
	str	r1, [r0]
	bl	arch52_undef_handler	/* should never return */
	ldmia	sp!, {r0-r3, r12, lr}
	subs	pc, lr, #8

.type	el1_fiq_handler, "function"
el1_fiq_handler:
	b	el1_fiq_handler
