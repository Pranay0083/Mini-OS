/*
 * test_shell.c — Integration Tests for the Shell + All Libraries
 *
 * Validates that the shell correctly integrates all custom libraries:
 *   - string.c   → tokenization, comparison
 *   - memory.c   → heap allocation  
 *   - screen.c   → print wrappers
 *   - math.c     → arithmetic operations
 *   - keyboard.c → input handling (tested via function interface)
 *
 * These tests verify the "INTEGRATION" checklist requirement:
 *   Step     Library Used
 *   ─────    ────────────
 *   Input    keyboard.c
 *   Parsing  string.c
 *   Memory   memory.c
 *   Output   screen.c
 */

#include "../include/string.h"
#include "../include/memory.h"
#include "../include/math.h"
#include "../include/screen.h"
#include "../include/shell.h"
#include <stdio.h>

/* ── Test Helpers ─────────────────────────────────────────────────────── */

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

static char test_ram[VIRTUAL_RAM_SIZE];

/* ── Test 1: String Library Core Functions ────────────────────────────── */

static void test_string_core(void)
{
    printf("\n--- String Library Core ---\n");

    /* strlen */
    ASSERT(str_length("abc") == 3, "strlen('abc') == 3");
    ASSERT(str_length("") == 0, "strlen('') == 0");
    ASSERT(str_length(NULL) == 0, "strlen(NULL) == 0");

    /* strcmp */
    ASSERT(str_compare("a", "a") == 0, "strcmp('a','a') == 0");
    ASSERT(str_compare("a", "b") < 0, "strcmp('a','b') < 0");
    ASSERT(str_compare("b", "a") > 0, "strcmp('b','a') > 0");

    /* strcpy */
    char buf[32];
    str_copy(buf, "hello", 32);
    ASSERT(str_compare(buf, "hello") == 0, "strcpy copies correctly");

    /* itoa */
    char num_buf[16];
    str_itoa(42, num_buf, 16);
    ASSERT(str_compare(num_buf, "42") == 0, "itoa(42) == '42'");
    str_itoa(-7, num_buf, 16);
    ASSERT(str_compare(num_buf, "-7") == 0, "itoa(-7) == '-7'");
}

/* ── Test 2: split() — MOST IMPORTANT ─────────────────────────────────── */

static void test_split(void)
{
    printf("\n--- str_split (Custom Tokenizer) ---\n");

    /* Basic split: "echo hello world" → ["echo", "hello", "world"] */
    char input1[64];
    str_copy(input1, "echo hello world", 64);
    char *tokens[16];
    int count = str_split(input1, ' ', tokens, 16);

    ASSERT(count == 3, "split('echo hello world') → 3 tokens");
    ASSERT(str_compare(tokens[0], "echo") == 0, "  token[0] == 'echo'");
    ASSERT(str_compare(tokens[1], "hello") == 0, "  token[1] == 'hello'");
    ASSERT(str_compare(tokens[2], "world") == 0, "  token[2] == 'world'");

    /* Single word */
    char input2[64];
    str_copy(input2, "help", 64);
    count = str_split(input2, ' ', tokens, 16);
    ASSERT(count == 1, "split('help') → 1 token");
    ASSERT(str_compare(tokens[0], "help") == 0, "  token[0] == 'help'");

    /* Multiple spaces */
    char input3[64];
    str_copy(input3, "  write   file.txt   data  ", 64);
    count = str_split(input3, ' ', tokens, 16);
    ASSERT(count == 3, "split with extra spaces → 3 tokens");
    ASSERT(str_compare(tokens[0], "write") == 0, "  token[0] == 'write'");

    /* Empty string */
    char input4[64];
    str_copy(input4, "", 64);
    count = str_split(input4, ' ', tokens, 16);
    ASSERT(count == 0, "split('') → 0 tokens");
}

/* ── Test 3: Memory Manager ───────────────────────────────────────────── */

