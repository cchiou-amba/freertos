/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "icm20648.h"

#ifndef R52_HOST_TEST
#include <plat.h>
#include <ambhw/spi.h>
#include "FreeRTOS.h"
#include "task.h"

static int r52_spi_write_then_read(uint8_t bus_id, uint8_t cs_id,
                                   const uint8_t *w_buf, size_t w_len,
                                   uint8_t *r_buf, size_t r_len)
{
    spi_cfg_t cfg;
    spi_write_then_read_t xfer;

    memset(&cfg, 0, sizeof(cfg));
    cfg.baud_rate = 5000000U; /* 5 MHz SPI clock */
    cfg.cfs_dfs = 8U;
    cfg.spi_mode = SPI_MODE_3;

    memset(&xfer, 0, sizeof(xfer));
    xfer.bus_id = bus_id;
    xfer.cs_id = cs_id;
    xfer.w_buffer = (u8 *)w_buf;
    xfer.w_size = (u32)w_len;
    xfer.r_buffer = (u8 *)r_buf;
    xfer.r_size = (u32)r_len;

    return spi_write_then_read(&cfg, &xfer);
}

static void r52_delay_ms(uint32_t ms)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskDelay(pdMS_TO_TICKS(ms) > 0 ? pdMS_TO_TICKS(ms) : 1);
    } else {
        /* Busy-wait approximate loop at ~1 GHz */
        for (volatile uint32_t i = 0; i < ms * 25000U; i++) {
            __asm__ __volatile__("nop");
        }
    }
}

static const icm20648_io_ops_t s_default_hw_ops = {
    .write_then_read = r52_spi_write_then_read,
    .delay_ms = r52_delay_ms,
};
#endif

static const icm20648_io_ops_t *s_active_ops =
#ifndef R52_HOST_TEST
    &s_default_hw_ops;
#else
    NULL;
#endif

void icm20648_set_io_ops(const icm20648_io_ops_t *ops)
{
    s_active_ops = ops;
}

static int icm20648_write_reg(uint8_t reg, uint8_t val)
{
    if (s_active_ops == NULL || s_active_ops->write_then_read == NULL) {
        return -1;
    }
    uint8_t buf[2];
    buf[0] = reg & 0x7FU; /* Write has MSB = 0 */
    buf[1] = val;
    return s_active_ops->write_then_read(ICM20648_SPI_BUS_ID, ICM20648_SPI_CS_ID,
                                         buf, 2, NULL, 0);
}

static int icm20648_read_reg(uint8_t reg, uint8_t *val)
{
    if (s_active_ops == NULL || s_active_ops->write_then_read == NULL) {
        return -1;
    }
    uint8_t cmd = reg | 0x80U; /* Read has MSB = 1 */
    return s_active_ops->write_then_read(ICM20648_SPI_BUS_ID, ICM20648_SPI_CS_ID,
                                         &cmd, 1, val, 1);
}

static int icm20648_select_bank(uint8_t bank)
{
    return icm20648_write_reg(ICM20648_REG_BANK_SEL, bank);
}

int icm20648_read_whoami(uint8_t *whoami_out)
{
    if (whoami_out == NULL) {
        return -1;
    }

    int ret = icm20648_select_bank(0x00U);
    if (ret != 0) {
        return ret;
    }

    ret = icm20648_read_reg(ICM20648_REG_WHOAMI, whoami_out);
    if (ret != 0) {
        return ret;
    }

    return (*whoami_out == ICM20648_WHOAMI_EXPECT) ? 0 : -1;
}

