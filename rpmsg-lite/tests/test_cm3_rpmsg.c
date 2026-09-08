/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-3-Clause
 *
 ******************************************************************************/

#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"
#include "plat.h"
#include "cmvi.h"
#include "cm3_rpmsg_protocol.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RPMSG_LITE_SHMEM_BASE           (CM3_DRAM_ADDRESS)
#define RPMSG_LITE_LINK_ID              (0)

#define LOCAL_EPT_ADDR                  (1)
#define REMOTE_EPT_ADDR                 (1024)

#define RPMSGRECV_TASK_STACK_SIZE       configMINIMAL_STACK_SIZE
#define RPMSGRECV_TASK_PRIORITY         (tskIDLE_PRIORITY + 3)

#define RX_BUF_SIZE                     CM3_RPMSG_MAX_FRAME
#define TX_BUF_SIZE                     CM3_RPMSG_MAX_FRAME

/*******************************************************************************
 * Variables
 ******************************************************************************/
static StackType_t  rpmsgRecv_stack_buf[RPMSGRECV_TASK_STACK_SIZE];
static StaticTask_t rpmsgRecv_task_buf;

static struct rpmsg_lite_instance *volatile my_rpmsg = NULL;
static struct rpmsg_lite_instance rpmsg_ctxt = {0};
static struct rpmsg_lite_endpoint *volatile my_ept = NULL;
static volatile rpmsg_queue_handle my_queue = NULL;

static rpmsg_static_queue_ctxt queue_ctxt;
static uint8_t queue_storage[RL_ENV_QUEUE_STATIC_STORAGE_SIZE];
static struct rpmsg_lite_ept_static_context ept_context;

