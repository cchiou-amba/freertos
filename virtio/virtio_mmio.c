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
#include <common.h>
#include <plat.h>
#include "virtio.h"

//#define DEBUG

#define VIRTIO_F_ACCESS_PLATFORM	33

#define VRING_DESC_F_NEXT		1
#define VRING_DESC_F_WRITE		2
#define VRING_DESC_F_INDIRECT		4

#define VIRTIO_CONFIG_S_ACKNOWLEDGE	1
#define VIRTIO_CONFIG_S_DRIVER		2
#define VIRTIO_CONFIG_S_DRIVER_OK	4
#define VIRTIO_CONFIG_S_FEATURES_OK	8
#define VIRTIO_CONFIG_S_NEEDS_RESET	0x40
#define VIRTIO_CONFIG_S_FAILED		0x80

#define VIRTIO_MMIO_CONFIG		0x100

#define VIRTIO_MMIO_INT_VRING		(1 << 0)
#define VIRTIO_MMIO_INT_CONFIG		(1 << 1)

#define MMIO_LIMIT_ADDR	0x7FD00000

#if defined(DEBUG)
#define DBG_FIELD(ptr,field)		printf("\t" #ptr"->"#field ": 0x%x\n", (uint32_t)(ptr)->field)
#else
#define DBG_FIELD(ptr, field)		do {} while(0)
#endif

#if defined(DEBUG)
#define mmio_debug(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#else
#define mmio_debug(fmt, ...)		do {} while(0)
#endif

struct virtio_mmio_hdr {
	char	magic[4];
	uint32_t	version;
	uint32_t	device_id;
	uint32_t	vendor_id;
	uint32_t	host_features;
	uint32_t	host_features_sel;
	uint32_t	reserved_1[2];
	uint32_t	guest_features;
	uint32_t	guest_features_sel;
	uint32_t	guest_page_size;	/* legacy */
	uint32_t	reserved_2;
	uint32_t	queue_sel;
	uint32_t	queue_num_max;
	uint32_t	queue_num;
	uint32_t	queue_align;		/* legacy */
	uint32_t	queue_pfn;		/* legacy */
	uint32_t	queue_ready;		/* modern */
	uint32_t	reserved_3[2];
	uint32_t	queue_notify;
	uint32_t	reserved_4[3];
	uint32_t	interrupt_state;
	uint32_t	interrupt_ack;
	uint32_t	reserved_5[2];
	uint32_t	status;
	uint32_t	reserved_7[3];
	uint32_t	queue_desc_low;		/* modern */
	uint32_t	queue_desc_high;
	uint32_t	reserved_8[2];
	uint32_t	queue_avail_low;
	uint32_t	queue_avail_high;
	uint32_t	reserved_9[2];
	uint32_t	queue_used_low;
	uint32_t	queue_used_high;
} __attribute__((packed));

struct vring_desc {
	uint64_t addr;
	uint32_t len;
	uint16_t flags;
	uint16_t next;
};

struct vring_avail {
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[0];
};

struct vring_used_elem {
	union {
		uint16_t event;
		uint32_t id;
	};
	uint32_t len;
};

struct vring_used {
	uint16_t flags;
	uint16_t idx;
	struct vring_used_elem ring[0];
};

struct vring {
	uint32_t num;
	struct vring_desc *desc;
	struct vring_avail *avail;
	struct vring_used *used;
};

