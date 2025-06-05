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
