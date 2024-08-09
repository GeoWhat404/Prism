#include "e9.h"
#include "port.h"

void e9_putc(char c) {
    outb(0xE9, c);
}
