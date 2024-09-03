#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct mem_seg_hdr_t {
    bool free;
    uint64_t len;

    struct mem_seg_hdr_t *next;
    struct mem_seg_hdr_t *prev;
    struct mem_seg_hdr_t *next_free;
    struct mem_seg_hdr_t *prev_free;

} mem_seg_hdr_t;

extern mem_seg_hdr_t *first_free_mem_seg;

void heap_init(uint64_t base, uint64_t len);

void *kmalloc(size_t size);
void *kcalloc(size_t num, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);
