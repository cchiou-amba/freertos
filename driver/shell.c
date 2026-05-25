/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include <shell.h>

static struct mcushell dbg_shell;
static struct mcushell *shell;
static struct shell_syscall *_syscall_table_begin  = NULL;
static struct shell_syscall *_syscall_table_end    = NULL;

ATTR_WEAK int console_getchar(void)
{
	return -1;

}

ATTR_WEAK int printf(const char *format, ...)
{
	return -1;
}

int cmd_help(int argc, char **argv)
{
	printf("shell commands:\n");
	{
		struct shell_syscall *index;

		for(index = _syscall_table_begin;
		                index < _syscall_table_end;
		                index++) {
			if(strncmp(index->name, "__cmd_", 6) != 0) {
				continue;
			}

			printf("%-16s - %s\n", &index->name[6], index->desc);
		}
	}
	printf("\n");
	return 0;
}
CMD_EXPORT_ALIAS(cmd_help, help, shell help);

static int shell_split(char *cmd, unsigned long length, char *argv[SHELL_ARG_MAX])
{
	char *ptr;
	unsigned long position;
	unsigned long argc;
	unsigned long i;
	ptr = cmd;
	position = 0;
	argc = 0;

	while(position < length) {
		/* strip bank and tab */
		while((*ptr == ' ' || *ptr == '\t') && position < length) {
			*ptr = '\0';
			ptr ++;
			position ++;
		}

		if(argc >= SHELL_ARG_MAX) {
			printf("Too many args ! We only Use:\n");

			for(i = 0; i < argc; i++) {
				printf("%s ", argv[i]);
			}

			printf("\n");
			break;
		}

		if(position >= length) {
			break;
		}

		if(*ptr == '"') {
			ptr ++;
			position ++;
			argv[argc] = ptr;
			argc ++;

			/* skip this string */
			while(*ptr != '"' && position < length) {
				if(*ptr == '\\') {
					if(* (ptr + 1) == '"') {
						ptr ++;
						position ++;
					}
				}

				ptr ++;
				position ++;
			}

			if(position >= length) {
				break;
			}

			/* skip '"' */
			*ptr = '\0';
			ptr ++;
			position ++;
		} else {
			argv[argc] = ptr;
			argc ++;

			while((*ptr != ' ' && *ptr != '\t') && position < length) {
				ptr ++;
				position ++;
			}

			if(position >= length) {
				break;
			}
		}
	}

	return argc;
}

static cmd_function_t shell_get_cmd(char *cmd, int size)
{
	struct shell_syscall *index;
	cmd_function_t cmd_func = NULL;

	for(index = _syscall_table_begin; index < _syscall_table_end; index++) {
		if(strncmp(index->name, "__cmd_", 6) != 0) {
			continue;
		}

		if(strncmp(&index->name[6], cmd, size) == 0 &&
		                index->name[6 + size] == '\0') {
			cmd_func = (cmd_function_t)index->func;
			break;
		}
	}

	return cmd_func;
}

static int _shell_run_cmd(char *cmd, unsigned long length, int *retp)
{
	int argc;
	unsigned long cmd0_size = 0;
	cmd_function_t cmd_func;
	char *argv[SHELL_ARG_MAX];
	SHELL_ASSERT(cmd);
	SHELL_ASSERT(retp);

	/* find the size of first command */
	while((cmd[cmd0_size] != ' ' && \
	                cmd[cmd0_size] != '\t') && cmd0_size < length) {
		cmd0_size ++;
	}

	if(cmd0_size == 0) {
		return -1;
	}

	cmd_func = shell_get_cmd(cmd, cmd0_size);

	if(cmd_func == NULL) {
		return -1;
	}

	memset(argv, 0x00, sizeof(argv));
	argc = shell_split(cmd, length, argv);

	if(argc == 0) {
		return -1;
	}

	/* exec this command */
	*retp = cmd_func(argc, argv);
	return 0;
}

static int shell_run(char *cmd, unsigned long length)
{
	int cmd_ret;

	/* strim the beginning of command */
	while((length > 0) && (*cmd == ' ' || *cmd == '\t')) {
		cmd++;
		length--;
	}

	if(length == 0) {
		return 0;
	}

	if(_shell_run_cmd(cmd, length, &cmd_ret) == 0) {
		return cmd_ret;
	}

	{
		char *tcmd;
		tcmd = cmd;

		while(*tcmd != ' ' && *tcmd != '\0') {
			tcmd++;
		}

		*tcmd = '\0';
	}

	printf("%s: command not found.\n", cmd);
	return -1;
}

static int str_common(const char *str1, const char *str2)
{
	const char *str = str1;

	while((*str != 0) && (*str2 != 0) && (*str == *str2)) {
		str ++;
		str2 ++;
	}

	return (str - str1);
}

