/*******************************************************************************
 *
 * Copyright (c) 2023 Ambarella International LP
 *
 * This file and its contents ("Software") are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella International LP and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella International LP or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella International LP
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA INTERNATIONAL LP OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
