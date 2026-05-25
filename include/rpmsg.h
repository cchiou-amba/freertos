/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __AMBA_RPMSG_H__
#define __AMBA_RPMSG_H__

typedef uint32_t u32;
typedef uint8_t u8;

/* include/uapi/linux/virtio_ids.h */
#define VIRTIO_ID_RPMSG			7 /* virtio remote processor messaging */

/* include/linux/remoteproc.h */
struct resource_table {
	u32 ver;
	u32 num;
	u32 reserved[2];
	u32 offset[];
} __packed;

struct fw_rsc_hdr {
	u32 type;
	u8 data[];
} __packed;

enum fw_resource_type {
	RSC_CARVEOUT		= 0,
	RSC_DEVMEM		= 1,
	RSC_TRACE		= 2,
	RSC_VDEV		= 3,
	RSC_LAST		= 4,
	RSC_VENDOR_START	= 128,
	RSC_VENDOR_END		= 512,
};

#define FW_RSC_ADDR_ANY (-1)

struct fw_rsc_vdev_vring {
	u32 da;
	u32 align;
	u32 num;
	u32 notifyid;
	u32 pa;
} __packed;

struct fw_rsc_vdev {
	u32 id;
	u32 notifyid;
	u32 dfeatures;
	u32 gfeatures;
	u32 config_len;
	u8 status;
	u8 num_of_vrings;
	u8 reserved[2];
	struct fw_rsc_vdev_vring vring[];
} __packed;

/* RPMsg-Lite */
extern void rpmsg_lite_remote(void);

#endif /* __CM3_RPMSG_H__ */
