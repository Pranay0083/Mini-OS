/*
 * memory.c — Virtual Heap Allocator Implementation
 *
 * First-Fit Free List allocator with:
 *   - 8-byte alignment for all allocations
 *   - Block splitting when remainder >= MIN_BLOCK_SIZE
 *   - Forward coalescing on free to prevent fragmentation
 *   - Safety: NULL checks, double-free protection, bounds validation
 *
 * Permitted dependency: <stdio.h> for mem_dump() debug output only
 */

#include "../include/memory.h"
#include <stdio.h>  /* printf — for mem_dump() debug output only */

/* ── Module State ─────────────────────────────────────────────────────── */

static BlockHeader *heap_start = NULL;  /* Head of the block linked list */
static void        *heap_base  = NULL;  /* Raw base pointer for bounds check */
static size_t       heap_size  = 0;     /* Total heap size */

/* ── Initialization ───────────────────────────────────────────────────── */

void mem_init(void *raw_memory, size_t size)
{
    if (!raw_memory || size < MIN_BLOCK_SIZE) {
        return;
    }

    heap_base  = raw_memory;
    heap_size  = size;

    /* Create a single free block spanning the entire region */
    heap_start       = (BlockHeader *)raw_memory;
    heap_start->size    = size;
    heap_start->is_free = 1;
    heap_start->next    = NULL;
}

/* ── Allocation (First-Fit with Splitting) ────────────────────────────── */

void *mem_alloc(size_t size)
{
    if (size == 0 || !heap_start) {
        return NULL;
    }

    /* Align the requested size */
    size_t aligned_size = ALIGN(size);
    size_t total_needed = HEADER_SIZE + aligned_size;

    BlockHeader *current = heap_start;

    while (current != NULL) {
        if (current->is_free && current->size >= total_needed) {

            /* ── Split if the remainder can hold another block ──────── */
            size_t remaining = current->size - total_needed;

            if (remaining >= MIN_BLOCK_SIZE) {
                /* Create a new free block after the allocated portion */
                BlockHeader *new_block = (BlockHeader *)
                    ((char *)current + total_needed);

                new_block->size    = remaining;
                new_block->is_free = 1;
                new_block->next    = current->next;

                current->size = total_needed;
                current->next = new_block;
            }

            current->is_free = 0;

            /* Return pointer past the header (to the data region) */
            return (void *)((char *)current + HEADER_SIZE);
        }

        current = current->next;
    }

    /* No suitable block found */
    return NULL;
}

/* ── Deallocation (with Forward Coalescing) ───────────────────────────── */

void mem_free(void *ptr)
{
    if (!ptr || !heap_start) {
        return;
    }

    /* Compute the header from the user pointer */
    BlockHeader *block = (BlockHeader *)((char *)ptr - HEADER_SIZE);

    /* Bounds check: ensure pointer is within the heap */
    if ((char *)block < (char *)heap_base ||
        (char *)block >= (char *)heap_base + heap_size) {
        return;  /* Pointer is outside our heap — ignore */
    }

    /* Double-free protection */
    if (block->is_free) {
        return;
    }

    block->is_free = 1;

    /* ── Forward Coalescing ───────────────────────────────────────── */
    /* Scan the entire list and merge any adjacent free blocks       */
    BlockHeader *current = heap_start;

    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            /* Merge current with next */
            current->size += current->next->size;
            current->next  = current->next->next;
            /* Don't advance — check if we can merge again */
        } else {
            current = current->next;
        }
    }
}

/* ── Query: Available Free Memory ─────────────────────────────────────── */

size_t mem_available(void)
{
    size_t total_free = 0;
    BlockHeader *current = heap_start;

    while (current != NULL) {
        if (current->is_free) {
            /* Subtract header size to report usable free space */
            if (current->size > HEADER_SIZE) {
                total_free += current->size - HEADER_SIZE;
            }
        }
        current = current->next;
    }

    return total_free;
}

/* ── Debug: Heap Block Count ──────────────────────────────────────────── */

int mem_block_count(void)
{
    int count = 0;
    BlockHeader *current = heap_start;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

/* ── Debug: Heap Dump ─────────────────────────────────────────────────── */

void mem_dump(void)
{
    printf("\n=== HEAP MEMORY MAP ===\n");
    printf("Heap base: %p | Total size: %zu bytes\n\n", heap_base, heap_size);

    BlockHeader *current = heap_start;
    int block_num = 0;
    size_t used_total = 0;
    size_t free_total = 0;

    while (current != NULL) {
        size_t data_size = (current->size > HEADER_SIZE)
                           ? current->size - HEADER_SIZE : 0;

        printf("  Block %2d | %s | size: %5zu B (data: %5zu B) | addr: %p\n",
               block_num,
               current->is_free ? "FREE" : "USED",
               current->size,
               data_size,
               (void *)current);

        if (current->is_free) {
            free_total += data_size;
        } else {
            used_total += data_size;
        }

        block_num++;
        current = current->next;
    }

    printf("\n  ─────────────────────────────────────────────────\n");
    printf("  Blocks: %d | Used: %zu B | Free: %zu B\n",
           block_num, used_total, free_total);
    printf("===========================\n\n");
}
