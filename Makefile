# ══════════════════════════════════════════════════════════════════════
#  Mini OS — Makefile
#  Freestanding Systems Programming in C
#
#  Structure:
#    main.c     → Entry point (clean bootstrap)
#    shell.c    → Shell REPL loop and commands
#    memory.c   → Virtual heap allocator
#    string.c   → String parser (strlen, strcpy, strcmp, split, itoa)
#    keyboard.c → Keyboard input handler
#    screen.c   → Screen output layer (print, clear)
#    math.c     → Arithmetic engine
# ══════════════════════════════════════════════════════════════════════

CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c99

SRC = src/main.c src/shell.c src/memory.c src/math.c src/string.c src/screen.c src/keyboard.c
LIB = src/memory.c src/math.c src/string.c src/screen.c src/keyboard.c

TARGET = mini_os

.PHONY: all test clean

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Game (Track A)
game:
	$(CC) $(CFLAGS) -o mini_game src/game.c $(LIB)

# Tests
test: test_memory test_math test_string test_shell
	@echo ""
	@echo "Running all tests..."
	@echo ""
	@./test_memory
	@echo ""
	@./test_math
	@echo ""
	@./test_string
	@echo ""
	@./test_shell
	@echo ""
	@echo "All tests completed."

test_memory: tests/test_memory.c src/memory.c
	$(CC) $(CFLAGS) -o $@ $^

test_math: tests/test_math.c src/math.c
	$(CC) $(CFLAGS) -o $@ $^

test_string: tests/test_string.c src/string.c
	$(CC) $(CFLAGS) -o $@ $^

test_shell: tests/test_shell.c src/memory.c src/math.c src/string.c src/screen.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) mini_game test_memory test_math test_string test_shell