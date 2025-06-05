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

uint32_t g_cpufreq = -1;
uint32_t g_apbfreq = -1;
uint32_t g_ahbfreq = -1;

#if defined(ARCH_R52)

uint64_t rct_get_integer_pll_freq(uint32_t ctrl, uint32_t ctrl2, uint32_t pres, uint32_t posts)
{
	uint32_t ctrl2_8, ctrl2_9, ctrl2_11, ctrl2_12;
	uint32_t intp, sout, sdiv;
	uint64_t fvco, freq;

	if (pres == 0 || posts == 0) {
		printf("Error: pres(%d) or posts(%d) is zero!\n", pres, posts);
		return 0;
	}

	if (ctrl & 0x20)
		return 0;

	if (ctrl & 0x00200004) {
		intp = REF_CLK_FREQ;
		intp /= pres;
		intp /= posts;
		return intp;
	}

	ctrl2_12 = ((ctrl2 >> 12) & 0x1);
	ctrl2_11 = ((ctrl2 >> 11) & 0x1) + 1;
	ctrl2_9 = ((ctrl2 >> 9) & 0x1) + 1;
	ctrl2_8 = ((ctrl2 >> 8) & 0x1) + 1;

	intp = ((ctrl >> 24) & 0x7f) + 1;
	sout = ((ctrl >> 16) & 0xf) + 1;
	sdiv = ((ctrl >> 12) & 0xf) + 1;

	fvco = REF_CLK_FREQ * ctrl2_8 * ctrl2_9 * sdiv * intp;

	if (ctrl2_12)
		freq = fvco;
	else
		freq = fvco / ctrl2_8 / ctrl2_11 / sout;

	return freq / posts;
}

static uint32_t get_core_bus_freq_hz(void)
{
	return rct_get_integer_pll_freq(readl(PLL_CORE_CTRL_REG), readl(PLL_CORE_CTRL2_REG), 1, 1);
}

void arch52_bus_clock_update(void)
{
	g_ahbfreq = get_core_bus_freq_hz() >> 1;
	g_apbfreq = g_ahbfreq >> 1;
	g_cpufreq = sys_freqcnt_get() * 12;
}

#endif
