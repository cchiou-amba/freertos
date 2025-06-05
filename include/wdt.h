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
