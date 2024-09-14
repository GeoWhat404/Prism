#pragma once

#include "isr.h"
#include <stdarg.h>

void stack_trace(int depth, uint64_t rbp, uint64_t rip);
void dump_regs(registers_t *regs);

void print_panic_msg(void);
void print_panic_reason(const char *fmt, va_list ap);
void panic(const char *fmt, ...);
