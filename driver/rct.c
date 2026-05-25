/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
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
