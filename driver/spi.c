/*******************************************************************************
 *
 * Copyright (c) 2024 Ambarella International LP
 *
 * This file and its contents ("Software") are protected by intellectual
 * property rights including, without limitation, U.S. and / or foreign
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
#include <gpio.h>
#include <spi.h>
#include <common.h>
#include <task.h>

#define FIFO_THRESHOLD	(8)

static List_t spi_dev_list = {0};

/*===========================================================================*/
static inline int gpio_set_value(int gpio, int value) {
	return value ? gpio_set(gpio) : gpio_clr(gpio);
}

static inline void spi_enable(spi_t *spi)
{
	if (spi->cfg.master == pdTRUE) {
		writel(spi->base + SPI_SER_OFFSET, 1 << spi->cfg.chip_select);
		if (spi->cs_gpios[spi->cfg.chip_select] >= 0)
			gpio_set_value(spi->cs_gpios[spi->cfg.chip_select], spi->cfg.mode & SPI_CS_HIGH);
	} else {
		if (spi->notify_gpio >= 0) {
			gpio_set(spi->notify_gpio);
		}
	}

	writel(spi->base + SPI_SSIENR_OFFSET, SPI_SSIENR_ENABLE);
}

static inline void spi_disable(spi_t *spi)
{
	writel(spi->base + SPI_SSIENR_OFFSET, SPI_SSIENR_DISABLE);
	writel(spi->base + SPI_DMAC_OFFSET, 0);
	writel(spi->base + SPI_IMR_OFFSET, 0);

	if (spi->cfg.master == pdTRUE) {
		writel(spi->base + SPI_SER_OFFSET, 0);
		if (spi->cs_gpios[spi->cfg.chip_select] >= 0)
			gpio_set_value(spi->cs_gpios[spi->cfg.chip_select], !(spi->cfg.mode & SPI_CS_HIGH));
	} else {
		if (spi->notify_gpio >= 0) {
			gpio_clr(spi->notify_gpio);
		}
	}
}

static inline void spi_tx(struct spi_s *spi)
{
	if (spi->cfg.bits_per_word > 8) {
		writel(spi->base + SPI_DR_OFFSET, *((uint16_t *)spi->tx_buf));
		__DSB();
		spi->tx_buf += 2U;
		spi->tx_cnt -= 2U;
	} else {
		writel(spi->base + SPI_DR_OFFSET, *spi->tx_buf);
		__DSB();
		spi->tx_buf++;
		spi->tx_cnt--;
	}
}

static inline void spi_rx(struct spi_s *spi)
{
	if (spi->cfg.bits_per_word > 8) {
		*((uint16_t *)spi->rx_buf) = (uint16_t)readl(spi->base + SPI_DR_OFFSET);
		__DSB();
		spi->rx_buf += 2U;
		spi->rx_cnt -= 2U;
	} else {
		*spi->rx_buf = (uint8_t)readl(spi->base + SPI_DR_OFFSET);
		__DSB();
		spi->rx_buf++;
		spi->rx_cnt--;
	}
}

static int spi_end_xfer(spi_t *spi, uint32_t timeout, TickType_t tick_start)
{
	writel(spi->base + SPI_DMAC_OFFSET, 0);
	spi_disable(spi);

	return 0;
}

int spi_xfer_poll(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, uint16_t size, uint32_t timeout)
{
	int err = 0;
	uint32_t sr = 0;
	TickType_t tick;

	tick = xTaskGetTickCount();

	spi->tx_buf = tx_buf;
	spi->tx_len = size;
	spi->tx_cnt = size;
	spi->tx_isr = NULL;
	spi->rx_buf = rx_buf;
	spi->rx_len = size;
	spi->rx_cnt = size;
	spi->rx_isr = NULL;

	if (spi->cfg.tmod == SPI_WRITE_READ) {
		spi->state  = SPI_STATE_BUSY_TX_RX;
	} else if (spi->cfg.tmod == SPI_WRITE_ONLY) {
		spi->state  = SPI_STATE_BUSY_TX;
		spi->rx_buf = NULL;
		spi->rx_len = 0U;
		spi->rx_cnt = 0U;
	} else if (spi->cfg.tmod == SPI_READ_ONLY) {
		spi->state  = SPI_STATE_BUSY_RX;
		spi->tx_buf = NULL;
		spi->tx_len = 0U;
		spi->tx_cnt = 0U;
	}

	writel(spi->base + SPI_IMR_OFFSET, 0);
	writel(spi->base + SPI_DMAC_OFFSET, SPI_DMAC_TX_EN);

	if (spi->cfg.master == pdFALSE) {
		spi_tx(spi);
	}

	if (readl(spi->base + SPI_SSIENR_OFFSET) != SPI_SSIENR_ENABLE) {
		spi_enable(spi);
	}

	while (spi->tx_cnt > 0 || spi->rx_cnt > 0) {
		sr = readl(spi->base + SPI_SR_OFFSET);
		if (sr & SPI_SR_TFNF && spi->tx_cnt > 0) {
			spi_tx(spi);
		}

		if (sr & SPI_SR_RFNE && spi->rx_cnt > 0) {
		 	spi_rx(spi);
		}

		if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(timeout)) {
			err = -1;
		 	goto error;
		}
	}

	spi_end_xfer(spi, timeout, tick);

