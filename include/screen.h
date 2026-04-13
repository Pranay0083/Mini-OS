#ifndef SCREEN_H
#define SCREEN_H

/*
 * screen.h — Terminal Rendering Interface
 *
 * Provides buffered rendering + console print abstraction.
 */

/* ── Colors ────────────────────────────────────────────── */

#define COLOR_BLACK    30
#define COLOR_RED      31
#define COLOR_GREEN    32
#define COLOR_YELLOW   33
#define COLOR_BLUE     34
#define COLOR_MAGENTA  35
#define COLOR_CYAN     36
#define COLOR_WHITE    37
#define COLOR_DEFAULT  39

#define BG_BLACK    40
#define BG_RED      41
#define BG_GREEN    42
#define BG_YELLOW   43
#define BG_BLUE     44
#define BG_MAGENTA  45
#define BG_CYAN     46
#define BG_WHITE    47
#define BG_DEFAULT  49

/* ── Limits ────────────────────────────────────────────── */

#define SCR_MAX_WIDTH   120
#define SCR_MAX_HEIGHT   40

/* ── API ───────────────────────────────────────────────── */

void scr_init(int width, int height);
void scr_clear(void);

void scr_clear_buffer(void);
void scr_move_cursor(int x, int y);

void scr_put_char(int x, int y, char ch, int fg, int bg);
void scr_put_string(int x, int y, const char *s, int fg, int bg);
void scr_draw_box(int x, int y, int w, int h, int fg, int bg);

void scr_refresh(void);

/* Console abstraction */
void scr_print(const char *s);
void scr_println(const char *s);

/* Cursor */
void scr_hide_cursor(void);
void scr_show_cursor(void);

/* Dimensions */
int scr_get_width(void);
int scr_get_height(void);

#endif /* SCREEN_H */
