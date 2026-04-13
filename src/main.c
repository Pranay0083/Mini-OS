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
    scr_init(80, 24);

    /* ── Step 4: Initialize shell subsystem ────────────────────────── */
    shell_init();

    /* ── Step 5: Run the shell REPL loop ───────────────────────────── */
    /*
     * shell_run() contains the core loop:
     *   print("> ")      → screen.c
     *   readLine()       → keyboard.c
     *   parse()          → string.c (str_split)
     *   execute()        → shell.c (shell_execute)
     *   loop again
     */
    shell_run();

    /* ── Step 6: Cleanup and exit ──────────────────────────────────── */
    kb_restore();

    return 0;
}
