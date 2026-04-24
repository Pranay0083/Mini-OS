/*
 * string.c — String Parser Implementation
 *
 * All operations use raw pointer arithmetic.
 * No <string.h> dependency.
 */

#include "../include/string.h"

/* ── Core String Operations ───────────────────────────────────────────── */

int str_length(const char *s)
{
    if (!s) return 0;
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

void str_copy(char *dst, const char *src, int max_len)
{
    if (!dst || !src || max_len <= 0) return;

    int i = 0;
    while (i < max_len - 1 && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

int str_compare(const char *a, const char *b)
{
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return  1;

    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

void str_concat(char *dst, const char *src, int max_len)
{
    if (!dst || !src || max_len <= 0) return;

    int dst_len = str_length(dst);
    int i = 0;
    while (dst_len + i < max_len - 1 && src[i] != '\0') {
        dst[dst_len + i] = src[i];
        i++;
    }
    dst[dst_len + i] = '\0';
}

/* ── Numeric Conversions ──────────────────────────────────────────────── */

void str_reverse(char *s, int len)
{
    if (!s || len <= 1) return;

    int left  = 0;
    int right = len - 1;

    while (left < right) {
        char tmp = s[left];
        s[left]  = s[right];
        s[right] = tmp;
        left++;
        right--;
    }
}

int str_itoa(int num, char *buf, int buf_size)
{
    if (!buf || buf_size <= 0) return 0;

    if (buf_size == 1) {
        buf[0] = '\0';
        return 0;
    }

    int i = 0;
    int is_negative = 0;

    /* Handle zero */
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    /* Use unsigned to handle INT_MIN safely (negating INT_MIN overflows signed int) */
    unsigned int magnitude;
    if (num < 0) {
        is_negative = 1;
        magnitude = (unsigned int)(-(num + 1)) + 1u;
    } else {
        magnitude = (unsigned int)num;
    }

    /* Extract digits in reverse order */
    while (magnitude > 0 && i < buf_size - 1) {
        buf[i++] = '0' + (magnitude % 10);
        magnitude /= 10;
    }

    /* Add minus sign */
    if (is_negative && i < buf_size - 1) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    /* Reverse to correct order */
    str_reverse(buf, i);

    return i;
}

int str_atoi(const char *s)
{
    if (!s) return 0;

    int i = 0;
    int sign = 1;
    int result = 0;

    /* Skip whitespace */
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n') {
        i++;
    }

    /* Handle sign */
    if (s[i] == '-') {
        sign = -1;
        i++;
    } else if (s[i] == '+') {
        i++;
    }

    /* Accumulate digits */
    while (s[i] >= '0' && s[i] <= '9') {
        result = result * 10 + (s[i] - '0');
        i++;
    }

    return result * sign;
}

/* ── Search & Match ───────────────────────────────────────────────────── */

int str_starts_with(const char *s, const char *prefix)
{
    if (!s || !prefix) return 0;

    while (*prefix) {
        if (*s != *prefix) return 0;
        s++;
        prefix++;
    }
    return 1;
}

char *str_find(const char *s, char ch)
{
    if (!s) return (char *)0;

    while (*s) {
        if (*s == ch) return (char *)s;
        s++;
    }
    return (char *)0;
}

/* ── Tokenizer ────────────────────────────────────────────────────────── */

int str_split(char *input, char delimiter, char **tokens, int max_tokens)
{
    if (!input || !tokens || max_tokens <= 0) return 0;

    int count = 0;
    int i = 0;

    /* Skip leading delimiters */
    while (input[i] == delimiter) {
        i++;
    }

    while (input[i] != '\0' && count < max_tokens) {
        /* Mark start of token */
        tokens[count] = &input[i];
        count++;

        /* Scan to next delimiter or end */
        while (input[i] != '\0' && input[i] != delimiter) {
            i++;
        }

        /* Replace delimiter with null terminator */
        if (input[i] == delimiter) {
            input[i] = '\0';
            i++;

            /* Skip consecutive delimiters */
            while (input[i] == delimiter) {
                i++;
            }
        }
    }

    return count;
}
