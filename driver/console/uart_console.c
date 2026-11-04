/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include <plat.h>
#include <uart.h>
#include "FreeRTOS.h"
#include "task.h"

#define UART_RX_RING_SIZE 256U
#define UART_RX_RING_MASK (UART_RX_RING_SIZE - 1U)

static volatile uint8_t s_rx_buf[UART_RX_RING_SIZE];
static volatile uint16_t s_rx_head = 0;
static volatile uint16_t s_rx_tail = 0;
static volatile TaskHandle_t s_waiting_task = NULL;

static inline void uart_rx_drain_fifo(void)
{
    while (readb(UART_CONSOLE_REG(UART_LS_OFFSET)) & UART_LS_DR) {
        uint8_t c = (uint8_t)readb(UART_CONSOLE_REG(UART_RB_OFFSET));
        uint16_t next_head = (s_rx_head + 1U) & UART_RX_RING_MASK;
        if (next_head != s_rx_tail) {
            s_rx_buf[s_rx_head] = c;
            s_rx_head = next_head;
        }
    }
}

static void uart3_rx_isr(void *data)
{
    (void)data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uart_rx_drain_fifo();

    if (s_waiting_task != NULL) {
        vTaskNotifyGiveFromISR(s_waiting_task, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void console_putchar(char c)
{
    if (c == '\n') {
        while (!(readb(UART_CONSOLE_REG(UART_LS_OFFSET)) & UART_LS_TEMT));
        writeb(UART_CONSOLE_REG(UART_TH_OFFSET), '\r');
    }

    while (!(readb(UART_CONSOLE_REG(UART_LS_OFFSET)) & UART_LS_TEMT));
    writeb(UART_CONSOLE_REG(UART_TH_OFFSET), c);
}

int console_getc_timeout(uint32_t timeout_ms)
{
    /* Always drain any bytes sitting in the hardware FIFO first */
    uart_rx_drain_fifo();

    if (s_rx_head != s_rx_tail) {
        uint8_t c = s_rx_buf[s_rx_tail];
        s_rx_tail = (s_rx_tail + 1U) & UART_RX_RING_MASK;
        return (int)c;
    }

    if (timeout_ms == 0U) {
        return -1;
    }

    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        /* Busy-wait polling loop when scheduler is inactive */
        for (uint32_t i = 0; i < timeout_ms * 1000U; i++) {
            uart_rx_drain_fifo();
            if (s_rx_head != s_rx_tail) {
                uint8_t c = s_rx_buf[s_rx_tail];
                s_rx_tail = (s_rx_tail + 1U) & UART_RX_RING_MASK;
                return (int)c;
            }
        }
        return -1;
    }

    s_waiting_task = xTaskGetCurrentTaskHandle();

    TickType_t start_tick = xTaskGetTickCount();
    TickType_t wait_ticks = pdMS_TO_TICKS(timeout_ms);
    if (wait_ticks == 0U && timeout_ms > 0U) {
        wait_ticks = 1U;
    }

    while (1) {
        TickType_t elapsed = xTaskGetTickCount() - start_tick;
        if (elapsed >= wait_ticks) {
            break;
        }

        TickType_t remaining = wait_ticks - elapsed;
        TickType_t slice = pdMS_TO_TICKS(2);
        if (slice == 0U) {
            slice = 1U;
        }
        if (slice > remaining) {
            slice = remaining;
        }

        ulTaskNotifyTake(pdTRUE, slice);

        /* Drain hardware FIFO on every wakeup (coalesced or unrouted crossbar IRQ fallback) */
        uart_rx_drain_fifo();

        if (s_rx_head != s_rx_tail) {
            uint8_t c = s_rx_buf[s_rx_tail];
            s_rx_tail = (s_rx_tail + 1U) & UART_RX_RING_MASK;
            return (int)c;
        }
    }

    return -1;
}

int console_getchar(void)
{
    return console_getc_timeout(0);
}

void console_init(void)
{
#if defined(ARCH_R52)
    /* 1. Un-gate UART3 clock */
    writel(CG_UART3_REG, 1U);

    /* 2. Check and preserve existing prboot baud divisor if already configured */
    writeb(UART_CONSOLE_REG(UART_LC_OFFSET), UART_LC_DLAB);
    uint8_t dll = readb(UART_CONSOLE_REG(UART_DLL_OFFSET));
    uint8_t dlh = readb(UART_CONSOLE_REG(UART_DLH_OFFSET));

    if (dll == 0U && dlh == 0U) {
        /* 24 MHz refclk / (16 * 115200) = 13 (0x000D) */
        writeb(UART_CONSOLE_REG(UART_DLL_OFFSET), 13U);
        writeb(UART_CONSOLE_REG(UART_DLH_OFFSET), 0U);
    }

    /* 8 bits, 1 stop bit, no parity (8N1) and clear DLAB */
    writeb(UART_CONSOLE_REG(UART_LC_OFFSET), UART_LC_8N1);

    /* Enable FIFO and reset RX & TX FIFOs */
    writeb(UART_CONSOLE_REG(UART_FC_OFFSET),
           UART_FC_FIFO_EN | UART_FC_RX_RESET | UART_FC_TX_RESET);

    /* Reset ring buffer state */
    s_rx_head = 0;
    s_rx_tail = 0;
    s_waiting_task = NULL;

    /* Drain any stale bytes from RX FIFO */
    while (readb(UART_CONSOLE_REG(UART_LS_OFFSET)) & UART_LS_DR) {
        (void)readb(UART_CONSOLE_REG(UART_RB_OFFSET));
    }

    /* 3. Register UART3 interrupt handler on INTID 148 and enable RX IRQ */
    request_irq(UART3_IRQ, IRQ_LEVEL_HIGH, uart3_rx_isr, NULL, "uart3_rx");
    writeb(UART_CONSOLE_REG(UART_IE_OFFSET), 0x01U);
#endif
}
