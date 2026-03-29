# Systems Requirement Specification (SRS)

## 1. Introduction

This document defines the complete functional and non-functional requirements for the **Mini OS** project — a freestanding systems programming capstone that builds five core C libraries from scratch and integrates them into two application tracks: an interactive Snake game (Track A) and a Mini Operating System with VFS and shell (Track B).

### 1.1 Purpose

To establish clear, measurable requirements that guide the implementation of a freestanding C runtime, ensuring all modules interact correctly without reliance on the C Standard Library for core logic.

### 1.2 Scope

The system encompasses:
- Five foundational libraries: `memory.c`, `math.c`, `string.c`, `screen.c`, `keyboard.c`
- Track A: Real-time Snake game
- Track B: Mini OS with Virtual File System, command shell, and cooperative task scheduler

### 1.3 Permitted Dependencies

| Header | Purpose | Usage Scope |
|--------|---------|-------------|
| `<stdio.h>` | `printf`, `putchar`, `fflush` | Terminal I/O simulation only |
| `<stdlib.h>` | `malloc` (one-time), `exit` | Initial bulk memory acquisition, process exit |
| `<termios.h>` | `tcgetattr`, `tcsetattr` | Raw mode terminal configuration |
| `<fcntl.h>` | `fcntl` | Non-blocking stdin configuration |
| `<unistd.h>` | `read`, `usleep` | Low-level byte read, frame timing |

---

## 2. Functional Requirements

### 2.1 Overview Diagram

```mermaid
graph TB
    subgraph "Functional Requirements"
        FR1["FR-1: Virtual Heap Management"]
        FR2["FR-2: Arithmetic & Boundary Logic"]
        FR3["FR-3: Safe String Manipulation"]
        FR4["FR-4: Terminal Rendering Engine"]
        FR5["FR-5: Non-Blocking Input Capture"]
        FR6["FR-6: VFS & Shell (Track B)"]
        FR7["FR-7: Real-Time Game Loop (Track A)"]
    end

    FR1 --> FR3
    FR1 --> FR6
    FR1 --> FR7
    FR2 --> FR7
    FR3 --> FR6
    FR4 --> FR6
    FR4 --> FR7
    FR5 --> FR6
    FR5 --> FR7
```

### 2.2 Detailed Requirements

| ID | Requirement | Description | Priority |
|----|------------|-------------|----------|
| FR-1 | Virtual Heap Management | Initialize a contiguous memory region and provide `mem_alloc()` and `mem_free()` with First-Fit search, block splitting, and forward coalescing. | **Critical** |
| FR-2 | Arithmetic & Boundary Logic | Implement `m_abs`, `m_mod`, `m_div`, `m_clamp`, `m_min`, `m_max`, and AABB intersection detection without `<math.h>`. | **High** |
| FR-3 | Safe String Manipulation | Provide `str_length`, `str_copy`, `str_compare`, `str_concat`, `str_itoa`, `str_atoi`, and `str_split` with bounds-safe semantics. | **Critical** |
| FR-4 | Terminal Rendering Engine | Interface with terminal via ANSI escape codes. Support cursor positioning, colored character output, box drawing, and diff-based framebuffer refresh. | **Critical** |
| FR-5 | Non-Blocking Input Capture | Switch terminal to raw mode. Implement `kb_key_pressed()` for non-blocking reads and `kb_read_line()` for interactive line input with echo. | **Critical** |
| FR-6 | VFS & Shell | Implement Virtual File System with inodes, superblock, and directory support. Shell must parse and execute: `help`, `ls`, `touch`, `write`, `read`, `rm`, `cat`, `echo`, `memmap`, `clear`. | **High** |
| FR-7 | Real-Time Game Loop | Snake game with dynamic entity allocation, collision detection, score tracking, difficulty scaling, and smooth rendering at consistent frame rate. | **High** |

---

## 3. Non-Functional Requirements

| ID | Requirement | Description | Metric |
|----|------------|-------------|--------|
| NFR-1 | Freestanding Execution | No standard library calls for core logic | Zero `#include` of `<string.h>`, `<math.h>`, standard `malloc`/`free` in core libs |
| NFR-2 | Memory Safety | Deterministic allocation with coalescing | No memory leaks after 1000+ alloc/free cycles |
| NFR-3 | Real-Time Performance | Consistent frame rate for Track A | Input-to-render latency < 50ms |
| NFR-4 | Zero Hard-Coded Logic | All logic computed dynamically | No magic numbers for boundaries or parsing |
| NFR-5 | Alignment | Heap allocations are 8-byte aligned | `(ptr & 7) == 0` for all returned pointers |
| NFR-6 | Portability | Compiles with `-Wall -Wextra -std=c99` | Zero warnings on macOS clang |

---

## 4. Constraint Matrix

```mermaid
graph LR
    subgraph "Constraints"
        C1["C1: No libc for core logic"]
        C2["C2: Single bulk malloc at startup"]
        C3["C3: POSIX terminal only"]
        C4["C4: C99 standard compliance"]
    end

    subgraph "Enabled By"
        E1["stdio.h → Terminal simulation"]
        E2["stdlib.h → Bulk memory + exit"]
        E3["termios.h → Raw mode"]
        E4["fcntl.h → Non-blocking I/O"]
    end

    C1 --> E1
    C2 --> E2
    C3 --> E3
    C3 --> E4
```

---

## 5. Use Case Diagram

```mermaid
graph TB
    User((User))
    
    subgraph "Track A: Snake Game"
        UC1["Move Snake (WASD/Arrows)"]
        UC2["Eat Food & Grow"]
        UC3["View Score"]
        UC4["Game Over / Restart"]
    end

    subgraph "Track B: Mini OS"
        UC5["Enter Shell Commands"]
        UC6["Create/Delete Files"]
        UC7["Read/Write File Contents"]
        UC8["View Memory Map"]
        UC9["Run Background Tasks"]
    end

    User --> UC1
    User --> UC2
    User --> UC3
    User --> UC4
    User --> UC5
    User --> UC6
    User --> UC7
    User --> UC8
    User --> UC9
```

---

## 6. Acceptance Criteria

1. **All five libraries compile** with `clang -Wall -Wextra -Werror -std=c99 -pedantic`
2. **Memory tests pass**: 1000+ alloc/free cycles with zero leaks and correct coalescing
3. **Snake game is playable**: responsive controls, correct collision, visible score
4. **Mini OS shell is functional**: all 10 commands execute correctly
5. **Background tasks run**: clock/counter updates while shell is interactive
6. **No standard library usage**: verified by `nm` output showing no libc symbol references in core logic
