# Track A — Snake Game Design

## 1. Overview

The Snake game demonstrates seamless integration of all five custom libraries. Every entity is allocated via `memory.c`, rendered via `screen.c`, controlled via `keyboard.c`, bounded via `math.c`, and score-formatted via `string.c`.

## 2. Game Loop

```mermaid
graph TB
    subgraph "Game Loop (~80ms per frame)"
        INPUT["1. Poll Input — kb_key_pressed()"]
        UPDATE["2. Update State — Move, Grow, Collide"]
        RENDER["3. Render Frame — scr_refresh()"]
    end
    INPUT --> UPDATE --> RENDER --> INPUT
```

## 3. Data Structures

### Snake Segment (Linked List)
```c
typedef struct SnakeSegment {
    int x, y;
    struct SnakeSegment *next;
} SnakeSegment;
```

### Game State
```c
typedef struct {
    SnakeSegment *head, *tail;
    int dir_x, dir_y;
    int food_x, food_y;
    int score, speed, game_over;
    int width, height;
} GameState;
```

## 4. Movement & Collision

```mermaid
flowchart TD
    A["Each Frame"] --> B["new_x = head.x + dir_x"]
    B --> C{"Hit wall?"}
    C -->|Yes| D["Game Over"]
    C -->|No| E{"Hit self?"}
    E -->|Yes| D
    E -->|No| F["Alloc new head segment"]
    F --> G{"On food?"}
    G -->|Yes| H["Score++, keep tail"]
    G -->|No| I["Free tail segment"]
```

## 5. Rendering Layout

| Element | Char | Color |
|---------|------|-------|
| Snake Head | `@` | Bright Green |
| Snake Body | `#` | Green |
| Food | `*` | Red |
| Wall | `#` | White |
| Score | — | Yellow |

## 6. Difficulty Scaling

```
delay = max(40000, 120000 - score * 2000) microseconds
```

## 7. Game Over → Restart or Quit
