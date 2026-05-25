/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#include <uart.h>

static int console_poll(void)
{
	return !!(readb(UART_CONSOLE_REG(UART_LS_OFFSET)) & UART_LS_DR);
}

void console_putchar(char c)
{
	while(!(readb(UART_CONSOLE_REG(UART_LS_OFFSET)) & UART_LS_TEMT));

	writeb(UART_CONSOLE_REG(UART_TH_OFFSET), c);
}

int console_getchar(void)
{
	if (!console_poll())
		return -1;
	else
		return readb(UART_CONSOLE_REG(UART_RB_OFFSET));
}

void console_init(void)
{
	const char str[] = "\x1b[4l\r\n";

	for(int i = 0; i < sizeof(str) - 1; i++) {
		console_putchar(str[i]);
	}
}
