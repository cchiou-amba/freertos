/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#include <wdt.h>
#include <stdint.h>

void watchdog_disable(uint32_t id)
{
	writel(WDT_CTRL_REG(id), 0);
}

void watchdog_enable(uint32_t id)
{
	writel(WDT_CTRL_REG(id), 3);
}

void watchdog_interrupt_enable(uint32_t id, int enable)
{
	uint32_t ctrl = readl(WDT_CTRL_REG(id));
	if (enable)
		ctrl |= 0x4; /* inten */
	else
		ctrl &=~0x4;
	writel(WDT_CTRL_REG(id), ctrl);
}

void watchdog_init(uint32_t id)
{
	writel(WDT_CLR_REG(id), 0x1);
	writel(WDT_RELOAD_REG(id), -1);
	writel(WDT_RESTART_REG(id), 0x4755);
	writel(WDT_CTRL_REG(id), 0xF); /* soft && external if signal can pass */
	writel(WDT_RST_WIDTH_REG(id), 0x1000);
}

void watchdog_keepalive(uint32_t id, uint32_t sec, uint32_t wdtfreq)
{
	uint32_t ticks = wdtfreq * sec;
	writel(WDT_RELOAD_REG(id), ticks);
	writel(WDT_RESTART_REG(id), 0x4755);
}
