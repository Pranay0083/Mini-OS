/*
 * screen.h — Terminal Renderer Interface
 *
 * ANSI escape code based 2D rendering with double-buffered framebuffer.
 * Only dependency: <stdio.h> for putchar/printf/fflush.
 */

#ifndef SCREEN_H
#define SCREEN_H

/* ── Color Constants (ANSI Foreground) ────────────────────────────────── */

#define COLOR_BLACK    30
#define COLOR_RED      31
#define COLOR_GREEN    32
#define COLOR_YELLOW   33
#define COLOR_BLUE     34
#define COLOR_MAGENTA  35
#define COLOR_CYAN     36
#define COLOR_WHITE    37
#define COLOR_DEFAULT  39

/* Background = foreground + 10 */
#define BG_BLACK    40
#define BG_RED      41
#define BG_GREEN    42
#define BG_YELLOW   43
#define BG_BLUE     44
#define BG_MAGENTA  45
#define BG_CYAN     46
#define BG_WHITE    47
#define BG_DEFAULT  49

/* ── Screen Dimensions ────────────────────────────────────────────────── */

#define SCR_MAX_WIDTH   120
#define SCR_MAX_HEIGHT   40

/* ── API ──────────────────────────────────────────────────────────────── */

/**
 * Initialize the framebuffer with given dimensions.
 */
void scr_init(int width, int height);

/**
 * Clear the terminal screen (ANSI escape).
 */
void scr_clear(void);

/**
 * Clear the back buffer (fill with spaces, default colors).
 */
void scr_clear_buffer(void);

/**
 * Move terminal cursor to position (x, y). 1-indexed for ANSI.
 */
void scr_move_cursor(int x, int y);

/**
 * Write a colored character to the back buffer at (x, y).
 * Silently ignores out-of-bounds coordinates.
 */
void scr_put_char(int x, int y, char ch, int fg, int bg);

/**
 * Write a colored string to the back buffer starting at (x, y).
 */
void scr_put_string(int x, int y, const char *s, int fg, int bg);

/**
 * Draw a bordered box using ASCII box-drawing characters.
 */
void scr_draw_box(int x, int y, int w, int h, int fg, int bg);

/**
 * Flush changed cells from back buffer to terminal (diff-based).
 * Copies back buffer to front buffer after flushing.
 */
void scr_refresh(void);

/**
 * Hide the terminal cursor.
 */
void scr_hide_cursor(void);

/**
 * Show the terminal cursor.
 */
void scr_show_cursor(void);

/**
 * Get the current screen width.
 */
int scr_get_width(void);

/**
 * Get the current screen height.
 */
int scr_get_height(void);

#endif /* SCREEN_H */
