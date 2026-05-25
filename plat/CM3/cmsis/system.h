/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef SYSTEM_ARMCM3_H
#define SYSTEM_ARMCM3_H

#include <stdint.h>

extern uint32_t SystemCoreClock;
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

#endif /* SYSTEM_ARMCM3_H */
