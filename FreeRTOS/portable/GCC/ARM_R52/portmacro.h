/*
 * FreeRTOS Kernel V10.5.1
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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

#ifndef PORTMACRO_H
    #define PORTMACRO_H

    #ifdef __cplusplus
        extern "C" {
    #endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the given hardware
 * and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
    #define portCHAR          char
    #define portFLOAT         float
    #define portDOUBLE        double
    #define portLONG          long
    #define portSHORT         short
    #define portSTACK_TYPE    uint32_t
    #define portBASE_TYPE     long

    typedef portSTACK_TYPE   StackType_t;
    typedef long             BaseType_t;
    typedef unsigned long    UBaseType_t;

    typedef uint32_t         TickType_t;
    #define portMAX_DELAY    ( TickType_t ) 0xffffffffUL

/*-----------------------------------------------------------*/
/* Hardware specifics. */
#define portSTACK_GROWTH	( -1 )
#define portTICK_PERIOD_MS	( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT	8

/*-----------------------------------------------------------*/

/* Task utilities. */

/* Called at the end of an ISR that can cause a context switch. */
    #define portEND_SWITCHING_ISR( xSwitchRequired ) \
    {                                                \
        extern uint32_t ulPortYieldRequired;         \
                                                     \
        if( xSwitchRequired != pdFALSE )             \
        {                                            \
            ulPortYieldRequired = pdTRUE;            \
        }                                            \
    }

    #define portYIELD_FROM_ISR( x )    portEND_SWITCHING_ISR( x )
    #define portYIELD()                __asm volatile ( "SWI 0" ::: "memory" );


/*-----------------------------------------------------------
* Critical section control
*----------------------------------------------------------*/

    extern void vPortEnterCritical( void );
    extern void vPortExitCritical( void );
    extern uint32_t ulPortSetInterruptMask( void );
    extern void vPortClearInterruptMask( uint32_t ulNewMaskValue );
    extern void vPortInstallFreeRTOSVectorTable( void );

/* These macros do not globally disable/enable interrupts.  They do mask off
 * interrupts that have a priority below configMAX_API_CALL_INTERRUPT_PRIORITY. */
    #define portENTER_CRITICAL()                      vPortEnterCritical();
    #define portEXIT_CRITICAL()                       vPortExitCritical();
    #define portDISABLE_INTERRUPTS()                  ulPortSetInterruptMask()
    #define portENABLE_INTERRUPTS()                   vPortClearInterruptMask( 0 )
    #define portSET_INTERRUPT_MASK_FROM_ISR()         ulPortSetInterruptMask()
    #define portCLEAR_INTERRUPT_MASK_FROM_ISR( x )    vPortClearInterruptMask( x )

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
 * not required for this port but included in case common demo code that uses these
 * macros is used. */
    #define portTASK_FUNCTION_PROTO( vFunction, pvParameters )    void vFunction( void * pvParameters )
    #define portTASK_FUNCTION( vFunction, pvParameters )          void vFunction( void * pvParameters )

/* Prototype of the FreeRTOS tick handler.  This must be installed as the
 * handler for whichever peripheral is used to generate the RTOS tick. */
    void FreeRTOS_Tick_Handler( void );

/* If configUSE_TASK_FPU_SUPPORT is set to 1 (or left undefined) then tasks are
created without an FPU context and must call vPortTaskUsesFPU() to give
themselves an FPU context before using any FPU instructions. If
configUSE_TASK_FPU_SUPPORT is set to 2 then all tasks will have an FPU context
by default. */
#if( configUSE_TASK_FPU_SUPPORT != 2 )
    void vPortTaskUsesFPU( void );
#else
    /* Each task has an FPU context already, so define this function away to
    nothing to prevent it being called accidentally. */
    #define vPortTaskUsesFPU()
#endif /* configUSE_TASK_FPU_SUPPORT */
    #define portTASK_USES_FLOATING_POINT()    vPortTaskUsesFPU()

    #define portLOWEST_INTERRUPT_PRIORITY           ( ( ( uint32_t ) configUNIQUE_INTERRUPT_PRIORITIES ) - 1UL )
    #define portLOWEST_USABLE_INTERRUPT_PRIORITY    ( portLOWEST_INTERRUPT_PRIORITY - 1UL )

/* Architecture specific optimisations. */
    #ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
        #define configUSE_PORT_OPTIMISED_TASK_SELECTION    1
    #endif

    #if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

/* Store/clear the ready priorities in a bit map. */
        #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )    ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
        #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )     ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

/*-----------------------------------------------------------*/

        #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )    uxTopPriority = ( 31 - __builtin_clz( uxReadyPriorities ) )

    #endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

    #ifdef configASSERT
        void vPortValidateInterruptPriority( void );
        #define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()    vPortValidateInterruptPriority()
    #endif /* configASSERT */

    #define portNOP()                                         __asm volatile ( "NOP" )


    #ifdef __cplusplus
        } /* extern C */
    #endif


/* The number of bits to shift for an interrupt priority is dependent on the
 * number of bits implemented by the interrupt controller. */
#if defined(ARCH_R52)
    #if configUNIQUE_INTERRUPT_PRIORITIES == 2
        #define portPRIORITY_SHIFT              7
        #define portMAX_BINARY_POINT_VALUE      7
    #elif configUNIQUE_INTERRUPT_PRIORITIES == 4
        #define portPRIORITY_SHIFT              6
        #define portMAX_BINARY_POINT_VALUE      6
    #elif configUNIQUE_INTERRUPT_PRIORITIES == 8
        #define portPRIORITY_SHIFT              5
        #define portMAX_BINARY_POINT_VALUE      5
    #elif configUNIQUE_INTERRUPT_PRIORITIES == 16
        #define portPRIORITY_SHIFT              4
        #define portMAX_BINARY_POINT_VALUE      4
    #elif configUNIQUE_INTERRUPT_PRIORITIES == 32
        #define portPRIORITY_SHIFT              3
        #define portMAX_BINARY_POINT_VALUE      3
    #else
        #error Invalid configUNIQUE_INTERRUPT_PRIORITIES setting [2,4,8,16,32 on R52-GIC]
    #endif
#else
#error "not R52 GIC, pls check your TRM"
#endif

    #define portMEMORY_BARRIER()    __asm volatile ( "" ::: "memory" )
#endif /* PORTMACRO_H */
