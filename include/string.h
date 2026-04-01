/*
 * string.h — String Parser Interface
 *
 * Bounds-safe string operations using raw pointer arithmetic.
 * No <string.h> dependency.
 */

#ifndef STRING_H
#define STRING_H

/* ── Core String Operations ───────────────────────────────────────────── */

/**
 * Return the length of string s (not counting null terminator).
 */
int  str_length(const char *s);

/**
 * Copy src into dst, writing at most max_len-1 chars. Always null-terminates.
 */
void str_copy(char *dst, const char *src, int max_len);

/**
 * Lexicographic comparison. Returns <0, 0, or >0.
 */
int  str_compare(const char *a, const char *b);

/**
 * Append src to the end of dst, total not exceeding max_len. Always null-terminates.
 */
void str_concat(char *dst, const char *src, int max_len);

/* ── Numeric Conversions ──────────────────────────────────────────────── */

/**
 * Convert integer to ASCII string. Returns number of chars written.
 */
int  str_itoa(int num, char *buf, int buf_size);

/**
 * Convert ASCII string to integer. Handles sign and leading whitespace.
 */
int  str_atoi(const char *s);

/* ── Search & Match ───────────────────────────────────────────────────── */

/**
 * Returns 1 if s starts with the given prefix.
 */
int  str_starts_with(const char *s, const char *prefix);

/**
 * Find first occurrence of ch in s. Returns pointer or NULL.
 */
char *str_find(const char *s, char ch);

/* ── Manipulation ─────────────────────────────────────────────────────── */

/**
 * Reverse string in-place for the first len characters.
 */
void str_reverse(char *s, int len);

/**
 * Tokenize input string by replacing delimiters with '\0'.
 * Stores pointers to each token in tokens[]. Returns token count.
 */
int  str_split(char *input, char delimiter, char **tokens, int max_tokens);

#endif /* STRING_H */
