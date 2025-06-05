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

#ifndef __AMBA_GPIO_H__
#define __AMBA_GPIO_H__

#include <plat.h>

#define PINID_TO_BANK(p)		((p) >> 5)
#define PINID_TO_OFFSET(p)		((p) & 0x1f)
#define GPIO(x)				(x)
#define IOMUX_REG(x)			(IOMUX_BASE + (x))
#define IOMUX_CTRL_SET_OFFSET		0xf0
#define IOMUX_REG_OFFSET(bank, n)	(((bank) * 0xc) + ((n) * 4))

#define GPIO_HIGH			1
#define GPIO_LOW			0

#define GPIO_FUNC_SW_INPUT		0
#define GPIO_FUNC_SW_OUTPUT		1
#define GPIO_FUNC_HW			2

#define PIN_MUX_ALT0			(0)
#define PIN_MUX_ALT1			(1)
#define PIN_MUX_ALT2			(2)
#define PIN_MUX_ALT3			(3)
#define PIN_MUX_ALT4			(4)
#define PIN_MUX_ALT5			(5)
#define PIN_MUX_ALT6			(6)
#define PIN_MUX_ALT7			(7)

#define GPIO_BANK_SIZE			32
#define AMBGPIO_SIZE			(GPIO_INSTANCES * GPIO_BANK_SIZE)

#define GPIO_DATA_OFFSET		0x00
#define GPIO_DIR_OFFSET			0x04
#define GPIO_IS_OFFSET			0x08
#define GPIO_IBE_OFFSET			0x0c
#define GPIO_IEV_OFFSET			0x10
#define GPIO_IE_OFFSET			0x14
#define GPIO_AFSEL_OFFSET		0x18
#define GPIO_RIS_OFFSET			0x1c
#define GPIO_MIS_OFFSET			0x20
#define GPIO_IC_OFFSET			0x24
#define GPIO_MASK_OFFSET		0x28
#define GPIO_ENABLE_OFFSET		0x2c

#define CMVI_GPIO_START			320     /* MAPS TO CMVI, From GPIO_BASE10 */

extern int gpio_clr(int gpio);
extern int gpio_set(int gpio);
extern int gpio_get(int gpio);
extern int gpio_config_sw_out(int gpio);
extern int gpio_config_sw_in(int gpio);
extern int gpio_config_hw(int gpio, int alt_func);
extern int gpio_pin_mode(int gpio, int alt_func);
extern int gpio_pin_interrupt(int gpio, int irqmode);
extern int gpio_pin_interrupt_clear(int gpio);

#endif
