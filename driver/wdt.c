/*******************************************************************************
 *
 * Copyright (c) 2023 Ambarella International LP
 *
 * This file and its contents ("Software") are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella International LP and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any idion thereof except pursuant to a signed license agreement
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
