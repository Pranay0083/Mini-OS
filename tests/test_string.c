/*
 * test_string.c — Unit tests for the String Parser
 */

#include "../include/string.h"
#include <stdio.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg)                                           \
    do {                                                            \
        if (cond) {                                                 \
            tests_passed++;                                         \
            printf("  [PASS] %s\n", msg);                          \
        } else {                                                    \
            tests_failed++;                                         \
            printf("  [FAIL] %s  (line %d)\n", msg, __LINE__);    \
        }                                                           \
    } while (0)

static void test_length(void)
{
    printf("\n--- str_length ---\n");
    ASSERT(str_length("hello") == 5, "length('hello') == 5");
    ASSERT(str_length("") == 0, "length('') == 0");
    ASSERT(str_length("a") == 1, "length('a') == 1");
    ASSERT(str_length(NULL) == 0, "length(NULL) == 0");
}

static void test_copy(void)
{
    printf("\n--- str_copy ---\n");
    char buf[20];

    str_copy(buf, "hello", 20);
    ASSERT(str_compare(buf, "hello") == 0, "copy 'hello' exact");

    str_copy(buf, "truncated", 5);
    ASSERT(str_compare(buf, "trun") == 0, "copy with truncation");
    ASSERT(buf[4] == '\0', "truncated string is null-terminated");
}

static void test_compare(void)
{
    printf("\n--- str_compare ---\n");
    ASSERT(str_compare("abc", "abc") == 0, "'abc' == 'abc'");
    ASSERT(str_compare("abc", "abd") < 0, "'abc' < 'abd'");
    ASSERT(str_compare("abd", "abc") > 0, "'abd' > 'abc'");
    ASSERT(str_compare("ab", "abc") < 0, "'ab' < 'abc'");
    ASSERT(str_compare("", "") == 0, "'' == ''");
}

static void test_concat(void)
{
    printf("\n--- str_concat ---\n");
    char buf[20];

    str_copy(buf, "hello", 20);
    str_concat(buf, " world", 20);
    ASSERT(str_compare(buf, "hello world") == 0, "concat 'hello' + ' world'");

    str_copy(buf, "abc", 20);
    str_concat(buf, "defghij", 8);
    ASSERT(str_compare(buf, "abcdefg") == 0, "concat with truncation at 8");
}

static void test_itoa(void)
{
    printf("\n--- str_itoa ---\n");
    char buf[20];

    str_itoa(42, buf, 20);
    ASSERT(str_compare(buf, "42") == 0, "itoa(42) == '42'");

    str_itoa(-123, buf, 20);
    ASSERT(str_compare(buf, "-123") == 0, "itoa(-123) == '-123'");

    str_itoa(0, buf, 20);
    ASSERT(str_compare(buf, "0") == 0, "itoa(0) == '0'");

    str_itoa(1000000, buf, 20);
    ASSERT(str_compare(buf, "1000000") == 0, "itoa(1000000) == '1000000'");
}

static void test_atoi(void)
{
    printf("\n--- str_atoi ---\n");
    ASSERT(str_atoi("42") == 42, "atoi('42') == 42");
    ASSERT(str_atoi("-123") == -123, "atoi('-123') == -123");
    ASSERT(str_atoi("0") == 0, "atoi('0') == 0");
    ASSERT(str_atoi("  +99") == 99, "atoi('  +99') == 99 (whitespace+sign)");
    ASSERT(str_atoi("abc") == 0, "atoi('abc') == 0");
    ASSERT(str_atoi(NULL) == 0, "atoi(NULL) == 0");
}

static void test_starts_with(void)
{
    printf("\n--- str_starts_with ---\n");
    ASSERT(str_starts_with("hello world", "hello") == 1, "'hello world' starts with 'hello'");
    ASSERT(str_starts_with("hello", "hello world") == 0, "'hello' doesn't start with longer");
    ASSERT(str_starts_with("test", "") == 1, "everything starts with ''");
}

static void test_find(void)
{
    printf("\n--- str_find ---\n");
    const char *s = "hello world";
    char *found = str_find(s, 'o');
    ASSERT(found != NULL && *found == 'o', "find 'o' in 'hello world'");
    ASSERT(str_find(s, 'z') == NULL, "'z' not found returns NULL");
}

static void test_split(void)
{
    printf("\n--- str_split ---\n");
    char input1[64];
    str_copy(input1, "write file.txt hello", 64);
    char *tokens[10];
    int count = str_split(input1, ' ', tokens, 10);

    ASSERT(count == 3, "3 tokens in 'write file.txt hello'");
    ASSERT(str_compare(tokens[0], "write") == 0, "token[0] == 'write'");
    ASSERT(str_compare(tokens[1], "file.txt") == 0, "token[1] == 'file.txt'");
    ASSERT(str_compare(tokens[2], "hello") == 0, "token[2] == 'hello'");

    /* Multiple delimiters */
    char input2[64];
    str_copy(input2, "  spaced   out  ", 64);
    count = str_split(input2, ' ', tokens, 10);
    ASSERT(count == 2, "2 tokens in '  spaced   out  '");
    ASSERT(str_compare(tokens[0], "spaced") == 0, "token[0] == 'spaced'");
    ASSERT(str_compare(tokens[1], "out") == 0, "token[1] == 'out'");

    /* Single token */
    char input3[64];
    str_copy(input3, "single", 64);
    count = str_split(input3, ' ', tokens, 10);
    ASSERT(count == 1, "1 token in 'single'");
}

static void test_reverse(void)
{
    printf("\n--- str_reverse ---\n");
    char buf[20];
    str_copy(buf, "abcde", 20);
    str_reverse(buf, 5);
    ASSERT(str_compare(buf, "edcba") == 0, "reverse 'abcde' => 'edcba'");

    str_copy(buf, "ab", 20);
    str_reverse(buf, 2);
    ASSERT(str_compare(buf, "ba") == 0, "reverse 'ab' => 'ba'");
}

int main(void)
{
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║       string.c — Unit Test Suite          ║\n");
    printf("╚═══════════════════════════════════════════╝\n");

    test_length();
    test_copy();
    test_compare();
    test_concat();
    test_itoa();
    test_atoi();
    test_starts_with();
    test_find();
    test_split();
    test_reverse();

    printf("\n════════════════════════════════════════════\n");
    printf("Results: %d passed, %d failed, %d total\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    printf("════════════════════════════════════════════\n");

    return tests_failed > 0 ? 1 : 0;
}
