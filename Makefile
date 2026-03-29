# ══════════════════════════════════════════════════════════════════════
#  Mini OS — Makefile
#  Freestanding Systems Programming in C
# ══════════════════════════════════════════════════════════════════════

CC       = clang
CFLAGS   = -Wall -Wextra -Werror -std=c99 -pedantic
SRCDIR   = src
INCDIR   = include
TESTDIR  = tests

# Source files for the shared libraries
LIB_SRCS = $(SRCDIR)/memory.c $(SRCDIR)/math.c $(SRCDIR)/string.c \
           $(SRCDIR)/screen.c $(SRCDIR)/keyboard.c

# Object files
LIB_OBJS = $(LIB_SRCS:.c=.o)

# ── Targets ──────────────────────────────────────────────────────────

.PHONY: all game os test clean

all: game os

# Track A: Snake Game
game: mini_game
mini_game: $(SRCDIR)/game.c $(LIB_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# Track B: Mini OS
os: mini_os
mini_os: $(SRCDIR)/os.c $(LIB_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# ── Tests ────────────────────────────────────────────────────────────

test: test_memory test_math test_string
	@echo ""
	@echo "╔═══════════════════════════════════════════╗"
	@echo "║       Running All Test Suites             ║"
	@echo "╚═══════════════════════════════════════════╝"
	@echo ""
	@./test_memory
	@echo ""
	@./test_math
	@echo ""
	@./test_string
	@echo ""
	@echo "══════════════════════════════════════════"
	@echo "  All test suites completed."
	@echo "══════════════════════════════════════════"

test_memory: $(TESTDIR)/test_memory.c $(SRCDIR)/memory.c
	$(CC) $(CFLAGS) -o $@ $^

test_math: $(TESTDIR)/test_math.c $(SRCDIR)/math.c
	$(CC) $(CFLAGS) -o $@ $^

test_string: $(TESTDIR)/test_string.c $(SRCDIR)/string.c
	$(CC) $(CFLAGS) -o $@ $^

# ── Cleanup ──────────────────────────────────────────────────────────

clean:
	rm -f mini_game mini_os test_memory test_math test_string
	rm -f $(SRCDIR)/*.o
