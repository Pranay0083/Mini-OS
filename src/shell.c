#include "../include/shell.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../include/screen.h"
#include "../include/keyboard.h"

#include <stdio.h>   /* printf for formatted output only */

/* ── Configuration ────────────────────────────────────────────────────── */

#define MAX_INODES     64
#define MAX_NAME_LEN   32
#define MAX_DATA_SIZE  1024
#define MAX_TASKS      8

/* ── Virtual File System ──────────────────────────────────────────────── */

typedef struct {
    char  name[MAX_NAME_LEN];
    int   size;
    int   is_dir;
    int   is_used;
    char *data;        /* Pointer into heap (via memory.c) */
    int   parent;      /* Parent inode index (-1 for root) */
} Inode;

typedef struct {
    int  total_files;
    int  total_dirs;
    int  current_dir;  /* Current working directory inode index */
} Superblock;

static Inode      inode_table[MAX_INODES];
static Superblock superblock;

/* ── Cooperative Task Scheduler ───────────────────────────────────────── */

typedef struct {
    void (*tick)(void *state);
    void  *state;
    int    active;
    char   name[MAX_NAME_LEN];
} Task;

static Task task_table[MAX_TASKS];

/* Counter task state */
typedef struct {
    int count;
} CounterState;

/* ── Shell State ──────────────────────────────────────────────────────── */

static int shell_running = 1;

/* ── Basic Commands ───────────────────────────────────── */

static void cmd_help(void)
{
    scr_println("\nCommands:");
    scr_println("  help           Show this help message");
    scr_println("  echo <text>    Print text");
    scr_println("  clear          Clear screen");
    scr_println("  memmap         Show memory state");
    scr_println("  exit           Exit shell\n");
}

static void cmd_echo(char **tokens, int count)
{
    for (int i = 1; i < count; i++) {
        scr_print(tokens[i]);
        if (i < count - 1) scr_print(" ");
    }
    scr_print("\n");
}

/* ── Command Dispatcher ───────────────────────────────── */

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
    else if (str_compare(cmd, "memmap") == 0) {
        mem_dump();
    }
    else if (str_compare(cmd, "exit") == 0) {
        shell_running = 0;
    }
    else {
        scr_print("Unknown command: ");
        scr_print(cmd);
        scr_print("\n");
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

    scr_println("Mini OS Started. Type 'help'.");

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
