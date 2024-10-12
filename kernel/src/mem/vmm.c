#include "vmm.h"
#include "pmm.h"
#include "mem.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include <boot/limine.h>

#include <util/logger.h>
#include <util/defines.h>

#include <hal/pit.h>
#include <hal/panic.h>
#include <hal/instr.h>

#define PT_POOL_SIZE 10

#define PML5E_IDX(virt)     (((uintptr_t)virt >> 48) & 0x1FFull)
#define PML4E_IDX(virt)     (((uintptr_t)virt >> 39) & 0x1FFull)
#define PDPE_IDX(virt)      (((uintptr_t)virt >> 30) & 0x1FFull)
#define PDE_IDX(virt)       (((uintptr_t)virt >> 21) & 0x1FFull)
#define PTE_IDX(virt)       (((uintptr_t)virt >> 12) & 0x1FFull)

union page_entry{
    uint64_t raw;

    struct {
        uint8_t present             : 1,
                read_write          : 1,
                user_supervisor     : 1,
                write_through       : 1,
                cache_disabled      : 1,
                accessed            : 1,
                dirty               : 1,
                large_page_or_pat   : 1;
        uint8_t addr_bits[6];
        uint8_t : 3, protection_key : 4, execute_disable : 1;
    };
};

typedef struct {
    union page_entry entries[512];
} page_table_t;

typedef struct {
    uint8_t phys_addr_size;
    uint8_t virt_addr_size;
    uint8_t guest_phys_addr_size;
} lm_size_id_t;

typedef struct {
    page_table_t *pml4_table;

    uint64_t pt_pool_next_idx;
    virt_addr_t pt_pool[PT_POOL_SIZE];

    uint8_t phys_addr_size;
    uint8_t virt_addr_size;

    bool pt_pool_ready;
} vmm_ctx_t;

static vmm_ctx_t vmm_ctx = {0};

static page_table_t *vmm_get_table_from_entry(union page_entry *entry) {
    uint64_t phys_mask = (1ull << (vmm_ctx.phys_addr_size - 12)) - 1;

    phys_addr_t phys = (phys_addr_t)((entry->raw >> 12) & phys_mask) *
        PAGE_BYTE_SIZE;

    virt_addr_t virt = mem_phys_to_virt(phys);

    return (page_table_t *)virt;
}

static page_table_t *vmm_get_new_page_table(void) {
    uint64_t idx = vmm_ctx.pt_pool_next_idx++ % PT_POOL_SIZE;

    virt_addr_t virt = vmm_ctx.pt_pool[idx];
    vmm_ctx.pt_pool[idx] = 0;

    return (page_table_t *)virt;
}

static void vmm_try_restock_pt_pool(void) {
    if (!vmm_ctx.pt_pool_ready)
        return;

    for (uint64_t i = 0; i < PT_POOL_SIZE; i++) {
        if (vmm_ctx.pt_pool[i] == 0) {
            phys_addr_t phys = pmm_alloc_mem(PAGE_BYTE_SIZE);

            if (phys == INVALID_PHYS)
                panic("unable to allocate page frame for a page table");

            virt_addr_t virt = mem_phys_to_virt(phys);

            vmm_ctx.pt_pool[i] = virt;

            vmm_map(phys, virt, PAGE_BYTE_SIZE, PAGE_MAP_WRITABLE);
            memset(virt, 0, PAGE_BYTE_SIZE);
        }
    }
}

static void vmm_set_page_entry(union page_entry *entry,
                               phys_addr_t phys, uint16_t flags) {
    entry->present = true;
    entry->read_write = flags & PAGE_MAP_WRITABLE ? true : false;
    entry->user_supervisor = flags & PAGE_MAP_USER ? true : false;

    uint64_t phys_mask = (1ull << (vmm_ctx.phys_addr_size - 12)) - 1;
    uint64_t phys_idx = phys / PAGE_BYTE_SIZE;

    if (phys_idx > phys_mask)
        panic("how did you manage this?");

    entry->raw &= ~(phys_mask << 12);       // clear existing addr if present
    entry->raw |= phys_idx << 12;           // set new address
}

