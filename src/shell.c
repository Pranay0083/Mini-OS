#include "../include/shell.h"
#include "../include/memory.h"
#include "../include/math.h"
#include "../include/string.h"
#include "../include/screen.h"
#include "../include/keyboard.h"
#include "../include/vfs.h"
#include "../include/scheduler.h"

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

    scr_println("  ls [-a]           List files (-a for hidden)");
    scr_println("  touch <name>      Create file");
    scr_println("  write <name> <text> Write to file");
    scr_println("  read <name>       Read file");
    scr_println("  rm <name>         Delete file");
    scr_println("  run <name>        Execute file as script");

    scr_println("  starttask         Start background task");
    scr_println("  tasks             List tasks");
    scr_println("  kill <id>         Kill task");

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

/* ── Memory Commands ─────────────────────────────────── */

static char *last_alloc = NULL;

static void cmd_alloc(char **tokens, int count)
{
    if (count < 2) {
        scr_println("  Usage: alloc <text>");
        return;
    }

    char content[CMD_BUF_SIZE] = {0};
    for (int i = 1; i < count; i++) {
        str_concat(content, tokens[i], CMD_BUF_SIZE);
        if (i < count - 1) str_concat(content, " ", CMD_BUF_SIZE);
    }

    int len = str_length(content);

    char *block = (char *)mem_alloc((size_t)(len + 1));
    if (!block) {
        scr_println("  Error: out of memory");
        return;
    }

    str_copy(block, content, len + 1);
    last_alloc = block;

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

/* ── Calculator ─────────────────────────────────────── */

static void cmd_calc(char **tokens, int count)
{
    if (count < 4) {
        scr_println("Usage: calc <a> <op> <b>");
        return;
    }

    int a = str_atoi(tokens[1]);
    int b = str_atoi(tokens[3]);
    char op = tokens[2][0];

    int result = 0;

    if (op == '+') result = m_add(a, b);
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
        scr_println("Error: invalid operator");
        return;
    }

    char buf[16];
    str_itoa(result, buf, 16);

    scr_print("Result: ");
    scr_println(buf);
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

    /* ── VFS ───────────────────── */

    else if (str_compare(cmd, "ls") == 0) {
        int show_hidden = 0;
        if (count > 1 && str_compare(tokens[1], "-a") == 0) {
            show_hidden = 1;
        }
        vfs_list(show_hidden);
    }
    else if (str_compare(cmd, "touch") == 0) {
        if (count > 1) vfs_create(tokens[1]);
        else scr_println("Usage: touch <name>");
    }
    else if (str_compare(cmd, "write") == 0) {
        if (count > 2) {
            char buffer[CMD_BUF_SIZE] = {0};
            for (int i = 2; i < count; i++) {
                str_concat(buffer, tokens[i], CMD_BUF_SIZE);
                if (i < count - 1) str_concat(buffer, " ", CMD_BUF_SIZE);
            }
            vfs_write(tokens[1], buffer);
        } else {
            scr_println("Usage: write <name> <text>");
        }
    }
    else if (str_compare(cmd, "read") == 0) {
        if (count > 1) {
            char *data = vfs_read(tokens[1]);
            if (data) scr_println(data);
        } else {
            scr_println("Usage: read <name>");
        }
    }
    else if (str_compare(cmd, "rm") == 0) {
        if (count > 1) vfs_delete(tokens[1]);
        else scr_println("Usage: rm <name>");
    }
    else if (str_compare(cmd, "run") == 0) {
        if (count < 2) {
            scr_println("Usage: run <filename>");
        } else {
            char *script = vfs_read(tokens[1]);
            if (script) {
                /* Copy script content — str_split modifies in place */
                char line_buf[CMD_BUF_SIZE];
                str_copy(line_buf, script, CMD_BUF_SIZE);

                /* Split by newline to get individual commands */
                char *lines[MAX_TOKENS];
                int nlines = str_split(line_buf, '\n', lines, MAX_TOKENS);

                for (int i = 0; i < nlines; i++) {
                    if (str_length(lines[i]) == 0) continue;

                    scr_print("> ");
                    scr_println(lines[i]);

                    char cmd_copy[CMD_BUF_SIZE];
                    str_copy(cmd_copy, lines[i], CMD_BUF_SIZE);

                    char *cmd_tokens[MAX_TOKENS];
                    int cmd_count = str_split(cmd_copy, ' ', cmd_tokens, MAX_TOKENS);
                    if (cmd_count > 0) {
                        shell_execute(cmd_tokens, cmd_count);
                    }
                }
            }
        }
    }

    /* ── Scheduler ───────────────── */

    else if (str_compare(cmd, "starttask") == 0) {
        sched_start_task();
    }
    else if (str_compare(cmd, "tasks") == 0) {
        sched_list();
    }
    else if (str_compare(cmd, "kill") == 0) {
        if (count > 1) {
            int id = str_atoi(tokens[1]);
            sched_kill(id);
        } else {
            scr_println("Usage: kill <id>");
        }
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

/* ── REPL LOOP ───────────────────── */

void shell_run(void)
{
    char buffer[CMD_BUF_SIZE];
    char *tokens[MAX_TOKENS];

    scr_println("Mini OS v1.0 — Type 'help' for commands.");

    while (shell_running) {

        for (int i = 0; i < 1000; i++) {
            sched_update();
        }

        scr_print("mini-os:/$ ");

        int len = kb_read_line(buffer, CMD_BUF_SIZE);
        if (len <= 0) continue;

        int count = str_split(buffer, ' ', tokens, MAX_TOKENS);
        if (count <= 0) continue;

        shell_execute(tokens, count);
    }

    scr_println("Shutting down...");
}

/* ── Status ─────────────────────────────────────────── */

int shell_is_running(void)
{
    return shell_running;
}
