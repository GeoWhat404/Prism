#include "pmm.h"

#include <string.h>

#include <mem/mem.h>
#include <boot/boot.h>

#include <hal/pit.h>
#include <hal/panic.h>

#include <util/debug.h>

#define PAGES_PER_BYTE      8ull
#define PAGES_PER_BITMAP    64ull

typedef struct {
    size_t bitmap_size;
    uint64_t *bitmap;
    uint64_t used_pages;
    uint64_t total_pages;
} phys_mem_ctx_t;

static phys_mem_ctx_t pmm_ctx = {0};

// aligns the size
static size_t page_align_size(size_t bytes) {
    if (bytes % PAGE_BYTE_SIZE)
        return bytes + (PAGE_BYTE_SIZE - (bytes % PAGE_BYTE_SIZE));
    return bytes;
}

static size_t pmm_bitmap_required_size(size_t total_memory) {
    size_t total_pages = total_memory / PAGE_BYTE_SIZE;
    return page_align_size(total_pages / PAGES_PER_BYTE);
}

static size_t pmm_bytes_to_pages(size_t bytes) {
    return (bytes / PAGE_BYTE_SIZE) + (bytes % PAGE_BYTE_SIZE ? 1 : 0);
}

static bool pmm_is_page_used(phys_mem_ctx_t *ctx, uintptr_t page_idx) {
    return ctx->bitmap[page_idx / PAGES_PER_BITMAP] &
    	   (1ull << (page_idx % PAGES_PER_BITMAP));
}

static uint64_t pmm_addr_to_page_idx(phys_mem_ctx_t *ctx, phys_addr_t addr) {
    if (addr % PAGE_BYTE_SIZE) {
        log_error("Cannot convert non-aligned address to page index (%llx)",
    		  addr);
        printf(
            "Error: cannot convert non-aligned address to page index (%llx)\n",
            addr);
        return INVALID_PHYS;
    }

    int page_idx = addr / PAGE_BYTE_SIZE;

    if (page_idx >= (int)ctx->total_pages)
        return INVALID_PHYS;

    return page_idx;
}

static void pmm_reserve_page(phys_mem_ctx_t *ctx, uintptr_t page_idx) {
    ctx->bitmap[page_idx / PAGES_PER_BITMAP] |=
        (1ull << (page_idx % PAGES_PER_BITMAP));

    ctx->used_pages++;
}

static void pmm_free_page(phys_mem_ctx_t *ctx, uintptr_t page_idx) {
    ctx->bitmap[page_idx / PAGES_PER_BITMAP] &=
        ~(1ull << (page_idx % PAGES_PER_BITMAP));

    // TODO: handle overflow?
    ctx->used_pages--;
}

// determines if a set of pages can be allocated at a given index contiguously
static bool pmm_can_store_pages(uintptr_t page_idx,
                                size_t pages,
                                size_t total_pages) {
    for (uintptr_t i = page_idx; i < page_idx + pages; i++) {
        if (i >= total_pages || pmm_is_page_used(&pmm_ctx, page_idx))
            return false;
    }
    return true;
}

// find a set of pages
static uintptr_t pmm_find_pages(size_t needed, uintptr_t start, uintptr_t end) {
    for (uintptr_t i = start; i < end; i++) {
        if (pmm_can_store_pages(i, needed, end))
            return i;
    }

    log_error("OOM: Cannot find %d contiguous free pages", needed);
    printf("Error: OOM: Cannot find %d contiguous free pages\n", needed);

    return INVALID_PHYS;
}

size_t pmm_total_memory(void) {
    size_t total = 0;

    for (uint64_t i = 0; i < boot_info.lmmr->entry_count; i++)
        total += boot_info.lmmr->entries[i]->length;
    return total;
}

bool pmm_free_mem(const phys_addr_t phys, const size_t bytes) {
    uintptr_t page_idx = pmm_addr_to_page_idx(&pmm_ctx, phys);

    if (page_idx == INVALID_PHYS) {
        log_warn("Tried to free invalid phys (0x%llx)", phys);
        return false;
    }

    size_t page_count = pmm_bytes_to_pages(bytes);
    for (uintptr_t i = page_idx; i < page_idx + page_count; i++) {
        if (!pmm_is_page_used(&pmm_ctx, i)) {
            log_warn("Cannot free a free page (%llu)", i);
            return false;
        }

        pmm_free_page(&pmm_ctx, i);
    }
    return true;
}

