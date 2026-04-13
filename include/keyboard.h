#ifndef KEYBOARD_H
#define KEYBOARD_H

/*
 * keyboard.h — Non-Blocking Input Handler Interface
 *
 * Uses raw terminal mode for real-time input handling.
 * Dependencies: <termios.h>, <fcntl.h>, <unistd.h>
 */

/* ── Key Constants ───────────────────────────────────────── */

#define KEY_UP        1000
#define KEY_DOWN      1001
#define KEY_LEFT      1002
#define KEY_RIGHT     1003
#define KEY_ESCAPE    27
#define KEY_ENTER     10
#define KEY_BACKSPACE 127

/* ── API ────────────────────────────────────────────────── */

/* Initialize raw mode input */
void kb_init(void);

/* Restore terminal to original state */
void kb_restore(void);

/* Non-blocking key read */
int kb_key_pressed(void);

/* Blocking line input */
int kb_read_line(char *buf, int max_len);

#endif /* KEYBOARD_H */
