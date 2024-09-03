#pragma once

#include <stdint.h>

typedef struct {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t address_align;
    uint64_t entry_size;
} __attribute__((packed)) elf_section_hdr_t;

typedef struct {
    uint32_t name;
    uint8_t info;
    uint8_t other;
    uint16_t shndx;
    uint64_t value;
    uint64_t size;
} __attribute__((packed)) symtab_entry_t;

int elf_is_hdr_valid(char *hdr);
