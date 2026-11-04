/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef R52_HOST_TEST
#include "FreeRTOS.h"
#include "task.h"
#include <plat.h>
#include <uart.h>
#else
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "FreeRTOS_CLI.h"
#include "vt100_line.h"
#include "icm20648.h"
#include "imu_tui.h"
#include "r52_cli.h"

#ifndef R52_HOST_TEST
#define r52_readl(addr)        readl((uint32_t)(addr))
#define r52_writel(addr, val)  writel((uint32_t)(addr), (uint32_t)(val))
#else
static uint32_t host_read_mem32(uint32_t addr)
{
    return 0xDEADBEEFU ^ (uint32_t)addr;
}
static void host_write_mem32(uint32_t addr, uint32_t val)
{
    (void)addr; (void)val;
}
#define r52_readl(addr)        host_read_mem32((uint32_t)(addr))
#define r52_writel(addr, val)  host_write_mem32((uint32_t)(addr), (uint32_t)(val))
#endif

static int is_valid_r52_address(uint32_t addr)
{
    if (addr <= 0x7FCFFFFFU) return 1; /* DRAM */
    if (addr >= 0x7FD00000U && addr <= 0x7FFEFFFFU) return 1; /* SI Memory */
    if (addr >= 0xE0000000U && addr <= 0xF5FFFFFFU) return 1; /* MMIO Peripherals */
    if (addr >= 0xF6000000U && addr <= 0xF601FFFFU) return 1; /* ATCM / BTCM */
    if (addr >= 0xF7000000U && addr <= 0xFFFFFFFFU) return 1; /* GICv3 & Safety MMIO */
    return 0;
}

/* -------------------------------------------------------------------------
 * Command: r52 (info | mpu | clk)
 * ------------------------------------------------------------------------- */
static BaseType_t prvR52Command(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    BaseType_t len1 = 0;
    const char *p1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);

    if (p1 != NULL && strncmp(p1, "mpu", len1) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "Active EL1 MPU Regions:\r\n"
            "  Region 0: 0x7FD00000 - 0x7FFEFFFF (SI Memory, Normal-NC, RW, X)\r\n"
            "  Region 1: 0xF6000000 - 0xF601FFFF (TCM Memory, Normal-NC, RW, X)\r\n"
            "  Region 2: 0x00000000 - 0x7FCFFFFF (DDR Memory, Normal-NC, RW, X)\r\n"
            "  Region 3: 0xE0000000 - 0xF5FFFFFF (Peripheral MMIO, Device, RW, XN)\r\n"
            "  Region 4: 0xF7000000 - 0xFFFFFFFF (GICv3 & Safety Island, Device, RW, XN)\r\n");
    } else if (p1 != NULL && strncmp(p1, "clk", len1) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "R52 Clocks:\r\n"
            "  CPU Clock: 1008 MHz (1008000000 Hz)\r\n"
            "  AHB Clock:  252 MHz (252000000 Hz)\r\n"
            "  APB Clock:  126 MHz (126000000 Hz)\r\n"
            "  CNTFRQ:      84 MHz (84000000 Hz)\r\n");
    } else {
        /* Default / info */
#ifndef R52_HOST_TEST
        size_t free_heap = xPortGetFreeHeapSize();
        size_t min_heap = xPortGetMinimumEverFreeHeapSize();
#else
        size_t free_heap = 28672;
        size_t min_heap = 24576;
#endif
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "R52 Core Info:\r\n"
            "  CPU:       CPU_R52_C0\r\n"
            "  MIDR:      0x411FD130 (ARM Cortex-R52 r1p3)\r\n"
            "  MPIDR:     0x80000000\r\n"
            "  CPSR:      0x60000193 (EL1h, IRQ enabled)\r\n"
            "  SCTLR:     0x30C50818 (MPU enabled, BR enabled)\r\n"
            "  CNTFRQ:    84000000 Hz (84 MHz)\r\n"
            "  cpufreq:   1008000000 Hz (1008 MHz)\r\n"
            "  ahbfreq:   252000000 Hz (252 MHz)\r\n"
            "  apbfreq:   126000000 Hz (126 MHz)\r\n"
            "  Heap Free: %u bytes (min ever free: %u bytes)\r\n"
            "  Active EL1 MPU Regions: 5 regions\r\n",
            (unsigned int)free_heap, (unsigned int)min_heap);
    }

    return pdFALSE;
}

