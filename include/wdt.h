/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __AMBA_WDT_H__
#define __AMBA_WDT_H__

#include <plat.h>

#define WDT_STATUS_REG(id)		((id) == 0 ? WDTC0_REG(0x00) : WDTC1_REG(0x00))
#define WDT_RELOAD_REG(id)		((id) == 0 ? WDTC0_REG(0x04) : WDTC1_REG(0x04))
#define WDT_RESTART_REG(id)		((id) == 0 ? WDTC0_REG(0x08) : WDTC1_REG(0x08))
#define WDT_CTRL_REG(id)		((id) == 0 ? WDTC0_REG(0x0C) : WDTC1_REG(0x0C))
#define WDT_TIMEOUT_REG(id)		((id) == 0 ? WDTC0_REG(0x10) : WDTC1_REG(0x10))
#define WDT_CLR_REG(id)			((id) == 0 ? WDTC0_REG(0x14) : WDTC1_REG(0x14))
#define WDT_RST_WIDTH_REG(id)		((id) == 0 ? WDTC0_REG(0x18) : WDTC1_REG(0x18))
#define WDT_CW_THRSHLD(id)		((id) == 0 ? WDTC0_REG(0x1C) : WDTC1_REG(0x1C))
#define WDT_INT_WIDTH(id)		((id) == 0 ? WDTC0_REG(0x20) : WDTC1_REG(0x20))
#define WDT_FAULT_INJECT(id)		((id) == 0 ? WDTC0_REG(0x24) : WDTC1_REG(0x24))
#define WDT_DUMMY_REG(id)		((id) == 0 ? WDTC0_REG(0x28) : WDTC1_REG(0x28))

void watchdog_init(uint32_t id);
void watchdog_keepalive(uint32_t id, uint32_t sec, uint32_t wdtfreq);
void watchdog_interrupt_enable(uint32_t id, int enable);

#endif
