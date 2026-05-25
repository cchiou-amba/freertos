/**
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
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
