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

#ifndef __AMBA_R52_PLAT_H__
#define __AMBA_R52_PLAT_H__

#include <r52_help.h>

/* CV3AD685: 288_SPI,
 * N1_655  : 160_SPI,
 * Use 288_SPI For both chips.
 */
#define SPI_IRQ_RAW_START		(480)
#define INSTALLED_IRQ_VECTORS		(32 + 288)

/**********************************************************/
#define INVLIAD_ADDRESS		0xFFFFFFFF
#define APB_UART_BASE		0xE4000000
#define AHB_UART0_BASE		0xE0017000
#define AHB_UART1_BASE		0xE0018000
#define AHB_UART2_BASE		0xE0019000
#define AHB_UART3_BASE		0xE001A000
#define AHB_UART4_BASE		(INVLIAD_ADDRESS + 0)
#define AHB_UART5_BASE		(INVLIAD_ADDRESS + 1)
#define AHB_UART6_BASE		(INVLIAD_ADDRESS + 2)
#define MAX_VALID_AHB_UART_PORT	5
/**********************************************************/
#define GPIO0_BASE		0xFFF03000
#define GPIO1_BASE		(INVLIAD_ADDRESS + 0)
#define GPIO2_BASE		(INVLIAD_ADDRESS + 1)
#define GPIO3_BASE		(INVLIAD_ADDRESS + 2)
/**********************************************************/
#define WDT0_BASE		0xFFF01000
#define WDT1_BASE		INVLIAD_ADDRESS
#define WDTC0_REG(x)		(WDT0_BASE + (x))
#define WDTC1_REG(x)		(WDT1_BASE + (x))
/**********************************************************/
#define IDC_INSTANCES	(3U)
#define IDCS_BASE		INVLIAD_ADDRESS
#define IDC0_BASE		0xFFF04000
#define IDC1_BASE		0xFFF05000
#define IDC2_BASE		0xFFF06000
#define IDC3_BASE		(INVLIAD_ADDRESS + 0)
#define IDC4_BASE		(INVLIAD_ADDRESS + 1)
#define IDC5_BASE		(INVLIAD_ADDRESS + 2)
#define IDC6_BASE		(INVLIAD_ADDRESS + 3)
#define IDC7_BASE		(INVLIAD_ADDRESS + 4)
#define IDC8_BASE		(INVLIAD_ADDRESS + 5)
/**********************************************************/
#define TIMER0_BASE		0xFFF16000
#define TIMER1_BASE		(INVLIAD_ADDRESS + 0)
#define TIMER2_BASE		(INVLIAD_ADDRESS + 1)

#define TIMER0_STATUS_OFFSET	0x00
#define TIMER0_RELOAD_OFFSET	0x04
#define TIMER0_MATCH1_OFFSET	0x08
#define TIMER0_MATCH2_OFFSET	0x0c
#define TIMER1_STATUS_OFFSET	0x10
#define TIMER1_RELOAD_OFFSET	0x14
#define TIMER1_MATCH1_OFFSET	0x18
#define TIMER1_MATCH2_OFFSET	0x1c
#define TIMER2_STATUS_OFFSET	0x20
#define TIMER2_RELOAD_OFFSET	0x24
#define TIMER2_MATCH1_OFFSET	0x28
#define TIMER2_MATCH2_OFFSET	0x2c
#define TIMER_CTRL_OFFSET	0x30
/**********************************************************/
#define PLL_R52_CTRL_REG	0xED080928
#define PLL_R52_FRAC_REG	0xED08092C
#define PLL_R52_CTRL2_REG	0xED080930
#define PLL_R52_CTRL3_REG	0xED080934

#define PLL_CORE_CTRL_REG	0xED080000
#define PLL_CORE_FRAC_REG	0xED080004
#define PLL_CORE_CTRL2_REG	0xED080100
#define PLL_CORE_CTRL3_REG	0xED080104
/**********************************************************/
#define EL2_CPUFREQ_REG		(64) /* el2 offset 64 bytes just after el2_vector table */
#define EL2_AHBFREQ_REG		(68) /* el2 offset 68 bytes just after el2_vector table */
#define EL1_AHBFREQ_REG		(32) /* el1 offset 32 bytes just after el1_vector table */
/**********************************************************/
#define CFG_SAFETY_EN_REG   (0xFFF17068U)
/**********************************************************/
#ifndef __ASM__
#include <stdint.h>
#include <hwio.h>
#include <common.h>

extern uint32_t g_cpufreq;
extern uint32_t g_apbfreq;
extern uint32_t g_ahbfreq;

extern void arch52_bus_clock_update(void);

static inline unsigned int current_cpufreq(void)
{
	return g_cpufreq;
}

static inline unsigned int current_apbfreq(void)
{
	/* apbfreq = ahbfreq >> 1 */
	return g_apbfreq;
}

static inline unsigned int current_ahbfreq(void)
{
	return g_ahbfreq;
}

static inline uint32_t current_cpuid(void)
{
	return 0;
}
#endif

#endif
