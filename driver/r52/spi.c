/*******************************************************************************
*
* Copyright (c) 2026 Ambarella International LP
*
* License Identifier: AMBARELLA-2-Clause
*
******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include <plat.h>
#include <ambhw/spi.h>
#include "portable.h"

#define SPI_INSTANCES 2
#define SPI_DUMMY_DATA 0xffff
#define SPI_MAX_QUERY_TIMES 10
#define SPI_BUF_MAX_LEN 4096
#define SPI_MAX_CS_NUM 1
#define FIFO_THRESHOLD 32

struct spi_bus {
    struct spi_dev dev;
    int status;
    unsigned long regbase;
    u32 clk_freq;
    int cs_pins[SPI_MAX_CS_NUM];
    SemaphoreHandle_t mutex;
    EventGroupHandle_t event;
    u8 *tx_ptr;
    u32 tx_cnt;
    u8 *rx_ptr;
    u32 rx_cnt;
    u32 dummy_cnt;
};

static StaticEventGroup_t spi_event_storage[SPI_INSTANCES];
static StaticSemaphore_t spi_mutex_storage[SPI_INSTANCES];

static struct spi_bus spi_bus[SPI_INSTANCES] = {
    [0] = {
        .dev = {
            .name = "spi0",
            .irq = SPI0_IRQ,

        },
        .regbase = SPI0_BASE,
        .status = SPI_STATE_RESET,
    },
    [1] = {
        .dev = {
            .name = "spi1",
            .irq = SPI1_IRQ,
        },
        .regbase = SPI1_BASE,
        .status = SPI_STATE_RESET,
    },
};

static int event_init(struct spi_bus *bus)
{
    int rval = 0;
    bus->event = xEventGroupCreateStatic(&spi_event_storage[bus->dev.id]);
    if (bus->event == NULL) {
        rval = -1;
        printf("spi%u event init failed\n", bus->dev.id);
    }
    return rval;
}

static int mutex_init(struct spi_bus *bus)
{
    int rval = 0;
    bus->mutex = xSemaphoreCreateMutexStatic(&spi_mutex_storage[bus->dev.id]);
    if (bus->mutex == NULL) {
        rval = -1;
        printf("spi%u mutex init failed\n", bus->dev.id);
    }
    return rval;
}

static void mutex_lock(SemaphoreHandle_t mutex)
{
    const TickType_t timeout_ticks = pdMS_TO_TICKS(100);
    xSemaphoreTake(mutex, timeout_ticks);
}

static void mutex_unlock(SemaphoreHandle_t mutex)
{
    xSemaphoreGive(mutex);
}


static inline void spi_enable(struct spi_bus *bus)
{
    writel(bus->regbase + SPI_SER_OFFSET, 1 << bus->dev.cs_id);
    writel(bus->regbase + SPI_SSIENR_OFFSET, SPI_SSIENR_ENABLE);
}

static inline void spi_disable(struct spi_bus *bus)
{
    writel(bus->regbase + SPI_SSIENR_OFFSET, 0);
    writel(bus->regbase + SPI_SER_OFFSET, 0);
    writel(bus->regbase + SPI_IMR_OFFSET, 0);
}

static void spi_safety_disable(u8 spi_id)
{
    setbitsl(CFG_SAFETY_EN_REG, 1U << (spi_id + 2));
}

static u32 spi_get_clk_source(void)
{
    u32 clk;
    clk = current_cpufreq();
    clk = clk / 2; // clk / 2 is the llpp clock
    clk = clk / readl( SI_CR52_CONFIG_BASE + CG_SSI_SI_REG_OFFSET); //gclk_ssi
    return clk;
}

static void spi_set_clk(struct spi_bus *bus, u32 freq)
{
    u32 sckdv = bus->clk_freq / freq;
    writel(bus->regbase + SPI_BAUDR_OFFSET, sckdv);
}

static void spi_irq_handler(void *arg)
{
    struct spi_bus *bus = (struct spi_bus *)arg;
    u32 isr = readl(bus->regbase + SPI_ISR_OFFSET);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (isr & SPI_TXEIS_MASK) {
        xEventGroupSetBitsFromISR(bus->event, SPI_EVENT_TX_READY, &xHigherPriorityTaskWoken);
    } else if (isr & SPI_TXOIS_MASK) {
        readl(bus->regbase + SPI_TXOICR_OFFSET);
        xEventGroupSetBitsFromISR(bus->event, SPI_EVENT_TX_OVERFLOW, &xHigherPriorityTaskWoken);
    }

    if (isr & SPI_RXFIS_MASK) {
        xEventGroupSetBitsFromISR(bus->event, SPI_EVENT_RX_READY, &xHigherPriorityTaskWoken);
    } else if (isr & SPI_RXUIS_MASK) {
        readl(bus->regbase + SPI_RXUICR_OFFSET);
        xEventGroupSetBitsFromISR(bus->event, SPI_EVENT_RX_UNDERFLOW, &xHigherPriorityTaskWoken);
    } else if (isr & SPI_RXOIS_MASK) {
        readl(bus->regbase + SPI_RXOICR_OFFSET);
        xEventGroupSetBitsFromISR(bus->event, SPI_EVENT_RX_OVERFLOW, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}

int spi_init(int id)
{
    struct spi_bus *bus;
    if (id >= SPI_INSTANCES) {
        printf("SPI%u init failed\n", id);
        return -1;
    }
    bus = &spi_bus[id];
    bus->dev.id = id;
    spi_safety_disable(id);
    spi_disable(bus);
    bus->clk_freq = spi_get_clk_source();
    mutex_init(bus);
    event_init(bus);
    request_irq(bus->dev.irq, IRQ_RISING_EDGE, spi_irq_handler, bus, bus->dev.name);
    bus->status = SPI_STATE_READY;

    return 0;
}

static void spi_end_xfer(struct spi_bus *bus)
{
    writel(bus->regbase + SPI_DMAC_OFFSET, 0);
    spi_disable(bus);
    bus->status = SPI_STATE_READY;
}

static void spi_rx_from_fifo(struct spi_bus *bus)
{
    u32 bytes_to_read;

    bytes_to_read = readl(bus->regbase + SPI_RXFLR_OFFSET);
    bytes_to_read = bytes_to_read < bus->rx_cnt ? bytes_to_read : bus->rx_cnt;
    bytes_to_read = bytes_to_read < FIFO_THRESHOLD ? bytes_to_read : FIFO_THRESHOLD;

    while (bytes_to_read--) {
        *(bus->rx_ptr++) = readl(bus->regbase + SPI_DR_OFFSET);
        bus->rx_cnt--;
    }

    if (bus->rx_cnt == 0)
        clrbitsl(bus->regbase + SPI_IMR_OFFSET, SPI_RXFIS_MASK);
}

static void spi_tx_to_fifo(struct spi_bus *bus)
{
    u32 bytes_to_send;

    bytes_to_send = bus->tx_cnt < FIFO_THRESHOLD ? bus->tx_cnt : FIFO_THRESHOLD;
    while (bytes_to_send--) {
        writel(bus->regbase + SPI_DR_OFFSET, *(bus->tx_ptr++));
        bus->tx_cnt--;
    }

    if (bus->tx_cnt == 0)
        clrbitsl(bus->regbase + SPI_IMR_OFFSET, SPI_TXEIS_MASK);
}

int spi_xfer_irq(struct spi_bus *bus)
{
    int ret = 0;
    EventBits_t bits;

    switch (bus->status) {
        case SPI_STATE_BUSY_TX_AND_RX:
            writel(bus->regbase + SPI_IMR_OFFSET, SPI_TXEIS_MASK | SPI_RXFIS_MASK);
            writel(bus->regbase + SPI_TXFTLR_OFFSET, 0);
            writel(bus->regbase + SPI_RXFTLR_OFFSET, 1);
            break;
        case SPI_STATE_BUSY_TX_THEN_RX:
            writel(bus->regbase + SPI_IMR_OFFSET, SPI_TXEIS_MASK | SPI_RXFIS_MASK);
            writel(bus->regbase + SPI_TXFTLR_OFFSET, 0);
            writel(bus->regbase + SPI_RXFTLR_OFFSET, 1);
            break;
        case SPI_STATE_BUSY_TX:
            writel(bus->regbase + SPI_IMR_OFFSET, SPI_TXEIS_MASK | SPI_RXFIS_MASK);
            writel(bus->regbase + SPI_TXFTLR_OFFSET, 0);
            writel(bus->regbase + SPI_RXFTLR_OFFSET, 1);
            break;
        default:
            printf("SPI bus %u not in valid state %d\n", bus->dev.id, bus->status);
            return -1;
    }

    writel(bus->regbase + SPI_DMAC_OFFSET, SPI_DMAC_TX_EN);
    if (bus->tx_cnt > 0)
        spi_tx_to_fifo(bus);

    spi_enable(bus);

    while ((bus->tx_cnt > 0 || bus->rx_cnt > 0)) {
        bits = xEventGroupWaitBits(bus->event, SPI_EVENT_TX_READY |SPI_EVENT_RX_READY |
            SPI_EVENT_TX_OVERFLOW | SPI_EVENT_RX_UNDERFLOW | SPI_EVENT_RX_OVERFLOW, pdTRUE,
            pdFALSE, pdMS_TO_TICKS(100));

        if (bits == 0) {
            ret = -1;
            printf("SPI transfer timeout\n");
            goto exit;
        }

        if (bits & SPI_EVENT_RX_READY) {
            if (bus->rx_cnt > 0)
                spi_rx_from_fifo(bus);
        }

        if (bits & SPI_EVENT_TX_READY) {
            if (bus->tx_cnt > 0)
                spi_tx_to_fifo(bus);
        }

        if (bits & SPI_EVENT_TX_OVERFLOW) {
            ret = -1;
            printf("SPI TXOIS_MASK\n");
            goto exit;
        }

        if (bits & SPI_EVENT_RX_UNDERFLOW) {
            ret = -1;
            printf("SPI RXUIS_MASK\n");
            goto exit;
        }

        if (bits & SPI_EVENT_RX_OVERFLOW) {
            ret = -1;
            printf("SPI RXOIS_MASK\n");
            goto exit;
        }

    }

exit:
    spi_end_xfer(bus);
    return ret;
}

int spi_write_then_read(spi_cfg_t *spi_cfg, spi_write_then_read_t *spi_write_then_read)
{
    int ret = 0;
    struct spi_bus *bus;
    spi_ctrl0_reg_t cr0 = {0};
    u32 buffer_size = 0;
    u8 *transfer_buffer = NULL;

    if (spi_write_then_read->bus_id >= SPI_INSTANCES) {
        printf("SPI bus id %u out of range\n", spi_write_then_read->bus_id);
        return -1;
    }

    bus = &spi_bus[spi_write_then_read->bus_id];
    if (!bus->status) {
        printf("SPI bus %u not initialized\n", spi_write_then_read->bus_id);
        return -1;
    }

    mutex_lock(bus->mutex);

    spi_disable(bus);
    spi_set_clk(bus, spi_cfg->baud_rate);
    cr0.s.dfs = spi_cfg->cfs_dfs - 1;
    cr0.s.scph = (spi_cfg->spi_mode & SPI_CPHA) ? 1 : 0;
    cr0.s.scpol = (spi_cfg->spi_mode & SPI_CPOL) ? 1 : 0;
    cr0.s.tmod = SPI_WRITE_READ;
    cr0.s.tx_lsb = !!(spi_cfg->spi_mode & SPI_LSB_FIRST);
    cr0.s.rx_lsb = !!(spi_cfg->spi_mode & SPI_LSB_FIRST);
    cr0.s.residue = 1;
    bus->status = SPI_STATE_BUSY_TX_THEN_RX;
    buffer_size = spi_write_then_read->w_size + spi_write_then_read->r_size;
    transfer_buffer = (u8 *)pvPortMalloc(buffer_size);
    if (transfer_buffer == NULL) {
        printf("Failed to allocate transfer buffer\n");
        mutex_unlock(bus->mutex);
        return -1;
    }
    memset(transfer_buffer, 0x00, buffer_size);
    memcpy(transfer_buffer, spi_write_then_read->w_buffer, spi_write_then_read->w_size);
    bus->tx_ptr = transfer_buffer;
    bus->tx_cnt = buffer_size;
    bus->rx_ptr = transfer_buffer;
    bus->rx_cnt = buffer_size;
    writel(bus->regbase + SPI_CTRLR0_OFFSET, cr0.w);

    ret = spi_xfer_irq(bus);
    if (ret != 0) {
        printf("SPI transfer failed: %d\n", ret);
    }

    memcpy(spi_write_then_read->r_buffer,
        transfer_buffer + spi_write_then_read->w_size,
        spi_write_then_read->r_size);
    vPortFree(transfer_buffer);

    mutex_unlock(bus->mutex);
    return ret;
}

int spi_write_and_read(spi_cfg_t *spi_cfg, spi_write_and_read_t *spi_write_and_read)
{
    int ret = 0;
    struct spi_bus *bus;
    spi_ctrl0_reg_t cr0 = {0};

    if (spi_write_and_read->bus_id >= SPI_INSTANCES) {
        printf("SPI bus id %u out of range\n", spi_write_and_read->bus_id);
        return -1;
    }

    bus = &spi_bus[spi_write_and_read->bus_id];
    if (!bus->status) {
        printf("SPI bus %u not initialized\n", spi_write_and_read->bus_id);
        return -1;
    }

    mutex_lock(bus->mutex);
    spi_disable(bus);
    spi_set_clk(bus, spi_cfg->baud_rate);
    cr0.s.dfs = spi_cfg->cfs_dfs - 1;
    cr0.s.scph = (spi_cfg->spi_mode & SPI_CPHA) ? 1 : 0;
    cr0.s.scpol = (spi_cfg->spi_mode & SPI_CPOL) ? 1 : 0;
    cr0.s.tmod = SPI_WRITE_READ;
    cr0.s.tx_lsb = !!(spi_cfg->spi_mode & SPI_LSB_FIRST);
    cr0.s.rx_lsb = !!(spi_cfg->spi_mode & SPI_LSB_FIRST);
    cr0.s.residue = 1;
    writel(bus->regbase + SPI_CTRLR0_OFFSET, cr0.w);
    bus->status = SPI_STATE_BUSY_TX_AND_RX;
    bus->dev.cs_id = spi_write_and_read->cs_id;
    bus->tx_ptr = spi_write_and_read->w_buffer;
    bus->tx_cnt = spi_write_and_read->n_size;
    bus->rx_ptr = spi_write_and_read->r_buffer;
    bus->rx_cnt = spi_write_and_read->n_size;

    ret = spi_xfer_irq(bus);
    if (ret != 0) {
        printf("SPI transfer failed: %d\n", ret);
    }

    mutex_unlock(bus->mutex);
    return ret;
}

int spi_write(spi_cfg_t *spi_cfg, spi_write_t *spi_write)
{
    int ret = 0;
    struct spi_bus *bus;
    spi_ctrl0_reg_t cr0 = {0};
    u8 *buffer = NULL;
    if (spi_write->bus_id >= SPI_INSTANCES) {
        printf("SPI bus id %u out of range\n", spi_write->bus_id);
        return -1;
    }
    bus = &spi_bus[spi_write->bus_id];
    if (!bus->status) {
        printf("SPI bus %u not initialized\n", spi_write->bus_id);
        return -1;
    }

    mutex_lock(bus->mutex);
    spi_disable(bus);
    spi_set_clk(bus, spi_cfg->baud_rate);
    cr0.s.dfs = spi_cfg->cfs_dfs - 1;
    cr0.s.scph = (spi_cfg->spi_mode & SPI_CPHA) ? 1 : 0;
    cr0.s.scpol = (spi_cfg->spi_mode & SPI_CPOL) ? 1 : 0;
    cr0.s.tmod = SPI_WRITE_READ;
    cr0.s.tx_lsb = !!(spi_cfg->spi_mode & SPI_LSB_FIRST);
    cr0.s.residue = 1;
    writel(bus->regbase + SPI_CTRLR0_OFFSET, cr0.w);
    bus->dev.cs_id = spi_write->cs_id;
    bus->status = SPI_STATE_BUSY_TX;
    bus->tx_ptr = spi_write->buffer;
    bus->tx_cnt = spi_write->n_size;
    buffer = (u8 *)pvPortMalloc(spi_write->n_size);
    if (buffer == NULL) {
        printf("Failed to allocate buffer\n");
        mutex_unlock(bus->mutex);
        return -1;
    }
    memset(buffer, 0x00, spi_write->n_size);
    bus->rx_cnt = spi_write->n_size;
    bus->rx_ptr = buffer;

    ret = spi_xfer_irq(bus);
    if (ret != 0) {
        printf("SPI transfer failed: %d\n", ret);
    }

    vPortFree(buffer);
    mutex_unlock(bus->mutex);
    return ret;
}

struct spi_dev *spi_get_bus_device(int id)
{
    if (id >= SPI_INSTANCES) {
        return NULL;
    }
    return &spi_bus[id].dev;
}