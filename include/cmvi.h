/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __AMBA_CMVI_H__
#define __AMBA_CMVI_H__

#include <plat.h>

/* scratchpad register */
#define CM3_IPIIRQ_OFFSET			(0x0)
#define CM3_SYSCNT_L32_OFFSET			(0x4)
#define CM3_SYSCNT_H32_OFFSET			(0x8)

#define CM3_PWR_GATE_OFFSET			(0xC)
#define PWR_GATE_SHUTDOWN_PI_MASK		(0x1)
#define PWR_GATE_ISOLATE_EN_MASK		(0x2)
#define PWR_GATE_ISOLATE_SW_RST_MASK		(0x4)

#define CM3_BOOT_STAT_OFFSET			(0x10)
#define CM3_BOOT_RDY_MASK			(1)

#define CM3_PI_RESET_MONITOR_OFFSET		(0x14)
#define PI_GREST_MONITOR_MASK			(0x2)
#define PI_POR_MONITOR_MASK			(0x1)

/* CM3 use this to trigger PI reset
 * when recving PI watchdog timeout interrupt
*/
#define CM3_RTC_CTRL_REG			(0x18)
#define CM3_WARM_RESET_PI_OFFSET		(CM3_RTC_CTRL_REG)
#define WARM_RESET_PI_MASK			(0x1)

/* tell A76 watchdog timeout */
#define CM3_PI_WDT_TMOUT_STAT_OFFSET		(0x1C)
#define WDT_MORC_TMOUT_MASK			(0x4)
#define WDT_1_TMOUT_MASK			(0x2)
#define WDT_0_TMOUT_MASK			(0x1)
#define WDT_TMOUT_MASK                          (WDT_0_TMOUT_MASK | WDT_1_TMOUT_MASK | WDT_MORC_TMOUT_MASK)

/* external wakeup source to resume PI power */
#define CM3_VBUS_WAKEUP_OFFSET			(0x44)
#define VBUS_WAKEUP_MASK			(0x1)


#define CM3_ALARM_EN_OFFSET			(0x44)
#define ALARM_EN_MASK				(0x1)
#define CM3_ALARM_TIME_OFFSET			(0x48)

#define CM3_CMVI_SPI_CLKDIV_OFFSET		(0x4C)

#define CM3_OSC_TUNE_OFFSET			(0x50)
/* default value: 0xa */
#define OSC_TNUE_MASK				(0xFF)

/* CM3 power-up-down gap */
#define CM3_PWR_DOWN_WIDHT_OFFSET		(0x54)
#define CM3_PWR_SEQ_GAP_OFFSET			(CM3_PWR_DOWN_WIDHT_OFFSET)

#define CM3_PWR_SEQ_STAT_OFFSET			(0x58)
#define PWR_SEQ_POR_LATCH_MASK			(0x1)
#define PWR_SEQ_CM3_WDT_LATCH_MASK		(0x2)
#define PWR_SEQ_CLK_AXI_FROM_PI_MASK		(0x4)

/* samsara mode: change 1 bit each time */
#define CM3_PWR_SEQ_OFFSET			(0x60)
#define PWR_SEQ1_MASK				(0x1)
#define PWR_SEQ2_MASK				(0x2)
#define PWR_SEQ3_MASK				(0x4)

#define CM3_SW_RST_OFFSET			(0x64)
#define SPI_SW_RST_MASK				(0x1)
#define IDC_SW_RST_MASK				(0x2)
#define WDT_SW_RST_MASK				(0x4)
#define TMR_SW_RST_MASK				(0x8)
#define GPIO_SW_RST_MASK			(0x10)
#define IIOMUX_SW_RST_MASK			(0x20)

#define CM3_PI_POR_OFFSET			(0x68)
#define PI_POR_MASK				(0x1)

#define CM3_DUMMY_REG0_OFFSET			(0x100)
#define CM3_DUMMY_REG1_OFFSET			(0x104)
#define CM3_DUMMY_REG2_OFFSET			(0x108)
#define CM3_DUMMY_REG3_OFFSET			(0x10C)

#define CM3_IOPAD_DS0_OFFSET			(0x110)
#define CM3_IOPAD_DS1_OFFSET			(0x114)
#define CM3_IOPAD_PLLEN_OFFSET			(0x118)
#define CM3_IOPAD_PLLSEL_OFFSET			(0x11c)
#define CM3_IOPAD_MISC_OFFSET			(0x120)

#define CM3_RTC_CURT_OFFSET			(0x124)

#define POWER_SEQ_OFF		(0)
#define POWER_SEQ_ON		(1)
#define POWER_NO_SRS		(0)
#define POWER_WITH_SRS		(1)

/* T.B.D: delay the interrupt to task */
#define CMVI_EVT_WKUP			(1 << 0)
#define CMVI_EVT_SRS_SLP		(1 << 1)
#define CMVI_EVT_POR			(1 << 2)
#define CMVI_EVT_WDT0			(1 << 3)
#define CMVI_EVT_WDT1			(1 << 4)
#define CMVI_EVT_USBPLG			(1 << 5)
#define CMVI_EVT_RTCALRM		(1 << 6)
#define CMVI_EVT_POWER_OFF		(1 << 7)
#define CMVI_EVT_CUSTOM_OP0		(1 << 8)
#define CMVI_EVT_CUSTOM_OP1		(1 << 9)
#define CMVI_EVT_CUSTOM_OP2		(1 << 10)
#define CMVI_EVT_CUSTOM_OP3		(1 << 11)
#define CMVI_EVT_PWR_TEST		(1 << 12)

#define CMVI_EVT_BITS			(CMVI_EVT_WKUP | CMVI_EVT_SRS_SLP| \
					CMVI_EVT_POR | CMVI_EVT_WDT0 | CMVI_EVT_WDT1 | \
					CMVI_EVT_USBPLG | CMVI_EVT_RTCALRM | CMVI_EVT_POWER_OFF | \
					CMVI_EVT_CUSTOM_OP0 | CMVI_EVT_CUSTOM_OP1 | CMVI_EVT_CUSTOM_OP2 | \
					CMVI_EVT_CUSTOM_OP3)

extern void vPortSetupTimerInterruptDynamic(uint32_t cpufreq);
#define CMVI_PI_OFF_CM3FREQ		(22000000)	/* internal osc range: 10M~22M */
#define CMVI_PWRSEQ_GAP_TICK		(10)		/* range: 10ms~15ms */

extern void cmvi_delay(volatile uint32_t loops);
extern void cmvi_entry_init(void);
extern void board_cmvi_dbgled_blink(void);
extern void cmvi_por_pi_prepare(void);
extern void cmvi_por_pi_unprepare(void);
extern void cmvi_powerseq_ops(int32_t on, int32_t srs);
extern void cmvi_cm3_mark_bootdone(void);
extern void cmvi_isr_event_setbits(uint32_t bitmask);
extern uint32_t cmvi_enter_critical(void);
extern void cmvi_exit_critical(uint32_t flag);

static inline uint32_t cmvi_spi_clkrate(void)
{
	return current_cpufreq() /
		(readl(CM3_SCRATCHPAD_REG(CM3_CMVI_SPI_CLKDIV_OFFSET)) & 0x00FFFFFF);
}
#endif
