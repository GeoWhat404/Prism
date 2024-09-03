#pragma once

#include "isr.h"

void stack_trace(int depth, uint64_t rbp, uint64_t rip);
void dump_regs(registers_t *regs);
void panic(const char *fmt, ...);
