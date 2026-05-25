/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#include <plat.h>

#if defined(CMVI_LOG_CONSOLE)
#include "cmvi_console.c"
#elif defined(UART_LOG_CONSOLE)
#include "uart_console.c"
#else
void console_putchar(char c) {}
int console_getchar(void) { return -1;}
void console_init(void) {}
#endif
