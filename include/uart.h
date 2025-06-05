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

void console_init(void);

#endif
