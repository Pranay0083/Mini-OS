#include "../include/keyboard.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Module State ───────────────────────────────────────── */

static struct termios original_termios;
static int raw_mode_active = 0;
static int original_flags  = 0;

/* ── Restore ───────────────────────────────────────────── */

void kb_restore(void)
{
    if (!raw_mode_active) return;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    fcntl(STDIN_FILENO, F_SETFL, original_flags);

    raw_mode_active = 0;
}

/* ── Init ─────────────────────────────────────────────── */

void kb_init(void)
{
    if (raw_mode_active) return;

    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) return;

    original_flags = fcntl(STDIN_FILENO, F_GETFL, 0);

    struct termios raw = original_termios;

    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, original_flags | O_NONBLOCK);

    raw_mode_active = 1;
    atexit(kb_restore);
}

/* ── Non-blocking key ─────────────────────────────────── */

int kb_key_pressed(void)
{
    unsigned char ch;
    int n = read(STDIN_FILENO, &ch, 1);

    if (n <= 0) return 0;

    if (ch == 27) {
        unsigned char seq[2];

        if (read(STDIN_FILENO, &seq[0], 1) <= 0) return KEY_ESCAPE;
        if (read(STDIN_FILENO, &seq[1], 1) <= 0) return KEY_ESCAPE;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
            }
        }
        return KEY_ESCAPE;
    }

    return (int)ch;
}

/* ── Blocking line read ───────────────────────────────── */

int kb_read_line(char *buf, int max_len)
{
    if (!buf || max_len <= 0) return 0;

    int pos = 0;

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

    while (1) {
        unsigned char ch;
        if (read(STDIN_FILENO, &ch, 1) <= 0) continue;

        if (ch == '\n' || ch == '\r') {
            buf[pos] = '\0';
            putchar('\n');
            fflush(stdout);
            break;
        }

        if (ch == KEY_BACKSPACE || ch == 8) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        if (ch < 32 && ch != '\t') continue;

        if (pos < max_len - 1) {
            buf[pos++] = (char)ch;
            putchar(ch);
            fflush(stdout);
        }
    }

    fflush(stdout);
    fcntl(STDIN_FILENO, F_SETFL, flags);

    return pos;
}
