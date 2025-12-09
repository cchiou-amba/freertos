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
#ifndef __AMBA_R52_SPI_H__
#define __AMBA_R52_SPI_H__

#include <common.h>

struct spi_dev {
    const char *name;
    int irq;
    u32	max_speed_hz;
    u8	chip_select;
    u8	bits_per_word;
    u32	mode;
    u8  cs_id;
    u8  id;
};

struct spi_transfer {
    uint8_t *tx_buf;
    uint8_t *rx_buf;
    uint16_t n_size;
};
/* ==========================================================================*/
#define SPI0			(0)
#define SPI1			(1)
/* ==========================================================================*/
#define SPI_INSTANCES 			2
#define SPI_MAX_CS_NUM 			1
#define SPI_MASTER_MAX_SPEED 	20000000
/* ==========================================================================*/
#define SPI_CPHA			    0x00000001	/* clock phase */
#define SPI_CPOL			    0x00000002	/* clock polarity */

#define SPI_MODE_0			    (0|0)		/* (original MicroWire) */
#define SPI_MODE_1			    (0|SPI_CPHA)
#define SPI_MODE_2			    (SPI_CPOL|0)
#define SPI_MODE_3			    (SPI_CPOL|SPI_CPHA)
/* ==========================================================================*/
#define SPI_CTRLR0_OFFSET		0x00
#define SPI_CTRLR1_OFFSET		0x04
#define SPI_SSIENR_OFFSET		0x08
#define SPI_SER_OFFSET			0x10
#define SPI_BAUDR_OFFSET		0x14
#define SPI_TXFTLR_OFFSET		0x18
#define SPI_RXFTLR_OFFSET		0x1c
#define SPI_TXFLR_OFFSET		0x20
#define SPI_RXFLR_OFFSET		0x24
#define SPI_SR_OFFSET			0x28
#define SPI_IMR_OFFSET			0x2c
#define SPI_ISR_OFFSET			0x30
#define SPI_RISR_OFFSET			0x34
#define SPI_TXOICR_OFFSET		0x38
#define SPI_RXOICR_OFFSET		0x3c
#define SPI_RXUICR_OFFSET		0x40
#define SPI_MSTICR_OFFSET		0x44
#define SPI_ICR_OFFSET			0x48
#define SPI_DMAC_OFFSET			0x4c
#define SPI_FAULTINJECT_OFFSET	0x50
#define SPI_IDR_OFFSET			0x58
#define SPI_VERSION_ID_OFFSET	0x5c
#define SPI_DR_OFFSET			0x60

#define SPI_SSIENPOLR_OFFSET	0x260
#define SPI_SCLK_OUT_DLY_OFFSET	0x264
#define SPI_START_BIT_OFFSET	0x268
/* ==========================================================================*/
/* SPI rw mode */
#define SPI_WRITE_READ			0
#define SPI_WRITE_ONLY			1
#define SPI_READ_ONLY			2

/* SPI enable register */
#define SPI_SSIENR_DISABLE		0
#define SPI_SSIENR_ENABLE		1

/* SPI interrupt mask */
#define SPI_TXEIS_MASK			0x00000001
#define SPI_TXOIS_MASK 			0x00000002
#define SPI_RXUIS_MASK 			0x00000004
#define SPI_RXOIS_MASK 			0x00000008
#define SPI_RXFIS_MASK 			0x00000010
#define SPI_FCRIS_MASK 			0x00000100

/* SPI status register */
#define SPI_SR_BUSY			    0x00000001
#define SPI_SR_TFNF			    0x00000002
#define SPI_SR_TFE			    0x00000004
#define SPI_SR_RFNE			    0x00000008
#define SPI_SR_RFF			    0x00000010
#define SPI_SR_TXE			    0x00000020
#define SPI_SR_DCOL			    0x00000040

/* SPI dma enable register */
#define SPI_DMAC_RX_EN			0x1
#define SPI_DMAC_TX_EN          0x2

