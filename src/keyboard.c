/*
 * keyboard.c — Non-Blocking Input Handler Implementation
 *
 * Raw mode terminal configuration for real-time keystroke capture.
 *
 * Permitted dependencies:
 *   <termios.h> — terminal attribute manipulation
 *   <fcntl.h>   — non-blocking I/O flags
 *   <unistd.h>  — read(), STDIN_FILENO
 *   <stdio.h>   — putchar for echo in read_line
 *   <stdlib.h>  — atexit() for cleanup registration
 */

#include "../include/keyboard.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Module State ─────────────────────────────────────────────────────── */

static struct termios original_termios;
static int            raw_mode_active = 0;
static int            original_flags  = 0;

/* ── Raw Mode Setup ───────────────────────────────────────────────────── */

void kb_restore(void)
{
    if (raw_mode_active) {
        /* Restore original terminal settings */
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);

        /* Restore original fcntl flags */
        fcntl(STDIN_FILENO, F_SETFL, original_flags);

        raw_mode_active = 0;
    }
}

void kb_init(void)
{
    if (raw_mode_active) return;

    /* Save original terminal settings */
    tcgetattr(STDIN_FILENO, &original_termios);

    /* Save original fcntl flags */
    original_flags = fcntl(STDIN_FILENO, F_GETFL, 0);

    /* Configure raw mode */
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);  /* No echo, no line buffering */
    raw.c_cc[VMIN]  = 0;               /* Non-blocking: return immediately */
    raw.c_cc[VTIME] = 0;               /* No timeout */

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    /* Set stdin to non-blocking */
    fcntl(STDIN_FILENO, F_SETFL, original_flags | O_NONBLOCK);

    raw_mode_active = 1;

    /* Register cleanup for program exit */
    atexit(kb_restore);
}

/* ── Non-Blocking Key Read ────────────────────────────────────────────── */

int kb_key_pressed(void)
{
    unsigned char ch;
    int n = (int)read(STDIN_FILENO, &ch, 1);

    if (n <= 0) {
        return 0;  /* No key available */
    }

    /* Check for escape sequence (arrow keys, etc.) */
    if (ch == 27) {
        unsigned char seq[2];
        int n1 = (int)read(STDIN_FILENO, &seq[0], 1);
        int n2 = (int)read(STDIN_FILENO, &seq[1], 1);

        if (n1 > 0 && n2 > 0 && seq[0] == '[') {
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

/* ── Blocking Line Read with Echo ─────────────────────────────────────── */

int kb_read_line(char *buf, int max_len)
{
    if (!buf || max_len <= 0) return 0;

    int pos = 0;

    /* Temporarily set stdin to blocking for line input */
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

    while (1) {
        unsigned char ch;
        int n = (int)read(STDIN_FILENO, &ch, 1);

        if (n <= 0) continue;

        if (ch == '\n' || ch == '\r') {
            buf[pos] = '\0';
            putchar('\n');
            fflush(stdout);
            break;
        }

        if (ch == KEY_BACKSPACE || ch == 8) {
            if (pos > 0) {
                pos--;
                /* Erase character on screen: backspace, space, backspace */
                putchar('\b');
                putchar(' ');
                putchar('\b');
                fflush(stdout);
            }
            continue;
        }

        /* Ignore non-printable characters (except tab) */
        if (ch < 32 && ch != '\t') {
            continue;
        }

        if (pos < max_len - 1) {
            buf[pos++] = (char)ch;
            putchar(ch);
            fflush(stdout);
        }
    }

    /* Restore non-blocking mode */
    fcntl(STDIN_FILENO, F_SETFL, flags);

    return pos;
}