static bool vmm_map_page(phys_addr_t phys, virt_addr_t virt, uint32_t flags) {
    page_table_t *pml4_table = vmm_ctx.pml4_table;
    if (!pml4_table) {
        kerror("PML4 table is NULL");
        return false;
    }

    union page_entry *pml4_entry = &pml4_table->entries[PML4E_IDX(virt)];
    page_table_t *pdp_table = 0;

    if (!pml4_entry->present) {
        pdp_table = vmm_get_new_page_table();
        vmm_set_page_entry(pml4_entry, mem_virt_to_phys((virt_addr_t)pdp_table),
                           PAGE_MAP_WRITABLE);
    } else {
        pdp_table = vmm_get_table_from_entry(pml4_entry);
    }

    union page_entry *pdp_entry = &pdp_table->entries[PDPE_IDX(virt)];
    page_table_t *pd_table = 0;

    if (!pdp_entry->present) {
        pd_table = vmm_get_new_page_table();
        vmm_set_page_entry(pdp_entry, mem_virt_to_phys((virt_addr_t)pd_table),
                           PAGE_MAP_WRITABLE);
    } else {
        pd_table = vmm_get_table_from_entry(pdp_entry);
    }

    union page_entry *pd_entry = &pd_table->entries[PDE_IDX(virt)];
    page_table_t *pt_table = 0;

    if (!pd_entry->present) {
        pt_table = vmm_get_new_page_table();
        vmm_set_page_entry(pd_entry, mem_virt_to_phys((virt_addr_t)pt_table),
                           PAGE_MAP_WRITABLE);
    } else {
        pt_table = vmm_get_table_from_entry(pd_entry);
    }

    union page_entry *pt_entry = &pt_table->entries[PTE_IDX(virt)];
    if (pt_entry->present) {
        kwarn("Page 0x%016llx is already mapped", phys);
        return false;
    } else {
        vmm_set_page_entry(pt_entry, phys, flags);
    }

    vmm_try_restock_pt_pool();

    return true;
}

bool vmm_map(phys_addr_t phys, virt_addr_t virt, size_t bytes, uint32_t flags) {
    for (uintptr_t off = 0; off < bytes; off += PAGE_BYTE_SIZE) {
        if (!vmm_map_page(phys + off, virt + off, flags))
            return false;
    }
    return true;
}

void vmm_print_memmap(void) {
    uint64_t entry_addr_mask =
    	(0xffffffffffffffff >> (64 - vmm_ctx.phys_addr_size))
        & 0xfffffffffffff000;

    page_table_t *pml4_table = vmm_ctx.pml4_table;
    kinfo("PML4 table: 0x%016llx", mem_virt_to_phys(pml4_table));

    for (uint64_t i = 0; i < 512; i++) {
        union page_entry *pml4_entry = &pml4_table->entries[i];

        if (!pml4_entry->present)
            continue;

        page_table_t *pdp_table = vmm_get_table_from_entry(pml4_entry);
        kinfo("PML4E index: %3d | PML4E: 0x%016llx | PDP table: 0x%016llx",
               i, *pml4_entry, mem_virt_to_phys(pdp_table));

        for (uint64_t j = 0; j < 512; j++) {
            union page_entry *pdp_entry = &pdp_table->entries[j];
            if (!pdp_entry->present)
                continue;

            page_table_t *pd_table = vmm_get_table_from_entry(pdp_entry);
            kinfo(" | PDPE index: %3d PDPE: 0x%016llx PD table: 0x%016llx",
	                j, *pdp_entry, mem_virt_to_phys(pd_table));

            for (uint64_t k = 0; k < 512; k++) {
                union page_entry *pd_entry = &pd_table->entries[k];

                if (!pd_entry->present)
                    continue;

                page_table_t *pt_table = vmm_get_table_from_entry(pd_entry);
                kinfo("    | PDE index: %3d PDE: "
                       "0x%016llx PT table: 0x%016llx",
                       k, *pd_entry, mem_virt_to_phys(pt_table));

                for (uint64_t l = 0; l < 512; l++) {
                    union page_entry *p_entry = &pt_table->entries[l];

                    if (!p_entry->present)
                        continue;

                    uintptr_t virt = ((i << 39) |
                                      (j << 30) |
                                      (k << 21) |
                                      (l << 12) |
                                      (0xffffffffffffffff <<
                                            vmm_ctx.virt_addr_size));
                    kinfo("      | PTE index: %3d PTE: 0x%016llx "
                           "Phys: 0x%016llx Virt:0x%016llx\n",
                           l, *p_entry, p_entry->raw & entry_addr_mask, virt);
                }
            }
        }
    }
}

