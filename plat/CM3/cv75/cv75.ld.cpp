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


OUTPUT_FORMAT("elf32-littlearm")
MEMORY
{
	RAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 32K  /* SRAM(share with CPU) */
	IRAM (xrw) : ORIGIN = 0x00010000, LENGTH = 64K  /* MCU Internal RAM */
}
ENTRY(start_handler)

_Min_Stack_Size = 2048;		/* default msp stack size */
VECTOR_RAM_SIZE = 128 * 4;	/* CV75 aligned to 128 interrupt vectors */

SECTIONS
{
	.fst_boot :
	{
		KEEP(*startup.o (.text.fst_boot_vector))
		KEEP(*startup.o (.text.fst_boot_text))
		. = ALIGN(4);
	} > RAM

	.snd_boot 0x00010000 : AT (ADDR(.fst_boot) + SIZEOF(.fst_boot))
	{
		__snd_ram_start__ = .;
		__vectors_start__ = .;
		KEEP(*startup.o (.text.vector))
		__vectors_end__ = .;

		/* other start code */
		KEEP(*startup.o (.text.snd_boot))
		. = ALIGN(4);
		*(.text*)

		/* shell */
		. = ALIGN(4);
		__fsymtab_start = .;
		KEEP(*(*cmdlist*))
		__fsymtab_end = .;
		. = ALIGN(4);

		KEEP (*(.init))
		KEEP (*(.fini))
		KEEP(*(.rodata*))

		. = ALIGN(4);
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		. = ALIGN(4);

		. = ALIGN(4);
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
		. = ALIGN(4);

		/* data */
		. = ALIGN(4);
		*(.data*)
		. = ALIGN(4);
	} > IRAM

	.resource_table :
	{
		KEEP(*(.resource_table))
	} > IRAM
	. = ALIGN(4);
	__snd_ram_end__ = .;

	.bss.noinit (NOLOAD):
	{
		. = ALIGN(8);
		KEEP(*(.bss.noinit*))
		. = ALIGN(8);
	} > IRAM
	_bss_noinit = .;

	.bss :
	{
		. = ALIGN(8);
		_sbss = .;
		__bss_start__ = _sbss;
		*(.bss*)
		*(COMMON)
		. = ALIGN(8);
		_ebss = .;
		__bss_end__ = _ebss;
	} >IRAM

	. = ALIGN(4);
	__relocate_start__ = LOADADDR(.snd_boot);
	__relocate_end__ =  __relocate_start__ + (__snd_ram_end__ - __snd_ram_start__);

	/DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }

	_estack = ORIGIN(IRAM) + LENGTH(IRAM);
	__StackTop = ORIGIN(IRAM) + LENGTH(IRAM) - 16;	/* Last 16 bytes store someinfo from CPU(deprecated) */
	__StackTop &= ~7; 				/* Stack must align to 8bytes */
	__StackLimit = __StackTop - _Min_Stack_Size;
	PROVIDE(__stack = __StackTop);
	ASSERT(__StackLimit >= __bss_end__, "CV75-CM3 MSP Stack Overflow")
	ASSERT((__vectors_end__ - __vectors_start__) == VECTOR_RAM_SIZE, "CV72-CM3 Vector Not Aligned")
}
