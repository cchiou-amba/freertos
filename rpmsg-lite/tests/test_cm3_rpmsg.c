/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"
#include "plat.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RPMSG_LITE_SHMEM_BASE           (CM3_DRAM_ADDRESS)
#define RPMSG_LITE_LINK_ID              (0) /* (0 << 1 | queue_id), queue_id = Interrupt0_IRQn or Interrupt1_IRQn */

#define LOCAL_EPT_ADDR                  (1)
#define REMOTE_EPT_ADDR                 (1024)

#define RPMSGRECV_TASK_STACK_SIZE       configMINIMAL_STACK_SIZE
#define RPMSGRECV_TASK_PRIORITY	        tskIDLE_PRIORITY + 3

typedef struct {
    uint32_t loop;
    uint32_t payload_sz;
} Msg_t;

static StackType_t  rpmsgRecv_stack_buf[RPMSGRECV_TASK_STACK_SIZE];
static StaticTask_t rpmsgRecv_task_buf;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static struct rpmsg_lite_instance *volatile my_rpmsg = NULL;
static struct rpmsg_lite_instance rpmsg_ctxt = {0};
static struct rpmsg_lite_endpoint *volatile my_ept = NULL;
static volatile rpmsg_queue_handle my_queue = NULL;

static rpmsg_static_queue_ctxt queue_ctxt;
static uint8_t queue_storage[RL_ENV_QUEUE_STATIC_STORAGE_SIZE];
static struct rpmsg_lite_ept_static_context ept_context;

static uint32_t remote_addr = 0;
static Msg_t msg = {0};

int32_t init_rpmsg(void)
{
    env_init();
    printf("RPMSG Share Base Addr is 0x%x\r\n", RPMSG_LITE_SHMEM_BASE);
    my_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS, &rpmsg_ctxt);
    rpmsg_lite_wait_for_link_up(my_rpmsg, RL_BLOCK);
    printf("Link is up!\r\n");

    my_queue = rpmsg_queue_create(my_rpmsg, queue_storage, &queue_ctxt);
    my_ept   = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue, &ept_context);

    return 0;
}

int32_t deinit_rpmsg(void)
{
    (void)rpmsg_queue_destroy(my_rpmsg, my_queue);
    my_queue = RL_NULL;
    (void)rpmsg_lite_destroy_ept(my_rpmsg, my_ept);
    my_ept = RL_NULL;

    rpmsg_lite_deinit(my_rpmsg);
    my_rpmsg = NULL;
    env_memset(&rpmsg_ctxt, 0, sizeof(struct rpmsg_lite_instance));
    return 0;
}

static void recv_task(void* params)
{
    TaskHandle_t parentTaskH = (TaskHandle_t)params;
    TickType_t start, stop;
    void *data_addr = NULL;
    uint32_t len;

    start = xTaskGetTickCount();
    for (uint32_t i = 0; i < msg.loop; i++)
    {
        (void)rpmsg_queue_recv_nocopy(my_rpmsg, my_queue, RL_NULL,
                                      (char **)&data_addr, &len, RL_BLOCK);
        (void)rpmsg_queue_nocopy_free(my_rpmsg, data_addr);
    }
    stop = xTaskGetTickCount();
    printf("recv task done, start=%lu, stop=%lu, exit...\r\n", start, stop);
    xTaskNotify(parentTaskH, 0, eNoAction);

    vTaskDelete(NULL);
}

void test_cm3_rpmsg(void)
{
    int32_t ret_value = 0;
    void *data_addr = NULL;
    uint32_t num;
    uint32_t buf_size = 0;

    ret_value = init_rpmsg();
    if (ret_value)
        goto end;

    while (1) {
        (void)rpmsg_queue_recv(my_rpmsg, my_queue, &remote_addr, (char *)&msg, sizeof(msg), &num, RL_BLOCK);
        printf("loop=%lu, payload_sz=%lu\r\n", msg.loop, msg.payload_sz);
        (void)rpmsg_lite_send(my_rpmsg, my_ept, remote_addr, (char *)&msg, sizeof(msg), RL_BLOCK);

        (void)xTaskCreateStatic(recv_task, "rpmsg_recv", RPMSGRECV_TASK_STACK_SIZE, xTaskGetCurrentTaskHandle(),
                                RPMSGRECV_TASK_PRIORITY, rpmsgRecv_stack_buf, &rpmsgRecv_task_buf);
        /* wait for recv task to finish first */
        xTaskNotifyWait(0x00,      /* Don't clear any notification bits on entry. */
                        ULONG_MAX, /* Reset the notification value to 0 on exit. */
                        NULL, /* Notified value pass out in ulNotifiedValue. */
                        RL_BLOCK);

        /* no copy */
        for (uint32_t i = 0; i < msg.loop; i++) {
            data_addr = rpmsg_lite_alloc_tx_buffer(my_rpmsg, &buf_size, RL_BLOCK);
            if (buf_size == 0 || data_addr == NULL) {
                printf("negative number\n");
                break;
            }
            /* env_memset(data_addr, i, msg.payload_sz); */
            rpmsg_lite_send_nocopy(my_rpmsg, my_ept, remote_addr, (char *)data_addr, msg.payload_sz);
            data_addr = NULL;
        }
    }

end:
    ret_value = deinit_rpmsg();
}
