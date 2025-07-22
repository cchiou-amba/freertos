/**
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
 */
#include "FreeRTOS.h"
#include "task.h"
#include <ambhw/gpio.h>

static int gpio_valid(int gpio)
{
	return gpio < GPIO_MAX_LINE ? 1 : 0;
}

static uint32_t gpio_to_bit(int gpio)
{
	return 1U << (gpio % GPIO_LINE_OF_BANK);
}

uint32_t gpio_max_line(void)
{
	return GPIO_MAX_LINE;
}

int gpio_config_sw_out(int gpio)
{
	if (!gpio_valid(gpio))
		return -1;

	return 0;
}

int gpio_config_sw_in(int gpio)
{
	if (!gpio_valid(gpio))
		return -1;

	return 0;
}

int gpio_config_hw(int gpio)
{
	if (!gpio_valid(gpio))
		return -1;

	return 0;
}

int gpio_set(int gpio)
{
	if (!gpio_valid(gpio))
		return -1;

	setbitsl(GPIO_DATA_REG(gpio), gpio_to_bit(gpio));

	return 0;
}

int gpio_get(int gpio)
{
	uint32_t value;

	if (!gpio_valid(gpio))
		return -1;

	value = readl(GPIO_DATA_REG(gpio));

	return value & gpio_to_bit(gpio) ? 1 : 0;
}

int gpio_clr(int gpio)
{
	if (!gpio_valid(gpio))
		return -1;

	clrbitsl(GPIO_DATA_REG(gpio), gpio_to_bit(gpio));

	return 0;
}

int gpio_s_dir(int gpio, int dir_out)
{
	if (!gpio_valid(gpio))
		return -1;

	clrbitsl(GPIO_AFSEL_REG(gpio), gpio_to_bit(gpio));
	setbitsl(GPIO_MASK_REG(gpio), gpio_to_bit(gpio));

	if (dir_out) {
		setbitsl(GPIO_DIR_REG(gpio), gpio_to_bit(gpio));
	} else {
		clrbitsl(GPIO_DIR_REG(gpio), gpio_to_bit(gpio));
	}

	writel(GPIO_ENABLE_REG(gpio), 0x1);

	return 0;
}

int gpio_g_dir(int gpio)
{
	uint32_t value;

	if (!gpio_valid(gpio))
		return -1;

	value = readl(GPIO_DIR_REG(gpio));

	return value & gpio_to_bit(gpio) ? 1 : 0;
}
