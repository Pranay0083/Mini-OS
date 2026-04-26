# Mini OS — Freestanding Operating System in C

A simulated command-line operating system built from scratch in C. Includes a shell, a virtual file system, and task scheduling — all using custom-built libraries with no standard library dependencies for core logic.

---

## Architecture

```
Input → Parse → Execute → Output
keyboard.c → string.c → shell.c → screen.c
                           ↓
                    memory.c + math.c
                           ↓
                    vfs.c + scheduler.c
```

| Layer | Components |
|-------|-----------|
| Application | shell.c (REPL + dispatch), vfs.c (file system), scheduler.c (task manager) |
| Libraries | memory.c, string.c, math.c, screen.c, keyboard.c |
| Hardware Abstraction | stdio.h, stdlib.h, termios.h, fcntl.h, unistd.h |

---

## Shell Commands

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `echo <text>` | Print text to console |
| `clear` | Clear the screen |
| `alloc <text>` | Allocate memory and store text |
| `free` | Free the last allocation |
| `calc <a> <op> <b>` | Calculator (+, -, *, /, %) |
| `memmap` | Show heap memory map |
| `ls [-a]` | List files with sizes (-a for hidden files) |
| `touch <name>` | Create empty file |
| `write <name> <text>` | Write to file (auto-creates if needed) |
| `read <name>` | Read file contents |
| `rm <name>` | Delete file |
| `run <name>` | Execute file as script |
| `starttask` | Start background task |
| `tasks` | List background tasks |
| `kill <id>` | Kill a background task |
| `exit` | Shut down Mini OS |

---

## Building & Running

```bash
make            # Build
./mini_os       # Run
make test       # Run all tests (125 tests)
make clean      # Clean
```

### Demo

```
$ ./mini_os
Mini OS v1.0 — Type 'help' for commands.

mini-os:/$ touch hello.txt
mini-os:/$ write hello.txt hello world
Written 11 bytes to 'hello.txt'

mini-os:/$ read hello.txt
hello world

mini-os:/$ ls
  hello.txt  11 B
1 file(s)

mini-os:/$ rm hello.txt
Removed 'hello.txt'

mini-os:/$ starttask
Started task ID: 1
[Tasks running in background]

mini-os:/$ tasks
Task ID: 1 | elapsed time (s): 3

mini-os:/$ kill 1
Task killed

mini-os:/$ calc 42 * 10
Result: 420

mini-os:/$ exit
Shutting down...
```

---

## Testing

| Test Suite | Tests | Coverage |
|-----------|-------|---------|
| `test_memory.c` | 20 | Alloc, free, splitting, coalescing, stress, edge cases |
| `test_math.c` | 28 | All arithmetic, AABB, distance, PRNG, clamp |
| `test_string.c` | 39 | length, copy, compare, split, itoa, atoi, concat, reverse |
| `test_shell.c` | 38 | Integration: tokenize → match → execute → output |

**All 125 tests pass with `-Wall -Wextra -Werror -std=c99`.**

---

## Project Structure

```
Mini-OS/
├── src/
│   ├── main.c          # System bootstrap
│   ├── shell.c         # REPL loop + command dispatch
│   ├── vfs.c           # Virtual File System
│   ├── scheduler.c     # Cooperative task scheduler
│   ├── memory.c        # Virtual heap allocator
│   ├── string.c        # String parser (no <string.h>)
│   ├── keyboard.c      # Raw terminal input
│   ├── screen.c        # Screen output
│   └── math.c          # Arithmetic engine
├── include/
│   ├── shell.h, memory.h, string.h, math.h
│   ├── keyboard.h, screen.h
│   ├── vfs.h, scheduler.h
├── tests/
│   ├── test_memory.c, test_math.c
│   ├── test_string.c, test_shell.c
├── Makefile
└── README.md
```

---

## Constraints

- **No `<string.h>`** — custom `string.c`
- **No `<math.h>`** — custom `math.c`
- **No `malloc`/`free`** — custom `memory.c`
- **`<stdio.h>`** only in screen.c (output) and memory.c (debug dump)
- **C99 compliant** — builds with `-Wall -Wextra -Werror`

---

## Known Issues

- `alloc`/`free` tracks only the most recent allocation (single-slot demo)
- `calc` uses integer arithmetic only
- `run` limited to files with newline-separated commands
- Terminal may need `reset` if program is killed abnormally

---

Educational project. MIT License.
