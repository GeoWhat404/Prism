#include "mem.h"

#include <stdio.h>
#include <boot/boot.h>
#include <boot/limine.h>

#include <hal/panic.h>
#include <util/debug.h>

#define KIB ((uint64_t) 1024)
#define MIB (1024 * KIB)
#define GIB (1024 * MIB)
#define TIB (1024 * GIB)

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

const char *data_size_names[] = {"Bytes", "KiB", "MiB", "GiB", "TiB"};

void print_formated_memory(uint64_t bytes) {
    uint64_t size;
    uint64_t factor = (uint64_t) TIB;
    int index = 4;

    while (index >= 0) {
        size = bytes / factor;
        if (size > 1) {
            printf("%llu %s\n", size, data_size_names[index]);
            return;
        }
        factor /= 1024;
        index--;
    }

    printf("%llu%s\n", bytes, data_size_names[0]);
}

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
        printf(" | Region: 0x%016llx -> 0x%016llx (%016llu bytes): %s\n",
               entry->base, entry->base + entry->length,
               entry->length, region_type);
    }
    printf("Total regions: %u | Total memory: ", boot_info.lmmr->entry_count);
    print_formated_memory(memory_get_total());
}
