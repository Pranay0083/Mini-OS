# 🖥️ Mini OS — Freestanding Systems Programming in C

A comprehensive systems programming capstone project that builds **five core C libraries from scratch** and integrates them into two complete applications — a real-time **Snake game** and a **Mini Operating System** with VFS and shell — all without standard library dependencies for core logic.

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

- [Architecture](#architecture)
- [Libraries](#libraries)
- [Applications](#applications)
- [Building](#building)
- [Running](#running)
- [Testing](#testing)
- [Documentation](#documentation)
- [Project Structure](#project-structure)

---

## 🏗️ Architecture

The system follows a four-layer model where each layer depends only on layers below it:

```
┌─────────────────────────────────────────────────┐
│  Layer 4: Applications                          │
│  ┌──────────────┐  ┌────────────────────────┐   │
│  │  Snake Game   │  │  Mini OS (VFS + Shell) │   │
│  └──────────────┘  └────────────────────────┘   │
├─────────────────────────────────────────────────┤
│  Layer 3: Middleware                            │
│  Framebuffer · VFS · Task Scheduler · Entities  │
├─────────────────────────────────────────────────┤
│  Layer 2: Custom Libraries (Engine)             │
│  memory.c · math.c · string.c · screen.c ·     │
│  keyboard.c                                     │
├─────────────────────────────────────────────────┤
│  Layer 1: Hardware Abstraction (Permitted)      │
│  stdio.h · stdlib.h · termios.h · fcntl.h ·    │
│  unistd.h                                       │
└─────────────────────────────────────────────────┘
```

---

## 📚 Libraries

### `memory.c` — Virtual Heap Allocator
- **First-Fit Free List** algorithm with 64KB Virtual RAM
- 8-byte aligned allocations: `(size + 7) & ~7`
- Block splitting when remainder ≥ `MIN_BLOCK_SIZE`
- Forward coalescing on free to prevent fragmentation
- Safety: NULL/double-free/bounds/zero-size protection

### `math.c` — Arithmetic Engine
- Integer operations: `abs`, `min`, `max`, `clamp`, `mod`, `div`, `mul`
- Spatial helpers: AABB intersection, point-in-rectangle, Manhattan distance
- Linear Congruential Generator (LCG) for pseudo-random numbers

### `string.c` — Parser
- Bounds-safe: `length`, `copy`, `compare`, `concat`
- Numeric conversions: `itoa`, `atoi` with sign handling
- Tokenizer: `str_split` with delimiter collapsing
- Utilities: `starts_with`, `find`, `reverse`

### `screen.c` — Terminal Renderer
- ANSI escape code based 2D rendering
- Double-buffered framebuffer (front + back)
- **Diff-based refresh**: only transmits changed cells
- Box drawing, colored text, cursor control

### `keyboard.c` — Input Handler
- Raw mode terminal via `termios` (no echo, no line buffering)
- Non-blocking `kb_key_pressed()` via `O_NONBLOCK`
- Arrow key escape sequence parsing
- Blocking `kb_read_line()` with backspace and character echo
- Guaranteed terminal restoration via `atexit()`

---

## 🎮 Applications

### Track A: Snake Game (`mini_game`)
- Real-time game loop: poll input → update state → render
- Snake segments dynamically allocated via `mem_alloc`/`mem_free`
- Wall and self-collision detection via `math.c`
- Score display via `str_itoa` + framebuffer rendering
- Difficulty scaling: speed increases as score grows
- Controls: **WASD** or **Arrow keys** | **Q** to quit | **R** to restart

### Track B: Mini OS (`mini_os`)
**Virtual File System:**
- Superblock + Inode table (64 entries)
- File CRUD: `touch`, `write`, `read`/`cat`, `rm`
- Directory hierarchy: `mkdir`, `cd`, `ls`

**Shell Commands:**
| Command | Description |
|---------|-------------|
| `help` | Show all commands |
| `echo <text>` | Print text |
| `clear` | Clear screen |
| `ls` | List files |
| `touch <name>` | Create file |
| `mkdir <name>` | Create directory |
| `cd <dir>` | Change directory |
| `write <name> <text>` | Write to file |
| `read` / `cat <name>` | Read file |
| `rm <name>` | Delete file/dir |
| `memmap` | Heap visualization |
| `sysinfo` | System info |
| `tasks` | List background tasks |
| `startcounter` | Start counter task |
| `kill <id>` | Kill task |
| `exit` | Shutdown |

**Cooperative Task Scheduler:**
- Background tasks ticked once per shell iteration
- Counter task for demonstration
- Task management: add, list, kill

---

## 🔨 Building

**Prerequisites:** `clang` or `gcc` with C99 support (macOS/Linux)

```bash
# Build both applications
make all

# Build only the game
make game

# Build only the OS
make os
```

---

## 🚀 Running

```bash
# Play Snake
./mini_game

# Launch Mini OS
./mini_os
```

---

## 🧪 Testing

```bash
# Run all test suites (memory + math + string)
make test
```

**Test coverage:**
- `test_memory`: 20 tests — alloc, free, splitting, coalescing, stress (1000 cycles)
- `test_math`: 28 tests — arithmetic, AABB, distance, PRNG
- `test_string`: 39 tests — copy, compare, itoa/atoi, split, edge cases

---

## 📖 Documentation

Comprehensive design documents are in the `/docs` directory:

| Document | Description |
|----------|-------------|
| [SRS.md](docs/SRS.md) | Systems Requirement Specification |
| [Architecture.md](docs/Architecture.md) | Layered architecture with Mermaid diagrams |
| [MemoryDesign.md](docs/MemoryDesign.md) | First-Fit allocator design |
| [MathDesign.md](docs/MathDesign.md) | Arithmetic engine design |
| [StringDesign.md](docs/StringDesign.md) | Parser library design |
| [ScreenDesign.md](docs/ScreenDesign.md) | ANSI renderer design |
| [KeyboardDesign.md](docs/KeyboardDesign.md) | Raw mode input design |
| [TrackA_GameDesign.md](docs/TrackA_GameDesign.md) | Snake game design |
| [TrackB_OSDesign.md](docs/TrackB_OSDesign.md) | Mini OS design |

All documents include **Mermaid diagrams** for architecture, data flow, algorithms, and state machines.

---

## 📁 Project Structure

```
mini-os/
├── docs/                    # Design documentation (9 docs)
│   ├── SRS.md
│   ├── Architecture.md
│   ├── MemoryDesign.md
│   ├── MathDesign.md
│   ├── StringDesign.md
│   ├── ScreenDesign.md
│   ├── KeyboardDesign.md
│   ├── TrackA_GameDesign.md
│   └── TrackB_OSDesign.md
├── include/                 # Header files
│   ├── memory.h
│   ├── math.h
│   ├── string.h
│   ├── screen.h
│   └── keyboard.h
├── src/                     # Implementation files
│   ├── memory.c             # Virtual Heap Allocator
│   ├── math.c               # Arithmetic Engine
│   ├── string.c             # String Parser
│   ├── screen.c             # ANSI Terminal Renderer
│   ├── keyboard.c           # Raw Mode Input Handler
│   ├── game.c               # Track A: Snake Game
│   └── os.c                 # Track B: Mini OS
├── tests/                   # Unit test suites
│   ├── test_memory.c        # 20 tests
│   ├── test_math.c          # 28 tests
│   └── test_string.c        # 39 tests
├── Makefile
├── .gitignore
└── README.md
```

---

## ⚙️ Constraints

- **No standard library for core logic**: Zero usage of `<string.h>`, `<math.h>`, or standard `malloc`/`free`
- **Permitted headers only**: `<stdio.h>`, `<stdlib.h>`, `<termios.h>`, `<fcntl.h>`, `<unistd.h>`
- **C99 compliance**: Compiles with `-Wall -Wextra -Werror -std=c99 -pedantic` (zero warnings)
- **8-byte alignment**: All heap allocations guaranteed aligned

---

## 📜 License

Educational capstone project. MIT License.
