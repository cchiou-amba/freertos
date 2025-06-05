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
#include <common.h>
#include <gpio.h>
#include <hwio.h>

static uint32_t gpio_enable_mask;

#if defined(CMVI_SUPPORT_ONCHIP)
static uint32_t cmvi_gpio_enable_mask = 0;
#endif

unsigned long gpio_bank_to_base(uint32_t bank)
{
	unsigned long regbase;

	switch(bank) {
		case 0:
			regbase = GPIO0_BASE;
			break;

		case 1:
			regbase = GPIO1_BASE;
			break;

		case 2:
			regbase = GPIO2_BASE;
			break;

		case 3:
			regbase = GPIO3_BASE;
			break;

		case 4:
			regbase = GPIO4_BASE;
			break;

		case 5:
			regbase = GPIO5_BASE;
			break;

		case 6:
			regbase = GPIO6_BASE;
			break;

		case 7:
			regbase = GPIO7_BASE;
			break;

		case 8:
			regbase = GPIO8_BASE;
			break;

		case 9:
			regbase = GPIO9_BASE;
			break;

		case 10:
			regbase = GPIO10_BASE;
			break;

		default:
			return -1;
	}

	return regbase;
}

uint32_t gpio_base_to_bank(unsigned long base)
{
	switch(base) {
		case GPIO0_BASE:
			return 0;

		case GPIO1_BASE:
			return 1;

		case GPIO2_BASE:
			return 2;

		case GPIO3_BASE:
			return 3;

		case GPIO4_BASE:
			return 4;

		case GPIO5_BASE:
			return 5;

		case GPIO6_BASE:
			return 6;

		case GPIO7_BASE:
			return 7;

		case GPIO8_BASE:
			return 8;

		case GPIO9_BASE:
			return 9;

		case GPIO10_BASE:
			return 10;

		default:
			return -1;
	}
}

static void cmvi_pinmux_cfg(uint32_t pin, uint32_t mode, uint32_t altfun)
{
#if defined(CMVI_SUPPORT_ONCHIP)
	uint32_t i = 0, data = 0, bank = 0;
	uint32_t pin_off = pin, reg_off = IOMUX0_OFFSET;

	bank = PINID_TO_BANK(pin);

	for (i = 0; i < 3; i++) {
		data = readl(CMVI_IOMUX_REG(reg_off + i * 4));
		data &= (~(0x1 << pin_off));
		data |= (((altfun >> i) & 0x1) << pin_off);
		writel(CMVI_IOMUX_REG(reg_off + i * 4), data);
	}

	writel(CMVI_IOMUX_REG(0xF0), 1);
	writel(CMVI_IOMUX_REG(0xF0), 0);

	if(!(cmvi_gpio_enable_mask & (1 << bank))) {
		writel(CMVI_GPIO_REG(GPIO_ENABLE_OFFSET), 0xffffffff);
		cmvi_gpio_enable_mask |= 1 << bank;
	}
#endif
}

static int gpio_cfg_mode(int gpio, int mode, int alt_func)
{
	uint32_t bank, offset, data, i;
	unsigned long regbase;
	bank = PINID_TO_BANK(gpio);
	offset = PINID_TO_OFFSET(gpio);
	regbase = gpio_bank_to_base(bank);

	if(mode != GPIO_FUNC_HW) {
		if(mode == GPIO_FUNC_SW_INPUT) {
			clrbitsl(regbase + GPIO_DIR_OFFSET, 0x1 << offset);
		} else {
			setbitsl(regbase + GPIO_DIR_OFFSET, 0x1 << offset);
		}

		clrbitsl(regbase + GPIO_AFSEL_OFFSET, 0x1 << offset);
		setbitsl(regbase + GPIO_MASK_OFFSET, 0x1 << offset);
	}

	if (gpio < CMVI_GPIO_START) {
		for(i = 0; i < 3; i++) {
			data = readl(IOMUX_REG(IOMUX_REG_OFFSET(bank, i)));
			data &= (~(0x1 << offset));
			data |= (((alt_func >> i) & 0x1) << offset);
			writel(IOMUX_REG(IOMUX_REG_OFFSET(bank, i)), data);
		}

		writel(IOMUX_REG(IOMUX_CTRL_SET_OFFSET), 0x1);
		writel(IOMUX_REG(IOMUX_CTRL_SET_OFFSET), 0x0);

		if(!(gpio_enable_mask & (1 << bank))) {
			writel(regbase + GPIO_ENABLE_OFFSET, 0xffffffff);
			gpio_enable_mask |= 1 << bank;
		}
	} else {
		gpio -= CMVI_GPIO_START;
		cmvi_pinmux_cfg(gpio, mode, alt_func);
	}

	return 0;
}

