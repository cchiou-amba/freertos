/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

.text
.syntax unified
.cpu cortex-r52
.arm


/*---------------------------------------------------------------------------*/
/* gic cpuinterface configuration: group0 by FIQ; group1:IRQ */
.global gic_cpu_set_bpr0		/* ICC_BPR0 for group0. interrupt preemption */
.global gic_cpu_set_bpr1		/* ICC_BPR1 for group1. interrupt preemption */
.global gic_cpu_get_bpr0		/* ICC_BPR0 for group0. interrupt preemption */
.global gic_cpu_get_bpr1		/* ICC_BPR1 for group1. interrupt preemption */
.global gic_cpu_set_ctrl		/* ICC_CTLR */
.global gic_cpu_set_eoir0		/* ICC_EOIR0: end of interrupt done in group0 */
.global gic_cpu_set_eoir1		/* ICC_EOIR1: end of interrupt done in group1 */
.global gic_cpu_set_sgi0		/* ICC_SGI0R in group0 */
.global gic_cpu_set_sgi1		/* ICC_SGI1R in group1 */
.global gic_cpu_get_pmr			/* ICC_PMR */
.global gic_cpu_set_pmr			/* ICC_PMR interrupt priority mask, bignum lower priority */
.global gic_cpu_set_gp0en		/* ICC_IGRPEN0 group0 intertupt enable */
.global gic_cpu_set_gp1en		/* ICC_IGRPEN1 group1 interrupt enable */
.global gic_cpu_get_iar0		/* ICC_IAR0: group0 interrupt acknowledge */
.global gic_cpu_get_iar1		/* ICC_IAR1: group1 interrupt acknowledge */
.global gic_cpu_get_ctrl		/* ICC_CTLR: interrupt control */
.global gic_distr_cbar			/* IMP_CBAR: gic distribute base address */
.global gic_cpu_get_runpri
/*---------------------------------------------------------------------------*/
.global DSB
.global DMB
.global ISB
.global __DSB
.global __DMB
.global __ISB
/*---------------------------------------------------------------------------*/
.global interrupt_enable	/* enable fiq and irq */
.global interrupt_disable	/* disable fiq and irq */
.global irq_enable		/* enable irq */
.global irq_disable		/* disable irq */
.global fiq_enable		/* enable fiq */
.global fiq_disable		/* disable irq */
.global interrupt_state		/* cpsr */
/*---------------------------------------------------------------------------*/
.global vbase_set_addr		/* el1 vbase */
.global hvbase_set_addr		/* el2 hvbase */
/*---------------------------------------------------------------------------*/
.global sys_freqcnt_get		/* get CNTFRQ for generic timer */
.global sys_freqcnt_set		/* update the CNTFRQ for the generic timer [EL2-ONLY]*/
.global phystimer_get_counter	/* phys timer counter, 64bits, return pass by r0,r1 */
.global phystimer_set_ctrl	/* phys timer control 32bits */
.global phystimer_set_cmpval	/* phys timer compare value: 64bits, pass by r0,r1 */
.global phystimer_set_tval	/* phys timer timervalue 32bits by r0 */
/*---------------------------------------------------------------------------*/

DSB:
__DSB:
	dsb	sy
	bx	lr

DMB:
__DMB:
	dmb	sy
	bx	lr

ISB:
__ISB:
	isb	sy
	bx	lr

interrupt_enable:
	cpsie	if
	bx	lr

interrupt_disable:
	cpsid if
	bx  lr

irq_enable:
	cpsie	i
	bx	lr

irq_disable:
	cpsid	i
	bx	lr

fiq_enable:
	cpsie	f
	bx	lr

fiq_disable:
	cpsid	f
	bx	lr

interrupt_state:
	mrs	r0, cpsr
	bx	lr

/* void vbase_set_addr(uint32_t base) at EL1 */
vbase_set_addr:
	mcr	p15, 0, r0, c12, c0, 0
	bx	lr

/* void hvbase_set_addr(uint32_t base) at EL2 */
hvbase_set_addr:
	mcr	p15, 4, r0, c12, c0, 0
	bx	lr

/*---------------------------[GenTimer]------------------------------------*/

/* CNTFRQ: get the syscnt to know generic timer freq */
sys_freqcnt_get:
	mrc	p15, 0, r0, c14, c0, 0
	isb
	bx	lr

