#include "pmm.h"

#include <string.h>

#include <mem/mem.h>
#include <boot/boot.h>

#include <hal/panic.h>

#include <util/debug.h>

#define round_up(a, b) ((a + b - 1) / b)

void pmm_initialize() {
    bitmap_t *bitmap = &phys;

    bitmap->intialized = false;

    phys.block_size = round_up(memory_get_total(), BLOCK_SIZE);
    phys.byte_size = round_up(phys.block_size, BLOCKS_PER_BYTE);

    struct limine_memmap_entry *mm = 0;

    for (int i = 0; i < boot_info.lmmr->entry_count; i++) {
        struct limine_memmap_entry *entry = boot_info.lmmr->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE ||
            entry->length < phys.byte_size)
            continue;
        mm = entry;
        break;
    }

    if (!mm) {
        log_warn(MODULE_PMM, "Out of memory (needed 0x%lx bytes)",
                 phys.byte_size);
        printf("Out of memory (needed 0x%lx bytes)\n",
               phys.byte_size);
        panic();
        return;
    }

    size_t bitmap_start_phys = mm->base;
    phys.bytes = (uint8_t *)(bitmap_start_phys + boot_info.lhhdmr->offset);

    if (!phys.bytes)
        log_warn(MODULE_PMM, "Ohhh");

    memset(phys.bytes, 0xFF, phys.byte_size);

    for (int i = 0; i < boot_info.lmmr->entry_count; i++) {
        struct limine_memmap_entry *entry = boot_info.lmmr->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE)
            bitmap_set_region(bitmap, (void *)entry->base, entry->length, 0);
        else
            bitmap_set_region(bitmap, (void *)entry->base, entry->length, 1);
    }
    bitmap_set_region(bitmap, (void *)bitmap_start_phys, phys.byte_size, 1);

    log_info(MODULE_PMM,
    		 "Bitmap allocated started at phys: 0x%lx, size: 0x%lx",
    		 bitmap_start_phys,
    		 phys.byte_size);

    bitmap->intialized = true;
}
