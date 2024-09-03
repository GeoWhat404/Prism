#pragma once

#include <boot/limine.h>
#include <util/dstructs/bitmap.h>

bitmap_t phys;

void pmm_initialize();
struct limine_memmap_entry *pmm_get_first_free_region();
