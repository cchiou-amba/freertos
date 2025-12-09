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
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include <ambhw/idc.h>
#include <i2c.h>

struct i2c_master {
	struct i2c_dev dev;
	int status;
	int opcode;
	unsigned long regbase;
	SemaphoreHandle_t mutex;
	EventGroupHandle_t event;
};

enum{
	OP_WRITE,
	OP_READ,
};

static struct i2c_master i2c_master_bus[IDC_INSTANCES] =
{
	[0] = {
		.dev = {
			.name = "i2c0",
			.irq = IDC0_IRQ,
		},
		.regbase = IDC0_BASE,
		.status = 0,
	},

	[1] = {
		.dev = {
			.name = "i2c1",
			.irq = IDC1_IRQ,
		},
		.regbase = IDC1_BASE,
		.status = 0,
	},

	[2] = {
		.dev = {
			.name = "i2c2",
			.irq = IDC2_IRQ,
		},
		.regbase = IDC2_BASE,
		.status = 0,
	},
};

static StaticEventGroup_t i2c_event_storage[IDC_INSTANCES];
static StaticSemaphore_t i2c_mutex_storage[IDC_INSTANCES];

static int event_init(struct i2c_master *bus)
{
	int rval = 0;

	bus->event = xEventGroupCreateStatic(&i2c_event_storage[bus->dev.chan]);

	if(bus->event == NULL)
	{
		rval = -1;
		printf("i2c%u event init failed\n", bus->dev.chan);
	}

	return rval;
}

static int mutex_init(struct i2c_master *bus)
{
	int rval = 0;

	bus->mutex = xSemaphoreCreateMutexStatic(&i2c_mutex_storage[bus->dev.chan]);

	if(bus->mutex == NULL)
	{
		rval = -1;
		printf("i2c%u mutex init failed\n", bus->dev.chan);
	}

	return rval;
}

static void mutex_lock(SemaphoreHandle_t mutex)
{
	const TickType_t timeout_ticks = pdMS_TO_TICKS(100);
	xSemaphoreTake(mutex,timeout_ticks);
}

static void mutex_unlock(SemaphoreHandle_t mutex)
{
	xSemaphoreGive(mutex);
}

static void i2c_master_set_clock(struct i2c_master *bus)
{
	uint32_t parent_clk, dutycycle;
	uint16_t prescale;

	parent_clk = current_cpufreq() / 2;
	dutycycle = readl(bus->regbase + IDC_DUTYCYCLE_OFFSET) & 0x3;

	prescale = ((parent_clk / bus->dev.freq - 2) / (4 + dutycycle)) - 1;

	writel(bus->regbase + IDC_ENR_OFFSET, IDC_ENR_REG_DISABLE);
	writel(bus->regbase + IDC_PSLL_OFFSET, prescale & 0xff);
	writel(bus->regbase + IDC_PSLH_OFFSET, (prescale >> 8) & 0xff);
	writel(bus->regbase + IDC_ENR_OFFSET, IDC_ENR_REG_ENABLE);
}

static void i2c_irq_handler(void *arg)
{
	struct i2c_master *bus = (struct i2c_master*)arg;
	uint32_t status, timeout = 1000;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	status = readl(bus->regbase + IDC_CTRL_OFFSET);
	/* if no interrupt, return */
	if(!(status & IDC_CTRL_IF))
		goto out;

	if (bus->opcode == OP_WRITE) {
		while (timeout--) {
			if (!(status & IDC_CTRL_ACK)) {
				xEventGroupSetBitsFromISR(bus->event, 1,&xHigherPriorityTaskWoken);
				break;
			}
			status = readl(bus->regbase + IDC_CTRL_OFFSET);
		}
	} else {
		/* if OP_READ, we just check if the interrupt is happened */
		xEventGroupSetBitsFromISR(bus->event, 1,&xHigherPriorityTaskWoken);
	}

	if (xHigherPriorityTaskWoken)
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
out:
	return;

}

