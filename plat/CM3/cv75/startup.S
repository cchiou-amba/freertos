/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 *************************First stage code*********************************/
	.syntax  unified
	.arch    armv7 - m
	.section .text.fst_boot_vector
	.align   2
	.globl   start_Vectors
start_Vectors:
	.long    0x7ff0 		/* End of the Internal 32KB SRAM */
	.long    start_handler		/* Reset Handler */
	.long    nmi_handler 		/* -14 NMI Handler */
	.long    hardfault_handler	/* -13 Hard Fault Handler */
	.long    0			/* -12 MPU Fault Handler */
	.long    0			/* -11 Bus Fault Handler */
	.long    0			/* -10 Usage Fault Handler */
	.long    0			/*  -9   Reserved */
	.long    0			/*  -8   Reserved */
	.long    0			/*  -7   Reserved */
	.long    0			/*  -6   Reserved */
	.long    0			/*  -5   Reserved */
	.long    0			/*  -4   Reserved */
	.long    0			/*  -3   Reserved */
	.long    0			/*  -2   Reserved */
	.long    0			/*  -1   Reserved */
/****************************************************************************/

	.thumb
	.section .text.fst_boot_text
	.align   2
	.thumb_func
	.type    hardfault_handler, % function
hardfault_handler:
	b        .

	.thumb
	.section .text.fst_boot_text
	.align   2
	.thumb_func
	.type    nmi_handler, % function
nmi_handler:
	b        .

	.thumb
	.section .text.fst_boot_text
	.align   2
	.thumb_func
	.type    start_handler, % function
	.globl   start_handler
start_handler:
	ldr	r0, = __relocate_start__
	ldr	r1, = __relocate_end__
	ldr	r2, = __snd_ram_start__
