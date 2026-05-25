/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#include <plat.h>
#include <common.h>
#include <task.h>

void delay(uint32_t ticks)
{
	uint32_t cur_tick;
	uint32_t delay_tick = ticks;
	uint32_t start_tick = xTaskGetTickCount();

	do {
		cur_tick = xTaskGetTickCount();

		if((cur_tick - start_tick) > delay_tick) {
			break;
		}
	} while(1);
}

void timer0_udelay(uint32_t n_us)
{
	uint32_t time_ctrl = 0;
	uint32_t ticks_per_us_apb = current_apbfreq() / 1000 / 1000;
	uint32_t ticks = n_us * ticks_per_us_apb;

	writel(TIMER0_BASE + TIMER0_STATUS_OFFSET, ticks);
	writel(TIMER0_BASE + TIMER0_RELOAD_OFFSET, 0);
	writel(TIMER0_BASE + TIMER0_MATCH1_OFFSET, 1);
	writel(TIMER0_BASE + TIMER0_MATCH2_OFFSET, 1);
	time_ctrl = readl(TIMER0_BASE + TIMER_CTRL_OFFSET);
	time_ctrl = (time_ctrl & ~0xF) | 1; /* APBclk && nointererupt && enable */
	writel(TIMER0_BASE + TIMER_CTRL_OFFSET, time_ctrl);

	while (readl(TIMER0_BASE + TIMER0_STATUS_OFFSET) > 0);
}

void udelay(uint32_t n_us)
{
	timer0_udelay(n_us);
}
