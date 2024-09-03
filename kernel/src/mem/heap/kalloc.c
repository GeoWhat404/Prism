#include "heap.h"

#include <string.h>

#include <hal/panic.h>
#include <util/debug.h>

#define HDR_SIZE (sizeof(mem_seg_hdr_t))

void *kmalloc(size_t size) {
    uint64_t rem = size % 8;
    size -= rem;

    if (rem != 0)
        size += 8;

    mem_seg_hdr_t *current = first_free_mem_seg;
    if (!current)
        log_debug(MODULE_HEAP, "Hello!");

    while (1) {
        if (current->len >= size) {
            if (current->len > size + HDR_SIZE) {
                mem_seg_hdr_t *new_seg = (mem_seg_hdr_t *)
                    ((uint64_t)current + HDR_SIZE + size);

                new_seg->free = true;
                new_seg->len = ((uint64_t)current->len) - (HDR_SIZE + size);
                new_seg->next = current->next;
                new_seg->next_free = current->next_free;
                new_seg->prev = current;
                new_seg->prev_free = current->prev_free;

                current->len = size;
                current->next = new_seg;
                current->next_free = new_seg;
            }

            if (current == first_free_mem_seg)
                first_free_mem_seg = current->next_free;

            current->free = false;

            if (current->prev_free != 0)
                current->prev_free->next_free = current->next_free;

            if (current->next_free != 0)
                current->next_free->prev_free = current->prev_free;

            if (current->prev != 0)
                current->prev->next_free = current->next_free;

            if (current->next != 0)
                current->next->prev_free = current->prev_free;

            return current + 1;
        }

        if (current->next_free == 0) {
            panic("heap: out of memory");
            return 0;
        }
        current = current->next_free;
    }

    panic("heap: no free mem segment found");

    // unreachable
    return 0;
}

void *calloc(size_t num, size_t size) {
    void *ptr = kmalloc(size * num);

    memset(ptr, 0, size * num);

    return ptr;
}

void *krealloc(void *ptr, size_t size) {
    void *new_ptr = kmalloc(size);
    memcpy(new_ptr, ptr, size);
    kfree(ptr);

    return new_ptr;
}

static void merge_free_segments(mem_seg_hdr_t *a, mem_seg_hdr_t *b);

void kfree(void *ptr) {
    if (ptr == 0)
        panic("heap: cannot kfree null memory");

    mem_seg_hdr_t *current = ((mem_seg_hdr_t *)ptr) - 1;
    current->free = true;

    if (current < first_free_mem_seg)
        first_free_mem_seg = current;

    if (current->next_free != 0) {
        if (current->next_free->prev_free < current)
            current->next_free->prev_free = current;
    }

    if (current->prev_free != 0) {
        if (current->prev_free->next_free > current)
            current->prev_free->next_free = current;
    }

    if (current->next != 0) {
        current->next->prev = current;

        if (current->next->free)
            merge_free_segments(current, current->next);
    }

    if (current->prev != 0) {
        current->prev->next = current;
        if (current->prev->free)
            merge_free_segments(current, current->prev);
    }
}

static void merge_free_segments(mem_seg_hdr_t *a, mem_seg_hdr_t *b) {
    if (a == 0 || b == 0 || a == b)
        return;

    mem_seg_hdr_t *smallest = (a < b) ? a : b;
    mem_seg_hdr_t *largest = (a > b) ? a : b;

    smallest->len += largest->len + HDR_SIZE;
    smallest->next = largest->next;
    smallest->next_free = largest->next_free;
    largest->next->prev = smallest;
    largest->prev_free = smallest;
    largest->next_free->prev_free = smallest;
}
