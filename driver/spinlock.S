/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
.text
.syntax unified
.align 2

#if defined(ARCH_CM3)
.cpu cortex-m3
.thumb
#endif

#if defined(ARCH_R52)
.cpu cortex-r52
.arm
#endif

.global	spin_lock
.global	spin_unlock
.global	__lock_print
.global	__unlock_print

spin_lock:
	push	{r1, r2}
	movs	r2, #1
spin_lock_loop:
	ldrex	r1, [r0]
	cmp	r1, #0
	bne	lock_is_set
	strex	r1, r2, [r0]
	cmp	r1, #0
	bne	spin_lock_loop
	dmb
	pop	{r1, r2}
	bx	lr
lock_is_set:
	wfe
	b	spin_lock_loop

spin_unlock:
	push	{r1}
	movs	r1, #0
	str	r1, [r0]
	dmb
	pop	{r1}
	sev
	bx	lr

__lock_print:
	bx	lr

__unlock_print:
	bx	lr