/* SPI_FIFO_SIZE */
#define SPI_DATA_FIFO_SIZE_16   0x10
#define SPI_DATA_FIFO_SIZE_32   0x20
#define SPI_DATA_FIFO_SIZE_64   0x40
#define SPI_DATA_FIFO_SIZE_128	0x80
/* ==========================================================================*/
#define	SPI_CS_HIGH			0x00000004	/* chipselect active high? */
#define	SPI_LSB_FIRST		0x00000008	/* per-word bits-on-wire */
#define	SPI_LOOP			0x00000020	/* loopback mode */
#define	SPI_NO_CS			0x00000040	/* 1 dev/bus, no chipselect */
/* ==========================================================================*/
typedef enum {
	SPI_STATE_RESET           = 0x00U,
	SPI_STATE_READY      	  = 0x01U,
	SPI_STATE_BUSY       	  = 0x02U,
	SPI_STATE_BUSY_TX    	  = 0x03U,
	SPI_STATE_BUSY_RX    	  = 0x04U,
	SPI_STATE_BUSY_TX_THEN_RX = 0x05U,
	SPI_STATE_BUSY_TX_AND_RX  = 0x06U,
	SPI_STATE_ERROR           = 0x07U,
	SPI_STATE_ABORT           = 0x08U
} spi_state_t;
/* ==========================================================================*/
#define SPI_EVENT_TX_READY		0x1
#define SPI_EVENT_TX_OVERFLOW	0x2
#define SPI_EVENT_RX_UNDERFLOW	0x4
#define SPI_EVENT_RX_OVERFLOW	0x8
#define SPI_EVENT_RX_READY		0x10
/* ==========================================================================*/
typedef union {
	struct {
		uint32_t		dfs	    : 4;	/* [3:0] */
		uint32_t		frf	    : 2;	/* [5:4] */
		uint32_t		scph	: 1;	/* [6] */
		uint32_t		scpol	: 1;	/* [7] */
		uint32_t		tmod	: 2;	/* [9:8] */
		uint32_t		slv_oe	: 1;	/* [10] */
		uint32_t		srl	    : 1;	/* [11] */
		uint32_t		reserv1	: 5;	/* [16:12] */
		uint32_t		residue	: 1;	/* [17] */
		uint32_t		tx_lsb	: 1;	/* [18] */
		uint32_t		rx_lsb	: 1;	/* [19] */
		uint32_t		reserv2	: 1;	/* [20] */
		uint32_t		fc_en	: 1;	/* [21] */
		uint32_t		rxd_mg	: 4;	/* [25:22] */
		uint32_t		byte_ws	: 1;	/* [26] */
		uint32_t		hold	: 1;	/* [27] */
		uint32_t		reserv3	: 4;	/* [31:28] */
	} s;
	uint32_t	w;
} spi_ctrl0_reg_t;

typedef union {
	struct {
		uint32_t		busy	: 1;	/* [0] */
		uint32_t		tfnf	: 1;	/* [1] */
		uint32_t		tfe	    : 1;	/* [2] */
		uint32_t		rfne	: 1;	/* [3] */
		uint32_t		rff	    : 1;	/* [4] */
		uint32_t		txe	    : 1;	/* [5] */
		uint32_t		dcol	: 1;	/* [6] */
		uint32_t		reserve	: 25;	/* [31:7] */
	} s;
	uint32_t	w;
} spi_status_reg_t;
/* ==========================================================================*/
typedef struct {
	uint8_t	 spi_mode;
	uint8_t	 cfs_dfs;
	uint8_t	 cs_change;
	uint32_t baud_rate;
} spi_cfg_t;

typedef struct {
	uint8_t	 bus_id;
	uint8_t	 cs_id;
	uint8_t	 *buffer;
	uint32_t n_size;
} spi_write_t;

typedef struct {
	uint8_t	 bus_id;
	uint8_t	 cs_id;
	uint8_t  *buffer;
	uint16_t n_size;
} spi_read_t;

typedef struct {
    uint8_t bus_id;
	uint8_t cs_id;
	uint8_t *w_buffer;
	uint8_t *r_buffer;
	uint16_t w_size;
	uint16_t r_size;
} spi_write_then_read_t;

typedef struct {
	uint8_t bus_id;
	uint8_t cs_id;
	uint8_t *w_buffer;
	uint8_t *r_buffer;
	uint16_t n_size;
} spi_write_and_read_t;

int spi_init(int id);
int spi_write(spi_cfg_t *spi_cfg,
 	spi_write_t *spi_write);
int spi_write_then_read(spi_cfg_t *spi_cfg,
	spi_write_then_read_t *spi_write_then_read);
int spi_write_and_read(spi_cfg_t *spi_cfg,
	spi_write_and_read_t *spi_write_and_read);
struct spi_dev *spi_get_bus_device(int id);
/* ==========================================================================*/
#endif