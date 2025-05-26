#include "mem.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

#include <boot/boot.h>
#include <boot/limine.h>
#include <util/logger.h>

#define HEAP_INITIAL_SIZE (0x1000 * 32)

// in linker.ld
extern uintptr_t kernel_start;
extern uintptr_t kernel_end;

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

void mem_print_layout() {
    size_t total = 0;
    size_t usable = 0;

    kinfo("MEM: Printing layout");

    uintptr_t kernel_size = (uintptr_t)&kernel_end - (uintptr_t)&kernel_start;
    kinfo("MEM: kernel_start=0x%016llx | kernel_end=0x%016llx | kernel_size=%llu bytes", 
          &kernel_start, &kernel_end, kernel_size);

    struct limine_memmap_entry *entry;
    for (uint64_t i = 0; i < boot_info.lmmr->entry_count; i++) {
        entry = boot_info.lmmr->entries[i];

        total += entry->length;

        if (entry->type == LIMINE_MEMMAP_USABLE)
            usable += entry->length;

        kinfo(" | 0x%016llx -> 0x%016llx size: %011llu %s",
               entry->base, entry->base + entry->length - 1,
               entry->length, mem_region_names[entry->type]);
    }
    kinfo("MEM: Total memory: %llu", total);
    kinfo("MEM: Total usable memory: %llu", usable);
}

void mem_init() {
    mem_bitmap_t bitmap = pmm_initialize();

    vmm_init(bitmap);

    uintptr_t heap_virt_addr = (uintptr_t)&kernel_end;
    kinfo("heap start=0x%016llx", mem_virt_to_phys((void *)heap_virt_addr));
    if (heap_virt_addr % PAGE_BYTE_SIZE)
        heap_virt_addr += (PAGE_BYTE_SIZE - (heap_virt_addr % PAGE_BYTE_SIZE));

    heap_init((void *)heap_virt_addr, HEAP_INITIAL_SIZE);
}

uint64_t mem_get_size() {
    uint64_t total = 0;
    struct limine_memmap_entry *entry;

    for (uint64_t i = 0; i < boot_info.lmmr->entry_count; i++) {
        entry = boot_info.lmmr->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE)
            total += entry->length;
    }
    return total;
}
