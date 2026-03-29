/*
 * game.c — Track A: Snake Game
 *
 * Real-time interactive Snake game built entirely on the five custom libraries.
 * - Snake segments dynamically allocated via memory.c
 * - Collision detection via math.c boundary helpers
 * - Score display via string.c itoa
 * - ANSI terminal rendering via screen.c framebuffer
 * - Non-blocking input via keyboard.c raw mode
 *
 * Permitted: <stdlib.h> for initial malloc + exit, <unistd.h> for usleep
 */

#include "../include/memory.h"
#include "../include/math.h"
#include "../include/string.h"
#include "../include/screen.h"
#include "../include/keyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* ── Configuration ────────────────────────────────────────────────────── */

#define GAME_WIDTH   60
#define GAME_HEIGHT  24
#define BASE_DELAY   120000   /* microseconds (120ms) */
#define MIN_DELAY    40000    /* microseconds (40ms)  */
#define SPEED_STEP   2000     /* delay reduction per point */

/*
 * Screen layout (total height = GAME_HEIGHT + 2 = 26 rows):
 *
 *   Row 0:        Title bar text
 *   Row 1:        Top border of play area  +-----------+
 *   Row 2..H-1:   Playable area            |           |
 *   Row H:        Bottom border            +-----------+
 *   Row H+1:      HUD / controls text
 *
 *   GAME_HEIGHT = 24, so:
 *     Play area border: rows 1 to 24  (box at y=1, h=GAME_HEIGHT)
 *     Playable cells:   rows 2 to 23  (inside the border)
 *     Snake/food y:     [2, GAME_HEIGHT-1)  →  [2, 23]
 *     Snake/food x:     [1, GAME_WIDTH-2]   →  [1, 58]
 */

#define PLAY_TOP     2                    /* First playable row */
#define PLAY_BOTTOM  (GAME_HEIGHT - 1)    /* Last playable row  */
#define PLAY_LEFT    1                    /* First playable col */
#define PLAY_RIGHT   (GAME_WIDTH - 2)     /* Last playable col  */

#define SCREEN_ROWS  (GAME_HEIGHT + 2)    /* Total screen rows  */

/* ── Snake Segment (Linked List) ──────────────────────────────────────── */

typedef struct SnakeSegment {
    int x, y;
    struct SnakeSegment *next;
} SnakeSegment;

/* ── Game State ───────────────────────────────────────────────────────── */

typedef struct {
    SnakeSegment *head;
    SnakeSegment *tail;
    int dir_x, dir_y;
    int food_x, food_y;
    int score;
    int game_over;
    int quit;
} GameState;

static GameState game;
static char virtual_ram[VIRTUAL_RAM_SIZE];

/* ── Helper: Create a Segment ─────────────────────────────────────────── */

static SnakeSegment *create_segment(int x, int y)
{
    SnakeSegment *seg = (SnakeSegment *)mem_alloc(sizeof(SnakeSegment));
    if (!seg) return NULL;
    seg->x    = x;
    seg->y    = y;
    seg->next = NULL;
    return seg;
}

/* ── Helper: Free All Segments ────────────────────────────────────────── */

static void free_snake(void)
{
    SnakeSegment *cur = game.head;
    while (cur) {
        SnakeSegment *next = cur->next;
        mem_free(cur);
        cur = next;
    }
    game.head = NULL;
    game.tail = NULL;
}

/* ── Spawn Food ───────────────────────────────────────────────────────── */

static void spawn_food(void)
{
    int attempts = 0;
    while (attempts < 1000) {
        int fx = m_rand_range(PLAY_LEFT, PLAY_RIGHT);
        int fy = m_rand_range(PLAY_TOP, PLAY_BOTTOM);

        /* Check if position is occupied by snake */
        int occupied = 0;
        SnakeSegment *cur = game.head;
        while (cur) {
            if (cur->x == fx && cur->y == fy) {
                occupied = 1;
                break;
            }
            cur = cur->next;
        }

        if (!occupied) {
            game.food_x = fx;
            game.food_y = fy;
            return;
        }
        attempts++;
    }
    /* Fallback */
    game.food_x = GAME_WIDTH / 2;
    game.food_y = (PLAY_TOP + PLAY_BOTTOM) / 2;
}

/* ── Initialize Game ──────────────────────────────────────────────────── */

