#include "mem.h"

#include <stdio.h>
#include <boot/boot.h>
#include <boot/limine.h>

#include <util/debug.h>

static const char *const region_type_names[] = {
    [LIMINE_MEMMAP_USABLE]                  = "Available",
    [LIMINE_MEMMAP_RESERVED]                = "Reserved",
    [LIMINE_MEMMAP_ACPI_RECLAIMABLE]        = "ACPI Reclaimable",
    [LIMINE_MEMMAP_ACPI_NVS]                = "ACPI NVS",
    [LIMINE_MEMMAP_BAD_MEMORY]              = "Bad",
    [LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE]  = "Bootloader Reclaimable",
    [LIMINE_MEMMAP_KERNEL_AND_MODULES]      = "Kernel and modules",
    [LIMINE_MEMMAP_FRAMEBUFFER]             = "Framebuffer",
};

uint64_t memory_get_total() {
    uint64_t bytes = 0;
    for (int i = 0; i < boot_info.lmmr->entry_count; i++) {
        struct limine_memmap_entry *entry = boot_info.lmmr->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE)
            bytes += entry->length;
    }
    return bytes;
}

void memory_print() {
    printf("Dumping memory regions\n");
    for (int i = 0; i < boot_info.lmmr->entry_count; i++) {
        struct limine_memmap_entry *entry = boot_info.lmmr->entries[i];

        const char *region_type = region_type_names[entry->type];
        printf(" | Region: 0x%016llx -> 0x%016llx (0x%016llx bytes): (%s)\n",
               entry->base, entry->base + entry->length,
               entry->length, region_type);
    }
    printf("Total regions: %u | Total memory (bytes): %llu\n",
            boot_info.lmmr->entry_count, memory_get_total());
}
