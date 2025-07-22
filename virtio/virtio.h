/**
 * virtio.h
 *
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

#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include <FreeRTOS.h>
#include <event_groups.h>
#include "virtio_config.h"

#define VIRTIO_MSG_MAX		8
struct virtio_pkt {
	void *data;
	uint32_t len;
	uint32_t is_write;
};

struct virtio_mmio_device {
	char *name;
	uintptr_t base;
	uint32_t device_id;
	uint32_t vendor_id;
#define VIRTIO_IRQ_DEVICE_TO_DRIVER		0
#define VIRTIO_IRQ_DRIVER_TO_DEVICE		1
	uint32_t irq[2];
	int (*callback)(void *binding, struct virtio_pkt *pkt, uint32_t num);
 	EventGroupHandle_t evt;
	StaticEventGroup_t evt_buffer;

	StackType_t *stack;
	StaticTask_t tcb;

	void *config_ptr;
	uint32_t config_size;
	void *binding;
};

int virtio_mmio_device_register(struct virtio_mmio_device *dev);

#endif /* __VIRTIO_H__ */