static void auto_complete(char *prefix)
{
	int length, min_length;
	const char *name_ptr, *cmd_name;
	struct shell_syscall *index;
	min_length = 0;
	name_ptr = NULL;

	if(*prefix == '\0') {
		cmd_help(0, NULL);
		return;
	}

	{
		for(index = _syscall_table_begin; index < _syscall_table_end; index++) {
			if(strncmp(index->name, "__cmd_", 6) != 0) {
				continue;
			}

			cmd_name = (const char *) &index->name[6];

			if(strncmp(prefix, cmd_name, strlen(prefix)) == 0) {
				if(min_length == 0) {
					name_ptr = cmd_name;
					min_length = strlen(name_ptr);
				}

				length = str_common(name_ptr, cmd_name);

				if(length < min_length) {
					min_length = length;
				}

				printf("%s\n", cmd_name);
			}
		}
	}

	if(name_ptr != NULL) {
		strncpy(prefix, name_ptr, min_length);
	}

	return ;
}

static void shell_auto_complete(char *prefix)
{
	printf("\n");
	auto_complete(prefix);
	printf("%s%s", SHELL_PROMPT, prefix);
}

void shell_task_entry(void *parameter)
{
	int ch;
	shell = &dbg_shell;
	printf(SHELL_PROMPT);

	while(1) {
		ch = console_getchar();

		if(ch < 0) {
			vTaskDelay(100);
			continue;
		}

		if(ch == 0x1b) {
			shell->stat = WAIT_SPEC_KEY;
			continue;
		} else if(shell->stat == WAIT_SPEC_KEY) {
			if(ch == 0x5b) {
				shell->stat = WAIT_FUNC_KEY;
				continue;
			}

			shell->stat = WAIT_NORMAL;
		} else if(shell->stat == WAIT_FUNC_KEY) {
			shell->stat = WAIT_NORMAL;

			if (ch == 0x44) {
				if (shell->line_curpos) {
					printf("\b");
					shell->line_curpos--;
				}
			} else if (ch == 0x43) {
				if (shell->line_curpos < shell->line_position) {
					printf("%c", shell->line[shell->line_curpos]);
					shell->line_curpos++;
				}
			}
			continue;
		}

		if(ch == '\0' || ch == 0xFF) {
			continue;
		} else if(ch == '\t') {
			int i;

			/* move the cursor to the beginning of line */
			for(i = 0; i < shell->line_curpos; i++) {
				printf("\b");
			}

			/* auto complete */
			shell_auto_complete(&shell->line[0]);
			/* re-calculate position */
			shell->line_curpos = shell->line_position = strlen(shell->line);
			continue;
		} else if(ch == 0x7f || ch == 0x08) {
			/* note that shell->line_curpos >= 0 */
			if(shell->line_curpos == 0) {
				continue;
			}

			shell->line_position--;
			shell->line_curpos--;

			if(shell->line_position > shell->line_curpos) {
				int i;
				memmove(&shell->line[shell->line_curpos],
				        &shell->line[shell->line_curpos + 1],
				        shell->line_position - shell->line_curpos);
				shell->line[shell->line_position] = 0;
				printf("\b%s  \b", &shell->line[shell->line_curpos]);

				/* move the cursor to the origin position */
				for(i = shell->line_curpos; i <= shell->line_position; i++) {
					printf("\b");
				}
			} else {
				printf("\b \b");
				shell->line[shell->line_position] = 0;
			}

			continue;
		}

		if(ch == '\r' || ch == '\n') {
			printf("\n");
			shell_run(shell->line, shell->line_position);
			printf(SHELL_PROMPT);
			memset(shell->line, 0, sizeof(shell->line));
			shell->line_curpos = shell->line_position = 0;
			continue;
		}

		if(shell->line_position >= SHELL_CMDLINE_BUFSIZE) {
			shell->line_position = 0;
		}

		if(shell->line_curpos < shell->line_position) {
			int i;
			memmove(&shell->line[shell->line_curpos + 1],
			        &shell->line[shell->line_curpos],
			        shell->line_position - shell->line_curpos);
			shell->line[shell->line_curpos] = ch;
			printf("%s", &shell->line[shell->line_curpos]);
			/* move the cursor to new position */
			for(i = shell->line_curpos; i < shell->line_position; i++) {
				printf("\b");
			}
		} else {
			shell->line[shell->line_position] = ch;
			printf("%c", ch);
		}

		ch = 0;
		shell->line_position ++;
		shell->line_curpos++;

		if(shell->line_position >= SHELL_CMDLINE_BUFSIZE) {
			/* clear command line */
			shell->line_position = 0;
			shell->line_curpos = 0;
		}
	}
}

void shell_init(void *arg)
{
	extern const int __fsymtab_start;
	extern const int __fsymtab_end;

	_syscall_table_begin = (struct shell_syscall *) &__fsymtab_start;
	_syscall_table_end = (struct shell_syscall *) &__fsymtab_end;

	shell_task_entry(arg);
}

/* shell task */
#define task_shell_stack_SIZE  (configMINIMAL_STACK_SIZE)
#define task_shell_PRIORITY    (tskIDLE_PRIORITY + 2 )
static StackType_t  task_shell_stack[task_shell_stack_SIZE];
static StaticTask_t task_shell;

void shell_task_init(void)
{
	extern void shell_init(void *arg);
	xTaskCreateStatic(
		shell_init,
		"main",
		task_shell_stack_SIZE,
		NULL,
		task_shell_PRIORITY,
		task_shell_stack,
		&task_shell);
}
