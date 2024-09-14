#include "port.h"

void outb(uint16_t port, uint8_t byte) {
  __asm__ volatile("outb %1, %0" : : "dN"(port), "a"(byte));
}

uint8_t inb(uint16_t port) {
    uint8_t rv;
    __asm__ volatile("inb %1, %0" : "=a"(rv) : "dN"(port));
    return rv;
}

void io_wait(void) {
    outb(UNUSED_PORT, 0x00);
}
