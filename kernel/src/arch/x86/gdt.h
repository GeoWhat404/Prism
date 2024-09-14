#pragma once

#include <stdint.h>
#include <util/defines.h>

#define GDT_KERNEL_CODE 0x28
#define GDT_KERNEL_DATA 0x38
#define GDT_USER_CODE   0x50
#define GDT_USER_DATA   0x48
#define GDT_TSS         0x50

typedef struct gdt_entry_t {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} _packed gdt_entry_t;

typedef struct tss_entry_t {
	uint16_t length;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t flags1;
	uint8_t flags2;
	uint8_t base_high;
	uint32_t base_upper32;
	uint32_t reserved;
} _packed tss_entry_t;

typedef struct tss_ptr_t {
	uint32_t unused0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t unused1;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t unused2;
	uint32_t iopb;
} _packed tss_ptr_t;

typedef struct gdt_entries_t {
	gdt_entry_t descriptors[11];
	tss_entry_t tss;
} _packed gdt_entries_t;

typedef struct gdt_ptr_t {
	uint16_t limit;
	uint64_t base;
} _packed gdt_ptr_t;

void gdt_initialize(void);

