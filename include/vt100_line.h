/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#ifndef __VT100_LINE_H__
#define __VT100_LINE_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VT100_LINE_MAX_LEN      128U
#define VT100_HISTORY_MAX_DEPTH 8U

typedef struct {
    char buf[VT100_LINE_MAX_LEN];
    size_t len;
    size_t cursor;
    char history[VT100_HISTORY_MAX_DEPTH][VT100_LINE_MAX_LEN];
    size_t hist_count;
    int hist_idx;
    char saved_line[VT100_LINE_MAX_LEN];
    int esc_state;
    char esc_buf[8];
    size_t esc_len;
    void (*putc_fn)(char c);
    const char * const *commands;
} vt100_line_t;

void vt100_line_init(vt100_line_t *vt, void (*putc_fn)(char c), const char * const *commands);
void vt100_line_reset(vt100_line_t *vt);
void vt100_line_add_history(vt100_line_t *vt, const char *line);
int vt100_line_feed(vt100_line_t *vt, char c);

#ifdef __cplusplus
}
#endif

#endif /* __VT100_LINE_H__ */