static lm_size_id_t vmm_get_lmsi(void) {
    uint32_t function = 0x80000008;
    uint32_t eax;
    __asm__ volatile(
        "cpuid\n"
        : "=a"(eax)
        : "0"(function)
	);

    return (lm_size_id_t) {
        .phys_addr_size = eax & 0xFF,
        .virt_addr_size = (eax >> 8) & 0xFF,
        .guest_phys_addr_size = (eax >> 16) & 0xFF
    };

}

void vmm_init(mem_bitmap_t bitmap) {
    uint64_t begin = pit_get_seconds();

    kinfo("Initializing VMM");

    lm_size_id_t lmsi = vmm_get_lmsi();

    vmm_ctx.phys_addr_size = lmsi.phys_addr_size;
    vmm_ctx.virt_addr_size = lmsi.virt_addr_size;

    kinfo(" | supported physical address bits: %d", vmm_ctx.phys_addr_size);
    kinfo(" | supported virtual address bits: %d", vmm_ctx.virt_addr_size);
    kinfo(" | HHDM offset: 0x%016llx", boot_info.lhhdmr->offset);

    phys_addr_t pml4_table_phys = pmm_alloc_mem(1);
    if (pml4_table_phys == INVALID_PHYS)
        panic("invalid phys addr for PML4 table");

    virt_addr_t pml4_table_virt = mem_phys_to_virt(pml4_table_phys);
    memset((void *)pml4_table_virt, 0, PAGE_BYTE_SIZE);

    vmm_ctx.pml4_table = pml4_table_virt;

    kinfo("VMM: Populating page table pool");

    for (uint64_t i = 0; i < PT_POOL_SIZE; i++) {
        phys_addr_t phys = pmm_alloc_mem(1);
        virt_addr_t virt = mem_phys_to_virt(phys);

        memset((void *)virt, 0, PAGE_BYTE_SIZE);

        vmm_ctx.pt_pool[i] = virt;
    }

    kinfo(" | %d pages added to the page pool", PT_POOL_SIZE);
    kinfo("VMM: Setting up new PML4 table");
    kinfo(" | Phys: 0x%016llx Virt: 0x%016llx",
          pml4_table_phys, pml4_table_virt);
    kinfo("VMM: Populating PML4 table");

    vmm_map(mem_virt_to_phys(vmm_ctx.pt_pool[0]), vmm_ctx.pt_pool[0],
            PT_POOL_SIZE * PAGE_BYTE_SIZE, PAGE_MAP_WRITABLE);

    vmm_ctx.pt_pool_ready = true;

    vmm_map(pml4_table_phys, pml4_table_virt, PAGE_BYTE_SIZE, PAGE_MAP_WRITABLE);
    vmm_map(mem_virt_to_phys(bitmap.virt), bitmap.virt,
            bitmap.size, PAGE_MAP_WRITABLE);

    uint64_t pages_mapped = 11 + bitmap.size / PAGE_BYTE_SIZE;

    for (uintptr_t i = 0; i < boot_info.lmmr->entry_count; i++) {
        struct limine_memmap_entry *entry = boot_info.lmmr->entries[i];

        switch (entry->type) {
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            case LIMINE_MEMMAP_FRAMEBUFFER: {
                phys_addr_t phys = entry->base;
                virt_addr_t virt = mem_phys_to_virt(entry->base);

                if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
                    virt = (virt_addr_t) boot_info.kernel_virt_base;

                vmm_map(phys, virt, entry->length, PAGE_MAP_WRITABLE);
                pages_mapped += entry->length / PAGE_BYTE_SIZE;
                break;
            }
        }
    }

    kinfo(" | %d pages mapped into PML4 table", pages_mapped);
    kinfo("VMM: Transfering to new PML4 table");

    uint64_t current_cr3 = read_cr3();
    uint64_t new_cr3_addr = (current_cr3 & 0xFFFull) |
                            (pml4_table_phys & ~0xFFFull);
    kinfo(" | old CR3: 0x%016llx", current_cr3);
    write_cr3(new_cr3_addr);
    kinfo(" | new CR3: 0x%016llx", read_cr3());

    uint64_t end = pit_get_seconds();
    kinfo("Initialization complete (%llus)", end - begin);
}
