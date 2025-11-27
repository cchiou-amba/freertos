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
#include <common.h>
#include <ambhw/spi.h>
#include "virtio.h"
#include "virtio_id.h"

#define SPI_VIRTIO_NUM	2

struct virtio_spi_config {
    u8 cs_max_number;
    u8 cs_change_supported;
#define VIRTIO_SPI_RX_TX_SUPPORT_DUAL   (1 << 0)
#define VIRTIO_SPI_RX_TX_SUPPORT_QUAD   (1 << 1)
#define VIRTIO_SPI_RX_TX_SUPPORT_OCTAL  (1 << 2)
    u8 tx_nbits_supported;
    u8 rx_nbits_supported;
    u32 bits_per_word_mask;
#define VIRTIO_SPI_MF_SUPPORT_CPHA_0        (1 << 0)
#define VIRTIO_SPI_MF_SUPPORT_CPHA_1        (1 << 1)
#define VIRTIO_SPI_MF_SUPPORT_CPOL_0        (1 << 2)
#define VIRTIO_SPI_MF_SUPPORT_CPOL_1        (1 << 3)
#define VIRTIO_SPI_MF_SUPPORT_CS_HIGH       (1 << 4)
#define VIRTIO_SPI_MF_SUPPORT_LSB_FIRST     (1 << 5)
#define VIRTIO_SPI_MF_SUPPORT_LOOPBACK      (1 << 6)
    u32 mode_func_supported;
    u32 max_freq_hz;
    u32 max_word_delay_ns;
    u32 max_cs_setup_ns;
    u32 max_cs_hold_ns;
    u32 max_cs_inactive_ns;
};

struct spi_transfer_head {
    u8 chip_select_id;
    u8 bits_per_word;
    u8 cs_change;
    u8 tx_nbits;
    u8 rx_nbits;
    u8 reserved[3];
    u32 mode;
    u32 freq;
    u32 word_delay_ns;
    u32 cs_setup_ns;
    u32 cs_delay_hold_ns;
    u32 cs_change_delay_inactive_ns;
};

struct spi_transfer_result {
    #define VIRTIO_SPI_TRANS_OK	    0
    #define VIRTIO_SPI_PARAM_ERR	1
    #define VIRTIO_SPI_TRANS_ERR	2
    u8 result;
};


static int spi_virtio_cb(void *binding, struct virtio_pkt *pkt, uint32_t num)
{
    struct spi_transfer_head *th = NULL;
    u8 cs_id, bits_per_word;
    u8 *tx_buf = NULL;
    u8 *rx_buf = NULL;
    u32 tx_len, rx_len;
    struct spi_transfer_result *result = NULL;
    u32 freq, mode;
    u8 i = 0;
    int ret = 0;
    struct spi_dev *dev = binding;
    if (num < 2 || num > 4) {
        printf("Invalid packet count: %u\n", num);
        return -1;
    }

    if (pkt[0].len != sizeof(struct spi_transfer_head)) {
        pr_err("Invalid transfer head size: %u vs %u\n",
               pkt[0].len, sizeof(struct spi_transfer_head));
        return -1;
    }

    th = (struct spi_transfer_head *)pkt[0].data;
    cs_id = th->chip_select_id;
    bits_per_word = th->bits_per_word;
    freq = th->freq;
    mode = th->mode;

    for (i = 1; i < num; i++) {
        if (!pkt[i].is_write) {
            tx_buf = pkt[i].data;
            tx_len = pkt[i].len;
        } else if (pkt[i].len > sizeof(struct spi_transfer_result)) {
            rx_buf = pkt[i].data;
            rx_len = pkt[i].len;
        } else {
            result = (struct spi_transfer_result *)pkt[i].data;
        }
    }
    spi_cfg_t config = {
        .cfs_dfs = bits_per_word,
        .baud_rate = freq,
        .spi_mode = mode
    };

    spi_write_and_read_t war = {
        .bus_id = dev->id,
        .cs_id = cs_id,
        .w_buffer = tx_buf,
        .r_buffer = rx_buf,
        .n_size = tx_len,
    };

    ret = spi_write_and_read(&config, &war);
    if (ret < 0) {
        pr_err("SPI transfer failed: %d\n", ret);
        result->result = VIRTIO_SPI_TRANS_ERR;
    } else {
        result->result = VIRTIO_SPI_TRANS_OK;
    }

    return 0;
}


