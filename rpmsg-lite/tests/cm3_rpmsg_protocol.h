/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 * cm3_rpmsg_protocol.h
 *
 * Linux (AArch64) <-> CM3 FreeRTOS RPMSG command protocol.
 * Keep this file in sync with unit_test/linux/misc/cm3_rpmsg_protocol.h
 *
 * Frame layout (little-endian, packed, 18-byte header):
 *
 *   magic(u16) | version(u8) | flags(u8) | cmd(u16) | seq(u16)
 *   delay_ms(u16) | status(i32) | payload_len(u16) | crc16(u16) | payload[]
 *
 * delay_ms: host-requested FreeRTOS sleep before handling REQ; default 0.
 *
 * CRC16-Modbus (poly 0xA001, init 0xFFFF):
 *   Compute over [header with crc16=0 | payload].
 *
 * Transaction:
 *   Host -> CM3 : flags = REQ | ACK_REQ, status ignored
 *   CM3  -> Host: flags = RSP [| ERR], status = CM3_STATUS_*, delay_ms echoed
 *
 * CM3_CMD_STRESS: after ACK, both sides switch to raw (unframed) data
 * phase for throughput measurement (legacy stress behavior).
 *
 * CM3_CMD_WAKEUP_AUTO_CFG:
 *   REQ payload: cm3_wakeup_auto_cfg_t (delay_ms).
 *   CM3 sets srs_auto_wkup_enable=1 and srs_auto_wkup_delay_ms=delay_ms,
 *   then returns RSP echoing the config. After next SRS sleep, CM3 waits
 *   delay_ms then auto-wakes (see cmvi_pwr.c).
 ******************************************************************************/
#ifndef CM3_RPMSG_PROTOCOL_H_
#define CM3_RPMSG_PROTOCOL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CM3_RPMSG_MAGIC             (0x4D43u) /* 'CM' */
#define CM3_RPMSG_VERSION           (2u)

#define CM3_RPMSG_MAX_FRAME         (492u)
#define CM3_RPMSG_HDR_SIZE          (18u)
#define CM3_RPMSG_MAX_PAYLOAD       (CM3_RPMSG_MAX_FRAME - CM3_RPMSG_HDR_SIZE)
#define CM3_RPMSG_DELAY_MS_MAX      (60000u)

/* flags */
#define CM3_FLG_REQ                 (1u << 0)
#define CM3_FLG_RSP                 (1u << 1)
#define CM3_FLG_ACK_REQ             (1u << 2)
#define CM3_FLG_ERR                 (1u << 3)

/* commands */
#define CM3_CMD_PING                (0x0001u)
#define CM3_CMD_ECHO                (0x0002u)
#define CM3_CMD_GET_INFO            (0x0003u)
#define CM3_CMD_STRESS              (0x0004u)
#define CM3_CMD_WAKEUP_AUTO_CFG     (0x0005u)

/* status */
#define CM3_STATUS_OK               (0)
#define CM3_STATUS_EINVAL           (-1)
#define CM3_STATUS_ENOSYS           (-2)
#define CM3_STATUS_EIO              (-3)
#define CM3_STATUS_EBADCRC          (-4)
#define CM3_STATUS_ETOOBIG          (-5)

#ifdef __GNUC__
#define CM3_RPMSG_PACKED            __attribute__((packed))
#else
#define CM3_RPMSG_PACKED
#endif

typedef struct CM3_RPMSG_PACKED {
	uint16_t magic;
	uint8_t  version;
	uint8_t  flags;
	uint16_t cmd;
	uint16_t seq;
	uint16_t delay_ms;          /* FreeRTOS sleep before handle; default 0 */
	int32_t  status;
	uint16_t payload_len;
	uint16_t crc16;
} cm3_rpmsg_hdr_t;

typedef struct CM3_RPMSG_PACKED {
	uint32_t loop;
	uint32_t payload_sz;
} cm3_stress_param_t;

typedef struct CM3_RPMSG_PACKED {
	uint32_t proto_ver;
	uint32_t local_ept;
	uint32_t tick_count;
	char     name[28];
} cm3_info_rsp_t;

/* Linux -> CM3 / echoed in RSP: SRS auto-wakeup config */
typedef struct CM3_RPMSG_PACKED {
	uint32_t delay_ms;          /* ms to wait after SRS enter before auto wkup */
} cm3_wakeup_auto_cfg_t;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(cm3_rpmsg_hdr_t) == CM3_RPMSG_HDR_SIZE,
			   "cm3_rpmsg_hdr_t size mismatch");
#endif