/* ==========================================================================*/
int gpio_pin_mode(int gpio, int alt_func)
{
	int mode;

	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	mode = alt_func == 0 ? GPIO_FUNC_SW_INPUT : GPIO_FUNC_HW;
	return gpio_cfg_mode(gpio, mode, alt_func);
}

int gpio_config_hw(int gpio, int alt_func)
{
	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	return gpio_cfg_mode(gpio, GPIO_FUNC_HW, alt_func);
}

int gpio_config_sw_in(int gpio)
{
	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	return gpio_cfg_mode(gpio, GPIO_FUNC_SW_INPUT, 0);
}

int gpio_config_sw_out(int gpio)
{
	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	return gpio_cfg_mode(gpio, GPIO_FUNC_SW_OUTPUT, 0);
}

int gpio_set(int gpio)
{
	unsigned long regbase;
	uint32_t bank, offset;

	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	bank = PINID_TO_BANK(gpio);
	offset = PINID_TO_OFFSET(gpio);
	regbase = gpio_bank_to_base(bank);
	setbitsl(regbase + GPIO_DATA_OFFSET, 0x1 << offset);
	return 0;
}

int gpio_clr(int gpio)
{
	unsigned long regbase;
	uint32_t bank, offset;

	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	bank = PINID_TO_BANK(gpio);
	offset = PINID_TO_OFFSET(gpio);
	regbase = gpio_bank_to_base(bank);
	clrbitsl(regbase + GPIO_DATA_OFFSET, 0x1 << offset);
	return 0;
}

int gpio_get(int gpio)
{
	unsigned long regbase;
	uint32_t bank, offset, data;

	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	bank = PINID_TO_BANK(gpio);
	offset = PINID_TO_OFFSET(gpio);
	regbase = gpio_bank_to_base(bank);
	data = readl(regbase + GPIO_DATA_OFFSET);
	clrbitsl(regbase + GPIO_DATA_OFFSET, 0x1 << offset);
	return !!(data & (0x1 << offset));
}

int gpio_pin_interrupt(int gpio, int irqmode)
{
	unsigned long regbase;
	uint32_t bank, offset, data;

	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	bank = PINID_TO_BANK(gpio);
	offset = PINID_TO_OFFSET(gpio);
	regbase = gpio_bank_to_base(bank);

	data = readl(regbase + GPIO_IS_OFFSET);
	if (irqmode == IRQ_LEVEL_HIGH || irqmode == IRQ_LEVEL_LOW)
		data |= (1 << offset);
	else
		data &= ~(1 << offset);
	writel(regbase + GPIO_IS_OFFSET, data);

	data = readl(regbase + GPIO_IBE_OFFSET);
	if (irqmode == IRQ_BOTH_EDGES)
		data |= (1 << offset);
	else
		data &= ~(1 << offset);
	writel(regbase + GPIO_IBE_OFFSET, data);

	data = readl(regbase + GPIO_IEV_OFFSET);
	if (irqmode == IRQ_FALLING_EDGE || irqmode == IRQ_LEVEL_LOW)
		data &= ~(1 << offset);
	if (irqmode == IRQ_RISING_EDGE || irqmode == IRQ_LEVEL_HIGH)
		data |= (1 << offset);
	writel(regbase + GPIO_IEV_OFFSET, data);

	data = 1 << offset;
	writel(regbase + GPIO_IC_OFFSET, data);

	data = readl(regbase + GPIO_IE_OFFSET);
	data |= 1 << offset;
	writel(regbase + GPIO_IE_OFFSET, data);

	/* for none-cmvi-case;
	 * cmvi-gpio-always enabled after console init
	*/
	if (gpio < CMVI_GPIO_START) {
		if(!(gpio_enable_mask & (1 << bank))) {
			writel(regbase + GPIO_ENABLE_OFFSET, 0xffffffff);
			gpio_enable_mask |= 1 << bank;
		}
	}

	return 0;
}

int gpio_pin_interrupt_clear(int gpio)
{
	unsigned long regbase;
	uint32_t bank, offset, data, rawis;

	if(gpio >= AMBGPIO_SIZE) {
		printf("Invalid gpio: %d\n", gpio);
		return -1;
	}

	bank = PINID_TO_BANK(gpio);
	offset = PINID_TO_OFFSET(gpio);
	regbase = gpio_bank_to_base(bank);

	/* Interrupt clear the edge-detection */
	data = 1 << offset;
	rawis = readl(regbase + GPIO_MIS_OFFSET);
	if (data & rawis)
		writel(regbase + GPIO_IC_OFFSET, data);

	return 0;
}