error:
	spi->state = SPI_STATE_READY;
	return err;
}

/*===========================================================================*/
__attribute((weak)) void spi_xfer_cplt(spi_t *spi)
{
}

static void spi_close_xfer_isr(spi_t *spi)
{
	TickType_t tick;

	tick = xTaskGetTickCount();

	spi_end_xfer(spi, 1000, tick);

	spi->state = SPI_STATE_READY;

	if (spi->xfer_cplt) {
		spi->xfer_cplt(spi);
	} else {
		spi_xfer_cplt(spi);
	}
}

static void spi_tx_isr(struct spi_s *spi)
{
	uint16_t len = spi->tx_cnt < FIFO_THRESHOLD ? spi->tx_cnt : FIFO_THRESHOLD;

	while (len--)
		spi_tx(spi);

	if (spi->tx_cnt == 0U) {
		clrbitsl(spi->base + SPI_IMR_OFFSET, SPI_TXEIS_MASK);

		if (spi->rx_cnt == 0U) {
			spi_close_xfer_isr(spi);
		}
	}
}

static void spi_rx_isr(struct spi_s *spi)
{
	uint32_t len = readl(spi->base + SPI_RXFLR_OFFSET);

	len = len < spi->rx_cnt ? (len < FIFO_THRESHOLD ? len : FIFO_THRESHOLD) : spi->rx_cnt;

	while (len--)
		spi_rx(spi);

	if (spi->rx_cnt == 0U) {
		clrbitsl(spi->base + SPI_IMR_OFFSET, SPI_RXFIS_MASK);

		if (spi->tx_cnt == 0U) {
			spi_close_xfer_isr(spi);
		}
	}
}

void spi_isr(spi_t *spi)
{
	uint32_t isr = readl(spi->base + SPI_ISR_OFFSET);

	readl(spi->base + SPI_ICR_OFFSET);

	if (isr & SPI_TXEIS_MASK) {
		spi->tx_isr(spi);
	}

	if (isr & SPI_RXFIS_MASK) {
		spi->rx_isr(spi);
	}

	return;
}

