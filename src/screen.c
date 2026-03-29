/*
 * screen.c — Terminal Renderer Implementation
 *
 * Double-buffered ANSI terminal renderer.
 * Uses a write buffer to batch all output into a single fflush,
 * eliminating flicker. Only draws cells that changed (diff-based).
 *
 * Permitted dependency: <stdio.h> for printf/putchar/fflush
 */

#include "../include/screen.h"
#include <stdio.h>

/* ── Screen Cell ──────────────────────────────────────────────────────── */

typedef struct {
    char ch;
    int  fg;
    int  bg;
} ScreenCell;

/* ── Write Buffer for Batched Output ──────────────────────────────────── */

#define WRITE_BUF_SIZE  (SCR_MAX_WIDTH * SCR_MAX_HEIGHT * 20)

static char  write_buf[WRITE_BUF_SIZE];
static int   write_pos = 0;

static void wb_reset(void)
{
    write_pos = 0;
}

static void wb_char(char c)
{
    if (write_pos < WRITE_BUF_SIZE - 1) {
        write_buf[write_pos++] = c;
    }
}

static void wb_str(const char *s)
{
    while (*s && write_pos < WRITE_BUF_SIZE - 1) {
        write_buf[write_pos++] = *s++;
    }
}

/* Simple int-to-string for ANSI codes (no string.c dependency) */
static void wb_int(int n)
{
    if (n < 0) { wb_char('-'); n = -n; }
    if (n == 0) { wb_char('0'); return; }

    char tmp[12];
    int len = 0;
    while (n > 0) {
        tmp[len++] = '0' + (n % 10);
        n /= 10;
    }
    for (int i = len - 1; i >= 0; i--) {
        wb_char(tmp[i]);
    }
}

static void wb_flush(void)
{
    if (write_pos > 0) {
        write_buf[write_pos] = '\0';
        fputs(write_buf, stdout);
        fflush(stdout);
        write_pos = 0;
    }
}

/* ── Module State ─────────────────────────────────────────────────────── */

static ScreenCell front[SCR_MAX_HEIGHT][SCR_MAX_WIDTH];
static ScreenCell back[SCR_MAX_HEIGHT][SCR_MAX_WIDTH];
static int screen_width  = 80;
static int screen_height = 24;
static int first_frame   = 1;

/* ── Initialization ───────────────────────────────────────────────────── */

void scr_init(int width, int height)
{
    if (width  > SCR_MAX_WIDTH)  width  = SCR_MAX_WIDTH;
    if (height > SCR_MAX_HEIGHT) height = SCR_MAX_HEIGHT;
    if (width  < 1) width  = 1;
    if (height < 1) height = 1;

    screen_width  = width;
    screen_height = height;
    first_frame   = 1;

    /* Initialize both buffers — use sentinel value for front buffer
       so the first frame renders everything */
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            front[y][x].ch = '\0';   /* Sentinel: force first draw */
            front[y][x].fg = -1;
            front[y][x].bg = -1;
            back[y][x].ch  = ' ';
            back[y][x].fg  = COLOR_WHITE;
            back[y][x].bg  = BG_BLACK;
        }
    }
}

/* ── Screen Operations ────────────────────────────────────────────────── */

void scr_clear(void)
{
    printf("\033[2J");   /* Clear entire screen */
    printf("\033[H");    /* Move cursor to home */
    fflush(stdout);
}

void scr_clear_buffer(void)
{
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            back[y][x].ch = ' ';
            back[y][x].fg = COLOR_WHITE;
            back[y][x].bg = BG_BLACK;
        }
    }
}

void scr_move_cursor(int x, int y)
{
    /* ANSI uses 1-indexed row;col */
    printf("\033[%d;%dH", y + 1, x + 1);
}

/* ── Buffer Drawing ───────────────────────────────────────────────────── */

void scr_put_char(int x, int y, char ch, int fg, int bg)
{
    /* Silently ignore out-of-bounds */
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) {
        return;
    }

    back[y][x].ch = ch;
    back[y][x].fg = fg;
    back[y][x].bg = bg;
}

void scr_put_string(int x, int y, const char *s, int fg, int bg)
{
    if (!s) return;

    int i = 0;
    while (s[i] != '\0') {
        scr_put_char(x + i, y, s[i], fg, bg);
        i++;
    }
}

void scr_draw_box(int x, int y, int w, int h, int fg, int bg)
{
    if (w < 2 || h < 2) return;

    /* Top border */
    scr_put_char(x, y, '+', fg, bg);
    for (int i = 1; i < w - 1; i++) {
        scr_put_char(x + i, y, '-', fg, bg);
    }
    scr_put_char(x + w - 1, y, '+', fg, bg);

    /* Side borders */
    for (int j = 1; j < h - 1; j++) {
        scr_put_char(x, y + j, '|', fg, bg);
        scr_put_char(x + w - 1, y + j, '|', fg, bg);
    }

    /* Bottom border */
    scr_put_char(x, y + h - 1, '+', fg, bg);
    for (int i = 1; i < w - 1; i++) {
        scr_put_char(x + i, y + h - 1, '-', fg, bg);
    }
    scr_put_char(x + w - 1, y + h - 1, '+', fg, bg);
}

/* ── Diff-Based Refresh (Batched) ─────────────────────────────────────── */

void scr_refresh(void)
{
    wb_reset();

    int last_fg = -1;
    int last_bg = -1;

    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            /* Only update changed cells */
            if (back[y][x].ch != front[y][x].ch ||
                back[y][x].fg != front[y][x].fg ||
                back[y][x].bg != front[y][x].bg) {

                /* Move cursor: ESC[row;colH */
                wb_str("\033[");
                wb_int(y + 1);
                wb_char(';');
                wb_int(x + 1);
                wb_char('H');

                /* Set colors only if they changed */
                if (back[y][x].fg != last_fg || back[y][x].bg != last_bg) {
                    wb_str("\033[");
                    wb_int(back[y][x].fg);
                    wb_char(';');
                    wb_int(back[y][x].bg);
                    wb_char('m');
                    last_fg = back[y][x].fg;
                    last_bg = back[y][x].bg;
                }

                wb_char(back[y][x].ch);

                /* Copy to front buffer */
                front[y][x] = back[y][x];
            }
        }
    }

    /* Reset color attributes and park cursor off-screen */
    wb_str("\033[0m");
    wb_str("\033[");
    wb_int(screen_height + 1);
    wb_str(";1H");

    wb_flush();

    first_frame = 0;
}

/* ── Cursor Visibility ────────────────────────────────────────────────── */

void scr_hide_cursor(void)
{
    printf("\033[?25l");
    fflush(stdout);
}

void scr_show_cursor(void)
{
    printf("\033[?25h");
    fflush(stdout);
}

/* ── Getters ──────────────────────────────────────────────────────────── */

int scr_get_width(void)
{
    return screen_width;
}

int scr_get_height(void)
{
    return screen_height;
}
