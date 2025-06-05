/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <plat.h>

#define configASSERT_DEFINED                             1
extern void vAssertCalled(void);
#define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled()

/* debug use only */
#define configQUEUE_REGISTRY_SIZE                        (0)

#define configMAX_PRIORITIES                             ( 16 )
#define configUSE_PREEMPTION                             1
#define configUSE_TIME_SLICING                           1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION          1
#define configMAX_CO_ROUTINE_PRIORITIES                  ( 2 )

#define configUSE_IDLE_HOOK                              1
#define configUSE_TICK_HOOK                              1
#define configUSE_MALLOC_FAILED_HOOK                     1
#define configUSE_DAEMON_TASK_STARTUP_HOOK               0

#define configUSE_TRACE_FACILITY                         1
#define configUSE_16_BIT_TICKS                           0	/* 0: also event-bits 24 usable each group */
#define configIDLE_SHOULD_YIELD                          1
#define configUSE_CO_ROUTINES                            0

/* configCPU_CLOCK_HZ is baseon CPU-clk-axi0;  bsp need setup the really clock */
#define configCPU_CLOCK_HZ                               ( current_cpufreq())
#define configTICK_RATE_HZ                               ( 1000 )

#define configSUPPORT_DYNAMIC_ALLOCATION                 0
#define configSUPPORT_STATIC_ALLOCATION                  1
#define configMINIMAL_STACK_SIZE                         (0x80)
#define configTOTAL_HEAP_SIZE                            ( ( size_t ) ( 900 ) )
#define configMAX_TASK_NAME_LEN                          ( 16 )	/* could adjust base on how many timer-related APIs used */

#define configUSE_TASK_NOTIFICATIONS			1

#define configNUM_TX_DESCRIPTORS                        15
#define configSTREAM_BUFFER_TRIGGER_LEVEL_TEST_MARGIN    2

#define configUSE_COUNTING_SEMAPHORES             1
#define configUSE_MUTEXES                         1
#define configUSE_RECURSIVE_MUTEXES               1

#define configUSE_TIMERS	                  1
#define configTIMER_TASK_STACK_DEPTH              ( configMINIMAL_STACK_SIZE * 2)
#define configTIMER_QUEUE_LENGTH                  16
#define configTIMER_TASK_PRIORITY                 ( configMAX_PRIORITIES - 1 ) /* highest priority */

/* Set the following definitions to 1 to include or zero to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                  1
#define INCLUDE_uxTaskPriorityGet                 1
#define INCLUDE_vTaskDelete                       1
#define INCLUDE_vTaskCleanUpResources             0
#define INCLUDE_vTaskSuspend                      1
#define INCLUDE_vTaskDelayUntil                   1
#define INCLUDE_vTaskDelay                        1
#define INCLUDE_uxTaskGetStackHighWaterMark       1
#define INCLUDE_uxTaskGetStackHighWaterMark2      1
#define INCLUDE_xTaskGetSchedulerState            1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle    1
#define INCLUDE_xTaskGetIdleTaskHandle            1
#define INCLUDE_xTaskGetHandle                    1
#define INCLUDE_eTaskGetState                     1
#define INCLUDE_xSemaphoreGetMutexHolder          1
#define INCLUDE_xTimerPendFunctionCall            1
#define INCLUDE_xTaskAbortDelay                   0

#define configKERNEL_INTERRUPT_PRIORITY         255
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * equivalent to 0xa0, or priority 5. 1011_1111 => MSB=101, 3bits */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191

#endif /* FREERTOS_CONFIG_H */