int icm20648_init(void)
{
    uint8_t whoami = 0;
    int ret = icm20648_read_whoami(&whoami);
    if (ret != 0 || whoami != ICM20648_WHOAMI_EXPECT) {
        return -1;
    }

    /* 1. Reset chip (PWR_MGMT_1 = 0x80) */
    ret = icm20648_write_reg(ICM20648_REG_PWR_MGMT_1, 0x80U);
    if (ret != 0) {
        return ret;
    }

    if (s_active_ops && s_active_ops->delay_ms) {
        s_active_ops->delay_ms(20);
    }

    /* 2. Select Bank 0 and disable I2C interface (USER_CTRL = 0x10) */
    ret = icm20648_select_bank(0x00U);
    if (ret != 0) {
        return ret;
    }

    ret = icm20648_write_reg(ICM20648_REG_USER_CTRL, 0x10U);
    if (ret != 0) {
        return ret;
    }

    /* 3. Wake chip: auto clock select (PWR_MGMT_1 = 0x01) */
    ret = icm20648_write_reg(ICM20648_REG_PWR_MGMT_1, 0x01U);
    if (ret != 0) {
        return ret;
    }

    /* 4. Enable all accel and gyro axes (PWR_MGMT_2 = 0x00) */
    ret = icm20648_write_reg(ICM20648_REG_PWR_MGMT_2, 0x00U);
    if (ret != 0) {
        return ret;
    }

    /* 5. Set LP_CONFIG = 0x00 (normal power mode) */
    ret = icm20648_write_reg(ICM20648_REG_LP_CONFIG, 0x00U);
    if (ret != 0) {
        return ret;
    }

    /* 6. Switch to Bank 2: configure sample rates and full-scale ranges */
    ret = icm20648_select_bank(0x20U);
    if (ret != 0) {
        return ret;
    }

    /* GYRO_SMPLRT_DIV = 0x01 */
    ret = icm20648_write_reg(ICM20648_REG_GYRO_SMPLRT_DIV, 0x01U);
    if (ret != 0) {
        return ret;
    }

    /* GYRO_CONFIG_1 = 0x09 (±250 dps, DLPF enabled) */
    ret = icm20648_write_reg(ICM20648_REG_GYRO_CONFIG_1, 0x09U);
    if (ret != 0) {
        return ret;
    }

    /* ACCEL_SMPLRT_DIV_2 = 0x01 */
    ret = icm20648_write_reg(ICM20648_REG_ACCEL_SMPLRT_DIV_2, 0x01U);
    if (ret != 0) {
        return ret;
    }

    /* ACCEL_CONFIG = 0x09 (±2g, DLPF enabled) */
    ret = icm20648_write_reg(ICM20648_REG_ACCEL_CONFIG, 0x09U);
    if (ret != 0) {
        return ret;
    }

    /* 7. Switch back to Bank 0 */
    return icm20648_select_bank(0x00U);
}

static uint32_t integer_isqrt(uint64_t val)
{
    if (val == 0ULL) {
        return 0U;
    }
    uint64_t op = val;
    uint64_t res = 0ULL;
    uint64_t one = 1ULL << 62;
    while (one > op) {
        one >>= 2;
    }
    while (one != 0ULL) {
        if (op >= res + one) {
            op -= res + one;
            res = (res >> 1) + one;
        } else {
            res >>= 1;
        }
        one >>= 2;
    }
    return (uint32_t)res;
}

