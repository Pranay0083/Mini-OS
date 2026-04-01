/*
 * screen.c — Terminal Renderer Implementation
 *
 * Double-buffered ANSI terminal renderer.
 * Only draws cells that changed between frames (diff-based refresh).
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

/* ── Module State ─────────────────────────────────────────────────────── */

static ScreenCell front[SCR_MAX_HEIGHT][SCR_MAX_WIDTH];
static ScreenCell back[SCR_MAX_HEIGHT][SCR_MAX_WIDTH];
static int screen_width  = 80;
static int screen_height = 24;

/* ── Initialization ───────────────────────────────────────────────────── */

void scr_init(int width, int height)
{
    if (width  > SCR_MAX_WIDTH)  width  = SCR_MAX_WIDTH;
    if (height > SCR_MAX_HEIGHT) height = SCR_MAX_HEIGHT;
    if (width  < 1) width  = 1;
    if (height < 1) height = 1;

    screen_width  = width;
    screen_height = height;

    /* Initialize both buffers */
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            front[y][x].ch = ' ';
            front[y][x].fg = COLOR_WHITE;
            front[y][x].bg = BG_BLACK;
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

/* ── Diff-Based Refresh ───────────────────────────────────────────────── */

void scr_refresh(void)
{
    int last_fg = -1;
    int last_bg = -1;

    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            /* Only update changed cells */
            if (back[y][x].ch != front[y][x].ch ||
                back[y][x].fg != front[y][x].fg ||
                back[y][x].bg != front[y][x].bg) {

                /* Move cursor */
                printf("\033[%d;%dH", y + 1, x + 1);

                /* Set colors only if they changed */
                if (back[y][x].fg != last_fg || back[y][x].bg != last_bg) {
                    printf("\033[%d;%dm", back[y][x].fg, back[y][x].bg);
                    last_fg = back[y][x].fg;
                    last_bg = back[y][x].bg;
                }

                putchar(back[y][x].ch);

                /* Copy to front buffer */
                front[y][x] = back[y][x];
            }
        }
    }

    /* Reset color attributes */
    printf("\033[0m");
    fflush(stdout);
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