bool pmm_reserve_mem(const phys_addr_t phys, const size_t bytes) {
    uintptr_t page_idx = pmm_addr_to_page_idx(&pmm_ctx, phys);
    if (page_idx == INVALID_PHYS) {
        log_warn("Tried to alloc invalid phys (0x%llx)", phys);
        return false;
    }

    size_t page_count = pmm_bytes_to_pages(bytes);
    for (uintptr_t i = page_idx; i < page_idx + page_count; i++) {
        if (pmm_is_page_used(&pmm_ctx, i)) {
            log_warn("Cannot reserve a reserved page (%d)", i);
            return false;
        }
        pmm_reserve_page(&pmm_ctx, i);
    }
    return true;
}

// returns the address to the phys addr or INVALID_PHYS if failed
phys_addr_t pmm_alloc_mem(const size_t bytes) {
    size_t needed = pmm_bytes_to_pages(bytes);
    uintptr_t page_idx = pmm_find_pages(needed, 1,  pmm_ctx.total_pages);

    if (page_idx == INVALID_PHYS) {
        return INVALID_PHYS;
    }

    phys_addr_t addr = page_idx * PAGE_BYTE_SIZE;
    if (!pmm_reserve_mem(addr, bytes))
        return INVALID_PHYS;

    return addr;
}

bool pmm_dealloc_mme(const phys_addr_t phys, const size_t bytes) {
    return pmm_free_mem(phys, bytes);
}

mem_bitmap_t pmm_initialize(void) {

    uint64_t begin = pit_get_seconds();

    printf("PMM: Initializing\n");
    log_info("Initializing PMM");

    size_t total_memory = pmm_total_memory();
    size_t bitmap_size = pmm_bitmap_required_size(total_memory);

    printf(" | total memory: %d bytes\n", total_memory);
    printf(" | total pages: %d\n", total_memory / PAGE_BYTE_SIZE);

    struct limine_memmap_entry *mm_entry = 0;

    for (uint64_t i = 0; i < boot_info.lmmr->entry_count; i++) {
        struct limine_memmap_entry *entry = boot_info.lmmr->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE || entry->length < bitmap_size)
            continue;

        if (mm_entry == 0 || entry->length < mm_entry->length)
            mm_entry = entry;
    }

    if (mm_entry == 0)
        panic("could not find a mem region for the paging bitmap");

    pmm_ctx.total_pages = total_memory / PAGE_BYTE_SIZE;
    pmm_ctx.used_pages = total_memory / PAGE_BYTE_SIZE;
    pmm_ctx.bitmap_size = bitmap_size;
    pmm_ctx.bitmap = (uint64_t *)(mm_entry->base + boot_info.lhhdmr->offset);

    printf("PMM: Setting up mem bitmap\n");
    printf(" | bitmap addr: 0x%016llx\n", pmm_ctx.bitmap);
    printf(" | bitmap size: %d bytes\n", pmm_ctx.bitmap_size);

    memset(pmm_ctx.bitmap, 0xFF, pmm_ctx.bitmap_size);

    log_info("Releasing usable memory regions");
    printf("PMM: Releasing usable memory regions\n");

    for (uint64_t i = 0; i < boot_info.lmmr->entry_count; i++) {
        struct limine_memmap_entry *entry = boot_info.lmmr->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE)
            continue;

        if (!pmm_free_mem(entry->base, entry->length)) {
            panic("failed to release mem region: 0x%llx->0x%llx",
                  entry->base, entry->base + entry->length - 1);
        }
    }

    if (!pmm_reserve_mem(mm_entry->base, bitmap_size))
        panic("failed to reserve mem region: 0x%016llx->0x%016llx",
               mm_entry->base, mm_entry->base + mm_entry->length - 1);

    printf(" | usable free mem: %d bytes\n",
           (pmm_ctx.total_pages - pmm_ctx.used_pages) * PAGE_BYTE_SIZE);

    mem_bitmap_t bitmap = {0};
    bitmap.virt = pmm_ctx.bitmap;
    bitmap.size = pmm_ctx.bitmap_size;

    uint64_t end = pit_get_seconds();

    log_info("Initialization complete (%llus)", end - begin);
    printf("PMM: Completed Initialization in %llus\n\n", end - begin);

    return bitmap;
}
