/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __ICM20648_H__
#define __ICM20648_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ICM20648_SPI_BUS_ID             1U
#define ICM20648_SPI_CS_ID              0U
#define ICM20648_WHOAMI_EXPECT          0xE0U

/* Register addresses in Bank 0 */
#define ICM20648_REG_BANK_SEL           0x7FU
#define ICM20648_REG_WHOAMI             0x00U
#define ICM20648_REG_USER_CTRL          0x03U
#define ICM20648_REG_LP_CONFIG          0x05U
#define ICM20648_REG_PWR_MGMT_1         0x06U
#define ICM20648_REG_PWR_MGMT_2         0x07U
#define ICM20648_REG_ACCEL_X_H          0x2DU

/* Register addresses in Bank 2 */
#define ICM20648_REG_GYRO_SMPLRT_DIV    0x00U
#define ICM20648_REG_GYRO_CONFIG_1      0x01U
#define ICM20648_REG_ACCEL_SMPLRT_DIV_2 0x11U
#define ICM20648_REG_ACCEL_CONFIG       0x14U

typedef struct {
    int16_t accel_raw[3];     /* X, Y, Z raw 16-bit signed */
    int16_t gyro_raw[3];      /* X, Y, Z raw 16-bit signed */
    int16_t temp_raw;         /* raw temperature */
    int32_t accel_mg[3];      /* X, Y, Z acceleration in milli-g (±16384 -> ±1000 mg) */
    int32_t accel_mms2[3];    /* X, Y, Z acceleration in mm/s^2 (±16384 -> ±9807 mm/s^2) */
    int32_t gyro_mdps[3];     /* X, Y, Z angular rate in milli-deg/s (±131 LSB/dps -> ±250000 mdps) */
    int32_t temp_centi_c;     /* hundredths of deg C: ((raw - 21) * 10000) / 33387 + 2100 */
    int32_t roll_decideg;     /* roll angle in tenths of a degree (-1800..+1800) */
    int32_t pitch_decideg;    /* pitch angle in tenths of a degree (-900..+900) */
    int32_t g_norm_mg;        /* norm of acceleration vector in mg */
} icm20648_sample_t;

typedef struct {
    int (*write_then_read)(uint8_t bus_id, uint8_t cs_id,
                           const uint8_t *w_buf, size_t w_len,
                           uint8_t *r_buf, size_t r_len);
    void (*delay_ms)(uint32_t ms);
} icm20648_io_ops_t;

void icm20648_set_io_ops(const icm20648_io_ops_t *ops);
int icm20648_read_whoami(uint8_t *whoami_out);
int icm20648_init(void);
int icm20648_parse_raw_sample(const uint8_t raw14[14], icm20648_sample_t *out);
int icm20648_read_sample(icm20648_sample_t *out);

#ifdef __cplusplus
}
#endif

#endif /* __ICM20648_H__ */