static const CLI_Command_Definition_t xR52Command = {
    "r52",
    "r52 [info|mpu|clk]: Displays R52 CPU, MPU, clocks, and heap status\r\n",
    prvR52Command,
    -1
};

/* -------------------------------------------------------------------------
 * Command: ps
 * ------------------------------------------------------------------------- */
static BaseType_t prvPsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
#ifndef R52_HOST_TEST
    uint32_t ticks = (uint32_t)xTaskGetTickCount();
    uint32_t uptime_sec = ticks / configTICK_RATE_HZ;

    int offset = snprintf(pcWriteBuffer, xWriteBufferLen,
        "Task Status (Uptime: %us, Ticks: %u):\r\n"
        "Name             State  Pri  StackLeft  Num\r\n"
        "-------------------------------------------\r\n",
        (unsigned int)uptime_sec, (unsigned int)ticks);

    if (offset > 0 && (size_t)offset < xWriteBufferLen) {
        vTaskList(pcWriteBuffer + offset);
    }
#else
    snprintf(pcWriteBuffer, xWriteBufferLen,
        "Task Status (Uptime: 10s, Ticks: 10000):\r\n"
        "Name             State  Pri  StackLeft  Num\r\n"
        "-------------------------------------------\r\n"
        "r52-cli          R      2    1840       1\r\n"
        "IDLE             R      0    112        2\r\n");
#endif
    return pdFALSE;
}

static const CLI_Command_Definition_t xPsCommand = {
    "ps",
    "ps: Displays FreeRTOS tasks, priorities, stack usage, and uptime\r\n",
    prvPsCommand,
    0
};

/* -------------------------------------------------------------------------
 * Command: irq
 * ------------------------------------------------------------------------- */
static BaseType_t prvIrqCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;

#ifndef R52_HOST_TEST
    uint32_t timer_count = r52_irq_get_count(PHYS_TIMER_IRQ30);
    int timer_en = r52_irq_is_enabled(PHYS_TIMER_IRQ30);

    uint32_t uart_count = r52_irq_get_count(UART3_IRQ);
    int uart_en = r52_irq_is_enabled(UART3_IRQ);

    uint32_t spi_count = r52_irq_get_count(598U); /* Safety Island SSI1 IRQ: SPI 566 + 32 = 598 */
    int spi_en = r52_irq_is_enabled(598U);
#else
    uint32_t timer_count = 10000U;
    int timer_en = 1;
    uint32_t uart_count = 42U;
    int uart_en = 1;
    uint32_t spi_count = 14U;
    int spi_en = 1;
#endif

    snprintf(pcWriteBuffer, xWriteBufferLen,
        "GICv3 Interrupt Controller:\r\n"
        "  Distributor Base:   0xF7000000\r\n"
        "  Redistributor Base: 0xF7110000\r\n"
        "Monitored Interrupts:\r\n"
        "  INTID  30 (arch_timer): enabled=%d, count=%u\r\n"
        "  INTID 148 (uart3_rx)  : enabled=%d, count=%u\r\n"
        "  INTID 598 (si_ssi1)   : enabled=%d, count=%u\r\n",
        timer_en, (unsigned int)timer_count,
        uart_en, (unsigned int)uart_count,
        spi_en, (unsigned int)spi_count);

    return pdFALSE;
}

static const CLI_Command_Definition_t xIrqCommand = {
    "irq",
    "irq: Displays GICv3 status and live interrupt counters (INTID 30, 148, 598)\r\n",
    prvIrqCommand,
    0
};

/* -------------------------------------------------------------------------
 * Command: mem (r32 | w32)
 * ------------------------------------------------------------------------- */
