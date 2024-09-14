#include "mem.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

#include <stdio.h>

#include <boot/boot.h>
#include <util/debug.h>
#include <boot/limine.h>

#define HEAP_INITIALIZE_SIZE (0x1000 * 20)

// in linker.ld
extern char kernel_end;

static const char *mem_region_names[] = {
    [LIMINE_MEMMAP_USABLE]                  = "Usable",
    [LIMINE_MEMMAP_RESERVED]                = "Reserved",
    [LIMINE_MEMMAP_ACPI_RECLAIMABLE]        = "ACPI Reclaimable",
    [LIMINE_MEMMAP_ACPI_NVS]                = "ACPI NVS",
    [LIMINE_MEMMAP_BAD_MEMORY]              = "Bad Memory",
    [LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE]  = "Bootloader Reclaimable",
    [LIMINE_MEMMAP_KERNEL_AND_MODULES]      = "Kernel and Modules",
    [LIMINE_MEMMAP_FRAMEBUFFER]             = "Framebuffer"
};

void mem_print_layout(void) {
    size_t total = 0;

    printf("MEM: Printing layout\n");
    log_debug(MODULE_MMU, "Printing layout");

    struct limine_memmap_entry *entry;
    for (uint64_t i = 0; i < boot_info.lmmr->entry_count; i++) {
        entry = boot_info.lmmr->entries[i];

        total += entry->length;

        printf(" | 0x%016x -> 0x%016x size: %011llu %s\n",
               entry->base, entry->base + entry->length - 1,
               entry->length, mem_region_names[entry->type]);

        log_debug(MODULE_MMU, " | 0x%016x -> 0x%016x size: %011llu %s",
               entry->base, entry->base + entry->length - 1,
               entry->length, mem_region_names[entry->type]);

    }
    printf("MEM: Total memory: %llu\n\n", total);
}

void mem_init(void) {
    mem_bitmap_t bitmap = pmm_initialize();

    vmm_init(bitmap);

    uintptr_t heap_virt_addr = (uintptr_t)&kernel_end;
    if (heap_virt_addr % PAGE_BYTE_SIZE)
        heap_virt_addr += (PAGE_BYTE_SIZE - (heap_virt_addr % PAGE_BYTE_SIZE));

    heap_init((void *)heap_virt_addr, HEAP_INITIALIZE_SIZE);
}

