/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __AMBA_SPI_H__
#define __AMBA_SPI_H__

#include <common.h>
#include <list.h>

#define	SPI_CPHA			0x00000001	/* clock phase */
#define	SPI_CPOL			0x00000002	/* clock polarity */

#define	SPI_MODE_0			(0|0)		/* (original MicroWire) */
#define	SPI_MODE_1			(0|SPI_CPHA)
#define	SPI_MODE_2			(SPI_CPOL|0)
#define	SPI_MODE_3			(SPI_CPOL|SPI_CPHA)
#define	SPI_MODE_X_MASK		(SPI_CPOL|SPI_CPHA)

#define	SPI_CS_HIGH			0x00000004	/* chipselect active high? */
#define	SPI_LSB_FIRST		0x00000008	/* per-word bits-on-wire */
#define	SPI_LOOP			0x00000020	/* loopback mode */
#define	SPI_NO_CS			0x00000040	/* 1 dev/bus, no chipselect */

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
#define SPI_MSTICR_OFFSET		0x44	/* clear @CV2FS */
#define SPI_ICR_OFFSET			0x48
#define SPI_DMAC_OFFSET			0x4c
#define SPI_FAULTINJECT_OFFSET		0x50	/* Introduced @CV2FS */
#define SPI_IDR_OFFSET			0x58
#define SPI_VERSION_ID_OFFSET		0x5c
#define SPI_DR_OFFSET			0x60

#define SPI_SSIENPOLR_OFFSET		0x260
#define SPI_SCLK_OUT_DLY_OFFSET		0x264
#define SPI_START_BIT_OFFSET		0x268

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
#define SPI_DESELECT_EDGE_MASK	0x00000400

/* SPI status register */
#define SPI_SR_BUSY			0x00000001
#define SPI_SR_TFNF			0x00000002
#define SPI_SR_TFE			0x00000004
#define SPI_SR_RFNE			0x00000008
#define SPI_SR_RFF			0x00000010
#define SPI_SR_TXE			0x00000020
#define SPI_SR_DCOL			0x00000040

/* SPI dma enable register */
#define SPI_DMAC_RX_EN			0x1
#define SPI_DMAC_TX_EN			0x2

/* SPI_FIFO_SIZE */
#define SPI_DATA_FIFO_SIZE_16		0x10
#define SPI_DATA_FIFO_SIZE_32		0x20
#define SPI_DATA_FIFO_SIZE_64		0x40
#define SPI_DATA_FIFO_SIZE_128		0x80

/* ==========================================================================*/

typedef enum {
	SPI_STATE_RESET      = 0x00U,
	SPI_STATE_READY      = 0x01U,
	SPI_STATE_BUSY       = 0x02U,
	SPI_STATE_BUSY_TX    = 0x03U,
	SPI_STATE_BUSY_RX    = 0x04U,
	SPI_STATE_BUSY_TX_RX = 0x05U,
	SPI_STATE_ERROR      = 0x06U,
	SPI_STATE_ABORT      = 0x07U
} spi_state_t;

typedef union {
	struct {
		uint32_t		dfs	: 4;	/* [3:0] */
		uint32_t		frf	: 2;	/* [5:4] */
		uint32_t		scph	: 1;	/* [6] */
		uint32_t		scpol	: 1;	/* [7] */
		uint32_t		tmod	: 2;	/* [9:8] */
		uint32_t		slv_oe	: 1;	/* [10] */
		uint32_t		srl	: 1;	/* [11] */
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
		uint32_t		tfe	: 1;	/* [2] */
		uint32_t		rfne	: 1;	/* [3] */
		uint32_t		rff	: 1;	/* [4] */
		uint32_t		txe	: 1;	/* [5] */
		uint32_t		dcol	: 1;	/* [6] */
		uint32_t		reserve	: 25;	/* [31:7] */
	} s;
	uint32_t	w;
} spi_status_reg_t;

typedef struct spi_cfg_s {
	uint8_t		master;
	uint32_t	mode;
	uint8_t		bits_per_word;
	uint8_t		tmod;
	uint16_t	chip_select;
	uint32_t	bitrate;
	uint32_t	spiclk;
} spi_cfg_t;

typedef struct spi_s {
	volatile uint8_t	*base;
	char			name[16];
	int			irq;
	spi_cfg_t		cfg;
	uint8_t			*tx_buf, *rx_buf;
	uint16_t		tx_len, rx_len;
	uint16_t		tx_cnt, rx_cnt;
#define SPI_MAX_CS_NUM	(8)
	int16_t			cs_gpios[SPI_MAX_CS_NUM];
	int16_t			notify_gpio;
	spi_state_t		state;
	ListItem_t		dev;
	void (*tx_isr)(struct spi_s *spi);
	void (*rx_isr)(struct spi_s *spi);
	void (*res_init)(struct spi_s *spi);
	void (*xfer_cplt)(struct spi_s *spi);
} spi_t;

/* ==========================================================================*/
extern int spi_xfer_poll(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, uint16_t size, uint32_t timeout);
extern int spi_xfer_intr(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, uint16_t size);
extern void spi_isr(spi_t *spi);
extern spi_t *spi_dev(char *name);
extern int spi_init(spi_t *spi);
extern void spi_task(void *param);

#endif /* __CM3_SPI_H__ */
