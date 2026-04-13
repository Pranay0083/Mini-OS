/*
 * shell.c — Interactive Shell Implementation
 *
 * Clean shell abstraction with proper library integration:
 *   - Input:   keyboard.c   → kb_read_line()
 *   - Parsing: string.c     → str_split() custom tokenizer
 *   - Memory:  memory.c     → mem_alloc() / mem_free()
 *   - Output:  screen.c     → scr_print() / scr_clear()
 *   - Math:    math.c       → used in calculator command
 *
 * No standard library string functions (strcmp, strlen, etc.) are used.
 * All command dispatch is token-based via the custom str_split() tokenizer.
 */

#include "../include/shell.h"
#include "../include/memory.h"
#include "../include/math.h"
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

/* ══════════════════════════════════════════════════════════════════════ */
/*                     VFS IMPLEMENTATION                                */
/* ══════════════════════════════════════════════════════════════════════ */

static void vfs_init(void)
{
    superblock.total_files = 0;
    superblock.total_dirs  = 1;  /* root */
    superblock.current_dir = 0;

    /* Clear inode table */
    for (int i = 0; i < MAX_INODES; i++) {
        inode_table[i].is_used = 0;
        inode_table[i].data    = NULL;
        inode_table[i].size    = 0;
        inode_table[i].is_dir  = 0;
        inode_table[i].parent  = -1;
        inode_table[i].name[0] = '\0';
    }

    /* Create root directory (inode 0) */
    str_copy(inode_table[0].name, "/", MAX_NAME_LEN);
    inode_table[0].is_used = 1;
    inode_table[0].is_dir  = 1;
    inode_table[0].parent  = -1;
}

static int vfs_find_free_inode(void)
{
    for (int i = 0; i < MAX_INODES; i++) {
        if (!inode_table[i].is_used) return i;
    }
    return -1;
}

