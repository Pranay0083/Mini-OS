/*
 * memory.h — Virtual Heap Allocator Interface
 *
 * Provides dynamic memory management over a pre-allocated Virtual RAM block.
 * Uses First-Fit Free List with block splitting and forward coalescing.
 *
 * NO standard library malloc/free used for core logic.
 * Only stdlib.h's malloc() is used ONCE at startup for bulk acquisition.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>  /* size_t */

/* ── Opaque Types ─────────────────────────────────────────────────────── */

/* Block metadata header — precedes every block in the heap */
typedef struct BlockHeader {
    size_t              size;     /* Total block size INCLUDING this header  */
    int                 is_free;  /* 1 = free, 0 = allocated                */
    struct BlockHeader *next;     /* Next block in the linked list          */
} BlockHeader;

/* ── Constants ────────────────────────────────────────────────────────── */

#define VIRTUAL_RAM_SIZE   (64 * 1024)   /* 64 KB default Virtual RAM      */
#define ALIGNMENT          8             /* 8-byte alignment                */
#define HEADER_SIZE        (sizeof(BlockHeader))
#define MIN_BLOCK_SIZE     (HEADER_SIZE + ALIGNMENT)

/* Align a size up to the nearest multiple of ALIGNMENT */
#define ALIGN(size)        (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/* ── API ──────────────────────────────────────────────────────────────── */

/**
 * Initialize the virtual heap.
 * @param raw_memory  Pointer to a contiguous memory region
 * @param size        Size of the memory region in bytes
 */
void mem_init(void *raw_memory, size_t size);

/**
 * Allocate a block of at least `size` bytes from the virtual heap.
 * Returns NULL if no suitable block is found.
 * Returned pointer is guaranteed to be 8-byte aligned.
 */
void *mem_alloc(size_t size);

/**
 * Free a previously allocated block, returning it to the free pool.
 * Performs forward coalescing with adjacent free blocks.
 * Safely ignores NULL pointers and double-frees.
 */
void mem_free(void *ptr);

/**
 * Return the total number of free bytes in the heap.
 */
size_t mem_available(void);

/**
 * Print a visual dump of the heap state (used/free blocks, sizes).
 * For debugging and the `memmap` shell command.
 */
void mem_dump(void);

/**
 * Return the total number of blocks (used + free) in the heap.
 */
int mem_block_count(void);

#endif /* MEMORY_H */
