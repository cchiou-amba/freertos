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
#ifndef __AMBA_IDC_H__
#define __AMBA_IDC_H__

#include <plat.h>

#define IDC_ENR_OFFSET			0x00
#define IDC_CTRL_OFFSET			0x04
#define IDC_DATA_OFFSET			0x08
#define IDC_STS_OFFSET			0x0c
#define IDC_PSLL_OFFSET			0x10
#define IDC_PSLH_OFFSET			0x14
#define IDC_FMCTRL_OFFSET		0x18
#define IDC_FMDATA_OFFSET		0x1c
#define IDC_PSHS_OFFSET			0x20
#define IDC_DUTYCYCLE_OFFSET		0x24
#define IDC_STRETCHSCL_OFFSET		0x28

#define IDC_ENR_REG_ENABLE		(0x01)
#define IDC_ENR_REG_DISABLE		(0x00)

#define IDC_CTRL_HSMODE			(0x10)
#define IDC_CTRL_STOP			(0x08)
#define IDC_CTRL_START			(0x04)
#define IDC_CTRL_IF			(0x02)
#define IDC_CTRL_ACK			(0x01)
#define IDC_CTRL_CLS			(0x00)

#define IDC_STS_FIFO_EMP		(0x04)
#define IDC_STS_FIFO_FUL		(0x02)

#define IDC_FIFO_BUF_SIZE		(63)

#define IDC_FMCTRL_HSMODE		(0x10)
#define IDC_FMCTRL_STOP			(0x08)
#define IDC_FMCTRL_START		(0x04)
#define IDC_FMCTRL_IF			(0x02)

#define I2C_W				(0)
#define I2C_R				(1)
#define I2C_ACK				(0)
#define I2C_NACK			(1)

#define I2C0				(0)
#define I2C1				(1)
#define I2C2				(2)
#define I2C3				(3)
#define I2C4				(4)
#define I2C5				(5)
#define I2C6				(6)
#define I2C7				(7)
#define I2C8				(8)
#define I2CS				(15)
#define I2C_CMVI			(31)

extern void idc_stop(uint8_t idc_id);
extern uint8_t idc_readb(uint8_t idc_id, uint32_t nack);
extern uint32_t idc_get_addr(uint8_t idc_id, uint32_t reg_offset);
extern uint32_t idc_readl(uint8_t idc_id, uint32_t reg_offset);
extern void idc_writel(uint8_t idc_id, uint32_t reg_offset, uint32_t reg_val);
extern void idc_clk_config(uint8_t idc_id, uint32_t idc_rate);
extern void idc_set_8bit_addr(uint8_t idc_id, uint8_t addr, uint8_t rw);
extern void idc_stop(uint8_t idc_id);
extern uint8_t idc_readb(uint8_t idc_id, uint32_t nack);
extern void idc_writeb(uint8_t idc_id, uint8_t data);

extern void idc_write_8_8(uint8_t idc_id, uint8_t adds, uint8_t sub_adds, uint8_t reg_val);
extern uint8_t idc_read_8_8(uint8_t idc_id, uint8_t adds, uint8_t sub_adds);

#endif
