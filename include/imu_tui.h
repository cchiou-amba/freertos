/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __IMU_TUI_H__
#define __IMU_TUI_H__

#include <stdint.h>
#include <stddef.h>

#include "icm20648.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t frame_count;
    uint32_t rate_hz;
    int paused;
    icm20648_sample_t cur;
    icm20648_sample_t min;
    icm20648_sample_t max;
    int32_t tare_accel_mg[3];
    int32_t tare_gyro_mdps[3];
} imu_tui_state_t;

int imu_tui_format_frame(char *buf, size_t buf_size, const imu_tui_state_t *st);
void imu_tui_run(void);

#ifdef __cplusplus
}
#endif

#endif /* __IMU_TUI_H__ */
