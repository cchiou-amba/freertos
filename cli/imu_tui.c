/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "imu_tui.h"
#include "icm20648.h"

#ifndef R52_HOST_TEST
#include <uart.h>
#endif

static void format_gauge(char *out, size_t out_size, int32_t val, int32_t max_val)
{
    if (out_size < 24) {
        return;
    }

    /* 21 chars: [10 chars negative | 10 chars positive] */
    char bar[22];
    memset(bar, ' ', 21);
    bar[10] = '|';
    bar[21] = '\0';

    if (max_val > 0) {
        if (val > 0) {
            int bars = (int)((val * 10) / max_val);
            if (bars > 10) bars = 10;
            for (int i = 0; i < bars; i++) {
                bar[11 + i] = '=';
            }
        } else if (val < 0) {
            int bars = (int)(((-val) * 10) / max_val);
            if (bars > 10) bars = 10;
            for (int i = 0; i < bars; i++) {
                bar[9 - i] = '=';
            }
        }
    }

    snprintf(out, out_size, "[%s]", bar);
}

int imu_tui_format_frame(char *buf, size_t buf_size, const imu_tui_state_t *st)
{
    if (buf == NULL || buf_size < 512 || st == NULL) {
        return -1;
    }

    char gauge_ax[32], gauge_ay[32], gauge_az[32];
    char gauge_gx[32], gauge_gy[32], gauge_gz[32];

    format_gauge(gauge_ax, sizeof(gauge_ax), st->cur.accel_mg[0], 2000);
    format_gauge(gauge_ay, sizeof(gauge_ay), st->cur.accel_mg[1], 2000);
    format_gauge(gauge_az, sizeof(gauge_az), st->cur.accel_mg[2], 2000);

    format_gauge(gauge_gx, sizeof(gauge_gx), st->cur.gyro_mdps[0] / 1000, 250);
    format_gauge(gauge_gy, sizeof(gauge_gy), st->cur.gyro_mdps[1] / 1000, 250);
    format_gauge(gauge_gz, sizeof(gauge_gz), st->cur.gyro_mdps[2] / 1000, 250);

    int roll_d = st->cur.roll_decideg;
    int roll_int = roll_d / 10;
    int roll_frac = (roll_d < 0 ? -roll_d : roll_d) % 10;

    int pitch_d = st->cur.pitch_decideg;
    int pitch_int = pitch_d / 10;
    int pitch_frac = (pitch_d < 0 ? -pitch_d : pitch_d) % 10;

    int temp_c = st->cur.temp_centi_c;
    int temp_int = temp_c / 100;
    int temp_frac = (temp_c < 0 ? -temp_c : temp_c) % 100;

    int len = snprintf(buf, buf_size,
        "\x1b[H"
        "+----------------------------------------------------------------------+\r\n"
        "|            AMBARELLA N1-655 CORTEX-R52 IMU MONITOR                   |\r\n"
        "| [TDK ICM-20648 | SI_SSI1:CS0 | 0xE0]  Frame: %-6u  Rate: %2u Hz %s |\r\n"
        "+----------------------------------------------------------------------+\r\n"
        "| ACC-X: %+5d mg %s  Min: %+5d  Max: %+5d |\r\n"
        "| ACC-Y: %+5d mg %s  Min: %+5d  Max: %+5d |\r\n"
        "| ACC-Z: %+5d mg %s  Min: %+5d  Max: %+5d |\r\n"
        "+----------------------------------------------------------------------+\r\n"
        "| GYR-X: %+5d dps %s  Min: %+5d  Max: %+5d |\r\n"
        "| GYR-Y: %+5d dps %s  Min: %+5d  Max: %+5d |\r\n"
        "| GYR-Z: %+5d dps %s  Min: %+5d  Max: %+5d |\r\n"
        "+-------------------------- ATTITUDE & TILT ---------------------------+\r\n"
        "| Roll: %+4d.%01d deg | Pitch: %+4d.%01d deg | |G|: %4d mg | Temp: %2d.%02d C |\r\n"
        "+----------------------------------------------------------------------+\r\n"
        "| [q/Ctrl-C] Exit  [z] Tare Zero  [r] Reset Min/Max  [p] Pause  [+/-] Rate |\r\n"
        "+----------------------------------------------------------------------+\r\n",
        (unsigned int)st->frame_count,
        (unsigned int)st->rate_hz,
        st->paused ? "[PAUSED]" : "        ",
        (int)st->cur.accel_mg[0], gauge_ax, (int)st->min.accel_mg[0], (int)st->max.accel_mg[0],
        (int)st->cur.accel_mg[1], gauge_ay, (int)st->min.accel_mg[1], (int)st->max.accel_mg[1],
        (int)st->cur.accel_mg[2], gauge_az, (int)st->min.accel_mg[2], (int)st->max.accel_mg[2],
        (int)(st->cur.gyro_mdps[0] / 1000), gauge_gx,
        (int)(st->min.gyro_mdps[0] / 1000), (int)(st->max.gyro_mdps[0] / 1000),
        (int)(st->cur.gyro_mdps[1] / 1000), gauge_gy,
        (int)(st->min.gyro_mdps[1] / 1000), (int)(st->max.gyro_mdps[1] / 1000),
        (int)(st->cur.gyro_mdps[2] / 1000), gauge_gz,
        (int)(st->min.gyro_mdps[2] / 1000), (int)(st->max.gyro_mdps[2] / 1000),
        roll_int, roll_frac,
        pitch_int, pitch_frac,
        (int)st->cur.g_norm_mg,
        temp_int, temp_frac);

    return (len > 0 && (size_t)len < buf_size) ? 0 : -1;
}