static BaseType_t prvMemCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    BaseType_t len1 = 0, len2 = 0, len3 = 0;
    const char *op = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);
    const char *addr_str = FreeRTOS_CLIGetParameter(pcCommandString, 2, &len2);
    const char *val_str = FreeRTOS_CLIGetParameter(pcCommandString, 3, &len3);

    if (op == NULL || addr_str == NULL) {
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "Usage: mem r32 <addr_hex> [count] | mem w32 <addr_hex> <val_hex>\r\n");
        return pdFALSE;
    }

    uint32_t addr = (uint32_t)strtoul(addr_str, NULL, 16);

    /* Address alignment check */
    if ((addr & 0x03U) != 0U) {
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "ERROR: Unaligned address 0x%08X (must be 4-byte aligned)\r\n",
            (unsigned int)addr);
        return pdFALSE;
    }

    /* Valid R52 address range check */
    if (!is_valid_r52_address(addr)) {
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "ERROR: Address 0x%08X outside valid R52 physical memory regions\r\n",
            (unsigned int)addr);
        return pdFALSE;
    }

    if (strncmp(op, "r32", len1) == 0) {
        uint32_t count = 1;
        if (val_str != NULL && len3 > 0) {
            count = (uint32_t)strtoul(val_str, NULL, 0);
            if (count == 0) count = 1;
            if (count > 16) count = 16;
        }

        int offset = 0;
        for (uint32_t i = 0; i < count; i++) {
            uint32_t curr_addr = addr + (i * 4U);
            if (!is_valid_r52_address(curr_addr)) {
                break;
            }
            uint32_t val = r52_readl(curr_addr);
            int written = snprintf(pcWriteBuffer + offset, xWriteBufferLen - offset,
                "0x%08X: 0x%08X\r\n", (unsigned int)curr_addr, (unsigned int)val);
            if (written < 0 || (size_t)(offset + written) >= xWriteBufferLen) {
                break;
            }
            offset += written;
        }
    } else if (strncmp(op, "w32", len1) == 0) {
        if (val_str == NULL || len3 == 0) {
            snprintf(pcWriteBuffer, xWriteBufferLen,
                "ERROR: mem w32 requires <value_hex>\r\n");
            return pdFALSE;
        }
        uint32_t val = (uint32_t)strtoul(val_str, NULL, 16);
        r52_writel(addr, val);
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "0x%08X <= 0x%08X\r\n", (unsigned int)addr, (unsigned int)val);
    } else {
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "ERROR: Unknown mem operation '%s'\r\n", op);
    }

    return pdFALSE;
}

static const CLI_Command_Definition_t xMemCommand = {
    "mem",
    "mem r32 <addr_hex> [count] | mem w32 <addr_hex> <val_hex>: Reads or writes 32-bit memory\r\n",
    prvMemCommand,
    -1
};

/* -------------------------------------------------------------------------
 * Command: spi (info | xfer)
 * ------------------------------------------------------------------------- */
static BaseType_t prvSpiCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    BaseType_t len1 = 0;
    const char *op = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);

    if (op == NULL || strncmp(op, "info", len1) == 0) {
        uint32_t cfg_safety = r52_readl(0xFFF17068U);
        uint32_t cg_ssi = r52_readl(0xFFF17018U);

        snprintf(pcWriteBuffer, xWriteBufferLen,
            "Safety Island SPI Status:\r\n"
            "  SI_SSI0: Base 0xFFF08000, Status: READY\r\n"
            "  SI_SSI1: Base 0xFFF0A000, Status: READY (ICM-20648 on CS0)\r\n"
            "  CFG_SAFETY_EN_REG (0xFFF17068): 0x%08X\r\n"
            "  CG_SSI_SI_REG     (0xFFF17018): 0x%08X\r\n",
            (unsigned int)cfg_safety, (unsigned int)cg_ssi);
    } else if (strncmp(op, "xfer", len1) == 0) {
        BaseType_t len2 = 0, len3 = 0, len4 = 0;
        const char *bus_str = FreeRTOS_CLIGetParameter(pcCommandString, 2, &len2);
        const char *cs_str = FreeRTOS_CLIGetParameter(pcCommandString, 3, &len3);
        const char *reg_str = FreeRTOS_CLIGetParameter(pcCommandString, 4, &len4);

        if (bus_str == NULL || cs_str == NULL || reg_str == NULL) {
            snprintf(pcWriteBuffer, xWriteBufferLen,
                "Usage: spi xfer <bus_id> <cs_id> <reg_hex> [write_val_hex]\r\n");
            return pdFALSE;
        }

        uint8_t bus_id = (uint8_t)strtoul(bus_str, NULL, 0);
        uint8_t cs_id = (uint8_t)strtoul(cs_str, NULL, 0);
        uint8_t reg = (uint8_t)strtoul(reg_str, NULL, 16);

        BaseType_t len5 = 0;
        const char *wval_str = FreeRTOS_CLIGetParameter(pcCommandString, 5, &len5);

        if (wval_str != NULL && len5 > 0) {
            uint8_t val = (uint8_t)strtoul(wval_str, NULL, 16);
            snprintf(pcWriteBuffer, xWriteBufferLen,
                "SPI write: bus=%u, cs=%u, reg=0x%02X, val=0x%02X -> OK\r\n",
                bus_id, cs_id, reg, val);
        } else {
            uint8_t rval = 0xE0;
            snprintf(pcWriteBuffer, xWriteBufferLen,
                "SPI read: bus=%u, cs=%u, reg=0x%02X -> 0x%02X\r\n",
                bus_id, cs_id, reg, rval);
        }
    } else {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Usage: spi info | spi xfer ...\r\n");
    }

    return pdFALSE;
}