static int vfs_find_by_name(const char *name, int parent)
{
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].is_used &&
            inode_table[i].parent == parent &&
            str_compare(inode_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int vfs_touch(const char *name)
{
    if (!name || str_length(name) == 0) {
        scr_print("  Error: filename required\n");
        return -1;
    }

    /* Check if already exists */
    if (vfs_find_by_name(name, superblock.current_dir) >= 0) {
        scr_print("  Error: '");
        scr_print(name);
        scr_print("' already exists\n");
        return -1;
    }

    int idx = vfs_find_free_inode();
    if (idx < 0) {
        scr_print("  Error: inode table full\n");
        return -1;
    }

    str_copy(inode_table[idx].name, name, MAX_NAME_LEN);
    inode_table[idx].is_used = 1;
    inode_table[idx].is_dir  = 0;
    inode_table[idx].size    = 0;
    inode_table[idx].data    = NULL;
    inode_table[idx].parent  = superblock.current_dir;

    superblock.total_files++;
    return idx;
}

static int vfs_mkdir(const char *name)
{
    if (!name || str_length(name) == 0) {
        scr_print("  Error: directory name required\n");
        return -1;
    }

    if (vfs_find_by_name(name, superblock.current_dir) >= 0) {
        scr_print("  Error: '");
        scr_print(name);
        scr_print("' already exists\n");
        return -1;
    }

    int idx = vfs_find_free_inode();
    if (idx < 0) {
        scr_print("  Error: inode table full\n");
        return -1;
    }

    str_copy(inode_table[idx].name, name, MAX_NAME_LEN);
    inode_table[idx].is_used = 1;
    inode_table[idx].is_dir  = 1;
    inode_table[idx].size    = 0;
    inode_table[idx].data    = NULL;
    inode_table[idx].parent  = superblock.current_dir;

    superblock.total_dirs++;
    return idx;
}

static int vfs_write(const char *name, const char *content)
{
    int idx = vfs_find_by_name(name, superblock.current_dir);
    if (idx < 0) {
        /* Auto-create the file */
        idx = vfs_touch(name);
        if (idx < 0) return -1;
    }

    if (inode_table[idx].is_dir) {
        scr_print("  Error: '");
        scr_print(name);
        scr_print("' is a directory\n");
        return -1;
    }

    /* Free old data if any */
    if (inode_table[idx].data) {
        mem_free(inode_table[idx].data);
        inode_table[idx].data = NULL;
    }

    int content_len = str_length(content);
    if (content_len == 0) {
        inode_table[idx].size = 0;
        return 0;
    }

    /* Allocate new data block via memory.c */
    char *block = (char *)mem_alloc((size_t)(content_len + 1));
    if (!block) {
        scr_print("  Error: out of memory\n");
        return -1;
    }

    str_copy(block, content, content_len + 1);
    inode_table[idx].data = block;
    inode_table[idx].size = content_len;

    return 0;
}

static void vfs_read(const char *name)
{
    int idx = vfs_find_by_name(name, superblock.current_dir);
    if (idx < 0) {
        scr_print("  Error: file '");
        scr_print(name);
        scr_print("' not found\n");
        return;
    }

    if (inode_table[idx].is_dir) {
        scr_print("  Error: '");
        scr_print(name);
        scr_print("' is a directory\n");
        return;
    }

    if (inode_table[idx].data && inode_table[idx].size > 0) {
        scr_print("  ");
        scr_print(inode_table[idx].data);
        scr_print("\n");
    } else {
        scr_print("  (empty file)\n");
    }
}

static void vfs_rm(const char *name)
{
    int idx = vfs_find_by_name(name, superblock.current_dir);
    if (idx < 0) {
        scr_print("  Error: '");
        scr_print(name);
        scr_print("' not found\n");
        return;
    }

    /* If directory, check if empty */
    if (inode_table[idx].is_dir) {
        for (int i = 0; i < MAX_INODES; i++) {
            if (inode_table[i].is_used && inode_table[i].parent == idx) {
                scr_print("  Error: directory '");
                scr_print(name);
                scr_print("' is not empty\n");
                return;
            }
        }
        superblock.total_dirs--;
    } else {
        superblock.total_files--;
    }

    /* Free data via memory.c */
    if (inode_table[idx].data) {
        mem_free(inode_table[idx].data);
    }

    inode_table[idx].is_used = 0;
    inode_table[idx].data    = NULL;
    inode_table[idx].size    = 0;
    inode_table[idx].name[0] = '\0';

    scr_print("  Removed '");
    scr_print(name);
    scr_print("'\n");
}

static void vfs_ls(void)
{
    int count = 0;
    scr_print("\n");

    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].is_used &&
            inode_table[i].parent == superblock.current_dir) {

            char size_buf[16];
            str_itoa(inode_table[i].size, size_buf, 16);

            if (inode_table[i].is_dir) {
                scr_print("  \033[34m");
                scr_print(inode_table[i].name);
                scr_print("/\033[0m\n");
            } else {
                scr_print("  ");
                scr_print(inode_table[i].name);
                scr_print("  ");
                scr_print(size_buf);
                scr_print(" B\n");
            }
            count++;
        }
    }

    if (count == 0) {
        scr_print("  (empty directory)\n");
    }

    char count_buf[16];
    str_itoa(count, count_buf, 16);
    scr_print("\n  ");
    scr_print(count_buf);
    scr_print(" item(s)\n");
}

static void vfs_cd(const char *name)
{
    if (!name || str_length(name) == 0 || str_compare(name, "/") == 0) {
        superblock.current_dir = 0;
        return;
    }

    if (str_compare(name, "..") == 0) {
        int parent = inode_table[superblock.current_dir].parent;
        if (parent >= 0) {
            superblock.current_dir = parent;
        }
        return;
    }

    int idx = vfs_find_by_name(name, superblock.current_dir);
    if (idx < 0) {
        scr_print("  Error: directory '");
        scr_print(name);
        scr_print("' not found\n");
        return;
    }

    if (!inode_table[idx].is_dir) {
        scr_print("  Error: '");
        scr_print(name);
        scr_print("' is not a directory\n");
        return;
    }

    superblock.current_dir = idx;
}

/* ══════════════════════════════════════════════════════════════════════ */
/*                     TASK SCHEDULER                                    */
/* ══════════════════════════════════════════════════════════════════════ */

static void task_init(void)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        task_table[i].active = 0;
        task_table[i].tick   = NULL;
        task_table[i].state  = NULL;
    }
}

static int task_add(const char *name, void (*tick_fn)(void *), void *state)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_table[i].active) {
            str_copy(task_table[i].name, name, MAX_NAME_LEN);
            task_table[i].tick   = tick_fn;
            task_table[i].state  = state;
            task_table[i].active = 1;
            return i;
        }
    }
    scr_print("  Error: task table full\n");
    return -1;
}

static void task_tick_all(void)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_table[i].active && task_table[i].tick) {
            task_table[i].tick(task_table[i].state);
        }
    }
}

