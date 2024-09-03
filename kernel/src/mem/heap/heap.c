#include "heap.h"

#include <stdio.h>

#include <mem/paging.h>
#include <util/debug.h>

mem_seg_hdr_t *first_free_mem_seg;

void heap_init(uint64_t base, uint64_t len) {
    log_debug(MODULE_HEAP, "Initializing the heap");
    printf("Initializing the heap\n");
    printf(" | from 0x%016llx -> 0x%016llx\n", base, base + len);
    printf(" | with size 0x%016llx (%llu)\n", len, len);

    first_free_mem_seg = (mem_seg_hdr_t *)base;
    first_free_mem_seg->len = len - sizeof(mem_seg_hdr_t);
    first_free_mem_seg->free = true;
    first_free_mem_seg->next = 0;
    first_free_mem_seg->prev = 0;
    first_free_mem_seg->next_free = 0;
}
