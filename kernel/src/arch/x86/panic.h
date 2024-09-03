#pragma once

#include "isr.h"

void dump_regs(registers_t *regs);
void panic(const char *fmt, ...);
