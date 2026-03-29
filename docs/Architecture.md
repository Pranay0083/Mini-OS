# System Architecture

## 1. Overview

The Mini OS project follows a **layered architecture** where each layer depends only on the layers below it. This ensures clean separation of concerns and allows the foundational libraries to be tested independently of the application logic.

---

## 2. The Four-Layer Model

```mermaid
graph TB
    subgraph "Layer 4: Application Layer"
        A1["Track A: Snake Game<br/>(game.c)"]
        A2["Track B: Mini OS<br/>(os.c)"]
    end

    subgraph "Layer 3: Middleware Service Layer"
        M1["Framebuffer Manager"]
        M2["Virtual File System"]
        M3["Task Scheduler"]
        M4["Game Entity Manager"]
    end

    subgraph "Layer 2: Foundational Library Layer (Engine)"
        L1["memory.c<br/>Virtual Heap"]
        L2["math.c<br/>Arithmetic Engine"]
        L3["string.c<br/>Parser"]
        L4["screen.c<br/>Renderer"]
        L5["keyboard.c<br/>Input Handler"]
    end

    subgraph "Layer 1: Hardware Abstraction Layer (HAL)"
        H1["stdio.h<br/>printf, putchar"]
        H2["stdlib.h<br/>malloc, exit"]
        H3["termios.h<br/>tcsetattr"]
        H4["fcntl.h<br/>fcntl"]
        H5["unistd.h<br/>read, usleep"]
    end

    A1 --> M1
    A1 --> M4
    A2 --> M2
    A2 --> M3

    M1 --> L4
    M2 --> L1
    M2 --> L3
    M3 --> L1
    M4 --> L1
    M4 --> L2

    L1 --> H2
    L4 --> H1
    L5 --> H3
    L5 --> H4
    L5 --> H5
```

---

## 3. Module Dependency Graph

```mermaid
graph LR
    memory["memory.c"]
    math["math.c"]
    string["string.c"]
    screen["screen.c"]
    keyboard["keyboard.c"]
    game["game.c"]
    os["os.c"]

    game --> memory
    game --> math
    game --> string
    game --> screen
    game --> keyboard

    os --> memory
    os --> math
    os --> string
    os --> screen
    os --> keyboard

    screen --> string
    string --> memory
```

---

## 4. Memory Map — Virtual RAM Layout

The entire system operates within a single contiguous block of memory allocated once at startup. The custom allocator then manages this block.

```mermaid
graph TB
    subgraph "Virtual RAM (64 KB)"
        direction TB
        A["0x0000 — BlockHeader (root)<br/>size: 65536, free: 1"]
        B["... Heap managed by memory.c ..."]
        C["Allocated blocks interspersed<br/>with free blocks"]
        D["0xFFFF — End of Virtual RAM"]
    end

    style A fill:#e74c3c,color:#fff
    style B fill:#3498db,color:#fff
    style C fill:#2ecc71,color:#fff
    style D fill:#e74c3c,color:#fff
```

### Memory Regions

| Region | Size | Purpose | Management |
|--------|------|---------|------------|
| Static Data | Compile-time | Global variables, constants | Fixed allocation |
| The Heap | ~64 KB | Dynamic objects, file blocks, buffers | `memory.c` First-Fit Free List |
| VFS Table | Variable | Inode metadata, directory structures | Heap allocations |
| Input Buffer | 256 bytes | Raw terminal input bytes | Linear buffer in `keyboard.c` |
| Framebuffer | W×H×2 | Front + back screen buffers | Static arrays in `screen.c` |

---

## 5. Data Flow Pipeline

```mermaid
sequenceDiagram
    participant KB as keyboard.c
    participant STR as string.c
    participant MEM as memory.c
    participant MATH as math.c
    participant SCR as screen.c

    Note over KB: User presses key
    KB->>STR: Raw byte sequence
    STR->>STR: Tokenize input
    STR->>MEM: Allocate storage for state
    MEM-->>STR: Pointer to allocated block
    STR->>MATH: Compute logic (movement/collision)
    MATH-->>SCR: Updated coordinates
    SCR->>SCR: Diff framebuffer
    SCR->>SCR: Emit ANSI escape codes
    Note over SCR: Terminal updated
```

---

## 6. Build Architecture

```mermaid
graph LR
    subgraph "Source Files"
        S1["memory.c"]
        S2["math.c"]
        S3["string.c"]
        S4["screen.c"]
        S5["keyboard.c"]
        S6["game.c"]
        S7["os.c"]
    end

    subgraph "Object Files"
        O1["memory.o"]
        O2["math.o"]
        O3["string.o"]
        O4["screen.o"]
        O5["keyboard.o"]
        O6["game.o"]
        O7["os.o"]
    end

    subgraph "Binaries"
        B1["mini_game"]
        B2["mini_os"]
        B3["test_runner"]
    end

    S1 --> O1
    S2 --> O2
    S3 --> O3
    S4 --> O4
    S5 --> O5
    S6 --> O6
    S7 --> O7

    O1 & O2 & O3 & O4 & O5 & O6 --> B1
    O1 & O2 & O3 & O4 & O5 & O7 --> B2
    O1 & O2 & O3 --> B3
```

---

## 7. Error Handling Strategy

The system uses a layered error reporting approach:

| Layer | Strategy | Example |
|-------|----------|---------|
| memory.c | Return `NULL` on failure | `mem_alloc(too_large)` → `NULL` |
| string.c | Truncate with null terminator | `str_copy(dst, src, 5)` → safe truncation |
| math.c | Clamp values, avoid UB | `m_div(x, 0)` → return 0 with error flag |
| screen.c | Silently ignore out-of-bounds | `scr_put_char(-1, -1, ...)` → no-op |
| keyboard.c | Return sentinel values | `kb_key_pressed()` → 0 if no key |

---

## 8. Thread of Control

```mermaid
stateDiagram-v2
    [*] --> Init: Program Start
    Init --> MemInit: Allocate Virtual RAM
    MemInit --> LibInit: Initialize screen + keyboard
    
    state "Track Selection" as TS {
        LibInit --> GameLoop: Track A
        LibInit --> ShellLoop: Track B
    }

    state "Game Loop" as GameLoop {
        PollInput --> UpdateState
        UpdateState --> Render
        Render --> PollInput
    }

    state "Shell Loop" as ShellLoop {
        Prompt --> ReadLine
        ReadLine --> Tokenize
        Tokenize --> Execute
        Execute --> TickTasks
        TickTasks --> Prompt
    }

    GameLoop --> Cleanup: Game Over / Quit
    ShellLoop --> Cleanup: exit command
    Cleanup --> [*]: Restore terminal
```
