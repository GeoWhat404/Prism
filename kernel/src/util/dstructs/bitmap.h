#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BLOCKS_PER_BYTE 8
#define BLOCK_SIZE      4096

typedef struct {
    bool intialized;

    uint8_t *bytes;
    size_t byte_size;
    size_t block_size;

    size_t last_deep_fragmented;

    size_t mem_start;
} bitmap_t;

void *bitmap_to_ptr(bitmap_t *bitmap, size_t block);
size_t bitmap_to_block(bitmap_t *bitmap, void *ptr);

size_t bitmap_get_size(size_t total_size);
int bitmap_get(bitmap_t *bitmap, size_t block);
void bitmap_set(bitmap_t *bitmap, size_t block, bool val);

void bitmap_set_blocks(bitmap_t *bitmap, size_t start, size_t size, bool val);
void bitmap_set_region(bitmap_t *bitmap, void *base_ptr,
                       size_t byte_size, int used);

size_t bitmap_find_first_free_region(bitmap_t *bitmap, size_t blocks);

void *bitmap_allocate(bitmap_t *bitmap, size_t blocks);
void bitmap_free(bitmap_t *bitmap, void *base, size_t blocks);

size_t bitmap_allocate_page_frame(bitmap_t *bitmap);
void bitmap_free_page_frame(bitmap_t *bitmap, void *addr);