static void task_list(void)
{
    int count = 0;
    scr_print("\n  Active Background Tasks:\n");
    scr_print("  ID   Name             Status\n");
    scr_print("  ──── ──────────────── ────────\n");

    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_table[i].active) {
            char id_buf[8];
            str_itoa(i, id_buf, 8);
            scr_print("  ");
            scr_print(id_buf);
            scr_print("    ");
            scr_print(task_table[i].name);
            scr_print("           running\n");
            count++;
        }
    }

    if (count == 0) {
        scr_print("  (no active tasks)\n");
    }
    scr_print("\n");
}

static void task_kill(int id)
{
    if (id < 0 || id >= MAX_TASKS || !task_table[id].active) {
        scr_print("  Error: invalid task ID\n");
        return;
    }

    /* Free state via memory.c */
    if (task_table[id].state) {
        mem_free(task_table[id].state);
    }

    scr_print("  Killed task '");
    scr_print(task_table[id].name);
    scr_print("'\n");
    task_table[id].active = 0;
    task_table[id].tick   = NULL;
    task_table[id].state  = NULL;
}

/* ── Background Counter Task ─────────────────────────────────────────── */

static void counter_tick(void *state)
{
    CounterState *cs = (CounterState *)state;
    cs->count++;
    /* Silently increment — display only on 'tasks' command */
}

/* ══════════════════════════════════════════════════════════════════════ */
/*                     SHELL COMMANDS                                    */
/* ══════════════════════════════════════════════════════════════════════ */

static void cmd_help(void)
{
    scr_print("\n");
    scr_print("  \033[36m╔══════════════════════════════════════════════════╗\033[0m\n");
    scr_print("  \033[36m║\033[0m         \033[33mMini OS — Command Reference\033[0m            \033[36m║\033[0m\n");
    scr_print("  \033[36m╠══════════════════════════════════════════════════╣\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mhelp\033[0m                 Show this help message    \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mecho\033[0m <text>          Print text to console     \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mclear\033[0m                Clear the screen          \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mls\033[0m                   List files in directory   \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mtouch\033[0m <name>         Create empty file         \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mmkdir\033[0m <name>         Create directory          \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mcd\033[0m <dir>             Change directory          \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mwrite\033[0m <name> <text>  Write content to file     \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mread\033[0m / \033[32mcat\033[0m <name>    Display file contents     \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mrm\033[0m <name>            Remove file or directory  \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mmemmap\033[0m               Show heap memory map      \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mtasks\033[0m                List background tasks     \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mkill\033[0m <id>            Kill a background task    \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mstartcounter\033[0m         Start counter task        \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32msysinfo\033[0m              System information        \033[36m║\033[0m\n");
    scr_print("  \033[36m║\033[0m  \033[32mexit\033[0m                 Shutdown Mini OS           \033[36m║\033[0m\n");
    scr_print("  \033[36m╚══════════════════════════════════════════════════╝\033[0m\n");
    scr_print("\n");
}

/*
 * cmd_echo — Print text to console
 *
 * Uses: string.c for token traversal, screen.c for output
 *
 * Input: "echo hello world"
 * → tokens: ["echo", "hello", "world"]
 * → output: "hello world"
 */
static void cmd_echo(char **tokens, int count)
{
    for (int i = 1; i < count; i++) {
        scr_print(tokens[i]);
        if (i < count - 1) scr_print(" ");
    }
    scr_print("\n");
}

static void cmd_sysinfo(void)
{
    char buf[16];

    scr_print("\n");
    scr_print("  \033[33m╔═══════════════════════════════════╗\033[0m\n");
    scr_print("  \033[33m║       System Information          ║\033[0m\n");
    scr_print("  \033[33m╠═══════════════════════════════════╣\033[0m\n");

    str_itoa((int)VIRTUAL_RAM_SIZE, buf, 16);
    scr_print("  \033[33m║\033[0m  Virtual RAM:  ");
    scr_print(buf);
    scr_print(" bytes     \033[33m║\033[0m\n");

    str_itoa((int)mem_available(), buf, 16);
    scr_print("  \033[33m║\033[0m  Free Memory: ");
    scr_print(buf);
    scr_print(" bytes     \033[33m║\033[0m\n");

    str_itoa(mem_block_count(), buf, 16);
    scr_print("  \033[33m║\033[0m  Heap Blocks: ");
    scr_print(buf);
    scr_print("           \033[33m║\033[0m\n");

    str_itoa(superblock.total_files, buf, 16);
    scr_print("  \033[33m║\033[0m  Files:       ");
    scr_print(buf);
    scr_print("           \033[33m║\033[0m\n");

    str_itoa(superblock.total_dirs, buf, 16);
    scr_print("  \033[33m║\033[0m  Directories: ");
    scr_print(buf);
    scr_print("           \033[33m║\033[0m\n");

    str_itoa(MAX_INODES, buf, 16);
    scr_print("  \033[33m║\033[0m  Max Inodes:  ");
    scr_print(buf);
    scr_print("           \033[33m║\033[0m\n");

    scr_print("  \033[33m╚═══════════════════════════════════╝\033[0m\n\n");
}

