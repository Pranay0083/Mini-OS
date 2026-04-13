#ifndef STRING_H
#define STRING_H

/*
 * string.h — String Utilities (No <string.h>)
 *
 * Implements safe string operations and tokenizer.
 */

/* ── Core Operations ───────────────────────────────────── */

int  str_length(const char *s);
void str_copy(char *dst, const char *src, int max_len);
int  str_compare(const char *a, const char *b);
void str_concat(char *dst, const char *src, int max_len);

/* ── Numeric ───────────────────────────────────────────── */

int  str_itoa(int num, char *buf, int buf_size);
int  str_atoi(const char *s);

/* ── Search ────────────────────────────────────────────── */

int   str_starts_with(const char *s, const char *prefix);
char *str_find(const char *s, char ch);

/* ── Manipulation ─────────────────────────────────────── */

void str_reverse(char *s, int len);

/*
 * Tokenize string in-place.
 *
 * Example:
 *   input: "echo hello world"
 *   tokens: ["echo", "hello", "world"]
 *
 * NOTE:
 * - modifies input string
 * - tokens point inside input buffer
 */
int str_split(char *input, char delimiter, char **tokens, int max_tokens);

#endif /* STRING_H */
