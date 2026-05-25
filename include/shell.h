/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
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

