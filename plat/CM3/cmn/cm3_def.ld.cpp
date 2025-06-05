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
	RAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 32K
}
ENTRY(Reset_Handler)

_Min_Stack_Size = 2048;		/* default msp stack size */
VECTOR_RAM_SIZE = 128 * 4;	/* aligned to 128 interrupt vectors */

SECTIONS
{
	.text :
	{
		__vectors_start__ = .;
		KEEP(*startup.o (.text.vector))
		__vectors_end__ = .;

		/* other code */
		KEEP(*startup.o (.text))
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
		KEEP(*(.eh_frame))
		KEEP(*(.rodata*))
		. = ALIGN(4);
		_etext = .;
	} > RAM

	.ARM.extab :
	{
		. = ALIGN(4);
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		. = ALIGN(4);
	} >RAM

	.ARM :
	{
		. = ALIGN(4);
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
		. = ALIGN(4);
	} >RAM

	_sidata = LOADADDR(.data);

	.data :
	{
		. = ALIGN(4);
		_sdata = .;
		*(.data*)
		. = ALIGN(4);
		_edata = .;
	} > RAM

	.resource_table :
	{
		KEEP(*(.resource_table))
	} > RAM

	.bss.noinit (NOLOAD):
	{
		. = ALIGN(8);
		KEEP(*(.bss.noinit*))
		. = ALIGN(8);
	} > RAM
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
	} >RAM

	/DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }

	_estack = ORIGIN(RAM) + LENGTH(RAM);
	__StackTop = ORIGIN(RAM) + LENGTH(RAM) - 16;	/* Last 16 bytes store someinfo from CPU */
	__StackTop &= ~7; 				/* Stack must align to 8bytes */
	__StackLimit = __StackTop - _Min_Stack_Size;
	PROVIDE(__stack = __StackTop);
	ASSERT(__StackLimit >= __bss_end__, "CM3 MSP Stack Overflow")
	ASSERT((__vectors_end__ - __vectors_start__) == VECTOR_RAM_SIZE, "CM3 Vector Not Aligned")
}
