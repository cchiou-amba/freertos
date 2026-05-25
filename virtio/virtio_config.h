/**
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 */
#ifndef __VIRTO_CONFIG_H__
#define __VIRTO_CONFIG_H__

#include <FreeRTOS.h>
#include <task.h>

#if !defined(VIRTIO_SHMEM_START)
#define VIRTIO_SHMEM_BASE		0x02000000
#else
#define VIRTIO_SHMEM_BASE		(VIRTIO_SHMEM_START)
#endif
#define VIRTIO_SHMEM_SIZE		0x00200000
/* id < 64 && n < 8*/
#define VIRTIO_BASE(id,n)		(VIRTIO_SHMEM_BASE + (id) * 0x8000 + (n) * 0x1000)

#define VIRTIO_TASK_STACK_DEPTH		(configMINIMAL_STACK_SIZE * 2)
#define VIRTIO_TASK_PRIOR		(tskIDLE_PRIORITY + 1)

#define VIRTIO_DEVICE_IRQ0		(523 + 32)
#define VIRTIO_DEVICE_IRQ1		(524 + 32)
#define VIRTIO_DEVICE_IRQ2		(525 + 32)
#define VIRTIO_DEVICE_IRQ3		(526 + 32)
#define VIRTIO_DEVICE_IRQ4		(527 + 32)
#define VIRTIO_DEVICE_IRQ5		(528 + 32)
#define VIRTIO_DEVICE_IRQ6		(529 + 32)
#define VIRTIO_DEVICE_IRQ7		(530 + 32)

#endif	/*__VIRTO_CONFIG_H__ */
