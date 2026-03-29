# keyboard.c — Input Handler Design

## 1. Overview

The keyboard library provides non-blocking input capture by switching the terminal from its default **canonical mode** to **raw mode**. This enables real-time keystroke detection for both the game loop (Track A) and the interactive shell (Track B).

---

## 2. Terminal Modes

```mermaid
graph LR
    subgraph "Canonical Mode (Default)"
        CM1["Input buffered until Enter"]
        CM2["Characters echoed to screen"]
        CM3["Line editing enabled"]
    end

    subgraph "Raw Mode (keyboard.c)"
        RM1["Each keystroke immediately available"]
        RM2["No echo"]
        RM3["No line buffering"]
        RM4["Special keys not interpreted"]
    end

    CM1 -.->|"kb_init()"| RM1
    RM1 -.->|"kb_restore()"| CM1
```

---

## 3. Raw Mode Configuration

### 3.1 termios Setup

```c
struct termios raw;
tcgetattr(STDIN_FILENO, &raw);

// Save original settings for restoration
original_termios = raw;

// Modify flags
raw.c_lflag &= ~(ECHO | ICANON);     // No echo, no line buffering
raw.c_cc[VMIN] = 0;                    // Non-blocking: return immediately
raw.c_cc[VTIME] = 0;                   // No timeout

tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
```

### 3.2 Non-Blocking I/O via fcntl

```c
int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
```

---

## 4. Key Detection Flow

```mermaid
flowchart TD
    A["kb_key_pressed()"] --> B["read(STDIN, &ch, 1)"]
    B --> C{"Return value?"}
    C -->|"> 0"| D{"ch == 27 (ESC)?"}
    C -->|"<= 0"| E["Return 0<br/>(no key pressed)"]
    D -->|No| F["Return ch<br/>(ASCII key)"]
    D -->|Yes| G["Read 2 more bytes<br/>(escape sequence)"]
    G --> H{"Sequence?"}
    H -->|"[A"| I["Return KEY_UP"]
    H -->|"[B"| J["Return KEY_DOWN"]
    H -->|"[C"| K["Return KEY_RIGHT"]
    H -->|"[D"| L["Return KEY_LEFT"]
    H -->|"other"| M["Return KEY_ESCAPE"]
```

### Arrow Key Escape Sequences

Arrow keys send 3-byte sequences:

| Key | Byte 1 | Byte 2 | Byte 3 | Constant |
|-----|--------|--------|--------|----------|
| Up | `27` (ESC) | `91` ([) | `65` (A) | `KEY_UP = 1000` |
| Down | `27` (ESC) | `91` ([) | `66` (B) | `KEY_DOWN = 1001` |
| Right | `27` (ESC) | `91` ([) | `67` (C) | `KEY_RIGHT = 1002` |
| Left | `27` (ESC) | `91` ([) | `68` (D) | `KEY_LEFT = 1003` |

---

## 5. Line Reading (Shell Mode)

```mermaid
flowchart TD
    A["kb_read_line(buf, max)"] --> B["Initialize pos = 0"]
    B --> C["Block: read one byte"]
    C --> D{"Byte value?"}
    D -->|"\\n or \\r"| E["buf[pos] = \\0<br/>Return pos"]
    D -->|"127 (Backspace)"| F{"pos > 0?"}
    F -->|Yes| G["pos--, erase char on screen"]
    F -->|No| C
    G --> C
    D -->|"Printable char"| H{"pos < max-1?"}
    H -->|Yes| I["buf[pos++] = ch<br/>Echo char to screen"]
    H -->|No| C
    I --> C
```

---

## 6. Terminal Restoration

**Critical**: The terminal MUST be restored to its original state when the program exits, otherwise the user's shell will remain in raw mode.

```mermaid
flowchart LR
    A["Program Start"] --> B["kb_init()<br/>Save + enter raw mode"]
    B --> C["Application runs..."]
    C --> D["kb_restore()<br/>Restore original termios"]
    D --> E["Program exit"]
```

The restoration function is registered with `atexit()` to ensure it runs even on unexpected termination.

---

## 7. API Reference

| Function | Signature | Description |
|----------|-----------|-------------|
| `kb_init` | `void kb_init(void)` | Enter raw mode, save original settings |
| `kb_restore` | `void kb_restore(void)` | Restore canonical mode |
| `kb_key_pressed` | `int kb_key_pressed(void)` | Non-blocking key read (0 = none) |
| `kb_read_line` | `int kb_read_line(char* buf, int max)` | Blocking line read with echo |

### Key Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `KEY_UP` | 1000 | Up arrow |
| `KEY_DOWN` | 1001 | Down arrow |
| `KEY_LEFT` | 1002 | Left arrow |
| `KEY_RIGHT` | 1003 | Right arrow |
| `KEY_ESCAPE` | 27 | Escape key |
| `KEY_ENTER` | 10 | Enter/Return |
| `KEY_BACKSPACE` | 127 | Backspace/Delete |