static void init_game(void)
{
    /* Reinit memory to clean slate */
    mem_init(virtual_ram, VIRTUAL_RAM_SIZE);

    /* Seed RNG with a simple counter-based value */
    static unsigned int seed_counter = 12345;
    seed_counter += 7919;  /* prime increment for variety */
    m_srand(seed_counter);

    /* Create initial snake (3 segments, moving right) */
    int start_x = GAME_WIDTH / 2;
    int start_y = (PLAY_TOP + PLAY_BOTTOM) / 2;

    game.head = create_segment(start_x, start_y);
    SnakeSegment *s2 = create_segment(start_x - 1, start_y);
    SnakeSegment *s3 = create_segment(start_x - 2, start_y);

    game.head->next = s2;
    s2->next = s3;
    game.tail = s3;

    game.dir_x     = 1;
    game.dir_y     = 0;
    game.score     = 0;
    game.game_over = 0;
    game.quit      = 0;

    spawn_food();
}

/* ── Process Input ────────────────────────────────────────────────────── */

static void process_input(void)
{
    /* Drain input buffer — only keep the last key pressed
       to prevent input lag from buffered keystrokes */
    int key = 0;
    int k;
    while ((k = kb_key_pressed()) != 0) {
        key = k;
    }
    if (key == 0) return;

    switch (key) {
        case 'w': case 'W': case KEY_UP:
            if (game.dir_y != 1) { game.dir_x = 0; game.dir_y = -1; }
            break;
        case 's': case 'S': case KEY_DOWN:
            if (game.dir_y != -1) { game.dir_x = 0; game.dir_y = 1; }
            break;
        case 'a': case 'A': case KEY_LEFT:
            if (game.dir_x != 1) { game.dir_x = -1; game.dir_y = 0; }
            break;
        case 'd': case 'D': case KEY_RIGHT:
            if (game.dir_x != -1) { game.dir_x = 1; game.dir_y = 0; }
            break;
        case 'q': case 'Q':
            game.quit = 1;
            break;
        case 'r': case 'R':
            if (game.game_over) {
                free_snake();
                init_game();
            }
            break;
    }
}

/* ── Update Game State ────────────────────────────────────────────────── */

static void update_game(void)
{
    if (game.game_over) return;

    int new_x = game.head->x + game.dir_x;
    int new_y = game.head->y + game.dir_y;

    /* ── Wall collision (playable area bounds) ──────────────────── */
    if (new_x < PLAY_LEFT || new_x > PLAY_RIGHT ||
        new_y < PLAY_TOP  || new_y > PLAY_BOTTOM) {
        game.game_over = 1;
        return;
    }

    /* ── Self collision ─────────────────────────────────────────── */
    SnakeSegment *cur = game.head;
    while (cur) {
        if (cur->x == new_x && cur->y == new_y) {
            game.game_over = 1;
            return;
        }
        cur = cur->next;
    }

    /* ── Create new head segment ────────────────────────────────── */
    SnakeSegment *new_head = create_segment(new_x, new_y);
    if (!new_head) {
        game.game_over = 1;  /* Out of memory */
        return;
    }
    new_head->next = game.head;
    game.head = new_head;

    /* ── Check food collision ───────────────────────────────────── */
    if (new_x == game.food_x && new_y == game.food_y) {
        game.score++;
        spawn_food();
        /* Don't remove tail — snake grows */
    } else {
        /* Remove tail segment */
        SnakeSegment *prev = game.head;
        while (prev->next != game.tail) {
            prev = prev->next;
        }
        mem_free(game.tail);
        game.tail = prev;
        game.tail->next = NULL;
    }
}

/* ── Render ────────────────────────────────────────────────────────────── */