#ifndef R52_HOST_TEST
void imu_tui_run(void)
{
    imu_tui_state_t st;
    static char frame_buf[2048];

    memset(&st, 0, sizeof(st));
    st.rate_hz = 10;
    st.frame_count = 0;
    st.paused = 0;

    for (int i = 0; i < 3; i++) {
        st.min.accel_mg[i] = 32767;
        st.max.accel_mg[i] = -32768;
        st.min.gyro_mdps[i] = 2000000;
        st.max.gyro_mdps[i] = -2000000;
    }

    /* Hide cursor and clear screen */
    printf("\x1b[?25l\x1b[2J\x1b[H");

    int running = 1;
    while (running) {
        if (!st.paused) {
            icm20648_sample_t raw_sample;
            if (icm20648_read_sample(&raw_sample) == 0) {
                st.cur = raw_sample;

                /* Apply tare offset */
                for (int i = 0; i < 3; i++) {
                    st.cur.accel_mg[i] -= st.tare_accel_mg[i];
                    st.cur.gyro_mdps[i] -= st.tare_gyro_mdps[i];
                }

                /* Update min / max */
                for (int i = 0; i < 3; i++) {
                    if (st.cur.accel_mg[i] < st.min.accel_mg[i])
                        st.min.accel_mg[i] = st.cur.accel_mg[i];
                    if (st.cur.accel_mg[i] > st.max.accel_mg[i])
                        st.max.accel_mg[i] = st.cur.accel_mg[i];

                    if (st.cur.gyro_mdps[i] < st.min.gyro_mdps[i])
                        st.min.gyro_mdps[i] = st.cur.gyro_mdps[i];
                    if (st.cur.gyro_mdps[i] > st.max.gyro_mdps[i])
                        st.max.gyro_mdps[i] = st.cur.gyro_mdps[i];
                }
                st.frame_count++;
            }
        }

        if (imu_tui_format_frame(frame_buf, sizeof(frame_buf), &st) == 0) {
            printf("%s", frame_buf);
        }

        uint32_t period_ms = (st.rate_hz > 0) ? (1000U / st.rate_hz) : 100U;
        int ch = console_getc_timeout(period_ms);
        if (ch >= 0) {
            if (ch == 'q' || ch == 'Q' || ch == 0x03) {
                running = 0;
            } else if (ch == 'p' || ch == 'P') {
                st.paused = !st.paused;
            } else if (ch == 'r' || ch == 'R') {
                for (int i = 0; i < 3; i++) {
                    st.min.accel_mg[i] = st.cur.accel_mg[i];
                    st.max.accel_mg[i] = st.cur.accel_mg[i];
                    st.min.gyro_mdps[i] = st.cur.gyro_mdps[i];
                    st.max.gyro_mdps[i] = st.cur.gyro_mdps[i];
                }
            } else if (ch == 'z' || ch == 'Z') {
                for (int i = 0; i < 3; i++) {
                    st.tare_accel_mg[i] += st.cur.accel_mg[i];
                    st.tare_gyro_mdps[i] += st.cur.gyro_mdps[i];
                }
            } else if (ch == '+' || ch == '=') {
                if (st.rate_hz < 50) st.rate_hz += 5;
            } else if (ch == '-' || ch == '_') {
                if (st.rate_hz > 5) st.rate_hz -= 5;
            }
        }
    }

    /* Restore cursor and clear screen */
    printf("\x1b[?25h\x1b[0m\r\n");
}
#endif
