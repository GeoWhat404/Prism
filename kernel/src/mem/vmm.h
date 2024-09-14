#pragma once

#include "pmm.h"

enum {
    PAGE_MAP_WRITABLE = 1,
    PAGE_MAP_USER = 1 << 1,
};

void vmm_init(mem_bitmap_t bitmap);
void vmm_print_memmap();

bool vmm_map(phys_addr_t phys, virt_addr_t virt, size_t bytes, uint32_t flags);
