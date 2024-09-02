#include "panic.h"
#include <util/debug.h>

void panic() {
    log_fatal(MODULE_MAIN, "Kernel panic!");
    __asm__ volatile("cli; hlt");
}

void dump_regs(registers_t *regs) {
    printf("---- < register dump > ----\n");

    printf("RIP=0x%016llx\n",
           regs->rip);

    printf("CS=0x%016llx\t\t DS =0x%016llx\n",
           regs->cs, regs->ds);

    printf("\n");
    printf("RAX=0x%016llx\t\tR8 =0x%016llx\n"
           "RBX=0x%016llx\t\tR9 =0x%016llx\n"
           "RCX=0x%016llx\t\tR10=0x%016llx\n"
           "RDX=0x%016llx\t\tR11=0x%016llx\n",
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
