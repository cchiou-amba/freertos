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

#define TASK_APP_STACK_SIZE	   (128)
#define TASK_APPA_PRI		   (tskIDLE_PRIORITY + 2)
#define TASK_APPB_PRI		   (tskIDLE_PRIORITY + 3)

static uint32_t dataa, datab;
static StaticTask_t tasktcba, tasktcbb;
static StackType_t task_appa_stack[TASK_APP_STACK_SIZE] __attribute__((aligned(8)));
static StackType_t task_appb_stack[TASK_APP_STACK_SIZE] __attribute__((aligned(8)));

static void app_task_entry(void *arg)
{
	while (1) {
#if 0	/* TODO */
		printf("This is app_task_%d \n", *(uint32_t *)arg);
#endif
		vTaskDelay(1000);
	}
}

void app_task_init(void)
{
	TaskHandle_t taskhandler;

	printf("app_task_init\n");

	dataa = 0;
	taskhandler = xTaskCreateStatic(
		app_task_entry,
		"appa",
		TASK_APP_STACK_SIZE,
		&dataa,
		TASK_APPA_PRI,
		task_appa_stack,
		&tasktcba);

	configASSERT(taskhandler != NULL);

	datab = 1;
	taskhandler = xTaskCreateStatic(
		app_task_entry,
		"appB",
		TASK_APP_STACK_SIZE,
		&datab,
		TASK_APPB_PRI,
		task_appb_stack,
		&tasktcbb);

	configASSERT(taskhandler != NULL);

}

void board_hw_init(void)
{

}
