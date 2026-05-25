/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

	.syntax  unified
	.cpu cortex-m3

	.section .text.vector
	.align   2
	.globl   __Vectors
	.globl   __Vectors_End
	.globl   __Vectors_Size
__Vectors:
	.long    __StackTop 		/* Top of Stack, must be aligned to 8bytes (0xE000ED14.bit8=1) */
	.long    Reset_Handler		/* Reset Handler */
	.long    NMI_Handler		/* -14 NMI Handler */
	.long    HardFault_Handler	/* -13 Hard Fault Handler */
	.long    MemManage_Handler	/* -12 MPU Fault Handler */
	.long    BusFault_Handler	/* -11 Bus Fault Handler */
	.long    UsageFault_Handler	/* -10 Usage Fault Handler */
	.long    0			/*     Reserved */
	.long    0			/*     Reserved */
	.long    0			/*     Reserved */
	.long    0			/*     Reserved */
	.long    vPortSVCHandler	/*  -5 SVC Handler */
	.long    DebugMon_Handler	/*  -4 Debug Monitor Handler */
	.long    0			/*     Reserved */
	.long    xPortPendSVHandler	/*  -2 PendSV Handler */
	.long    xPortSysTickHandler	/*  -1 SysTick Handler */

	.long arm_cm3_irq0		/* 16 + 0 */
	.long arm_cm3_irq1		/* 16 + 1 */
	.long uart0_ahb_intr		/* 16 + 2 */
	.long uart1_ahb_intr		/* 16 + 3 */
	.long uart2_ahb_intr		/* 16 + 4 */
	.long uart3_ahb_intr		/* 16 + 5 */
	.long ssi0_ahb_intr		/* 16 + 6 */
	.long ssi1_ahb_intr		/* 16 + 7 */
	.long ssi2_ahb_intr		/* 16 + 8 */
	.long ssi3_ahb_intr		/* 16 + 9 */
	.long ssis0_ahb_intr		/* 16 + 10 */
	.long tcc_interrupt		/* 16 + 11 */
	.long timer0_intr1		/* 16 + 12 */
	.long timer0_intr2		/* 16 + 13 */
	.long timer0_intr3		/* 16 + 14 */
	.long timer0_intr4		/* 16 + 15 */
	.long timer0_intr5		/* 16 + 16 */
	.long timer0_intr6		/* 16 + 17 */
	.long timer0_intr7		/* 16 + 18 */
	.long timer0_intr8		/* 16 + 19 */
	.long timer0_intr9		/* 16 + 20 */
	.long timer0_intr10		/* 16 + 21 */
	.long timer1_intr1		/* 16 + 22 */
	.long timer1_intr2		/* 16 + 23 */
	.long timer1_intr3		/* 16 + 24 */
	.long timer1_intr4		/* 16 + 25 */
	.long timer1_intr5		/* 16 + 26 */
	.long timer1_intr6		/* 16 + 27 */
	.long timer1_intr7		/* 16 + 28 */
	.long timer1_intr8		/* 16 + 20 */
	.long timer1_intr9		/* 16 + 30 */
	.long timer1_intr10		/* 16 + 31 */
	.long timer2_intr1		/* 16 + 32 */
	.long timer2_intr2		/* 16 + 33 */
	.long timer2_intr3		/* 16 + 34 */
	.long timer2_intr4		/* 16 + 35 */
	.long timer2_intr5		/* 16 + 36 */
	.long timer2_intr6		/* 16 + 37 */
	.long timer2_intr7		/* 16 + 38 */
	.long timer2_intr8		/* 16 + 39 */
	.long timer2_intr9		/* 16 + 40 */
	.long timer2_intr10		/* 16 + 41 */
	.long wtd_cm3_intr		/* 16 + 42 */
	.long i2c0_intr			/* 16 + 43 */
	.long i2c1_intr			/* 16 + 44 */
	.long i2c2_intr			/* 16 + 45 */
	.long i2c3_intr			/* 16 + 46 */
	.long idcs_intr			/* 16 + 47 */
	.long gpio0_intr		/* 16 + 48 */
	.long gpio1_intr		/* 16 + 49 */
	.long gpio2_intr		/* 16 + 50 */
	.long gpio3_intr		/* 16 + 51 */
	.long adc_intr			/* 16 + 52 */
	.long uart_apb_intr		/* 16 + 53 */
	.long irif_intr			/* 16 + 54 */
	.long i3c_intr			/* 16 + 55 */
	.long wdt0_intr			/* 16 + 56 */
	.long wdt1_intr			/* 16 + 57 */
	.long dummy_intr		/* 16 + 58 */
	.long dummy_intr		/* 16 + 59 */
	.long dummy_intr		/* 16 + 60 */
	.long dummy_intr		/* 16 + 61 */
	.long dummy_intr		/* 16 + 62 */
	.long dummy_intr		/* 16 + 63 */

	/* 16 + 64 + 48 => 128 must align to 2^n */
	.space(48 * 4)

__Vectors_End:
.equ     __Vectors_Size, __Vectors_End - __Vectors
.size    __Vectors, . - __Vectors

	.thumb
	.section .text
	.align   2

	.thumb_func
	.type    Reset_Handler, % function
	.globl   Reset_Handler
Reset_Handler:
	mov	r0, #0
	ldr	r1, = __bss_start__
	ldr	r2, = __bss_end__
clr_bss:
	str	r0, [r1], #4
	cmp	r1, r2
	blt	clr_bss

	bl	SystemInit
	b	main
	.size    Reset_Handler, . - Reset_Handler

	/* HardFault_Handler */
	.thumb_func
	.type    HardFault_Handler, % function
	.weak    HardFault_Handler
	.fnstart
