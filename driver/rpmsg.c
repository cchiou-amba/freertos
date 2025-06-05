/*******************************************************************************
 *
 * Copyright (c) 2023 Ambarella International LP
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
#ifdef CONFIG_BOOT_CM3
    (void)env_memcpy((void *)(RSC_TABLE_ADDR), &rsc_table, sizeof(rsc_table));
#endif

    test_cm3_rpmsg();
}
