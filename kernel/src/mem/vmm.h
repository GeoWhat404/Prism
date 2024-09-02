#pragma once

#include <stdbool.h>
#include <util/dstructs/bitmap.h>

bitmap_t virt;

void vmm_initialize();

void *vmm_allocate(int pages);
void *vmm_allocate_contiguous(int pages);
bool vmm_free(void *ptr, int pages);