static const uint16_t s_tan_table[451] = {
    0, 17, 35, 52, 70, 87, 105, 122, 140, 157,
    175, 192, 209, 227, 244, 262, 279, 297, 314, 332,
    349, 367, 384, 402, 419, 437, 454, 472, 489, 507,
    524, 542, 559, 577, 594, 612, 629, 647, 664, 682,
    699, 717, 734, 752, 769, 787, 805, 822, 840, 857,
    875, 892, 910, 928, 945, 963, 981, 998, 1016, 1033,
    1051, 1069, 1086, 1104, 1122, 1139, 1157, 1175, 1192, 1210,
    1228, 1246, 1263, 1281, 1299, 1317, 1334, 1352, 1370, 1388,
    1405, 1423, 1441, 1459, 1477, 1495, 1512, 1530, 1548, 1566,
    1584, 1602, 1620, 1638, 1655, 1673, 1691, 1709, 1727, 1745,
    1763, 1781, 1799, 1817, 1835, 1853, 1871, 1890, 1908, 1926,
    1944, 1962, 1980, 1998, 2016, 2035, 2053, 2071, 2089, 2107,
    2126, 2144, 2162, 2180, 2199, 2217, 2235, 2254, 2272, 2290,
    2309, 2327, 2345, 2364, 2382, 2401, 2419, 2438, 2456, 2475,
    2493, 2512, 2530, 2549, 2568, 2586, 2605, 2623, 2642, 2661,
    2679, 2698, 2717, 2736, 2754, 2773, 2792, 2811, 2830, 2849,
    2867, 2886, 2905, 2924, 2943, 2962, 2981, 3000, 3019, 3038,
    3057, 3076, 3096, 3115, 3134, 3153, 3172, 3191, 3211, 3230,
    3249, 3269, 3288, 3307, 3327, 3346, 3365, 3385, 3404, 3424,
    3443, 3463, 3482, 3502, 3522, 3541, 3561, 3581, 3600, 3620,
    3640, 3659, 3679, 3699, 3719, 3739, 3759, 3779, 3799, 3819,
    3839, 3859, 3879, 3899, 3919, 3939, 3959, 3979, 4000, 4020,
    4040, 4061, 4081, 4101, 4122, 4142, 4163, 4183, 4204, 4224,
    4245, 4265, 4286, 4307, 4327, 4348, 4369, 4390, 4411, 4431,
    4452, 4473, 4494, 4515, 4536, 4557, 4578, 4599, 4621, 4642,
    4663, 4684, 4706, 4727, 4748, 4770, 4791, 4813, 4834, 4856,
    4877, 4899, 4921, 4942, 4964, 4986, 5008, 5029, 5051, 5073,
    5095, 5117, 5139, 5161, 5184, 5206, 5228, 5250, 5272, 5295,
    5317, 5340, 5362, 5384, 5407, 5430, 5452, 5475, 5498, 5520,
    5543, 5566, 5589, 5612, 5635, 5658, 5681, 5704, 5727, 5750,
    5774, 5797, 5820, 5844, 5867, 5890, 5914, 5938, 5961, 5985,
    6009, 6032, 6056, 6080, 6104, 6128, 6152, 6176, 6200, 6224,
    6249, 6273, 6297, 6322, 6346, 6371, 6395, 6420, 6445, 6469,
    6494, 6519, 6544, 6569, 6594, 6619, 6644, 6669, 6694, 6720,
    6745, 6771, 6796, 6822, 6847, 6873, 6899, 6924, 6950, 6976,
    7002, 7028, 7054, 7080, 7107, 7133, 7159, 7186, 7212, 7239,
    7265, 7292, 7319, 7346, 7373, 7400, 7427, 7454, 7481, 7508,
    7536, 7563, 7590, 7618, 7646, 7673, 7701, 7729, 7757, 7785,
    7813, 7841, 7869, 7898, 7926, 7954, 7983, 8012, 8040, 8069,
    8098, 8127, 8156, 8185, 8214, 8243, 8273, 8302, 8332, 8361,
    8391, 8421, 8451, 8481, 8511, 8541, 8571, 8601, 8632, 8662,
    8693, 8724, 8754, 8785, 8816, 8847, 8878, 8910, 8941, 8972,
    9004, 9036, 9067, 9099, 9131, 9163, 9195, 9228, 9260, 9293,
    9325, 9358, 9391, 9424, 9457, 9490, 9523, 9556, 9590, 9623,
    9657, 9691, 9725, 9759, 9793, 9827, 9861, 9896, 9930, 9965,
    10000,
};

