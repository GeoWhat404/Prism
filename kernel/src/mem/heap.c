#include "heap.h"

#include "pmm.h"
#include "vmm.h"

#include <string.h>
#include <hal/panic.h>
#include <util/logger.h>
#include <boot/boot.h>

typedef struct heap_blk {
    size_t length;
    uint64_t free;

    struct heap_blk *prev;
    struct heap_blk *next;
    struct heap_blk *next_free;
} heap_blk_t;

static heap_blk_t *root_blk = 0;
static heap_blk_t *first_free_blk = 0;
static uintptr_t heap_end;
static uintptr_t heap_address;
static size_t heap_size = 0;

void heap_init(void *heap_addr, size_t size) {
    kinfo("Heap: Initializing");
    kinfo(" | addr: 0x%016llx", heap_addr);
    kinfo(" | size: %u bytes", size);

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

    heap_address = (uintptr_t)heap_addr;
    heap_size += size;
    heap_end = (uintptr_t)heap_addr + size;

    boot_info.heap = true;
    kinfo("Heap: Completed Initialization");
}

static void expand_heap(size_t minimum_expansion_size) {
	size_t new_size = MAX(minimum_expansion_size, heap_size * 2) + 0x1000;

	phys_addr_t physical_address = INVALID_PHYS;
	if ((physical_address = pmm_alloc_mem(new_size)) == INVALID_PHYS) {
		panic("failed to allocate new physical memory to expand heap.");
	}

	virt_addr_t virtual_address = (virt_addr_t)(heap_address + heap_size);
	if (!vmm_map(physical_address, virtual_address, new_size, PAGE_MAP_WRITABLE)) {
		panic("failed to map new physical memory to expand heap.");
	}

	heap_blk_t *last_block = root_blk;
	while (last_block->next != NULL) {
		last_block = last_block->next;
	}

	if (last_block->free == true) {
		last_block->length += new_size;
	} else {
		panic("last heap block is not free. Time to implement this case\n");
	}
}

void heap_print(void) {
    heap_blk_t *ptr = root_blk;
    kinfo("Heap: Dumping the heap");
    do {
        kinfo(" | blk addr: 0x%016llx size: %lu bytes (%s)",
               ptr, ptr->length, ptr->free ? "Free" : "Allocated");
        kinfo("   prev: 0x%016llx next: 0x%016llx next free: 0x%016llx",
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
        expand_heap(size);

        blk = first_free_blk;
        while (blk != 0 && blk->length < size)
            blk = blk->next_free;

        if (blk == 0) {
            panic("heap out of memory");
            return 0;
        }
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

void kmalloc_null_error() {
    panic("kmalloc call failed - returned a null pointer");
}
