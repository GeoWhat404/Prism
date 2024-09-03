#include "bitmap.h"

#include <util/math.h>
#include <util/debug.h>

void *bitmap_to_ptr(bitmap_t *bitmap, size_t block) {
    return (void *)((uint8_t *)(bitmap->mem_start + (block * BLOCK_SIZE)));
}

size_t bitmap_to_block(bitmap_t *bitmap, void *ptr) {
    uint8_t *u8_ptr = (uint8_t *)ptr;
    return (size_t)(u8_ptr - bitmap->mem_start) / BLOCK_SIZE;
}

size_t bitmap_to_block_ceil(bitmap_t *bitmap, void *ptr) {
    uint8_t *u8_ptr = (uint8_t *)ptr;
    return (size_t)CEIL((size_t)(u8_ptr - bitmap->mem_start), BLOCK_SIZE);
}

size_t bitmap_get_size(size_t total_size) {
    size_t blocks = CEIL(total_size, BLOCK_SIZE);
    size_t bytes = CEIL(blocks, 8);
    return bytes;
}

int bitmap_get(bitmap_t *bitmap, size_t block) {
    size_t addr = block / BLOCKS_PER_BYTE;
    size_t off = block % BLOCKS_PER_BYTE;
    return (bitmap->bytes[addr] & (1 << off)) != 0;
}

void bitmap_set(bitmap_t *bitmap, size_t block, bool val) {
    size_t addr = block / BLOCKS_PER_BYTE;
    size_t off = block % BLOCKS_PER_BYTE;

    if (val)
        bitmap->bytes[addr] |= (1 << off);
    else
        bitmap->bytes[addr] &= ~(1 << off);
}

void bitmap_set_blocks(bitmap_t *bitmap, size_t start, size_t size, bool val) {
    if (!val && start < bitmap->last_deep_fragmented)
        bitmap->last_deep_fragmented = start;

    for (int i = start; i < start + size; i++)
        bitmap_set(bitmap, i, val);
}

int bitmap_get_all_free_blocks(bitmap_t *bitmap, size_t start, size_t size) {
    int flag = 1;

    if (start < bitmap->last_deep_fragmented)
        bitmap->last_deep_fragmented = start;

    for (int i = start; i < start + size; i++)
        flag &= bitmap_get(bitmap, i);
    return flag;
}

void bitmap_set_region(bitmap_t *bitmap, void *base_ptr,
                       size_t byte_size, int used) {
    size_t base;
    size_t size;

    if (used) {
        base = bitmap_to_block(bitmap, base_ptr);
        size = byte_size / BLOCK_SIZE;
    } else {
        base = bitmap_to_block_ceil(bitmap, base_ptr);
        size = byte_size / BLOCK_SIZE;
    }
    bitmap_set_blocks(bitmap, base, size, used);
}

int bitmap_get_region(bitmap_t *bitmap, void *base_ptr, size_t byte_size) {
    size_t base;
    size_t size;

    base = bitmap_to_block_ceil(bitmap, base_ptr);
    size = byte_size / BLOCK_SIZE;
    return bitmap_get_all_free_blocks(bitmap, base, size);
}

size_t bitmap_find_first_free_region(bitmap_t *bitmap, size_t blocks) {
    size_t current_region_start = bitmap->last_deep_fragmented;
    size_t current_region_size = 0;

    for (size_t i = 0; i < bitmap->block_size; i++) {
        if (bitmap_get(bitmap, i)) {
            current_region_size = 0;
            current_region_start = i + 1;
        } else {
            if (blocks == 1)
                bitmap->last_deep_fragmented = current_region_start + 1;
            current_region_size++;
            if (current_region_size >= blocks)
                return current_region_start;
        }
    }
    return -1;
}

void *bitmap_allocate(bitmap_t *bitmap, size_t blocks) {
    // why?? just why??
    if (blocks == 0)
        return 0;

    size_t region = bitmap_find_first_free_region(bitmap, blocks);
    if (region == -1) {
        log_error(MODULE_PMM, "Failed to find a free region (OOM)");
        return 0;
    }
    bitmap_set_blocks(bitmap, region, blocks, 1);
    return bitmap_to_ptr(bitmap, region);
}

void bitmap_free(bitmap_t *bitmap, void *base, size_t blocks) {
    bitmap_set_region(bitmap, base, blocks * BLOCK_SIZE, 0);
}

size_t bitmap_allocate_page_frame(bitmap_t *bitmap) {
    size_t region = bitmap_find_first_free_region(bitmap, 1);
    bitmap_set_blocks(bitmap, region, 1, 1);
    return bitmap->mem_start + (region * BLOCK_SIZE);
}

void bitmap_free_page_frame(bitmap_t *bitmap, void *addr) {
    bitmap_set_region(bitmap, addr, BLOCK_SIZE, 0);
}

