/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#include <plat.h>
#include <common.h>
#include <task.h>

void __attribute__((weak)) board_hw_init(void) {}
void __attribute__((weak)) shell_task_init(void) {}
void __attribute__((weak)) app_task_init(void) {}
void __attribute__((weak)) app1_task_init(void) {}
void __attribute__((weak)) app2_task_init(void) {}

static void boot_message(void)
{
	printf("\n FreeRTOS Runs on CPU_%s_C%u "
		"cpufreq:%uHz "
		"apbfreq:%uHz "
		"ahbfreq:%uHz\n",
		ARCH_NAME,
		current_cpuid(),
		current_cpufreq(),
		current_apbfreq(),
		current_ahbfreq());
}

static void boot_crashed(void)
{
	printf("FreeRTOS crashed \n");
	__asm__ __volatile__("b	 .");
}

#if defined(CONFIG_BOOT_R52)
#include <uart.h>
#include <ambhw/spi.h>
#include "r52_cli.h"

void r52_hw_init(void)
{
	console_init();
	spi_init(SPI1);
	r52_cli_task_init();
}
#endif

int main(void)
{
#if defined(CONFIG_BOOT_R52)
	r52_hw_init();
#endif
	board_hw_init();
	boot_message();

	/* tasks init */
	shell_task_init();
	app_task_init();
	app1_task_init();
	app2_task_init();

	/* start the tasks */
	vTaskStartScheduler();

	/* Should Not Reach Here */
	boot_crashed();

	return 0;
}