static uint32_t remote_addr = 0;
static cm3_stress_param_t stress_msg;
static uint8_t rx_buf[RX_BUF_SIZE];
static uint8_t tx_buf[TX_BUF_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/
int32_t init_rpmsg(void)
{
	env_init();
	printf("RPMSG Share Base Addr is 0x%x\r\n", RPMSG_LITE_SHMEM_BASE);
	my_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS, &rpmsg_ctxt);
	rpmsg_lite_wait_for_link_up(my_rpmsg, RL_BLOCK);
	printf("Link is up! proto v%u\r\n", (unsigned)CM3_RPMSG_VERSION);

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

static int32_t send_rsp(uint16_t cmd, uint16_t seq, uint16_t delay_ms,
						int32_t status, const void *payload, uint16_t payload_len)
{
	uint8_t flags = CM3_FLG_RSP;
	int frame_len;

	if (status != CM3_STATUS_OK) {
		flags |= CM3_FLG_ERR;
	}

	/* RSP echoes delay_ms from REQ; default 0 when unused */
	frame_len = cm3_rpmsg_build_frame(tx_buf, TX_BUF_SIZE, cmd, flags, seq,
									  status, delay_ms, payload, payload_len);
	if (frame_len < 0) {
		return frame_len;
	}

	return rpmsg_lite_send(my_rpmsg, my_ept, remote_addr,
						   (char *)tx_buf, (uint32_t)frame_len, RL_BLOCK);
}

static uint32_t apply_req_delay(uint16_t delay_ms)
{
	TickType_t t0, t1;
	TickType_t ticks;

	if (delay_ms == 0u) {
		return 0;
	}
	if (delay_ms > CM3_RPMSG_DELAY_MS_MAX) {
		delay_ms = (uint16_t)CM3_RPMSG_DELAY_MS_MAX;
	}

	ticks = pdMS_TO_TICKS(delay_ms);
	if (ticks == 0u) {
		ticks = 1;
	}

	printf("apply delay_ms=%u (ticks=%lu)\r\n",
		   (unsigned)delay_ms, (unsigned long)ticks);

	t0 = xTaskGetTickCount();
	vTaskDelay(ticks);
	t1 = xTaskGetTickCount();
	return (uint32_t)((t1 - t0) * portTICK_PERIOD_MS);
}

static void recv_task(void *params)
{
	TaskHandle_t parentTaskH = (TaskHandle_t)params;
	TickType_t start, stop;
	void *data_addr = NULL;
	uint32_t len;
	uint32_t i;

	start = xTaskGetTickCount();
	for (i = 0; i < stress_msg.loop; i++) {
		(void)rpmsg_queue_recv_nocopy(my_rpmsg, my_queue, RL_NULL,
									  (char **)&data_addr, &len, RL_BLOCK);
		(void)rpmsg_queue_nocopy_free(my_rpmsg, data_addr);
	}
	stop = xTaskGetTickCount();
	printf("stress recv done, start=%lu, stop=%lu\r\n",
		   (unsigned long)start, (unsigned long)stop);
	xTaskNotify(parentTaskH, 0, eNoAction);

	vTaskDelete(NULL);
}

static void handle_stress(const cm3_rpmsg_hdr_t *hdr,
						  const uint8_t *payload, uint16_t payload_len)
{
	void *data_addr = NULL;
	uint32_t buf_size = 0;
	uint32_t i;

	if (payload == NULL || payload_len != sizeof(cm3_stress_param_t)) {
		(void)send_rsp(CM3_CMD_STRESS, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_EINVAL, NULL, 0);
		return;
	}

	cm3_rpmsg_memcpy(&stress_msg, payload, sizeof(stress_msg));
	if (stress_msg.loop == 0u || stress_msg.payload_sz == 0u ||
		stress_msg.payload_sz > CM3_RPMSG_MAX_FRAME) {
		(void)send_rsp(CM3_CMD_STRESS, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_EINVAL, NULL, 0);
		return;
	}

	printf("STRESS loop=%lu, payload_sz=%lu\r\n",
		   (unsigned long)stress_msg.loop, (unsigned long)stress_msg.payload_sz);

	/* ACK then enter raw data phase */
	(void)send_rsp(CM3_CMD_STRESS, hdr->seq, hdr->delay_ms,
				   CM3_STATUS_OK, payload, payload_len);

	(void)xTaskCreateStatic(recv_task, "rpmsg_recv", RPMSGRECV_TASK_STACK_SIZE,
							xTaskGetCurrentTaskHandle(), RPMSGRECV_TASK_PRIORITY,
							rpmsgRecv_stack_buf, &rpmsgRecv_task_buf);

	xTaskNotifyWait(0x00, ULONG_MAX, NULL, RL_BLOCK);

	for (i = 0; i < stress_msg.loop; i++) {
		data_addr = rpmsg_lite_alloc_tx_buffer(my_rpmsg, &buf_size, RL_BLOCK);
		if (buf_size == 0u || data_addr == NULL) {
			printf("alloc tx buffer failed\r\n");
			break;
		}
		if (stress_msg.payload_sz > buf_size) {
			printf("payload_sz %lu > buf %lu\r\n",
				   (unsigned long)stress_msg.payload_sz, (unsigned long)buf_size);
			break;
		}
		(void)rpmsg_lite_send_nocopy(my_rpmsg, my_ept, remote_addr,
									 (char *)data_addr, stress_msg.payload_sz);
		data_addr = NULL;
	}
}

static void handle_get_info(const cm3_rpmsg_hdr_t *hdr)
{
	cm3_info_rsp_t info;
	const char *name = "cm3-freertos";
	uint32_t i;

	env_memset(&info, 0, sizeof(info));
	info.proto_ver = CM3_RPMSG_VERSION;
	info.local_ept = LOCAL_EPT_ADDR;
	info.tick_count = (uint32_t)xTaskGetTickCount();
	for (i = 0; i < sizeof(info.name) - 1u && name[i] != '\0'; i++) {
		info.name[i] = name[i];
	}
	info.name[i] = '\0';

	(void)send_rsp(CM3_CMD_GET_INFO, hdr->seq, hdr->delay_ms,
				   CM3_STATUS_OK, &info, sizeof(info));
}

static void handle_wakeup_auto_cfg(const cm3_rpmsg_hdr_t *hdr,
								   const uint8_t *payload, uint16_t payload_len)
{
	cm3_wakeup_auto_cfg_t cfg;

	if (payload == NULL || payload_len != sizeof(cm3_wakeup_auto_cfg_t)) {
		(void)send_rsp(CM3_CMD_WAKEUP_AUTO_CFG, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_EINVAL, NULL, 0);
		return;
	}

	cm3_rpmsg_memcpy(&cfg, payload, sizeof(cfg));
	if (cfg.delay_ms > CM3_RPMSG_DELAY_MS_MAX) {
		(void)send_rsp(CM3_CMD_WAKEUP_AUTO_CFG, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_EINVAL, NULL, 0);
		return;
	}

	/* enable SRS auto-wakeup with configured delay */
	if (cfg.delay_ms == CM3_RPMSG_DELAY_MS_MAX) {
		cmvi_srs_auto_wkup_cfg(0u, cfg.delay_ms);
	} else {
	   cmvi_srs_auto_wkup_cfg(1u, cfg.delay_ms);
	}
	(void)send_rsp(CM3_CMD_WAKEUP_AUTO_CFG, hdr->seq, hdr->delay_ms,
				   CM3_STATUS_OK, &cfg, sizeof(cfg));
}

static void handle_command(const cm3_rpmsg_hdr_t *hdr, const uint8_t *payload)
{
	if ((hdr->flags & CM3_FLG_REQ) == 0u) {
		printf("drop non-REQ frame cmd=0x%x\r\n", hdr->cmd);
		return;
	}

	if (hdr->delay_ms > CM3_RPMSG_DELAY_MS_MAX) {
		(void)send_rsp(hdr->cmd, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_EINVAL, NULL, 0);
		return;
	}

	/* delay_ms==0: no-op; otherwise vTaskDelay before command body */
	(void)apply_req_delay(hdr->delay_ms);

	switch (hdr->cmd) {
	case CM3_CMD_PING:
		printf("Get ping\r\n");
		(void)send_rsp(CM3_CMD_PING, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_OK, payload, hdr->payload_len);
		break;

	case CM3_CMD_ECHO:
		printf("Get string: %s\r\n", payload);
		(void)send_rsp(CM3_CMD_ECHO, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_OK, payload, hdr->payload_len);
		break;

	case CM3_CMD_GET_INFO:
		handle_get_info(hdr);
		break;

	case CM3_CMD_STRESS:
		handle_stress(hdr, payload, hdr->payload_len);
		break;

	case CM3_CMD_WAKEUP_AUTO_CFG:
		handle_wakeup_auto_cfg(hdr, payload, hdr->payload_len);
		break;

	default:
		(void)send_rsp(hdr->cmd, hdr->seq, hdr->delay_ms,
					   CM3_STATUS_ENOSYS, NULL, 0);
		break;
	}
}

void test_cm3_rpmsg(void)
{
	int32_t ret_value;
	uint32_t num;
	cm3_rpmsg_hdr_t hdr;
	const uint8_t *payload = NULL;
	int parse_ret;

	ret_value = init_rpmsg();
	if (ret_value) {
		goto end;
	}

	printf("CM3 RPMSG command server ready (ept=%u)\r\n", LOCAL_EPT_ADDR);

	while (1) {
		num = 0;
		memset(rx_buf, 0, RX_BUF_SIZE);
		(void)rpmsg_queue_recv(my_rpmsg, my_queue, &remote_addr,
							   (char *)rx_buf, sizeof(rx_buf), &num, RL_BLOCK);

		parse_ret = cm3_rpmsg_parse_frame(rx_buf, num, &hdr, &payload);
		if (parse_ret != CM3_STATUS_OK) {
			printf("bad frame len=%lu ret=%d\r\n", (unsigned long)num, parse_ret);
			/* best-effort error RSP if magic/cmd/seq are readable */
			if (num >= sizeof(cm3_rpmsg_hdr_t)) {
				cm3_rpmsg_hdr_t *raw = (cm3_rpmsg_hdr_t *)rx_buf;
				if (raw->magic == CM3_RPMSG_MAGIC && (raw->flags & CM3_FLG_ACK_REQ)) {
					(void)send_rsp(raw->cmd, raw->seq, raw->delay_ms,
								   parse_ret, NULL, 0);
				}
			}
			continue;
		}

		handle_command(&hdr, payload);
	}

end:
	ret_value = deinit_rpmsg();
}