static int i2c_bus_init(int chan,uint32_t freq)
{
	int rval = 0;
	struct i2c_master *bus;

	bus = &i2c_master_bus[chan];
	bus->dev.chan = chan;
	bus->dev.freq = freq;

	i2c_master_set_clock(bus);
	rval = mutex_init(bus);
	rval = event_init(bus);
	request_irq(bus->dev.irq, IRQ_RISING_EDGE, i2c_irq_handler, bus, bus->dev.name);

	return rval;
}

static void i2c_safety_disable(uint8_t i2c_id)
{
	setbitsl(CFG_SAFETY_EN_REG, 1U << (i2c_id + 4));
}

static void i2c_enable(struct i2c_master *bus, int enable)
{
	writel(bus->regbase, enable ? IDC_ENR_REG_ENABLE : IDC_ENR_REG_DISABLE);
}

static int i2c_wait_ack(struct i2c_master *bus, uint8_t slave, int verbose)
{
	int rval = 0;
	EventBits_t bit;
	const TickType_t timeout_ticks = pdMS_TO_TICKS(100);

	bit = xEventGroupWaitBits(bus->event, 1, pdTRUE, pdTRUE, timeout_ticks);
	if (bit != 1){
		if (verbose) {
			printf("%s bus slave[0x%02x] with NACK.\n", bus->dev.name, slave >> 1);
		}
		rval = -1;
	}

	return rval;
}

static void i2c_send_stop(struct i2c_master *bus)
{
	writel(bus->regbase + IDC_CTRL_OFFSET, IDC_CTRL_STOP);
}

static int i2c_send_slave(struct i2c_master *bus, uint8_t slave_rw)
{
	int ret = 0, verbose, retry = 10;

	/* if it is read operation, send the re-start bit */
	bus->opcode = (slave_rw & 0x01) ? OP_READ : OP_WRITE;

	/* omit error log */
	verbose = bus->opcode == OP_WRITE ? 0 : 1;

retry_if_noack:
	writel(bus->regbase + IDC_DATA_OFFSET, slave_rw);
	writel(bus->regbase + IDC_CTRL_OFFSET, IDC_CTRL_START);

	ret = i2c_wait_ack(bus, slave_rw, verbose);
	if (ret && bus->opcode == OP_WRITE && retry-- > 0) {
		if (retry == 0) {
			verbose = 1;
		}
		goto retry_if_noack;
	}

	return ret;
}

static int i2c_write_byte(struct i2c_master *bus, uint8_t slave, uint8_t byte)
{
	bus->opcode = OP_WRITE;

	writel(bus->regbase + IDC_DATA_OFFSET, byte);
	/* send ACK to slave after WRITE operation */
	writel(bus->regbase + IDC_CTRL_OFFSET, 0x0);

	return i2c_wait_ack(bus, slave, 1);
}

static int i2c_send_reg(struct i2c_master *bus, uint8_t slave, struct i2c_msg *msg)
{
	int i, rval = 0;

	for (i = 0 ; i < msg->len; i++) {
		rval = i2c_write_byte(bus, slave, msg->buf[i]);
		if (rval < 0)
			break;
	}
	return rval;
}

static int i2c_send_data(struct i2c_master *bus, uint8_t slave, struct i2c_msg *msg)
{
	int i, rval = 0;

	for (i = 0 ; i < msg->len; i++) {
		rval = i2c_write_byte(bus, slave, msg->buf[i]);
		if (rval < 0)
			break;
	}
	return rval;
}

static int i2c_recv_data(struct i2c_master *bus, uint8_t slave, struct i2c_msg *msg)
{
	uint32_t i, nack;
	int rval = 0;

	bus->opcode = OP_READ;

	for(i = 0; i < msg->len; i++) {
		nack = ((i + 1) == msg->len) ? IDC_CTRL_ACK : 0;
		writel(bus->regbase + IDC_CTRL_OFFSET, nack);

		rval = i2c_wait_ack(bus, slave, 1);
		if (!rval)
			msg->buf[i] = readl(bus->regbase + IDC_DATA_OFFSET);
	}

	return rval;
}