/*===========================================================================*/
int spi_xfer_intr(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, uint16_t size)
{
	int err = 0;

	spi->tx_buf = tx_buf;
	spi->tx_len = size;
	spi->tx_cnt = size;
	spi->tx_isr = spi_tx_isr;
	spi->rx_buf = rx_buf;
	spi->rx_len = size;
	spi->rx_cnt = size;
	spi->rx_isr = spi_rx_isr;

	if (spi->cfg.tmod == SPI_WRITE_READ) {
		spi->state  = SPI_STATE_BUSY_TX_RX;
		writel(spi->base + SPI_IMR_OFFSET, SPI_TXEIS_MASK | SPI_RXFIS_MASK);
	} else if (spi->cfg.tmod == SPI_WRITE_ONLY) {
		spi->state  = SPI_STATE_BUSY_TX;
		spi->rx_buf = (uint8_t *)NULL;
		spi->rx_len = 0U;
		spi->rx_cnt = 0U;
		spi->rx_isr = NULL;
		writel(spi->base + SPI_IMR_OFFSET, SPI_TXEIS_MASK);
	} else if (spi->cfg.tmod == SPI_READ_ONLY) {
		spi->state  = SPI_STATE_BUSY_RX;
		spi->tx_buf = (uint8_t *)NULL;
		spi->tx_len = 0U;
		spi->tx_cnt = 0U;
		spi->tx_isr = NULL;
		writel(spi->base + SPI_IMR_OFFSET, SPI_RXFIS_MASK);
	}

	/* FIFO: SSI0-SSI3 32 entries, SSIS0 128 entries */
	if (spi->cfg.master == pdTRUE) {
		writel(spi->base + SPI_TXFTLR_OFFSET, 0);
		writel(spi->base + SPI_RXFTLR_OFFSET, 1);
	} else {
		writel(spi->base + SPI_TXFTLR_OFFSET, 128 - 1);
		writel(spi->base + SPI_RXFTLR_OFFSET, FIFO_THRESHOLD - 1);
	}

	/* WA */
	if (spi->cfg.tmod != SPI_READ_ONLY) {
		writel(spi->base + SPI_DMAC_OFFSET, SPI_DMAC_TX_EN);
		spi->tx_isr(spi);
	}

	/* SPI-bitrate Configuration */
	if (spi->cfg.spiclk != 0 && spi->cfg.bitrate != 0)
		writel(spi->base + SPI_BAUDR_OFFSET, (spi->cfg.spiclk / spi->cfg.bitrate));

	if (readl(spi->base + SPI_SSIENR_OFFSET) != SPI_SSIENR_ENABLE) {
		spi_enable(spi);
	}

	return err;
}

spi_t *spi_dev(char *name)
{
	ListItem_t *dev = NULL;
	spi_t *spi = NULL;

	if (listLIST_IS_EMPTY(&spi_dev_list) == pdTRUE) {
		return NULL;
	}

	for (dev = listGET_HEAD_ENTRY(&spi_dev_list); dev != listGET_END_MARKER(&spi_dev_list); dev = listGET_NEXT(dev)) {
		spi = container_of(dev, spi_t, dev);
		if (!strcmp(spi->name, name)) {
			return spi;
		}
	}

	return NULL;
}

int spi_init(spi_t *spi)
{
	spi_ctrl0_reg_t reg_cr0 = {0};

	configASSERT(spi);

	if (spi->res_init) {
		spi->res_init(spi);
	}

	spi_disable(spi);

	/* SPI-bitrate Configuration */
	if (spi->cfg.spiclk != 0 && spi->cfg.bitrate != 0) {
		writel(spi->base + SPI_BAUDR_OFFSET, (spi->cfg.spiclk / spi->cfg.bitrate));
	} else {
		BUG_ON(spi->cfg.master == pdTRUE, "SPI-bitrate config err, \
		spi->cfg.spiclk:%d, spi->cfg.bitrate:%d\n", spi->cfg.spiclk, spi->cfg.bitrate);
	}

	reg_cr0.w = 0;
	reg_cr0.s.dfs = spi->cfg.bits_per_word - 1;
	reg_cr0.s.scph = !!(spi->cfg.mode & SPI_CPHA);
	reg_cr0.s.scpol = !!(spi->cfg.mode & SPI_CPOL);
	reg_cr0.s.srl = !!(spi->cfg.mode & SPI_LOOP);
	reg_cr0.s.tmod = spi->cfg.tmod;
	reg_cr0.s.tx_lsb = !!(spi->cfg.mode & SPI_LSB_FIRST);
	reg_cr0.s.rx_lsb = !!(spi->cfg.mode & SPI_LSB_FIRST);
	reg_cr0.s.residue = 1;
	//reg_cr0.s.rxd_mg = bus->ext_rx_dly; //master config only
	writel(spi->base + SPI_CTRLR0_OFFSET, reg_cr0.w);

	if (spi->cfg.master == pdTRUE) {
		if (spi->cfg.mode & SPI_CS_HIGH)
			setbitsl(spi->base + SPI_SSIENPOLR_OFFSET, 1 << spi->cfg.chip_select);
		else
			clrbitsl(spi->base + SPI_SSIENPOLR_OFFSET, 1 << spi->cfg.chip_select);
	}

	if (listLIST_IS_INITIALISED(&spi_dev_list) == pdFALSE) {
		vListInitialise(&spi_dev_list);
	}

	if (listIS_CONTAINED_WITHIN(&spi_dev_list, &spi->dev) == pdFALSE) {
		vListInitialiseItem(&spi->dev);
		vListInsertEnd(&spi_dev_list, &spi->dev);
	}

	return 0;
}

