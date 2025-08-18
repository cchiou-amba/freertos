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

#include <cmvi.h>
#include <gpio.h>
#include <shell.h>
#include <common.h>
#include <task.h>
#include <event_groups.h>

#if defined(CMVI_SUPPORT_ONCHIP)
static uint32_t srs_pwrctrl = 0;
static uint32_t srs_auto_wkup_enable = 0;
static volatile uint32_t suspend_state = 0;
static EventGroupHandle_t cmvi_EventGroupHandle;
static StaticEventGroup_t cmvi_CreatedEventGroup;
#define task_cmvi_stack_SIZE  (configMINIMAL_STACK_SIZE * 2)
#define task_cmvi_PRIORITY    (tskIDLE_PRIORITY + 3)
static StaticTask_t task_cmvi;
static StackType_t  task_cmvi_stack[task_cmvi_stack_SIZE];
static void cmvipwr_task(void *data);
static void cmvipwr_task_init(void)
{
	cmvi_EventGroupHandle = xEventGroupCreateStatic(&cmvi_CreatedEventGroup);
	configASSERT(cmvi_EventGroupHandle);

	xTaskCreateStatic(
		cmvipwr_task,
		"cmvipwr",
		task_cmvi_stack_SIZE,
		NULL,
		task_cmvi_PRIORITY,
		task_cmvi_stack,
		&task_cmvi);
}

/* called by main.c in main() */
void app_task_init(void)
{
	cmvipwr_task_init();
}

void __attribute__((weak)) board_cmvi_wkup_clear(void)
{
	/* board related pin */
}

void __attribute__((weak)) board_cmvi_dbgled_blink(void)
{
	/* board related pin */
}

void __attribute__((weak)) board_cmvi_ddrrst_ctrl(uint32_t on)
{
	(void)on;
	/* board related pin */
}

void __attribute__((weak)) board_cmvi_disable_por(void)
{
	/* board related */
}

void __attribute__((weak)) board_cmvi_enable_por(void)
{
}

void __attribute__((weak))  board_cmvi_srsflag_set(void)
{
	/* board related */
}

void __attribute__((weak))  board_cmvi_srsflag_clr(void)
{

}

void __attribute__((weak))  board_cmvi_wkup_custom_post(void)
{
}

void __attribute__((weak))  cmvi_custom_op0(void)
{
}

void __attribute__((weak))  cmvi_custom_op1(void)
{
}

void __attribute__((weak))  cmvi_custom_op2(void)
{
}

void __attribute__((weak))  cmvi_custom_op3(void)
{
}

void __attribute__((weak))  board_cmvi_poweroff_custom_post(void)
{
}

void cmvi_cm3_mark_bootdone(void)
{
	/*
	 * set this flag to tell CPU that CM3 is rdy
	 * So CPU will not re-load CM3 firmware
	*/
	writel(CM3_SCRATCHPAD_REG(CM3_BOOT_STAT_OFFSET), CM3_BOOT_RDY_MASK);
}

/* ISR delay: tick not update during ISR service */
void cmvi_delay(volatile uint32_t loops)
{
	while (loops--) {
		__NOP();
	}
}

void cmvi_por_pi_prepare(void)
{
	/* get some lock and make sure other thread no accessing PI anymore*/
	uint32_t data;

	/* step1: set isolation_sw-reset */
	data = readl(CM3_SCRATCHPAD_REG(CM3_PWR_GATE_OFFSET));
	data |= PWR_GATE_ISOLATE_SW_RST_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_PWR_GATE_OFFSET), data);

	/* step2: set isolate_en */
	data |= PWR_GATE_ISOLATE_EN_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_PWR_GATE_OFFSET), data);

	/* step3: prepare shutdown pi */
	data |= PWR_GATE_SHUTDOWN_PI_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_PWR_GATE_OFFSET), data);

	/* Now: cm3 begin to use internal OSC clock */

	/* step4: POR_L = 0 && Keep POR_L=0 for >=2ms */
	data = readl(CM3_SCRATCHPAD_REG(CM3_PI_POR_OFFSET));
	data &= ~PI_POR_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_PI_POR_OFFSET), data);
