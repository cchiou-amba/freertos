/**
 * virtio.h
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
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
	uint32_t features;
};

int virtio_mmio_device_register(struct virtio_mmio_device *dev);

#endif /* __VIRTIO_H__ */