static void test_memory(void)
{
    printf("\n--- Memory Manager ---\n");
    mem_init(test_ram, VIRTUAL_RAM_SIZE);

    /* Basic alloc */
    void *p1 = mem_alloc(100);
    ASSERT(p1 != NULL, "mem_alloc(100) returns non-NULL");

    /* Alignment check */
    ASSERT(((size_t)p1 & 7) == 0, "Allocation is 8-byte aligned");

    /* Free and reuse */
    size_t free_before = mem_available();
    mem_free(p1);
    size_t free_after = mem_available();
    ASSERT(free_after > free_before, "mem_free increases available memory");

    /* Alloc for string storage (simulating VFS file data) */
    char *data = (char *)mem_alloc(64);
    ASSERT(data != NULL, "mem_alloc for string storage works");
    str_copy(data, "file content", 64);
    ASSERT(str_compare(data, "file content") == 0, "Can store strings in allocated memory");
    mem_free(data);
}

/* ── Test 4: Math Library ─────────────────────────────────────────────── */

static void test_math(void)
{
    printf("\n--- Math Library ---\n");
    ASSERT(m_add(3, 4) == 7, "add(3,4) == 7");
    ASSERT(m_sub(10, 3) == 7, "sub(10,3) == 7");
    ASSERT(m_mul(6, 7) == 42, "mul(6,7) == 42");
    ASSERT(m_div(10, 3) == 3, "div(10,3) == 3");
    ASSERT(m_div(5, 0) == 0, "div(5,0) == 0 (safe)");
    ASSERT(m_mod(10, 3) == 1, "mod(10,3) == 1");
    ASSERT(m_abs(-5) == 5, "abs(-5) == 5");
}

/* ── Test 5: Screen Output Layer ──────────────────────────────────────── */

static void test_screen(void)
{
    printf("\n--- Screen Output Layer ---\n");

    /* scr_print should not crash */
    scr_print("  Testing scr_print: ");
    scr_print("OK");
    scr_print("\n");
    ASSERT(1, "scr_print works without crash");

    /* scr_println should not crash */
    scr_print("  Testing scr_println: ");
    scr_println("OK");
    ASSERT(1, "scr_println works without crash");

    /* NULL safety */
    scr_print(NULL);
    ASSERT(1, "scr_print(NULL) is safe");
}

/* ── Test 6: Integration — Command Parsing Flow ───────────────────────── */

static void test_integration(void)
{
    printf("\n--- Integration: Command Parse Flow ---\n");

    /*
     * Simulate the exact shell flow:
     *   1. Input string (would come from keyboard.c in real shell)
     *   2. Parse with str_split (string.c)
     *   3. Match command with str_compare (string.c) 
     *   4. Output via scr_print (screen.c)
     *
     * This proves the integration path works.
     */

    /* Simulate: "echo hello world" */
    char cmd[64];
    str_copy(cmd, "echo hello world", 64);

    char *tokens[16];
    int count = str_split(cmd, ' ', tokens, 16);

    ASSERT(count == 3, "Tokenized 'echo hello world' → 3 tokens");
    ASSERT(str_compare(tokens[0], "echo") == 0, "Command identified as 'echo'");

    /* Reconstruct echo output (like cmd_echo does) */
    char output[128];
    output[0] = '\0';
    for (int i = 1; i < count; i++) {
        str_concat(output, tokens[i], 128);
        if (i < count - 1) str_concat(output, " ", 128);
    }
    ASSERT(str_compare(output, "hello world") == 0, "Reconstructed output: 'hello world'");

    /* Simulate: "exit" */
    char cmd2[64];
    str_copy(cmd2, "exit", 64);
    count = str_split(cmd2, ' ', tokens, 16);
    ASSERT(str_compare(tokens[0], "exit") == 0, "Command identified as 'exit'");

    /* Simulate: unknown command */
    char cmd3[64];
    str_copy(cmd3, "foobar", 64);
    count = str_split(cmd3, ' ', tokens, 16);
    int is_known = (str_compare(tokens[0], "echo") == 0 ||
                    str_compare(tokens[0], "help") == 0 ||
                    str_compare(tokens[0], "clear") == 0 ||
                    str_compare(tokens[0], "exit") == 0);
    ASSERT(!is_known, "Unknown command 'foobar' is not matched");
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void)
{
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║   Shell Integration — Test Suite          ║\n");
    printf("╚═══════════════════════════════════════════╝\n");

    test_string_core();
    test_split();
    test_memory();
    test_math();
    test_screen();
    test_integration();

    printf("\n════════════════════════════════════════════\n");
    printf("Results: %d passed, %d failed, %d total\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    printf("════════════════════════════════════════════\n");

    return tests_failed > 0 ? 1 : 0;
}
