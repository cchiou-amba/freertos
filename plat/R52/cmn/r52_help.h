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

#ifndef __AMBA_ARCH_52_HELP_H
#define __AMBA_ARCH_52_HELP_H

/* interrupt */
#define IRQ_RISING_EDGE			0
#define IRQ_FALLING_EDGE		1
#define IRQ_BOTH_EDGES			2
#define IRQ_LEVEL_LOW			3
#define IRQ_LEVEL_HIGH			4

#define SGI_INT_VEC(x)			(x)
#define PPI_INT_VEC(x)			(x)
#define SPI_INT_VEC(x)			((x) + 32)

#define SGI_INT_IRQ0			SGI_INT_VEC(0)
#define PHYS_TIMER_IRQ30		PPI_INT_VEC(30)
#define GIC_TOTAL_IRQ_VECTORS		(1024)

/* GIC distribute from IMP_CBAR */
#define GIC_REDIST_BASE			0x100000 /* redistribute offset */
#define GIC_REDIST_WKUP			0x100014 /* redistribute wakeup offset */
#define GIC_REDIST_SGIPPI		0x110000 /* Redistribute SGIPPI offset */

#define GICD_CTLR			0x0000
#define GICD_TYPER			0x0004
#define GICD_IIDR			0x0008
#define GICD_IGROUPR0			0x0080
#define GICD_ISENABLER0			0x0100
#define GICD_ICENABLER0			0x0180
#define GICD_ISPENDR0			0x0200
#define GICD_ICPENDR0			0x0280
#define GICD_ISACTIVER0			0x0300
#define GICD_ICACTIVER0			0x0380
#define GICD_IPRIORITYR0		0x0400
#define GICD_ICFGR0			0x0C00	/* sgi always edge-trigger, read-only 0xAAAA_AAAA */
#define GICD_ICFGR1			0x0C04	/* ppi */
#define GICD_ICFGR2			0x0C08	/* spi */
#define GICD_IRQROUTE0			0x6000	/* GICD_IRQROUTE0 + n * 8, n from 32 */

/* MPU-memory attribute */
#define MEM_ATTR_NORMAL			0xBB
#define MEM_ATTR_NORMAL_NC		0x44
#define MEM_ATTR_DEVICE			0x04

/* MPU-memory attribute index */
#define ATTR_IDX_NORMAL			0b000
#define ATTR_IDX_NORMAL_NC		0b001
#define ATTR_IDX_DEVICE			0b010

/* MPU-SH */
#define SH_NONE				0b00
#define SH_OUTER			0b10
#define SH_INNER			0b11

/* MPU-AP */
#define AP_RW_ELX_ONLY			0b00	/* RW only at EL2/EL1 */
#define AP_RW				0b01
#define AP_RO_ELX_ONLY			0b10	/* RO only at EL2/EL1 */
#define AP_RO				0b11

/* MPU_BAR(64aligned)->LAR */
#define XPRBAR(addr, sh, ap, xn)	(((addr) & 0xffffffc0) | ((sh) << 3) | ((ap) << 1) | (xn))
#define XPRLAR(addr, attridx)		(((addr) & 0xffffffc0) | ((attridx) << 1) | 1)

#define MPU_BR_BIT			(1 << 17)	/* background */
#define MPU_I_BIT			(1 << 12)	/* instruction cache */
#define MPU_C_BIT			(1 << 2)	/* data-cache */
#define MPU_A_BIT			(1 << 1)	/* MPU alignment check */
#define MPU_M_BIT			(1 << 0)	/* MPU enable bit */
#define MPU_CTRL_ENABLE			(MPU_M_BIT | MPU_C_BIT | MPU_I_BIT | MPU_BR_BIT)

/* don't overlap with other address:
 * TCM will override this region
 * ATCM:64KB  BTCM:32KB  CTCM:32KB
*/
#define ATCM_SIZE			0x10000
#define BTCM_SIZE			0x8000
#define CTCM_SIZE			0x8000
#define ATCM_BASE_ADDR			0xF6000000
#define BTCM_BASE_ADDR			(ATCM_BASE_ADDR + ATCM_SIZE)
#define CTCM_BASE_ADDR			(BTCM_BASE_ADDR + BTCM_SIZE)

#ifndef __ASM__

#include <stdint.h>
#include <common.h>

#define configTICK_RATE_HZ	1000

/* arch init */
extern int arch52_system_init(void);

/* tick event */
extern void arch52_tick_event_init(void);
extern void arch52_tick_event_restart(void);

/* dsb dmb isb helper */
extern void DMB(void);
extern void ISB(void);
extern void DSB(void);
extern void __DMB(void);
extern void __ISB(void);
extern void __DSB(void);

/* cpsr : both irq and fiq, not abort */
extern void interrupt_enable(void);
extern void interrupt_disable(void);
extern void irq_enable(void);
extern void fiq_enable(void);
extern void irq_disable(void);
extern void fiq_disable(void);
extern uint32_t interrupt_state(void);

/* vector-base */
void vbase_set_addr(uint32_t base);
void hvbase_set_addr(uint32_t base);

/* Generic timer */
uint32_t sys_freqcnt_get(void);
void sys_freqcnt_set(uint32_t syscnt);
uint64_t phystimer_get_counter(void);
void phystimer_set_counter(uint64_t counter);
void phystimer_set_ctrl(uint32_t ctrl);
void phystimer_set_cmpval(uint64_t cmpval);
void phystimer_set_tval(uint32_t tval);

/* GIC_ICC: group0 use FIQ, group1 use: IRQ */
uint32_t gic_distr_cbar(void);
uint32_t gic_cpu_get_iar0(void);
uint32_t gic_cpu_get_iar1(void);
void gic_cpu_set_eoir0(uint32_t intid);
void gic_cpu_set_eoir1(uint32_t intid);
void gic_cpu_set_bpr0(uint32_t bpval);		/* always set as 3 to use [7:3] => 32 level interrupt priority */
void gic_cpu_set_bpr1(uint32_t bpval);
void gic_cpu_get_bpr0(void);
uint32_t gic_cpu_get_bpr1(void);
void gic_cpu_set_pmr(uint32_t pmrpri);		/* always set as 0xFF to pass all interrupt to cpu */
uint32_t gic_cpu_get_pmr(void);
uint32_t gic_cpu_get_runpri(void);
void gic_cpu_set_ctrl(uint32_t ctrl);		/* always keep CBPR=0 && EOImode=0 */
uint32_t gic_cpu_get_ctrl(void);		/* get the icc_ctrl */
void gic_cpu_set_gp0en(uint32_t en);		/* bit0=1 group0 enable, bit0=0 group0 disable */
void gic_cpu_set_gp1en(uint32_t en);		/* bit0=1 group1 enable, bit0=0 group1 disable */
void gic_cpu_set_sgi0(uint64_t sgi);		/* bit40_IMR=0, all cores include self. IMR=1 all cores exclude self in group0 */
void gic_cpu_set_sgi1(uint64_t sgi);		/* bit40_IMR=0, all cores include self. IMR=1 all cores exclude self in group1 */

typedef void (*irq_handler_t)(void *);
typedef struct irq_manager_s
{
	void *data;
	irq_handler_t handler;
} irq_manager_t;

#endif
#endif