/* CNTFRQ: set the syscnt for generic timer freq [EL2 only]*/
sys_freqcnt_set:
	mcr	p15, 0, r0, c14, c0, 0
	isb
	bx	lr

/* CNTPCT: uint64_t phystimer_get_counter(void)
 * r0: low32 r1:high32
*/
phystimer_get_counter:
	mrrc	p15, 0, r0, r1, c14
	bx	lr

/* void phystimer_set_ctrl(uint32_t ctrl) */
phystimer_set_ctrl:
	mcr	p15, 0, r0, c14, c2, 1
	bx	lr

/* CNTP_CVAL:
 * void phys_timer_compare_set(uint64_t value)
 * r0: low32bit  r1: high32
*/
phystimer_set_cmpval:
	mcrr	p15, 2, r0, r1, c14
	bx	lr

/* CNTP_TVAL:
 * void phystimer_set_tval(uint32_t timeoffset)
*/
phystimer_set_tval:
	mcr	p15, 0, r0, c14, c2, 0
	bx	lr

/*-----------------------------[GIC_ICC]--------------------------------------*/
/* ICC_IAR0: bit[9:0] INTID */
gic_cpu_get_iar0:
	mrc	p15, 0, r0, c12, c8, 0
	bx	lr

/* ICC_IAR1: bit[9:0] INTID */
gic_cpu_get_iar1:
	mrc	p15, 0, r0, c12, c12, 0
	bx	lr

/* ICC_EOIR0: end of group0 interrupts */
gic_cpu_set_eoir0:
	mcr	p15, 0, r0, c12, c8, 1
	bx	lr

/* ICC_EOIR1: end of group1 interrupts */
gic_cpu_set_eoir1:
	mcr	p15, 0, r0, c12, c12, 1
	bx	lr

/* ICC_BPR0: binaryPoint, set as 2, so we can get [7:3]=> 32 prirotiy level */
gic_cpu_set_bpr0:
	mcr	p15, 0, r0, c12, c8, 3
	bx	lr

/* ICC_BPR1: binaryPoint, set as 2, so we can get [7:3]=> 32 prirotiy level */
gic_cpu_set_bpr1:
	mcr	p15, 0, r0, c12, c12, 3
	bx	lr

/* ICC_BPR0: binaryPoint */
gic_cpu_get_bpr0:
	mrc	p15, 0, r0, c12, c8, 3
	bx	lr

/* ICC_BPR1: binaryPoint */
gic_cpu_get_bpr1:
	mrc	p15, 0, r0, c12, c12, 3
	bx	lr

/* ICC_PMR: interrupt priority mask, set as 0xFF to pass all interrupt */
gic_cpu_set_pmr:
	mcr	p15, 0, r0, c4, c6, 0
	bx	lr

/* ICC_PMR  */
gic_cpu_get_pmr:
	mrc	p15, 0, r0, c4, c6, 0
	bx	lr

/* ICC_RPR: the Running priority */
gic_cpu_get_runpri:
	mrc	p15, 0, r0, c12, c11, 3
	bx	lr

/* ICC_ICR: use default value, don't change it [keep CBPR=0&&EOImode=0] */
gic_cpu_set_ctrl:
	mcr	p15, 0, r0, c12, c12, 4
	bx	lr

/* ICC_ICR: */
gic_cpu_get_ctrl:
	mrc	p15 , 0, r0, c12, c12, 4
	bx	lr

/* ICC_IGRPEN0: group0 interrupt enable, bit0 only */
gic_cpu_set_gp0en:
	mcr	p15, 0, r0, c12, c12, 6
	bx	lr

/* ICC_IGRPEN1: group1 interrupt enable, bit0 only */
gic_cpu_set_gp1en:
	mcr	p15, 0, r0, c12, c12, 7
	bx	lr

/* ICC_SGI0R in group0, 64bits
 * Write R0 to ICC_SGI0R[31:0] and R1 to ICC_SGI1R[63:32]
 * BIT_40: IMR=0 all cores include self
 * BIT_40: IMR=1 all cores exclude self
*/
gic_cpu_set_sgi0:
	mcrr	p15, 2, r0, r1, c12
	bx	lr

gic_cpu_set_sgi1:
	mcrr	p15, 0, r0, r1, c12
	bx	lr

/*-----------------------------[GIC_DIS]--------------------------------------*/
/* IMP_CBAR: [31:21, 20-0:0] */
gic_distr_cbar:
	mrc	p15, 1, r0, c15, c3, 0
	bx	lr