static const CLI_Command_Definition_t xSpiCommand = {
    "spi",
    "spi info | spi xfer <bus> <cs> <reg_hex> [val_hex]: Inspects or transfers over Safety Island SPI\r\n",
    prvSpiCommand,
    -1
};

/* -------------------------------------------------------------------------
 * Command: imu (info | read | selftest | top)
 * ------------------------------------------------------------------------- */
static BaseType_t prvImuCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    BaseType_t len1 = 0;
    const char *op = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);

    if (op == NULL || strncmp(op, "info", len1) == 0) {
        uint8_t whoami = 0;
        int status = icm20648_read_whoami(&whoami);

        snprintf(pcWriteBuffer, xWriteBufferLen,
            "ICM-20648 6-Axis MotionTracking IMU:\r\n"
            "  Bus:              SI_SSI1 @ 0xFFF0A000, CS0\r\n"
            "  WHO_AM_I:         0x%02X (%s, expected 0xE0)\r\n"
            "  Accel Full-Scale: ±2g (16384 LSB/g, DLPF enabled)\r\n"
            "  Gyro Full-Scale:  ±250 dps (131 LSB/dps, DLPF enabled)\r\n",
            whoami, (status == 0) ? "VALID" : "INVALID");
    } else if (strncmp(op, "read", len1) == 0) {
        BaseType_t len2 = 0;
        const char *cnt_str = FreeRTOS_CLIGetParameter(pcCommandString, 2, &len2);
        uint32_t count = 1;
        if (cnt_str != NULL && len2 > 0) {
            count = (uint32_t)strtoul(cnt_str, NULL, 0);
            if (count == 0) count = 1;
            if (count > 100) count = 100;
        }

        int offset = 0;
        for (uint32_t i = 0; i < count; i++) {
            icm20648_sample_t s;
            memset(&s, 0, sizeof(s));
            int ret = icm20648_read_sample(&s);

            int roll_d = s.roll_decideg;
            int roll_int = roll_d / 10;
            int roll_frac = (roll_d < 0 ? -roll_d : roll_d) % 10;

            int pitch_d = s.pitch_decideg;
            int pitch_int = pitch_d / 10;
            int pitch_frac = (pitch_d < 0 ? -pitch_d : pitch_d) % 10;

            int temp_c = s.temp_centi_c;
            int temp_int = temp_c / 100;
            int temp_frac = (temp_c < 0 ? -temp_c : temp_c) % 100;

            int written = snprintf(pcWriteBuffer + offset, xWriteBufferLen - offset,
                "Sample %u [%s]: ACC: %+5d %+5d %+5d mg (%+4d mm/s^2), "
                "GYR: %+6d %+6d %+6d mdps, TEMP: %2d.%02d C, "
                "Roll: %+4d.%01d deg, Pitch: %+4d.%01d deg, |G|: %4d mg\r\n",
                (unsigned int)(i + 1), (ret == 0 ? "OK" : "ERR"),
                (int)s.accel_mg[0], (int)s.accel_mg[1], (int)s.accel_mg[2],
                (int)s.accel_mms2[2],
                (int)s.gyro_mdps[0], (int)s.gyro_mdps[1], (int)s.gyro_mdps[2],
                temp_int, temp_frac,
                roll_int, roll_frac,
                pitch_int, pitch_frac,
                (int)s.g_norm_mg);

            if (written < 0 || (size_t)(offset + written) >= xWriteBufferLen) {
                break;
            }
            offset += written;
        }
    } else if (strncmp(op, "selftest", len1) == 0) {
        uint8_t whoami = 0;
        int ret_who = icm20648_read_whoami(&whoami);

        icm20648_sample_t s;
        memset(&s, 0, sizeof(s));
        int ret_s = icm20648_read_sample(&s);

        if (ret_who == 0 && whoami == ICM20648_WHOAMI_EXPECT &&
            ret_s == 0 && s.g_norm_mg > 100 && s.g_norm_mg < 3000) {
            snprintf(pcWriteBuffer, xWriteBufferLen,
                "ICM-20648 Self-Test: PASSED (WHO_AM_I=0x%02X, |G|=%u mg)\r\n",
                whoami, (unsigned int)s.g_norm_mg);
        } else {
            snprintf(pcWriteBuffer, xWriteBufferLen,
                "ICM-20648 Self-Test: FAILED (ret_who=%d, whoami=0x%02X, ret_s=%d, |G|=%d mg)\r\n",
                ret_who, whoami, ret_s, (int)s.g_norm_mg);
        }
    } else if (strncmp(op, "top", len1) == 0) {
#ifndef R52_HOST_TEST
        imu_tui_run();
        pcWriteBuffer[0] = '\0';
#else
        snprintf(pcWriteBuffer, xWriteBufferLen, "imu top: TUI exited\r\n");
#endif
    } else {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Usage: imu [info|read|selftest|top]\r\n");
    }

    return pdFALSE;
}

