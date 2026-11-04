/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __AMBA_UART_H__
#define __AMBA_UART_H__

#include <plat.h>

#define UART_LS_OFFSET			0x14
#define UART_LS_TEMT			0x40
#define UART_TH_OFFSET			0x00
#define UART_RB_OFFSET			0x00
#define UART_LC_DLAB			0x80
#define UART_LC_OFFSET			0x0c
#define UART_DLL_OFFSET			0x00
#define UART_IE_OFFSET			0x04
#define UART_DLH_OFFSET			0x04
#define	UART_LC_8N1			0x03
#define UART_SRR_OFFSET			0x88
#define UART_LS_DR			0x01

/* suppose APB_UART used by CPU. AHB_UART used by CM3 */
#define UART_APB_REG(x)			(APB_UART_BASE  + (x))
#define UART0_REG(x)			(AHB_UART0_BASE + (x))
#define UART1_REG(x)			(AHB_UART1_BASE + (x))
#define UART2_REG(x)			(AHB_UART2_BASE + (x))
#define UART3_REG(x)			(AHB_UART3_BASE + (x))
#define UART4_REG(x)			(AHB_UART4_BASE + (x))
#define UART5_REG(x)			(AHB_UART5_BASE + (x))
#define UART6_REG(x)			(AHB_UART6_BASE + (x))

#define UART_REG(n, x)			((n) == 0 ? UART_APB_REG(x): \
					 (n) == 1 ? UART0_REG(x) : \
					 (n) == 2 ? UART1_REG(x) : \
					 (n) == 3 ? UART2_REG(x) : \
					 (n) == 4 ? UART3_REG(x) : \
					 (n) == 5 ? UART4_REG(x) : \
					 (n) == 6 ? UART5_REG(x) : \
						    UART6_REG(x))
#if defined(UART_LOG_CONSOLE)
#if (LOG_CONSOLE > MAX_VALID_AHB_UART_PORT)
#error "Wrong CM3-UART index,Pls Check !"
#define UART_CONSOLE_REG(x)		UART_APB_REG(UART_LOG_CONSOLE, x)
#else
#define UART_CONSOLE_REG(x)		UART_REG(UART_LOG_CONSOLE, x)
#endif
#endif

#define UART_FC_OFFSET			0x08
#define UART_FC_FIFO_EN			0x01
#define UART_FC_RX_RESET		0x02
#define UART_FC_TX_RESET		0x04
#define CG_UART3_REG			0xEC17071CU
#define UART3_IRQ			148U

void console_init(void);
void console_putchar(char c);
int console_getchar(void);
int console_getc_timeout(uint32_t timeout_ms);

#endif
