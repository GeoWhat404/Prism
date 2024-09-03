#include "panic.h"

#include <stdio.h>
#include <stdarg.h>
#include <util/debug.h>

extern void asm_dump_regs();

void panic(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    log_fatal(MODULE_MAIN, "Kernel panic: %s", fmt);

    fprintf(VFS_FD_STDERR, "--- [ kernel panic ] ---\n"
                            "reason: ");
    vfprintf(VFS_FD_STDERR, fmt, ap);
    fprintf(VFS_FD_STDERR, "\n");

    asm_dump_regs();
}

void dump_regs(registers_t *regs) {
    printf("-- < register dump > --\n");

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
}