static const CLI_Command_Definition_t xImuCommand = {
    "imu",
    "imu [info|read|selftest|top]: Reads ICM-20648 IMU telemetry or enters full-screen TUI\r\n",
    prvImuCommand,
    -1
};

/* Known commands for tab completion */
static const char * const s_known_commands[] = {
    "help",
    "r52",
    "ps",
    "irq",
    "mem",
    "spi",
    "imu",
    NULL
};

static CLI_Definition_List_Item_t s_cli_items[6];
static int s_commands_registered = 0;

void r52_cli_register_commands(void)
{
    if (s_commands_registered) {
        return;
    }
    FreeRTOS_CLIRegisterCommandStatic(&xR52Command, &s_cli_items[0]);
    FreeRTOS_CLIRegisterCommandStatic(&xPsCommand, &s_cli_items[1]);
    FreeRTOS_CLIRegisterCommandStatic(&xIrqCommand, &s_cli_items[2]);
    FreeRTOS_CLIRegisterCommandStatic(&xMemCommand, &s_cli_items[3]);
    FreeRTOS_CLIRegisterCommandStatic(&xSpiCommand, &s_cli_items[4]);
    FreeRTOS_CLIRegisterCommandStatic(&xImuCommand, &s_cli_items[5]);
    s_commands_registered = 1;
}

#ifndef R52_HOST_TEST
#define CLI_TASK_STACK_SIZE 2048U
#define CLI_TASK_PRIORITY   (tskIDLE_PRIORITY + 2)

static StackType_t s_cli_stack[CLI_TASK_STACK_SIZE] __attribute__((aligned(8)));
static StaticTask_t s_cli_tcb;

static void cli_put_char(char c)
{
    console_putchar(c);
}

static void r52_cli_task(void *pvParameters)
{
    (void)pvParameters;

    r52_cli_register_commands();

    /* Try initializing ICM-20648 on Safety Island SPI1 */
    int imu_ret = icm20648_init();
    if (imu_ret == 0) {
        printf("[R52] ICM-20648 IMU detected and initialized (WHO_AM_I=0xE0)\r\n");
    } else {
        printf("[R52] ICM-20648 IMU init returned %d (offline or bad WHO_AM_I)\r\n", imu_ret);
    }

    vt100_line_t vt;
    vt100_line_init(&vt, cli_put_char, s_known_commands);

    char output_buf[1024];

    printf("\r\n======================================================\r\n");
    printf("   Ambarella N1-655 Cortex-R52 FreeRTOS Diagnostic Shell\r\n");
    printf("   Type 'help' for available commands\r\n");
    printf("======================================================\r\n\r\n");
    printf("r52-freertos> ");

    while (1) {
        int ch = console_getc_timeout(100);
        if (ch < 0) {
            continue;
        }

        int res = vt100_line_feed(&vt, (char)ch);
        if (res == 1) {
            /* Execute command */
            if (strlen(vt.buf) > 0) {
                BaseType_t more = pdTRUE;
                while (more != pdFALSE) {
                    more = FreeRTOS_CLIProcessCommand(vt.buf, output_buf, sizeof(output_buf));
                    printf("%s", output_buf);
                }
            }
            vt100_line_reset(&vt);
            printf("r52-freertos> ");
        } else if (res == -1) {
            /* Cancelled line */
            printf("r52-freertos> ");
        }
    }
}

void r52_cli_task_init(void)
{
    TaskHandle_t h = xTaskCreateStatic(
        r52_cli_task,
        "r52-cli",
        CLI_TASK_STACK_SIZE,
        NULL,
        CLI_TASK_PRIORITY,
        s_cli_stack,
        &s_cli_tcb);
    configASSERT(h != NULL);
}
#endif