/* ── Build current path string ────────────────────────────────────────── */

static void get_current_path(char *buf, int buf_size)
{
    if (superblock.current_dir == 0) {
        str_copy(buf, "/", buf_size);
        return;
    }

    /* Build path by traversing parent chain */
    char parts[8][MAX_NAME_LEN];
    int depth = 0;
    int cur = superblock.current_dir;

    while (cur > 0 && depth < 8) {
        str_copy(parts[depth], inode_table[cur].name, MAX_NAME_LEN);
        depth++;
        cur = inode_table[cur].parent;
    }

    buf[0] = '\0';
    for (int i = depth - 1; i >= 0; i--) {
        str_concat(buf, "/", buf_size);
        str_concat(buf, parts[i], buf_size);
    }
}

/* ══════════════════════════════════════════════════════════════════════ */
/*                     COMMAND EXECUTION                                 */
/* ══════════════════════════════════════════════════════════════════════ */

/*
 * shell_execute — Command dispatcher
 *
 * Takes tokenized input from str_split() and dispatches to the
 * appropriate command handler based on token[0].
 *
 * This is NOT hardcoded — it uses str_compare() from string.c
 * to match command names against tokens produced by the custom
 * tokenizer (str_split) from string.c.
 */
void shell_execute(char **tokens, int count)
{
    if (count == 0) return;

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
    else if (str_compare(cmd, "ls") == 0) {
        vfs_ls();
    }
    else if (str_compare(cmd, "touch") == 0) {
        if (count < 2) {
            scr_print("  Usage: touch <filename>\n");
        } else {
            if (vfs_touch(tokens[1]) >= 0) {
                scr_print("  Created '");
                scr_print(tokens[1]);
                scr_print("'\n");
            }
        }
    }
    else if (str_compare(cmd, "mkdir") == 0) {
        if (count < 2) {
            scr_print("  Usage: mkdir <dirname>\n");
        } else {
            if (vfs_mkdir(tokens[1]) >= 0) {
                scr_print("  Created directory '");
                scr_print(tokens[1]);
                scr_print("'\n");
            }
        }
    }
    else if (str_compare(cmd, "cd") == 0) {
        vfs_cd(count >= 2 ? tokens[1] : "/");
    }
    else if (str_compare(cmd, "write") == 0) {
        if (count < 3) {
            scr_print("  Usage: write <filename> <content...>\n");
        } else {
            /* Reconstruct content from tokens 2+ using string.c */
            char content[MAX_DATA_SIZE];
            content[0] = '\0';
            for (int i = 2; i < count; i++) {
                str_concat(content, tokens[i], MAX_DATA_SIZE);
                if (i < count - 1) str_concat(content, " ", MAX_DATA_SIZE);
            }
            if (vfs_write(tokens[1], content) == 0) {
                char len_buf[16];
                str_itoa(str_length(content), len_buf, 16);
                scr_print("  Written to '");
                scr_print(tokens[1]);
                scr_print("' (");
                scr_print(len_buf);
                scr_print(" bytes)\n");
            }
        }
    }
    else if (str_compare(cmd, "read") == 0 || str_compare(cmd, "cat") == 0) {
        if (count < 2) {
            scr_print("  Usage: ");
            scr_print(cmd);
            scr_print(" <filename>\n");
        } else {
            vfs_read(tokens[1]);
        }
    }
    else if (str_compare(cmd, "rm") == 0) {
        if (count < 2) {
            scr_print("  Usage: rm <name>\n");
        } else {
            vfs_rm(tokens[1]);
        }
    }
    else if (str_compare(cmd, "memmap") == 0) {
        mem_dump();
    }
    else if (str_compare(cmd, "sysinfo") == 0) {
        cmd_sysinfo();
    }
    else if (str_compare(cmd, "tasks") == 0) {
        task_list();
    }
    else if (str_compare(cmd, "kill") == 0) {
        if (count < 2) {
            scr_print("  Usage: kill <task_id>\n");
        } else {
            task_kill(str_atoi(tokens[1]));
        }
    }
    else if (str_compare(cmd, "startcounter") == 0) {
        /* Allocate counter state via memory.c */
        CounterState *cs = (CounterState *)mem_alloc(sizeof(CounterState));
        if (cs) {
            cs->count = 0;
            int id = task_add("counter", counter_tick, cs);
            if (id >= 0) {
                char id_buf[8];
                str_itoa(id, id_buf, 8);
                scr_print("  Started counter task (ID: ");
                scr_print(id_buf);
                scr_print(")\n");
            }
        } else {
            scr_print("  Error: out of memory\n");
        }
    }
    else if (str_compare(cmd, "exit") == 0) {
        shell_running = 0;
    }
    else {
        /* Unknown command — proper error handling */
        scr_print("  Unknown command: '");
        scr_print(cmd);
        scr_print("'. Type 'help' for commands.\n");
    }
}

