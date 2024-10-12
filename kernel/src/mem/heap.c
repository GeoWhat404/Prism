#include "heap.h"

#include "pmm.h"
#include "vmm.h"

#include <stdio.h>
#include <string.h>
#include <hal/panic.h>
#include <util/debug.h>

typedef struct heap_blk {
    size_t length;
    uint64_t free;

    struct heap_blk *prev;
    struct heap_blk *next;
    struct heap_blk *next_free;
} heap_blk_t;

static heap_blk_t *root_blk = 0;
static heap_blk_t *first_free_blk = 0;

void heap_init(void *heap_addr, size_t size) {
    printf("Heap: Initializing\n");
    printf(" | addr: 0x%016llx\n", heap_addr);
    printf(" | size: %u bytes\n", size);

    phys_addr_t phys = pmm_alloc_mem(size);
    if (phys == INVALID_PHYS)
        panic("not enough contiguous mem for the heap (required %lu)", size);

    if (!vmm_map(phys, (virt_addr_t)heap_addr, size, PAGE_MAP_WRITABLE))
        panic("failed to map vmem for the kheap");

    root_blk = (heap_blk_t *)heap_addr;
    root_blk->length = size - sizeof(heap_blk_t);
    root_blk->free = 1;
    root_blk->prev = 0;
    root_blk->next = 0;
    root_blk->next_free = 0;

    first_free_blk = root_blk;

    printf("Heap: Completed Initialization\n\n");

    log_info("Heap initilized");
}

void heap_print(void) {
    heap_blk_t *ptr = root_blk;
    printf("Heap: Dumping the heap\n");
    do {
        printf(" | blk addr: 0x%016llx size: %lu bytes (%s)\n",
               ptr, ptr->length, ptr->free ? "Free" : "Allocated");
        printf("   prev: 0x%016llx next: 0x%016llx next free: 0x%016llx\n",
               ptr->prev, ptr->next, ptr->next_free);
        ptr = ptr->next;
    } while (ptr);
}

void *kmalloc(size_t size) {
    uint64_t rem = size % 8;
    size -= rem;
    if (rem != 0)
        size += 8;

    heap_blk_t *blk = first_free_blk;
    while (blk != 0 && blk->length < size)
        blk = blk->next_free;

    if (blk == 0) {
        panic("heap out of memory");
        return 0;           // unreachable but ok
    }

    if (blk->length == size) {
        blk->free = 0;

        if (blk == first_free_blk)
            first_free_blk = blk->next_free;

        void *ptr = (void *)blk + sizeof(heap_blk_t);
        memset(ptr, 0, size);

        return ptr;
    }

    heap_blk_t *cleaved = ((void *)blk) + sizeof(heap_blk_t) + size;
    cleaved->length = blk->length - (sizeof(heap_blk_t) + size);
    cleaved->free = 1;
    cleaved->prev = blk;
    cleaved->next = blk->next;
    cleaved->next_free = blk->next_free;

    if (cleaved->next)
        cleaved->next->prev = cleaved;

    blk->length = size;
    blk->free = 0;
    blk->next = cleaved;
    blk->next_free = cleaved;

    if (blk->prev)
        blk->prev->next_free = cleaved;

    if (cleaved < first_free_blk || blk == first_free_blk)
        first_free_blk = cleaved;

    void *ptr = (void *)blk + sizeof(heap_blk_t);
    memset(ptr, 0, size);

    return ptr;
}

void kfree(void *ptr) {
    if (!ptr)
        panic("Exception in thread \"main\" java.lang.NullPointerException");

    heap_blk_t *blk = ptr - sizeof(heap_blk_t);
    blk->free = 1;

    if (blk < first_free_blk)
        first_free_blk = blk;

    if (blk->prev)
        blk->prev->next_free = blk;

    // maybe merge with next block
    heap_blk_t *next = blk->next;
    if (next && next->free) {
        if (next->next)
            next->next->prev = blk;

        blk->length = blk->length + sizeof(heap_blk_t) + next->length;
        blk->next = next->next;
        blk->next_free = next->next_free;

        if (blk < first_free_blk)
            first_free_blk = blk;
    }

    // maybe merge with prev block
    heap_blk_t *prev = blk->prev;
    if (prev && prev->free) {
        prev->length = prev->length + sizeof(heap_blk_t) + blk->length;
        prev->free = 1;
        prev->next = blk->next;
        prev->next_free = blk->next_free;

        if (blk->next)
            blk->next->prev = prev;

        if (prev < first_free_blk)
            first_free_blk = prev;
    }
}