static void *get_i2c_bus(int chan)
{
	if (chan >= IDC_INSTANCES) {
		return NULL;
	}

	return &i2c_master_bus[chan];
}

void *i2c_get_bus_device(int chan)
{
	if (chan >= IDC_INSTANCES) {
		return NULL;
	}

	return &i2c_master_bus[chan].dev;
}

int i2c_msg_write(uint8_t chan, uint8_t slave, struct i2c_msg *msg, uint32_t flag)
{
	struct i2c_master *bus = get_i2c_bus(chan);
	int rval = -1;

	if(!bus->status) {
		printf("i2c%u isn't initialized\n", chan);
		return -1;
	}

	mutex_lock(bus->mutex);
	slave <<= 1;

	if (i2c_send_slave(bus, slave))
		goto __write_err;
	if (i2c_send_reg(bus, slave, &msg[0]))
		goto __write_err;
	if (i2c_send_data(bus, slave, &msg[1]))
		goto __write_err;

	rval = 0;

__write_err:
	i2c_send_stop(bus);
	mutex_unlock(bus->mutex);

	return rval;
}

int i2c_msg_read(uint8_t chan, uint8_t slave, struct i2c_msg *msg, uint32_t flag)
{
	struct i2c_master *bus = get_i2c_bus(chan);
	int rval = -1;

	if (!bus->status) {
		printf("i2c%u isn't initialized\n", chan);
		return -1;
	}

	mutex_lock(bus->mutex);
	slave <<= 1;

	if (i2c_send_slave(bus, slave))
		goto __read_err;
	if (i2c_send_reg(bus, slave, &msg[0]))
		goto __read_err;

	if (flag & I2C_M_STOP)
		i2c_send_stop(bus);

	if (i2c_send_slave(bus, slave | 1))
		goto __read_err;
	if (i2c_recv_data(bus, slave, &msg[1]))
		goto __read_err;

	rval = 0;

__read_err:
	i2c_send_stop(bus);
	mutex_unlock(bus->mutex);

	return rval;
}

int i2c_write(uint8_t chan, uint8_t slave, struct i2c_msg *msg)
{
	return i2c_msg_write(chan, slave, msg, 0);
}

int i2c_read(uint8_t chan, uint8_t slave, struct i2c_msg *msg)
{
	return i2c_msg_read(chan, slave, msg, 0);
}

int i2c_init(int id, uint32_t freq, int status)
{
	i2c_safety_disable(id);

	if (id > IDC_INSTANCES)
		return -1;

	if (status == 1){
		if (i2c_bus_init(id,freq) < 0){
			printf("I2C%u init failed\n",id);
			return -1;

		} else{
			// printf("I2C%u is OK, freq %u Hz\n", id, freq);
			i2c_master_bus[id].status = 1;
		}
	}

	return 0;
}

int i2c_transfer(uint8_t chan, uint8_t dev_addr,
		 struct i2c_msg *msg, uint8_t xfer_mode)
{
	int ret = 0;
	struct i2c_master *bus = get_i2c_bus(chan);

	if (!bus->status) {
		printf("i2c%u isn't initialized\n", chan);
		return -1;
	}

	mutex_lock(bus->mutex);
	i2c_enable(bus, 1);

	do {
		if (xfer_mode == I2C_XFER_WRITE) {
			ret = i2c_send_slave(bus, dev_addr);
			if (ret)
				break;
			ret = i2c_send_reg(bus, dev_addr, msg);
			if (ret)
				break;
		} else {
			ret = i2c_send_slave(bus, dev_addr | 1);
			if (ret)
				break;
			ret = i2c_recv_data(bus, dev_addr, msg);
			if (ret)
				break;
		}
	} while(0);

	i2c_send_stop(bus);
	i2c_enable(bus, 0);
	mutex_unlock(bus->mutex);

	return ret;
}