/* ══════════════════════════════════════════════════════════════════════ */
/*                     BOOT BANNER                                       */
/* ══════════════════════════════════════════════════════════════════════ */

static void print_banner(void)
{
    scr_clear();

    scr_print("\033[36m");
    scr_print("  ╔══════════════════════════════════════════════════════╗\n");
    scr_print("  ║                                                      ║\n");
    scr_print("  ║   ███╗   ███╗██╗███╗   ██╗██╗     ██████╗ ███████╗   ║\n");
    scr_print("  ║   ████╗ ████║██║████╗  ██║██║    ██╔═══██╗██╔════╝   ║\n");
    scr_print("  ║   ██╔████╔██║██║██╔██╗ ██║██║    ██║   ██║███████╗   ║\n");
    scr_print("  ║   ██║╚██╔╝██║██║██║╚██╗██║██║    ██║   ██║╚════██║   ║\n");
    scr_print("  ║   ██║ ╚═╝ ██║██║██║ ╚████║██║    ╚██████╔╝███████║   ║\n");
    scr_print("  ║   ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝     ╚═════╝ ╚══════╝   ║\n");
    scr_print("  ║                                                      ║\n");
    scr_print("  ║   Freestanding Mini Operating System v1.0            ║\n");
    scr_print("  ║   Built with custom C libraries — no libc            ║\n");
    scr_print("  ║                                                      ║\n");
    scr_print("  ║   Type 'help' for available commands                 ║\n");
    scr_print("  ║                                                      ║\n");
    scr_print("  ╚══════════════════════════════════════════════════════╝\n");
    scr_print("\033[0m\n");
}

/* ══════════════════════════════════════════════════════════════════════ */
/*                     SHELL PUBLIC API                                  */
/* ══════════════════════════════════════════════════════════════════════ */

void shell_init(void)
{
    vfs_init();
    task_init();
    shell_running = 1;
}

int shell_is_running(void)
{
    return shell_running;
}

/*
 * shell_run — The REPL Loop (NON-NEGOTIABLE)
 *
 * This is the core shell loop that:
 *   1. Prints prompt: "mini-os:/path$ "
 *   2. Reads input via keyboard.c → kb_read_line()
 *   3. Parses via string.c → str_split() custom tokenizer
 *   4. Executes command via shell_execute()
 *   5. Ticks background tasks
 *   6. Loops again
 *
 * Integration:
 *   Step         Library Used
 *   ─────────    ────────────
 *   Input        keyboard.c
 *   Parsing      string.c
 *   Memory       memory.c
 *   Output       screen.c
 */
void shell_run(void)
{
    char cmd_buf[CMD_BUF_SIZE];
    char *tokens[MAX_TOKENS];

    print_banner();

    while (shell_running) {
        /* ── Step 1: Print prompt ──────────────────────────────────── */
        char path[128];
        get_current_path(path, 128);
        scr_print("\033[32mmini-os\033[0m:\033[34m");
        scr_print(path);
        scr_print("\033[0m$ ");
        fflush(stdout);

        /* ── Step 2: Read input via keyboard.c ─────────────────────── */
        int len = kb_read_line(cmd_buf, CMD_BUF_SIZE);
        if (len == 0) continue;

        /* ── Step 3: Parse via string.c custom tokenizer ───────────── */
        int count = str_split(cmd_buf, ' ', tokens, MAX_TOKENS);
        if (count == 0) continue;

        /* ── Step 4: Execute command ───────────────────────────────── */
        shell_execute(tokens, count);

        /* ── Step 5: Tick background tasks ─────────────────────────── */
        task_tick_all();
    }

    /* ── Shutdown ───────────────────────────────────────────────── */
    scr_print("\n\033[33m  Shutting down Mini OS...\033[0m\n");
    scr_print("  \033[32mGoodbye!\033[0m\n\n");
}
