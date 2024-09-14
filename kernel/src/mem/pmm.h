#pragma once

#include "mem.h"
#include <stdbool.h>
#include <boot/limine.h>

#define INVALID_PHYS    (1ull << 52)

mem_bitmap_t pmm_initialize();

bool pmm_reserve_mem(const phys_addr_t phys, const size_t bytes);
bool pmm_free_mem(const phys_addr_t phys, const size_t bytes);

phys_addr_t pmm_alloc_mem(const size_t bytes);
bool pmm_dealloc_mem(const phys_addr_t phys, const size_t bytes);
