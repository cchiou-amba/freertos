/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

/*
 * EL2 run code at DRAM and relocate el1 code to TCM and run at TCM
 * EL1 mpu enable ATCM BTCM CTCM DRAM those regions can run code.
 * Here only DRAM and ATCM set execute-rights.
*/
#include <plat.h>

MEMORY
{
	DRAM (rwx)  : ORIGIN = RTOS_BOOT_ADDR, LENGTH = 120K		/* DDR memory */
	ATCM (rwx)  : ORIGIN = 0xf6000000, LENGTH = 64K		/* ATCM memory code & data */
	BTCM (rw)   : ORIGIN = 0xf6010000, LENGTH = 64K		/* BTCM and CTCM for stack and bss */
}

SECTIONS
{
	.el2_boot :
	{
		KEEP(*startup.o (.vectors.el2))
		KEEP(*startup.o (.text.el2))
		*(.text.el2)
	} > DRAM

	.tcm_boot 0xf6000000 : AT (ADDR(.el2_boot) + SIZEOF(.el2_boot))
	{
		__snd_text_start__ = .;
		__vectors_start__ = .;
		KEEP(*startup.o (.text.atcm))
		__vectors_end__ = .;
		*(.text*)
		*(.rodata*)

		/* shell */
		. = ALIGN(8);
		__fsymtab_start = .;
		KEEP(*(*cmdlist*))
		__fsymtab_end = .;

		. = ALIGN(8);
		__data_start__ = .;
		*(.data*)
		. = ALIGN(8);
		__data_end__ = .;
		__snd_text_end__ = .;
	} > ATCM

	.bss (NOLOAD) :
	{
		. = ALIGN(8);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(8);
		__bss_end__ = .;

		. = ALIGN(8);
		__stack_start__ = .;
		. = . + 0x800;
		__svc_stack_top__ = .;
		. = . + 0x400;
		__irq_stack_top__ = .;
		. = . + 0x400;
		__udef_stack_top__  = .;	/* shared stack for undef/abort/prefetch exception */
		__abort_stack_top__ = .;	/* shared stack for undef/abort/prefetch exception */
		__prfch_stack_top__ = .;	/* shared stack for undef/abort/prefetch exception */
		. = . + 0x800;
		__sys_stack_top__ = .;
		. = ALIGN(8);
		__stack_end__  = .;
	} > BTCM

	__relocate_start__ = LOADADDR(.tcm_boot);
	__relocate_end__ =  __relocate_start__ + (__snd_text_end__ - __snd_text_start__);

	/DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*)  *(.ARM.exidx*) }
}
