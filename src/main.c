/*
 * main.c — Mini OS Entry Point
 *
 * This is the system bootstrap. It initializes all subsystems
 * and launches the interactive shell.
 *
 * Architecture:
 *   main.c     → Entry point, system bootstrap
 *   shell.c    → Shell REPL loop and command logic
 *   memory.c   → Virtual heap allocator
 *   string.c   → String parser (strlen, strcpy, strcmp, split, itoa)
 *   keyboard.c → Keyboard input handler
 *   screen.c   → Screen output layer
 *   math.c     → Arithmetic engine
 *
 * Integration flow:
 *   1. main() initializes memory, keyboard, shell
 *   2. shell_run() enters the REPL loop
 *   3. Each iteration: read (keyboard.c) → parse (string.c) → execute → output (screen.c)
 *   4. Memory allocations go through memory.c
 */

#include "../include/memory.h"
#include "../include/keyboard.h"
#include "../include/shell.h"
#include "../include/screen.h"
#include "../include/vfs.h"
#include "../include/scheduler.h"

/* ── Default Boot Configuration ───────────────────────────────────────── */

#define DEFAULT_SCR_WIDTH   80
#define DEFAULT_SCR_HEIGHT  24

/* ── Virtual RAM (backing store for the heap allocator) ───────────────── */

static char virtual_ram[VIRTUAL_RAM_SIZE];

/* ══════════════════════════════════════════════════════════════════════ */
/*                     MAIN ENTRY POINT                                  */
/* ══════════════════════════════════════════════════════════════════════ */

int main(void)
{
    /* ── Step 1: Initialize memory subsystem ───────────────────────── */
    mem_init(virtual_ram, VIRTUAL_RAM_SIZE);

    /* ── Step 2: Initialize keyboard input ─────────────────────────── */
    kb_init();

    /* ── Step 3: Initialize screen output ──────────────────────────── */
    scr_init(DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT);

    /* ── Step 4: Initialize subsystems ─────────────────────────────── */
    vfs_init();   
    sched_init(); 
    shell_init();

    /* ── Step 5: Run the shell REPL loop ───────────────────────────── */
    shell_run();

    /* ── Step 6: Cleanup and exit ──────────────────────────────────── */
    kb_restore();

    return 0;
}
