#include "panic.h"

#include "fb.h"

#include <stdio.h>
#include <string.h>

#include <boot/boot.h>

#include <util/elf.h>
#include <util/debug.h>

extern __attribute__((noreturn)) void asm_dump_regs(void);

typedef struct stack_frame_t {
    struct stack_frame_t *rbp;
    uint64_t rip;
} stack_frame_t;

char *get_function_name(uint64_t addr) {
    if (!addr)
        return "ERROR";

    char *krnl_file_start = (char *)boot_info.lkrnl->address;

    if (!elf_is_hdr_valid(krnl_file_start)) {
        return "INVALID";
    }

    elf_section_hdr_t *hdr_offset =
    	(elf_section_hdr_t *)(*(uint64_t *)((krnl_file_start + 40)) +
    						  ((uint64_t)krnl_file_start));
    uint16_t hdr_entry_count = *(uint16_t *)(krnl_file_start + 60);

    uint16_t hdr_str_table_idx = *(uint16_t *)(krnl_file_start + 62);
    char *hdr_str_table_off =
        (char *)(hdr_offset[hdr_str_table_idx].offset +
        ((uint64_t)krnl_file_start));

    symtab_entry_t *symtab_off = 0;
    char *strtab_off = 0;
    uint64_t symbtab_idx = 0;

    for (int i = 0; i < hdr_entry_count; i++) {
        if (strcmp(&hdr_str_table_off[hdr_offset[i].name], ".strtab") == 0) {
            strtab_off = (char *)(hdr_offset[i].offset +
                                  (uint64_t)krnl_file_start);
        }
        if (strcmp(&hdr_str_table_off[hdr_offset[i].name], ".symtab") == 0) {
            symtab_off = (symtab_entry_t *)(hdr_offset[i].offset +
                                            (uint64_t)krnl_file_start);
            symbtab_idx = i;
        }
    }

	for (int i = 0; i < hdr_offset[symbtab_idx].size / sizeof(symtab_entry_t);
		i++) {
			if(addr >= symtab_off[i].value &&
			   addr < symtab_off[i].value + symtab_off[i].size) {
				return &strtab_off[symtab_off[i].name];
		}
	}
    return "INVALID";
}

void print_panic_msg(void) {
    log_error("Kernel Panic");

    fprintf(VFS_FD_STDERR, "\n--- < kernel panic > ---\n"
                           "no need to panic though :)\n"
                           "reason: ");
}

void print_panic_reason(const char *fmt, va_list ap) {
    vfprintf(VFS_FD_STDERR, fmt, ap);
    fprintf(VFS_FD_STDERR, "\n");
}

void panic(const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);

    print_panic_msg();
    print_panic_reason(fmt, ap);

    asm_dump_regs();
}

void stack_trace(int depth, uint64_t rbp, uint64_t rip) {
    printf("-- < stack trace > --\n");

    printf("0x%016llx ", rip);
    if (rip)
        printf("\t%s", get_function_name(rip));
    printf("\n");

    stack_frame_t *stack = (stack_frame_t *)rbp;

    do {
        printf("0x%016llx ", stack->rip);
        if (stack->rip)
            printf("\t%s", get_function_name(stack->rip));
        printf("\n");
        stack = stack->rbp;
    } while (stack && --depth && stack->rip);
    printf(" | This is a very sad moment :(\n");
}

void dump_regs(registers_t *regs) {
    printf("-- < register dump > --\n");

    uint64_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

    printf("RIP=0x%016llx\n",
           regs->rip);

    printf("CS =0x%016llx\t\tDS =0x%016llx\n",
           regs->cs, regs->ds);

    printf("\n");
    printf("RAX=0x%016llx\t\tR8 =0x%016llx\n"
           "RBX=0x%016llx\t\tR9 =0x%016llx\n"
           "RCX=0x%016llx\t\tR10=0x%016llx\n"
           "RDX=0x%016llx\t\tR11=0x%016llx\n"
           "RSI=0x%016llx\t\tR12=0x%016llx\n"
           "RDI=0x%016llx\t\tR13=0x%016llx\n"
           "RBP=0x%016llx\t\tR14=0x%016llx\n"
           "RSP=0x%016llx\t\tR15=0x%016llx\n",
           regs->rax, regs->r8,
           regs->rbx, regs->r9,
           regs->rcx, regs->r10,
           regs->rdx, regs->r11,
           regs->rsi, regs->r12,
           regs->rdi, regs->r13,
           regs->rbp, regs->r14,
           regs->usermode_rsp, regs->r15);

    printf("CR2=0x%016llx\n", cr2);

    stack_trace(10, regs->rbp, regs->rip);

    fb_clear_color(COLOR(0, 0, 0), COLOR(0x88, 0xDF, 0x7C));
}
