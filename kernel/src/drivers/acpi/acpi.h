#pragma once

#include <stdint.h>

struct RSDP {
    char signature[8];
    uint8_t checksum;
    char OEM_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
} __attribute__((packed));

struct ISDT_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEM_id[6];
    char OEM_table_id[8];
    uint32_t OEM_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct RSDT {
    struct ISDT_header header;
    uint32_t entries[0];
} __attribute__((packed));

void acpi_init();
void *acpi_find_MADT(struct RSDT *root);
