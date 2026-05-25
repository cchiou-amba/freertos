 /*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#ifndef __AMBA_IDC_H__
#define __AMBA_IDC_H__

#include <plat.h>

#define IDC_ENR_OFFSET				(0x00)
#define IDC_CTRL_OFFSET				(0x04)
#define IDC_DATA_OFFSET				(0x08)
#define IDC_STS_OFFSET				(0x0c)
#define IDC_PSLL_OFFSET				(0x10)
#define IDC_PSLH_OFFSET				(0x14)
#define IDC_FMCTRL_OFFSET			(0x18)
#define IDC_FMDATA_OFFSET			(0x1c)
#define IDC_PSHS_OFFSET				(0x20)
#define IDC_DUTYCYCLE_OFFSET			(0x24)
#define IDC_STRETCHSCL_OFFSET			(0x28)

#define IDC_ENR_REG_ENABLE			(0x01)
#define IDC_ENR_REG_DISABLE			(0x00)

#define IDC_CTRL_HSMODE				(0x10)
#define IDC_CTRL_STOP				(0x08)
#define IDC_CTRL_START				(0x04)
#define IDC_CTRL_IF				(0x02)
#define IDC_CTRL_ACK				(0x01)
#define IDC_CTRL_CLS				(0x00)

#define IDC_STS_FIFO_EMP			(0x04)
#define IDC_STS_FIFO_FUL			(0x02)

#define IDC_FIFO_BUF_SIZE			(63)

#define IDC_FMCTRL_HSMODE			(0x10)
#define IDC_FMCTRL_STOP				(0x08)
#define IDC_FMCTRL_START			(0x04)
#define IDC_FMCTRL_IF				(0x02)

#endif /*__AMBA_I2C_H__*/