#if 0
	/* switch to use internal OSC clock(not accurate) */
	vPortSetupTimerInterruptDynamic(CMVI_PI_OFF_CM3FREQ);
#endif
}

void cmvi_por_pi_unprepare(void)
{
	uint32_t data;

	/* step1: check pi_por_L_monitor=H then Next */
	do {
		data = readl(CM3_SCRATCHPAD_REG(CM3_PI_RESET_MONITOR_OFFSET));
	} while (!(data & PI_POR_MONITOR_MASK));

	/* step2: POR_L = 1, relase PORL */
	data = readl(CM3_SCRATCHPAD_REG(CM3_PI_POR_OFFSET));
	data |= PI_POR_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_PI_POR_OFFSET), data);

	/* check pi_grst_L_monitor=H then Next */
	do {
		data = readl(CM3_SCRATCHPAD_REG(CM3_PI_RESET_MONITOR_OFFSET));
	} while (!(data & PI_GREST_MONITOR_MASK));

	/* clear  shutdown-pi and isolation_en then isolation-sw-rest */
	data = readl(CM3_SCRATCHPAD_REG(CM3_PWR_GATE_OFFSET));
	data &= ~(PWR_GATE_SHUTDOWN_PI_MASK | PWR_GATE_ISOLATE_EN_MASK);
	writel(CM3_SCRATCHPAD_REG(CM3_PWR_GATE_OFFSET), data);
	data &= ~PWR_GATE_ISOLATE_SW_RST_MASK; /* clean isolation_sw-reset */
	writel(CM3_SCRATCHPAD_REG(CM3_PWR_GATE_OFFSET), data);

#if 0
	/* restore cpufreq */
	vPortSetupTimerInterruptDynamic(current_cpufreq());
#endif
	cmvi_delay(40);
}

void __attribute__((weak)) cmvi_powerseq_ops(int32_t on, int32_t srs)
{
	uint32_t data = 0;

#define CMVI_DELAY_LOOPS	(20000)	/* gap about 28ms, like CMVI-cold boot */
	if (on) {
		data = readl(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET));
		data &= ~(PWR_SEQ1_MASK | PWR_SEQ3_MASK);
		if (!srs) {
			data &= ~PWR_SEQ2_MASK;	/* PSEQ2 = 1; EVK->PSEQ3->DDR */
		}
		data |= PWR_SEQ1_MASK; 		/* PSEQ1 = 1 */
		writel(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET), data);
		cmvi_delay(CMVI_DELAY_LOOPS);
		if (!srs) {
			data |= PWR_SEQ2_MASK; /* PSEQ2 = 1; EVK->PSEQ3->DDR */
			writel(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET), data);
			cmvi_delay(CMVI_DELAY_LOOPS);
		}
		data |= PWR_SEQ3_MASK; 		/* PSEQ3 = 1 */
		writel(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET), data);
	} else {
		data = readl(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET));
		data &= ~PWR_SEQ3_MASK; 	/* PSEQ3=0 */
		writel(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET), data);
		cmvi_delay(CMVI_DELAY_LOOPS);
		if (!srs) {
			data &= ~PWR_SEQ2_MASK;	/* PSEQ2=0; EVK->PSEQ3->DDR */
			writel(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET), data);
			cmvi_delay(CMVI_DELAY_LOOPS);
		}
		data &= ~PWR_SEQ1_MASK;		/* PSEQ1=0 */
		writel(CM3_SCRATCHPAD_REG(CM3_PWR_SEQ_OFFSET), data);
	}
	cmvi_delay(CMVI_DELAY_LOOPS);
}

/*  or wakeup the dedicated thread to
 * do the interrupt handler jobs */
