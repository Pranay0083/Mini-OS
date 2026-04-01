/*
 * keyboard.h — Non-Blocking Input Handler Interface
 *
 * Switches terminal to raw mode for real-time keystroke capture.
 * Dependencies: <termios.h>, <fcntl.h>, <unistd.h>
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

/* ── Key Constants ────────────────────────────────────────────────────── */

#define KEY_UP        1000
#define KEY_DOWN      1001
#define KEY_LEFT      1002
#define KEY_RIGHT     1003
#define KEY_ESCAPE    27
#define KEY_ENTER     10
#define KEY_BACKSPACE 127

/* ── API ──────────────────────────────────────────────────────────────── */

/**
 * Enter raw mode: disable echo, disable canonical buffering,
 * set stdin to non-blocking. Saves original terminal state.
 * Registers kb_restore() with atexit().
 */
void kb_init(void);

/**
 * Restore original terminal settings (canonical mode, echo).
 */
void kb_restore(void);

/**
 * Non-blocking key read.
 * Returns the key code if a key was pressed, or 0 if no key is available.
 * Handles multi-byte arrow key escape sequences.
 */
int kb_key_pressed(void);

/**
 * Blocking line read with character echo.
 * Reads until newline or max_len-1 chars. Handles backspace.
 * Returns the number of characters read.
 */
int kb_read_line(char *buf, int max_len);

#endif /* KEYBOARD_H */
