/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/

#include <string.h>
#include <stdio.h>

#include "vt100_line.h"

static void emit_str(vt100_line_t *vt, const char *s)
{
    if (vt->putc_fn == NULL || s == NULL) {
        return;
    }
    while (*s != '\0') {
        vt->putc_fn(*s++);
    }
}

static void emit_back(vt100_line_t *vt, size_t count)
{
    if (vt->putc_fn == NULL || count == 0) {
        return;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1b[%uD", (unsigned int)count);
    emit_str(vt, buf);
}

static void emit_forward(vt100_line_t *vt, size_t count)
{
    if (vt->putc_fn == NULL || count == 0) {
        return;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1b[%uC", (unsigned int)count);
    emit_str(vt, buf);
}

void vt100_line_init(vt100_line_t *vt, void (*putc_fn)(char c), const char * const *commands)
{
    if (vt == NULL) {
        return;
    }
    memset(vt, 0, sizeof(*vt));
    vt->putc_fn = putc_fn;
    vt->commands = commands;
    vt->hist_idx = -1;
}

void vt100_line_reset(vt100_line_t *vt)
{
    if (vt == NULL) {
        return;
    }
    vt->buf[0] = '\0';
    vt->len = 0;
    vt->cursor = 0;
    vt->hist_idx = -1;
    vt->esc_state = 0;
    vt->esc_len = 0;
}

void vt100_line_add_history(vt100_line_t *vt, const char *line)
{
    if (vt == NULL || line == NULL || line[0] == '\0') {
        return;
    }

    /* Avoid duplicate consecutive entries */
    if (vt->hist_count > 0 && strcmp(vt->history[0], line) == 0) {
        return;
    }

    /* Shift older entries */
    size_t limit = (vt->hist_count < VT100_HISTORY_MAX_DEPTH) ?
                   vt->hist_count : (VT100_HISTORY_MAX_DEPTH - 1);
    for (size_t i = limit; i > 0; i--) {
        strncpy(vt->history[i], vt->history[i - 1], VT100_LINE_MAX_LEN - 1);
        vt->history[i][VT100_LINE_MAX_LEN - 1] = '\0';
    }

    strncpy(vt->history[0], line, VT100_LINE_MAX_LEN - 1);
    vt->history[0][VT100_LINE_MAX_LEN - 1] = '\0';

    if (vt->hist_count < VT100_HISTORY_MAX_DEPTH) {
        vt->hist_count++;
    }
}

static void handle_backspace(vt100_line_t *vt)
{
    if (vt->cursor == 0) {
        return;
    }

    if (vt->cursor == vt->len) {
        vt->cursor--;
        vt->len--;
        vt->buf[vt->len] = '\0';
        if (vt->putc_fn) {
            vt->putc_fn('\b');
            vt->putc_fn(' ');
            vt->putc_fn('\b');
        }
    } else {
        /* Mid-line backspace */
        size_t tail_len = vt->len - vt->cursor;
        memmove(&vt->buf[vt->cursor - 1], &vt->buf[vt->cursor], tail_len);
        vt->cursor--;
        vt->len--;
        vt->buf[vt->len] = '\0';

        if (vt->putc_fn) {
            vt->putc_fn('\b');
            for (size_t i = vt->cursor; i < vt->len; i++) {
                vt->putc_fn(vt->buf[i]);
            }
            vt->putc_fn(' ');
            emit_back(vt, tail_len + 1);
        }
    }
}

static void handle_delete(vt100_line_t *vt)
{
    if (vt->cursor >= vt->len) {
        return;
    }

    size_t tail_len = vt->len - vt->cursor - 1;
    memmove(&vt->buf[vt->cursor], &vt->buf[vt->cursor + 1], tail_len);
    vt->len--;
    vt->buf[vt->len] = '\0';

    if (vt->putc_fn) {
        for (size_t i = vt->cursor; i < vt->len; i++) {
            vt->putc_fn(vt->buf[i]);
        }
        vt->putc_fn(' ');
        emit_back(vt, tail_len + 1);
    }
}

static void replace_line_from_history(vt100_line_t *vt, const char *new_text)
{
    if (vt->putc_fn) {
        emit_back(vt, vt->cursor);
        emit_str(vt, "\x1b[K");
    }

    strncpy(vt->buf, new_text, VT100_LINE_MAX_LEN - 1);
    vt->buf[VT100_LINE_MAX_LEN - 1] = '\0';
    vt->len = strlen(vt->buf);
    vt->cursor = vt->len;

    emit_str(vt, vt->buf);
}

static void handle_history_up(vt100_line_t *vt)
{
    if (vt->hist_count == 0) {
        return;
    }

    if (vt->hist_idx == -1) {
        /* Save current line draft */
        strncpy(vt->saved_line, vt->buf, VT100_LINE_MAX_LEN - 1);
        vt->saved_line[VT100_LINE_MAX_LEN - 1] = '\0';
        vt->hist_idx = 0;
    } else if (vt->hist_idx + 1 < (int)vt->hist_count) {
        vt->hist_idx++;
    } else {
        return; /* Reached oldest */
    }

    replace_line_from_history(vt, vt->history[vt->hist_idx]);
}

static void handle_history_down(vt100_line_t *vt)
{
    if (vt->hist_idx < 0) {
        return;
    }

    if (vt->hist_idx > 0) {
        vt->hist_idx--;
        replace_line_from_history(vt, vt->history[vt->hist_idx]);
    } else {
        /* Return to draft */
        vt->hist_idx = -1;
        replace_line_from_history(vt, vt->saved_line);
    }
}

static void handle_tab(vt100_line_t *vt)
{
    if (vt->commands == NULL || vt->cursor == 0) {
        return;
    }

    /* Only complete the first token */
    size_t space_idx = 0;
    while (space_idx < vt->cursor && vt->buf[space_idx] != ' ') {
        space_idx++;
    }
    if (space_idx != vt->cursor) {
        return; /* Already past first word */
    }

    const char *match = NULL;
    size_t match_count = 0;

    for (size_t i = 0; vt->commands[i] != NULL; i++) {
        if (strncmp(vt->commands[i], vt->buf, vt->cursor) == 0) {
            match = vt->commands[i];
            match_count++;
        }
    }

    if (match_count == 1 && match != NULL) {
        size_t full_len = strlen(match);
        if (full_len >= vt->cursor) {
            size_t add_len = full_len - vt->cursor;
            if (vt->len + add_len + 1 < VT100_LINE_MAX_LEN - 1) {
                /* Append the rest of the command plus space */
                for (size_t j = vt->cursor; j < full_len; j++) {
                    vt->buf[vt->len++] = match[j];
                    if (vt->putc_fn) {
                        vt->putc_fn(match[j]);
                    }
                }
                vt->buf[vt->len++] = ' ';
                vt->buf[vt->len] = '\0';
                vt->cursor = vt->len;
                if (vt->putc_fn) {
                    vt->putc_fn(' ');
                }
            }
        }
    }
}

int vt100_line_feed(vt100_line_t *vt, char c)
{
    if (vt == NULL) {
        return 0;
    }

    /* State machine for ANSI escape sequences */
    if (vt->esc_state == 1) {
        if (c == '[') {
            vt->esc_state = 2;
            vt->esc_len = 0;
            return 0;
        } else if (c == 'O') {
            vt->esc_state = 2;
            vt->esc_len = 0;
            return 0;
        }
        vt->esc_state = 0;
        return 0;
    }

    if (vt->esc_state == 2) {
        if (c >= '0' && c <= '9') {
            if (vt->esc_len < sizeof(vt->esc_buf) - 1) {
                vt->esc_buf[vt->esc_len++] = c;
            }
            return 0;
        }
        if (c == '~') {
            vt->esc_buf[vt->esc_len] = '\0';
            if (strcmp(vt->esc_buf, "3") == 0) {
                handle_delete(vt);
            } else if (strcmp(vt->esc_buf, "1") == 0 || strcmp(vt->esc_buf, "7") == 0) {
                /* Home */
                emit_back(vt, vt->cursor);
                vt->cursor = 0;
            } else if (strcmp(vt->esc_buf, "4") == 0 || strcmp(vt->esc_buf, "8") == 0) {
                /* End */
                emit_forward(vt, vt->len - vt->cursor);
                vt->cursor = vt->len;
            }
            vt->esc_state = 0;
            return 0;
        }

        vt->esc_state = 0;
        switch (c) {
            case 'A': /* Up */
                handle_history_up(vt);
                return 0;
            case 'B': /* Down */
                handle_history_down(vt);
                return 0;
            case 'C': /* Right */
                if (vt->cursor < vt->len) {
                    vt->cursor++;
                    emit_str(vt, "\x1b[C");
                }
                return 0;
            case 'D': /* Left */
                if (vt->cursor > 0) {
                    vt->cursor--;
                    emit_str(vt, "\x1b[D");
                }
                return 0;
            case 'H': /* Home */
                emit_back(vt, vt->cursor);
                vt->cursor = 0;
                return 0;
            case 'F': /* End */
                emit_forward(vt, vt->len - vt->cursor);
                vt->cursor = vt->len;
                return 0;
            default:
                return 0;
        }
    }

    /* Normal character handling */
    if (c == '\x1b') {
        vt->esc_state = 1;
        return 0;
    }

    if (c == '\r' || c == '\n') {
        emit_str(vt, "\r\n");
        vt->buf[vt->len] = '\0';
        if (vt->len > 0) {
            vt100_line_add_history(vt, vt->buf);
        }
        vt->hist_idx = -1;
        return 1; /* Complete line ready */
    }

    if (c == '\b' || c == 0x7F) {
        handle_backspace(vt);
        return 0;
    }

    if (c == 0x01) { /* Ctrl-A: Home */
        emit_back(vt, vt->cursor);
        vt->cursor = 0;
        return 0;
    }

    if (c == 0x05) { /* Ctrl-E: End */
        emit_forward(vt, vt->len - vt->cursor);
        vt->cursor = vt->len;
        return 0;
    }

    if (c == 0x03) { /* Ctrl-C: Cancel line */
        emit_str(vt, "^C\r\n");
        vt100_line_reset(vt);
        return -1;
    }

    if (c == 0x0C) { /* Ctrl-L: Clear screen */
        emit_str(vt, "\x1b[2J\x1b[H");
        emit_str(vt, "r52-freertos> ");
        emit_str(vt, vt->buf);
        emit_back(vt, vt->len - vt->cursor);
        return 0;
    }

    if (c == '\t') {
        handle_tab(vt);
        return 0;
    }

    /* Printable characters */
    if ((unsigned char)c >= 0x20 && (unsigned char)c <= 0x7E) {
        if (vt->len >= VT100_LINE_MAX_LEN - 1) {
            return 0; /* Line buffer full */
        }

        if (vt->cursor == vt->len) {
            vt->buf[vt->len++] = c;
            vt->cursor++;
            vt->buf[vt->len] = '\0';
            if (vt->putc_fn) {
                vt->putc_fn(c);
            }
        } else {
            /* Mid-line insertion */
            size_t tail_len = vt->len - vt->cursor;
            memmove(&vt->buf[vt->cursor + 1], &vt->buf[vt->cursor], tail_len);
            vt->buf[vt->cursor] = c;
            vt->cursor++;
            vt->len++;
            vt->buf[vt->len] = '\0';

            if (vt->putc_fn) {
                for (size_t i = vt->cursor - 1; i < vt->len; i++) {
                    vt->putc_fn(vt->buf[i]);
                }
                emit_back(vt, tail_len);
            }
        }
    }

    return 0;
}