void cmvi_warm_reset_pi(uint32_t wdtvect)
{
	uint32_t data;

	wdtvect &= WDT_TMOUT_MASK;

	/*trigger warm rest by RCT */
	data = readl(CM3_SCRATCHPAD_REG(CM3_RTC_CTRL_REG));
	data &= ~WARM_RESET_PI_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_RTC_CTRL_REG), data);

	/* tell CPU watchdog reset trigger warm rest */
	data = readl(CM3_SCRATCHPAD_REG(CM3_PI_WDT_TMOUT_STAT_OFFSET));
	data |= wdtvect;
	writel(CM3_SCRATCHPAD_REG(CM3_PI_WDT_TMOUT_STAT_OFFSET), data);

	/* check pi_grst_monitor */
	do {
		data = readl(CM3_SCRATCHPAD_REG(CM3_PI_RESET_MONITOR_OFFSET));
	} while (data & PI_GREST_MONITOR_MASK);

	/* clear warm rest by RCT */
	data = readl(CM3_SCRATCHPAD_REG(CM3_RTC_CTRL_REG));
	data |= WARM_RESET_PI_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_RTC_CTRL_REG), data);
}

/* div resume ops into two part
 * cmvi_wkup_exit: wakeup CPU as soon as possible
 * cmvi_wkup_post: some cleanup jobs
*/
static void cmvi_wkup_exit(void)
{
	if (srs_pwrctrl)
		cmvi_powerseq_ops(POWER_SEQ_ON, POWER_WITH_SRS);
	else
		cmvi_powerseq_ops(POWER_SEQ_ON, POWER_NO_SRS);

	srs_pwrctrl = 0;
	cmvi_por_pi_unprepare();
}

static void cmvi_wkup_post(void)
{
	printf("CMVI:Wakeup Post! \n");
	board_cmvi_wkup_custom_post();
	if (suspend_state) {
		vTaskDelay(10);	/* wait for SOC bootup(covery bst stage) */
		board_cmvi_srsflag_clr();	/* this bit must clear or por will failed */
		board_cmvi_enable_por();	/* now re-enable por */
		board_cmvi_ddrrst_ctrl(1);	/* re-connect dram reset ctrl */
		suspend_state = 0;		/* clear suspend state if set */
	}
}

/********************************CMVI_ISR*******************************/
void cmvi_isr_event_setbits(uint32_t bitmask)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;

	xResult = xEventGroupSetBitsFromISR(
				cmvi_EventGroupHandle,
				bitmask,
				&xHigherPriorityTaskWoken );

	if(xResult != pdFAIL) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

/* ISR: wdt0 as warm reset */
void wdt0_pi_intr(void)
{
	printf("CMVI: PI_WDT0!\r\n");
	cmvi_warm_reset_pi(WDT_0_TMOUT_MASK);
}

/* ISR: wdt1 as cold reset */
void wdt1_pi_intr(void)
{
	/* we re-use watchdog timeout flag as SRS flag when RTC broken
	 * wdt1 should not be use by A76 anymore.
	*/
#if 0
	uint32_t data = 0;
	printf("CMVI: PI_WDT1 !\r\n");
	/* tell CPU watchdog1 timeout */
	data = readl(CM3_SCRATCHPAD_REG(CM3_PI_WDT_TMOUT_STAT_OFFSET));
	data |= WDT_1_TMOUT_MASK;
	writel(CM3_SCRATCHPAD_REG(CM3_PI_WDT_TMOUT_STAT_OFFSET), data);

	cmvi_por_pi_prepare();
	cmvi_powerseq_ops(POWER_SEQ_OFF, POWER_NO_SRS);
	cmvi_delay(80000);
	cmvi_powerseq_ops(POWER_SEQ_ON, POWER_NO_SRS);
	cmvi_por_pi_unprepare();
#endif
}

/* ISR: pi_ext_por_deassert
 * trigger by press PI-reset-button,
 * trigger por-L interrupt to CM3
 * CM3 do POR_L to PI
 * cmvi_isr_event_setbits(CMVI_EVT_POR);
 */
void pi_ext_por_deassert(void)
{
	/* must handle it as soon as possible, POC sample issue */
	srs_pwrctrl = 0;
	cmvi_por_pi_prepare();
	cmvi_por_pi_unprepare();
	printf("CMVI: POR deassert !\n");
}

/* wakeup source: used for SRS or poweroff wakeup */
void cmvi_gpio_intr(void)
{
	board_cmvi_wkup_clear();
	cmvi_isr_event_setbits(CMVI_EVT_WKUP);
}

