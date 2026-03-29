/*
 * test_memory.c — Unit tests for the Virtual Heap Allocator
 *
 * Tests: basic alloc/free, splitting, coalescing, stress test,
 *        edge cases (NULL, double-free, zero-size, OOM).
 */

#include "../include/memory.h"
#include <stdio.h>
#include <stdlib.h>

/* ── Test Helpers ─────────────────────────────────────────────────────── */

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg)                                           \
    do {                                                            \
        if (cond) {                                                 \
            tests_passed++;                                         \
            printf("  [PASS] %s\n", msg);                          \
        } else {                                                    \
            tests_failed++;                                         \
            printf("  [FAIL] %s  (line %d)\n", msg, __LINE__);    \
        }                                                           \
    } while (0)

/* Fresh heap for each test group */
static char raw_ram[VIRTUAL_RAM_SIZE];

static void reset_heap(void)
{
    /* Zero out the raw RAM to start clean */
    for (int i = 0; i < VIRTUAL_RAM_SIZE; i++) {
        raw_ram[i] = 0;
    }
    mem_init(raw_ram, VIRTUAL_RAM_SIZE);
}

/* ── Test 1: Basic Allocation ─────────────────────────────────────────── */

static void test_basic_alloc(void)
{
    printf("\n--- Test 1: Basic Allocation ---\n");
    reset_heap();

    void *p1 = mem_alloc(100);
    ASSERT(p1 != NULL, "Allocate 100 bytes returns non-NULL");

    /* Check alignment */
    ASSERT(((size_t)p1 & 7) == 0, "Returned pointer is 8-byte aligned");

    void *p2 = mem_alloc(200);
    ASSERT(p2 != NULL, "Allocate 200 bytes returns non-NULL");
    ASSERT(p2 != p1, "Second allocation returns different pointer");

    void *p3 = mem_alloc(50);
    ASSERT(p3 != NULL, "Allocate 50 bytes returns non-NULL");
}

/* ── Test 2: Free and Reuse ───────────────────────────────────────────── */

static void test_free_reuse(void)
{
    printf("\n--- Test 2: Free and Reuse ---\n");
    reset_heap();

    void *p1 = mem_alloc(100);
    ASSERT(p1 != NULL, "Allocate 100 bytes");

    size_t free_before = mem_available();
    mem_free(p1);
    size_t free_after = mem_available();

    ASSERT(free_after > free_before, "Free increases available memory");

    /* Should be able to reuse the freed block */
    void *p2 = mem_alloc(100);
    ASSERT(p2 != NULL, "Re-allocate 100 bytes after free");
}

/* ── Test 3: Block Splitting ──────────────────────────────────────────── */

static void test_splitting(void)
{
    printf("\n--- Test 3: Block Splitting ---\n");
    reset_heap();

    int blocks_before = mem_block_count();
    ASSERT(blocks_before == 1, "Initially 1 block");

    void *p1 = mem_alloc(100);
    (void)p1;
    int blocks_after = mem_block_count();
    ASSERT(blocks_after == 2, "After alloc: 2 blocks (used + remaining free)");

    void *p2 = mem_alloc(200);
    (void)p2;
    int blocks_after2 = mem_block_count();
    ASSERT(blocks_after2 == 3, "After 2nd alloc: 3 blocks");
}

/* ── Test 4: Coalescing ───────────────────────────────────────────────── */

static void test_coalescing(void)
{
    printf("\n--- Test 4: Coalescing ---\n");
    reset_heap();

    void *p1 = mem_alloc(100);
    void *p2 = mem_alloc(200);
    void *p3 = mem_alloc(300);

    int blocks_allocated = mem_block_count();
    printf("    Blocks after 3 allocs: %d\n", blocks_allocated);

    /* Free middle block */
    mem_free(p2);
    /* Free first block — should coalesce with p2 */
    mem_free(p1);

    int blocks_after_coalesce = mem_block_count();
    printf("    Blocks after freeing p1 & p2 (coalesced): %d\n",
           blocks_after_coalesce);

    ASSERT(blocks_after_coalesce < blocks_allocated,
           "Coalescing reduces block count");

    /* Free last allocated block — should coalesce with the big free block */
    mem_free(p3);
    int blocks_final = mem_block_count();
    ASSERT(blocks_final == 1,
           "After freeing all: 1 coalesced free block");
}

/* ── Test 5: Edge Cases ───────────────────────────────────────────────── */

static void test_edge_cases(void)
{
    printf("\n--- Test 5: Edge Cases ---\n");
    reset_heap();

    /* Zero-size allocation */
    void *p0 = mem_alloc(0);
    ASSERT(p0 == NULL, "Allocate 0 bytes returns NULL");

    /* NULL free */
    mem_free(NULL);  /* Should not crash */
    ASSERT(1, "Free(NULL) does not crash");

    /* Double free */
    void *p1 = mem_alloc(64);
    mem_free(p1);
    mem_free(p1);  /* Should be safely ignored */
    ASSERT(1, "Double free is safely ignored");

    /* Out-of-memory */
    void *huge = mem_alloc(VIRTUAL_RAM_SIZE * 2);
    ASSERT(huge == NULL, "Over-size allocation returns NULL");
}

/* ── Test 6: Stress Test ──────────────────────────────────────────────── */

static void test_stress(void)
{
    printf("\n--- Test 6: Stress Test (1000 alloc/free cycles) ---\n");
    reset_heap();

    size_t initial_free = mem_available();
    void *ptrs[100];
    int success = 1;

    for (int cycle = 0; cycle < 10; cycle++) {
        /* Allocate 100 blocks */
        for (int i = 0; i < 100; i++) {
            ptrs[i] = mem_alloc(32 + (i % 5) * 16);
            if (!ptrs[i]) {
                printf("    OOM at cycle %d, alloc %d\n", cycle, i);
                success = 0;
                break;
            }
        }
        if (!success) break;

        /* Free all blocks */
        for (int i = 0; i < 100; i++) {
            mem_free(ptrs[i]);
        }
    }

    size_t final_free = mem_available();
    ASSERT(success, "1000 alloc/free cycles complete without OOM");
    ASSERT(final_free == initial_free,
           "All memory recovered after stress test (no leaks)");
    ASSERT(mem_block_count() == 1,
           "Heap fully coalesced to 1 block after stress test");
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void)
{
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║      memory.c — Unit Test Suite           ║\n");
    printf("╚═══════════════════════════════════════════╝\n");

    test_basic_alloc();
    test_free_reuse();
    test_splitting();
    test_coalescing();
    test_edge_cases();
    test_stress();

    printf("\n════════════════════════════════════════════\n");
    printf("Results: %d passed, %d failed, %d total\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    printf("════════════════════════════════════════════\n");

    return tests_failed > 0 ? 1 : 0;
}
