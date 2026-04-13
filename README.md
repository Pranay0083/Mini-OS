# 🖥️ Mini OS — Freestanding Operating System in C

A mini operating system built entirely from scratch in C, demonstrating core OS concepts — **memory management**, **file system**, **I/O abstraction**, **process scheduling**, and a **command shell** — all using **custom-built libraries** with no standard library dependencies for core logic.

```
╔══════════════════════════════════════════════════════╗
║                                                      ║
║   ███╗   ███╗██╗███╗   ██╗██╗     ██████╗ ███████╗   ║
║   ████╗ ████║██║████╗  ██║██║    ██╔═══██╗██╔════╝   ║
║   ██╔████╔██║██║██╔██╗ ██║██║    ██║   ██║███████╗   ║
║   ██║╚██╔╝██║██║██║╚██╗██║██║    ██║   ██║╚════██║   ║
║   ██║ ╚═╝ ██║██║██║ ╚████║██║    ╚██████╔╝███████║   ║
║   ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝     ╚═════╝ ╚══════╝   ║
║                                                      ║
╚══════════════════════════════════════════════════════╝
```

---

## 📋 Table of Contents

- [Architecture](#-architecture)
- [Core Pipeline](#-core-pipeline)
- [Libraries](#-libraries)
- [Shell Commands](#-shell-commands)
- [OS Concepts Demonstrated](#-os-concepts-demonstrated)
- [Building & Running](#-building--running)
- [Testing](#-testing)
- [Project Structure](#-project-structure)
- [Documentation](#-documentation)

---

## 🏗️ Architecture

```
┌──────────────────────────────────────────────────┐
│              USER (types commands)                │
└────────────────────┬─────────────────────────────┘
                     │
         ┌───────────▼───────────┐
         │      keyboard.c       │  INPUT LAYER
         │    kb_read_line()     │  Raw terminal mode
         └───────────┬───────────┘
                     │
         ┌───────────▼───────────┐
         │       string.c        │  PARSING ENGINE
         │     str_split()       │  In-place tokenizer
         └───────────┬───────────┘
                     │
         ┌───────────▼───────────┐
         │       shell.c         │  COMMAND DISPATCH
         │   shell_execute()     │  REPL loop + VFS
         └───────┬───┬───────────┘
                 │   │
        ┌────────┘   └────────┐
        ▼                     ▼
┌──────────────┐    ┌──────────────┐
│  memory.c    │    │  screen.c    │
│  mem_alloc() │    │  scr_print() │  OUTPUT LAYER
│  mem_free()  │    │  scr_clear() │
└──────────────┘    └──────────────┘
```

The system follows a layered model where each layer depends only on layers below it:

```
┌─────────────────────────────────────────────────┐
│  Layer 3: Application                           │
│  Shell REPL · VFS · Task Scheduler              │
├─────────────────────────────────────────────────┤
│  Layer 2: Custom Libraries                      │
│  memory.c · math.c · string.c · screen.c ·     │
│  keyboard.c                                     │
├─────────────────────────────────────────────────┤
│  Layer 1: Hardware Abstraction (Permitted)      │
│  stdio.h · stdlib.h · termios.h · fcntl.h ·    │
│  unistd.h                                       │
└─────────────────────────────────────────────────┘
```

---

## 🔄 Core Pipeline

Every command follows this exact flow:

```
Input → Parse → Execute → Output
keyboard.c → string.c → shell.c → screen.c
                           ↓
                        memory.c (used where needed)
```

| Step | Library | Function |
|------|---------|----------|
| Input | `keyboard.c` | `kb_read_line()` |
| Parsing | `string.c` | `str_split()` |
| Cmd Match | `string.c` | `str_compare()` |
| Memory | `memory.c` | `mem_alloc()` / `mem_free()` |
| Output | `screen.c` | `scr_print()` / `scr_clear()` |

---

## 📚 Libraries

### `memory.c` — Virtual Heap Allocator
- **First-Fit Free List** algorithm over 64KB virtual RAM
- 8-byte aligned allocations
- Block splitting when remainder ≥ `MIN_BLOCK_SIZE`
- Forward coalescing on free to prevent fragmentation
- Safety: NULL/double-free/bounds/zero-size protection
- `memmap` command for heap visualization

### `string.c` — String Parser (No `<string.h>`)
- Core: `str_length`, `str_copy`, `str_compare`, `str_concat`
- **⭐ `str_split`** — In-place tokenizer (tokens point inside buffer, no malloc)
- Numeric: `str_itoa`, `str_atoi` with sign handling
- Utilities: `str_starts_with`, `str_find`, `str_reverse`

### `keyboard.c` — Raw Terminal Input
- Raw mode via `termios` (no echo, no line buffering)
- Non-blocking `kb_key_pressed()` via `O_NONBLOCK`
- Blocking `kb_read_line()` with:
  - Instant character echo (flushed per keystroke)
  - Backspace handling
  - Enter detection
- Guaranteed terminal restoration via `atexit()`

### `screen.c` — Output Abstraction
- `scr_print()` / `scr_println()` — console output wrappers
- `scr_clear()` — ANSI screen clear
- Double-buffered framebuffer with diff-based refresh
- Box drawing, colored text, cursor control

### `math.c` — Arithmetic Engine (No `<math.h>`)
- Basic: `m_add`, `m_sub`, `m_mul`, `m_div`, `m_mod`, `m_abs`
- Helpers: `m_min`, `m_max`, `m_clamp`
- Spatial: AABB intersection, point-in-rect, Manhattan distance
- PRNG: Linear Congruential Generator with `m_rand()`, `m_rand_range()`

---

## 💻 Shell Commands

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `echo <text>` | Print text to console |
| `clear` | Clear the screen |
| `ls` | List files in current directory |
| `touch <name>` | Create empty file |
| `mkdir <name>` | Create directory |
| `cd <dir>` | Change directory (`..` and `/` supported) |
| `write <name> <text>` | Write content to file |
| `read` / `cat <name>` | Display file contents |
| `rm <name>` | Remove file or empty directory |
| `memmap` | Show heap memory map |
| `sysinfo` | Display system information |
| `tasks` | List background tasks |
| `startcounter` | Start a background counter task |
| `kill <id>` | Kill a background task |
| `exit` | Shut down Mini OS |

---

## 🧠 OS Concepts Demonstrated

| OS Concept | Implementation | Status |
|-----------|---------------|--------|
| **Memory Management** | First-Fit allocator, 64KB heap, alloc/free/split/coalesce | ✅ Done |
| **File System** | Inode-based VFS with directories, CRUD, parent-child hierarchy | ✅ Done |
| **I/O Management** | Input (keyboard.c) + Output (screen.c) abstraction layers | ✅ Done |
| **User Interface** | Shell REPL with 16 commands, colored prompt, error handling | ✅ Done |
| **Error Handling** | Null checks, bounds validation, unknown cmd, OOM, double-free | ✅ Done |
| **Halt / Shutdown** | `exit` → graceful shutdown → terminal restore → clean return | ✅ Done |
| **Process Management** | Cooperative task scheduler (startcounter, tasks, kill) | ✅ Partial |

### Virtual File System
- **Superblock** — tracks total files, directories, current directory
- **Inode table** — 64 entries, each with name, size, type, parent index, data pointer
- **File data** — allocated via `mem_alloc()`, freed via `mem_free()`
- **Directory hierarchy** — parent-child relationships, `cd ..` traversal

### Cooperative Task Scheduler
- Background tasks tick once per shell loop iteration
- `startcounter` — spawns a counter task
- `tasks` — lists active tasks with IDs
- `kill <id>` — terminates a task and frees its memory

---

## 🔨 Building & Running

**Prerequisites:** `clang` or `gcc` with C99 support (macOS/Linux)

```bash
# Build the OS
make

# Run Mini OS
./mini_os

# Clean build artifacts
make clean
```

### Demo Flow

```
$ ./mini_os

mini-os:/$ help
mini-os:/$ echo hello mini os
hello mini os
mini-os:/$ mkdir projects
mini-os:/$ touch hello.txt
mini-os:/$ write hello.txt hello world
mini-os:/$ cat hello.txt
  hello world
mini-os:/$ memmap
mini-os:/$ sysinfo
mini-os:/$ clear
mini-os:/$ exit
  Goodbye!
```

---

## 🧪 Testing

```bash
# Run all test suites (125 tests)
make test
```

| Test Suite | Tests | Coverage |
|-----------|-------|---------|
| `test_memory.c` | 20 | Alloc, free, splitting, coalescing, stress (1000 cycles), edge cases |
| `test_math.c` | 28 | All arithmetic ops, AABB, distance, PRNG, clamp |
| `test_string.c` | 39 | length, copy, compare, split, itoa, atoi, concat, reverse, find |
| `test_shell.c` | 38 | Full pipeline integration: tokenize → match → execute → output |

**All 125 tests pass with zero warnings under `-Wall -Wextra -Werror -std=c99`.**

---

## 📁 Project Structure

```
Mini-OS/
├── src/                         # Source files
│   ├── main.c                   # System bootstrap (entry point)
│   ├── shell.c                  # Shell REPL + VFS + task scheduler
│   ├── memory.c                 # Virtual heap allocator
│   ├── string.c                 # String parser (no <string.h>)
│   ├── keyboard.c               # Raw terminal input handler
│   ├── screen.c                 # Screen output abstraction
│   └── math.c                   # Arithmetic engine
├── include/                     # Header files
│   ├── shell.h
│   ├── memory.h
│   ├── string.h
│   ├── keyboard.h
│   ├── screen.h
│   └── math.h
├── tests/                       # Unit + integration tests
│   ├── test_memory.c            # 20 tests
│   ├── test_math.c              # 28 tests
│   ├── test_string.c            # 39 tests
│   └── test_shell.c             # 38 integration tests
├── docs/                        # Design documents
│   ├── SRS.md                   # System Requirements Specification
│   ├── Architecture.md          # Layered architecture
│   ├── MemoryDesign.md          # Allocator design
│   ├── MathDesign.md            # Arithmetic engine design
│   ├── StringDesign.md          # Parser library design
│   ├── ScreenDesign.md          # Renderer design
│   ├── KeyboardDesign.md        # Input handler design
│   └── TrackB_OSDesign.md       # OS design
├── Makefile
├── .gitignore
└── README.md
```

---

## 📖 Documentation

Design documents in `/docs`:

| Document | Description |
|----------|-------------|
| [SRS.md](docs/SRS.md) | System Requirements Specification |
| [Architecture.md](docs/Architecture.md) | Layered architecture design |
| [MemoryDesign.md](docs/MemoryDesign.md) | First-Fit allocator design |
| [MathDesign.md](docs/MathDesign.md) | Arithmetic engine design |
| [StringDesign.md](docs/StringDesign.md) | Parser library design |
| [ScreenDesign.md](docs/ScreenDesign.md) | ANSI renderer design |
| [KeyboardDesign.md](docs/KeyboardDesign.md) | Raw mode input design |
| [TrackB_OSDesign.md](docs/TrackB_OSDesign.md) | Mini OS design |

---

## ⚙️ Constraints

- **No standard library for core logic**: Zero usage of `<string.h>`, `<math.h>`, or standard `malloc`/`free`
- **Custom everything**: All string ops, memory management, and I/O go through custom libraries
- **C99 compliance**: Compiles with `-Wall -Wextra -Werror -std=c99` (zero warnings)
- **8-byte alignment**: All heap allocations guaranteed aligned

---

## 📜 License

Educational project. MIT License.
