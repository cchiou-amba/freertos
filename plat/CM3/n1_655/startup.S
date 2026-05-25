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

	.long armcl0_2_cm3_ipi0_intr	/* 16 + 0 */
	.long armcl0_2_cm3_ipi1_intr	/* 16 + 1 */
	.long armcl0_2_cm3_ipi2_intr	/* 16 + 2 */
	.long armcl0_2_cm3_ipi3_intr	/* 16 + 3 */

	.long armcl1_2_cm3_ipi0_intr	/* 16 + 4 */
	.long armcl1_2_cm3_ipi1_intr	/* 16 + 5 */
	.long armcl1_2_cm3_ipi2_intr	/* 16 + 6 */
	.long armcl1_2_cm3_ipi3_intr	/* 16 + 7 */

	.long armcl2_2_cm3_ipi0_intr	/* 16 + 8 */
	.long armcl2_2_cm3_ipi1_intr	/* 16 + 9 */
	.long armcl2_2_cm3_ipi2_intr	/* 16 + 10 */
	.long armcl2_2_cm3_ipi3_intr	/* 16 + 11 */

	.long uart0_ahb_intr		/* 16 + 12 */
	.long uart1_ahb_intr		/* 16 + 13 */
	.long uart2_ahb_intr		/* 16 + 14 */
	.long uart3_ahb_intr		/* 16 + 15 */

	.long ssi0_ahb_intr		/* 16 + 16 */
	.long ssi1_ahb_intr		/* 16 + 17 */
	.long ssi2_ahb_intr		/* 16 + 18 */
	.long ssi3_ahb_intr		/* 16 + 19 */
	.long ssi4_ahb_intr		/* 16 + 20 */
	.long ssi5_ahb_intr		/* 16 + 21 */
	.long ssi6_ahb_intr		/* 16 + 22 */
	.long ssi7_ahb_intr		/* 16 + 23 */
	.long ssis0_ahb_intr		/* 16 + 24 */

	.long timer0_intr1		/* 16 + 25 */
	.long timer0_intr2		/* 16 + 26 */
	.long timer0_intr3		/* 16 + 27 */
	.long timer0_intr4		/* 16 + 28 */
	.long timer0_intr5		/* 16 + 29 */
	.long timer0_intr6		/* 16 + 30 */
	.long timer0_intr7		/* 16 + 31 */
	.long timer0_intr8		/* 16 + 32 */
	.long timer0_intr9		/* 16 + 33 */
	.long timer0_intr10		/* 16 + 34 */
	.long timer1_intr1		/* 16 + 35 */
	.long timer1_intr2		/* 16 + 36 */
	.long timer1_intr3		/* 16 + 37 */
	.long timer1_intr4		/* 16 + 38 */
	.long timer1_intr5		/* 16 + 39 */
	.long timer1_intr6		/* 16 + 40 */
	.long timer1_intr7		/* 16 + 41 */
	.long timer1_intr8		/* 16 + 42 */
	.long timer1_intr9		/* 16 + 43 */
	.long timer1_intr10		/* 16 + 44 */
	.long timer2_intr1		/* 16 + 45 */
	.long timer2_intr2		/* 16 + 46 */
	.long timer2_intr3		/* 16 + 47 */
	.long timer2_intr4		/* 16 + 48 */
	.long timer2_intr5		/* 16 + 49 */
	.long timer2_intr6		/* 16 + 50 */
	.long timer2_intr7		/* 16 + 51 */
	.long timer2_intr8		/* 16 + 52 */
	.long timer2_intr9		/* 16 + 53 */
	.long timer2_intr10		/* 16 + 54 */

	.long wtd_cm3c0_intr		/* 16 + 55 */
	.long wtd_cm3c1_intr		/* 16 + 56 */
	.long wtd_comn_0_intr		/* 16 + 57 */
	.long wtd_comn_1_intr		/* 16 + 58 */
	.long wtd_comn_2_intr		/* 16 + 59 */
	.long wtd_comn_3_intr		/* 16 + 60 */
	.long wtd_comn_4_intr		/* 16 + 61 */
	.long wtd_comn_5_intr		/* 16 + 62 */

	.long i2c0_intr			/* 16 + 63 */
	.long i2c1_intr			/* 16 + 64 */
	.long i2c2_intr			/* 16 + 65 */
	.long i2c3_intr			/* 16 + 66 */
	.long i2c4_intr			/* 16 + 67 */
	.long i2c5_intr			/* 16 + 68 */
	.long i2c6_intr			/* 16 + 69 */
	.long i2c7_intr			/* 16 + 70 */
	.long i2c8_intr			/* 16 + 71 */
	.long idcs_intr			/* 16 + 72 */

	.long gpio0_intr		/* 16 + 73 */
	.long gpio1_intr		/* 16 + 74 */
	.long gpio2_intr		/* 16 + 75 */
	.long gpio3_intr		/* 16 + 76 */
	.long gpio4_intr		/* 16 + 77 */
	.long gpio5_intr		/* 16 + 78 */
	.long gpio6_intr		/* 16 + 79 */

	.long uart_apb_intr		/* 16 + 80 */
	.long irif_intr			/* 16 + 81 */

	.long r52c0_2_cm3c0		/* 16 + 82 */
	.long r52c0_2_cm3c1		/* 16 + 83 */
	.long r52c1_2_cm3c0		/* 16 + 84 */
	.long r52c1_2_cm3c1		/* 16 + 85 */
	.long r52c2_2_cm3c0		/* 16 + 86 */
	.long r52c2_2_cm3c1		/* 16 + 87 */

	/* 16 + 88 + 24 => 128 must align up to 2^n */
	.space(24 * 4)

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

	/* apply for both cm3-core0 and cm3-core1 */
	Set_Default_Handler armcl0_2_cm3_ipi0_intr	/* 16 + 0 */
	Set_Default_Handler armcl0_2_cm3_ipi1_intr	/* 16 + 1 */
	Set_Default_Handler armcl0_2_cm3_ipi2_intr	/* 16 + 2 */
	Set_Default_Handler armcl0_2_cm3_ipi3_intr	/* 16 + 3 */

	Set_Default_Handler armcl1_2_cm3_ipi0_intr	/* 16 + 4 */
	Set_Default_Handler armcl1_2_cm3_ipi1_intr	/* 16 + 5 */
	Set_Default_Handler armcl1_2_cm3_ipi2_intr	/* 16 + 6 */
	Set_Default_Handler armcl1_2_cm3_ipi3_intr	/* 16 + 7 */

	/* not used */
	Set_Default_Handler armcl2_2_cm3_ipi0_intr	/* 16 + 8 */
	Set_Default_Handler armcl2_2_cm3_ipi1_intr	/* 16 + 9 */
	Set_Default_Handler armcl2_2_cm3_ipi2_intr	/* 16 + 10 */
	Set_Default_Handler armcl2_2_cm3_ipi3_intr	/* 16 + 11 */

	Set_Default_Handler uart0_ahb_intr		/* 16 + 12 */
	Set_Default_Handler uart1_ahb_intr		/* 16 + 13 */
	Set_Default_Handler uart2_ahb_intr		/* 16 + 14 */
	Set_Default_Handler uart3_ahb_intr		/* 16 + 15 */

	Set_Default_Handler ssi0_ahb_intr		/* 16 + 16 */
	Set_Default_Handler ssi1_ahb_intr		/* 16 + 17 */
	Set_Default_Handler ssi2_ahb_intr		/* 16 + 18 */
	Set_Default_Handler ssi3_ahb_intr		/* 16 + 19 */
	Set_Default_Handler ssi4_ahb_intr		/* 16 + 20 */
	Set_Default_Handler ssi5_ahb_intr		/* 16 + 21 */
	Set_Default_Handler ssi6_ahb_intr		/* 16 + 22 */
	Set_Default_Handler ssi7_ahb_intr		/* 16 + 23 */
	Set_Default_Handler ssis0_ahb_intr		/* 16 + 24 */

	Set_Default_Handler timer0_intr1		/* 16 + 25 */
	Set_Default_Handler timer0_intr2		/* 16 + 26 */
	Set_Default_Handler timer0_intr3		/* 16 + 27 */
	Set_Default_Handler timer0_intr4		/* 16 + 28 */
	Set_Default_Handler timer0_intr5		/* 16 + 29 */
	Set_Default_Handler timer0_intr6		/* 16 + 30 */
	Set_Default_Handler timer0_intr7		/* 16 + 31 */
	Set_Default_Handler timer0_intr8		/* 16 + 32 */
	Set_Default_Handler timer0_intr9		/* 16 + 33 */
	Set_Default_Handler timer0_intr10		/* 16 + 34 */
	Set_Default_Handler timer1_intr1		/* 16 + 35 */
	Set_Default_Handler timer1_intr2		/* 16 + 36 */
	Set_Default_Handler timer1_intr3		/* 16 + 37 */
	Set_Default_Handler timer1_intr4		/* 16 + 38 */
	Set_Default_Handler timer1_intr5		/* 16 + 39 */
	Set_Default_Handler timer1_intr6		/* 16 + 40 */
	Set_Default_Handler timer1_intr7		/* 16 + 41 */
	Set_Default_Handler timer1_intr8		/* 16 + 42 */
	Set_Default_Handler timer1_intr9		/* 16 + 43 */
	Set_Default_Handler timer1_intr10		/* 16 + 44 */
	Set_Default_Handler timer2_intr1		/* 16 + 45 */
	Set_Default_Handler timer2_intr2		/* 16 + 46 */
	Set_Default_Handler timer2_intr3		/* 16 + 47 */
	Set_Default_Handler timer2_intr4		/* 16 + 48 */
	Set_Default_Handler timer2_intr5		/* 16 + 49 */
	Set_Default_Handler timer2_intr6		/* 16 + 50 */
	Set_Default_Handler timer2_intr7		/* 16 + 51 */
	Set_Default_Handler timer2_intr8		/* 16 + 52 */
	Set_Default_Handler timer2_intr9		/* 16 + 53 */
	Set_Default_Handler timer2_intr10		/* 16 + 54 */

	Set_Default_Handler wtd_cm3c0_intr		/* 16 + 55 */
	Set_Default_Handler wtd_cm3c1_intr		/* 16 + 56 */
	Set_Default_Handler wtd_comn_0_intr		/* 16 + 57 */
	Set_Default_Handler wtd_comn_1_intr		/* 16 + 58 */
	Set_Default_Handler wtd_comn_2_intr		/* 16 + 59 */
	Set_Default_Handler wtd_comn_3_intr		/* 16 + 60 */
	Set_Default_Handler wtd_comn_4_intr		/* 16 + 61 */
	Set_Default_Handler wtd_comn_5_intr		/* 16 + 62 */

	Set_Default_Handler i2c0_intr			/* 16 + 63 */
	Set_Default_Handler i2c1_intr			/* 16 + 64 */
	Set_Default_Handler i2c2_intr			/* 16 + 65 */
	Set_Default_Handler i2c3_intr			/* 16 + 66 */
	Set_Default_Handler i2c4_intr			/* 16 + 67 */
	Set_Default_Handler i2c5_intr			/* 16 + 68 */
	Set_Default_Handler i2c6_intr			/* 16 + 69 */
	Set_Default_Handler i2c7_intr			/* 16 + 70 */
	Set_Default_Handler i2c8_intr			/* 16 + 71 */

	Set_Default_Handler idcs_intr			/* 16 + 72 */

	Set_Default_Handler gpio0_intr			/* 16 + 73 */
	Set_Default_Handler gpio1_intr			/* 16 + 74 */
	Set_Default_Handler gpio2_intr			/* 16 + 75 */
	Set_Default_Handler gpio3_intr			/* 16 + 76 */
	Set_Default_Handler gpio4_intr			/* 16 + 77 */
	Set_Default_Handler gpio5_intr			/* 16 + 78 */
	Set_Default_Handler gpio6_intr			/* 16 + 79 */

	Set_Default_Handler uart_apb_intr		/* 16 + 80 */
	Set_Default_Handler irif_intr			/* 16 + 81 */

	Set_Default_Handler r52c0_2_cm3c0		/* 16 + 82 */
	Set_Default_Handler r52c0_2_cm3c1		/* 16 + 83 */
	Set_Default_Handler r52c1_2_cm3c0		/* 16 + 84 */
	Set_Default_Handler r52c1_2_cm3c1		/* 16 + 85 */
	Set_Default_Handler r52c2_2_cm3c0		/* 16 + 86 */
	Set_Default_Handler r52c2_2_cm3c1		/* 16 + 87 */

	.end
