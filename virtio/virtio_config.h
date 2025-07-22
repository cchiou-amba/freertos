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
