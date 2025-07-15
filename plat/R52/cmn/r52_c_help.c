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
#include <hwio.h>

/* SGI_PPI[0-31]
 * 655:SPI[480-623]  685:SPI[480-743]
 * Use the INSTALLED_IRQ_VECTORS to save BSS size.
 */
static irq_manager_t irq_manager[INSTALLED_IRQ_VECTORS];

/* which address trigger the exception */
uint32_t undef_exception_addr;
uint32_t prfch_exception_addr;
uint32_t abort_exception_addr;

static void gic_irq_disable(uint32_t irq)
{
	uint32_t dist_base = gic_distr_cbar();
	uint32_t redist_base = dist_base + GIC_REDIST_SGIPPI;

	if (irq < 32)
		writel(redist_base + GICD_ICENABLER0, 1 << (irq % 32));
	else
	 	writel(dist_base + GICD_ICENABLER0 + (irq / 32) * 4, 1 << (irq % 32));
}

static void gic_irq_enable(uint32_t irq)
{
	uint32_t dist_base = gic_distr_cbar();
	uint32_t redist_base = dist_base + GIC_REDIST_SGIPPI;

	if (irq < 32)
		writel(redist_base + GICD_ISENABLER0, 1 << (irq % 32));
	else
	 	writel(dist_base + GICD_ISENABLER0 + (irq / 32) * 4, 1 << (irq % 32));
}

static void gic_irq_set_type(uint32_t irq, uint32_t type)
{
	uint32_t value, typemask;
	uint32_t dist_base = gic_distr_cbar();

	switch (type) {
		case IRQ_LEVEL_HIGH:
			type = 0;
			break;
		case IRQ_RISING_EDGE:
			type = 1;
			break;
		default:
			BUG_ON(1, "Invalid gic irq type: %d\n", type);
			break;
	}

	if (irq < 32)	/* SGI and PPI are readonly, can't alert */
		return;

	value = readl(dist_base + GICD_ICFGR0 + (irq / 16 ) * 4);
	typemask = 0x2 << ((irq % 16) * 2);
	if (type)
		value |= typemask;
	else
	 	value &= ~typemask;

	writel(dist_base + GICD_ICFGR0 + (irq / 16 ) * 4, value);
}

/* gic distribute is for SPI used only on R52 */
static void gic_dist_init(void)
{
	uint32_t i, irq_nr = (GIC_TOTAL_IRQ_VECTORS - 32);
	uint32_t dist_base = gic_distr_cbar();

	/* disable group0 and group1 */
	writel(dist_base + GICD_CTLR, 0);

	/* set type: 0: level-sensitive; 1: edge triggered */
	for (i = 32; i < irq_nr; i += 16)
		writel(dist_base + GICD_ICFGR0 + i / 4, 0x0);

	/* set priority, default 0xa0 higher than cpu priority
	 * 0xf8 [7:3]_0_0_0, 5bits to define priority
	 * must be compatilbe with cpu interface settings
	 * let's use 0xa0a0a0a0 < 0xF8F8F8F8
	*/
	for (i = 32; i < irq_nr; i += 4)
		writel(dist_base + GICD_IPRIORITYR0 + i, 0xa0a0a0a0);

	/* disable all spi */
	for (i = 32; i < irq_nr; i += 32)
		writel(dist_base + GICD_ICENABLER0 + i / 8, ~0);

	/* all spi interrupts route to processor 0 [aff0 only] */
	for (i = 32; i < irq_nr; i++)
		writel(dist_base + GICD_IRQROUTE0 + i * 8 , 0);

	/* all spi interrupt route to group1 for IRQ */
	for (i = 32; i < irq_nr; i += 32)
		writel(dist_base + GICD_IGROUPR0 + i / 8, ~0);

	/* Only Enable group1 interrupts forward as IRQ */
	writel(dist_base + GICD_CTLR, 0x2);
}

/* redistribute init: for PPI and SGI only
 * reditribute WAKEUP configure at the startup.S
 * must enable PPI(phys_timer: INTID30) for timer-tick-handler
*/
static void gic_redist_init(void)
{
	uint32_t i;
	uint32_t redist_base = gic_distr_cbar() + GIC_REDIST_SGIPPI;

	/* disable SGI and enable PPI */
	writel(redist_base + GICD_ICENABLER0, 0xFFFFFFFF);
	writel(redist_base + GICD_ICENABLER0, 0xFFFF0000);

	/* set SGI and PPI to group 1 */
	writel(redist_base + GICD_IGROUPR0, 0xFFFFFFFF);

	/* Set priority for PPI and SGI interrupts */
	for (i = 0; i < 32; i += 4)
		writel(redist_base + GICD_IPRIORITYR0 + i, 0xa0a0a0a0);
}

