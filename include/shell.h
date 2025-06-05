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

#ifndef __AMBA_SHELL_H__
#define __AMBA_SHELL_H__

#include <common.h>
#include <task.h>

#define SHELL_PROMPT			"FreeRTOS=> "
#define SHELL_ARG_MAX			8
#define SHELL_CMDLINE_BUFSIZE		(64)
#define SHELL_ASSERT(EX)		do { if (!(EX)) printf("Assert Address: 0x%lx \n", \
					(unsigned long)__builtin_return_address(0)); } while(0)

typedef long (*syscall_func)(void);
typedef int (*cmd_function_t)(int argc, char **argv);

struct shell_syscall {
	const char     *name;       /* the name of system call */
	const char     *desc;       /* description of system call */
	syscall_func    func;       /* the function address of system call */
};

enum shell_stat {
	WAIT_NORMAL,
	WAIT_SPEC_KEY,
	WAIT_FUNC_KEY,
};

struct mcushell {
	enum shell_stat stat;
	char line[SHELL_CMDLINE_BUFSIZE];
	uint16_t line_position;
	uint16_t line_curpos;
};


#define FUNCTION_EXPORT_CMD(name, cmd, desc)                      \
	const char __fsym_##cmd##_name[] ATTR_SECTION(".rodata.name") = #cmd;    \
	const char __fsym_##cmd##_desc[] ATTR_SECTION(".rodata.name") = #desc;   \
	ATTR_USED const struct shell_syscall __fsym_##cmd ATTR_SECTION("cmdlist")= \
	                {                           \
	                                            __fsym_##cmd##_name,    \
	                                            __fsym_##cmd##_desc,    \
	                                            (syscall_func)&name     \
	                };

#define CMD_EXPORT_ALIAS(command, alias, desc) \
	FUNCTION_EXPORT_CMD(command, __cmd_##alias, desc)

#endif

