 /*******************************************************************************
 *
 * Copyright (c) 2025 Ambarella International LP
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