loop:
	ldr	r3, [r0], #4
	str	r3, [r2], #4
	cmp	r0, r1
	blt	loop
	dsb
	isb

	/* jump to sndstage code */
	ldr 	r2, =__snd_ram_start__
	ldr	r2, [r2, #4]
	orr	r2, r2, #1
	mov 	r15, r2
	dsb
	b	.

/*******************Following is the sndstage code**********************/
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
	.long    0			/*  -9   Reserved */
	.long    0			/*  -8   Reserved */
	.long    0			/*  -7   Reserved */
	.long    0			/*  -6   Reserved */
	.long    vPortSVCHandler	/*  -5 SVC Handler */
	.long    DebugMon_Handler	/*  -4 Debug Monitor Handler */
	.long    0			/*  -3 Reserved */
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

	.long i2c0_intr			/* 16 + 42 */
	.long i2c1_intr			/* 16 + 43 */
	.long i2c2_intr			/* 16 + 44 */
	.long i2c3_intr			/* 16 + 45 */
	.long idcs_intr			/* 16 + 46 */

	.long gpio0_intr		/* 16 + 47 */
	.long gpio1_intr		/* 16 + 48 */
	.long gpio2_intr		/* 16 + 49 */

	.long dummy_intr		/* 16 + 50 */

	.long adc_intr			/* 16 + 51 */
	.long uart_apb_intr		/* 16 + 52 */
	.long irif_intr			/* 16 + 53 */

	.long wdt0_pi_intr			/* 16 + 54 */
	.long wdt1_pi_intr			/* 16 + 55 */
	.long wdt_morc_pi_intr		/* 16 + 56 */

	.long cmvi_i2c_intr		/* 16 + 57 */
	.long cmvi_ssi_intr		/* 16 + 58 */
	.long cmvi_tim1_intr		/* 16 + 59 */
	.long cmvi_tim2_intr		/* 16 + 60 */
	.long cmvi_tim3_intr		/* 16 + 61 */
	.long cmvi_tim4_intr		/* 16 + 62 */
	.long cmvi_tim5_intr		/* 16 + 63 */
	.long cmvi_tim6_intr		/* 16 + 64 */
	.long cmvi_tim7_intr		/* 16 + 65 */
	.long cmvi_tim8_intr		/* 16 + 66 */
	.long cmvi_tim9_intr		/* 16 + 67 */
	.long cmvi_tim10_intr		/* 16 + 68 */
	.long cmvi_vbus_connect_intr	/* 16 + 69 */
	.long cmvi_rtc_alarm_intr	/* 16 + 70 */
	.long cmvi_gpio_intr		/* 16 + 71 */
	.long pi_ext_por_deassert	/* 16 + 72 */

	.long dummy_intr		/* 16 + 73 */
	.long dummy_intr		/* 16 + 74 */
	.long dummy_intr		/* 16 + 75 */
	.long dummy_intr		/* 16 + 76 */
	.long dummy_intr		/* 16 + 77 */
	.long dummy_intr		/* 16 + 78 */
	.long dummy_intr		/* 16 + 79 */

	/* 16 + 80 + 32 => 128 must align to 2^n;
	 * vector align to 128*4=512
	 */
	.space(32 * 4)

__Vectors_End:
.equ     __Vectors_Size, __Vectors_End - __Vectors
.size    __Vectors, . - __Vectors

	.thumb
	.section .text.snd_boot
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

	/* always manual set MSP(snd stage jump) */
	ldr	r0, =__Vectors
	ldr	r0, [r0, #0]
	msr	msp, r0
	dsb	sy
	isb

	/* vector will be set in SystemInit */
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
	Set_Default_Handler  timer1_intr8	/* 16 + 29 */
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

	Set_Default_Handler  i2c0_intr		/* 16 + 42 */
	Set_Default_Handler  i2c1_intr		/* 16 + 43 */
	Set_Default_Handler  i2c2_intr		/* 16 + 44 */
	Set_Default_Handler  i2c3_intr		/* 16 + 45 */
	Set_Default_Handler  idcs_intr		/* 16 + 46 */

	Set_Default_Handler  gpio0_intr		/* 16 + 47 */
	Set_Default_Handler  gpio1_intr		/* 16 + 48 */
	Set_Default_Handler  gpio2_intr		/* 16 + 49 */

	Set_Default_Handler  dummy_intr		/* 16 + 50 */

	Set_Default_Handler  adc_intr		/* 16 + 51 */
	Set_Default_Handler  uart_apb_intr	/* 16 + 52 */
	Set_Default_Handler  irif_intr		/* 16 + 53 */

	Set_Default_Handler  wdt0_pi_intr		/* 16 + 54 */
	Set_Default_Handler  wdt1_pi_intr		/* 16 + 55 */
	Set_Default_Handler  wdt_morc_pi_intr		/* 16 + 56 */

	Set_Default_Handler  cmvi_i2c_intr		/* 16 + 57 */
	Set_Default_Handler  cmvi_ssi_intr		/* 16 + 58 */
	Set_Default_Handler  cmvi_tim1_intr		/* 16 + 59 */
	Set_Default_Handler  cmvi_tim2_intr		/* 16 + 60 */
	Set_Default_Handler  cmvi_tim3_intr		/* 16 + 61 */
	Set_Default_Handler  cmvi_tim4_intr		/* 16 + 62 */
	Set_Default_Handler  cmvi_tim5_intr		/* 16 + 63 */
	Set_Default_Handler  cmvi_tim6_intr		/* 16 + 64 */
	Set_Default_Handler  cmvi_tim7_intr		/* 16 + 65 */
	Set_Default_Handler  cmvi_tim8_intr		/* 16 + 66 */
	Set_Default_Handler  cmvi_tim9_intr		/* 16 + 67 */
	Set_Default_Handler  cmvi_tim10_intr		/* 16 + 68 */

	Set_Default_Handler  cmvi_vbus_connect_intr	/* 16 + 69 */
	Set_Default_Handler  cmvi_rtc_alarm_intr	/* 16 + 70 */
	Set_Default_Handler  cmvi_gpio_intr		/* 16 + 71 */
	Set_Default_Handler  pi_ext_por_deassert	/* 16 + 72 */

	.end