static void mmio_isr(void *args)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	struct virtio_mmio_device *dev = (struct virtio_mmio_device *)args;
	xEventGroupSetBitsFromISR(dev->evt, 1, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

static void send_irq(uint32_t irq)
{

#define R52_SP_BASE		0xFFF00000
#define R52_SP_REG(x)		(R52_SP_BASE + (x))
#define R52_CA78_IRQ_REG	R52_SP_REG(0x4)
#define R52_CA78_IRQ_NUM	8
	configASSERT(irq < 8);

	writel(R52_CA78_IRQ_REG,  1U << irq);
}

static void notify_driver(struct virtio_mmio_hdr *mmio, uint32_t irq)
{
	mmio->interrupt_state = VIRTIO_MMIO_INT_VRING;

	send_irq(irq);
}

static int check_status(struct virtio_mmio_hdr *mmio, uint32_t status)
{

	if ((mmio->status & status) != status)
		return -1;

	return 0;
}

static int setup_vq(struct virtio_mmio_hdr *mmio, struct vring *vr)
{
	uint32_t n = mmio->queue_num;
	uint32_t align = mmio->queue_align;
	uintptr_t vring_base = mmio->queue_pfn * mmio->queue_align;

	DBG_FIELD(mmio, queue_pfn);
	DBG_FIELD(mmio, queue_align);

	if (!align || !n) {
		return -1;
	}

	if (mmio->queue_pfn >= (MMIO_LIMIT_ADDR / align)) {
		printf("memory space for virtio_mmio must not be above 0x%x (queue_pfn = 0x%x)\n",
		       MMIO_LIMIT_ADDR, mmio->queue_pfn);
		return -1;
	}

	vr->num = n;
	vr->desc = (void *)vring_base;
	vr->avail = (struct vring_avail *)&vr->desc[n];
	vr->used = (struct vring_used *)(((uintptr_t)&vr->avail->ring[n] +
					  sizeof(uint16_t) + align - 1) & ~(align -1));

	DBG_FIELD(vr, desc);
	DBG_FIELD(vr, avail);
	DBG_FIELD(vr, used);

	return 0;
}

static void mmio_listen(struct virtio_mmio_device *dev, struct vring *vr)
{
	int ret;
	uint32_t used_idx, desc_idx, avail_idx;
	struct virtio_mmio_hdr *mmio = (struct virtio_mmio_hdr *)dev->base;
	struct vring_desc *vdesc;
	struct virtio_pkt pkt[VIRTIO_MSG_MAX];
	uint32_t n;

	do {
		EventBits_t wbit;

		wbit = xEventGroupWaitBits(dev->evt,
					   1,
					   pdTRUE,
					   pdTRUE,
					   ~0);
		if (wbit != 1) {
			pr_err("xEventGroupWaitBits: hit error\n");
			break;
		}

		mmio_debug("Message(s):\n");
		DBG_FIELD(vr->avail, flags);
		DBG_FIELD(vr->avail, idx);
		DBG_FIELD(vr->used, flags);
		DBG_FIELD(vr->used, idx);
		mmio_debug("\n");

		used_idx = vr->used->idx;
		avail_idx = vr->avail->idx;

		if (used_idx > avail_idx) {
			avail_idx += 0x10000;	// fix u16 overflow
		}

		desc_idx = vr->avail->ring[used_idx % vr->num] % vr->num;
		vdesc = &vr->desc[desc_idx];
		n = 0;
		while (used_idx < avail_idx) {
			if (vdesc->addr > MMIO_LIMIT_ADDR) {
				pr_err("protocol error: desc addr 0x%08x%08x is above limit 0x%08x\n",
				       (uint32_t)(vdesc->addr >> 32), (uint32_t)vdesc->addr, MMIO_LIMIT_ADDR);
				break;
			}
			pkt[n].data = (void *)(uintptr_t)vdesc->addr;
			pkt[n].len = vdesc->len;
			pkt[n].is_write = (vdesc->flags & VRING_DESC_F_WRITE) ? 1 : 0;
			mmio_debug("[%u]:\n", n);
			DBG_FIELD(vdesc, addr);
			DBG_FIELD(vdesc, len);
			DBG_FIELD(vdesc, flags);
			DBG_FIELD(vdesc, next);

			n++;
			if (vdesc->flags & VRING_DESC_F_NEXT) {
				vdesc = &vr->desc[vdesc->next];
				BUG_ON(n > VIRTIO_MSG_MAX, "-EIO");
				continue;
			} else {
				/* submit messge(s) */
				ret = dev->callback(dev->binding, pkt, n);
			}
			n = 0;
			vr->used->ring[used_idx % vr->num].id =
				vr->avail->ring[used_idx % vr->num];
			vr->used->ring[used_idx % vr->num].len = ret;
			vr->used->idx++;
			used_idx++;

			desc_idx = vr->avail->ring[used_idx % vr->num] % vr->num;
			vdesc = &vr->desc[desc_idx];
		}

		notify_driver(mmio, dev->irq[VIRTIO_IRQ_DEVICE_TO_DRIVER]);
	} while(1);
}

int virtio_mmio_device_register(struct virtio_mmio_device *dev)
{
	int ret;
	struct vring vr;
	struct virtio_mmio_hdr *mmio = (struct virtio_mmio_hdr *)dev->base;

	if (!dev || !dev->name) {
		pr_err("%s: invalid args\n", __func__);
		return -1;
	}

	printf("mmio:\n");
	printf("\tname: %s\n", dev->name);
	printf("\tbase: 0x%08x\n", dev->base);
	printf("\t irq: notify(%u), local(%u)\n",
	       dev->irq[VIRTIO_IRQ_DEVICE_TO_DRIVER],
	       dev->irq[VIRTIO_IRQ_DRIVER_TO_DEVICE]);

	memset(mmio, 0, sizeof(*mmio));

	mmio->magic[0] = 'v';
	mmio->magic[1] = 'i';
	mmio->magic[2] = 'r';
	mmio->magic[3] = 't';

	mmio->version = 1;
	mmio->device_id = dev->device_id;
	mmio->vendor_id = dev->vendor_id;
	mmio->queue_num_max = 16;
	mmio->queue_notify = 0;
	mmio->host_features = 1 << (VIRTIO_F_ACCESS_PLATFORM % 32) | dev->features;		/* Ask Linux Using DMA */

	if (dev->config_ptr) {
		memcpy((void *)dev->base + VIRTIO_MMIO_CONFIG, dev->config_ptr, dev->config_size);
	}

	dev->evt = xEventGroupCreateStatic(&dev->evt_buffer);
	ret = request_irq(dev->irq[VIRTIO_IRQ_DRIVER_TO_DEVICE], IRQ_RISING_EDGE, mmio_isr,
			  dev, dev->name);
	if (ret) {
		pr_err("request irq%u failed\n", dev->irq[VIRTIO_IRQ_DRIVER_TO_DEVICE]);
		return ret;
	}

	xEventGroupWaitBits(dev->evt,
			    1,
			    pdFALSE,
			    pdTRUE,
			    ~0);
	ret = check_status(mmio, VIRTIO_CONFIG_S_DRIVER_OK);
	if (ret) {
		pr_err("driver is not ok\n");
		return ret;
	}

	ret = setup_vq(mmio, &vr);
	if (ret)
		return ret;
	mmio_listen(dev, &vr);

	return 0;
}