struct virtio_spi_config spi0_config = {
    .cs_max_number = SPI_MAX_CS_NUM,
    .cs_change_supported = 1,
    .tx_nbits_supported = VIRTIO_SPI_RX_TX_SUPPORT_OCTAL | VIRTIO_SPI_RX_TX_SUPPORT_QUAD,
    .rx_nbits_supported = VIRTIO_SPI_RX_TX_SUPPORT_OCTAL | VIRTIO_SPI_RX_TX_SUPPORT_QUAD,
    .mode_func_supported = 0x3F,
    .max_freq_hz = SPI_MASTER_MAX_SPEED,
    .max_cs_setup_ns = 0,
    .max_cs_hold_ns = 0,
    .max_cs_inactive_ns = 0,
    .max_word_delay_ns = 0,
};

static struct virtio_mmio_device spi0_dev = {
    .name = "spi0-virtio",
    .base = VIRTIO_BASE(VIRTIO_ID_SPI, 0),
    .device_id = VIRTIO_ID_SPI,
    .vendor_id = 0xffffffff,
    .irq[VIRTIO_IRQ_DEVICE_TO_DRIVER] = 1,
    .irq[VIRTIO_IRQ_DRIVER_TO_DEVICE] = VIRTIO_DEVICE_IRQ1,
    .callback = spi_virtio_cb,
    .config_ptr = &spi0_config,
    .config_size = sizeof(spi0_config)
};

struct virtio_spi_config spi1_config = {
    .cs_max_number = SPI_MAX_CS_NUM,
    .cs_change_supported = 1,
    .tx_nbits_supported = VIRTIO_SPI_RX_TX_SUPPORT_OCTAL | VIRTIO_SPI_RX_TX_SUPPORT_QUAD,
    .rx_nbits_supported = VIRTIO_SPI_RX_TX_SUPPORT_OCTAL | VIRTIO_SPI_RX_TX_SUPPORT_QUAD,
    .mode_func_supported = 0x3F,
    .max_freq_hz = SPI_MASTER_MAX_SPEED,
    .max_cs_setup_ns = 0,
    .max_cs_hold_ns = 0,
    .max_cs_inactive_ns = 0,
    .max_word_delay_ns = 0,
};

static struct virtio_mmio_device spi1_dev = {
    .name = "spi1-virtio",
    .base = VIRTIO_BASE(VIRTIO_ID_SPI, 1),
    .device_id = VIRTIO_ID_SPI,
    .vendor_id = 0xffffffff,
    .irq[VIRTIO_IRQ_DEVICE_TO_DRIVER] = 2,
    .irq[VIRTIO_IRQ_DRIVER_TO_DEVICE] = VIRTIO_DEVICE_IRQ2,
    .callback = spi_virtio_cb,
    .config_ptr = &spi1_config,
    .config_size = sizeof(spi1_config)
};

static void virtio_spi_task(void *arg)
{
	if (virtio_mmio_device_register(arg)) {
		printf("Failed to register virtio-mmio: spi\n");
	}

	vTaskDelete(NULL);
}

static StackType_t stack[SPI_VIRTIO_NUM][VIRTIO_TASK_STACK_DEPTH];
void virtio_spi_init(int id)
{
    struct virtio_mmio_device *dev = NULL;
    int ret = 0;

	if (id >= SPI_VIRTIO_NUM)
		return;

	if (id == 0) {
		dev = &spi0_dev;
	} else if (id == 1) {
		dev = &spi1_dev;
	}

	if (dev == NULL)
		return;

	dev->stack = stack[id];
	dev->binding = spi_get_bus_device(id);

    printf("mmio:\n");
	printf("\tname: %s\n", dev->name);
	printf("\tbase: 0x%08x\n", dev->base);
	printf("\t irq: notify(%u), local(%u)\n",
	       dev->irq[VIRTIO_IRQ_DEVICE_TO_DRIVER],
	       dev->irq[VIRTIO_IRQ_DRIVER_TO_DEVICE]);

	ret = xTaskCreateStatic(virtio_spi_task, dev->name, VIRTIO_TASK_STACK_DEPTH,
				dev, VIRTIO_TASK_PRIOR, dev->stack, &dev->tcb);
	if (!ret) {
		printf("Failed to create task %s\n", dev->name);
	}
}
