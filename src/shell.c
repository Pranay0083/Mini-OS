/*
 * shell.c — Interactive Shell Implementation
 *
 * Integrates all five custom libraries:
 *   keyboard.c → string.c → memory.c → math.c → screen.c
 */

#include "../include/shell.h"
#include "../include/memory.h"
#include "../include/math.h"
#include "../include/string.h"
#include "../include/screen.h"
#include "../include/keyboard.h"

/* ── Shell State ─────────────────────────────────────── */

static int shell_running = 1;

/* ── Commands ────────────────────────────────────────── */

static void cmd_help(void)
{
    scr_println("\nCommands:");
    scr_println("  help              Show this help message");
    scr_println("  echo <text>       Print text to console");
    scr_println("  clear             Clear screen");
    scr_println("  alloc <text>      Allocate memory and store text");
    scr_println("  free              Free the last allocation");
    scr_println("  calc <a> <op> <b> Calculator (+, -, *, /, %)");
    scr_println("  memmap            Show heap memory map");
    scr_println("  exit              Exit shell\n");
}

static void cmd_echo(char **tokens, int count)
{
    for (int i = 1; i < count; i++) {
        scr_print(tokens[i]);
        if (i < count - 1) scr_print(" ");
    }
    scr_print("\n");
}

/*
 * cmd_alloc — Demonstrates memory.c integration
 *
 * Pipeline: keyboard → string (parse) → memory (alloc + store) → screen (output)
 * Allocates a block via mem_alloc, copies user text into it with str_copy,
 * then prints confirmation with the stored string and byte count.
 */
static char *last_alloc = NULL;

static void cmd_alloc(char **tokens, int count)
{
    if (count < 2) {
        scr_println("  Usage: alloc <text>");
        return;
    }

    /* Reconstruct the text from tokens using string.c */
    char content[CMD_BUF_SIZE];
    content[0] = '\0';
    for (int i = 1; i < count; i++) {
        str_concat(content, tokens[i], CMD_BUF_SIZE);
        if (i < count - 1) str_concat(content, " ", CMD_BUF_SIZE);
    }

    int len = str_length(content);

    /* Allocate via memory.c */
    char *block = (char *)mem_alloc((size_t)(len + 1));
    if (!block) {
        scr_println("  Error: out of memory");
        return;
    }

    /* Store text via string.c */
    str_copy(block, content, len + 1);
    last_alloc = block;

    /* Display result using str_itoa (string.c → screen.c) */
    char size_buf[16];
    str_itoa(len, size_buf, 16);

    scr_print("  Allocated ");
    scr_print(size_buf);
    scr_print(" bytes: \"");
    scr_print(block);
    scr_print("\"\n");
}

static void cmd_free(void)
{
    if (!last_alloc) {
        scr_println("  Nothing to free");
        return;
    }

    mem_free(last_alloc);
    last_alloc = NULL;
    scr_println("  Memory freed");
}

/*
 * cmd_calc — Demonstrates math.c integration
 *
 * Pipeline: keyboard → string (parse + atoi) → math (compute) → screen (output)
 * Usage: calc 10 + 5
 */
static void cmd_calc(char **tokens, int count)
{
    if (count < 4) {
        scr_println("  Usage: calc <a> <op> <b>");
        scr_println("  Operators: + - * / %");
        return;
    }

    int a = str_atoi(tokens[1]);
    int b = str_atoi(tokens[3]);
    int result = 0;
    int valid = 1;
    char op = tokens[2][0];

    if (op == '+')      result = m_add(a, b);
    else if (op == '-') result = m_sub(a, b);
    else if (op == '*') result = m_mul(a, b);
    else if (op == '/') {
        if (b == 0) {
            scr_println("  Error: division by zero");
            return;
        }
        result = m_div(a, b);
    }
    else if (op == '%') {
        if (b == 0) {
            scr_println("  Error: modulo by zero");
            return;
        }
        result = m_mod(a, b);
    }
    else {
        scr_print("  Error: unknown operator '");
        scr_print(tokens[2]);
        scr_println("'");
        valid = 0;
    }

    if (valid) {
        char a_buf[16], b_buf[16], r_buf[16];
        str_itoa(a, a_buf, 16);
        str_itoa(b, b_buf, 16);
        str_itoa(result, r_buf, 16);

        scr_print("  ");
        scr_print(a_buf);
        scr_print(" ");
        scr_print(tokens[2]);
        scr_print(" ");
        scr_print(b_buf);
        scr_print(" = ");
        scr_print(r_buf);
        scr_print("\n");
    }
}

/* ── Command Dispatcher ──────────────────────────────── */

void shell_execute(char **tokens, int count)
{
    if (count == 0 || !tokens[0]) return;

    const char *cmd = tokens[0];

    if (str_compare(cmd, "help") == 0) {
        cmd_help();
    }
    else if (str_compare(cmd, "echo") == 0) {
        cmd_echo(tokens, count);
    }
    else if (str_compare(cmd, "clear") == 0) {
        scr_clear();
    }
    else if (str_compare(cmd, "alloc") == 0) {
        cmd_alloc(tokens, count);
    }
    else if (str_compare(cmd, "free") == 0) {
        cmd_free();
    }
    else if (str_compare(cmd, "calc") == 0) {
        cmd_calc(tokens, count);
    }
    else if (str_compare(cmd, "memmap") == 0) {
        mem_dump();
    }
    else if (str_compare(cmd, "exit") == 0) {
        shell_running = 0;
    }
    else {
        scr_print("Unknown command: ");
        scr_print(cmd);
        scr_println(". Type 'help'.");
    }
}

/* ── Init ───────────────────────────────────────────── */

void shell_init(void)
{
    shell_running = 1;
}

/* ── REPL LOOP (CORE OF PHASE 1) ───────────────────── */

void shell_run(void)
{
    char buffer[CMD_BUF_SIZE];
    char *tokens[MAX_TOKENS];

    scr_println("Mini OS v1.0 — Type 'help' for commands.");

    while (shell_running) {

        /* Step 1: Prompt */
        scr_print("mini-os:/$ ");

        /* Step 2: Input */
        int len = kb_read_line(buffer, CMD_BUF_SIZE);
        if (len <= 0) continue;

        /* Step 3: Parse (string.c) */
        int count = str_split(buffer, ' ', tokens, MAX_TOKENS);
        if (count <= 0) continue;

        /* Step 4: Execute */
        shell_execute(tokens, count);
    }

    scr_println("Shutting down...");
}

/* ── Status ─────────────────────────────────────────── */

int shell_is_running(void)
{
    return shell_running;
}
