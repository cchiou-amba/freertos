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
#include <gpio.h>

#include "virtio.h"
#include "virtio_id.h"

/* Virtio GPIO request types */
#define VIRTIO_GPIO_MSG_GET_NAMES		0x0001
#define VIRTIO_GPIO_MSG_GET_DIRECTION		0x0002
#define VIRTIO_GPIO_MSG_SET_DIRECTION		0x0003
#define VIRTIO_GPIO_MSG_GET_VALUE		0x0004
#define VIRTIO_GPIO_MSG_SET_VALUE		0x0005

/* Possible values of the status field */
#define VIRTIO_GPIO_STATUS_OK			0x0
#define VIRTIO_GPIO_STATUS_ERR			0x1

/* Direction types */
#define VIRTIO_GPIO_DIRECTION_NONE		0x00
#define VIRTIO_GPIO_DIRECTION_OUT		0x01
#define VIRTIO_GPIO_DIRECTION_IN		0x02

// #define DEBUG

struct virtio_gpio_config {
	uint16_t ngpio;
	uint8_t padding[2];
	uint32_t gpio_names_size;
} __packed;

/* Virtio GPIO Request / Response */
struct virtio_gpio_request {
	uint16_t type;
	uint16_t gpio;
	uint32_t value;
};

struct virtio_gpio_response {
	uint8_t status;
	uint8_t value;
};

struct virtio_gpio_response_get_names {
	uint8_t status;
	uint8_t value[];
};

/* ------------------------------------------------------------ */
static const char *gpio_name = "vgpio";
static int gpio_response_get_name(void *resp, uint32_t len)
{
	int i, n;
	uint8_t *p;
	struct virtio_gpio_response_get_names *r = resp;
	uint32_t ngpio = gpio_max_line();

	r->status = VIRTIO_GPIO_STATUS_OK;
	p = r->value;

	for (i = 0; i < ngpio; i++) {
		n = sprintf((char *)p, "%s%02u", gpio_name, i);
		p += n;
		*p++ = '\0';
	}

	return p - r->value + sizeof(r->status);
}

#if defined(DEBUG)
static const char *gpio_type_name(uint16_t type)
{
	const char *names[] = {"rsvd", "get_name", "get_dir", "set_dir", "get_val", "set_val"};

	if (type > VIRTIO_GPIO_MSG_SET_VALUE) {
		return "none";
	}
	return names[type];
}
#endif

static int gpio_virtio_cb(void *binding, struct virtio_pkt *pkt, uint32_t num)
{
	uint16_t dir;
	struct virtio_gpio_request *req;
	struct virtio_gpio_response *res;

	if (num != 2) {
		return -1;
	}

	req = pkt[0].data;
	res = pkt[1].data;

	switch (req->type) {
	case VIRTIO_GPIO_MSG_GET_NAMES:
		return gpio_response_get_name(pkt[1].data, pkt[1].len);
	case VIRTIO_GPIO_MSG_GET_DIRECTION:
		dir = gpio_g_dir(req->gpio);
		res->value = (dir == 1 ? VIRTIO_GPIO_DIRECTION_OUT : VIRTIO_GPIO_DIRECTION_IN);
		break;
	case VIRTIO_GPIO_MSG_SET_DIRECTION:
		if (req->value != VIRTIO_GPIO_DIRECTION_NONE) {
			dir = (req->value == VIRTIO_GPIO_DIRECTION_OUT ? 1 : 0);
			res->value = gpio_s_dir(req->gpio, dir);
		}
		break;
	case VIRTIO_GPIO_MSG_GET_VALUE:
		res->value = gpio_get(req->gpio);
		break;
	case VIRTIO_GPIO_MSG_SET_VALUE:
		if (req->value)
			res->value = gpio_set(req->gpio);
		else
			res->value = gpio_clr(req->gpio);
		break;
	}

#if defined(DEBUG)
	printf("gpio%u: %s, %u\n", req->gpio, gpio_type_name(req->type), req->value);
#endif
	res->status = VIRTIO_GPIO_STATUS_OK;

	return num;
}

static struct virtio_gpio_config gpio_config = {
	.ngpio = 0,
	.gpio_names_size = 0,
};
/*
 * Typical device tree node for gpio-virtio, eg.:
 *
 * virtio@2148000 {
 *            compatible = "virtio,mmio";
 *            reg = <0x0 0x2148000 0x0 0x1000>;
 *            interrupts = <0 70 0x1>;
 *            dma-ranges = <0 0 0 0 0 0x7FD00000>;
 *
 *            amb,irq-regmap = <&ca78ae_scratchpad_syscon 0x1000>;
 *            amb,irq-notify = <0x10 1 1>; // offset mask value
 *            amb,irq-clear = <>; // offset mask value
 *            gpio0 {
 *                    compatible = "virtio,device29";
 *                    gpio-controller;
 *                    #gpio-cells = <2>;
 *            };
 *    };
 *
 */
static struct virtio_mmio_device gpio_dev = {
	.name = "gpio-virtio",
	.base = VIRTIO_BASE(VIRTIO_ID_GPIO, 0),
	.device_id = VIRTIO_ID_GPIO,
	.vendor_id = 0xffffffff,
	.irq[VIRTIO_IRQ_DEVICE_TO_DRIVER] = 0,
	.irq[VIRTIO_IRQ_DRIVER_TO_DEVICE] = VIRTIO_DEVICE_IRQ0,
	.callback = gpio_virtio_cb,
	.config_ptr = &gpio_config,
	.config_size = sizeof(gpio_config)
};

static void virtio_gpio_task(void *arg)
{
	if (virtio_mmio_device_register(arg))
		printf("Failed to register virtio-mmio: gpio\n");

	vTaskDelete(NULL);
}

static StackType_t stack[VIRTIO_TASK_STACK_DEPTH];
/* ------------------------------------------------------------ */
void virtio_gpio_init(void)
{
	TaskHandle_t ret;
	struct virtio_mmio_device *dev = &gpio_dev;

	gpio_config.ngpio = gpio_max_line();
	gpio_config.gpio_names_size = gpio_config.ngpio * (strlen(gpio_name) + 2 + 1);

	dev->stack = stack;
	ret = xTaskCreateStatic(virtio_gpio_task, dev->name, VIRTIO_TASK_STACK_DEPTH,
				dev, VIRTIO_TASK_PRIOR, dev->stack,&dev->tcb);
	if (!ret) {
		printf("Failed to create task %s\n", dev->name);
	}
}