static void gic_percpu_init(void)
{
	/* 32 supported priority levels[0xF8],
	 * set cpu interface to 0xF8 or 0xFF
	 * to pass all interrupt.
	*/
	gic_cpu_set_pmr(0xFF);

	/* use 32-levle interrupt priority */
	gic_cpu_set_bpr0(3);
	gic_cpu_set_bpr1(3);

	/* EOImode = 0(must write EOI to finish interrupt)
	 * and CBPR = 0
	 * must keep compatilbe with portASM.S
	 * ref: FreeRTOS_IRQ_Handler
	 */
	gic_cpu_set_ctrl(0);

	/* group0 disable and group1 enable */
	gic_cpu_set_gp0en(0);
	gic_cpu_set_gp1en(1);
}

/* tick interrupt init for the schedule */
void arch52_tick_event_init(void)
{
	uint32_t syscnt, ticks;

	/* Disable the generic timer */
	phystimer_set_ctrl(0);

	syscnt = sys_freqcnt_get();
	ticks = syscnt / configTICK_RATE_HZ;

	/* set the tval: for next tick event interrupt */
	phystimer_set_tval(ticks);

	/* enable timer interrupt=> bit1:IMASK bit0:Enable(timer interrupt enable) */
	phystimer_set_ctrl(1);
}

void arch52_tick_event_restart(void)
{
	uint32_t syscnt, ticks;

	syscnt = sys_freqcnt_get();
	ticks = syscnt / configTICK_RATE_HZ;

	/* next tick event interrupt */
	phystimer_set_tval(ticks);
}

int request_irq(uint32_t irq, uint32_t type,
	irq_handler_t handler, void *data, const char *name)
{
	uint32_t offset;

	(void)name;
	BUG_ON(irq >= GIC_TOTAL_IRQ_VECTORS);

	gic_irq_disable(irq);

	if (irq < 32)
		offset = irq;
	else
	 	offset = irq - SPI_INT_VEC(SPI_IRQ_RAW_START);
	irq_manager[offset].handler = handler;
	irq_manager[offset].data = data;

	gic_irq_set_type(irq, type);
	DSB();
	gic_irq_enable(irq);

	return 0;
}

extern void FreeRTOS_Tick_Handler(void);
static void arch52_phystimer_handler(void *data)
{
	(void)data;
	FreeRTOS_Tick_Handler();
}

int arch52_system_init(void)
{
	gic_dist_init();
	gic_redist_init();
	gic_percpu_init();
	arch52_bus_clock_update();

	request_irq(PHYS_TIMER_IRQ30, IRQ_LEVEL_HIGH,
		arch52_phystimer_handler, NULL, "tick_isr");

	return 0;
}

/* check portASM.S, FreeRTOS_IRQ_Handler */
void vApplicationIRQHandler(uint32_t irq)
{
	uint32_t offset;
	BUG_ON(irq >= GIC_TOTAL_IRQ_VECTORS);

	irq &= 0x3FF;	/* only 10bits */
	if (irq < 32)
		offset = irq;
	else
	 	offset = irq - SPI_INT_VEC(SPI_IRQ_RAW_START);

	if (irq_manager[offset].handler != NULL)
		irq_manager[offset].handler(irq_manager[offset].data);

	return;
}

/******************** [EL1 undef/abort/prefetch] ************************************/
void arch52_undef_handler(void)
{
	printf("address: 0x%x trigger undef exception \n", undef_exception_addr);
	__asm__ __volatile__("b .");
}

void arch52_abort_handler(void)
{
	printf("address: 0x%x trigger abort exception \n", abort_exception_addr);
	__asm__ __volatile__("b .");
}

void arch52_prfch_handler(void)
{
	printf("address: 0x%x trigger prfch exception \n", prfch_exception_addr);
	__asm__ __volatile__("b .");
}

#if 0
/* can only called at EL2 */
void arch52_set_syscnt_rate(uint32_t rate)
{
	uint32_t cpufreq, syncnt;

	cpufreq = rct_get_integer_pll_freq(readl(PLL_R52_CTRL_REG),
		 readl(PLL_R52_CTRL2_REG), 1, 1);

	syncnt = cpufreq / 12;

	sys_freqcnt_set(syncnt);
}
#endif