HardFault_Handler:
	b        .
	.fnend
	.size    HardFault_Handler, . - HardFault_Handler

	/* Default_Handler Dummy */
	.thumb_func
	.type    Default_Handler, % function
	.weak    Default_Handler
	.fnstart
Default_Handler:
	b        .
	.fnend
	.size    Default_Handler, . - Default_Handler

	/* Macro to define default[WEAK] exception/interrupt handlers.*/
	.macro   Set_Default_Handler  Handler_Name
	.weak    \Handler_Name
	.set     \Handler_Name, Default_Handler
	.endm

	/* Default exception/interrupt handler */
	Set_Default_Handler  NMI_Handler
	Set_Default_Handler  MemManage_Handler
	Set_Default_Handler  BusFault_Handler
	Set_Default_Handler  UsageFault_Handler
	Set_Default_Handler  vPortSVCHandler
	Set_Default_Handler  DebugMon_Handler
	Set_Default_Handler  xPortPendSVHandler
	Set_Default_Handler  xPortSysTickHandler

	Set_Default_Handler  arm_cm3_irq0	/* 16 + 0 */
	Set_Default_Handler  arm_cm3_irq1	/* 16 + 1 */

	Set_Default_Handler  uart0_ahb_intr	/* 16 + 2 */
	Set_Default_Handler  uart1_ahb_intr	/* 16 + 3 */
	Set_Default_Handler  uart2_ahb_intr	/* 16 + 4 */
	Set_Default_Handler  uart3_ahb_intr	/* 16 + 5 */

	Set_Default_Handler  ssi0_ahb_intr	/* 16 + 6 */
	Set_Default_Handler  ssi1_ahb_intr	/* 16 + 7 */
	Set_Default_Handler  ssi2_ahb_intr	/* 16 + 8 */
	Set_Default_Handler  ssi3_ahb_intr	/* 16 + 9 */
	Set_Default_Handler  ssis0_ahb_intr	/* 16 + 10 */

	Set_Default_Handler  tcc_interrupt	/* 16 + 11 */

	Set_Default_Handler  timer0_intr1	/* 16 + 12 */
	Set_Default_Handler  timer0_intr2	/* 16 + 13 */
	Set_Default_Handler  timer0_intr3	/* 16 + 14 */
	Set_Default_Handler  timer0_intr4	/* 16 + 15 */
	Set_Default_Handler  timer0_intr5	/* 16 + 16 */
	Set_Default_Handler  timer0_intr6	/* 16 + 17 */
	Set_Default_Handler  timer0_intr7	/* 16 + 18 */
	Set_Default_Handler  timer0_intr8	/* 16 + 19 */
	Set_Default_Handler  timer0_intr9	/* 16 + 20 */
	Set_Default_Handler  timer0_intr10	/* 16 + 21 */

	Set_Default_Handler  timer1_intr1	/* 16 + 22 */
	Set_Default_Handler  timer1_intr2	/* 16 + 23 */
	Set_Default_Handler  timer1_intr3	/* 16 + 24 */
	Set_Default_Handler  timer1_intr4	/* 16 + 25 */
	Set_Default_Handler  timer1_intr5	/* 16 + 26 */
	Set_Default_Handler  timer1_intr6	/* 16 + 27 */
	Set_Default_Handler  timer1_intr7	/* 16 + 28 */
	Set_Default_Handler  timer1_intr8	/* 16 + 20 */
	Set_Default_Handler  timer1_intr9	/* 16 + 30 */
	Set_Default_Handler  timer1_intr10	/* 16 + 31 */

	Set_Default_Handler  timer2_intr1	/* 16 + 32 */
	Set_Default_Handler  timer2_intr2	/* 16 + 33 */
	Set_Default_Handler  timer2_intr3	/* 16 + 34 */
	Set_Default_Handler  timer2_intr4	/* 16 + 35 */
	Set_Default_Handler  timer2_intr5	/* 16 + 36 */
	Set_Default_Handler  timer2_intr6	/* 16 + 37 */
	Set_Default_Handler  timer2_intr7	/* 16 + 38 */
	Set_Default_Handler  timer2_intr8	/* 16 + 39 */
	Set_Default_Handler  timer2_intr9	/* 16 + 40 */
	Set_Default_Handler  timer2_intr10	/* 16 + 41 */

	Set_Default_Handler  wtd_cm3_intr	/* 16 + 42 */

	Set_Default_Handler  i2c0_intr		/* 16 + 43 */
	Set_Default_Handler  i2c1_intr		/* 16 + 44 */
	Set_Default_Handler  i2c2_intr		/* 16 + 45 */
	Set_Default_Handler  i2c3_intr		/* 16 + 46 */
	Set_Default_Handler  idcs_intr		/* 16 + 47 */

	Set_Default_Handler  gpio0_intr		/* 16 + 48 */
	Set_Default_Handler  gpio1_intr		/* 16 + 49 */
	Set_Default_Handler  gpio2_intr		/* 16 + 50 */
	Set_Default_Handler  gpio3_intr		/* 16 + 51 */

	Set_Default_Handler  adc_intr		/* 16 + 52 */
	Set_Default_Handler  uart_apb_intr	/* 16 + 53 */
	Set_Default_Handler  irif_intr		/* 16 + 54 */
	Set_Default_Handler  i3c_intr		/* 16 + 55 */
	Set_Default_Handler  wdt0_intr		/* 16 + 56 */
	Set_Default_Handler  wdt1_intr		/* 16 + 57 */

	Set_Default_Handler  dummy_intr		/* 16 + 58 */

	.end
