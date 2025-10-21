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

#include <stdint.h>
#include <hwio.h>
#include <i2c.h>
#include "virtio.h"

#include "virtio_id.h"

#define I2C0_VIRTIO	1
#define I2C1_VIRTIO	1
#define I2C2_VIRTIO	1
#define I2C_VIRTIO_NUM	3

#define VIRTIO_I2C_MSG_OK	0
#define VIRTIO_I2C_MSG_ERR	1

/* Linux 6.12 hints 'Zero-length request feature is mandatory' */
#define VIRTIO_I2C_F_ZERO_LENGTH_REQUEST	0

// #define DEBUG

struct virtio_i2c_out_hdr {
	uint16_t addr;
	uint16_t padding;
	uint32_t flags;
};

struct virtio_i2c_in_hdr {
	uint8_t status;
};

static int i2c_virtio_cb(void *binding, struct virtio_pkt *pkt, uint32_t num)
{
	uint8_t *buf, *sta;
	uint32_t len;
	int ret = 0, xfer_mode;
	struct virtio_i2c_out_hdr *out;
	struct i2c_dev *dev = binding;
	struct i2c_msg msg;

	if (num != 3)
		return -1;

	// i2c_send_slave()
	// i2c_send_regs()
	//  [w]
	//  	- i2c_send_data()
	//  [r]
	//  	- i2c_send_slave(addr|1)
	//  	- i2c_recv_data()

	if (pkt[0].len != sizeof(struct virtio_i2c_out_hdr) || pkt[2].len != 1) {
		pr_err("illegal pkt len %u,%u\n",
		       pkt[0].len, pkt[2].len);
		return -1;
	}

	if (pkt[0].is_write || !pkt[2].is_write) {
		pr_err("illegal value of is_write: %u.%u\n",
		       pkt[0].is_write, !pkt[2].is_write);
		return -1;
	}


	out = pkt[0].data;
	buf = pkt[1].data;
	sta = pkt[2].data;
	len = pkt[1].len;
	xfer_mode = pkt[1].is_write ? I2C_XFER_READ : I2C_XFER_WRITE;	// write-back the buffer means I2C read operation

	msg.buf = buf;
	msg.len = len;

#if defined(DEBUG)
	printf("out->addr = 0x%02x\n", out->addr);
	printf("out->padding = 0x%02x\n", out->padding);
	printf("out->flags = 0x%02x\n", out->flags);
	printf("len = %u\n", len);
#endif
	ret = i2c_transfer(dev->chan, out->addr, &msg, xfer_mode);
	if (!ret) {
		*sta = VIRTIO_I2C_MSG_OK;
	} else {
		*sta = VIRTIO_I2C_MSG_ERR;
	}

	if (!pkt[1].is_write) {
#if defined(DEBUG)
		int i;
		printf("W: ");
		for (i = 0; i < len; i++) {
			printf("%02x ", buf[i]);
		}
		printf("\n");
#endif
	}

	return 0;
}
/*
 * Typical device tree node for i2c-virtio, eg.:
 *
 * // I2C2
 * virtio@2111000 {
 *         compatible = "virtio,mmio";
 *         reg = <0x0 0x2112000 0x0 0x1000>;
 *         interrupts = <0 73 0x1>;
 *         dma-ranges = <0 0 0 0 0 0x7FD00000>;
 *
 *         amb,irq-regmap = <&ca78ae_scratchpad_syscon 0x1000>;
 *         amb,irq-notify = <0x10 0x8 0x8>;
 *         // offset mask value
 *         amb,irq-clear = <>;
 *
 *         i2c0 {
 *                 compatible = "virtio,device22";
 *                 #address-cells = <1>;
 *                 #size-cells = <0>;
 *         };
 * };
 */

#if defined(I2C0_VIRTIO)
static struct virtio_mmio_device i2c0_dev = {
	.name = "i2c-virtio",
	.base = VIRTIO_BASE(VIRTIO_ID_I2C_ADAPTER, 0),
	.device_id = VIRTIO_ID_I2C_ADAPTER,
	.vendor_id = 0xffffffff,
	.irq[VIRTIO_IRQ_DEVICE_TO_DRIVER] = 1,
	.irq[VIRTIO_IRQ_DRIVER_TO_DEVICE] = VIRTIO_DEVICE_IRQ1,
	.callback = i2c_virtio_cb,
	.features =  1U << VIRTIO_I2C_F_ZERO_LENGTH_REQUEST,
};
#endif

#if defined(I2C1_VIRTIO)
static struct virtio_mmio_device i2c1_dev = {
	.name = "i2c-virtio",
	.base = VIRTIO_BASE(VIRTIO_ID_I2C_ADAPTER, 1),
	.device_id = VIRTIO_ID_I2C_ADAPTER,
	.vendor_id = 0xffffffff,
	.irq[VIRTIO_IRQ_DEVICE_TO_DRIVER] = 2,
	.irq[VIRTIO_IRQ_DRIVER_TO_DEVICE] = VIRTIO_DEVICE_IRQ2,
	.callback = i2c_virtio_cb,
	.features =  1U << VIRTIO_I2C_F_ZERO_LENGTH_REQUEST,
};
#endif

#if defined(I2C2_VIRTIO)
static struct virtio_mmio_device i2c2_dev = {
	.name = "i2c-virtio",
	.base = VIRTIO_BASE(VIRTIO_ID_I2C_ADAPTER, 2),
	.device_id = VIRTIO_ID_I2C_ADAPTER,
	.vendor_id = 0xffffffff,
	.irq[VIRTIO_IRQ_DEVICE_TO_DRIVER] = 3,
	.irq[VIRTIO_IRQ_DRIVER_TO_DEVICE] = VIRTIO_DEVICE_IRQ3,
	.callback = i2c_virtio_cb,
	.features =  1U << VIRTIO_I2C_F_ZERO_LENGTH_REQUEST,
};
#endif

static void virtio_i2c_task(void *arg)
{
	if (virtio_mmio_device_register(arg)) {
		printf("Failed to register virtio-mmio: i2c\n");
	}

	vTaskDelete(NULL);
}

static StackType_t stack[I2C_VIRTIO_NUM][VIRTIO_TASK_STACK_DEPTH];
/* ------------------------------------------------------------ */
void virtio_i2c_init(int id)
{
	TaskHandle_t ret;
	struct virtio_mmio_device *dev = NULL;
	uint8_t i2c_bus = 0;

	if (id >= I2C_VIRTIO_NUM)
		return;

	switch(id) {
	case 0:
#if defined(I2C0_VIRTIO)
		dev = &i2c0_dev;
		i2c_bus = I2C0;
#endif
		break;
	case 1:
#if defined(I2C1_VIRTIO)
		dev = &i2c1_dev;
		i2c_bus = I2C1;
#endif
		break;
	case 2:
#if defined(I2C2_VIRTIO)
		dev = &i2c2_dev;
		i2c_bus = I2C2;
#endif
		break;
	default:
		break;
	}

	if (dev == NULL)
		return;

	dev->stack = stack[id];
	dev->binding = i2c_get_bus_device(i2c_bus);
	ret = xTaskCreateStatic(virtio_i2c_task, dev->name, VIRTIO_TASK_STACK_DEPTH,
				dev, VIRTIO_TASK_PRIOR, dev->stack, &dev->tcb);
	if (!ret) {
		printf("Failed to create task %s\n", dev->name);
	}
}
