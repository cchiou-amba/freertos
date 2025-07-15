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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* FreeRTOS run at EL1 On R52 AND use the IRQ-GIC-group1
 * for scheduler and other kernel serevice
 */

/* 16 software priority in tasks, bignumber high priority task */
#define configMAX_PRIORITIES			(16)
#define configMINIMAL_STACK_SIZE		(128)
#define configUSE_PREEMPTION			1
#define configTOTAL_HEAP_SIZE			(0x1000)
#define configUSE_IDLE_HOOK			0
#define configUSE_TICK_HOOK			0
#define configUSE_16_BIT_TICKS			0
#define configUSE_TASK_NOTIFICATIONS		1
#define configUSE_DAEMON_TASK_STARTUP_HOOK	0
#define configSUPPORT_STATIC_ALLOCATION		1
#define configSUPPORT_DYNAMIC_ALLOCATION	0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION	1

/* Other RTOS feature flags (mutexes, semaphores, etc.) */
#define configUSE_MUTEXES			1
#define configUSE_RECURSIVE_MUTEXES		1
#define configUSE_SEMAPHORES			1
#define configUSE_COUNTING_SEMAPHORES		1
#define configMAX_TASK_NAME_LEN			(16)

#if 0
#define configUSE_TICK_HOOK			0
#define configNUM_TX_DESCRIPTORS		15
#define configSTREAM_BUFFER_TRIGGER_LEVEL_TEST_MARGIN	2
#define configUSE_TIMERS			1
#define configTIMER_TASK_STACK_DEPTH		(configMINIMAL_STACK_SIZE * 2)
#define configTIMER_QUEUE_LENGTH		16
#define configTIMER_TASK_PRIORITY		( configMAX_PRIORITIES - 1 ) /* highest priority */

#endif

/* Set the following definitions to 1 to include or zero to exclude the API function. */
#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete			1
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay			1
#define INCLUDE_uxTaskGetStackHighWaterMark	1
#define INCLUDE_uxTaskGetStackHighWaterMark2	1
#define INCLUDE_xTaskGetSchedulerState		1
#if (configUSE_TIMERS == 1)
#define INCLUDE_xTimerGetTimerDaemonTaskHandle	1
#define INCLUDE_xTimerPendFunctionCall		1
#endif
#define INCLUDE_xTaskGetIdleTaskHandle		1
#define INCLUDE_xTaskGetHandle			1
#define INCLUDE_eTaskGetState			1
#define INCLUDE_xSemaphoreGetMutexHolder	1
#define INCLUDE_xTaskAbortDelay			0

/* on float-point saved context */
#define configUSE_TASK_FPU_SUPPORT		(0)

#define configCHECK_FOR_STACK_OVERFLOW		2


/* as portPRIORITY_SHIFT = 3, MSB-5bits to define gic interrupt priority
 * so we can get 32 interrupt pri level and the GIC_CPU use the 0xa0: (20 << 3)
 * configMAX_API_CALL_INTERRUPT_PRIORITY can use 0x90: (18 << 3)
 * must compatible with the GIC settings, ISR call xxxFromIsr priority >= cpu interface
*/
#define configUNIQUE_INTERRUPT_PRIORITIES	32
#define configKERNEL_INTERRUPT_PRIORITY		(255)	/* alway set 0xFF */
#define configMAX_API_CALL_INTERRUPT_PRIORITY	18	/* gic prirotiy */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY	configMAX_API_CALL_INTERRUPT_PRIORITY

#ifndef __ASM__
#include <common.h>
#define configASSERT(x)	 do { if (!(x)) { printf("Error: %s(%d): \n", __func__, __LINE__); } } while (0)
#endif

#endif /* FREERTOS_CONFIG_H */