static void render_game(void)
{
    scr_clear_buffer();

    /* ── Title bar (row 0) ─────────────────────────────────────── */
    scr_put_char(0, 0, '[', COLOR_CYAN, BG_BLACK);
    scr_put_string(1, 0, " MINI SNAKE ", COLOR_GREEN, BG_BLACK);
    scr_put_char(13, 0, ']', COLOR_CYAN, BG_BLACK);

    /* Score on the right side of title */
    char score_buf[32];
    char score_str[48];
    str_copy(score_str, "Score: ", 48);
    str_itoa(game.score, score_buf, 32);
    str_concat(score_str, score_buf, 48);
    int score_x = GAME_WIDTH - str_length(score_str) - 1;
    scr_put_string(score_x, 0, score_str, COLOR_YELLOW, BG_BLACK);

    /* Decorative line across title row */
    for (int x = 14; x < score_x - 1; x++) {
        scr_put_char(x, 0, '-', COLOR_CYAN, BG_BLACK);
    }

    /* ── Play area border (row 1 to GAME_HEIGHT) ───────────────── */
    scr_draw_box(0, 1, GAME_WIDTH, GAME_HEIGHT, COLOR_WHITE, BG_BLACK);

    /* ── Food ───────────────────────────────────────────────────── */
    scr_put_char(game.food_x, game.food_y, '*', COLOR_RED, BG_BLACK);

    /* ── Snake ──────────────────────────────────────────────────── */
    SnakeSegment *seg = game.head;
    int is_head = 1;
    while (seg) {
        if (is_head) {
            scr_put_char(seg->x, seg->y, '@', COLOR_GREEN, BG_BLACK);
            is_head = 0;
        } else {
            scr_put_char(seg->x, seg->y, '#', COLOR_GREEN, BG_BLACK);
        }
        seg = seg->next;
    }

    /* ── HUD (row GAME_HEIGHT + 1) ─────────────────────────────── */
    scr_put_string(1, GAME_HEIGHT + 1,
                   "WASD/Arrows: Move | Q: Quit",
                   COLOR_CYAN, BG_BLACK);

    /* Show memory usage on HUD right side */
    char mem_str[48];
    char mem_buf[16];
    str_copy(mem_str, "Mem: ", 48);
    str_itoa((int)mem_available(), mem_buf, 16);
    str_concat(mem_str, mem_buf, 48);
    str_concat(mem_str, "B", 48);
    scr_put_string(GAME_WIDTH - str_length(mem_str) - 1, GAME_HEIGHT + 1,
                   mem_str, COLOR_MAGENTA, BG_BLACK);

    /* ── Game Over overlay ──────────────────────────────────────── */
    if (game.game_over) {
        int box_w = 24;
        int box_h = 7;
        int cx = (GAME_WIDTH - box_w) / 2;
        int cy = (PLAY_TOP + PLAY_BOTTOM - box_h) / 2 + 1;

        /* Fill box background */
        for (int by = cy; by < cy + box_h; by++) {
            for (int bx = cx; bx < cx + box_w; bx++) {
                scr_put_char(bx, by, ' ', COLOR_WHITE, BG_BLACK);
            }
        }

        scr_draw_box(cx, cy, box_w, box_h, COLOR_RED, BG_BLACK);

        /* Center text inside the box */
        scr_put_string(cx + 4, cy + 1, "  GAME OVER!  ", COLOR_RED, BG_BLACK);

        char final_score[48];
        str_copy(final_score, "  Score: ", 48);
        str_itoa(game.score, score_buf, 32);
        str_concat(final_score, score_buf, 48);
        /* Pad to center */
        int pad = (box_w - 2 - str_length(final_score)) / 2;
        for (int p = 0; p < pad; p++) {
            str_concat(final_score, " ", 48);
        }
        scr_put_string(cx + 1, cy + 3, final_score, COLOR_YELLOW, BG_BLACK);

        scr_put_string(cx + 2, cy + 5, " R:Restart  Q:Quit ", COLOR_WHITE, BG_BLACK);
    }

    scr_refresh();
}

/* ── Main Game Loop ───────────────────────────────────────────────────── */

int main(void)
{
    /* Initialize systems */
    mem_init(virtual_ram, VIRTUAL_RAM_SIZE);
    scr_init(GAME_WIDTH, SCREEN_ROWS);
    kb_init();
    scr_hide_cursor();
    scr_clear();

    init_game();

    while (!game.quit) {
        process_input();

        if (!game.game_over) {
            update_game();
        }

        render_game();

        /* Difficulty scaling: faster as score increases */
        int delay = BASE_DELAY - (game.score * SPEED_STEP);
        delay = m_max(delay, MIN_DELAY);
        usleep((unsigned int)delay);
    }

    /* Cleanup */
    free_snake();
    scr_clear();
    scr_show_cursor();
    kb_restore();

    printf("Thanks for playing Mini Snake! Final score: %d\n", game.score);

    return 0;
}
