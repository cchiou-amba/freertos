 /*******************************************************************************
 *
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
 ******************************************************************************/
#ifndef __AMBA_I2C_H__
#define __AMBA_I2C_H__

#include <stdint.h>

struct i2c_msg {
	uint8_t *buf;
	int len;
};

struct i2c_dev {
	char *name;
	uint8_t chan;
	int irq;
	unsigned int freq;
};

/* ==========================================================================*/
#define I2C_M_NOSTART		(1U << 0)	/* Not implemented yet */
#define I2C_M_STOP		(1U << 1)
#define I2C_M_IGNORE_NAK	(1U << 2)	/* Not implemented yet */
#define I2C_M_NO_RD_ACK		(1U << 3)	/* Not implemented yet */
#define I2C_M_REG_16B		(1U << 4)
#define I2C_M_DATA_16B		(1U << 5)
#define I2C_M_MASK		(0x3f)

#define I2C0			(0)
#define I2C1			(1)
#define I2C2			(2)
#define I2C3			(3)
#define I2C4			(4)
#define I2C5			(5)
#define I2C6			(6)
#define I2C7			(7)
#define I2C8			(8)
#define I2CS			(15)
#define I2C_CMVI		(31)

#define I2C_XFER_READ		(0)
#define I2C_XFER_WRITE		(1)

/* ==========================================================================*/
int i2c_init(int id,uint32_t freq,int status);
int i2c_enabled(int id);
int i2c_msg_read(uint8_t chan, uint8_t slave, struct i2c_msg *msg, uint32_t flag);
int i2c_msg_write(uint8_t chan, uint8_t slave, struct i2c_msg *msg, uint32_t flag);
int i2c_read(uint8_t chan, uint8_t slave, struct i2c_msg *msg);
int i2c_write(uint8_t chan, uint8_t slave, struct i2c_msg *msg);
int i2c_bus_irq(uint8_t chan);
int i2c_transfer(uint8_t chan, uint8_t dev_addr, struct i2c_msg *msg, uint8_t xfer_mode);
void *i2c_get_bus_device(int chan);
#endif /*__AMBA_I2C_H__*/
