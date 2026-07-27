/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include "rpmsg_lite.h"
#include "FreeRTOS.h"
#include "rpmsg.h"
#include <plat.h>

#define RSC_TABLE_SIZE                  (0x1000)
#define RSC_TABLE_ADDR                  (CM3_DRAM_ADDRESS + CONFIG_CM3_REMAP_SIZE - RSC_TABLE_SIZE)

#define RSC_NUM                         1
#define VRING_NUM                       2

typedef struct ambarella_resource_table {
    struct resource_table base;
    u32 offset[RSC_NUM];

    struct fw_rsc_hdr hdr;
    struct fw_rsc_vdev vdev;
    struct fw_rsc_vdev_vring vring0;
    struct fw_rsc_vdev_vring vring1;
} __packed ambarella_resource_table_t;

const ambarella_resource_table_t rsc_table __section(".resource_table") = {
    /*.base = */{
        1,
        RSC_NUM,
        {0, 0},
    },
    /*.offset[] = */{
        offsetof(ambarella_resource_table_t, hdr),
    },
    /*.hdr = */{
        RSC_VDEV,
    },
    /*.vdev = */{
        VIRTIO_ID_RPMSG,
        0, 1, 0, 0, 0, VRING_NUM, {0, 0},
    },
    /*.vring0 = */{
        FW_RSC_ADDR_ANY, VRING_ALIGN, RL_BUFFER_COUNT, 0, 0
    },
    /*.vring1 = */{
        FW_RSC_ADDR_ANY, VRING_ALIGN, RL_BUFFER_COUNT, 1, 0
    }
};

__attribute__((weak)) void test_cm3_rpmsg(void) { ; }

void rpmsg_lite_remote(void)
{
#ifndef CONFIG_CM3_DELAYBOOT
    (void)env_memcpy((void *)(RSC_TABLE_ADDR), &rsc_table, sizeof(rsc_table));
#endif

    test_cm3_rpmsg();
}