/* vbus connect signal, CV75EVK PPS connect to
 * VBUS. if PPS enable then VBUS can get a lot of VBUS interrupts
 * [disable PPS or disable VBUS detected]
 */
void cmvi_vbus_connect_intr(void)
{
	printf("CMVI:VBUS !\n");
}

void cmvi_rtc_alarm_intr(void)
{
	printf("CMVI:RTC Alarm !\n");
}

static void cmvi_srs_enter(void)
{
	printf("CMVI:SRS Enter \n");
	srs_pwrctrl = 1;
	board_cmvi_ddrrst_ctrl(0);	/* dis-connect dram reset ctrl */
	board_cmvi_disable_por();	/* avoid spurisous POR when wakeup */
	cmvi_por_pi_prepare();
	cmvi_powerseq_ops(POWER_SEQ_OFF, POWER_WITH_SRS);
	board_cmvi_srsflag_set();
	suspend_state = 1;		/* mark this is suspend state */
}

void cmvipwr_task(void *data)
{
	EventBits_t evtbits;
	(void)data;

	while (1) {
		evtbits = xEventGroupWaitBits(
		cmvi_EventGroupHandle,	/* The event group being tested. */
		CMVI_EVT_BITS,		/* The bits within the event group to wait for. */
		pdTRUE,			/* should be cleared before returning. */
		pdFALSE,		/* either bit will do. */
		portMAX_DELAY);

		if (evtbits & CMVI_EVT_WKUP) {
			cmvi_wkup_exit();
			cmvi_wkup_post();
		}
		if (evtbits & CMVI_EVT_SRS_SLP) {
			cmvi_srs_enter();
			if (srs_auto_wkup_enable) {
				/* the clock source is 16~20Mhz,
				* keep the original clock source configure
				* for simplicit [console interaction stuck]
				*/
				vTaskDelay(50);
				cmvi_wkup_exit();
				cmvi_wkup_post();
			}
		}
		if (evtbits & CMVI_EVT_POR) {
			/* it is too later for POC sample point */
		}
		if (evtbits & CMVI_EVT_WDT0) {

		}
		if (evtbits & CMVI_EVT_WDT1) {

		}
		if (evtbits & CMVI_EVT_USBPLG) {

		}
		if (evtbits & CMVI_EVT_RTCALRM) {

		}

		if (evtbits & CMVI_EVT_CUSTOM_OP0) {
			cmvi_custom_op0();
		}
		if (evtbits & CMVI_EVT_CUSTOM_OP1) {
			cmvi_custom_op1();
		}
		if (evtbits & CMVI_EVT_CUSTOM_OP2) {
			cmvi_custom_op2();
		}

		if (evtbits & CMVI_EVT_CUSTOM_OP3) {
			cmvi_custom_op3();
		}

		if (evtbits & CMVI_EVT_POWER_OFF) {
			printf("CMVI:PowerOff !\n");
			board_cmvi_disable_por();
			cmvi_por_pi_prepare();
			cmvi_powerseq_ops(POWER_SEQ_OFF, POWER_NO_SRS);
			board_cmvi_poweroff_custom_post();
		}
	}
}

/* reset: poweroff -> poweron */
int cmd_reset(int argc, char *argv[])
{
	xEventGroupSetBits(cmvi_EventGroupHandle, CMVI_EVT_POWER_OFF);
	vTaskDelay(200);
	xEventGroupSetBits(cmvi_EventGroupHandle, CMVI_EVT_WKUP);
	return 0;
}
CMD_EXPORT_ALIAS(cmd_reset,reset,reset PI);

/* srs auto wakeup switch */
int cmd_autowkup(int argc, char *argv[])
{
	srs_auto_wkup_enable ^= 1;
	printf("srs_auto_wakeup:%s \n", srs_auto_wkup_enable ? "Yes":"NO");
	return 0;
}
CMD_EXPORT_ALIAS(cmd_autowkup,autowk,auto wakeup);

/* poweroff */
int cmd_poweroff(int argc, char *argv[])
{
	xEventGroupSetBits(cmvi_EventGroupHandle, CMVI_EVT_POWER_OFF);
	return 0;
}
CMD_EXPORT_ALIAS(cmd_poweroff, poweroff, poweroff);

#endif