static inline uint16_t cm3_rpmsg_crc16(const uint8_t *data, uint32_t len)
{
	uint32_t i;
	uint16_t crc = 0xFFFFu;

	while (len--) {
		crc ^= *data++;
		for (i = 0; i < 8u; i++) {
			if (crc & 0x0001u) {
				crc = (uint16_t)((crc >> 1) ^ 0xA001u);
			} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}

/* CRC over header(crc16 forced 0) + payload */
static inline uint16_t cm3_rpmsg_calc_crc(const cm3_rpmsg_hdr_t *hdr,
										  const uint8_t *payload)
{
	cm3_rpmsg_hdr_t tmp;
	uint16_t crc;

	tmp = *hdr;
	tmp.crc16 = 0;
	crc = cm3_rpmsg_crc16((const uint8_t *)&tmp, sizeof(tmp));
	if (payload != NULL && hdr->payload_len > 0u) {
		uint32_t n = hdr->payload_len;
		uint32_t i;
		while (n--) {
			crc ^= *payload++;
			for (i = 0; i < 8u; i++) {
				if (crc & 0x0001u) {
					crc = (uint16_t)((crc >> 1) ^ 0xA001u);
				} else {
					crc >>= 1;
				}
			}
		}
	}
	return crc;
}

static inline void cm3_rpmsg_memcpy(void *dst, const void *src, uint32_t n)
{
	uint8_t *d = (uint8_t *)dst;
	const uint8_t *s = (const uint8_t *)src;
	while (n--) {
		*d++ = *s++;
	}
}

/**
 * Build a frame into buf.
 * @param delay_ms  Requested FreeRTOS delay in ms; use 0 when unused.
 * @return total frame length on success, negative CM3_STATUS_* on error.
 */
static inline int cm3_rpmsg_build_frame(uint8_t *buf, uint32_t buf_sz,
										uint16_t cmd, uint8_t flags,
										uint16_t seq, int32_t status,
										uint16_t delay_ms,
										const void *payload, uint16_t payload_len)
{
	cm3_rpmsg_hdr_t *hdr;
	uint32_t total = (uint32_t)sizeof(cm3_rpmsg_hdr_t) + (uint32_t)payload_len;

	if (buf == NULL || total > buf_sz || total > CM3_RPMSG_MAX_FRAME) {
		return CM3_STATUS_ETOOBIG;
	}
	if (payload_len > 0u && payload == NULL) {
		return CM3_STATUS_EINVAL;
	}

	hdr = (cm3_rpmsg_hdr_t *)buf;
	hdr->magic = CM3_RPMSG_MAGIC;
	hdr->version = (uint8_t)CM3_RPMSG_VERSION;
	hdr->flags = flags;
	hdr->cmd = cmd;
	hdr->seq = seq;
	hdr->delay_ms = delay_ms;
	hdr->status = status;
	hdr->payload_len = payload_len;
	hdr->crc16 = 0;

	if (payload_len > 0u) {
		cm3_rpmsg_memcpy(buf + sizeof(cm3_rpmsg_hdr_t), payload, payload_len);
	}

	hdr->crc16 = cm3_rpmsg_calc_crc(hdr, buf + sizeof(cm3_rpmsg_hdr_t));
	return (int)total;
}

/**
 * Validate and parse a received frame.
 * @return CM3_STATUS_OK on success.
 */
static inline int cm3_rpmsg_parse_frame(const uint8_t *buf, uint32_t len,
										cm3_rpmsg_hdr_t *out_hdr,
										const uint8_t **out_payload)
{
	const cm3_rpmsg_hdr_t *hdr;
	uint16_t expect;

	if (buf == NULL || out_hdr == NULL || len < sizeof(cm3_rpmsg_hdr_t)) {
		return CM3_STATUS_EINVAL;
	}

	hdr = (const cm3_rpmsg_hdr_t *)buf;
	if (hdr->magic != CM3_RPMSG_MAGIC) {
		return CM3_STATUS_EINVAL;
	}
	if (hdr->version != (uint8_t)CM3_RPMSG_VERSION) {
		return CM3_STATUS_ENOSYS;
	}
	if (((uint32_t)sizeof(cm3_rpmsg_hdr_t) + (uint32_t)hdr->payload_len) != len) {
		return CM3_STATUS_EINVAL;
	}
	if (hdr->payload_len > CM3_RPMSG_MAX_PAYLOAD) {
		return CM3_STATUS_ETOOBIG;
	}

	expect = cm3_rpmsg_calc_crc(hdr, buf + sizeof(cm3_rpmsg_hdr_t));
	if (expect != hdr->crc16) {
		return CM3_STATUS_EBADCRC;
	}

	*out_hdr = *hdr;
	if (out_payload != NULL) {
		*out_payload = (hdr->payload_len > 0u) ?
					   (buf + sizeof(cm3_rpmsg_hdr_t)) : NULL;
	}
	return CM3_STATUS_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* CM3_RPMSG_PROTOCOL_H_ */
