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
#include <plat.h>
#include <cmvi.h>
#include <idc.h>

#define I2C_CMVI_SLAVEADDR	(0x6A)	/* device_addr: 0110 101X */
#define UART_DLL_REG		(0x0)	/* can access when LCR[7] = 1 */
#define UART_DLH_REG		(0x1)	/* can access when LCR[7] = 1 */
#define UART_LCR_REG		(0x3)	/* LCR_CTRL */
#define UART_FCR_REG		(0x2)	/* can access when LCR[7] = 0 */
#define UART_DATA_REG		(0x0)	/* can access when LCR[7] = 0 DATA */
#define UART_LSR_REG		(0x5)	/* can access when LCR[7] = 0 LSR */
#define UART_TXFIFO_FREE_MASK	(0x60)	/* if (x & mask) then can write */
#define UART_RXFIFO_DATA_MASK	(0x1)	/* if (x & mask) then can read */
#define UART_REG_ADDR(reg)	(((reg) & 0xF) << 3) /* 7, 6-3:(reg),2-1:00, 0:0 */

void console_init(void)
{
	/* I2C:100KHz<->UART */
	idc_writel(I2C_CMVI, IDC_ENR_OFFSET, 0);
	idc_clk_config(I2C_CMVI, 100000);
	idc_writel(I2C_CMVI, IDC_ENR_OFFSET, 1);
	cmvi_delay(1000);

	/* select: LCR_CTRL and configure LCR[7]=1 */
	idc_write_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_LCR_REG), 0x80);

	/* write UART_DLH_REG Firstly, 115200; 24M/8/26 */
	idc_write_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_DLH_REG), 0);

	/* write UART_DLL_REG: 0x0D(115200 can OK) */
	idc_write_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_DLL_REG), 0xD);

	/* select: LCR_CTRL and configure LCR[7]=0,8n1 */
	idc_write_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_LCR_REG), 0x3);

	/* FCR: Reset FIFO */
	idc_write_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_FCR_REG), 0x6);

	/* FCR: Enable FIFO */
	idc_write_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_FCR_REG), 0x1);
}

void console_putchar(char c)
{
	/* 64byte fifo quick enougth \n */
	idc_write_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_DATA_REG), c);
}

int console_getchar(void)
{
	int lsr, data = -1;

	/* Read LSR */
	lsr = idc_read_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
		UART_REG_ADDR(UART_LSR_REG));

	if (lsr & UART_RXFIFO_DATA_MASK) {
		data = idc_read_8_8(I2C_CMVI, I2C_CMVI_SLAVEADDR,
			UART_REG_ADDR(UART_DATA_REG));
	}

	board_cmvi_dbgled_blink();

	return data;
}