static int32_t integer_atan2_decideg(int32_t y, int32_t x)
{
    if (x == 0 && y == 0) {
        return 0;
    }
    if (x == 0) {
        return (y > 0) ? 900 : -900;
    }
    if (y == 0) {
        return (x > 0) ? 0 : 1800;
    }

    uint32_t abs_y = (y < 0) ? (uint32_t)(-y) : (uint32_t)y;
    uint32_t abs_x = (x < 0) ? (uint32_t)(-x) : (uint32_t)x;

    uint32_t u = (abs_y < abs_x) ? abs_y : abs_x;
    uint32_t v = (abs_y < abs_x) ? abs_x : abs_y;

    uint32_t ratio = (u * 10000U) / v;

    /* Binary search in s_tan_table[0..450] */
    int low = 0;
    int high = 450;
    int best = 0;
    int min_diff = 20000;

    while (low <= high) {
        int mid = (low + high) / 2;
        int diff = (int)s_tan_table[mid] - (int)ratio;
        int abs_diff = (diff < 0) ? -diff : diff;
        if (abs_diff < min_diff) {
            min_diff = abs_diff;
            best = mid;
        }
        if (diff < 0) {
            low = mid + 1;
        } else if (diff > 0) {
            high = mid - 1;
        } else {
            best = mid;
            break;
        }
    }

    int32_t theta = (int32_t)best;
    if (abs_y > abs_x) {
        theta = 900 - theta;
    }

    /* Quadrant adjustment */
    if (x < 0 && y >= 0) {
        return 1800 - theta;
    } else if (x < 0 && y < 0) {
        return -1800 + theta;
    } else if (x >= 0 && y < 0) {
        return -theta;
    }

    return theta;
}

int icm20648_parse_raw_sample(const uint8_t raw14[14], icm20648_sample_t *out)
{
    if (raw14 == NULL || out == NULL) {
        return -1;
    }

    out->accel_raw[0] = (int16_t)(((uint16_t)raw14[0] << 8) | raw14[1]);
    out->accel_raw[1] = (int16_t)(((uint16_t)raw14[2] << 8) | raw14[3]);
    out->accel_raw[2] = (int16_t)(((uint16_t)raw14[4] << 8) | raw14[5]);

    out->gyro_raw[0]  = (int16_t)(((uint16_t)raw14[6] << 8) | raw14[7]);
    out->gyro_raw[1]  = (int16_t)(((uint16_t)raw14[8] << 8) | raw14[9]);
    out->gyro_raw[2]  = (int16_t)(((uint16_t)raw14[10] << 8) | raw14[11]);

    out->temp_raw     = (int16_t)(((uint16_t)raw14[12] << 8) | raw14[13]);

    for (int i = 0; i < 3; i++) {
        out->accel_mg[i]   = ((int32_t)out->accel_raw[i] * 1000) / 16384;
        out->accel_mms2[i] = ((int32_t)out->accel_raw[i] * 9807) / 16384;
        out->gyro_mdps[i]  = ((int32_t)out->gyro_raw[i] * 1000) / 131;
    }

    out->temp_centi_c = (((int32_t)out->temp_raw - 21) * 10000) / 33387 + 2100;

    int64_t ax = out->accel_mg[0];
    int64_t ay = out->accel_mg[1];
    int64_t az = out->accel_mg[2];
    out->g_norm_mg = (int32_t)integer_isqrt((uint64_t)(ax * ax + ay * ay + az * az));

    out->roll_decideg  = integer_atan2_decideg(out->accel_mg[1], out->accel_mg[2]);
    out->pitch_decideg = integer_atan2_decideg(out->accel_mg[0], out->accel_mg[2]);

    return 0;
}

int icm20648_read_sample(icm20648_sample_t *out)
{
    if (out == NULL) {
        return -1;
    }
    if (s_active_ops == NULL || s_active_ops->write_then_read == NULL) {
        return -1;
    }

    int ret = icm20648_select_bank(0x00U);
    if (ret != 0) {
        return ret;
    }

    uint8_t cmd = ICM20648_REG_ACCEL_X_H | 0x80U;
    uint8_t raw14[14];
    memset(raw14, 0, sizeof(raw14));

    ret = s_active_ops->write_then_read(ICM20648_SPI_BUS_ID, ICM20648_SPI_CS_ID,
                                        &cmd, 1, raw14, 14);
    if (ret != 0) {
        return ret;
    }

    return icm20648_parse_raw_sample(raw14, out);
}
